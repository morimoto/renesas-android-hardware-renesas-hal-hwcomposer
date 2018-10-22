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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_DISPLAY_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_DISPLAY_H

#include "HwcLayer.h"
#include "DrmDisplayComposition.h"
#include "config.h"
#include "platform.h"
#include "vsyncworker.h"
#include "drm/DRMMode.h"
#include "drm/DRMPlane.h"

#include <hardware/hardware.h>
#include <hardware/hwcomposer2.h>

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <deque>

/* DEBUG_FRAMERATE
 * report frame rate every second.
 *  1   effective.
 *  0   unavailable.
 */
#define DEBUG_FRAMERATE 1 // report frame rate if 1 specified

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
    Error presentDisplay(int32_t* retire_fence);
    Error setActiveConfig(hwc2_config_t config);
    Error setClientTarget(buffer_handle_t target, int32_t acquire_fence,
                          int32_t dataspace, hwc_region_t damage);
    Error setColorMode(int32_t mode);
    Error setColorTransform(const float* matrix, int32_t hint);
    Error setOutputBuffer(buffer_handle_t buffer, int32_t release_fence);
    Error setPowerMode(int32_t mode);
    Error setVsyncEnabled(int32_t enabled);
    Error validateDisplay(uint32_t* num_types, uint32_t* num_requests);
    HwcLayer& getLayer(hwc2_layer_t layer);

    void updateConfig();

    void startEVSCameraLayer(buffer_handle_t layer);
    void stopEVSCameraLayer();
    void invalidate();

#if DEBUG_FRAMERATE
    int mFpsFrameCount = 0;
    uint32_t mFpsStartSec = 0;
    uint32_t mFpsStartUsec = 0;
    std::string mDispName;
#endif //DEBUG_FRAMERATE

private:
    void addFenceToRetireFence(int fd);
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

    DRMProperty mCrtcModeProperty;
    DRMProperty mCrtcActiveProperty;
    DRMProperty mCrtcOutFenceProperty;
    DRMProperty mConnCrtcProperty;
    int mCrtcCount;
    int mCrtcPipe;

    UniqueFd mRetireFence = -1;
    UniqueFd mNextRetireFence = -1;

    std::unique_ptr<DrmDisplayComposition> mActiveComposition;

#if DEBUG_FRAMERATE
    drmEventContext mEventContext;
#endif //DEBUG_FRAMERATE
    VSyncWorker mVsyncWorker;
    hwc2_display_t mHandle;
    HWC2::DisplayType mType;
    uint32_t mLayerIndex = 0;
    std::map<hwc2_layer_t, HwcLayer> mLayers;
    std::map<int, HwcLayer*> mLayersSortedByZ;
    HwcLayer mClientLayer;
    HwcLayer mCameraLayer;
    bool mFirstDraw = false;
    uint32_t mCrtId;
    hwdisplay mDisplayParams;

    std::shared_ptr<Importer> mImporter;
    DummyImporter mDummyImp;
    std::deque<DRMPlane> mPlanes;
    std::mutex mLock;
    int32_t mColorTransform = 0;
    bool mIsVGAConnectorType = false;
    bool mInitialized = false;
    bool mUsingCameraLayer = false;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_DISPLAY_H