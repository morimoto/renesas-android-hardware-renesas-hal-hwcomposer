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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_BUFFER_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_BUFFER_H

#include "autofd.h"
#include "HwcDump.h"

#include <stdbool.h>
#include <stdint.h>

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <hardware/hwcomposer2.h>

namespace android {

class Importer;

struct DrmHwcBo {
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFormat; /* DRM_FORMAT_* from drm_fourcc.h */
    uint32_t mPitches[4];
    uint32_t mOffsets[4];
    uint32_t mGemHandles[4];
    uint32_t mFbId = 0;
    int mAcquireFence;
    int index = -1;
    unsigned long long mStamp = 0;
};

class DrmHwcBuffer {
public:
    DrmHwcBuffer() = default;
    DrmHwcBuffer(const DrmHwcBo& bo, Importer* importer)
        : mBo(bo), mImporter(importer) {
    }

    DrmHwcBuffer(DrmHwcBuffer&& rhs) : mBo(rhs.mBo), mImporter(rhs.mImporter) {
        rhs.mImporter = NULL;
    }

    ~DrmHwcBuffer() {
        clear();
    }

    DrmHwcBuffer& operator=(DrmHwcBuffer&& rhs) {
        clear();
        mImporter = rhs.mImporter;
        rhs.mImporter = NULL;
        mBo = rhs.mBo;
        return *this;
    }

    operator bool() const {
        return mImporter != NULL;
    }

    const DrmHwcBo* operator->() const;

    void clear();
    int createFrameBuffer();
    int importBuffer(buffer_handle_t handle, Importer* importer, int index);

private:
    DrmHwcBo mBo;
    Importer* mImporter = NULL;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_BUFFER_H
