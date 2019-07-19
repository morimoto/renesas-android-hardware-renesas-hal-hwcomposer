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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_EVENT_WORKER_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_EVENT_WORKER_H

#include "worker.h"

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <map>
#include <stdint.h>

namespace android {

class VsyncCallback {
public:
    virtual ~VsyncCallback() {
    }
    virtual void callback(int display, int64_t timestamp) = 0;
};

class VSyncWorker : public Worker {
public:
    VSyncWorker();
    ~VSyncWorker() override;

    int init(int drmFd, int display, int refresh);
    void registerCallback(std::shared_ptr<VsyncCallback> callback);

    void controlVSync(bool enabled);

protected:
    void routine() override;

private:
    int64_t getPhasedVSync(int64_t frame_ns, int64_t current);
    int syntheticWaitVBlank(int64_t* timestamp);

    int mDrmFd;

    // shared_ptr since we need to use this outside of the thread lock (to
    // actually call the hook) and we don't want the memory freed until we're
    // done
    std::shared_ptr<VsyncCallback> mCallback = NULL;

    int mDisplay;
    int mRefreshRate = 60; // Default to 60Hz refresh rate
    bool mEnabled;
    int64_t mLastTimestamp;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_EVENT_WORKER_H
