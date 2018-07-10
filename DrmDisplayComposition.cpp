/*
 * Copyright (C) 2015 The Android Open Source Project
 * Copyright (C) 2018 GlobalLogic
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

#include "DrmDisplayComposition.h"

#include <algorithm>
#include <unordered_set>

#include <sync/sync.h>
#include <xf86drmMode.h>
#include <log/log.h>
#include <stdlib.h>

namespace android {

struct sw_sync_create_fence_data {
    __u32 value;
    char name[32];
    __s32 fence;
};

#define SW_SYNC_IOC_MAGIC 'W'
#define SW_SYNC_IOC_CREATE_FENCE _IOWR(SW_SYNC_IOC_MAGIC, 0, struct sw_sync_create_fence_data)
#define SW_SYNC_IOC_INC _IOW(SW_SYNC_IOC_MAGIC, 1, __u32)

DrmDisplayComposition::~DrmDisplayComposition() {
    if (mTimelineFd >= 0) {
        signalCompositionDone();
        close(mTimelineFd);
    }
}

int DrmDisplayComposition::init(int drm, uint32_t crtc) {
    mDrm = drm;
    mCrtc = crtc;  // Can be NULL if we haven't modeset yet

    // init timeline
    mTimelineFd = open("/sys/kernel/debug/sync/sw_sync", O_RDWR);
    if (mTimelineFd < 0)
        mTimelineFd = open("/dev/sw_sync", O_RDWR);

    if (mTimelineFd < 0) {
        ALOGE("Failed to create sw sync timeline, error=%d", -errno);
        return -1;
    }

    return 0;
}

bool DrmDisplayComposition::validateCompositionType(DrmCompositionType des) {
    return mType == DRM_COMPOSITION_TYPE_EMPTY || mType == des;
}

int DrmDisplayComposition::createNextTimelineFence() {
    ++mTimeline;

    struct sw_sync_create_fence_data fence_data = {
        .name = "drm display composition fence",
        .value = (__u32)mTimeline
    };

    int err = ioctl(mTimelineFd, SW_SYNC_IOC_CREATE_FENCE, &fence_data);
    if (err < 0)
        return err;

    return fence_data.fence;
}

int DrmDisplayComposition::signalCompositionDone() {
    return increaseTimelineToPoint(mTimeline);
}

int DrmDisplayComposition::increaseTimelineToPoint(int point) {
    int timeline_increase = point - mTtimelineCurrent;

    if (timeline_increase <= 0)
        return 0;

    int ret = ioctl(mTimelineFd, SW_SYNC_IOC_INC, &timeline_increase);

    if (ret)
        ALOGE("Failed to increment sync timeline %d", ret);
    else
        mTtimelineCurrent = point;

    return ret;
}

int DrmDisplayComposition::setLayers(DrmHwcLayer* layers, size_t num_layers) {
    if (!validateCompositionType(DRM_COMPOSITION_TYPE_FRAME)) {
        ALOGE("DrmDisplayComposition::SetLayers !validate_composition_type");
        return -EINVAL;
    }

    for (size_t layer_index = 0; layer_index < num_layers; layer_index++) {
        mLayers.emplace_back(std::move(layers[layer_index]));
    }

    mType = DRM_COMPOSITION_TYPE_FRAME;
    return 0;
}

int DrmDisplayComposition::setDisplayMode(const DRMMode& display_mode) {
    if (!validateCompositionType(DRM_COMPOSITION_TYPE_MODESET)) {
        ALOGE("DrmDisplayComposition::SetDisplayMode !validate_composition_type");
        return -EINVAL;
    }

    mDisplayMode = display_mode;
    mType = DRM_COMPOSITION_TYPE_MODESET;
    return 0;
}

int DrmDisplayComposition::createAndAssignReleaseFences() {
    if (mType != DRM_COMPOSITION_TYPE_FRAME)
        return 0;

    for (DrmHwcLayer& layer : mLayers) {
        if (!layer.mReleaseFence || layer.isClient)
            continue;

        int ret = layer.mReleaseFence.set(createNextTimelineFence());

        if (ret < 0) {
            ALOGE("Failed to set the release fence (comp) %d", ret);
            return ret;
        }
    }

    return 0;
}

std::vector<DrmHwcLayer>& DrmDisplayComposition::getLayers() {
    return mLayers;
}

DrmCompositionType DrmDisplayComposition::getType() const {
    return mType;
}

const DRMMode& DrmDisplayComposition::getDisplayMode() const {
    return mDisplayMode;
}

int DrmDisplayComposition::takeOutFence() {
    return mOutFence.release();
}

void DrmDisplayComposition::setOutFence(int out_fence) {
    mOutFence.set(dup(out_fence));
}

} // namespace android
