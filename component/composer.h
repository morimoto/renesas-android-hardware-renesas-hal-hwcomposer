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

#ifndef _HWC_INTERFACE_COMPOSER
#define _HWC_INTERFACE_COMPOSER

#include "imgpctrl/hwcomposer_imgpctrl.h"
#include "component/fencemerge.h"
#include <sys/ioctl.h>

#define COMPOSER_SRC     4                 /* num of source image that used to blend. */
#define COMPOSER_BUFFER  (COMPOSER_SRC+1)  /* num of all buffer for dest and source.  */
#define MAX_COMPOSER_JOBS   1

typedef struct hwc_blend_post_t {
	int           num_buffer;

	/* graphic buffer parameter */
	uint64_t  physAddress[COMPOSER_BUFFER];
	int           buffer_fd[COMPOSER_BUFFER];
	int           buffer_cached[COMPOSER_BUFFER];

	/* hwcomposer parameter */
	int           acquire_fd;

	int           operation_type;

	screen_grap_layer buffer[COMPOSER_BUFFER];

	unsigned int      bgcolor;
} hwc_blend_data;

typedef struct hwc_register_composerinfo_t {
	int max_size;
	int max_area;
	int max_rotbuf;
	int job_memory[MAX_COMPOSER_JOBS];
} hwc_register_composerinfo;


#define OPTYPE_BLENDONLY   0
#define OPTYPE_DMACOPY     1

#ifndef CMP_IOC_POST
#define IOC_R_CAR_COMP_MAGIC 'S'
#define CMP_IOC_POST      _IOW(IOC_R_CAR_COMP_MAGIC, 0x21, hwc_blend_data)

#define CMP_IOCGS_REGISTER _IOWR(IOC_R_CAR_COMP_MAGIC, 0x25, hwc_register_composerinfo)

#endif

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <utils/String8.h>
using namespace android;

/*! @class HWCComposer
 *  @brief Interface to composer driver
 */
class HWCComposer {
private:
	/* function for settarget. */
	bool settarget_getstride(const int format, const int width, const int height, const int arg_stride, int& stride);

	/* function for request. */
	bool request_setparam(unsigned int& index, hwc_layer_1_t *layer, hwc_layer_1_t *blendlayer[COMPOSER_SRC], bool& need_blend);

	/* function for hwc_setup_layerinformation. */
	bool hwc_setup_layerinformation_prepare(
		int fmt, int w, int h, int stride, int stride_c, const hwc_layer_1_t& layer,
		unsigned long offset[3], int& rtapi_fmt, int& rtapi_yuv_type, int& rtapi_yuv_range, int& num_plane, int& rtapi_premulti);
	bool hwc_setup_layerinformation_prepare_transform(const hwc_layer_1_t& layer, int& rotate, int& mirror);
	bool hwc_setup_layerinformation_prepare_offset(int fmt, int stride, int stride_c, int crop_x, int crop_y, unsigned long offset[3]);

	int additional_memory[MAX_COMPOSER_JOBS];
public:
	static int fd;
protected:
/* private */
	int max_size;
	int max_area;
	int max_rotbuf;
	hwc_rect_t fb_crop;
	hwc_rect_t fb_display;

	/* data of post */
	bool             allow_bgcolor_only;
	hwc_blend_data   data;
	unsigned int     num_layer;
	buffer_handle_t  data_handle[COMPOSER_SRC];

	int              composition_index[COMPOSER_SRC];
	int              fbtarget_layer;
	bool             prev_drawbgcolor;

	FenceMerge       usefence;

	/* struct for common utility */
	struct geometry_t {
		int iFormat;
		int iHeight;
		int iWidth;
		int iStride;
		int iStride_C;
	};

	/* function for common utility. */
	inline int hwc_width(const hwc_rect_t& rect) {
		return rect.right - rect.left;
	}
	inline int hwc_height(const hwc_rect_t& rect) {
		return rect.bottom - rect.top;
	}
	int initialize_geometry(struct geometry_t& geometry, const hwc_layer_1_t& layer);
	int initialize_geometry(struct geometry_t& geometry, int fmt, int w, int h, int stride, int stride_c);
	int hwc_setup_layerinformation(screen_grap_layer *image, const hwc_rect_t& crop, const hwc_rect_t& win, const hwc_layer_1_t& layer, const struct geometry_t& geometry);

	/* function for composer private */
	bool is_samebuffer(buffer_handle_t p);

public:
	/* function for composer */
	int setbgcolor(hwc_color_t &color);

	int settarget(const int format, const int width, const int height, const int arg_stride = 0);
	int settarget_buffer(unsigned long phys, int fd, int fence, int cached);

	void init(void);
	int addlayer(hwc_display_contents_1_t *list, int index, hwc_rect_t& crop, hwc_rect_t& disp);

	int request(hwc_display_contents_1_t *list);

	void enable(bool flag);
	void enable_bgonly(bool flag);
	inline int get_numoverlay(void) {
		return num_layer;
	}

	void dump(String8 &msg);

	/*! check whether composer driver is available */
	inline bool isValid(void) {
		return (fd >= 0);
	}

	/*! return max size*/
	inline int get_maxSize(void) {
		return max_size;
	}

	/*! return max area*/
	inline int get_maxArea(void) {
		return max_area;
	}

	/*! return max rotation buffer*/
	inline int get_maxRotbuffer(void) {
		return max_rotbuf;
	}

	HWCComposer();
	~HWCComposer();
};

#endif
