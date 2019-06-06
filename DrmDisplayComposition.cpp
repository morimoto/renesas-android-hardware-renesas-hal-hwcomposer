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

namespace android {

int DrmDisplayComposition::init(int drm, uint32_t crtc) {
    mDrm = drm;
    mCrtc = crtc;  // Can be NULL if we haven't modeset yet
    return 0;
}

bool DrmDisplayComposition::validateCompositionType(DrmCompositionType des) {
    return mType == DRM_COMPOSITION_TYPE_EMPTY || mType == des;
}

int DrmDisplayComposition::setLayers(std::vector<DrmHwcLayer>&& layers) {
    if (!validateCompositionType(DRM_COMPOSITION_TYPE_FRAME)) {
        ALOGE("DrmDisplayComposition::SetLayers !validate_composition_type");
        return -EINVAL;
    }

    mLayers = std::move(layers);
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

std::vector<DrmHwcLayer>& DrmDisplayComposition::getLayers() {
    return mLayers;
}

DrmCompositionType DrmDisplayComposition::getType() const {
    return mType;
}

const DRMMode& DrmDisplayComposition::getDisplayMode() const {
    return mDisplayMode;
}

} // namespace android
