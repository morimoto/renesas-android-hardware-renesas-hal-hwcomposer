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

#ifndef HWC_CONFIG_FUNCTION_SWITCH_H
#define HWC_CONFIG_FUNCTION_SWITCH_H

/***********************************
 define function switch
***********************************/

/* DEBUG_HWC
 * controlled whether the debug function is made effective.
 *  4   log level 4   log full debug information.
 *  3   log level 3
 *  2   log level 2
 *  1   log level 1.  minimum debug information.
 *  0   unavailable.
 */
#define DEBUG_HWC        0

#define USE_DBGLEVEL(X)   (DEBUG_HWC >= (X))


/* DEBUG_USE_ATRACE
 * controlled whether the debug function is made effective.
 *  1   effective.
 *  0   unavailable.
 */
#define DEBUG_USE_ATRACE     0

/* num of virtual display supported
 *  2   two display effective. (reserved.)
 *  1   one display effective.
 *  0   unavailable. all virtual display handled same as HWC1.2.
 */
#define NUM_OF_VIRTUALDISPLAY   1

/* support HWC V1.2
 *  1   hwc version 1.2 used.
 *  0   hwc version 1.1 used.
 */
#define USE_HWC_VERSION1_2   1

/* support HWC V1.3
 *  1   hwc version 1.3 used. necessary to set USE_HWC_VERSION1_2.
 *  0   -
 */
#define USE_HWC_VERSION1_3   0

#define NUM_DISPLAYS (3)

struct hwdisplay {
    char encoder[64];
    char connector[64];
    char status[64];
    char bootargs[64];
};

static const hwdisplay hwdisplays[NUM_DISPLAYS] = {
#if defined(THIRD_DISPLAY_SUPPORT)
    { // primary display
        "/sys/class/drm/card0-VGA-1/encoder_type",
        "/sys/class/drm/card0-VGA-1/connector_id",
        "/sys/class/drm/card0-VGA-1/status",
        "ro.boot.display.resolution.VGA"
    },
#endif

    { // external display 1
        "/sys/class/drm/card0-HDMI-A-1/encoder_type",
        "/sys/class/drm/card0-HDMI-A-1/connector_id",
        "/sys/class/drm/card0-HDMI-A-1/status",
        "ro.boot.display.res.HDMI1",
    },

#if defined(TARGET_BOARD_PLATFORM_R8A7795)
    { // external display 2
        "/sys/class/drm/card0-HDMI-A-2/encoder_type",
        "/sys/class/drm/card0-HDMI-A-2/connector_id",
        "/sys/class/drm/card0-HDMI-A-2/status",
        "ro.boot.display.res.HDMI2",
    }
#endif
};

#endif // HWC_CONFIG_FUNCTION_SWITCH_H
