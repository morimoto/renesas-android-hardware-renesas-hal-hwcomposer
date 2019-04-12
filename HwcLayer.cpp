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

#include "HwcLayer.h"
#include "Hwc.h"
#include "img_gralloc1_public.h"

namespace android {
using Error = android::hardware::graphics::composer::V2_1::Error;

Error HwcLayer::setCursorPosition(int32_t x, int32_t y) {
    unsupported(__func__, x, y);
    // TODO: Implement proper cursor support
    return Error::NONE;
}

Error HwcLayer::setLayerBlendMode(int32_t mode) {
    supported(__func__);
    mBlending = static_cast<HWC2::BlendMode>(mode);
    return Error::NONE;
}

Error HwcLayer::setLayerBuffer(buffer_handle_t buffer,
                               int32_t acquire_fence) {
    supported(__func__);
    UniqueFd af(acquire_fence);

    // The buffer and acquire_fence are handled elsewhere
    if (mSfType == HWC2::Composition::Client
        || mSfType == HWC2::Composition::Sideband
        || mSfType == HWC2::Composition::SolidColor)
        return Error::NONE;

    setBuffer(buffer);
    setAcquireFence(af.get());
    return Error::NONE;
}

Error HwcLayer::setLayerColor(hwc_color_t color) {
    unsupported(__func__, color);
    // Probably we should query for the plane capabilities here, before
    // always falling back for client composition ?
    // TODO: Punt to client composition here?
    mSfType = HWC2::Composition::Client;
    return Error::NONE;
}

Error HwcLayer::setLayerCompositionType(int32_t type) {
    mSfType = static_cast<HWC2::Composition>(type);

    if (mSfType == HWC2::Composition::Cursor)
        mIsCursorLayer = true;

    return Error::NONE;
}

Error HwcLayer::setLayerDataspace(int32_t dataspace) {
    supported(__func__);
    mDataspace = static_cast<android_dataspace_t>(dataspace);
    return Error::NONE;
}

Error HwcLayer::setLayerDisplayFrame(hwc_rect_t frame) {
    supported(__func__);
    mDisplayFrame = frame;
    return Error::NONE;
}

Error HwcLayer::setLayerPlaneAlpha(float alpha) {
    supported(__func__);
    mAlpha = alpha;
    return Error::NONE;
}

Error HwcLayer::setLayerSidebandStream(
    const native_handle_t* stream) {
    unsupported(__func__, stream);
    // TODO: We don't support sideband
    mSfType = HWC2::Composition::Client;
    return Error::NONE;
}

Error HwcLayer::setLayerSourceCrop(hwc_frect_t crop) {
    supported(__func__);
    mSourceCrop = crop;
    return Error::NONE;
}

Error HwcLayer::setLayerSurfaceDamage(hwc_region_t damage) {
    supported(__func__);
    // TODO: We don't use surface damage, marking as unsupported
    unsupported(__func__, damage);
    return Error::NONE;
}

Error HwcLayer::setLayerTransform(int32_t transform) {
    supported(__func__);
    mTransform = static_cast<HWC2::Transform>(transform);
    return Error::NONE;
}

Error HwcLayer::setLayerVisibleRegion(hwc_region_t visible) {
    supported(__func__);
    // TODO: We don't use this information, marking as unsupported
    unsupported(__func__, visible);
    return Error::NONE;
}

Error HwcLayer::setLayerZOrder(uint32_t order) {
    supported(__func__);
    mZorder = order;
    return Error::NONE;
}

void HwcLayer::populateDrmLayer(DrmHwcLayer* layer) {
    supported(__func__);
    layer->mBuffHandle = mHandle;
    layer->mAcquireFence = mAcquireFence.release();
    layer->mDisplayFrame = mDisplayFrame;
    layer->mAlpha = static_cast<uint8_t>(255.0f * mAlpha + 0.5f);
    layer->mSourceCrop = mSourceCrop;
    layer->index = mIndex;
}

} // namespace android
