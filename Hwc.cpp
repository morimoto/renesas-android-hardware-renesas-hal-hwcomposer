/*
 * Copyright 2016 The Android Open Source Project
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

#include "Hwc.h"
#include "ComposerClient.h"
#include "HwcHotPlug.h"

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_3 {
namespace implementation {

HwcHal::HwcHal()
    : mDrmFd(-1)
    , mIsHotplugInitialized(false)
    , mInitDisplays(false) {
    mDrmFd = drmOpen("rcar-du", NULL);

    if (mDrmFd < 0) {
        ALOGE("drmOpen error");
    }

#if HWC_PRIME_CACHE
    PrimeCache::getInstance().setDrmFd(mDrmFd);
    ALOGD("PRIME_CACHE enabled");
#endif

    drmSetMaster(mDrmFd);
    CHECK_RES_WARN(drmSetClientCap(mDrmFd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1));
    CHECK_RES_WARN(drmSetClientCap(mDrmFd, DRM_CLIENT_CAP_ATOMIC, 1));
    dump_all(mDrmFd);
    mImporter.reset(Importer::createInstance(mDrmFd));

    if (!mImporter) {
        ALOGE("Failed to create importer instance");
        return;// HWC2::Error::NoResources;
    }

    initDisplays();
    initCapabilities();
}

HwcHal::~HwcHal() {
    drmClose(mDrmFd);
}

void HwcHal::initCapabilities() {
    supported(__func__);
}

void HwcHal::initDisplays() {
    mDisplays.clear();
    ALOGD("initDisplays. displays count: %d.", NUM_DISPLAYS);

    HotPlug& hotPlug = HotPlug::getInstance();
    hotPlug.initDisplays(this);

    mDisplays.at(HWC_DISPLAY_PRIMARY).getCurrentDisplaySize(mDisplayWidth, mDisplayHeight);
}

Return<uint32_t> HwcHal::getDisplayHeight()  {
    return mDisplayHeight;
}

Return<uint32_t> HwcHal::getDisplayWidth() {
    return mDisplayWidth;
}

bool HwcHal::isDisplayValid(hwc2_display_t display) const {
        return display < mDisplays.size();
}

bool HwcHal::hasCapability(Capability capability) const {
    return (mCapabilities.count(capability) > 0);
}

Return<void> HwcHal::getCapabilities(getCapabilities_cb hidl_cb) {
    std::vector<Capability> caps(mCapabilities.cbegin(), mCapabilities.cend());
    hidl_vec<Capability> caps_reply;
    caps_reply.setToExternal(caps.data(), caps.size());
    hidl_cb(caps_reply);
    return Void();
}

Return<void> HwcHal::dumpDebugInfo(dumpDebugInfo_cb hidl_cb) {
    //unsupported(__func__);
    //    uint32_t len = 0;
    //    mHwComposer2.Dump(&len, nullptr);
    //    std::vector<char> buf(len + 1);
    //    mHwComposer2.Dump(&len, buf.data());
    //    buf.resize(len + 1);
    //    buf[len] = '\0';
    //    hidl_string buf_reply;
    //    buf_reply.setToExternal(buf.data(), len);
    //    hidl_cb(buf_reply);
    hidl_cb("This is a dump from HwcHal.\n");
    return Void();
}

Return<void> HwcHal::createClient(createClient_cb hidl_cb) {
    Error err = Error::NONE;
    sp<ComposerClient> client;
    {
        std::lock_guard<std::mutex> lock(mClientMutex);

        // only one client is allowed
        if (mClient == nullptr) {
            client = new ComposerClient(*this);
            client->initialize();
            mClient = client;
        } else {
            err = Error::NO_RESOURCES;
        }
    }
    hidl_cb(err, client);
    return Void();
}

// Duplicate of HwcHal::createClient
Return<void> HwcHal::createClient_2_3(createClient_2_3_cb _hidl_cb) {
    Error err = Error::NONE;
    sp<ComposerClient> client;
    {
        std::lock_guard<std::mutex> lock(mClientMutex);

        // only one client is allowed
        if (mClient == nullptr) {
            client = new ComposerClient(*this);
            client->initialize();
            mClient = client;
        } else {
            err = Error::NO_RESOURCES;
        }
    }
    _hidl_cb(err, client);
    return Void();
};

sp<ComposerClient> HwcHal::getClient() {
    std::lock_guard<std::mutex> lock(mClientMutex);
    return (mClient != nullptr) ? mClient.promote() : nullptr;
}

void HwcHal::removeClient() {
    std::lock_guard<std::mutex> lock(mClientMutex);
    mClient = nullptr;
}

void HwcHal::hotplugHook(
    hwc2_callback_data_t callbackData, hwc2_display_t display, int32_t connected) {
    auto hal = reinterpret_cast<HwcHal*>(callbackData);
    auto client = hal->getClient();

    if (client != nullptr) {
        client->onHotplug(display,
                          static_cast<IComposerCallback::Connection>(connected));
    }
}

void HwcHal::refreshHook(hwc2_callback_data_t callbackData,
                         hwc2_display_t display) {
    auto hal = reinterpret_cast<HwcHal*>(callbackData);
    auto client = hal->getClient();

    if (client != nullptr) {
        client->onRefresh(display);
    }
}

void HwcHal::vsyncHook(hwc2_callback_data_t callbackData,
                       hwc2_display_t display, int64_t timestamp) {
    auto hal = reinterpret_cast<HwcHal*>(callbackData);
    auto client = hal->getClient();

    if (client != nullptr) {
        client->onVsync(display, timestamp);
    }
}

HwcDisplay& HwcHal::getDisplay(hwc2_display_t display) {
    return mDisplays.at(display);
}

HwcLayer& HwcHal::getLayer(hwc2_display_t display, hwc2_layer_t layer) {
    return mDisplays.at(display).getLayer(layer);
}

void HwcHal::enableCallback(bool enable) {
    if (enable) {
        RegisterCallback(
            HWC2::Callback::Hotplug, this,
            reinterpret_cast<hwc2_function_pointer_t>(hotplugHook));
        RegisterCallback(
            HWC2::Callback::Refresh, this,
            reinterpret_cast<hwc2_function_pointer_t>(refreshHook));
        RegisterCallback(
            HWC2::Callback::Vsync, this,
            reinterpret_cast<hwc2_function_pointer_t>(vsyncHook));
    } else {
        RegisterCallback(HWC2::Callback::Hotplug, this, nullptr);
        RegisterCallback(HWC2::Callback::Refresh, this, nullptr);
        RegisterCallback(HWC2::Callback::Vsync, this, nullptr);
    }
}

uint32_t HwcHal::getMaxVirtualDisplayCount() {
    // TODO: Implement virtual display
    unsupported(__func__);
    return 0;
}

Error HwcHal::createVirtualDisplay(
    uint32_t width, uint32_t height, PixelFormat* format, Display* outDisplay) {
    // TODO: Implement virtual display
    return unsupported(__func__, width, height, format, outDisplay);
}

Error HwcHal::destroyVirtualDisplay(Display display) {
    return unsupported(__func__, display);
}

Error HwcHal::createLayer(Display display, Layer* outLayer) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    return getDisplay(display).createLayer(outLayer);
}

Error HwcHal::destroyLayer(Display display, Layer layer) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    return getDisplay(display).destroyLayer(layer);
}

Error HwcHal::getActiveConfig(Display display, Config* outConfig) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    return getDisplay(display).getActiveConfig(outConfig);
}

Error HwcHal::getClientTargetSupport(
    Display display, uint32_t width, uint32_t height, PixelFormat format,
    Dataspace dataspace) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    return getDisplay(display).getClientTargetSupport(
               width, height, static_cast<int32_t>(format), static_cast<int32_t>(dataspace));
}

Error HwcHal::getColorModes(Display display, hidl_vec<ColorMode>* outModes) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    uint32_t count = 0;
    auto err = getDisplay(display).getColorModes(&count, nullptr);

    if (err != Error::NONE) {
        return err;
    }

    outModes->resize(count);
    err = getDisplay(display).getColorModes(
              &count, reinterpret_cast<std::underlying_type<ColorMode>::type*>
              (outModes->data()));

    if (err != Error::NONE) {
        *outModes = hidl_vec<ColorMode>();
        return err;
    }

    return Error::NONE;
}

Error HwcHal::getDisplayAttribute(
    Display display, Config config, IComposerClient::Attribute attribute,
    int32_t* outValue) {
    return getDisplay(display).getDisplayAttribute(
               config, static_cast<int32_t>(attribute), outValue);
}

Error HwcHal::getDisplayConfigs(Display display, hidl_vec<Config>* outConfigs) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    uint32_t count = 0;
    auto err = getDisplay(display).getDisplayConfigs(&count, nullptr);

    if (err != Error::NONE) {
        return err;
    }

    outConfigs->resize(count);
    err = getDisplay(display).getDisplayConfigs(&count, outConfigs->data());

    if (err != Error::NONE) {
        *outConfigs = hidl_vec<Config>();
        return err;
    }

    return Error::NONE;
}

Error HwcHal::getDisplayName(Display display, hidl_string* outName) {
    uint32_t count = 0;
    auto err = getDisplay(display).getDisplayName(&count, nullptr);

    if (err != Error::NONE) {
        return err;
    }

    std::vector<char> buf(count + 1);
    err = getDisplay(display).getDisplayName(&count, buf.data());

    if (err != Error::NONE) {
        return err;
    }

    buf.resize(count + 1);
    buf[count] = '\0';
    *outName = buf.data();
    return err;
}

Error HwcHal::getDisplayType(Display display,
                             IComposerClient::DisplayType* outType) {
    int32_t hwc_type = HWC2_DISPLAY_TYPE_INVALID;
    const auto err = getDisplay(display).getDisplayType(&hwc_type);
    *outType = static_cast<IComposerClient::DisplayType>(hwc_type);
    return err;
}

Error HwcHal::getDozeSupport(Display display, bool* outSupport) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    int32_t hwc_support = 0;
    const auto err = getDisplay(display).getDozeSupport(&hwc_support);
    *outSupport = hwc_support;
    return err;
}

Error HwcHal::getHdrCapabilities(
    Display display,
    hidl_vec<Hdr>* outTypes,
    float* outMaxLuminance,
    float* outMaxAverageLuminance,
    float* outMinLuminance) {
    uint32_t count = 0;
    auto err = getDisplay(display).getHdrCapabilities(
                   &count, nullptr, outMaxLuminance, outMaxAverageLuminance, outMinLuminance);

    if (err != Error::NONE) {
        return err;
    }

    outTypes->resize(count);
    err = getDisplay(display).getHdrCapabilities(
              &count,
              reinterpret_cast<std::underlying_type<Hdr>::type*>(outTypes->data()),
              outMaxLuminance,
              outMaxAverageLuminance,
              outMinLuminance);

    if (err != Error::NONE) {
        *outTypes = hidl_vec<Hdr>();
        return err;
    }

    return Error::NONE;
}

Error HwcHal::setActiveConfig(Display display, Config config) {
    return getDisplay(display).setActiveConfig(config);
}

Error HwcHal::setColorMode(Display display, ColorMode mode, RenderIntent intent) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    return getDisplay(display).setColorMode(static_cast<int32_t>(mode),
                                            static_cast<int32_t>(intent));
}

Error HwcHal::setPowerMode(Display display, PowerMode_V2_1 mode) {
    if (!isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    return getDisplay(display).setPowerMode(static_cast<int32_t>(mode));
}

Error HwcHal::setVsyncEnabled(Display display, IComposerClient::Vsync enabled) {
    return getDisplay(display).setVsyncEnabled(static_cast<int32_t>(enabled));
}

Error HwcHal::setColorTransform(Display display, const float* matrix,
                                int32_t hint) {
    return getDisplay(display).setColorTransform(matrix, hint);
}

Error HwcHal::setClientTarget(
    Display display,
    buffer_handle_t target,
    int32_t acquireFence,
    int32_t dataspace,
    const std::vector<hwc_rect_t>& damage) {
    hwc_region region = {damage.size(), damage.data()};
    return getDisplay(display).setClientTarget(target, acquireFence, dataspace,
            region);
}

Error HwcHal::setOutputBuffer(Display display, buffer_handle_t buffer,
                              int32_t releaseFence) {
    auto err = getDisplay(display).setOutputBuffer(buffer, releaseFence);

    if (err == Error::NONE && releaseFence >= 0) {
        close(releaseFence);
    }

    return err;
}

Error HwcHal::validateDisplay(
    Display display,
    std::vector<Layer>* outChangedLayers,
    std::vector<IComposerClient::Composition>* outCompositionTypes,
    uint32_t* outDisplayRequestMask,
    std::vector<Layer>* outRequestedLayers,
    std::vector<uint32_t>* outRequestMasks) {

    uint32_t types_count = 0;
    uint32_t reqs_count = 0;
    auto err = getDisplay(display).validateDisplay(&types_count, &reqs_count);

    if (err != Error::NONE /*&& err != HWC2_ERROR_HAS_CHANGES*/) {
        return err;
    }

    err = getDisplay(display).getChangedCompositionTypes(&types_count, nullptr,
            nullptr);

    if (err != Error::NONE) {
        return err;
    }

    outChangedLayers->resize(types_count);
    outCompositionTypes->resize(types_count);
    err = getDisplay(display).getChangedCompositionTypes(
              &types_count,
              outChangedLayers->data(),
              reinterpret_cast<std::underlying_type<IComposerClient::Composition>::type*>(
                  outCompositionTypes->data()));

    if (err != Error::NONE) {
        outChangedLayers->clear();
        outCompositionTypes->clear();
        return err;
    }

    int32_t display_reqs = 0;
    err = getDisplay(display).getDisplayRequests(&display_reqs, &reqs_count,
            nullptr, nullptr);

    if (err != Error::NONE) {
        outChangedLayers->clear();
        outCompositionTypes->clear();
        return err;
    }

    outRequestedLayers->resize(reqs_count);
    outRequestMasks->resize(reqs_count);
    err = getDisplay(display).getDisplayRequests(
        &display_reqs,
        &reqs_count,
        outRequestedLayers->data(),
        reinterpret_cast<int32_t*>(outRequestMasks->data()));

    if (err != Error::NONE) {
        outChangedLayers->clear();
        outCompositionTypes->clear();
        outRequestedLayers->clear();
        outRequestMasks->clear();
        return err;
    }

    *outDisplayRequestMask = display_reqs;
    return err;
}

Error HwcHal::acceptDisplayChanges(Display display) {
    return getDisplay(display).acceptDisplayChanges();
}

void HwcHal::evsCameraStream(size_t cameraDisplay, const hidl_handle& buffer) {
    if (mIsCameraEnabled) {
#if HWC_PRIME_CACHE
        PrimeCache::getInstance().isCacheEnabled() = false;
        PrimeCache::getInstance().clear();
#endif
        mCameraHidlHandle = buffer;
        mDisplays.at(cameraDisplay).evsStartCameraLayer(static_cast<buffer_handle_t>
                                                        (mCameraHidlHandle.getNativeHandle()));

        mDisplays.at(cameraDisplay).evsPresentDisplay();
    } else {
        mDisplays.at(cameraDisplay).evsStopCameraLayer();
        getClient()->onRefresh(cameraDisplay);
#if HWC_PRIME_CACHE
        PrimeCache::getInstance().isCacheEnabled() = true;
#endif
    }
}

Error HwcHal::presentDisplay(
    Display display,
    int32_t* outPresentFence,
    std::vector<Layer>* outLayers,
    std::vector<int32_t>* outReleaseFences) {
    if (!mIsHotplugInitialized) {
        for (std::pair<const hwc2_display_t, HwcDisplay>& disp : mDisplays) {
            if (disp.first != HWC_DISPLAY_PRIMARY) {
                ALOGD("hotplug crunch. %d", static_cast<int32_t>(disp.first));
                hotplugHook(this, disp.first,
                            static_cast<int32_t>(HWC2::Connection::Connected));
            }
        }

        mIsHotplugInitialized = true;
#if HWC_HOTPLUG_SUPPORT
        ALOGD("HotPlug support");
        HotPlug::getInstance().startHotPlugMonitor();
#endif
    }

    *outPresentFence = -1;
    if (mIsCameraEnabled &&
            (display == mCameraDisplayId || mCameraStreamAllDisplays)) {
        return Error::NONE;
    }

    Error err = getDisplay(display).presentDisplay(outPresentFence);

    if (err != Error::NONE) {
        return err;
    }

    if (mIsCameraEnabled && display == HWC_DISPLAY_EXTERNAL) {
        getDisplay(display).syncFence( mDisplays.at(HWC_DISPLAY_PRIMARY).getDisplayHandle() );
    }

    uint32_t count = 0;
    err = getDisplay(display).getReleaseFences(&count, nullptr, nullptr);

    if (err != Error::NONE) {
        ALOGW("failed to get release fences");
        return Error::NONE;
    }

    outLayers->resize(count);
    outReleaseFences->resize(count);
    err = getDisplay(display).getReleaseFences(&count, outLayers->data(),
                                               outReleaseFences->data());

    if (err != Error::NONE) {
        ALOGW("failed to get release fences");
        outLayers->clear();
        outReleaseFences->clear();
        return Error::NONE;
    }

    return err;
}

Error HwcHal::setLayerCursorPosition(Display display, Layer layer, int32_t x,
                                     int32_t y) {
    return getLayer(display, layer).setCursorPosition(x, y);
}

Error HwcHal::setLayerBuffer(
    Display display, Layer layer, buffer_handle_t buffer, int32_t acquireFence) {
    return getLayer(display, layer).setLayerBuffer(buffer, acquireFence);
}

Error HwcHal::setLayerSurfaceDamage(
    Display display, Layer layer, const std::vector<hwc_rect_t>& damage) {
    hwc_region region = {damage.size(), damage.data()};
    return getLayer(display, layer).setLayerSurfaceDamage(region);
}

Error HwcHal::setLayerBlendMode(Display display, Layer layer, int32_t mode) {
    return getLayer(display, layer).setLayerBlendMode(mode);
}

Error HwcHal::setLayerColor(Display display, Layer layer,
                            IComposerClient::Color color) {
    hwc_color_t hwc_color{color.r, color.g, color.b, color.a};
    return getLayer(display, layer).setLayerColor(hwc_color);
}

Error HwcHal::setLayerCompositionType(Display display, Layer layer,
                                      int32_t type) {
    return getLayer(display, layer).setLayerCompositionType(type);
}

Error HwcHal::setLayerDataspace(Display display, Layer layer,
                                int32_t dataspace) {
    return getLayer(display, layer).setLayerDataspace(dataspace);
}

Error HwcHal::setLayerDisplayFrame(Display display, Layer layer,
                                   const hwc_rect_t& frame) {
    return getLayer(display, layer).setLayerDisplayFrame(frame);
}

Error HwcHal::setLayerPlaneAlpha(Display display, Layer layer, float alpha) {
    return getLayer(display, layer).setLayerPlaneAlpha(alpha);
}

Error HwcHal::setLayerSidebandStream(Display display, Layer layer,
                                     buffer_handle_t stream) {
    return getLayer(display, layer).setLayerSidebandStream(stream);
}

Error HwcHal::setLayerSourceCrop(Display display, Layer layer,
                                 const hwc_frect_t& crop) {
    return getLayer(display, layer).setLayerSourceCrop(crop);
}

Error HwcHal::setLayerTransform(Display display, Layer layer,
                                int32_t transform) {
    return getLayer(display, layer).setLayerTransform(transform);
}

Error HwcHal::setLayerVisibleRegion(
    Display display, Layer layer, const std::vector<hwc_rect_t>& visible) {
    hwc_region_t region = {visible.size(), visible.data()};
    return getLayer(display, layer).setLayerVisibleRegion(region);
}

Error HwcHal::setLayerZOrder(Display display, Layer layer, uint32_t z) {
    return getLayer(display, layer).setLayerZOrder(z);
}

void HwcHal::RegisterCallback(
    HWC2::Callback descriptor, hwc2_callback_data_t data,
    hwc2_function_pointer_t function) {
    supported(__func__);

    if (function == nullptr) {
        mCallbacks.erase(descriptor);
        return;
    }

    mCallbacks.emplace(descriptor, HwcCallback(data, function));

    switch (descriptor) {
    case HWC2::Callback::Hotplug: {
        hotplugHook(data, HWC_DISPLAY_PRIMARY,
                static_cast<int32_t>(HWC2::Connection::Connected));
        break;
    }

    case HWC2::Callback::Refresh: {
        break;
    }

    case HWC2::Callback::Vsync: {
        for (std::pair<const hwc2_display_t, HwcDisplay>& d : mDisplays)
            d.second.registerVsyncCallback(data, function);

        break;
    }

    default:
        ALOGE("Unsupported callback");
        break;
    }
}

Return<Error> HwcHal::setEVSCameraData(const hidl_handle& buffer, int8_t currDisplay) {
    mIsCameraEnabled = (buffer != nullptr);

    switch(currDisplay) {
    case HWC_DISPLAY_PRIMARY:

        [[fallthrough]];
    case HWC_DISPLAY_EXTERNAL:
        mCameraDisplayId = currDisplay;
        evsCameraStream(currDisplay, buffer);
        break;

    default:
        mCameraStreamAllDisplays = true;
        for(size_t display = 0; display < mDisplays.size(); ++display) {
            evsCameraStream(display, buffer);
        }
        break;
    }
    return Error::NONE;
}

template <typename Deleter>
static auto deleterFactory(Deleter const& del) {
    return [del](auto *p){ del(p); };
}

void HwcHal::getDisplayIdentificationData(uint64_t display,
        IComposerClient::getDisplayIdentificationData_cb _hidl_cb) {

    if (!isDisplayValid(display)) {
        _hidl_cb(Error::BAD_DISPLAY, {}, {});
        return;
    }

    const auto connectorId = mDisplays.at(display).getConnectorId();
    std::vector<uint8_t> data;

    const auto& objPropsDeleter = deleterFactory(drmModeFreeObjectProperties);
    std::unique_ptr<drmModeObjectProperties, decltype(objPropsDeleter)> props {
        drmModeObjectGetProperties(mDrmFd, connectorId, DRM_MODE_OBJECT_CONNECTOR),
        objPropsDeleter
    };

    if (!props) {
        _hidl_cb(Error::NO_RESOURCES, {}, {});
        return;
    }

    for (uint32_t i = 0; i < props->count_props; i++) {
        const auto& propResDeleter = deleterFactory(drmModeFreeProperty);
        std::unique_ptr<drmModePropertyRes, decltype(propResDeleter)> prop_info {
            drmModeGetProperty(mDrmFd, props->props[i]),
            propResDeleter
        };

        if (!prop_info) {
            _hidl_cb(Error::NO_RESOURCES, {}, {});
            return;
        }

        if (drm_property_type_is(prop_info.get(), DRM_MODE_PROP_BLOB)) {
            const auto& propBlobDeleter = deleterFactory(drmModeFreePropertyBlob);
            std::unique_ptr<drmModePropertyBlobRes, decltype(propBlobDeleter)> blob {
                drmModeGetPropertyBlob(mDrmFd, props->prop_values[i]),
                propBlobDeleter
            };

            // blob may be nullptr if there is no EDID
            if (blob) {
                auto blob_data = static_cast<const unsigned char*>(blob->data);
                for (uint32_t j = 0; j < blob->length; j++) {
                    data.push_back(blob_data[j]);
                }
            }
        }

    }
    _hidl_cb(Error::NONE, connectorId, data);
}

}  // namespace implementation
}  // namespace V2_3
}  // namespace composer
}  // namespace graphics
}  // namespace hardware
}  // namespace android
