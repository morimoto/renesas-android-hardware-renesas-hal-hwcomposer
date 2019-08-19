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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_COMPOSER_CLIENT_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_COMPOSER_CLIENT_H

#include "IComposerCommandBuffer.h"
#include "ComposerBase.h"
#include <hardware/hwcomposer2.h>

#include <mutex>
#include <unordered_map>
#include <vector>
#include <android/hardware/graphics/composer/2.3/IComposerClient.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android::hardware::graphics::composer::V2_3::implementation {

using IComposerCallback
    = ::android::hardware::graphics::composer::V2_1::IComposerCallback;
using Error = ::android::hardware::graphics::composer::V2_1::Error;
using RenderIntent = ::android::hardware::graphics::common::V1_1::RenderIntent;
using Vsync
    = ::android::hardware::graphics::composer::V2_1::IComposerClient::Vsync;
using Attribute
    = ::android::hardware::graphics::composer::V2_1::IComposerClient::Attribute;
using DisplayedContentSampling
    = ::android::hardware::graphics::composer::V2_3::IComposerClient::DisplayedContentSampling;

using PixelFormat_V1_0
    = ::android::hardware::graphics::common::V1_0::PixelFormat;
using PixelFormat_V1_1
    = ::android::hardware::graphics::common::V1_1::PixelFormat;
using PixelFormat_V1_2
    = ::android::hardware::graphics::common::V1_2::PixelFormat;

using ColorMode_V1_0 = ::android::hardware::graphics::common::V1_0::ColorMode;
using ColorMode_V1_1 = ::android::hardware::graphics::common::V1_1::ColorMode;
using ColorMode_V1_2 = ::android::hardware::graphics::common::V1_2::ColorMode;

using Dataspace_V1_0 = ::android::hardware::graphics::common::V1_0::Dataspace;
using Dataspace_V1_1 = ::android::hardware::graphics::common::V1_1::Dataspace;
using Dataspace_V1_2 = ::android::hardware::graphics::common::V1_2::Dataspace;

using PowerMode_V2_1
    = ::android::hardware::graphics::composer::V2_1::IComposerClient::PowerMode;
using PowerMode_V2_2
    = ::android::hardware::graphics::composer::V2_2::IComposerClient::PowerMode;

class BufferCacheEntry {
public:
    BufferCacheEntry();
    BufferCacheEntry(BufferCacheEntry&& other);

    BufferCacheEntry(const BufferCacheEntry& other) = delete;
    BufferCacheEntry& operator=(const BufferCacheEntry& other) = delete;

    BufferCacheEntry& operator=(buffer_handle_t handle);
    ~BufferCacheEntry();

    buffer_handle_t getHandle() const { return mHandle; }

private:
    void clear();

    buffer_handle_t mHandle;
};

class ComposerClient : public IComposerClient {
public:
    ComposerClient(ComposerBase& hal);
    virtual ~ComposerClient();

    void initialize();

    void onHotplug(Display display, IComposerCallback::Connection connected);
    void onRefresh(Display display);
    void onVsync(Display display, int64_t timestamp);

    // ...::V2_1::IComposerClient
    Return<void> registerCallback(
        const sp<IComposerCallback>& callback) override;
    Return<uint32_t> getMaxVirtualDisplayCount() override;
    Return<void> createVirtualDisplay(uint32_t width, uint32_t height,
                                      PixelFormat_V1_0 formatHint,
                                      uint32_t outputBufferSlotCount,
                                      createVirtualDisplay_cb hidl_cb) override;
    Return<Error> destroyVirtualDisplay(Display display) override;
    Return<void> createLayer(Display display, uint32_t bufferSlotCount,
                             createLayer_cb hidl_cb) override;
    Return<Error> destroyLayer(Display display, Layer layer) override;
    Return<void> getActiveConfig(Display display,
                                 getActiveConfig_cb hidl_cb) override;
    Return<Error> getClientTargetSupport(Display display,
                                         uint32_t width, uint32_t height,
                                         PixelFormat_V1_0 format,
                                         Dataspace_V1_0 dataspace) override;
    Return<void> getColorModes(Display display,
                               getColorModes_cb hidl_cb) override;
    Return<void> getDisplayAttribute(Display display,
                                     Config config, Attribute attribute,
                                     getDisplayAttribute_cb hidl_cb) override;
    Return<void> getDisplayConfigs(Display display,
                                   getDisplayConfigs_cb hidl_cb) override;
    Return<void> getDisplayName(Display display,
                                getDisplayName_cb hidl_cb) override;
    Return<void> getDisplayType(Display display,
                                getDisplayType_cb hidl_cb) override;
    Return<void> getDozeSupport(Display display,
                                getDozeSupport_cb hidl_cb) override;
    Return<void> getHdrCapabilities(Display display,
                                    getHdrCapabilities_cb hidl_cb) override;
    Return<Error> setActiveConfig(Display display, Config config) override;
    Return<Error> setColorMode(Display display, ColorMode_V1_0 mode) override;
    Return<Error> setPowerMode(Display display, PowerMode_V2_1 mode) override;
    Return<Error> setVsyncEnabled(Display display, Vsync enabled) override;
    Return<Error> setClientTargetSlotCount(Display display,
                                           uint32_t clientTargetSlotCount) override;
    Return<Error> setInputCommandQueue(
        const MQDescriptorSync<uint32_t>& descriptor) override;
    Return<void> getOutputCommandQueue(
        getOutputCommandQueue_cb hidl_cb) override;
    Return<void> executeCommands(uint32_t inLength,
                                 const hidl_vec<hidl_handle>& inHandles,
                                 executeCommands_cb hidl_cb) override;

    // ...::V2_2::IComposerClient
    Return<void> getPerFrameMetadataKeys(uint64_t display,
                                         getPerFrameMetadataKeys_cb _hidl_cb) override;

    Return<void> getReadbackBufferAttributes(uint64_t display,
            getReadbackBufferAttributes_cb _hidl_cb) override;

    Return<Error> setReadbackBuffer(uint64_t display,
                                    const hidl_handle& buffer,
                                    const hidl_handle& releaseFence) override;

    Return<void> createVirtualDisplay_2_2(uint32_t width,
                                          uint32_t height,
                                          PixelFormat_V1_1 formatHint,
                                          uint32_t outputBufferSlotCount,
                                          createVirtualDisplay_2_2_cb _hidl_cb) override;

    Return<Error> getClientTargetSupport_2_2(uint64_t display,
            uint32_t width, uint32_t height,
            PixelFormat_V1_1 format, Dataspace_V1_1 dataspace) override;

    Return<Error> setPowerMode_2_2(uint64_t display, PowerMode_V2_2 mode) override;
    Return<void> getColorModes_2_2(uint64_t display,
                                   getColorModes_2_2_cb _hidl_cb) override;
    Return<void> getRenderIntents(uint64_t display, ColorMode_V1_1 mode,
                                  getRenderIntents_cb _hidl_cb) override;
    Return<Error> setColorMode_2_2(uint64_t display, ColorMode_V1_1 mode,
                                   RenderIntent intent) override;
    Return<void> getDataspaceSaturationMatrix(Dataspace_V1_1 dataspace,
            getDataspaceSaturationMatrix_cb _hidl_cb) override;
    Return<void> executeCommands_2_2(uint32_t inLength,
                                     const hidl_vec<hidl_handle>& inHandles,
                                     executeCommands_2_2_cb _hidl_cb) override;

    // ...::V2_3::IComposerClient
    Return<void> getDisplayIdentificationData(uint64_t display,
            getDisplayIdentificationData_cb _hidl_cb) override;
    Return<void> getReadbackBufferAttributes_2_3(uint64_t display,
            getReadbackBufferAttributes_2_3_cb _hidl_cb) override;
    Return<Error> getClientTargetSupport_2_3(uint64_t display,
            uint32_t width, uint32_t height,
            PixelFormat_V1_2 format, Dataspace_V1_2 dataspace) override;
    Return<void> getDisplayedContentSamplingAttributes(uint64_t display,
            getDisplayedContentSamplingAttributes_cb _hidl_cb) override;
    Return<Error> setDisplayedContentSamplingEnabled(uint64_t display,
            DisplayedContentSampling enable,
            hidl_bitfield<FormatColorComponent> componentMask,
            uint64_t maxFrames) override;
    Return<void> getDisplayedContentSample(uint64_t display,
                                           uint64_t maxFrames, uint64_t timestamp,
                                           getDisplayedContentSample_cb _hidl_cb) override;
    Return<void> executeCommands_2_3(uint32_t inLength,
                                     const hidl_vec<hidl_handle>& inHandles,
                                     executeCommands_2_3_cb _hidl_cb) override;
    Return<void> getRenderIntents_2_3(uint64_t display, ColorMode_V1_2 mode,
                                      getRenderIntents_2_3_cb _hidl_cb) override;
    Return<void> getColorModes_2_3(uint64_t display,
                                   getColorModes_2_3_cb _hidl_cb) override;
    Return<Error> setColorMode_2_3(uint64_t display,
                                   ColorMode_V1_2 mode, RenderIntent intent) override;
    Return<void> getDisplayCapabilities(uint64_t display,
                                        getDisplayCapabilities_cb _hidl_cb) override;
    Return<void> getPerFrameMetadataKeys_2_3(uint64_t display,
            getPerFrameMetadataKeys_2_3_cb _hidl_cb) override;
    Return<void> getHdrCapabilities_2_3(uint64_t display,
                                        getHdrCapabilities_2_3_cb _hidl_cb) override;
    Return<void> getDisplayBrightnessSupport(uint64_t display,
            getDisplayBrightnessSupport_cb _hidl_cb) override;
    Return<Error> setDisplayBrightness(uint64_t display,
                                       float brightness) override;
    Return<void> getReadbackBufferFence(uint64_t display,
                                        getReadbackBufferFence_cb _hidl_cb) override;

protected:
    struct LayerBuffers {
        std::vector<BufferCacheEntry> Buffers;
        BufferCacheEntry SidebandStream;
    };

    struct DisplayData {
        bool IsVirtual;

        std::vector<BufferCacheEntry> ClientTargets;
        std::vector<BufferCacheEntry> OutputBuffers;

        std::unordered_map<Layer, LayerBuffers> Layers;

        DisplayData(bool isVirtual) : IsVirtual(isVirtual) {}
    };

    class CommandReader : public CommandReaderBase {
    public:
        CommandReader(ComposerClient& client);
        virtual ~CommandReader();

        Error parse();

    protected:
        virtual bool parseCommand(IComposerClient::Command command,
                                  uint16_t length);

        bool parseSetLayerColorTransform(uint16_t length);
        bool parseSetLayerPerFrameMetadata(uint16_t length);
        bool parseSetLayerFloatColor(uint16_t length);
        bool parseSetLayerPerFrameMetadataBlobs(uint16_t length);
        bool parseSelectDisplay(uint16_t length);
        bool parseSelectLayer(uint16_t length);
        bool parseSetColorTransform(uint16_t length);
        bool parseSetClientTarget(uint16_t length);
        bool parseSetOutputBuffer(uint16_t length);
        bool parseValidateDisplay(uint16_t length);
        bool parsePresentOrValidateDisplay(uint16_t length);
        bool parseAcceptDisplayChanges(uint16_t length);
        bool parsePresentDisplay(uint16_t length);
        bool parseSetLayerCursorPosition(uint16_t length);
        bool parseSetLayerBuffer(uint16_t length);
        bool parseSetLayerSurfaceDamage(uint16_t length);
        bool parseSetLayerBlendMode(uint16_t length);
        bool parseSetLayerColor(uint16_t length);
        bool parseSetLayerCompositionType(uint16_t length);
        bool parseSetLayerDataspace(uint16_t length);
        bool parseSetLayerDisplayFrame(uint16_t length);
        bool parseSetLayerPlaneAlpha(uint16_t length);
        bool parseSetLayerSidebandStream(uint16_t length);
        bool parseSetLayerSourceCrop(uint16_t length);
        bool parseSetLayerTransform(uint16_t length);
        bool parseSetLayerVisibleRegion(uint16_t length);
        bool parseSetLayerZOrder(uint16_t length);

        hwc_rect_t readRect();
        std::vector<hwc_rect_t> readRegion(size_t count);
        hwc_frect_t readFRect();

        enum class BufferCache {
            CLIENT_TARGETS,
            OUTPUT_BUFFERS,
            LAYER_BUFFERS,
            LAYER_SIDEBAND_STREAMS,
        };
        Error lookupBufferCacheEntryLocked(BufferCache cache, uint32_t slot,
                                           BufferCacheEntry** outEntry);
        Error lookupBuffer(BufferCache cache, uint32_t slot,
                           bool useCache, buffer_handle_t handle,
                           buffer_handle_t* outHandle);
        Error updateBuffer(BufferCache cache, uint32_t slot,
                           bool useCache, buffer_handle_t handle);

        Error lookupLayerSidebandStream(buffer_handle_t handle,
                                        buffer_handle_t* outHandle) {
            return lookupBuffer(BufferCache::LAYER_SIDEBAND_STREAMS,
                                0, false, handle, outHandle);
        }
        Error updateLayerSidebandStream(buffer_handle_t handle) {
            return updateBuffer(BufferCache::LAYER_SIDEBAND_STREAMS,
                                0, false, handle);
        }

        ComposerClient& mClient;
        ComposerBase& mHal;
        CommandWriterBase& mWriter;

        Display mDisplay;
        Layer mLayer;
    };

    virtual std::unique_ptr<CommandReader> createCommandReader();

    ComposerBase& mHal;

    // 64KiB minus a small space for metadata such as read/write pointers
    static constexpr size_t kWriterInitialSize =
        64 * 1024 / sizeof(uint32_t) - 16;
    std::mutex mCommandMutex;
    std::unique_ptr<CommandReader> mReader;
    CommandWriterBase mWriter;

    sp<IComposerCallback> mCallback;

    std::mutex mDisplayDataMutex;
    std::unordered_map<Display, DisplayData> mDisplayData;
};

} // android::hardware::graphics::composer::V2_3::implementation

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_COMPOSER_CLIENT_H
