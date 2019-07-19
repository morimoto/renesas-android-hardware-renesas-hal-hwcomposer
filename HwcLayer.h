/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_HWC_LAYER_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_HWC_LAYER_H

#include "HwcBuffer.h"

#include <android/hardware/graphics/composer/2.1/types.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>
#include <map>
#include <set>

namespace android {

class Importer;

class DrmHwcLayer {
public:
    int importBuffer(Importer* importer);

    buffer_handle_t mBuffHandle = NULL;
    DrmHwcBuffer mBuffer;
    UniqueFd mAcquireFence;
    uint32_t mTransform;
    uint8_t mAlpha = 0xff;
    hwc_frect_t mSourceCrop;
    hwc_rect_t mDisplayFrame;
    int index = -1;
};

class HwcLayer {
    using Error = android::hardware::graphics::composer::V2_1::Error;
public:

    HwcLayer(int index): mIndex(index) {}
    HwcLayer() = default;

    HWC2::Composition getSfType() const {
        return mSfType;
    }
    HWC2::Composition getValidatedType() const {
        return mValidatedType;
    }
    void acceptTypeChange() {
        mSfType = mValidatedType;
    }
    void setValidatedType(HWC2::Composition type) {
        mValidatedType = type;
    }
    bool typeChanged() const {
        return mSfType != mValidatedType;
    }
    HWC2::Transform getTransform() const {
        return mTransform;
    }
    bool isCursorLayer() const {
        return mIsCursorLayer;
    }
    bool isSkipComposition() const {
        return mIsSkipComposition;
    }

    uint32_t getZorder() const {
        return mZorder;
    }

    buffer_handle_t getBuffer() {
        return mHandle;
    }
    void setBuffer(buffer_handle_t buffer) {
        mHandle = buffer;
    }

    void setAcquireFence(int acquire_fence) {
        mAcquireFence.set(dup(acquire_fence));
    }

    void setReleaseFence(int fence) {
        mReleaseFence.set(dup(fence));
    }
    int takeReleaseFence() {
        return mReleaseFence.release();
    }

    bool checkLayer() const {
        int disp_w = mDisplayFrame.right - mDisplayFrame.left;
        int disp_h = mDisplayFrame.bottom - mDisplayFrame.top;
        int src_w = (int)(mSourceCrop.right - mSourceCrop.left);
        int src_h = (int)(mSourceCrop.bottom - mSourceCrop.top);
        bool res  = (disp_w > 1) && (disp_h > 1) && (src_w > 1) && (src_h > 1)
                    && src_w == disp_w && src_h == disp_h
                    ;
        return res;
    }

    int getIndex() const {
        return mIndex;
    }

    void populateDrmLayer(DrmHwcLayer* layer);

    // Layer hooks
    Error setCursorPosition(int32_t x, int32_t y);
    Error setLayerBlendMode(int32_t mode);
    Error setLayerBuffer(buffer_handle_t getBuffer, int32_t acquire_fence);
    Error setLayerColor(hwc_color_t color);
    Error setLayerCompositionType(int32_t type);
    Error setLayerDataspace(int32_t dataspace);
    Error setLayerDisplayFrame(hwc_rect_t frame);
    Error setLayerPlaneAlpha(float alpha);
    Error setLayerSidebandStream(const native_handle_t* stream);
    Error setLayerSourceCrop(hwc_frect_t crop);
    Error setLayerSurfaceDamage(hwc_region_t damage);
    Error setLayerTransform(int32_t transform);
    Error setLayerVisibleRegion(hwc_region_t visible);
    Error setLayerZOrder(uint32_t z);

private:
    // sf_type_ stores the initial type given to us by surfaceflinger,
    // validated_type_ stores the type after running ValidateDisplay
    HWC2::Composition mSfType = HWC2::Composition::Invalid;
    HWC2::Composition mValidatedType = HWC2::Composition::Invalid;
    HWC2::BlendMode mBlending = HWC2::BlendMode::None;
    buffer_handle_t mHandle;
    UniqueFd mAcquireFence;
    UniqueFd mReleaseFence;
    hwc_rect_t mDisplayFrame;
    float mAlpha = 1.0f;
    hwc_frect_t mSourceCrop;
    HWC2::Transform mTransform = HWC2::Transform::None;
    uint32_t mZorder = 0;
    android_dataspace_t mDataspace = HAL_DATASPACE_UNKNOWN;
    bool mIsCursorLayer = false;
    int mIndex = -1;
    bool mIsSkipComposition = false;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_HWC_LAYER_H
