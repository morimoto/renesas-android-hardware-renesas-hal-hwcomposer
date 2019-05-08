#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_CMS_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_CMS_H

#include <inttypes.h>

#define CMM_LUT_NUM 256
#define CMM_CLU_NUM (17 * 17 * 17)
#define CMM_HGO_NUM (64 * 3)

namespace android {

int cms_reset(int drm_fd, int crtc_id);
int cms_set_lut(int drm_fd, int crtc_id, const uint32_t* buff, uint32_t size);
int cms_set_clu(int drm_fd, int crtc_id, const uint32_t* buff, uint32_t size);
int cms_get_hgo(int drm_fd, int crtc_id, uint32_t* buff, uint32_t size);

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_HWC_CMS_H
