#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_HOTPLUG_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_HOTPLUG_H

#include "Hwc.h"

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_1 {
namespace implementation {

class HotPlug {
private:
    struct DisplayInfo {
        bool isConnected = false;
        bool isProtected = true;
        bool isReInit = false;
        bool isSupportedConfig = true;
        hwc2_display_t displayType;
    } mConnectDisplays[NUM_DISPLAYS];

    enum Status {
        CONNECTED_DISPLAY = 'c',
        DISCONNECTED_DISPLAY = 'd',
        UNDEFINED = 'u'
    };

    HotPlug() = default;
    HotPlug(const HotPlug&)  = delete;
    HotPlug(const HotPlug&&) = delete;
    HotPlug& operator=(const HotPlug&)  = delete;
    HotPlug&& operator=(HotPlug&&) = delete;
public:
    static HotPlug& getInstance();
    Status getStatusDisplay(size_t display);
    Error loadDisplayConfiguration(HwcHal* hwc, size_t display);
    void initDisplays(HwcHal* hwc);
    void hookEventHotPlug(HwcHal* hwc, bool isRunHotPlug = false);
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace composer
}  // namespace graphics
}  // namespace hardware
}  // namespace android
#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_HOTPLUG_H

