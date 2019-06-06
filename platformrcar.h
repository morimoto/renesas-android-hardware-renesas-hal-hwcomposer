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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_PLATFORM_RCAR_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_PLATFORM_RCAR_H

#include "platform.h"
#include "PrimeCache.h"
#include <mutex>

namespace android {

class RCarImporter : public Importer {
public:
    RCarImporter(int drm_id);
    ~RCarImporter() override = default;


    int importBuffer(buffer_handle_t handle, DrmHwcBo* bo) override;
    int releaseBuffer(DrmHwcBo* bo) override;
    int createFrameBuffer(DrmHwcBo* bo) override;
#if HWC_PRIME_CACHE
    void setPrimeCache(PrimeCache* primeCache) override {
        mPrimeCache = primeCache;
    };
    PrimeCache* getPrimeCache() const override { return mPrimeCache; }
#endif

private:
    int getIonBufferFd(int bufferFd, int format
                       , int width, int height
                       , DrmHwcBo* bo);

    int mDrmFd;
    std::mutex mLock;
#if HWC_PRIME_CACHE
    PrimeCache* mPrimeCache = nullptr;
#endif
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_PLATFORM_RCAR_H
