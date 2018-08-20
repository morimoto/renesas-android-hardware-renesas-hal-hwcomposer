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

#include "HwcDump.h"
#include "platform.h"
#include "platformrcar.h"
#include "img_gralloc1_public.h"

#include <drm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <cutils/log.h>

#define ALIGN_ROUND_UP(X, Y)  (((X)+(Y)-1) & ~((Y)-1))

namespace android {

Importer* Importer::createInstance(int drm_id) {
    RCarImporter* importer = new RCarImporter(drm_id);

    if (!importer)
        return NULL;

    int ret = importer->init();

    if (ret) {
        ALOGE("Failed to initialize the rcar importer %d", ret);
        delete importer;
        return NULL;
    }

    return importer;
}

RCarImporter::RCarImporter(int drm_id) : mDrmFd(drm_id) {
}

RCarImporter::~RCarImporter() {
}

int RCarImporter::init() {
    return 0;
}

int RCarImporter::getIonBufferFd(int bufferFd, int format
                                 , int width, int height
                                 , DrmHwcBo* bo) {
    int stride_width;
    uint32_t handle;
    handle = 0;
    int fd = -1;

    /* configure width with align */
    if (format == HAL_PIXEL_FORMAT_NV12_CUSTOM
        || format == HAL_PIXEL_FORMAT_NV21_CUSTOM) {
        stride_width = ALIGN_ROUND_UP(width, 128);
    } else {
        stride_width = ALIGN_ROUND_UP(width, HW_ALIGN);
    }

    /* register ion memory to drm driver */
    if (drmPrimeFDToHandle(mDrmFd, bufferFd, &handle)) {
        ALOGE("mIonBuffers: drmPrimeFDToHandle error fd: %d", bufferFd);
    } else {
        uint32_t bo_handles[4] = { 0 };
        uint32_t pitches[4] = { 0 };
        uint32_t offsets[4] = { 0 };
        drm_gem_close arg = { handle, 0, };

        /* configure format and pitch */
        switch (format) {
        case HAL_PIXEL_FORMAT_BGRX_8888:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 4;
            format = DRM_FORMAT_XRGB8888;
            break;

        case HAL_PIXEL_FORMAT_BGRA_8888:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 4;
            format = DRM_FORMAT_ARGB8888;
            break;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 4;
            format = DRM_FORMAT_XBGR8888;
            break;

        case HAL_PIXEL_FORMAT_RGBA_8888:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 4;
            format = DRM_FORMAT_ABGR8888;
            break;

        case HAL_PIXEL_FORMAT_RGB_888:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 3;
            format = DRM_FORMAT_RGB888;
            break;

        case HAL_PIXEL_FORMAT_RGB_565:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 2;
            format = DRM_FORMAT_RGB565;
            break;

        case HAL_PIXEL_FORMAT_UYVY:
            bo_handles[0] = handle;
            pitches[0] = stride_width * 2;
            format = DRM_FORMAT_UYVY;
            break;

        case HAL_PIXEL_FORMAT_NV12:
        case HAL_PIXEL_FORMAT_NV12_CUSTOM:
            bo_handles[0] = handle;
            bo_handles[1] = handle;
            pitches[0] = stride_width;
            pitches[1] = stride_width;
            offsets[1] = pitches[0] * height;
            format = DRM_FORMAT_NV12;
            break;

        case HAL_PIXEL_FORMAT_NV21:
        case HAL_PIXEL_FORMAT_NV21_CUSTOM:
            bo_handles[0] = handle;
            bo_handles[1] = handle;
            pitches[0] = stride_width;
            pitches[1] = stride_width;
            offsets[1] = pitches[0] * height;
            format = DRM_FORMAT_NV21;
            break;

        case HAL_PIXEL_FORMAT_YV12:
            bo_handles[0] = handle;
            bo_handles[1] = handle;
            bo_handles[2] = handle;
            pitches[0] = stride_width;
            pitches[1] = stride_width / 2;
            pitches[2] = stride_width / 2;
            offsets[1] = pitches[0] * height;
            offsets[2] = offsets[1] + pitches[1] * ((height + 1) / 2);
            format = DRM_FORMAT_YVU420;
            break;

        default:
            format = 0;
        }

        bo->mWidth = width;
        bo->mHeight = height;
        bo->mFormat = format;

        for (int i = 0; i < 4; ++i) {
            bo->mPitches[i] = pitches[i];
            bo->mOffsets[i] = offsets[i];
            bo->mGemHandles[i] = bo_handles[i];
        }

        CHECK_RES_WARN(drmModeAddFB2(mDrmFd, width, height, format, bo_handles, pitches,
                                     offsets, (uint32_t*) &fd, 0));

        // close import reference
        if (drmIoctl(mDrmFd, DRM_IOCTL_GEM_CLOSE, &arg)) {
            ALOGE("mIonBuffers: free handle from drmPrimeFDToHandle");
        }

        handle = 0;
    }

    return fd;
}

int RCarImporter::importBuffer(buffer_handle_t handle, DrmHwcBo* bo) {
    const IMG_native_handle_t* imgHnd2 =
        reinterpret_cast<const IMG_native_handle_t*>(handle);

    if (!imgHnd2) {
        ALOGE("mIonBuffers: ImportBuffer fail");
        return -1;
    }

    int fb_id = getIonBufferFd(imgHnd2->fd[0], imgHnd2->iFormat,
                               imgHnd2->iWidth, imgHnd2->iHeight, bo);

    if (fb_id > 0)
        bo->mFbId = (uint32_t)fb_id;

    return (int)(fb_id <= 0);
}

int RCarImporter::createFrameBuffer(DrmHwcBo*) {
    ALOGD("mIonBuffers: CreateFrameBuffer");
    return 0;
}

int RCarImporter::releaseBuffer(DrmHwcBo* bo) {
    if (bo->mFbId) {
        if (drmModeRmFB(mDrmFd, bo->mFbId)) {
            ALOGE("mIonBuffers: Failed to rm fb");
        }
    }

    return 0;
}

} // namespace android
