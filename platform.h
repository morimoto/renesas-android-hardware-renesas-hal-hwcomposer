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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_PLATFORM_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_PLATFORM_H

#include "DrmDisplayComposition.h"

namespace android {

class Importer {
public:
    virtual ~Importer() {}

    // Creates a platform-specific importer instance
    static Importer* createInstance(int drm_id);

    // Imports the buffer referred to by handle into bo.
    //
    // Note: This can be called from a different thread than ReleaseBuffer. The
    //       implementation is responsible for ensuring thread safety.
    virtual int importBuffer(buffer_handle_t handle, DrmHwcBo* bo) = 0;

    // Releases the buffer object (ie: does the inverse of ImportBuffer)
    //
    // Note: This can be called from a different thread than ImportBuffer. The
    //       implementation is responsible for ensuring thread safety.
    virtual int releaseBuffer(DrmHwcBo* bo) = 0;

    // Creates a new framebuffer with bo as it's scanout buffer.
    //
    // Note: This can be called from a different thread than ImportBuffer. The
    //       implementation is responsible for ensuring thread safety.
    virtual int createFrameBuffer(DrmHwcBo* bo) = 0;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_PLATFORM_H
