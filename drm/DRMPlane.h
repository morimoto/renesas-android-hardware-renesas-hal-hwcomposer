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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_DRM_PLANE_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_DRM_PLANE_H

#include "DRMProperty.h"

namespace android {

class DrmHwcLayer;

class DRMPlane {
public:
    DRMPlane(uint32_t drm, drmModePlanePtr p);
    DRMPlane() = default;
    DRMPlane(DRMPlane&&) = default;
    DRMPlane& operator=(DRMPlane&&) = default;
    ///BCC: deleted
    DRMPlane(const DRMPlane&) = default;
    DRMPlane& operator=(const DRMPlane&) = default;

    int init();
    int updateProperties(drmModeAtomicReqPtr property_set, uint32_t crtc_id,
                         const DrmHwcLayer& layer, int zpos) const;
    int disable(drmModeAtomicReqPtr property_set) const;
    uint32_t getId() const;
    bool getCrtcSupported(uint32_t crtc_id) const;
    uint32_t getType() const;

    const DRMProperty& getCrtcProperty() const;
    const DRMProperty& getFbProperty() const;
    const DRMProperty& getCrtcXProperty() const;
    const DRMProperty& getCrtcYProperty() const;
    const DRMProperty& getCrtcWProperty() const;
    const DRMProperty& getCrtcHProperty() const;
    const DRMProperty& getSrcXProperty() const;
    const DRMProperty& getSrcYProperty() const;
    const DRMProperty& getSrcWProperty() const;
    const DRMProperty& getSrcHProperty() const;
    const DRMProperty& getAlphaProperty() const;
    const DRMProperty& getZposProperty() const;
    const DRMProperty& getFenceProperty() const;

private:
    bool isSupportedFormat(uint32_t format);

    uint32_t mDrm;
    uint32_t mId;

    uint32_t mPossibleCrtcMask;
    uint32_t mType;

    uint32_t mLastValidFormat;
    std::vector<uint32_t> mSupportedFormats;

    DRMProperty mCrtcProperty;
    DRMProperty mFbProperty;
    DRMProperty mCrtcXProperty;
    DRMProperty mCrtcYProperty;
    DRMProperty mCrtcWProperty;
    DRMProperty mCrtcHProperty;
    DRMProperty mSrcXProperty;
    DRMProperty mSrcYProperty;
    DRMProperty mSrcWProperty;
    DRMProperty mSrcHProperty;
    DRMProperty mAlphaProperty;
    DRMProperty mZposProperty;
    DRMProperty mFenceProperty;
};

} // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_DRM_PLANE_H
