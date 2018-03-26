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

#include "DRMPlane.h"
#include "DRMProperty.h"
#include "HwcLayer.h"

#include <cutils/log.h>
#include <cinttypes>
#include <errno.h>

namespace android {

DRMPlane::DRMPlane(uint32_t drm, drmModePlanePtr p)
    : mDrm(drm)
    , mId(p->plane_id)
    , mPossibleCrtcMask(p->possible_crtcs) {
    for (uint32_t j = 0; j < p->count_formats; j++)
        mSupportedFormats.emplace_back(p->formats[j]);

    mLastValidFormat = 0;
}

int DRMPlane::init() {
    DRMProperty p;
    int ret = DRMProperty::getPlaneProperty(mDrm, mId, "type", &p);

    if (ret) {
        ALOGE("DrmPlane: Could not get plane type property");
        return ret;
    }

    uint64_t type;
    ret = p.getValue(&type);

    if (ret) {
        ALOGE("DrmPlane: Failed to get plane type property value");
        return ret;
    }

    switch (type) {
    case DRM_PLANE_TYPE_OVERLAY:
    case DRM_PLANE_TYPE_PRIMARY:
    case DRM_PLANE_TYPE_CURSOR:
        mType = (uint32_t)type;
        break;

    default:
        ALOGE("DrmPlane: Invalid plane type %zu", type);
        return -EINVAL;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "CRTC_ID", &mCrtcProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get CRTC_ID property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "FB_ID", &mFbProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get FB_ID property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "CRTC_X", &mCrtcXProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get CRTC_X property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "CRTC_Y", &mCrtcYProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get CRTC_Y property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "CRTC_W", &mCrtcWProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get CRTC_W property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "CRTC_H", &mCrtcHProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get CRTC_H property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "SRC_X", &mSrcXProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get SRC_X property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "SRC_Y", &mSrcYProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get SRC_Y property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "SRC_W", &mSrcWProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get SRC_W property");
        return ret;
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "SRC_H", &mSrcHProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get SRC_H property");
        return ret;
    }

    if (mType == DRM_PLANE_TYPE_OVERLAY) {
        ret = DRMProperty::getPlaneProperty(mDrm, mId, "alpha", &mAlphaProperty);

        if (ret) {
            ALOGE("DrmPlane: Could not get alpha property");
            return ret;
        }

        ret = DRMProperty::getPlaneProperty(mDrm, mId, "zpos", &mZposProperty);

        if (ret) {
            ALOGE("DrmPlane: Could not get zpos property");
            return ret;
        }
    }

    ret = DRMProperty::getPlaneProperty(mDrm, mId, "IN_FENCE_FD", &mFenceProperty);

    if (ret) {
        ALOGE("DrmPlane: Could not get IN_FENCE_FD property");
        return ret;
    }

    return 0;
}

int DRMPlane::updateProperties(drmModeAtomicReqPtr property_set,
                               uint32_t crtc_id,
                               const DrmHwcLayer& layer, int zpos) const {
    uint64_t alpha = layer.mAlpha;
    hwc_rect_t display_frame = layer.mDisplayFrame;
    hwc_frect_t source_crop = layer.mSourceCrop;
    int fence = layer.mAcquireFence.get();
    int success = drmModeAtomicAddProperty(property_set, mId, mCrtcProperty.getId(),
                                           crtc_id) < 0;
    success |= drmModeAtomicAddProperty(property_set, mId, mFbProperty.getId(),
                                        layer.mBuffer->mFbId) < 0;
    success |= drmModeAtomicAddProperty(property_set, mId, mCrtcXProperty.getId(),
                                        display_frame.left) < 0;
    success |= drmModeAtomicAddProperty(property_set, mId, mCrtcYProperty.getId(),
                                        display_frame.top) < 0;
    success |=
        drmModeAtomicAddProperty(property_set, mId, mCrtcWProperty.getId(),
                                 display_frame.right - display_frame.left) < 0;
    success |=
        drmModeAtomicAddProperty(property_set, mId, mCrtcHProperty.getId(),
                                 display_frame.bottom - display_frame.top) < 0;
    success |= drmModeAtomicAddProperty(property_set, mId, mSrcXProperty.getId(),
                                        (int)(source_crop.left) << 16) < 0;
    success |= drmModeAtomicAddProperty(property_set, mId, mSrcYProperty.getId(),
                                        (int)(source_crop.top) << 16) < 0;
    success |= drmModeAtomicAddProperty(
                   property_set, mId, mSrcWProperty.getId(),
                   (int)(source_crop.right - source_crop.left) << 16) < 0;
    success |= drmModeAtomicAddProperty(
                   property_set, mId, mSrcHProperty.getId(),
                   (int)(source_crop.bottom - source_crop.top) << 16) < 0;

    if (mType == DRM_PLANE_TYPE_OVERLAY) {
        if (mAlphaProperty.getId()) {
            success |= drmModeAtomicAddProperty(property_set, mId, mAlphaProperty.getId(),
                                                alpha) < 0;
        }

        if (mZposProperty.getId()) {
            success |= drmModeAtomicAddProperty(property_set, mId, mZposProperty.getId(),
                                                zpos) < 0;
        }
    }

    if (fence != -1 && mFenceProperty.getId()) {
        success |= drmModeAtomicAddProperty(property_set, mId,
                                            mFenceProperty.getId(), fence) < 0;
    }

    if (success) {
        ALOGE("DrmPlane: Could not update properties for plane with id: %d", mId);
        return -EINVAL;
    }

    return success;
}

int DRMPlane::disable(drmModeAtomicReqPtr property_set) const {
    int success =
        drmModeAtomicAddProperty(property_set, mId, mCrtcProperty.getId(), 0) < 0;
    success |=
        drmModeAtomicAddProperty(property_set, mId, mFbProperty.getId(), 0) < 0;

    if (success) {
        ALOGE("DrmPlane: Failed to disable plane with id: %d", mId);
        return -EINVAL;
    }

    return success;
}

uint32_t DRMPlane::getId() const {
    return mId;
}

bool DRMPlane::getCrtcSupported(uint32_t crtc_id) const {
    return !!((1 << crtc_id) & mPossibleCrtcMask);
}

uint32_t DRMPlane::getType() const {
    return mType;
}

const DRMProperty& DRMPlane::getCrtcProperty() const {
    return mCrtcProperty;
}

const DRMProperty& DRMPlane::getFbProperty() const {
    return mFbProperty;
}

const DRMProperty& DRMPlane::getCrtcXProperty() const {
    return mCrtcXProperty;
}

const DRMProperty& DRMPlane::getCrtcYProperty() const {
    return mCrtcYProperty;
}

const DRMProperty& DRMPlane::getCrtcWProperty() const {
    return mCrtcWProperty;
}

const DRMProperty& DRMPlane::getCrtcHProperty() const {
    return mCrtcHProperty;
}

const DRMProperty& DRMPlane::getSrcXProperty() const {
    return mSrcXProperty;
}

const DRMProperty& DRMPlane::getSrcYProperty() const {
    return mSrcYProperty;
}

const DRMProperty& DRMPlane::getSrcWProperty() const {
    return mSrcWProperty;
}

const DRMProperty& DRMPlane::getSrcHProperty() const {
    return mSrcHProperty;
}

const DRMProperty& DRMPlane::getAlphaProperty() const {
    return mAlphaProperty;
}

const DRMProperty& DRMPlane::getZposProperty() const {
    return mZposProperty;
}

const DRMProperty& DRMPlane::getFenceProperty() const {
    return mFenceProperty;
}

bool DRMPlane::isSupportedFormat(uint32_t format) {
    if (mLastValidFormat == format)
        return true;

    for (auto& element : mSupportedFormats) {
        if (element == format) {
            mLastValidFormat = format;
            return true;
        }
    }

    return false;
}

} // namespace android
