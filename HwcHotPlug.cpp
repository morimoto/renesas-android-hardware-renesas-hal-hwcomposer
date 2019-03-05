#include  "HwcHotPlug.h"

#include <fstream>
#include <thread>
#include <chrono>

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <poll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/types.h>

#define DRM_HOTPLUG_EVENT_SIZE 256

namespace android {
namespace hardware {
namespace graphics {
namespace composer {
namespace V2_1 {
namespace implementation {

HotPlug::HotPlug() : android::Worker("Hotplug", HAL_PRIORITY_URGENT_DISPLAY) {
}

HotPlug& HotPlug::getInstance() {
    static HotPlug hotPlug;
    return hotPlug;
}

void HotPlug::startHotPlugMonitor() {
    if (initWorker()) {
        ALOGE("Failed to initalize worker for Hotplug events.");
    }
}

void HotPlug::routine() {
    pollfd req;
    req.fd = mHotplugFd.get();
    req.events = POLLIN;
    if (poll(&req, 1, -1) > 0) {
        if (req.revents & POLLIN) {
            hotPlugEventHandler();
        }
    } else {
        ALOGE("poll fail");
    }
}

void HotPlug::hotPlugEventHandler() {
    int fd = mHotplugFd.get();
    char buffer[DRM_HOTPLUG_EVENT_SIZE];
    int ret;
    memset(&buffer, 0, sizeof(buffer));

    while (true) {
        bool drm_event = false, hotplug_event = false;
        ret = read(fd, &buffer, DRM_HOTPLUG_EVENT_SIZE - 1);

        if (ret <= 0) {
            return;
        }

        buffer[ret] = '\0';

        for (int32_t i = 0; i < ret;) {
            char* event = buffer + i;

            if (!strcmp(event, "DEVTYPE=drm_minor")) {
                drm_event = true;
            } else if (!strcmp(event, "HOTPLUG=1")
                       || !strcmp(event, "HDMI-Change")) {
                hotplug_event = true;
            }

            if (hotplug_event && drm_event)
                break;

            i += strlen(event) + 1;
        }

        if (drm_event && hotplug_event) {
            ALOGD("Recieved Hot Plug event");
            hookEventHotPlug();
        }
    }
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

Error HotPlug::loadDisplayConfiguration(size_t display) {
    if (!mConnectDisplays[display].isReInit) {
        hwc2_display_t type = static_cast<hwc2_display_t>(mHwc->mDisplays.size());
        mHwc->mDisplays.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(type),
                    std::forward_as_tuple(mHwc->mDrmFd, type, hwdisplays[display],
                                          HWC2::DisplayType::Physical,
                                          mHwc->mImporter));
        if (mHwc->mDisplays.at(type).init() != Error::NONE) {
            mHwc->mDisplays.erase(type);
            return Error::BAD_CONFIG;
        }
        mConnectDisplays[display].displayType = type;
        mConnectDisplays[display].isReInit = true;
        mHwc->mInitDisplays = true;
    } else {
        mHwc->mDisplays.at(mConnectDisplays[display].displayType).loadNewConfig();
    }
    return Error::NONE;
}

void HotPlug::hookEventHotPlug() {
    for (int display = 0; display < NUM_DISPLAYS; ++display) {
        const char currentStatusDisplay = getStatusDisplay(display);
        if (currentStatusDisplay == DISCONNECTED_DISPLAY
            && mConnectDisplays[display].isConnected) {
            if (mConnectDisplays[display].displayType == HWC_DISPLAY_PRIMARY) {
                continue;
            }
            HwcHal::hotplugHook(mHwc, mConnectDisplays[display].displayType,
                                static_cast<int32_t>(HWC2::Connection::Disconnected));
            mConnectDisplays[display].isConnected = false;
        } else if (currentStatusDisplay == CONNECTED_DISPLAY
                   && !mConnectDisplays[display].isConnected) {
            if (loadDisplayConfiguration(display) != Error::NONE) {
                continue;
            }
            HwcHal::hotplugHook(mHwc, mConnectDisplays[display].displayType,
                                static_cast<int32_t>(HWC2::Connection::Connected));
            mConnectDisplays[display].isConnected = true;
        }
    }
}

void HotPlug::initDisplays(HwcHal* hwc) {
    mHwc = hwc;

    mHotplugFd.set(socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT));
    if (mHotplugFd.get() < 0) {
        ALOGE("Failed to create socket for hotplug monitor");
    }
    sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;
    int ret = bind(mHotplugFd.get(), (sockaddr*)&addr, sizeof(addr));
    if (ret) {
        ALOGE("Failed to bind sockaddr and hotplug monitor");
    }

    while (!mHwc->mInitDisplays) {
        hookEventHotPlug();
    }
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace composer
}  // namespace graphics
}  // namespace hardware
}  // namespace android
