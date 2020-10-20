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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_HWC_DISPLAY_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_HWC_DISPLAY_H

#include "config.h"
#include "platform.h"
#include "vsyncworker.h"
#include "img_gralloc1_public.h"

#include <memory>
#include <deque>
#include <unordered_set>

namespace android {

class HwcDisplay {
    using Error = android::hardware::graphics::composer::V2_1::Error;
public:
    HwcDisplay(int drmFd, hwc2_display_t handle, hwdisplay params,
               HWC2::DisplayType type, std::shared_ptr<Importer> importer);

    HwcDisplay(const HwcDisplay&) = delete;
    Error init();

    Error registerVsyncCallback(hwc2_callback_data_t data,
                                hwc2_function_pointer_t func);
    Error registerVsyncCallback_2_4(hwc2_callback_data_t data,
                                    hwc2_function_pointer_t func);

    // HWC Hooks
    Error acceptDisplayChanges();
    Error createLayer(hwc2_layer_t* layer);
    Error destroyLayer(hwc2_layer_t layer);
    Error getActiveConfig(hwc2_config_t* config);
    Error getChangedCompositionTypes(uint32_t* num_elements,
                                     hwc2_layer_t* layers, int32_t* types);
    Error getClientTargetSupport(uint32_t width, uint32_t height,
                                 int32_t format, int32_t dataspace);
    Error getColorModes(uint32_t* num_modes, int32_t* modes);
    Error getDisplayAttribute(hwc2_config_t config, int32_t attribute,
                              int32_t* value);
    Error getDisplayConfigs(uint32_t* num_configs,
                            hwc2_config_t* configs);
    Error getDisplayName(uint32_t* size, char* name);
    Error getDisplayRequests(int32_t* display_requests,
                             uint32_t* num_elements, hwc2_layer_t* layers,
                             int32_t* layer_requests);
    Error getDisplayType(int32_t* type);
    Error getDozeSupport(int32_t* support);
    Error getHdrCapabilities(uint32_t* num_types, int32_t* types,
                             float* max_luminance, float* max_average_luminance,
                             float* min_luminance);
    Error getReleaseFences(uint32_t* num_elements, hwc2_layer_t* layers,
                           int32_t* fences);
    Error compositionLayers(std::vector<DrmHwcLayer> & layers);
    Error presentDisplay(int32_t* retire_fence);
    Error setActiveConfig(hwc2_config_t config);
    Error setClientTarget(buffer_handle_t target, int32_t acquire_fence,
                          int32_t dataspace, hwc_region_t damage);
    Error setColorMode(int32_t mode, int32_t intent = 0 /* COLORIMETRIC */);
    Error setColorTransform(const float* matrix, int32_t hint);
    Error setOutputBuffer(buffer_handle_t buffer, int32_t release_fence);
    Error setPowerMode(int32_t mode);
    Error setVsyncEnabled(int32_t enabled);
    Error validateDisplay(uint32_t* num_types, uint32_t* num_requests);
    HwcLayer& getLayer(hwc2_layer_t layer);

    uint32_t getCrtId() const;

    void updateConfig();
    void loadNewConfig();
    void getCurrentDisplaySize(uint32_t & inWidth, uint32_t & inHeight);
    void hwcDisplayPoll(int32_t fd, int32_t timeout = 100) const;
    void syncFence(const hwc2_display_t handle);
    void evsStartCameraLayer(buffer_handle_t layer);
    void evsStopCameraLayer();
    hwc2_display_t getDisplayHandle() const noexcept;
    Error evsPresentDisplay();

    uint32_t getConnectorId() const;
    std::string getDisplayInfo() const;
    std::string getHardwareDisplayType() const;

    void cmsReset();
    void cmsSetLut(const hardware::hidl_vec<uint32_t>& buff);
    void cmsSetClu(const hardware::hidl_vec<uint32_t>& buff);
    void cmsGetHgo(uint32_t* buff, uint32_t size);

private:
    int loadDisplayModes();
    int selectConfig();
    int selectConfig(uint32_t width, uint32_t height, uint32_t HZ = 60);
    bool getResolutionFromProperties(uint32_t& width, uint32_t& height,
                                     uint32_t& refresh,
                                     bool& interlace);
    std::vector<HwcLayer*> getSortedLayersByZOrder();
    int applyComposition(std::unique_ptr<DrmDisplayComposition> composition);
    int applyFrame(std::unique_ptr<DrmDisplayComposition> composition);

    uint32_t createModeBlob(const DRMMode& mode);
    int destroyPropertyBlob(uint32_t blob_id);
    int calcMaxDevicePlanes();
    bool layerSupported(HwcLayer* layer, const uint32_t &num_device_planes);

    std::vector<DRMMode> mDrmModes;
    struct ModeState {
        DRMMode mMode;
        uint32_t mBlobId = 0;
        uint32_t mOldBlobId = 0;
    };
    ModeState mMode;

    int mDrmFd;
    int mCurrConfig;
    uint32_t mConnectorId;
    uint32_t mmWidth = 0;
    uint32_t mmHeight = 0;

    uint32_t mCurrentDisplayHeight = 0;
    uint32_t mCurrentDisplayWidth = 0;
    float mCurrentDisplayVRefresh = 0.0f;

    DRMProperty mCrtcModeProperty;
    DRMProperty mCrtcActiveProperty;
    DRMProperty mCrtcOutFenceProperty;
    DRMProperty mConnCrtcProperty;
    int mCrtcCount;
    int mCrtcPipe;

    int32_t mReleaseFence = -1;

    std::unique_ptr<DrmDisplayComposition> mActiveComposition;

    VSyncWorker mVsyncWorker;
    hwc2_display_t mHandle;
    HWC2::DisplayType mType;
    uint32_t mLayerIndex = 2;
    std::map<hwc2_layer_t, HwcLayer*> mLayers;
    std::map<int, HwcLayer*> mLayersSortedByZ;
    HwcLayer mClientLayer;
    HwcLayer mCameraLayer;
    bool mFirstDraw = false;
    uint32_t mCrtId;
    hwdisplay mDisplayParams;

    std::mutex mImporterLock;
    std::shared_ptr<Importer> mImporter;
    std::deque<DRMPlane> mPlanes;

    const std::unordered_set<int> mSupportedFormats = {
        HAL_PIXEL_FORMAT_BGRX_8888,
        HAL_PIXEL_FORMAT_BGRA_8888,
        HAL_PIXEL_FORMAT_RGB_888,
        HAL_PIXEL_FORMAT_RGB_565,
        HAL_PIXEL_FORMAT_UYVY,
        HAL_PIXEL_FORMAT_NV12,
        HAL_PIXEL_FORMAT_NV12_CUSTOM,
        HAL_PIXEL_FORMAT_NV21,
        HAL_PIXEL_FORMAT_NV21_CUSTOM
    };

    int32_t mColorTransform = 0;
    uint32_t mMaxDevicePlanes = 0;
    bool mIsVGAConnectorType = false;
    bool mInitialized = false;
    bool mUsingCameraLayer = false;
    bool mValidated = false;
    bool mIsInterlaceMode = false;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_HWC_DISPLAY_H
