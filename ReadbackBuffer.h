#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_READBACK_BUFFER_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_READBACK_BUFFER_H

#include "img_gralloc1_public.h"

namespace android::hardware::graphics::composer::V2_4::implementation {

// should be in libdrm
struct rcar_du_screen_shot {
    unsigned long   buff;
    unsigned int    buff_len;
    unsigned int    crtc_id;
    unsigned int    fmt;
    unsigned int    width;
    unsigned int    height;
};

class ReadbackBuffer {
public:
    ReadbackBuffer(int drmFd, uint32_t width, uint32_t height,
                   uint32_t crtcId, const IMG_native_handle_t* buffer,
                   int releseFence);
    ReadbackBuffer(const ReadbackBuffer&) = delete;
    ReadbackBuffer& operator=(const ReadbackBuffer&) = delete;
    int getAcquireFence() const;
    void waitAndTakeCapture(int releaseFence);
    bool isInitialized() const;
private:
    int mDrmFd;
    int mBufferReleaseFence;
    bool mInitialized = false;
    rcar_du_screen_shot mData;
};

} // namespace android::hardware::graphics::composer::v2_4::implementation

#endif //ANDROID_HARDWARE_GRAPHICS_COMPOSER_READBACK_BUFFER_H
