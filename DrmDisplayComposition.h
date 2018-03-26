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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_DRM_DISPLAY_COMPOSITION_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_DRM_DISPLAY_COMPOSITION_H

#include "HwcLayer.h"
#include "drm/DRMMode.h"
#include "drm/DRMPlane.h"

#include <hardware/gralloc.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <sstream>
#include <vector>

namespace android {

class Importer;

enum DrmCompositionType {
    DRM_COMPOSITION_TYPE_EMPTY,
    DRM_COMPOSITION_TYPE_FRAME,
    DRM_COMPOSITION_TYPE_MODESET,
};

class DrmDisplayComposition {
public:
    DrmDisplayComposition() = default;
    DrmDisplayComposition(const DrmDisplayComposition&) = delete;
    ~DrmDisplayComposition();

    int init(int drm, uint32_t crtc);
    DrmCompositionType getType() const;

    int setLayers(DrmHwcLayer* layers, size_t num_layers);
    std::vector<DrmHwcLayer>& getLayers();

    int setDisplayMode(const DRMMode& display_mode);
    const DRMMode& getDisplayMode() const;

    int createNextTimelineFence();
    int signalCompositionDone();
    int increaseTimelineToPoint(int point);
    int createAndAssignReleaseFences();

    int takeOutFence();
    void setOutFence(int out_fence);

private:
    bool validateCompositionType(DrmCompositionType desired);

    int mDrm;
    uint32_t mCrtc;

    DrmCompositionType mType = DRM_COMPOSITION_TYPE_EMPTY;
    DRMMode mDisplayMode;

    int mTimelineFd = -1;
    int mTimeline = 0;
    int mTtimelineCurrent = 0;
    UniqueFd mOutFence = -1;

    std::vector<DrmHwcLayer> mLayers;
};

} // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_DRM_DISPLAY_COMPOSITION_H
