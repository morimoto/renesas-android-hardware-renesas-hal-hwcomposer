/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_HWC_DUMP_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_HWC_DUMP_H

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <cerrno>
#include <libgen.h>
#include <cstring>
#include <log/log.h>

namespace android {

void debug_error(const char* format, const char* file, int line
                 , const char* func, const char* call, const char* msg);

#define CHECK_RES_FATAL(ret)                                    \
    do {                                                        \
        int loc_ret = (ret);                                    \
        if (loc_ret < 0) {                                      \
            debug_error("ERROR at [%s:%d] in [%s()]; [%s] fail error: %s" \
            , __FILE__, __LINE__, __func__, #ret, strerror(-loc_ret));\
            return -1;                                          \
        }                                                       \
    } while (0)

#define CHECK_RES_WARN(ret)                                     \
    do {                                                        \
        int loc_ret = (ret);                                    \
        if (loc_ret < 0) {                                      \
            debug_error("WARNING at [%s:%d] in [%s()]; [%s] fail error: %s" \
            , __FILE__, __LINE__, __func__, #ret, strerror(-loc_ret));\
        }                                                       \
    } while (0)

void dump_crtc_props(int gfx_fd, uint32_t crtc_id);
void dump_plane_props(int gfx_fd, uint32_t plane_id);
void dump_conn_props(int gfx_fd, uint32_t conn_id);
void dump_connectors(int gfx_fd, drmModeRes* resources);
void dump_crtcs(int gfx_fd, drmModeRes* resources);
void dump_planes(int gfx_fd);
void dump_device_name(int gfx_fd);
void dump_all(int gfx_fd);

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_HWC_DUMP_H
