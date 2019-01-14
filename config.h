/*
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_CONFIG_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_CONFIG_H

/***********************************
 define function switch
***********************************/

#define NUM_DISPLAYS (3)
/* DEBUG_FRAMERATE
 * report frame rate every second.
 *  1   effective.
 *  0   unavailable.
 */
#define DEBUG_FRAMERATE 1 // report frame rate if 1 specified

struct hwdisplay {
    char encoder[64];
    char connector[64];
    char status[64];
    char property[64];
};

static const hwdisplay hwdisplays[NUM_DISPLAYS] = {

#if defined(TARGET_BOARD_KINGFISHER)
    {
        // primary display
        "/sys/class/drm/card0-HDMI-A-2/encoder_type",
        "/sys/class/drm/card0-HDMI-A-2/connector_id",
        "/sys/class/drm/card0-HDMI-A-2/status",
        "ro.boot.display.res.HDMI2",
    },
    {
        // secondary display 1
        "/sys/class/drm/card0-HDMI-A-1/encoder_type",
        "/sys/class/drm/card0-HDMI-A-1/connector_id",
        "/sys/class/drm/card0-HDMI-A-1/status",
        "ro.boot.display.res.HDMI1"
    },
    {
        // external display 2
        "/sys/class/drm/card0-LVDS-1/encoder_type",
        "/sys/class/drm/card0-LVDS-1/connector_id",
        "/sys/class/drm/card0-LVDS-1/status",
        "ro.boot.display.res.LVDS",
    }
#else /* LAGER, KOELSCH, ALT, SALVATOR, ULCB*/
    {
        // primary display
        "/sys/class/drm/card0-VGA-1/encoder_type",
        "/sys/class/drm/card0-VGA-1/connector_id",
        "/sys/class/drm/card0-VGA-1/status",
        "ro.boot.display.res.VGA"
    },
    {
        // external display 1
        "/sys/class/drm/card0-HDMI-A-1/encoder_type",
        "/sys/class/drm/card0-HDMI-A-1/connector_id",
        "/sys/class/drm/card0-HDMI-A-1/status",
        "ro.boot.display.res.HDMI1",
    },
#if defined(TARGET_BOARD_PLATFORM_R8A7795)
    {
        // external display 2
        "/sys/class/drm/card0-HDMI-A-2/encoder_type",
        "/sys/class/drm/card0-HDMI-A-2/connector_id",
        "/sys/class/drm/card0-HDMI-A-2/status",
        "ro.boot.display.res.HDMI2",
    }
#endif
#endif /* defined(TARGET_BOARD_KINGFISHER) */
};

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_CONFIG_H
