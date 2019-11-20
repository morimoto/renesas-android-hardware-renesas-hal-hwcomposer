/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vsyncworker.h"
#include "worker.h"

#include <log/log.h>
#include <hardware/hardware.h>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <cerrno>

namespace android {

VSyncWorker::VSyncWorker()
    : Worker("vsync", HAL_PRIORITY_URGENT_DISPLAY)
    , mDrmFd(-1)
    , mDisplay(-1)
    , mEnabled(false)
    , mLastTimestamp(-1) {
}

VSyncWorker::~VSyncWorker() {
}

int VSyncWorker::init(int drmFd , int display, int refresh) {
    mDrmFd = drmFd;
    mDisplay = display;
    if (refresh)
        mRefreshRate = refresh;
    return initWorker();
}

void VSyncWorker::registerCallback(std::shared_ptr<VsyncCallback> callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mCallback = callback;
}

void VSyncWorker::controlVSync(bool enabled) {
    std::lock_guard<std::mutex> lock(mMutex);

    mEnabled = enabled;
    mLastTimestamp = -1;

    signal();
}

/*
 * Returns the timestamp of the next vsync in phase with last_timestamp_.
 * For example:
 *  last_timestamp_ = 137
 *  frame_ns = 50
 *  current = 683
 *
 *  ret = (50 * ((683 - 137)/50 + 1)) + 137
 *  ret = 687
 *
 *  Thus, we must sleep until timestamp 687 to maintain phase with the last
 *  timestamp.
 */
int64_t VSyncWorker::getPhasedVSync(int64_t frame_ns, int64_t current) {
    if (mLastTimestamp < 0)
        return current + frame_ns;

    return frame_ns * ((current - mLastTimestamp) / frame_ns + 1)
           + mLastTimestamp;
}

static const int64_t kOneSecondNs = 1 * 1000 * 1000 * 1000;

int VSyncWorker::syntheticWaitVBlank(int64_t* timestamp) {
    struct timespec vsync;
    int ret = clock_gettime(CLOCK_MONOTONIC, &vsync);
    int64_t phased_timestamp = getPhasedVSync(kOneSecondNs / mRefreshRate,
                               vsync.tv_sec * kOneSecondNs + vsync.tv_nsec);
    vsync.tv_sec = phased_timestamp / kOneSecondNs;
    vsync.tv_nsec = phased_timestamp - (vsync.tv_sec * kOneSecondNs);

    do {
        ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &vsync, NULL);
    } while (ret == -1 && errno == EINTR);

    if (ret)
        return ret;

    *timestamp = (int64_t) vsync.tv_sec * kOneSecondNs
                 + (int64_t) vsync.tv_nsec;
    return 0;
}

void VSyncWorker::routine() {
    int ret = 0;
    std::unique_lock<std::mutex> lock(mMutex);

    if (!mEnabled) {
        ret = waitForSignalOrExitLocked();

        if (ret == -EINTR) {
            return;
        }
    }

    bool enabled = mEnabled;
    int display = mDisplay;
    std::shared_ptr<VsyncCallback> callback(mCallback);
    lock.unlock();

    if (!enabled)
        return;

    uint32_t high_crtc = (display << DRM_VBLANK_HIGH_CRTC_SHIFT);
    drmVBlank vblank;
    memset(&vblank, 0, sizeof(vblank));
    vblank.request.type = (drmVBlankSeqType) (DRM_VBLANK_RELATIVE
                          | (high_crtc & DRM_VBLANK_HIGH_CRTC_MASK));
    vblank.request.sequence = 1;
    int64_t timestamp;
    ret = drmWaitVBlank(mDrmFd, &vblank);

    if (ret == -EINTR) {
        return;
    } else if (ret) {
        ret = syntheticWaitVBlank(&timestamp);

        if (ret)
            return;
    } else {
        timestamp = (int64_t) vblank.reply.tval_sec * kOneSecondNs
                    + (int64_t) vblank.reply.tval_usec * 1000;
    }

    /*
     * There's a race here where a change in callback_ will not take effect until
     * the next subsequent requested vsync. This is unavoidable since we can't
     * call the vsync hook while holding the thread lock.
     *
     * We could shorten the race window by caching callback_ right before calling
     * the hook. However, in practice, callback_ is only updated once, so it's not
     * worth the overhead.
     *
     * Update: hwcomposer vts'es may change the mCallback and mEnabled and expect
     *         that changes will take effect immediately but it is not the case.
     *         The window of unexpected VSYNC callback triggering is maximum shortened
     *         but still exist.
     *         In normal hwcomposer work (not VTS'es) this problem don't arise,
     *         satisfying questionable vts consider as unreasonable.
     */
    if (callback && mEnabled) {
        callback->callback(display, timestamp);
    }

    mLastTimestamp = timestamp;
}

} // namespace android
