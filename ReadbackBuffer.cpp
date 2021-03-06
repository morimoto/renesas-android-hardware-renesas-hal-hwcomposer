#include "ReadbackBuffer.h"
#include "xf86drm.h"
#include "HwcDump.h"

#include <ui/GraphicBuffer.h>
#include <drm/drm_fourcc.h>
#include <poll.h>

#define ALIGN_ROUND_UP(X, Y)  (((X)+(Y)-1) & ~((Y)-1))

// should be in libdrm
#define DRM_RCAR_DU_SCRSHOT 4

namespace android::hardware::graphics::composer::V2_3::implementation {

class GrallocModuleHandler {
public:
    GrallocModuleHandler() {
        const int eError = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mModule);

        if (!eError) {
            gralloc1_open(mModule, &mImgGrallocModule);
        } else {
            ALOGE("Can't find %s module err = %#x",
                  GRALLOC_HARDWARE_MODULE_ID, eError);
            return;
        }

        mInitialized = true;
    }
    bool isInitialized() const {
        return mInitialized;
    }
    int getPhysAddr(uint64_t* address, uint64_t bufferFd) const {
        if (!mInitialized) return -1;

        const int ret = gralloc_get_buffer_phys_addr(mImgGrallocModule,
                        bufferFd,
                        address);

        if (ret != 0) {
            ALOGE("gralloc api GetPhysAddr() returned error %#x", ret);
            return ret;
        }

        return 0;
    }
    ~GrallocModuleHandler() {
        if (!mInitialized) return;

        gralloc1_close(mImgGrallocModule);
    }
private:
    gralloc_t* mImgGrallocModule;
    hw_module_t const* mModule;
    bool mInitialized = false;
};

// -----------------------------------------------------------------------------

ReadbackBuffer::ReadbackBuffer(int drmFd, uint32_t width, uint32_t height,
                               uint32_t crtcId,
                               const IMG_native_handle_t* buffer,
                               int releaseFence)
    : mDrmFd(drmFd)
    , mBufferReleaseFence(releaseFence) {
    static GrallocModuleHandler grHandler;

    if (!grHandler.isInitialized() || !buffer) {
        return;
    }

    uint64_t address = 0;

    if (grHandler.getPhysAddr(&address, buffer->fd[0])) {
        return;
    }

    const auto widthWithStride = ALIGN_ROUND_UP(buffer->iWidth,
                                 HW_ALIGN);
    // change this if format changed
    const auto bpp = 4;
    const unsigned int size = buffer->iHeight * widthWithStride * bpp;
    mData = {
        .buff = address,
        .buff_len = size,
        .crtc_id = crtcId,
        // HAL_PIXEL_FORMAT_RGB_565 is also supported
        // if you change format, you must change bpp above also
        .fmt = DRM_FORMAT_ARGB8888,
        .width = width,
        .height = height,
    };
    mInitialized = true;
}

int ReadbackBuffer::getAcquireFence() const {
    return -1;
}

static void pollFence(int32_t fd) {
    if (fd < 0) return;

    pollfd request[1] = { { fd, POLLIN, 0 } };
    const int32_t ret = poll(request, 1, -1);

    if (ret < 0) {
        ALOGE("Poll Failed in ReadbackBuffer");
    }
}

void ReadbackBuffer::waitAndTakeCapture(int compositionReleaseFence) {
    if (!mInitialized) return;

    // wait for composition to be completed
    pollFence(compositionReleaseFence);
    // wait for wb buffer to be available
    pollFence(mBufferReleaseFence);
    CHECK_RES_WARN(drmCommandWrite(mDrmFd, DRM_RCAR_DU_SCRSHOT,
                                   &mData, sizeof(mData)));
}

bool ReadbackBuffer::isInitialized() const {
    return mInitialized;
}

} // namespace android::hardware::graphics::composer::V2_3::implementation

