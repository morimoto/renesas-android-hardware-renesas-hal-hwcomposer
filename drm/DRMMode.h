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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_DRM_MODE_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_DRM_MODE_H

#include <string>
#include <xf86drmMode.h>

namespace android {

class DRMMode {
public:
    DRMMode() = default;
    DRMMode(drmModeModeInfoPtr m);

    bool operator==(const drmModeModeInfo& m) const;
    void toDrmModeModeInfo(drmModeModeInfo* m) const;

    uint32_t getId() const;
    void setId(uint32_t id);

    uint32_t getClock() const;

    uint16_t getHDisplay() const;
    uint16_t getHSyncStart() const;
    uint16_t getHSyncEnd() const;
    uint16_t getHTotal() const;
    uint16_t getHSkew() const;

    uint16_t getVDisplay() const;
    uint16_t getVSyncStart() const;
    uint16_t getVSyncEnd() const;
    uint16_t getVTotal() const;
    uint16_t getVScan() const;
    float getVRefresh() const;

    uint32_t getFlags() const;
    uint32_t getType() const;

    std::string getName() const;

private:
    uint32_t mId = 0;

    uint32_t mClock = 0;

    uint16_t mHDisplay = 0;
    uint16_t mHSyncStart = 0;
    uint16_t mHSyncEnd = 0;
    uint16_t mHTotal = 0;
    uint16_t mHSkew = 0;

    uint16_t mVDisplay = 0;
    uint16_t mVSyncStart = 0;
    uint16_t mVSyncEnd = 0;
    uint16_t mVTotal = 0;
    uint16_t mVScan = 0;
    uint32_t mVRefresh = 0;

    uint32_t mFlags = 0;
    uint32_t mType = 0;

    std::string mName;
};

} // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_DRM_MODE_H
