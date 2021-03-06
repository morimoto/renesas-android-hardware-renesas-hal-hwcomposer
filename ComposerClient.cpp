/*
 * Copyright 2016 The Android Open Source Project
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

#include "ComposerClient.h"

#include <android/hardware/graphics/mapper/3.0/IMapper.h>
#include <log/log.h>

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_3 {
namespace implementation {

namespace {

using MapperError = android::hardware::graphics::mapper::V3_0::Error;
using android::hardware::graphics::mapper::V3_0::IMapper;
using Hdr_V1_2 = ::android::hardware::graphics::common::V1_2::Hdr;

class HandleImporter {
public:
    bool initialize() {
        // allow only one client
        if (mInitialized) {
            return false;
        }

        mMapper = IMapper::getService();
        mInitialized = true;
        return true;
    }

    void cleanup() {
        mMapper.clear();
        mInitialized = false;
    }

    // In IComposer, any buffer_handle_t is owned by the caller and we need to
    // make a clone for hwcomposer2.  We also need to translate empty handle
    // to nullptr.  This function does that, in-place.
    bool importBuffer(buffer_handle_t& handle) {
        if (!handle) {
            return true;
        }

        if (!handle->numFds && !handle->numInts) {
            handle = nullptr;
            return true;
        }

        MapperError error;
        buffer_handle_t importedHandle;
        mMapper->importBuffer(
            hidl_handle(handle),
        [&](const auto & tmpError, const auto & tmpBufferHandle) {
            error = tmpError;
            importedHandle = static_cast<buffer_handle_t>(tmpBufferHandle);
        });

        if (error != MapperError::NONE) {
            return false;
        }

        handle = importedHandle;
        return true;
    }

    void freeBuffer(buffer_handle_t handle) {
        if (!handle) {
            return;
        }

        mMapper->freeBuffer(const_cast<native_handle_t*>(handle));
    }

private:
    bool mInitialized = false;
    sp<IMapper> mMapper;
};

HandleImporter sHandleImporter;

} // anonymous namespace

BufferCacheEntry::BufferCacheEntry()
    : mHandle(nullptr) {
}

BufferCacheEntry::BufferCacheEntry(BufferCacheEntry&& other) {
    mHandle = other.mHandle;
    other.mHandle = nullptr;
}

BufferCacheEntry& BufferCacheEntry::operator=(buffer_handle_t handle) {
    clear();
    mHandle = handle;
    return *this;
}

BufferCacheEntry::~BufferCacheEntry() {
    clear();
}

void BufferCacheEntry::clear() {
    if (mHandle) {
        sHandleImporter.freeBuffer(mHandle);
    }
}

ComposerClient::ComposerClient(ComposerBase& hal)
    : mHal(hal), mWriter(kWriterInitialSize) {
}

ComposerClient::~ComposerClient() {
    // We want to call hwc2_close here (and move hwc2_open to the
    // constructor), with the assumption that hwc2_close would
    //
    //  - clean up all resources owned by the client
    //  - make sure all displays are blank (since there is no layer)
    //
    // But since SF used to crash at this point, different hwcomposer2
    // implementations behave differently on hwc2_close.  Our only portable
    // choice really is to abort().  But that is not an option anymore
    // because we might also have VTS or VR as clients that can come and go.
    //
    // Below we manually clean all resources (layers and virtual
    // displays), and perform a presentDisplay afterwards.
    ALOGW("destroying composer client");
    mHal.enableCallback(false);

    // no need to grab the mutex as any in-flight hwbinder call would have
    // kept the client alive
    for (const auto& dpy : mDisplayData) {
        ALOGW("destroying client resources for display %zu", dpy.first);

        for (const auto& ly : dpy.second.Layers) {
            mHal.destroyLayer(dpy.first, ly.first);
        }

        if (dpy.second.IsVirtual) {
            mHal.destroyVirtualDisplay(dpy.first);
        } else {
            ALOGW("performing a final presentDisplay");
            std::vector<Layer> changedLayers;
            std::vector<IComposerClient::Composition> compositionTypes;
            uint32_t displayRequestMask = 0;
            std::vector<Layer> requestedLayers;
            std::vector<uint32_t> requestMasks;
            mHal.validateDisplay(dpy.first, &changedLayers, &compositionTypes,
                                 &displayRequestMask, &requestedLayers, &requestMasks);
            mHal.acceptDisplayChanges(dpy.first);
            int32_t presentFence = -1;
            std::vector<Layer> releasedLayers;
            std::vector<int32_t> releaseFences;
            mHal.presentDisplay(dpy.first, &presentFence, &releasedLayers, &releaseFences);

            if (presentFence >= 0) {
                close(presentFence);
            }

            for (auto fence : releaseFences) {
                if (fence >= 0) {
                    close(fence);
                }
            }
        }
    }

    mDisplayData.clear();
    sHandleImporter.cleanup();
    mHal.removeClient();
    ALOGW("removed composer client");
}

void ComposerClient::initialize() {
    mReader = createCommandReader();

    if (!sHandleImporter.initialize()) {
        LOG_ALWAYS_FATAL("failed to initialize handle importer");
    }
}

Return<void> ComposerClient::getDisplayedContentSample(
        [[maybe_unused]]uint64_t display, [[maybe_unused]]uint64_t maxFrames,
        [[maybe_unused]]uint64_t timestamp,
        [[maybe_unused]]getDisplayedContentSample_cb _hidl_cb) {
    // TODO implement
    _hidl_cb(Error::UNSUPPORTED, {}, {}, {}, {}, {});
    return Void();
}

// Duplicate of ComposerClient::executeCommands
Return<void> ComposerClient::executeCommands_2_3(uint32_t inLength,
        const hidl_vec<hidl_handle>& inHandles,
        executeCommands_2_3_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    bool outChanged = false;
    uint32_t outLength = 0;
    hidl_vec<hidl_handle> outHandles;

    if (!mReader->readQueue(inLength, inHandles)) {
        _hidl_cb(Error::BAD_PARAMETER, outChanged, outLength, outHandles);
        return Void();
    }

    Error err = mReader->parse();

    if (err == Error::NONE &&
        !mWriter.writeQueue(&outChanged, &outLength, &outHandles)) {
        err = Error::NO_RESOURCES;
    }

    _hidl_cb(err, outChanged, outLength, outHandles);
    mReader->reset();
    mWriter.reset();
    return Void();
}

Return<void> ComposerClient::getRenderIntents_2_3(
        uint64_t display, ColorMode_V1_2 mode,
        getRenderIntents_2_3_cb _hidl_cb) {
    auto err = Error::NONE;

    if (!mHal.isDisplayValid(display)) {
        err = Error::BAD_DISPLAY;
    }

    if (static_cast<int32_t>(mode) < 0) {
        err = Error::BAD_PARAMETER;
    }

    // COLORIMETRIC the only render intent compatible with ColorMode::NATIVE
    _hidl_cb(err, {RenderIntent::COLORIMETRIC});
    return Void();
}

Return<void> ComposerClient::getColorModes_2_3(uint64_t display,
        getColorModes_2_3_cb _hidl_cb) {
    hidl_vec<ColorMode_V1_2> modes;
    auto err = mHal.getColorModes(display,
            // new formats don't matter for now
            reinterpret_cast<hidl_vec<ColorMode>*>(&modes));
    _hidl_cb(err, modes);
    return Void();
}

Return<Error> ComposerClient::setColorMode_2_3(
        uint64_t display, ColorMode_V1_2 mode, RenderIntent intent) {
    auto err = mHal.setColorMode(display,
            // new formats don't matter for now
            static_cast<ColorMode>(mode), intent);
    return err;
}

Return<void> ComposerClient::getDisplayCapabilities(
        uint64_t display, getDisplayCapabilities_cb _hidl_cb) {
    auto err = Error::NONE;

    if (!mHal.isDisplayValid(display)) {
        err = Error::BAD_DISPLAY;
    }

    // stub
    _hidl_cb(err, {DisplayCapability::INVALID});
    return Void();
}

Return<void> ComposerClient::getPerFrameMetadataKeys_2_3(
        [[maybe_unused]]uint64_t display,
        [[maybe_unused]]getPerFrameMetadataKeys_2_3_cb _hidl_cb) {
    // TODO implement
    _hidl_cb(Error::UNSUPPORTED, {});
    return Void();
}

Return<void> ComposerClient::getHdrCapabilities_2_3(
        uint64_t display, getHdrCapabilities_2_3_cb _hidl_cb) {
    hidl_vec<Hdr_V1_2> types;
    float max_lumi = 0.0f;
    float max_avg_lumi = 0.0f;
    float min_lumi = 0.0f;
    auto err = mHal.getHdrCapabilities(display,
            // new formats don't matter for now
            reinterpret_cast<hidl_vec<Hdr>*>(&types),
            &max_lumi, &max_avg_lumi, &min_lumi);
    _hidl_cb(err, types, max_lumi, max_avg_lumi, min_lumi);
    return Void();
}

Return<void> ComposerClient::getDisplayBrightnessSupport(
        [[maybe_unused]]uint64_t display,
        [[maybe_unused]]getDisplayBrightnessSupport_cb _hidl_cb) {
    // TODO implement
    _hidl_cb(Error::UNSUPPORTED, {});
    return Void();
}

Return<Error> ComposerClient::setDisplayBrightness(
        [[maybe_unused]]uint64_t display,
        [[maybe_unused]]float brightness) {
    // TODO implement
    return Error::UNSUPPORTED;
}

void ComposerClient::onHotplug(Display display,
        IComposerCallback::Connection connected) {
    {
        std::lock_guard<std::mutex> lock(mDisplayDataMutex);

        if (connected == IComposerCallback::Connection::CONNECTED) {
            mDisplayData.emplace(display, DisplayData(false));
        } else if (connected == IComposerCallback::Connection::DISCONNECTED) {
            mDisplayData.emplace(display, DisplayData(true));
        }
    }

    if (!mCallback.get()) {
        ALOGE("ComposerClient::onHotplug. mCallback is invalid.");
        return;
    }

    auto ret = mCallback->onHotplug(display, connected);
    ALOGE_IF(!ret.isOk(), "failed to send onHotplug: %s",
             ret.description().c_str());
}

void ComposerClient::onRefresh(Display display) {
    if (!mCallback.get()) {
        ALOGE("ComposerClient::onRefresh. mCallback is invalid.");
        return;
    }

    auto ret = mCallback->onRefresh(display);
    ALOGE_IF(!ret.isOk(), "failed to send onRefresh: %s",
             ret.description().c_str());
}

void ComposerClient::onVsync(Display display, int64_t timestamp) {
    if (!mCallback.get()) {
        ALOGE("ComposerClient::onVsync. mCallback is invalid.");
        return;
    }

    auto ret = mCallback->onVsync(display, timestamp);
    ALOGE_IF(!ret.isOk(), "failed to send onVsync: %s",
             ret.description().c_str());
}

Return<void> ComposerClient::registerCallback(
    const sp<IComposerCallback>& callback) {
    // no locking as we require this function to be called only once
    mCallback = callback;
    mHal.enableCallback(callback != nullptr);
    return Void();
}

Return<uint32_t> ComposerClient::getMaxVirtualDisplayCount() {
    return mHal.getMaxVirtualDisplayCount();
}

Return<void> ComposerClient::createVirtualDisplay(uint32_t width,
        uint32_t height, PixelFormat_V1_0 formatHint,
        uint32_t outputBufferSlotCount, createVirtualDisplay_cb hidl_cb) {
    Display display = 0;
    Error err = mHal.createVirtualDisplay(width, height,
                                          &formatHint, &display);

    if (err == Error::NONE) {
        std::lock_guard<std::mutex> lock(mDisplayDataMutex);
        auto dpy = mDisplayData.emplace(display, DisplayData(true)).first;
        dpy->second.OutputBuffers.resize(outputBufferSlotCount);
    }

    hidl_cb(err, display, formatHint);
    return Void();
}

Return<Error> ComposerClient::destroyVirtualDisplay(Display display) {
    Error err = mHal.destroyVirtualDisplay(display);

    if (err == Error::NONE) {
        std::lock_guard<std::mutex> lock(mDisplayDataMutex);
        mDisplayData.erase(display);
    }

    return err;
}

Return<void> ComposerClient::createLayer(Display display,
        uint32_t bufferSlotCount, createLayer_cb hidl_cb) {
    Layer layer = 0;
    Error err = mHal.createLayer(display, &layer);

    if (err == Error::NONE) {
        std::lock_guard<std::mutex> lock(mDisplayDataMutex);
        auto dpy = mDisplayData.find(display);
        auto ly = dpy->second.Layers.emplace(layer, LayerBuffers()).first;
        ly->second.Buffers.resize(bufferSlotCount);
    }

    hidl_cb(err, layer);
    return Void();
}

Return<Error> ComposerClient::destroyLayer(Display display, Layer layer) {
    Error err = mHal.destroyLayer(display, layer);

    if (err == Error::NONE) {
        std::lock_guard<std::mutex> lock(mDisplayDataMutex);
        auto dpy = mDisplayData.find(display);
        dpy->second.Layers.erase(layer);
    }

    return err;
}

Return<void> ComposerClient::getColorModes_2_2(uint64_t display,
        getColorModes_2_2_cb _hidl_cb) {
    hidl_vec<ColorMode_V1_1> modes;
    auto err = mHal.getColorModes(display,
            // new formats don't matter for now
            reinterpret_cast<hidl_vec<ColorMode>*>(&modes));
    _hidl_cb(err, modes);
    return Void();
}

Return<Error> ComposerClient::setPowerMode_2_2(
        uint64_t display, PowerMode_V2_2 mode) {
    auto err = mHal.setPowerMode(display,
            // new formats don't matter for now
            static_cast<PowerMode_V2_1>(mode));
    return err;
}


Return<Error> ComposerClient::getClientTargetSupport_2_2(
        uint64_t display, uint32_t width,
        uint32_t height, PixelFormat_V1_1 format,
        Dataspace_V1_1 dataspace) {
    auto err = mHal.getClientTargetSupport(display,
            width, height,
            // new formats don't matter for now
            static_cast<PixelFormat_V1_0>(format),
            static_cast<Dataspace_V1_0>(dataspace));
    return err;
}

Return<void> ComposerClient::getActiveConfig(Display display,
        getActiveConfig_cb hidl_cb) {
    Config config = 0;
    Error err = mHal.getActiveConfig(display, &config);
    hidl_cb(err, config);
    return Void();
}

Return<Error> ComposerClient::getClientTargetSupport(Display display,
        uint32_t width, uint32_t height,
        PixelFormat_V1_0 format, Dataspace_V1_0 dataspace) {
    Error err = mHal.getClientTargetSupport(display,
                                            width, height,
                                            format, dataspace);
    return err;
}

Return<void> ComposerClient::getColorModes(Display display,
        getColorModes_cb hidl_cb) {
    hidl_vec<ColorMode> modes;
    Error err = mHal.getColorModes(display, &modes);
    hidl_cb(err, modes);
    return Void();
}

Return<void> ComposerClient::getReadbackBufferAttributes_2_3(
        uint64_t display, getReadbackBufferAttributes_2_3_cb _hidl_cb) {
    Error err = mHal.isDisplayValid(display) ? Error::NONE : Error::BAD_DISPLAY;

    // RGB_565 also supported
    _hidl_cb(err, PixelFormat_V1_2::BGRA_8888, Dataspace_V1_2::RANGE_FULL);
    return Void();
}

Return<Error> ComposerClient::getClientTargetSupport_2_3(
        uint64_t display, uint32_t width,
        uint32_t height, PixelFormat_V1_2 format,
        Dataspace_V1_2 dataspace) {
    auto err = mHal.getClientTargetSupport(display,
            width, height,
            // new formats don't matter for now
            static_cast<PixelFormat_V1_0>(format),
            static_cast<Dataspace_V1_0>(dataspace));
    return err;
}

// ...::V2_3::IComposerClient
Return<void> ComposerClient::getDisplayIdentificationData(
        uint64_t display, getDisplayIdentificationData_cb _hidl_cb) {
    mHal.getDisplayIdentificationData(display, _hidl_cb);
    return Void();
}

Return<void> ComposerClient::getDisplayAttribute(Display display,
        Config config, Attribute attribute,
        getDisplayAttribute_cb hidl_cb) {
    int32_t value = 0;
    Error err = mHal.getDisplayAttribute(display, config, attribute, &value);
    hidl_cb(err, value);
    return Void();
}

Return<void> ComposerClient::getDisplayConfigs(Display display,
        getDisplayConfigs_cb hidl_cb) {
    hidl_vec<Config> configs;
    Error err = mHal.getDisplayConfigs(display, &configs);
    hidl_cb(err, configs);
    return Void();
}

Return<void> ComposerClient::getDisplayName(Display display,
        getDisplayName_cb hidl_cb) {
    hidl_string name;
    Error err = mHal.getDisplayName(display, &name);
    hidl_cb(err, name);
    return Void();
}

Return<void> ComposerClient::getDisplayType(Display display,
        getDisplayType_cb hidl_cb) {
    DisplayType type = DisplayType::INVALID;
    Error err = mHal.getDisplayType(display, &type);
    hidl_cb(err, type);
    return Void();
}

Return<void> ComposerClient::getDozeSupport(Display display,
        getDozeSupport_cb hidl_cb) {
    bool support = false;
    Error err = mHal.getDozeSupport(display, &support);
    hidl_cb(err, support);
    return Void();
}

Return<void> ComposerClient::getHdrCapabilities(Display display,
        getHdrCapabilities_cb hidl_cb) {
    hidl_vec<Hdr> types;
    float max_lumi = 0.0f;
    float max_avg_lumi = 0.0f;
    float min_lumi = 0.0f;
    Error err = mHal.getHdrCapabilities(display, &types,
                                        &max_lumi, &max_avg_lumi, &min_lumi);
    hidl_cb(err, types, max_lumi, max_avg_lumi, min_lumi);
    return Void();
}

Return<Error> ComposerClient::setClientTargetSlotCount(Display display,
        uint32_t clientTargetSlotCount) {
    std::lock_guard<std::mutex> lock(mDisplayDataMutex);
    auto dpy = mDisplayData.find(display);

    if (dpy == mDisplayData.end()) {
        return Error::BAD_DISPLAY;
    }

    dpy->second.ClientTargets.resize(clientTargetSlotCount);
    return Error::NONE;
}

Return<Error> ComposerClient::setActiveConfig(Display display, Config config) {
    Error err = mHal.setActiveConfig(display, config);
    return err;
}

Return<Error> ComposerClient::setColorMode(Display display, ColorMode mode) {
    Error err = mHal.setColorMode(display, mode);
    return err;
}

Return<Error> ComposerClient::setPowerMode(Display display, PowerMode_V2_1 mode) {
    Error err = mHal.setPowerMode(display, mode);
    return err;
}

Return<Error> ComposerClient::setVsyncEnabled(Display display, Vsync enabled) {
    Error err = mHal.setVsyncEnabled(display, enabled);
    return err;
}

Return<Error> ComposerClient::setInputCommandQueue(
    const MQDescriptorSync<uint32_t>& descriptor) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    return mReader->setMQDescriptor(descriptor) ?
           Error::NONE : Error::NO_RESOURCES;
}

Return<void> ComposerClient::getOutputCommandQueue(
    getOutputCommandQueue_cb hidl_cb) {
    // no locking as we require this function to be called inside
    // executeCommands_cb
    auto outDescriptor = mWriter.getMQDescriptor();

    if (outDescriptor) {
        hidl_cb(Error::NONE, *outDescriptor);
    } else {
        hidl_cb(Error::NO_RESOURCES, CommandQueueType::Descriptor());
    }

    return Void();
}

Return<void> ComposerClient::executeCommands(uint32_t inLength,
        const hidl_vec<hidl_handle>& inHandles,
        executeCommands_cb hidl_cb) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    bool outChanged = false;
    uint32_t outLength = 0;
    hidl_vec<hidl_handle> outHandles;

    if (!mReader->readQueue(inLength, inHandles)) {
        hidl_cb(Error::BAD_PARAMETER, outChanged, outLength, outHandles);
        return Void();
    }

    Error err = mReader->parse();

    if (err == Error::NONE &&
        !mWriter.writeQueue(&outChanged, &outLength, &outHandles)) {
        err = Error::NO_RESOURCES;
    }

    hidl_cb(err, outChanged, outLength, outHandles);
    mReader->reset();
    mWriter.reset();
    return Void();
}

std::unique_ptr<ComposerClient::CommandReader>
ComposerClient::createCommandReader() {
    return std::unique_ptr<ComposerClient::CommandReader>(
               new CommandReader(*this));
}

// Methods from ::android::hardware::graphics::composer::V2_2::IComposerClient follow.
Return<void> ComposerClient::getPerFrameMetadataKeys(
        [[maybe_unused]]uint64_t display,
        [[maybe_unused]]getPerFrameMetadataKeys_cb _hidl_cb) {
    // TODO implement
    _hidl_cb(Error::UNSUPPORTED, {});
    return Void();
}

ComposerClient::CommandReader::CommandReader(ComposerClient& client)
    : mClient(client), mHal(client.mHal), mWriter(client.mWriter) {
}


Return<void> ComposerClient::createVirtualDisplay_2_2(
        [[maybe_unused]]uint32_t width, [[maybe_unused]]uint32_t height,
        [[maybe_unused]]PixelFormat_V1_1 formatHint,
        [[maybe_unused]]uint32_t outputBufferSlotCount,
        [[maybe_unused]]createVirtualDisplay_2_2_cb _hidl_cb) {
    // TODO implement
    _hidl_cb(Error::UNSUPPORTED, {}, {});
    return Void();
}

ComposerClient::CommandReader::~CommandReader() {
}

Error ComposerClient::CommandReader::parse() {
    IComposerClient::Command command;
    uint16_t length = 0;

    while (!isEmpty()) {
        if (!beginCommand(&command, &length)) {
            break;
        }

        bool parsed = parseCommand(command, length);
        endCommand();

        if (!parsed) {
            ALOGE("failed to parse command 0x%x, length %" PRIu16,
                  command, length);
            break;
        }
    }

    return (isEmpty()) ? Error::NONE : Error::BAD_PARAMETER;
}

bool ComposerClient::CommandReader::parseCommand(
    IComposerClient::Command command, uint16_t length) {
    switch (command) {
    case IComposerClient::Command::SET_LAYER_COLOR_TRANSFORM:
        return parseSetLayerColorTransform(length);

    case IComposerClient::Command::SET_LAYER_PER_FRAME_METADATA:
        return parseSetLayerPerFrameMetadata(length);

    case IComposerClient::Command::SET_LAYER_FLOAT_COLOR:
        return parseSetLayerFloatColor(length);

    case IComposerClient::Command::SET_LAYER_PER_FRAME_METADATA_BLOBS:
        return parseSetLayerPerFrameMetadataBlobs(length);

    case IComposerClient::Command::SELECT_DISPLAY:
        return parseSelectDisplay(length);

    case IComposerClient::Command::SELECT_LAYER:
        return parseSelectLayer(length);

    case IComposerClient::Command::SET_COLOR_TRANSFORM:
        return parseSetColorTransform(length);

    case IComposerClient::Command::SET_CLIENT_TARGET:
        return parseSetClientTarget(length);

    case IComposerClient::Command::SET_OUTPUT_BUFFER:
        return parseSetOutputBuffer(length);

    case IComposerClient::Command::VALIDATE_DISPLAY:
        return parseValidateDisplay(length);

    case IComposerClient::Command::PRESENT_OR_VALIDATE_DISPLAY:
        return parsePresentOrValidateDisplay(length);

    case IComposerClient::Command::ACCEPT_DISPLAY_CHANGES:
        return parseAcceptDisplayChanges(length);

    case IComposerClient::Command::PRESENT_DISPLAY:
        return parsePresentDisplay(length);

    case IComposerClient::Command::SET_LAYER_CURSOR_POSITION:
        return parseSetLayerCursorPosition(length);

    case IComposerClient::Command::SET_LAYER_BUFFER:
        return parseSetLayerBuffer(length);

    case IComposerClient::Command::SET_LAYER_SURFACE_DAMAGE:
        return parseSetLayerSurfaceDamage(length);

    case IComposerClient::Command::SET_LAYER_BLEND_MODE:
        return parseSetLayerBlendMode(length);

    case IComposerClient::Command::SET_LAYER_COLOR:
        return parseSetLayerColor(length);

    case IComposerClient::Command::SET_LAYER_COMPOSITION_TYPE:
        return parseSetLayerCompositionType(length);

    case IComposerClient::Command::SET_LAYER_DATASPACE:
        return parseSetLayerDataspace(length);

    case IComposerClient::Command::SET_LAYER_DISPLAY_FRAME:
        return parseSetLayerDisplayFrame(length);

    case IComposerClient::Command::SET_LAYER_PLANE_ALPHA:
        return parseSetLayerPlaneAlpha(length);

    case IComposerClient::Command::SET_LAYER_SIDEBAND_STREAM:
        return parseSetLayerSidebandStream(length);

    case IComposerClient::Command::SET_LAYER_SOURCE_CROP:
        return parseSetLayerSourceCrop(length);

    case IComposerClient::Command::SET_LAYER_TRANSFORM:
        return parseSetLayerTransform(length);

    case IComposerClient::Command::SET_LAYER_VISIBLE_REGION:
        return parseSetLayerVisibleRegion(length);

    case IComposerClient::Command::SET_LAYER_Z_ORDER:
        return parseSetLayerZOrder(length);

    default:
        return false;
    }
}

bool ComposerClient::CommandReader::parseSetLayerColorTransform(
    uint16_t length) {
    const auto expLen = CommandWriterBase::kSetLayerColorTransformLength;

    if (length != expLen) {
        return false;
    }

    for (int i = 0; i < expLen; ++i) {
        readFloat();
    }

    mWriter.setError(getCommandLoc(), Error::UNSUPPORTED);
    return true;
}

bool ComposerClient::CommandReader::parseSetLayerPerFrameMetadata(
    uint16_t length) {
    auto size = length / 2;

    for (int i = 0; i < size; ++i) {
        readSigned();
        readFloat();
    }

    mWriter.setError(getCommandLoc(), Error::UNSUPPORTED);
    return true;
}

bool ComposerClient::CommandReader::parseSetLayerFloatColor(uint16_t length) {
    auto expLen = CommandWriterBase::kSetLayerFloatColorLength;

    if (length != expLen) {
        return false;
    }

    for (int i = 0; i < expLen; ++i) {
        readFloat();
    }

    mWriter.setError(getCommandLoc(), Error::UNSUPPORTED);
    return true;
}

bool ComposerClient::CommandReader::parseSetLayerPerFrameMetadataBlobs(
    uint16_t /* length */) {
    const auto size = read();

    for (int i = 0; i < size; ++i) {
        readSigned();
        const auto blobSize = read();
        readBlob(blobSize);
    }

    mWriter.setError(getCommandLoc(), Error::UNSUPPORTED);
    return true;
}

bool ComposerClient::CommandReader::parseSelectDisplay(uint16_t length) {
    if (length != CommandWriterBase::kSelectDisplayLength) {
        return false;
    }

    mDisplay = read64();
    mWriter.selectDisplay(mDisplay);
    return true;
}

bool ComposerClient::CommandReader::parseSelectLayer(uint16_t length) {
    if (length != CommandWriterBase::kSelectLayerLength) {
        return false;
    }

    mLayer = read64();
    return true;
}

bool ComposerClient::CommandReader::parseSetColorTransform(uint16_t length) {
    if (length != CommandWriterBase::kSetColorTransformLength) {
        return false;
    }

    float matrix[16];

    for (int i = 0; i < 16; i++) {
        matrix[i] = readFloat();
    }

    auto transform = readSigned();
    auto err = mHal.setColorTransform(mDisplay, matrix, transform);

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetClientTarget(uint16_t length) {
    // 4 parameters followed by N rectangles
    if ((length - 4) % 4 != 0) {
        return false;
    }

    bool useCache = false;
    auto slot = read();
    auto clientTarget = readHandle(&useCache);
    auto fence = readFence();
    auto dataspace = readSigned();
    auto damage = readRegion((length - 4) / 4);
    bool closeFence = true;
    auto err = lookupBuffer(BufferCache::CLIENT_TARGETS,
                            slot, useCache, clientTarget, &clientTarget);

    if (err == Error::NONE) {
        err = mHal.setClientTarget(mDisplay, clientTarget, fence,
                                   dataspace, damage);
        auto updateBufErr = updateBuffer(BufferCache::CLIENT_TARGETS, slot,
                                         useCache, clientTarget);

        if (err == Error::NONE) {
            closeFence = false;
            err = updateBufErr;
        }
    }

    if (closeFence) {
        close(fence);
    }

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetOutputBuffer(uint16_t length) {
    if (length != CommandWriterBase::kSetOutputBufferLength) {
        return false;
    }

    bool useCache = false;
    auto slot = read();
    auto outputBuffer = readHandle(&useCache);
    auto fence = readFence();
    bool closeFence = true;
    auto err = lookupBuffer(BufferCache::OUTPUT_BUFFERS,
                            slot, useCache, outputBuffer, &outputBuffer);

    if (err == Error::NONE) {
        err = mHal.setOutputBuffer(mDisplay, outputBuffer, fence);
        auto updateBufErr = updateBuffer(BufferCache::OUTPUT_BUFFERS,
                                         slot, useCache, outputBuffer);

        if (err == Error::NONE) {
            closeFence = false;
            err = updateBufErr;
        }
    }

    if (closeFence) {
        close(fence);
    }

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseValidateDisplay(uint16_t length) {
    if (length != CommandWriterBase::kValidateDisplayLength) {
        return false;
    }

    std::vector<Layer> changedLayers;
    std::vector<IComposerClient::Composition> compositionTypes;
    uint32_t displayRequestMask = 0x0;
    std::vector<Layer> requestedLayers;
    std::vector<uint32_t> requestMasks;
    auto err = mHal.validateDisplay(mDisplay, &changedLayers,
                                    &compositionTypes, &displayRequestMask,
                                    &requestedLayers, &requestMasks);

    if (err == Error::NONE) {
        mWriter.setChangedCompositionTypes(changedLayers,
                                           compositionTypes);
        mWriter.setDisplayRequests(displayRequestMask,
                                   requestedLayers, requestMasks);
    } else {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parsePresentOrValidateDisplay(
    uint16_t length) {
    if (length != CommandWriterBase::kPresentOrValidateDisplayLength) {
        return false;
    }

    // First try to Present as is.
    int presentFence = -1;
    std::vector<Layer> layers;
    std::vector<int> fences;
    auto err = mHal.presentDisplay(mDisplay, &presentFence, &layers, &fences);

    if (err == Error::NONE) {
        mWriter.setPresentOrValidateResult(1);
        mWriter.setPresentFence(presentFence);
        mWriter.setReleaseFences(layers, fences);
        return true;
    }

    // Present has failed. We need to fallback to validate
    std::vector<Layer> changedLayers;
    std::vector<IComposerClient::Composition> compositionTypes;
    uint32_t displayRequestMask = 0x0;
    std::vector<Layer> requestedLayers;
    std::vector<uint32_t> requestMasks;
    err = mHal.validateDisplay(mDisplay, &changedLayers,
                               &compositionTypes, &displayRequestMask,
                               &requestedLayers, &requestMasks);

    if (err == Error::NONE) {
        mWriter.setPresentOrValidateResult(0);
        mWriter.setChangedCompositionTypes(changedLayers,
                                           compositionTypes);
        mWriter.setDisplayRequests(displayRequestMask,
                                   requestedLayers, requestMasks);
    } else {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseAcceptDisplayChanges(uint16_t length) {
    if (length != CommandWriterBase::kAcceptDisplayChangesLength) {
        return false;
    }

    auto err = mHal.acceptDisplayChanges(mDisplay);

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

Return<void> ComposerClient::executeCommands_2_2(
        uint32_t inLength, const hidl_vec<hidl_handle>& inHandles,
        executeCommands_2_2_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    bool outChanged = false;
    uint32_t outLength = 0;
    hidl_vec<hidl_handle> outHandles;

    if (!mReader->readQueue(inLength, inHandles)) {
        _hidl_cb(Error::BAD_PARAMETER, outChanged, outLength, outHandles);
        return Void();
    }

    Error err = mReader->parse();

    if (err == Error::NONE &&
        !mWriter.writeQueue(&outChanged, &outLength, &outHandles)) {
        err = Error::NO_RESOURCES;
    }

    _hidl_cb(err, outChanged, outLength, outHandles);
    mReader->reset();
    mWriter.reset();
    return Void();
}

Return<void> ComposerClient::getDataspaceSaturationMatrix(
    Dataspace_V1_1 dataspace,
    getDataspaceSaturationMatrix_cb _hidl_cb) {
    auto err = Error::NONE;

    if (dataspace != Dataspace_V1_1::SRGB_LINEAR) {
        err = Error::BAD_PARAMETER;
    }

    float matrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    _hidl_cb(err, matrix);
    return Void();
}

Return<void> ComposerClient::getRenderIntents(uint64_t display,
        ColorMode_V1_1 mode, getRenderIntents_cb _hidl_cb) {
    auto err = Error::NONE;

    if (!mHal.isDisplayValid(display)) {
        err = Error::BAD_DISPLAY;
    }

    if (static_cast<int32_t>(mode) < 0) {
        err = Error::BAD_PARAMETER;
    }

    _hidl_cb(err, {RenderIntent::COLORIMETRIC});
    return Void();
}

Return<Error> ComposerClient::setColorMode_2_2(
        uint64_t display, ColorMode_V1_1 mode, RenderIntent intent) {
    auto err = mHal.setColorMode(display,
            // new formats don't matter for now
            static_cast<ColorMode>(mode), intent);
    return err;
}

bool ComposerClient::CommandReader::parsePresentDisplay(uint16_t length) {
    if (length != CommandWriterBase::kPresentDisplayLength) {
        return false;
    }

    int presentFence = -1;
    std::vector<Layer> layers;
    std::vector<int> fences;
    auto err = mHal.presentDisplay(mDisplay, &presentFence, &layers, &fences);

    if (err == Error::NONE) {
        mWriter.setPresentFence(presentFence);
        mWriter.setReleaseFences(layers, fences);
    } else {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerCursorPosition(
    uint16_t length) {
    if (length != CommandWriterBase::kSetLayerCursorPositionLength) {
        return false;
    }

    auto err = mHal.setLayerCursorPosition(mDisplay, mLayer,
                                           readSigned(), readSigned());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerBuffer(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerBufferLength) {
        return false;
    }

    bool useCache = false;
    auto slot = read();
    auto buffer = readHandle(&useCache);
    auto fence = readFence();
    bool closeFence = true;
    auto err = lookupBuffer(BufferCache::LAYER_BUFFERS,
                            slot, useCache, buffer, &buffer);

    if (err == Error::NONE) {
        err = mHal.setLayerBuffer(mDisplay, mLayer, buffer, fence);
        auto updateBufErr = updateBuffer(BufferCache::LAYER_BUFFERS, slot,
                                         useCache, buffer);

        if (err == Error::NONE) {
            closeFence = false;
            err = updateBufErr;
        }
    }

    if (closeFence) {
        close(fence);
    }

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerSurfaceDamage(
    uint16_t length) {
    // N rectangles
    if (length % 4 != 0) {
        return false;
    }

    auto damage = readRegion(length / 4);
    auto err = mHal.setLayerSurfaceDamage(mDisplay, mLayer, damage);

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

Return<Error> ComposerClient::setDisplayedContentSamplingEnabled(
    [[maybe_unused]] uint64_t display,
    [[maybe_unused]] DisplayedContentSampling enable,
    [[maybe_unused]] hidl_bitfield<FormatColorComponent> componentMask,
    [[maybe_unused]] uint64_t maxFrames) {
    // TODO implement
    return Error::UNSUPPORTED;
}

Return<void> ComposerClient::getDisplayedContentSamplingAttributes(
    [[maybe_unused]] uint64_t display,
    [[maybe_unused]] getDisplayedContentSamplingAttributes_cb _hidl_cb) {
    // TODO implement
    _hidl_cb(Error::UNSUPPORTED, {}, {}, {});
    return Void();
}

bool ComposerClient::CommandReader::parseSetLayerBlendMode(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerBlendModeLength) {
        return false;
    }

    auto err = mHal.setLayerBlendMode(mDisplay, mLayer, readSigned());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerColor(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerColorLength) {
        return false;
    }

    auto err = mHal.setLayerColor(mDisplay, mLayer, readColor());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerCompositionType(
    uint16_t length) {
    if (length != CommandWriterBase::kSetLayerCompositionTypeLength) {
        return false;
    }

    auto err = mHal.setLayerCompositionType(mDisplay, mLayer, readSigned());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerDataspace(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerDataspaceLength) {
        return false;
    }

    auto err = mHal.setLayerDataspace(mDisplay, mLayer, readSigned());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerDisplayFrame(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerDisplayFrameLength) {
        return false;
    }

    auto err = mHal.setLayerDisplayFrame(mDisplay, mLayer, readRect());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerPlaneAlpha(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerPlaneAlphaLength) {
        return false;
    }

    auto err = mHal.setLayerPlaneAlpha(mDisplay, mLayer, readFloat());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerSidebandStream(
    uint16_t length) {
    if (length != CommandWriterBase::kSetLayerSidebandStreamLength) {
        return false;
    }

    auto stream = readHandle();
    auto err = lookupLayerSidebandStream(stream, &stream);

    if (err == Error::NONE) {
        err = mHal.setLayerSidebandStream(mDisplay, mLayer, stream);
        auto updateErr = updateLayerSidebandStream(stream);

        if (err == Error::NONE) {
            err = updateErr;
        }
    }

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerSourceCrop(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerSourceCropLength) {
        return false;
    }

    auto err = mHal.setLayerSourceCrop(mDisplay, mLayer, readFRect());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerTransform(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerTransformLength) {
        return false;
    }

    auto err = mHal.setLayerTransform(mDisplay, mLayer, readSigned());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerVisibleRegion(
    uint16_t length) {
    // N rectangles
    if (length % 4 != 0) {
        return false;
    }

    auto region = readRegion(length / 4);
    auto err = mHal.setLayerVisibleRegion(mDisplay, mLayer, region);

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

bool ComposerClient::CommandReader::parseSetLayerZOrder(uint16_t length) {
    if (length != CommandWriterBase::kSetLayerZOrderLength) {
        return false;
    }

    auto err = mHal.setLayerZOrder(mDisplay, mLayer, read());

    if (err != Error::NONE) {
        mWriter.setError(getCommandLoc(), err);
    }

    return true;
}

hwc_rect_t ComposerClient::CommandReader::readRect() {
    return hwc_rect_t{
        readSigned(),
        readSigned(),
        readSigned(),
        readSigned(),
    };
}

std::vector<hwc_rect_t> ComposerClient::CommandReader::readRegion(
    size_t count) {
    std::vector<hwc_rect_t> region;
    region.reserve(count);

    while (count > 0) {
        region.emplace_back(readRect());
        count--;
    }

    return region;
}

hwc_frect_t ComposerClient::CommandReader::readFRect() {
    return hwc_frect_t{
        readFloat(),
        readFloat(),
        readFloat(),
        readFloat(),
    };
}

Error ComposerClient::CommandReader::lookupBufferCacheEntryLocked(
    BufferCache cache, uint32_t slot, BufferCacheEntry** outEntry) {
    auto dpy = mClient.mDisplayData.find(mDisplay);

    if (dpy == mClient.mDisplayData.end()) {
        return Error::BAD_DISPLAY;
    }

    BufferCacheEntry* entry = nullptr;

    switch (cache) {
    case BufferCache::CLIENT_TARGETS:
        if (slot < dpy->second.ClientTargets.size()) {
            entry = &dpy->second.ClientTargets[slot];
        }

        break;

    case BufferCache::OUTPUT_BUFFERS:
        if (slot < dpy->second.OutputBuffers.size()) {
            entry = &dpy->second.OutputBuffers[slot];
        }

        break;

    case BufferCache::LAYER_BUFFERS: {
        auto ly = dpy->second.Layers.find(mLayer);

        if (ly == dpy->second.Layers.end()) {
            return Error::BAD_LAYER;
        }

        if (slot < ly->second.Buffers.size()) {
            entry = &ly->second.Buffers[slot];
        }
    }
    break;

    case BufferCache::LAYER_SIDEBAND_STREAMS: {
        auto ly = dpy->second.Layers.find(mLayer);

        if (ly == dpy->second.Layers.end()) {
            return Error::BAD_LAYER;
        }

        if (slot == 0) {
            entry = &ly->second.SidebandStream;
        }
    }
    break;

    default:
        break;
    }

    if (!entry) {
        ALOGW("invalid buffer slot %" PRIu32, slot);
        return Error::BAD_PARAMETER;
    }

    *outEntry = entry;
    return Error::NONE;
}

Error ComposerClient::CommandReader::lookupBuffer(BufferCache cache,
        uint32_t slot, bool useCache, buffer_handle_t handle,
        buffer_handle_t* outHandle) {
    if (useCache) {
        std::lock_guard<std::mutex> lock(mClient.mDisplayDataMutex);
        BufferCacheEntry* entry;
        Error error = lookupBufferCacheEntryLocked(cache, slot, &entry);

        if (error != Error::NONE) {
            return error;
        }

        // input handle is ignored
        *outHandle = entry->getHandle();
    } else {
        if (!sHandleImporter.importBuffer(handle)) {
            return Error::NO_RESOURCES;
        }

        *outHandle = handle;
    }

    return Error::NONE;
}

Error ComposerClient::CommandReader::updateBuffer(BufferCache cache,
        uint32_t slot, bool useCache, buffer_handle_t handle) {
    // handle was looked up from cache
    if (useCache) {
        return Error::NONE;
    }

    std::lock_guard<std::mutex> lock(mClient.mDisplayDataMutex);
    BufferCacheEntry* entry = nullptr;
    Error error = lookupBufferCacheEntryLocked(cache, slot, &entry);

    if (error != Error::NONE) {
        return error;
    }

    *entry = handle;
    return Error::NONE;
}

Return<void> ComposerClient::getReadbackBufferAttributes(
    uint64_t display, getReadbackBufferAttributes_cb _hidl_cb) {
    Error err = mHal.isDisplayValid(display) ? Error::NONE : Error::BAD_DISPLAY;

    // RGB_565 also supported
    _hidl_cb(err, PixelFormat_V1_1::BGRA_8888, Dataspace_V1_1::RANGE_FULL);
    return Void();
}

Return<void> ComposerClient::getReadbackBufferFence(
        uint64_t display, getReadbackBufferFence_cb _hidl_cb) {
    if (!mHal.isDisplayValid(display)) {
        _hidl_cb(Error::BAD_DISPLAY, {});
        return Void();
    }

    if (!mHal.isReadbackBufferSet()) {
        _hidl_cb(Error::UNSUPPORTED, {});
        return Void();
    }

    auto nh = native_handle_create(/* numFds */1, /* numInts*/0);
    // no fence for readback buffer
    nh->data[0] = -1;

    // takes ownership
    hidl_handle handle;
    handle.setTo(nh, true);
    _hidl_cb(Error::NONE, handle);
    return Void();
}

Return<Error> ComposerClient::setReadbackBuffer(
        uint64_t display, const hidl_handle& buffer,
    const hidl_handle& releaseFence) {
    if (!mHal.isDisplayValid(display)) {
        return Error::BAD_DISPLAY;
    }

    auto fenceNativeHandle = releaseFence.getNativeHandle();

    if (fenceNativeHandle && fenceNativeHandle->numFds) {
        return mHal.setReadbackBuffer(display, buffer.getNativeHandle(),
                                      fenceNativeHandle->data[0]);
    } else {
        return mHal.setReadbackBuffer(display, buffer.getNativeHandle(), -1);
    }
}

} // namespace implementation
} // namespace V2_3
} // namespace composer
} // namespace graphics
} // namespace hardware
} // namespace android
