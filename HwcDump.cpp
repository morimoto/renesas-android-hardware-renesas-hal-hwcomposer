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

#include "HwcDump.h"

#include <log/log.h>

namespace android {

void debug_error(const char* format, const char* file, int line
                 , const char* func, const char* call, const char* msg) {
    ALOGE(format, basename(file), line, func, call, msg);
}

const char* obj_name(uint32_t obj_type) {
    switch (obj_type) {
    case DRM_MODE_OBJECT_PLANE:
        return "plane";
        break;

    case DRM_MODE_OBJECT_CRTC:
        return "crtc";
        break;

    case DRM_MODE_OBJECT_CONNECTOR:
        return "connector";
        break;

    default:
        return "[unknown object]";
        break;
    }
}

void dump_mode(drmModeModeInfo* mode) {
    if (!mode) return;

    //%d %d %d %d %d %d
    ALOGD("  %12s %d %4d  %4d %4x %4x %d",
          mode->name, mode->vrefresh, mode->hdisplay,
          //mode->hsync_start, mode->hsync_end, mode->htotal,
          mode->vdisplay,
          //mode->vsync_start, mode->vsync_end, mode->vtotal,
          mode->flags, mode->type, mode->clock);
}

void dump_obj_props(int gfx_fd, uint32_t obj_id, uint32_t obj_type) {
    drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(gfx_fd, obj_id,
                                       obj_type);

    if (!props) {
        ALOGE("failed to get %s %d properties", obj_name(obj_type), obj_id);
        return;
    }

    ALOGD("%s %d props num: %d", obj_name(obj_type), obj_id, props->count_props);

    for (uint32_t j = 0; j < props->count_props; j++) {
        drmModePropertyPtr p = drmModeGetProperty(gfx_fd, props->props[j]);
        ALOGD("    %s id: %d", p->name, p->prop_id);

        for (int k = 0; k < p->count_values; ++k) {
            ALOGD("         %zu", p->values[k]);
        }

        drmModeFreeProperty(p);
    }

    drmModeFreeObjectProperties(props);
}

void dump_crtc_props(int gfx_fd, uint32_t crtc_id) {
    if (!crtc_id) return;

    dump_obj_props(gfx_fd, crtc_id, DRM_MODE_OBJECT_CRTC);
}

void dump_plane_props(int gfx_fd, uint32_t plane_id) {
    if (!plane_id) return;

    dump_obj_props(gfx_fd, plane_id, DRM_MODE_OBJECT_PLANE);
}

void dump_conn_props(int gfx_fd, uint32_t conn_id) {
    if (!conn_id) return;

    dump_obj_props(gfx_fd, conn_id, DRM_MODE_OBJECT_CONNECTOR);
}

void dump_connectors(int gfx_fd, drmModeRes* resources) {
    if (!resources) return;

    ALOGD("Connectors =======================================");
    ALOGD("id\tenc\tsize\tmodes");

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector* connector;
        connector = drmModeGetConnector(gfx_fd, resources->connectors[i]);

        if (!connector) {
            ALOGD("could not get connector %i: %s",
                  resources->connectors[i], strerror(errno));
            continue;
        }

        ALOGD("%d\t%d\t%dx%d\t\t%d",
              connector->connector_id,
              connector->encoder_id,
              //status\t\ttype\t
              //%s\t%s\t
              //kmstest_connector_status_str(connector->connection),
              //kmstest_connector_type_str(connector->connector_type),
              connector->mmWidth, connector->mmHeight,
              connector->count_modes);

        if (!connector->count_modes)
            continue;

        ALOGD("  modes:");
        // hss hse htot vss vse vtot
        ALOGD("          name Hz hdisp vdisp flags type clock");

        for (int j = 0; j < connector->count_modes; j++)
            dump_mode(&connector->modes[j]);

        drmModeFreeConnector(connector);
    }

    ALOGD("==================================================");
}

void dump_crtcs(int gfx_fd, drmModeRes* resources) {
    if (!resources) return;

    int i;
    ALOGD("CRTCs ============================================");
    ALOGD("id\tfb\tpos\t\tsize");

    for (i = 0; i < resources->count_crtcs; i++) {
        drmModeCrtc* crtc;
        crtc = drmModeGetCrtc(gfx_fd, resources->crtcs[i]);

        if (!crtc) {
            ALOGD("could not get crtc %i: %s", resources->crtcs[i], strerror(errno));
            continue;
        }

        ALOGD("%d\t%d\t(%d,%d)\t(%dx%d)",
              crtc->crtc_id,
              crtc->buffer_id,
              crtc->x, crtc->y,
              crtc->width, crtc->height);
        dump_mode(&crtc->mode);
        drmModeFreeCrtc(crtc);
    }

    ALOGD("==================================================");
}

void dump_planes(int gfx_fd) {
    if (gfx_fd <= 0) return;

    drmModePlaneRes*             plane_resources;
    drmModePlane*                ovr;
    uint32_t i;
    plane_resources = drmModeGetPlaneResources(gfx_fd);

    if (!plane_resources) {
        ALOGD("drmModeGetPlaneResources failed: %s",
              strerror(errno));
        return;
    }

    ALOGD("Planes ===========================================");
    ALOGD("id\tpC\tC\tfb\tCx,Cy\tx,y\tgm"); // pC-possible_crtcs; C-crtc; gm-gamma_size

    for (i = 0; i < plane_resources->count_planes; i++) {
        ovr = drmModeGetPlane(gfx_fd, plane_resources->planes[i]);

        if (!ovr) {
            ALOGD("drmModeGetPlane failed: %s",
                  strerror(errno));
            continue;
        }

        ALOGD("%d\t%d\t%d\t%d\t%d,%d\t\t%d,%d\t%d",
              ovr->plane_id, ovr->possible_crtcs, ovr->crtc_id, ovr->fb_id,
              ovr->crtc_x, ovr->crtc_y, ovr->x, ovr->y,
              ovr->gamma_size);
        drmModeFreePlane(ovr);
    }

    ALOGD("==================================================");
    return;
}

void dump_device_name(int gfx_fd) {
    if (gfx_fd <= 0) return;

    drm_version_t version;
    char name[32];
    memset(&version, 0, sizeof(version));
    version.name_len = 4;
    version.name = name;

    if (!drmIoctl(gfx_fd, DRM_IOCTL_VERSION, &version)) {
        return;
    }

    ALOGD("DRM DEV NAME: %s", name);
}

void dump_all(int gfx_fd) {
    drmModeResPtr resources = drmModeGetResources(gfx_fd);

    if (gfx_fd <= 0 || !resources) return;

    dump_device_name(gfx_fd);
    dump_crtcs(gfx_fd, resources);
    dump_connectors(gfx_fd, resources);
    dump_planes(gfx_fd);
    dump_plane_props(gfx_fd, 0);
    dump_crtc_props(gfx_fd, 0);
    dump_conn_props(gfx_fd, 0);

    if (resources)
        drmModeFreeResources(resources);
}

} // namespace android
