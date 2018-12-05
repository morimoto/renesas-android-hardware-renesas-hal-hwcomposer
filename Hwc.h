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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_H

#include "ComposerBase.h"
#include "HwcDisplay.h"
#include "HwcLayer.h"
#include "vsyncworker.h"
#include "drm/DRMMode.h"

#include <vendor/renesas/graphics/composer/1.0/IComposer.h>
#include <hardware/hwcomposer2.h>
#include <log/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <memory>
#include <mutex>
#include <unordered_set>
#include <map>
#include <vector>

namespace android {
class HWC2On1Adapter;
}

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_1 {
namespace implementation {

using android::hardware::graphics::common::V1_0::PixelFormat;
using android::hardware::graphics::common::V1_0::Transform;
using android::hardware::graphics::common::V1_0::Dataspace;
using android::hardware::graphics::common::V1_0::ColorMode;
using android::hardware::graphics::common::V1_0::ColorTransform;
using android::hardware::graphics::common::V1_0::Hdr;
using vendor::renesas::graphics::composer::V1_0::IComposer;

class ComposerClient;

class HwcHal : public IComposer, public ComposerBase {
public:  // functions
    HwcHal();
    virtual ~HwcHal();

    bool hasCapability(Capability capability) const;

    // IComposer interface
    Return<void> getCapabilities(getCapabilities_cb hidl_cb) override;
    Return<void> dumpDebugInfo(dumpDebugInfo_cb hidl_cb) override;
    Return<void> createClient(createClient_cb hidl_cb) override;
    // Methods from IComposer follow.
    Return<::android::hardware::graphics::composer::V2_1::Error> setEVSCameraData(
        const hidl_handle& buffer, int8_t currDisplay) override;
    Return<uint32_t> getDisplayHeight() override;
    Return<uint32_t> getDisplayWidth()  override;

    // ComposerBase interface
    void removeClient() override;
    void enableCallback(bool enable) override;
    uint32_t getMaxVirtualDisplayCount() override;
    Error createVirtualDisplay(
        uint32_t width, uint32_t height, PixelFormat* format,
        Display* outDisplay) override;
    Error destroyVirtualDisplay(Display display) override;

    Error createLayer(Display display, Layer* outLayer) override;
    Error destroyLayer(Display display, Layer layer) override;

    Error getActiveConfig(Display display, Config* outConfig) override;
    Error getClientTargetSupport(
        Display display,
        uint32_t width,
        uint32_t height,
        PixelFormat format,
        Dataspace dataspace) override;
    Error getColorModes(Display display, hidl_vec<ColorMode>* outModes) override;
    Error getDisplayAttribute(
        Display display,
        Config config,
        IComposerClient::Attribute attribute,
        int32_t* outValue) override;
    Error getDisplayConfigs(Display display, hidl_vec<Config>* outConfigs) override;
    Error getDisplayName(Display display, hidl_string* outName) override;
    Error getDisplayType(Display display,
                         IComposerClient::DisplayType* outType) override;
    Error getDozeSupport(Display display, bool* outSupport) override;
    Error getHdrCapabilities(
        Display display,
        hidl_vec<Hdr>* outTypes,
        float* outMaxLuminance,
        float* outMaxAverageLuminance,
        float* outMinLuminance) override;

    Error setActiveConfig(Display display, Config config) override;
    Error setColorMode(Display display, ColorMode mode) override;
    Error setPowerMode(Display display, IComposerClient::PowerMode mode) override;
    Error setVsyncEnabled(Display display, IComposerClient::Vsync enabled) override;

    Error setColorTransform(Display display, const float* matrix,
                            int32_t hint) override;
    Error setClientTarget(
        Display display,
        buffer_handle_t target,
        int32_t acquireFence,
        int32_t dataspace,
        const std::vector<hwc_rect_t>& damage) override;
    Error setOutputBuffer(Display display, buffer_handle_t buffer,
                          int32_t releaseFence) override;
    Error validateDisplay(
        Display display,
        std::vector<Layer>* outChangedLayers,
        std::vector<IComposerClient::Composition>* outCompositionTypes,
        uint32_t* outDisplayRequestMask,
        std::vector<Layer>* outRequestedLayers,
        std::vector<uint32_t>* outRequestMasks) override;
    Error acceptDisplayChanges(Display display) override;
    Error presentDisplay(
        Display display,
        int32_t* outPresentFence,
        std::vector<Layer>* outLayers,
        std::vector<int32_t>* outReleaseFences) override;

    Error setLayerCursorPosition(Display display, Layer layer, int32_t x,
                                 int32_t y) override;
    Error setLayerBuffer(
        Display display, Layer layer, buffer_handle_t buffer,
        int32_t acquireFence) override;
    Error setLayerSurfaceDamage(
        Display display, Layer layer, const std::vector<hwc_rect_t>& damage) override;
    Error setLayerBlendMode(Display display, Layer layer, int32_t mode) override;
    Error setLayerColor(Display display, Layer layer,
                        IComposerClient::Color color) override;
    Error setLayerCompositionType(Display display, Layer layer,
                                  int32_t type) override;
    Error setLayerDataspace(Display display, Layer layer,
                            int32_t dataspace) override;
    Error setLayerDisplayFrame(Display display, Layer layer,
                               const hwc_rect_t& frame) override;
    Error setLayerPlaneAlpha(Display display, Layer layer, float alpha) override;
    Error setLayerSidebandStream(Display display, Layer layer,
                                 buffer_handle_t stream) override;
    Error setLayerSourceCrop(Display display, Layer layer,
                             const hwc_frect_t& crop) override;
    Error setLayerTransform(Display display, Layer layer,
                            int32_t transform) override;
    Error setLayerVisibleRegion(
        Display display, Layer layer, const std::vector<hwc_rect_t>& visible) override;
    Error setLayerZOrder(Display display, Layer layer, uint32_t z) override;

private:  // types
    struct HwcCallback {
        HwcCallback(hwc2_callback_data_t d, hwc2_function_pointer_t f)
            : data(d)
            , func(f) {
        }
        hwc2_callback_data_t data;
        hwc2_function_pointer_t func;
    };
    struct DisplayInfo {
        bool isConnected;
        hwc2_display_t displayType;
    } mConnectDisplays[NUM_DISPLAYS];

private:  // functions
    void RegisterCallback(
        HWC2::Callback descriptor, hwc2_callback_data_t data,
        hwc2_function_pointer_t function);

    void initCapabilities();
    void initDisplays();
    void hookEventHotPlug();

    sp<ComposerClient> getClient();
    static void hotplugHook(
        hwc2_callback_data_t callbackData, hwc2_display_t display, int32_t connected);
    static void refreshHook(hwc2_callback_data_t callbackData,
                            hwc2_display_t display);
    static void vsyncHook(
        hwc2_callback_data_t callbackData, hwc2_display_t display, int64_t timestamp);
    inline HwcDisplay& getDisplay(hwc2_display_t display);

    inline HwcLayer& getLayer(hwc2_display_t display, hwc2_layer_t layer);

private:  // members
    int mDrmFd;
    std::shared_ptr<Importer> mImporter;

    std::unordered_set<Capability> mCapabilities;

    std::mutex mClientMutex;
    std::atomic<bool> mInitDisplay;
    wp<ComposerClient> mClient;

    std::map<hwc2_display_t, HwcDisplay> mDisplays;
    std::map<HWC2::Callback, HwcCallback> mCallbacks;

    hidl_handle mCameraHidlHandle;
    bool mIsHotplugInitialized;

    uint32_t mDisplayHeight;
    uint32_t mDisplayWidth;
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace composer
}  // namespace graphics
}  // namespace hardware

template <typename... Args>
static inline android::hardware::graphics::composer::V2_1::Error unsupported(
    char const* /*func*/, Args... /*args*/) {
    //ALOGV("Unsupported function: %s", func);
    return android::hardware::graphics::composer::V2_1::Error::UNSUPPORTED;
}

static inline void supported(char const* /*func*/) {
    //ALOGV("supported function: %s", func);
}

} // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_H
