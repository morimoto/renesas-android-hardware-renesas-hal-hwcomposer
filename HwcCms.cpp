// -----------------------------------------------------------------------------
#include "HwcCms.h"
#include "HwcDump.h"

#include "rcar_du_drm.h"

#include <log/log.h>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

namespace android {

// Structure
struct cmm_mem_t {
    rcar_du_cmm_buf  buf;
    void* user_virt_addr;
};

// Function
int cmm_alloc(int fd, cmm_mem_t* mem_info, unsigned long size);
void cmm_free(int fd, cmm_mem_t* mem_info);
int cmm_set_clu(int fd, uint32_t crtc_id, cmm_mem_t* clu);
int cmm_set_lut(int fd, uint32_t crtc_id, cmm_mem_t* lut);
int cmm_get_hgo(int fd, uint32_t crtc_id, cmm_mem_t* hgo);

//------------------------------------------------------------------------------
int cms_reset(int drm_fd, int crtc_id) {
    // set default LUT
    {
        cmm_mem_t table;
        CHECK_RES_FATAL(cmm_alloc(drm_fd, &table, CMM_LUT_NUM*4));
        uint32_t* ptr = (uint32_t*)table.user_virt_addr;
        for (int i = 0; i < CMM_LUT_NUM; ++i) {
            ptr[i] = ((i << 16) | (i << 8) | (i << 0));
        }
        cmm_set_lut(drm_fd, crtc_id, &table);
        cmm_free(drm_fd, &table);
    }

    // set default CLU
    {
        cmm_mem_t table;
        CHECK_RES_FATAL(cmm_alloc(drm_fd, &table, CMM_CLU_NUM*4));
        uint32_t* ptr = (uint32_t*)table.user_virt_addr;
        for (int i = 0; i < CMM_CLU_NUM; i++) {
            int index = i;

            int r = index % 17;
            r = (r << 20);
            if (r > (255 << 16))
                r = (255 << 16);

            index /= 17;
            int g = index % 17;
            g = (g << 12);
            if (g > (255 << 8))
                g = (255 << 8);

            index /= 17;
            int b = index % 17;
            b = (b << 4);
            if (b > (255 << 0))
                b = (255 << 0);

            ptr[i] = r | g | b;
        }
        cmm_set_clu(drm_fd, crtc_id, &table);
        cmm_free(drm_fd, &table);
    }

    return 0;
}

int cms_set_lut(int drm_fd, int crtc_id, const uint32_t* buff, uint32_t size) {
    cmm_mem_t table;
    CHECK_RES_FATAL(cmm_alloc(drm_fd, &table, size*4));
    uint32_t* ptr = (uint32_t*)table.user_virt_addr;
    memcpy(ptr, buff, size*4);
    cmm_set_lut(drm_fd, crtc_id, &table);
    cmm_free(drm_fd, &table);
    return 0;
}

int cms_set_clu(int drm_fd, int crtc_id, const uint32_t* buff, uint32_t size) {
    cmm_mem_t table;
    CHECK_RES_FATAL(cmm_alloc(drm_fd, &table, size*4));
    uint32_t* ptr = (uint32_t*)table.user_virt_addr;
    memcpy(ptr, buff, size*4);
    cmm_set_clu(drm_fd, crtc_id, &table);
    cmm_free(drm_fd, &table);
    return 0;
}

int cms_get_hgo(int drm_fd, int crtc_id, uint32_t* buff, uint32_t size) {
    // Create HGO table
    cmm_mem_t table;
    CHECK_RES_FATAL(cmm_alloc(drm_fd, &table, size*4));
    CHECK_RES_WARN(cmm_get_hgo(drm_fd, crtc_id, &table));
    uint32_t* ptr = (uint32_t*)table.user_virt_addr;
    memcpy(buff, ptr, size*4);
    cmm_free(drm_fd, &table);
    return 0;
}

//------------------------------------------------------------------------------
int du_cmm_tp_set_table(int drm_fd, uint32_t crtc_id, cmm_mem_t* tp_mem, uint32_t command, uint32_t done) {
    rcar_du_cmm_table table;
    table.user_data  = tp_mem->buf.handle;
    table.crtc_id    = crtc_id;
    table.buff_len   = (uint32_t)tp_mem->buf.size;
    table.buff       = tp_mem->buf.handle;

    rcar_du_cmm_event event;
    event.crtc_id   = crtc_id;
    event.event     = done;

    // Que CLU table
    CHECK_RES_FATAL(drmCommandWrite(drm_fd, command, &table, sizeof table));
    // Wait CLU table done
    CHECK_RES_WARN(drmCommandWriteRead(drm_fd, DRM_RCAR_DU_CMM_WAIT_EVENT, &event, sizeof event));
    unsigned long handle = event.callback_data;

    if ((handle != tp_mem->buf.handle) || (event.event != done)) {
        ALOGE("error: CLU event. event %u(get %u), addr 0x%x(get 0x%lx)",
                        done, event.event,
                        tp_mem->buf.handle, handle);
        return -1;
    }

    return 0;
}

int cmm_set_clu(int drm_fd, uint32_t crtc_id, cmm_mem_t* clu) {
    return du_cmm_tp_set_table(drm_fd, crtc_id, clu, DRM_RCAR_DU_CMM_SET_CLU, CMM_EVENT_CLU_DONE);
}

int cmm_set_lut(int drm_fd, uint32_t crtc_id, cmm_mem_t* lut) {
    return du_cmm_tp_set_table(drm_fd, crtc_id, lut, DRM_RCAR_DU_CMM_SET_LUT, CMM_EVENT_LUT_DONE);
}

int cmm_get_hgo(int drm_fd, uint32_t crtc_id, cmm_mem_t* hgo) {
    rcar_du_cmm_table hgo_table;
    hgo_table.crtc_id = crtc_id;
    hgo_table.buff = hgo->buf.handle;
    hgo_table.buff_len = hgo->buf.size;
    hgo_table.user_data = hgo->buf.handle;

    rcar_du_cmm_table* work_hgo_table = &hgo_table;
    int arg = crtc_id;
    CHECK_RES_FATAL(drmCommandWrite(drm_fd, DRM_RCAR_DU_CMM_START_HGO, &arg, sizeof arg));
    CHECK_RES_FATAL(drmCommandWrite(drm_fd, DRM_RCAR_DU_CMM_GET_HGO, work_hgo_table,
                                    sizeof hgo_table));

    rcar_du_cmm_event event;
    event.crtc_id = crtc_id;
    event.event   = CMM_EVENT_HGO_DONE;
    CHECK_RES_WARN(drmCommandWriteRead(drm_fd, DRM_RCAR_DU_CMM_WAIT_EVENT, &event, sizeof event));

    unsigned long handle = event.callback_data;
    if ((handle != hgo->buf.handle) || (event.event != CMM_EVENT_HGO_DONE)) {
        ALOGE("HGO event error. event %u(get %u), addr 0x%x(get 0x%lx)",
                        CMM_EVENT_HGO_DONE, event.event,
                        hgo->buf.handle, handle);
    }

    return 0;
}

int cmm_alloc(int fd, cmm_mem_t* mem_info,
                    unsigned long size) {
    // Create CMM color table
    rcar_du_cmm_buf cmm_buf;
    cmm_buf.size = size;
    CHECK_RES_FATAL(drmCommandWriteRead(fd, DRM_RCAR_DU_CMM_ALLOC,
                                        &cmm_buf, sizeof cmm_buf));

    // Map memory for user space
    void* map = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
               cmm_buf.mmap_offset);

    if (map == MAP_FAILED) {
        drmCommandWrite(fd, DRM_RCAR_DU_CMM_FREE, &cmm_buf, sizeof cmm_buf);
        return -1;
    }

    mem_info->buf = cmm_buf;
    mem_info->user_virt_addr = map;
    return 0;
}

void cmm_free(int fd, cmm_mem_t* mem_info) {
    // Relase memory mapping
    munmap(mem_info->user_virt_addr, mem_info->buf.size);
    // Relase CMM color table
    drmCommandWrite(fd, DRM_RCAR_DU_CMM_FREE, &mem_info->buf, sizeof mem_info->buf);
}

} // namespace android
