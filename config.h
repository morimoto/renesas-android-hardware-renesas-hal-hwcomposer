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

#if defined(TARGET_BOARD_SALVATOR_H3)

#define VGA_ENCODER_ID 50
#define VGA_CONNECTOR_ID 51

#define HDMI_A_1_ENCODER_ID 52
#define HDMI_A_1_CONNECTOR_ID 53

#define HDMI_A_2_ENCODER_ID 54
#define HDMI_A_2_CONNECTOR_ID 55

struct hwdisplay {
    int encoder_id;
    int connector_id;
    char status[50];
    char bootargs[50];
};

static const hwdisplay hwdisplays[3] = {
        // primary display
        VGA_ENCODER_ID,
        VGA_CONNECTOR_ID,
        "/sys/class/drm/card0-VGA-1/status",
        "video=VGA-1:",

        // external display 1
        HDMI_A_1_ENCODER_ID,
        HDMI_A_1_CONNECTOR_ID,
        "/sys/class/drm/card0-HDMI-A-1/status",
        "video=HDMI-A-1:",

        // external display 2
        HDMI_A_2_ENCODER_ID,
        HDMI_A_2_CONNECTOR_ID,
        "/sys/class/drm/card0-HDMI-A-2/status",
        "video=HDMI-A-2:",
};

#endif


#endif
