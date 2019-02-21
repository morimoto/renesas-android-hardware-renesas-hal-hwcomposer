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

#include "HwcDisplay.h"
#include "Hwc.h"
#include "HwcDump.h"
#include "DrmDisplayComposition.h"
#include "drm/DRMProperty.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>

#include <drm_fourcc.h>
#include <cutils/properties.h>
#include <inttypes.h>
#include <string.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

namespace android {
using Error = android::hardware::graphics::composer::V2_1::Error;
using android::hardware::graphics::composer::V2_1::implementation::HwcHal;

class DrmVsyncCallback: public VsyncCallback {
public:
    DrmVsyncCallback(hwc2_callback_data_t data, hwc2_function_pointer_t hook) :
        data_(data), hook_(hook) {
    }

    void callback(int display, int64_t timestamp) {
        auto hook = reinterpret_cast<HWC2_PFN_VSYNC>(hook_);
        hook(data_, display, timestamp);
    }

private:
    hwc2_callback_data_t data_;
    hwc2_function_pointer_t hook_;
};

#if DEBUG_FRAMERATE
static void page_flip_handler(int //fd
                              , uint32_t //sequence
                              , uint32_t tv_sec
                              , uint32_t tv_usec
                              , void* user_data
                              ) {
    static char value[PROPERTY_VALUE_MAX];
    property_get("debug.hwc.showfps", value, "0");
    bool show_fps = std::atoi(value);
    if (!show_fps)
        return;

    HwcDisplay* disp = reinterpret_cast<HwcDisplay*>(user_data);
    if (!disp)
        return;

    int& fps_frame_count = disp->mFpsFrameCount;
    uint32_t& start_sec = disp->mFpsStartSec;
    uint32_t& start_usec = disp->mFpsStartUsec;

    if (fps_frame_count == 0) {
        start_sec = tv_sec;
        start_usec = tv_usec;
        fps_frame_count++;

    } else {
        int sec = tv_sec - start_sec;
        int usec = tv_usec - start_usec;
        int time = usec;
        time += sec*1000000;

        if (sec > 2 || time >= 1*1000000) {
            float float_sec = (float)usec / (float)1000000;
            float_sec += sec;
            ALOGD("fps:%5.1f (%d frame per %f sec)", (float)fps_frame_count / float_sec, fps_frame_count, float_sec);
            start_sec = tv_sec;
            start_usec = tv_usec;
            fps_frame_count = 0;
        }
        fps_frame_count++;
    }
}
#endif //DEBUG_FRAMERATE

HwcDisplay::HwcDisplay(int drmFd, hwc2_display_t handle, hwdisplay params,
                       HWC2::DisplayType type, std::shared_ptr<Importer> importer)
    : mDrmFd(drmFd)
    , mCurrConfig(-1)
    , mConnectorId(0)
    , mHandle(handle)
    , mType(type)
    , mCrtId(-1)
    , mDisplayParams(params)
    , mImporter(importer)

{
    supported(__func__);
}

Error HwcDisplay::init() {
    supported(__func__);
    int display = static_cast<int>(mHandle);
    ALOGD("=== init display %d ==========", display);
    int err = loadDisplayModes();

    if (err != 0) {
        return Error::NO_RESOURCES;
    }

    err = selectConfig();

    if (err != 0) {
        return Error::NO_RESOURCES;
    }

#if DEBUG_FRAMERATE
    mEventContext.version = DRM_EVENT_CONTEXT_VERSION;
    mEventContext.page_flip_handler = page_flip_handler;
#endif //DEBUG_FRAMERATE

    setActiveConfig(mCurrConfig);

    int ret = mVsyncWorker.init(mDrmFd, display, (int)mDrmModes[mCurrConfig].getVRefresh());
    if (ret) {
        ALOGE("Failed to create event worker for d=%d %d", display, ret);
        return Error::BAD_DISPLAY;
    }

    mMaxDevicePlanes = calcMaxDevicePlanes();
    mInitialized = true;
    return Error::NONE;
}

Error HwcDisplay::registerVsyncCallback(hwc2_callback_data_t data,
                                        hwc2_function_pointer_t func) {
    supported(__func__);
    auto callback = std::make_shared<DrmVsyncCallback>(data, func);
    int ret = mVsyncWorker.registerCallback(std::move(callback));

    if (ret) {
        ALOGE("Failed to register callback for display=%d ret=%d", (int)mHandle, ret);
        return Error::BAD_DISPLAY;
    }

    return Error::NONE;
}

Error HwcDisplay::acceptDisplayChanges() {
    supported(__func__);

    for (std::pair<const hwc2_layer_t, HwcLayer>& l : mLayers)
        l.second.acceptTypeChange();

    return Error::NONE;
}

Error HwcDisplay::createLayer(hwc2_layer_t* layer) {
    supported(__func__);
    mLayers.emplace(static_cast<hwc2_layer_t>(mLayerIndex), HwcLayer());
    *layer = static_cast<hwc2_layer_t>(mLayerIndex);
    ++mLayerIndex;
    return Error::NONE;
}

Error HwcDisplay::destroyLayer(hwc2_layer_t layer) {
    supported(__func__);
    mLayers.erase(layer);
    return Error::NONE;
}

void HwcDisplay::startEVSCameraLayer(buffer_handle_t layer) {
    mCameraLayer.setBuffer(layer);

    if (!mUsingCameraLayer) {
        // Setup camera layer's dimensions
        const IMG_native_handle_t* imgHnd2 =
            reinterpret_cast<const IMG_native_handle_t*>(layer);

        if (!imgHnd2) {
            ALOGE("mIonBuffers: ImportBuffer fail");
            return;
        }

        hwc_rect_t display_frame = {0, 0,
                                    imgHnd2->iWidth,
                                    imgHnd2->iHeight
                                   };
        hwc_frect_t source_crop = {0.0f, 0.0f,
                                   imgHnd2->iWidth + 0.0f,
                                   imgHnd2->iHeight + 0.0f
                                  };
        mCameraLayer.setLayerDisplayFrame(display_frame);
        mCameraLayer.setLayerSourceCrop(source_crop);
        mUsingCameraLayer = true;
        //SelectConfig(imgHnd2->iWidth, imgHnd2->iHeight);
        //updateConfig();
    }

    invalidate();
}

void HwcDisplay::stopEVSCameraLayer() {
    invalidate();
    if (mUsingCameraLayer) {
        mUsingCameraLayer = false;
        //SelectConfig();
        //updateConfig();
    }
}

void HwcDisplay::invalidate() {
    int retire_fence = -1;
    presentDisplay(&retire_fence);
    close(retire_fence);
}

void HwcDisplay::getCurrentDisplaySize(uint32_t & inWidth, uint32_t & inHeight) {
    inWidth  = mCurrentDisplayWidth;
    inHeight = mCurrentDisplayHeight;
}

Error HwcDisplay::getActiveConfig(hwc2_config_t* config) {
    supported(__func__);

    if (mCurrConfig == -1)
        return Error::BAD_CONFIG;

    *config = mCurrConfig;
    return Error::NONE;
}

Error HwcDisplay::getChangedCompositionTypes(uint32_t* num_elements,
        hwc2_layer_t* layers, int32_t* types) {
    supported(__func__);
    uint32_t num_changes = 0;

    for (std::pair<const hwc2_layer_t, HwcLayer>& l : mLayers) {
        if (l.second.typeChanged()) {
            if (layers && num_changes < *num_elements)
                layers[num_changes] = l.first;

            if (types && num_changes < *num_elements)
                types[num_changes] =
                    static_cast<int32_t>(l.second.getValidatedType());

            ++num_changes;
        }
    }

    if (!layers && !types)
        *num_elements = num_changes;

    return Error::NONE;
}

Error HwcDisplay::getClientTargetSupport(uint32_t width, uint32_t height,
        int32_t /*format*/, int32_t dataspace) {
    supported(__func__);

    if (width < 1 || height < 1)
        return Error::UNSUPPORTED;

    if (width > 4096 || height > 4096)
        return Error::UNSUPPORTED;

    if (dataspace != HAL_DATASPACE_UNKNOWN
        && dataspace != HAL_DATASPACE_STANDARD_UNSPECIFIED)
        return Error::UNSUPPORTED;

    // TODO: Validate format can be handled by either GL or planes
    return Error::NONE;
}

Error HwcDisplay::getColorModes(uint32_t* num_modes, int32_t* modes) {
    unsupported(__func__, num_modes, modes);
    // TODO: android_color_mode_t isn't defined yet!
    return Error::NONE;
}

Error HwcDisplay::getDisplayAttribute(hwc2_config_t config,
                                      int32_t attribute_in, int32_t* value) {
    supported(__func__);

    if (config >= mDrmModes.size()) {
        ALOGE("Display %d : Could not find active mode for %d", (int)mHandle, config);
        return Error::BAD_CONFIG;
    }

    const DRMMode& mode = mDrmModes[config];
    auto attribute = static_cast<HWC2::Attribute>(attribute_in);
    static const int32_t kUmPerInch = 25400;
    static const int32_t defaultDpi = 38100;

    switch (attribute) {
    case HWC2::Attribute::Width:
        *value = mode.getHDisplay();
        break;

    case HWC2::Attribute::Height:
        *value = mode.getVDisplay();
        break;

    case HWC2::Attribute::VsyncPeriod:
        // in nanoseconds
        if (mode.getFlags() & DRM_MODE_FLAG_INTERLACE)
            *value = 1000 * 1000 * 1000 * 2 / mode.getVRefresh();
        else
            *value = 1000 * 1000 * 1000 / mode.getVRefresh();
        break;

    case HWC2::Attribute::DpiX:
        // Dots per 1000 inches
        *value = mmWidth ? (mode.getHDisplay() * kUmPerInch) / mmWidth : defaultDpi;
        break;

    case HWC2::Attribute::DpiY:
        // Dots per 1000 inches
        *value = mmHeight ? (mode.getVDisplay() * kUmPerInch) / mmHeight : defaultDpi;
        break;

    default:
        *value = -1;
        return Error::BAD_CONFIG;
    }

    return Error::NONE;
}

Error HwcDisplay::getDisplayConfigs(uint32_t* num_configs,
                                    hwc2_config_t* configs) {
    supported(__func__);
    *num_configs = 1;

    //*num_configs = mDrmModes.size();
    if (!configs) {
        return Error::NONE;
    }

    configs[0] = mCurrConfig;
    //for (size_t i = 0; i < mDrmModes.size(); ++i) {
    //    configs[i] = i;
    //}
    return Error::NONE;
}

Error HwcDisplay::getDisplayName(uint32_t* size, char* name) {
    supported(__func__);
    std::ostringstream stream;
    stream << "display-" << mHandle;
    std::string string = stream.str();
    size_t length = string.length();

    if (!name) {
        *size = length;
        return Error::NONE;
    }

    *size = std::min<uint32_t>(static_cast<uint32_t>(length), *size);
    strncpy(name, string.c_str(), *size);
    return Error::NONE;
}

Error HwcDisplay::getDisplayRequests(int32_t* display_requests,
                                     uint32_t* num_elements, hwc2_layer_t* layers, int32_t* layer_requests) {
    supported(__func__);
    // TODO: I think virtual display should request
    //      HWC2_DISPLAY_REQUEST_WRITE_CLIENT_TARGET_TO_OUTPUT here
    unsupported(__func__, display_requests, num_elements, layers, layer_requests);
    *num_elements = 0;
    return Error::NONE;
}

Error HwcDisplay::getDisplayType(int32_t* type) {
    supported(__func__);
    *type = static_cast<int32_t>(mType);
    return Error::NONE;
}

Error HwcDisplay::getDozeSupport(int32_t* support) {
    supported(__func__);
    *support = 0;
    return Error::NONE;
}

Error HwcDisplay::getHdrCapabilities(uint32_t* num_types,
                                     int32_t* /*types*/, float* /*max_luminance*/,
                                     float* /*max_average_luminance*/, float* /*min_luminance*/) {
    supported(__func__);
    *num_types = 0;
    return Error::NONE;
}

Error HwcDisplay::getReleaseFences(uint32_t* num_elements,
                                   hwc2_layer_t* layers, int32_t* fences) {
    supported(__func__);

    if (layers == NULL || fences == NULL) {
        *num_elements = mLayers.size();
        return Error::NONE;
    }

    uint32_t num_layers = 0;

    for (std::pair<const hwc2_layer_t, HwcLayer>& l : mLayers) {
        ++num_layers;

        if (num_layers > *num_elements) {
            ALOGW("Overflow num_elements %d/%d", num_layers, *num_elements);
            return Error::NONE;
        }

        layers[num_layers - 1] = l.first;
        fences[num_layers - 1] = l.second.takeReleaseFence();
    }

    *num_elements = num_layers;
    return Error::NONE;
}

std::vector<HwcLayer*> HwcDisplay::getSortedLayersByZOrder() {
    bool use_client_layer = false;
    uint32_t client_z_order = 0;
    std::vector<HwcLayer*> layers;

    if (!mValidated) {
        mLayersSortedByZ.clear();
        for (auto& l : mLayers) {
            mLayersSortedByZ.emplace(std::make_pair(l.second.getZorder(), &l.second));
        }
    }

    for (auto& l : mLayersSortedByZ) {
        HWC2::Composition validated_type = l.second->getValidatedType();
        if (HWC2::Composition::Device == validated_type || mUsingCameraLayer) {
            layers.push_back(l.second);
        } else if (HWC2::Composition::Client == validated_type) {
            use_client_layer = true;
            client_z_order = std::max(client_z_order, l.second->getZorder());
        } else if (!mValidated) {
            // layer Composition not suported, need to validate
            return std::vector<HwcLayer*>();
        }
    }

    if (use_client_layer) {
        mClientLayer.setLayerZOrder(client_z_order);
        layers.push_back(&mClientLayer);
    }

    if (mUsingCameraLayer) {
        mCameraLayer.setLayerZOrder(client_z_order + 1);
        layers.push_back(&mCameraLayer);
    }

    return layers;
}

int HwcDisplay::applyComposition(std::unique_ptr<DrmDisplayComposition>
                                 composition) {
    int ret = 0;

    switch (composition->getType()) {
    case DRM_COMPOSITION_TYPE_FRAME:
        applyFrame(std::move(composition));
        break;

    case DRM_COMPOSITION_TYPE_MODESET:
        mMode.mMode = mDrmModes[mCurrConfig];//composition->display_mode();

        if (mMode.mBlobId)
            destroyPropertyBlob(mMode.mBlobId);

        mMode.mBlobId = createModeBlob(mMode.mMode);

        if (!mMode.mBlobId) {
            ALOGE("Failed to create mode blob");
            return -ENOMEM;
        }

        mFirstDraw = true;
        return 0;

    default:
        ALOGE("Unknown composition type %d", composition->getType());
        return -EINVAL;
    }

    return ret;
}

int HwcDisplay::applyFrame(std::unique_ptr<DrmDisplayComposition> composition) {
    ATRACE_CALL();

    int ret = 0;
    DrmDisplayComposition* display_comp = composition.get();
    std::vector<DrmHwcLayer>& layers = display_comp->getLayers();
    uint32_t out_fences[mCrtcCount];
    drmModeAtomicReqPtr pset = drmModeAtomicAlloc();

    if (!pset) {
        ALOGE("PresentDisplay| Failed to allocate property set");
        return -ENOMEM;
    }

    if (mCrtcOutFenceProperty.getId()) {
        ret = drmModeAtomicAddProperty(pset, mCrtId,
                                       mCrtcOutFenceProperty.getId(),
                                       (uint64_t)(&out_fences[mCrtcPipe]));

        if (ret < 0) {
            ALOGE("PresentDisplay| Failed to add OUT_FENCE_PTR property to pset: %d", ret);
        }
    }

    uint32_t blob_id = mMode.mBlobId;

    if (mFirstDraw && mCrtcModeProperty.getId() && mConnCrtcProperty.getId()) {
        if (blob_id) {
            ret  = drmModeAtomicAddProperty(pset, mConnectorId,
                                            mConnCrtcProperty.getId(), mCrtId) < 0;
            ret |= drmModeAtomicAddProperty(pset, mCrtId, mCrtcModeProperty.getId(),
                                            blob_id) < 0;
            ret |= drmModeAtomicAddProperty(pset, mCrtId, mCrtcActiveProperty.getId(),
                                            1) < 0;

            if (ret) {
                ALOGE("PresentDisplay| Failed to add blob %d to pset", blob_id);
            }
        } else {
            ALOGE("PresentDisplay| Failed to CreateModeBlob blob_id=0");
        }
    }

    uint32_t j = 0;

    for (uint32_t i = 0; i < layers.size() && (mUsingCameraLayer
            || i < mPlanes.size()); ++i) {
        DrmHwcLayer& layer = layers[i];
        int fb_id = -1;
        fb_id = layer.mBuffer->mFbId;

        if (fb_id <= 0) {
            if (!mUsingCameraLayer)
                ALOGE("ApplyComposition| layer error fb_id <= 0");
        } else {
            DRMPlane& plane = mPlanes[j];
            plane.updateProperties(pset, mCrtId, layer, j);
            ++j;
        }
    }

    uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK;

#if DEBUG_FRAMERATE
    bool isConnectCamera = mUsingCameraLayer;

    if (!mHandle && !isConnectCamera) {
        flags |= DRM_MODE_PAGE_FLIP_EVENT;
    }
#endif //DEBUG_FRAMERATE

    if (mFirstDraw)
        flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;

    ret = -EBUSY;

    while (ret == -EBUSY)
        ret = drmModeAtomicCommit(mDrmFd, pset, flags, this);

    CHECK_RES_WARN(ret);

#if DEBUG_FRAMERATE
    if (!mHandle && !isConnectCamera) {
        CHECK_RES_WARN(drmHandleEvent(mDrmFd, &mEventContext));
    }
#endif //DEBUG_FRAMERATE

    if (mFirstDraw) {
        ret = destroyPropertyBlob(mMode.mOldBlobId);
        mFirstDraw = false;
        mMode.mOldBlobId = mMode.mBlobId;
        mMode.mBlobId = 0;
    }

    if (pset)
        drmModeAtomicFree(pset);

    int32_t cur_out_fence = out_fences[mCrtcPipe];
    if (mCrtcOutFenceProperty.getId() && cur_out_fence > 0) {
        mReleaseFence = cur_out_fence;
    }

    mActiveComposition.swap(composition);
    return 0;
}

Error HwcDisplay::presentDisplay(int32_t* retire_fence) {
    supported(__func__);
    int ret = 0;

    *retire_fence = -1;

    if (mLayers.empty()) {
        return Error::NONE;
    }

    std::vector<HwcLayer*> sorted_layers = getSortedLayersByZOrder();
    mValidated = false; // from here mLayersSortedByZ is not reliable

    if (sorted_layers.empty()) {
        return Error::NOT_VALIDATED;
    }

    std::vector<DrmHwcLayer> layers;

    for (HwcLayer* layer : sorted_layers) {
        DrmHwcLayer drm_layer;
        layer->populateDrmLayer(&drm_layer);
        int ret  = 0;

        if (!mUsingCameraLayer) {
            ret = drm_layer.importBuffer(mImporter.get());
        } else {
            if (layer == &mCameraLayer) {
                ret = drm_layer.importBuffer(mImporter.get());
                drm_layer.isCameraLayer = true;
            } else {
                ret = drm_layer.importBuffer(&mDummyImp);
            }
        }

        if (ret) {
            ALOGE("Failed to import layer, ret=%d", ret);
            continue;
        } else {
            if (layer == &mClientLayer)
                drm_layer.isClient = true;

            layers.emplace_back(std::move(drm_layer));
        }
    }

    if (layers.empty()) {
        return Error::NONE;
    }

    std::unique_ptr<DrmDisplayComposition> composition(new DrmDisplayComposition());
    composition->init(mDrmFd, mCrtId);
    ret = composition->setLayers(layers.data(), layers.size());

    if (ret) {
        ALOGE("Failed to set layers in the composition ret=%d", ret);
        return Error::BAD_LAYER;
    }

    ret = applyComposition(std::move(composition));

    if (ret) {
        ALOGE("Failed to apply the frame composition ret=%d", ret);
        return Error::BAD_PARAMETER;
    }

    *retire_fence = mReleaseFence;
    for (auto& l: mLayers) {
        l.second.setReleaseFence(mReleaseFence);
    }

    return Error::NONE;
}


Error HwcDisplay::setActiveConfig(hwc2_config_t config) {
    supported(__func__);
    mCurrConfig = config;
    const DRMMode& mode = mDrmModes[mCurrConfig];
    std::unique_ptr<DrmDisplayComposition> composition(new DrmDisplayComposition());
    composition->init(mDrmFd, mCrtId);
    int ret = composition->setDisplayMode(mode);
    ret = applyComposition(std::move(composition));

    if (ret) {
        ALOGE("SetActiveConfig: Failed to queue dpms composition on %d", ret);
        return Error::BAD_CONFIG;
    }

    // Setup the client layer's dimensions
    hwc_rect_t display_frame = {0, 0,
                                static_cast<int>(mode.getHDisplay()),
                                static_cast<int>(mode.getVDisplay())
                               };
    mClientLayer.setLayerDisplayFrame(display_frame);
    hwc_frect_t source_crop = {0.0f,
                               0.0f,
                               mode.getHDisplay() + 0.0f,
                               mode.getVDisplay() + 0.0f
                              };
    mClientLayer.setLayerSourceCrop(source_crop);
    //mCameraLayer.SetLayerDisplayFrame(display_frame);
    //mCameraLayer.SetLayerSourceCrop(source_crop);
    return Error::NONE;
}

Error HwcDisplay::setClientTarget(buffer_handle_t target,
                                  int32_t acquire_fence, int32_t dataspace,
                                  /*__attribute__((unused))*/  hwc_region_t /*damage*/) {
    supported(__func__);
    UniqueFd af(acquire_fence);
    mClientLayer.setBuffer(target);
    mClientLayer.setAcquireFence(af.get());
    mClientLayer.setLayerDataspace(dataspace);
    return Error::NONE;
}

Error HwcDisplay::setColorMode(int32_t mode) {
    unsupported(__func__, mode);
    // TODO: android_color_mode_t isn't defined yet!
    return Error::NONE;
}

Error HwcDisplay::setColorTransform(const float* matrix, int32_t hint) {
    unsupported(__func__, matrix, hint);
    // TODO: Force client composition if we get this
    mColorTransform = hint;
    return Error::NONE;
}

Error HwcDisplay::setOutputBuffer(buffer_handle_t buffer,
                                  int32_t release_fence) {
    supported(__func__);
    // TODO: Need virtual display support
    return unsupported(__func__, buffer, release_fence);
}

Error HwcDisplay::setPowerMode(int32_t mode_in) {
    supported(__func__);
    auto mode = static_cast<HWC2::PowerMode>(mode_in);

    switch (mode) {
    case HWC2::PowerMode::Off:
    case HWC2::PowerMode::On:
        break;

    default:
        ALOGI("Power mode %d is unsupported", mode);
        return Error::UNSUPPORTED;
    };

    return Error::NONE;
}

Error HwcDisplay::setVsyncEnabled(int32_t enabled_in) {
    supported(__func__);
    auto enabled = static_cast<HWC2::Vsync>(enabled_in);

    switch (enabled) {
    case HWC2::Vsync::Enable:
        mVsyncWorker.controlVSync(true);
        break;

    case HWC2::Vsync::Disable:
        mVsyncWorker.controlVSync(false);
        break;

    default:
        ALOGE("SetVsyncEnabled called with invalid parameter: %d", enabled_in);
        return Error::BAD_PARAMETER;
    }

    return Error::NONE;
}

int HwcDisplay::calcMaxDevicePlanes() {
    return (mPlanes.size() > 1
            && !(mDrmModes[mCurrConfig].getFlags() & DRM_MODE_FLAG_INTERLACE))
            ? mPlanes.size() - 1 : 0;
}

bool HwcDisplay::layerSupported(HwcLayer* layer, const uint32_t& num_device_planes) {
    if (!layer || mColorTransform)
        return false;

    const IMG_native_handle_t* imgHnd =
            reinterpret_cast<const IMG_native_handle_t*>(layer->getBuffer());

    if (!imgHnd)
        return false;

    const bool formatSupported = mSupportedFormats.find(imgHnd->iFormat) != mSupportedFormats.end();
    return ((num_device_planes < mMaxDevicePlanes)
            && (layer->getSfType() == HWC2::Composition::Device)
            && (layer->getTransform() == HWC2::Transform::None)
            && layer->checkLayer()
            && formatSupported);
}

Error HwcDisplay::validateDisplay(uint32_t* num_types,
                                  uint32_t* num_requests) {
    supported(__func__);
    *num_types = 0;
    *num_requests = 0;
    int num_device_planes = 0;
    mLayersSortedByZ.clear();

    for (auto& l : mLayers) {
        mLayersSortedByZ.emplace(std::make_pair(l.second.getZorder(), &l.second));
    }

    if (mUsingCameraLayer) {
        for (auto& l : mLayersSortedByZ) {
            l.second->setValidatedType (HWC2::Composition::Device);
            ++*num_types;
        }
    } else {
        bool lastDeviceLayer = true;

        for (auto& l : mLayersSortedByZ) {
            HwcLayer* layer = l.second;

            switch (layer->getSfType()) {
            case HWC2::Composition::Device:
                if (layerSupported(layer, num_device_planes) && lastDeviceLayer) {
                    layer->setValidatedType (HWC2::Composition::Device);
                    ++num_device_planes;
                    break;
                }

            [[fallthrough]];
            case HWC2::Composition::SolidColor:
            case HWC2::Composition::Sideband:
                [[fallthrough]];
            case HWC2::Composition::Cursor:
                layer->setValidatedType(HWC2::Composition::Client);
                lastDeviceLayer = false;
                ++*num_types;
                break;

            case HWC2::Composition::Client:
                layer->setValidatedType(HWC2::Composition::Client);
                lastDeviceLayer = false;
                break;

            default:
                layer->setValidatedType(layer->getSfType());
                break;
            }
        }

        // Check possibility of composition all layers on Device
        if (mLayersSortedByZ.size() == mPlanes.size() && mLayersSortedByZ.size() > 0) {
            HwcLayer* layer = mLayersSortedByZ.rbegin()->second;
            int current_planes = num_device_planes - 1;
            if (layerSupported(layer, current_planes)
                    && !lastDeviceLayer && (num_device_planes == (int)mPlanes.size() - 1)) {
                layer->setValidatedType (HWC2::Composition::Device);
                --*num_types;
            }
        }
    }

    mValidated = true;
    return Error::NONE;
}

HwcLayer& HwcDisplay::getLayer(hwc2_layer_t layer) {
    return mLayers.at(layer);
}

void HwcDisplay::updateConfig() {
    //mFirstDraw = true;
    setActiveConfig(mCurrConfig);
}

void HwcDisplay::loadNewConfig() {
    std::deque<DRMPlane>().swap(mPlanes);
    std::vector<DRMMode>().swap(mDrmModes);

    loadDisplayModes();
    selectConfig();
    setActiveConfig(mCurrConfig);

    mMaxDevicePlanes = calcMaxDevicePlanes();
    invalidate();
}

int HwcDisplay::loadDisplayModes() {
    drmModeConnectorPtr connector = nullptr;
    drmModeEncoderPtr encoder = nullptr;
    int len;
    char value[8];
    int fd = -1;
    int display = static_cast<int>(mHandle);

    // Read connector ID from sysfs
    if ((fd = open(mDisplayParams.connector, O_RDONLY)) < 0) {
        ALOGE("Error open '%s', error %d", mDisplayParams.connector, errno);
    } else {
        if ((len = read(fd, &value, sizeof(value))) > 0) {
            value[len] = 0;
            mConnectorId = atoi(value);
        }

        close(fd);
    }

    int ret = 0;
    drmModeResPtr resources = drmModeGetResources(mDrmFd);

    if (!resources) {
        ALOGE("drmModeGetResources error");
        return -1;
    }

    bool modesetIsPossible = true;

    connector = drmModeGetConnector(mDrmFd, mConnectorId);
    if (!connector) {
        ALOGE("drmModeGetConnector %d error", mConnectorId);
        drmModeFreeResources(resources);
        return -1;
    }

    mmWidth = connector->mmWidth;
    mmHeight = connector->mmHeight;

    if (connector->connection == DRM_MODE_DISCONNECTED) {
        ALOGD("disconnected\n");
        modesetIsPossible = false;
    } else if (connector->connection == DRM_MODE_UNKNOWNCONNECTION) {
        ALOGE("unknown connection\n");
        modesetIsPossible = false;
    }

    if (connector->count_encoders == 0) {
        ALOGE("connector has no encoders\n");
        modesetIsPossible = false;
    } else if (modesetIsPossible) {

        mCrtId = 0;

        /* Find an encoder for this connector */
        for (int i = 0; i < connector->count_encoders; i++) {

            encoder = drmModeGetEncoder(mDrmFd, connector->encoders[i]);
            if (!encoder) {
                ALOGD("drmModeGetEncoder id %d error", connector->encoders[i]);
                continue;
            }

            mCrtId = encoder->crtc_id;
            if (mCrtId == 0) {
                /* This encoder is available, find a suiteable crtc */
                ALOGD("Found available encoder (id = %d)", encoder->encoder_id);
                for (int j = 0; j < resources->count_crtcs; ++j) {
                    if (encoder->possible_crtcs & (1 << j)) {
                        ALOGD("crtc_id  %d is compatible with encoder %d", resources->crtcs[j], encoder->encoder_id);
                        mCrtId = resources->crtcs[j];
                        break;
                    }
                }

                if (mCrtId) {
                    /* crtc found, break out the loop */
                    drmModeFreeEncoder(encoder);
                    break;
                }
            }

            /* If we get here it means this encoder is available but
             * no crtc were found. Try with the next encoder.*/
            drmModeFreeEncoder(encoder);
        }

        if (mCrtId == 0) {
            ALOGE("no crtc found\n");
            modesetIsPossible = false;
        }
    }

    if (!modesetIsPossible) {
        ALOGE("no matching connector_type");
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        return -1;
    }

#if defined(TARGET_BOARD_KINGFISHER)
    // on Kingfisher VGA shows as HDMI (there is hw converter), and it's not primary display
    mIsVGAConnectorType = (display != 0) && (connector->connector_type != DRM_MODE_CONNECTOR_LVDS);
#else
    // it's Salvator VGA
    mIsVGAConnectorType = (connector->connector_type == DRM_MODE_CONNECTOR_VGA);
#endif


    for (int i = 0; i < connector->count_modes; i++) {
        DRMMode m(&connector->modes[i]);
        mDrmModes.push_back(m);
    }

    //sorting modes
    if (mDrmModes.size()) {
        std::sort(mDrmModes.begin(), mDrmModes.end(), [](const DRMMode& s1, const DRMMode& s2) {
            int w1 = s1.getHDisplay();
            int w2 = s2.getHDisplay();
            int h1 = s1.getVDisplay();
            int h2 = s2.getVDisplay();

            return ((w1 > w2) || ((w1 == w2) && (h1 > h2)));
        });
    }

    ret = DRMProperty::getCrtcProperty(mDrmFd, mCrtId, "MODE_ID",
                                       &mCrtcModeProperty);

    if (ret) {
        ALOGE("getCrtcProperty: Failed to get MODE_ID property");
        return ret;
    }

    CHECK_RES_WARN(DRMProperty::getCrtcProperty(mDrmFd, mCrtId, "ACTIVE",
                   &mCrtcActiveProperty));
    ret = DRMProperty::getCrtcProperty(mDrmFd, mCrtId, "OUT_FENCE_PTR",
                                       &mCrtcOutFenceProperty);

    if (ret) {
        ALOGE("getCrtcProperty: Failed to get OUT_FENCE_PTR property");
        return ret;
    }

    ret = DRMProperty::getConnectorProperty(mDrmFd, mConnectorId, "CRTC_ID",
                                            &mConnCrtcProperty);

    if (ret) {
        ALOGE("getConnectorProperty: Could not get CRTC_ID property\n");
        return ret;
    }

    ALOGD("LoadDisplayModes for display: %d; mCrtId: %d; mConnectorId: %d; open [%s]"
          , display, mCrtId, mConnectorId, mDisplayParams.connector);
    drmModePlaneResPtr pr = drmModeGetPlaneResources(mDrmFd);

    if (!pr) {
        ALOGE("drmModeGetPlaneResources error");
        return -1;
    }

    int cur_crts_index = -1;

    for (int j = 0; j < resources->count_crtcs; j++) {
        if (resources->crtcs[j] == mCrtId)
            cur_crts_index = j;
    }

    mCrtcCount = resources->count_crtcs;
    mCrtcPipe = cur_crts_index;

    for (uint32_t i = 0; i < pr->count_planes; ++i) {
        uint32_t plane_obj_id = pr->planes[i];
        drmModePlanePtr plane_ptr = drmModeGetPlane(mDrmFd, plane_obj_id);

        if (!plane_ptr) {
            ALOGE("PLANE ERROR: failed to get plane %d", i);
            continue;
        }

        if (plane_ptr->possible_crtcs & (1 << cur_crts_index)) {
            drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(mDrmFd,
                                               plane_obj_id, DRM_MODE_OBJECT_PLANE);

            if (!props) {
                ALOGE("PLANE ERROR: failed to get plane properties");
                continue;
            }

            DRMPlane drm_plane(mDrmFd, plane_ptr);
            drm_plane.init();

            if (drm_plane.getType() == DRM_PLANE_TYPE_OVERLAY) {
                mPlanes.push_back(std::move(drm_plane));
            } else if (drm_plane.getType() == DRM_PLANE_TYPE_PRIMARY) {
                mPlanes.push_front(std::move(drm_plane));
            }

            drmModeFreeObjectProperties(props);
        }
    }

    ALOGD("PLANES num: %zu", mPlanes.size());

    if (pr)
        drmModeFreePlaneResources(pr);

    if (resources)
        drmModeFreeResources(resources);

    return 0;
}

bool HwcDisplay::getResolutionFromProperties(uint32_t& width, uint32_t& height,
        uint32_t& refresh,
        bool& interlace) {
    char value[PROPERTY_VALUE_MAX];
    property_get(mDisplayParams.property, value, "");
    // Property has next format:
    // [<width>x<height>][-<bpp>][@<refresh rate>][i]
    std::string s(value);
    size_t found = s.find("x");

    if (found == std::string::npos)
        return false;

    std::string s_width = s.substr(0, found);
    s = s.substr(found + 1); // 1 - "x" letter
    found = s.find("-");

    if (found == std::string::npos)
        return false;

    std::string s_height = s.substr(0, found);
    s = s.substr(found + 1); // 1 - "-" letter
    found = s.find("@");

    if (found == std::string::npos)
        return false;

    std::string s_bpp = s.substr(0, found);
    s = s.substr(found + 1); // 1 - "@" letter
    std::string s_refresh;

    if (s.back() == 'i') {
        interlace = true;
        s_refresh = s.substr(0, s.length() - 1); // 1 - "i" letter
    } else {
        s_refresh = s;
    }

    width = std::stoi(s_width);
    height = std::stoi(s_height);
    refresh = std::stoi(s_refresh);
    return true;
}

int HwcDisplay::selectConfig() {
    int i = 0;

    for (const DRMMode& mode : mDrmModes) {
        ALOGD("mode %d name = %s vrefresh = %f", i, mode.getName().c_str(),
              mode.getVRefresh());
        i++;
    }

    float preferred_aspect_ratio = 0;

    if (!mDrmModes.empty()) {
        const DRMMode& mode = mDrmModes[0];
        preferred_aspect_ratio = mode.getHDisplay() / (float) mode.getVDisplay();
    }

    for (const DRMMode& mode : mDrmModes) {
        if (mode.getType() & DRM_MODE_TYPE_PREFERRED) {
            preferred_aspect_ratio = mode.getHDisplay()
                                     / (float) mode.getVDisplay();
            break;
        }
    }

    uint32_t width = 0, height = 0, HZ = 60;
    bool isInterlace = false;
    bool isBootargsPresent = getResolutionFromProperties(width, height, HZ,
                             isInterlace);
    isInterlace = isInterlace && isBootargsPresent;
    i = 0;

    for (const DRMMode& mode : mDrmModes) {
        i++;

        if (!isBootargsPresent && mIsVGAConnectorType // if no bootargs and it's VGA
#if defined(TARGET_BOARD_KINGFISHER)
                && !(mode.getFlags() & DRM_MODE_FLAG_INTERLACE) // for KF default is 1080i
#endif // TARGET_BOARD_KINGFISHER
                && (mode.getHDisplay() > 1280 || mode.getVDisplay() > 720)) {
            // VGA connector mIsVGAConnectorType use 720p as default on Salvator
            continue;
        }

        float aspect = mode.getHDisplay() / (float) mode.getVDisplay();

        if (!isBootargsPresent && !mIsVGAConnectorType
                && (fabs(aspect - preferred_aspect_ratio) > 0.001)) {
            // use only prefered aspect ratio, except VGA
            continue;
        }

        if (!isBootargsPresent && ((mode.getHDisplay() > 1920) || (mode.getVDisplay() > 1080))) {
            // Now we don't support greater than 1080p resolution
            continue;
        }

        if (isBootargsPresent
                && (mode.getHDisplay() != width || mode.getVDisplay() != height)) {
            // display size is not meet request value from bootargs
            continue;
        }

        if (fabs(mode.getVRefresh() - HZ) > 3) { // there are cases where refresh not strictly 60 hz
            // refresh rate doesn't meet request value
            continue;
        }

        if (isBootargsPresent   // if bootargs with interlace - skip all not interlace, and vice versa
                && (isInterlace != !!(mode.getFlags() & DRM_MODE_FLAG_INTERLACE))) {
            continue;
        }

        if (!isBootargsPresent          // if no bootargs
                && !!(mode.getFlags() & DRM_MODE_FLAG_INTERLACE) // skip all interlace
#if defined(TARGET_BOARD_KINGFISHER)
                && !mIsVGAConnectorType // except VGA on Kingfisher
#endif // TARGET_BOARD_KINGFISHER
                ) {
            continue;
        }

        mCurrConfig = i - 1;
        ALOGD("selected mode %d name = %s vrefresh = %f", i - 1,
              mode.getName().c_str(), mode.getVRefresh());
        mCurrentDisplayHeight = mode.getVDisplay();
        mCurrentDisplayWidth  = mode.getHDisplay();
        break;
    }

    if (mCurrConfig == -1) {
        ALOGE("Unable to select config");
    }

    return (mCurrConfig == -1) ? -1 : 0;
}

int HwcDisplay::selectConfig(uint32_t width, uint32_t height, uint32_t HZ) {
    int i = 0;
    int config = -1;
    ALOGD("Select config mode needed: %d %dx%d", HZ, width, height);

    for (const DRMMode& mode : mDrmModes) {
        ALOGD("mode %d name = %s vrefresh = %f", i, mode.getName().c_str(),
              mode.getVRefresh());
        i++;
    }

    i = 0;

    for (const DRMMode& mode : mDrmModes) {
        i++;

        if (mode.getHDisplay() != width || mode.getVDisplay() != height) {
            // display size does not meet request value
            continue;
        }

        if (fabs(mode.getVRefresh() - HZ) > 0.001) {
            continue;
        }

        config = i - 1;
        ALOGD("custom selected mode %d name = %s vrefresh = %f", config,
              mode.getName().c_str(), mode.getVRefresh());
        break;
    }

    if (config == -1) {
        ALOGE("Unable to select config");
        return -1;
    }

    mCurrConfig = config;
    return (mCurrConfig == -1) ? -1 : 0;
}

uint32_t HwcDisplay::createModeBlob(const DRMMode& mode) {
    drmModeModeInfo drm_mode;
    memset(&drm_mode, 0, sizeof(drm_mode));
    mode.toDrmModeModeInfo(&drm_mode);
    uint32_t blob_id = 0;
    int ret = drmModeCreatePropertyBlob(mDrmFd, &drm_mode, sizeof(drm_mode),
                                        &blob_id);

    if (ret) {
        ALOGE("CreateModeBlob Failed to create mode property blob %d", ret);
        return 0;
    }

    return blob_id;
}

int HwcDisplay::destroyPropertyBlob(uint32_t blob_id) {
    if (!blob_id)
        return 0;

    int ret = drmModeDestroyPropertyBlob(mDrmFd, blob_id);

    if (ret) {
        ALOGE("DestroyPropertyBlob Failed to destroy mode property blob: %d; ret:%d",
              blob_id, ret);
        return ret;
    }

    return 0;
}

} // namespace android
