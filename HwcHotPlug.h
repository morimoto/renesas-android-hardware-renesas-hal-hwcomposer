#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_HOTPLUG_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_HOTPLUG_H

#include "Hwc.h"
#include "autofd.h"

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_1 {
namespace implementation {

class HotPlug: public android::Worker {
private:
    struct DisplayInfo {
        bool isConnected = false;
        bool isReInit = false;
        hwc2_display_t displayType;
    } mConnectDisplays[NUM_DISPLAYS];

    enum Status {
        CONNECTED_DISPLAY = 'c',
        DISCONNECTED_DISPLAY = 'd',
        UNDEFINED = 'u'
    };

    HotPlug();
    ~HotPlug() = default;
    HotPlug(const HotPlug&)  = delete;
    HotPlug(const HotPlug&&) = delete;
    HotPlug& operator=(const HotPlug&)  = delete;
    HotPlug& operator=(HotPlug&&) = delete;

    Status getStatusDisplay(size_t display);
    Error loadDisplayConfiguration(size_t display);
    void hookEventHotPlug();
    void hotPlugEventHandler();

    void routine() override;

public:
    static HotPlug& getInstance();
    void initDisplays(HwcHal* hwc);
    void startHotPlugMonitor();

private:
    android::UniqueFd mHotplugFd;
    HwcHal* mHwc;
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace composer
}  // namespace graphics
}  // namespace hardware
}  // namespace android
#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_HOTPLUG_H

