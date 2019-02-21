#include  "HwcHotPlug.h"

#include <fstream>
#include <thread>
#include <chrono>

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_1 {
namespace implementation {

HotPlug& HotPlug::getInstance() {
    static HotPlug hotPlug;
    return hotPlug;
}

HotPlug::Status HotPlug::getStatusDisplay(size_t display) {
    std::ifstream inStatusDisplay(hwdisplays[display].status);

    if (!inStatusDisplay.is_open()) {
        return UNDEFINED;
    }

    static char currentStatusDisplay = UNDEFINED;
    inStatusDisplay >> currentStatusDisplay;
    inStatusDisplay.close();

    switch (currentStatusDisplay) {
    case CONNECTED_DISPLAY:
        return CONNECTED_DISPLAY;

    case DISCONNECTED_DISPLAY:
        return DISCONNECTED_DISPLAY;

    default:
        return UNDEFINED;
    }
}

Error HotPlug::loadDisplayConfiguration(HwcHal* hwc, size_t display) {
    if (!mConnectDisplays[display].isReInit) {
        hwc2_display_t type = static_cast<hwc2_display_t>(hwc->mDisplays.size());
        hwc->mDisplays.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(type),
            std::forward_as_tuple(hwc->mDrmFd, type, hwdisplays[display],
                                  HWC2::DisplayType::Physical,
                                  hwc->mImporter));
        if (hwc->mDisplays.at(type).init() != Error::NONE) {
            hwc->mDisplays.erase(type);
            return Error::BAD_CONFIG;
        }
        mConnectDisplays[display].displayType = type;
        mConnectDisplays[display].isReInit = true;
        hwc->mInitDisplays = true;
    } else {
        hwc->mDisplays.at(mConnectDisplays[display].displayType).loadNewConfig();
    }
    return Error::NONE;
}

void HotPlug::hookEventHotPlug(HwcHal* hwc, bool isRunHotPlug) {
    using namespace std::chrono_literals;

    if (isRunHotPlug) {
        hwc->mWaitForPresentDisplay.get_future().wait();
    }

    char currentStatusDisplay = DISCONNECTED_DISPLAY;

    while (true) {
        for (int display = 0; display < NUM_DISPLAYS; ++display) {
            currentStatusDisplay = getStatusDisplay(display);

            if (currentStatusDisplay == DISCONNECTED_DISPLAY
                && mConnectDisplays[display].isConnected) {
                if (mConnectDisplays[display].displayType == HWC_DISPLAY_PRIMARY) {
                    continue;
                }

                if (mConnectDisplays[display].isProtected) {
                    mConnectDisplays[display].isProtected = false;
                    continue;
                }

                HwcHal::hotplugHook(hwc, mConnectDisplays[display].displayType,
                                    static_cast<int32_t>(HWC2::Connection::Disconnected));
                mConnectDisplays[display].isConnected = false;
                mConnectDisplays[display].isSupportedConfig = true;
            } else if (currentStatusDisplay == CONNECTED_DISPLAY
                       && !mConnectDisplays[display].isConnected
                       && mConnectDisplays[display].isSupportedConfig) {
                if (loadDisplayConfiguration(hwc, display) != Error::NONE) {
                    mConnectDisplays[display].isSupportedConfig = false;
                    continue;
                }
                HwcHal::hotplugHook(hwc, mConnectDisplays[display].displayType,
                                    static_cast<int32_t>(HWC2::Connection::Connected));
                mConnectDisplays[display].isConnected = true;
                mConnectDisplays[display].isProtected = true;
            } else {
                mConnectDisplays[display].isProtected = true;
            }
        }

        if (hwc->mInitDisplays && !isRunHotPlug) {
            return;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1s));
    }
}

void HotPlug::initDisplays(HwcHal* hwc) {
    hookEventHotPlug(hwc);
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace composer
}  // namespace graphics
}  // namespace hardware
}  // namespace android
