/*
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
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

#include "component/drm_display.h"
#include "config.h"
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>

#include <system/graphics.h>
#include <cutils/log.h>

#include "img_gralloc_public.h"

#include <hardware/hwcomposer_defs.h>

#include <cmath>
#include <string>
#include <fstream>
#include <streambuf>

#define DRM_THREAD_NAME "HWCDirectRender"
#define DRM_VBLANK_THREAD_NAME "HWCDirectRenderVBlank"

#define CODE_TERM             0

#define DEBUG_ATRACE          (DEBUG_USE_ATRACE && USE_DBGLEVEL(1))
#define DEBUG                 USE_DBGLEVEL(4)

#define DISABLE_BLANK_OPERATION   (1 || (DRMSUPPORT_BLANK_DESKTOP==0))

#if DEBUG_ATRACE
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#include <utils/Trace.h>

using namespace android;
#endif

/* macro for alignment */
#define ALIGN_ROUND_UP(X, Y)  (((X)+(Y)-1) & ~((Y)-1))

#define DBG_PRINT(FMT,ARG...) ALOGD_IF(DEBUG,"[HWCDRM] " FMT,##ARG)
#define ERR_PRINT(FMT,ARG...) ALOGE("[HWCDRM] " FMT,##ARG)

#define MATCH_SIZE_OFF 0
#define MATCH_SIZE_ON 1

#define VSYNC_PRIM_DISP 0
#define VSYNC_EXT_DISP  1

struct display_fps_t {
	int mode_num;
	int fps;
};

#if DRMSUPPORT_BLANK_DESKTOP
struct rcar_du_desktop_plane {
	int crtc_id;
	int on;
};
#endif


/*! \brief Set plane with fence extension
 *  \param[in] fd       file descriptor
 *  \param[in] plane_id identify of plane object
 *  \param[in] crtc_id  identify of crt object
 *  \param[in] fb_id    identify of frame buffer object
 *  \param[in] flags    reserved to 0.
 *  \param[in] crtc_x   display position x
 *  \param[in] crtc_y   display position y
 *  \param[in] crtc_w   display size width
 *  \param[in] crtc_h   display size height
 *  \param[in] src_x    crop position x     fixed point[16.16]
 *  \param[in] src_y    crop position y     fixed point[16.16]
 *  \param[in] src_w    crop size width     fixed point[16.16]
 *  \param[in] src_h    crop size height    fixed point[16.16]
 *  \return processing result.
 *  \retval fence no error if fb_id is positive.
 *  \retval 0     no error if fb_id is zero.
 *  \retval -1    error.
 */
int DRMUtil::drmModeSetPlaneFence(int fd, uint32_t plane_id, uint32_t crtc_id,
		    uint32_t fb_id, uint32_t flags,
		    uint32_t crtc_x, uint32_t crtc_y,
		    uint32_t crtc_w, uint32_t crtc_h,
		    uint32_t src_x, uint32_t src_y,
		    uint32_t src_w, uint32_t src_h)
{
	unsigned long request;
	struct drm_mode_set_plane s;

	s.plane_id = plane_id;
	s.crtc_id = crtc_id;
	s.fb_id = fb_id;
	s.flags = flags;
	s.crtc_x = crtc_x;
	s.crtc_y = crtc_y;
	s.crtc_w = crtc_w;
	s.crtc_h = crtc_h;
	s.src_x = src_x;
	s.src_y = src_y;
	s.src_w = src_w;
	s.src_h = src_h;

	request = DRM_IOC( DRM_IOC_WRITE, DRM_IOCTL_BASE, DRM_COMMAND_BASE + 0, (sizeof s));

	return drmIoctl(fd, request, &s);
}

/*! \brief Set plane with fence extension
 *  \param[in] fd       file descriptor
 *  \param[in] crt      identify of crt object
 *  \param[in] on       state of desktop plane
 *  \return processing result.
 *  \retval 0     no error.
 *  \retval -1    error.
 */
int DRMUtil::drmRcarDU_DesktopPlane(int fd, int crt, int on)
{
#if DRMSUPPORT_BLANK_DESKTOP
	struct rcar_du_desktop_plane data;
	data.crtc_id = crt;
	data.on = on;
	return drmCommandWrite(fd, 3, &data, sizeof(data));
#else
	UNUSED(fd);
	UNUSED(crt);
	UNUSED(on);
	return -1;
#endif
}


/*! \brief Set attributes
 *  \param[in] connector pointer to a drmModeConnector structure
 *  \param[in] mode pointer to a drmModeModeInfo structure
 *  \param[in] disp_id  display ID
 *  \param[in] vrefresh vrefresh rate
 *  \return none
 */
void DRMDisplay::setattributes(drmModeConnector *connector, drmModeModeInfo *mode, int disp_id, float vrefresh)
{
	float xdpi, ydpi;
	int  format = DRM_FORMAT_XRGB8888;

	/* update attributes */
	display[disp_id].attributes.display_dotclock = mode->clock;
	display[disp_id].attributes.display_vsync_period = 1000000000 / vrefresh;
	if (mode->flags & DRM_MODE_FLAG_INTERLACE) {
		display[disp_id].attributes.display_vsync_period *= 2;
	}
	display[disp_id].attributes.display_width = mode->hdisplay;
	display[disp_id].attributes.display_height = mode->vdisplay;

	display[disp_id].attributes.display_stride  = get_drm_fb_pitch(mode->hdisplay, format);
	if (connector->mmWidth <= 0) {
		/* dpi unknown */
		xdpi = 0;
	} else {
		xdpi = (25.4 * mode->hdisplay) / connector->mmWidth;
	}

	if (connector->mmHeight <= 0) {
		/* dpi unknown */
		ydpi = 0;
	} else {
		ydpi = (25.4 * mode->vdisplay) / connector->mmHeight;
	}
	display[disp_id].attributes.display_dpi_x = xdpi;
	display[disp_id].attributes.display_dpi_y = ydpi;

	DBG_PRINT("disp_id:%d attributes:%d %d %d %f %f %d", disp_id,
		display[disp_id].attributes.display_vsync_period,
		display[disp_id].attributes.display_width,
		display[disp_id].attributes.display_height,
		display[disp_id].attributes.display_dpi_x,
		display[disp_id].attributes.display_dpi_y,
		display[disp_id].attributes.display_stride);

	return;
}

/*! \brief Get vrefresh
 *  \param[in] connector pointer to a drmModeConnector structure
 *  \param[in] mode pointer to a drmModeModeInfo structure
 *  \return result of processing
 *  \retval refresh vrefresh rate
 */
float DRMDisplay::get_drm_mode_vrefresh(drmModeConnector *connector, drmModeModeInfo *mode)
{

	float    refresh;

	UNUSED(connector);

	if (mode->vrefresh > 0) {
		refresh = mode->vrefresh;
	} else {
		ALOGE("can not determine vrefresh, assume 60Hz.");
		refresh = 60;
	}

	DBG_PRINT("refresh:%f", refresh);

	return refresh;
}


/*! \brief Get pitch size
 *  \param[in] width width
 *  \param[in] fmt format
 *  \return result of processing
 *  \retval pitch size
 */
uint32_t DRMDisplay::get_drm_fb_pitch(int width, int fmt)
{
	uint32_t pitch;
	uint32_t align;

	/* for koelsche require 16pixel alignment, for lager require 128 byte alignment.
	 * to make implementation simply, select 128bytes alignment. */
	align = 128;

	/* currently only support format XRGB8888 */
	switch(fmt) {
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ARGB8888:
		pitch = ALIGN_ROUND_UP(width * 4, align);
		break;
	case DRM_FORMAT_RGB888:
		pitch = ALIGN_ROUND_UP(width * 3, align);
		break;
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_UYVY:
		pitch = ALIGN_ROUND_UP(width * 2, align);
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_YVU420:
		pitch = ALIGN_ROUND_UP(width * 1, align);
		break;
	default:
		ERR_PRINT("format %d unknwon assume RGBA8888", fmt);
		pitch = ALIGN_ROUND_UP(width * 4, align);
		break;
	}

	return pitch;
}

/*! \brief Get attributes
 *  \param[in] disp_id  display ID
 *  \param[in] hwc_attr attributes
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 */
bool DRMDisplay::getattributes(int disp_id,  struct drm_attributes_t *hwc_attr)
{
	if (disp_id >= 0 && disp_id <= 2) {
		memcpy(hwc_attr, &display[disp_id].attributes, sizeof(*hwc_attr));
		DBG_PRINT("disp_id:%d attributes:%d %d %d %f %f %d", disp_id, hwc_attr->display_vsync_period, hwc_attr->display_width, hwc_attr->display_height, hwc_attr->display_dpi_x, hwc_attr->display_dpi_y, hwc_attr->display_stride);
		return true;
	} else {
		ERR_PRINT("invalid disp_id");
		return false;
	}
}

/*! \brief post process of getmode
 *  \param[in] disp_id    display id
 *  \param[in] resources  pointer to drmModeRes structure
 *  \return none
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for get_mode function.
 */
void DRMDisplay::get_mode_part1(int disp_id, drmModeRes *resources)
{
	uint32_t crt_id = display[disp_id].crtc_id;
	int      i;

	/* setup highcrtc */
	display[disp_id].high_crtc = -1;
	for (i = 0; i < resources->count_crtcs; i++) {
		if (resources->crtcs[i] == crt_id) {
			display[disp_id].high_crtc = i;
			break;
		}
	}

	/* update plane info */
	if (display[disp_id].high_crtc >= 0) {
		drmModePlaneRes *plane_res = drmModeGetPlaneResources(drm_fd);
		if (plane_res) {
			int num_plane = 0;
			for (i = 0; num_plane < 1 && i < (int)plane_res->count_planes; i++) {
				drmModePlane *plane =drmModeGetPlane(drm_fd, plane_res->planes[i]);

				if (plane->possible_crtcs & (1 << display[disp_id].high_crtc)) {
					display[disp_id].plane_id[num_plane++] = plane_res->planes[i];
				}

				drmModeFreePlane(plane);
			}

			drmModeFreePlaneResources(plane_res);
		}
	}

	/* configure property of desktop */
	if (display[disp_id].plane_id[0] > 0) {
		int plane_id = display[disp_id].plane_id[0];
		drmModeObjectPropertiesPtr properties;

		properties = drmModeObjectGetProperties(drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);
		if (properties) {
			drmModePropertyPtr prop;
			bool set_zpos_property = false;

			for (i = 0; i < (int)properties->count_props; i++) {
				prop = drmModeGetProperty(drm_fd, properties->props[i]);
				if (strcmp(prop->name, "zpos") == 0) {
					set_zpos_property = true;
					if (drmModeObjectSetProperty(drm_fd, plane_id, DRM_MODE_OBJECT_PLANE, prop->prop_id, 0)) {
						ERR_PRINT("drmModeObjectSetProperty error.");
					}
				}
				drmModeFreeProperty(prop);
			}
			if (!set_zpos_property) {
				ERR_PRINT("property zpos not found.");
			}
			drmModeFreeObjectProperties(properties);
		}
	}

	if (display[disp_id].high_crtc < 0) {
		ERR_PRINT("can not determine high_crtc.");
	}
	if (display[disp_id].plane_id[0] == 0) {
		ERR_PRINT("can not determine plane ID for YUV.");
	}
}

/*! \brief Get mode
 *  \param[in] disp_id  display ID
 *  \param[in] mode_enc mode encoder type
 *  \param[in] mode_con mode connector type
 *  \param[out] width width
 *  \param[out] height height
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 *  \details
 *  search crtc mode from encoder/connector type
 */
bool DRMDisplay::getmode(int disp_id, uint32_t mode_enc, uint32_t mode_con,
			 int *width, int *height)
{
	drmModeRes *resources;
	drmModeEncoder *encoder;
	drmModeCrtc *crtc = NULL;
	drmModeModeInfo *mode;
	drmModeConnector *connector = NULL;
	float vrefresh;
	uint32_t crt_id, con_id;
	int set_flag = 0;
	bool result = false;
	int i;

	resources = drmModeGetResources(drm_fd);
	if (!resources) {
		ERR_PRINT("drmModeGetResources error");
		goto err_exit;
	}

	if (disp_id < 0 || disp_id > 2) {
		ERR_PRINT("invalid disp_id");
		goto err_exit;
	}

	for (i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(drm_fd, resources->encoders[i]);
		if (!encoder) {
			ERR_PRINT("drmModeGetEncoder error");
			goto err_exit;
		}

		if(encoder->encoder_type == mode_enc) {
			set_flag = 1;
			break;
		}
		drmModeFreeEncoder(encoder);
	}
	if (!set_flag) {
		ERR_PRINT("no matching encoder_type");
		goto err_exit;
	}

	crt_id = encoder->crtc_id;
	drmModeFreeEncoder(encoder);

	set_flag = 0;
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!connector) {
			ERR_PRINT("drmModeGetConnector error");
			goto err_exit;
		}
		if (connector->connector_type == mode_con) {
			set_flag = 1;
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}
	if (!set_flag) {
		ERR_PRINT("no matching connector_type");
		goto err_exit;
	}

	con_id = connector->connector_id;

	crtc = drmModeGetCrtc(drm_fd, crt_id);
	if (!crtc) {
		ERR_PRINT("drmModeGetCrtc error");
		goto err_exit;
	}

	mode = &crtc->mode;

	if (mode->hdisplay == 0 || mode->vdisplay == 0) {
		ERR_PRINT("mode uninitialized");
		goto err_exit;
	}

	*height = mode->vdisplay;
	*width  = mode->hdisplay;

	/* record information. */
	display[disp_id].crtc_id      = crt_id;
	display[disp_id].connector_id = con_id;
	display[disp_id].plane_id[0]  = 0; /* assume no plane */
	display[disp_id].mode         = *mode;

	vrefresh = get_drm_mode_vrefresh(connector, mode);

	/* update attributes */
	setattributes(connector, mode, disp_id, vrefresh);

	/* post process of getmode */
	get_mode_part1(disp_id, resources);

	result = true;

err_exit:
	if (crtc) {
		/* free crtc */
		drmModeFreeCrtc(crtc);
	}
	if (connector) {
		/* free connector */
		drmModeFreeConnector(connector);
	}
	if (resources) {
		/* free resource */
		drmModeFreeResources(resources);
	}

	return result;
}

bool DRMDisplay::getResolutionFromBootargs(int disp_id, int& width, int& height, int& refresh) {

	std::ifstream infile { "/proc/cmdline" };
	std::string args { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };

	std::string hdmi_args(hwdisplays[disp_id].bootargs);
	size_t found = args.find(hdmi_args);
	if (found == std::string::npos)
		return false;

	int w, h;
	std::string s = args.substr(found + hdmi_args.length());
	found = s.find("x");
	if (found == std::string::npos)
		return false;

	std::string s_width = s.substr(0, found);

	s = s.substr(found + 1); // 1 - "x" letter

	found = s.find("-");
	if (found == std::string::npos)
		return false;

	std::string s_height = s.substr(0, found);

	s = s.substr(found + 1); // 1 - "-" letter

	found = s.find("@");
	if (found == std::string::npos)
		return false;

	std::string s_bpp = s.substr(0, found);

	s = s.substr(found + 1); // 1 - "@" letter

	found = s.find(" ");

	std::string s_refresh;
	if (found != std::string::npos)
		s_refresh = s.substr(0, found);
	else
		s_refresh = s;

	width = std::stoi(s_width);
	height = std::stoi(s_height);
	refresh = std::stoi(s_refresh);

	return true;
}

/*! \brief Set mode
 *  \param[in] disp_id  display ID
 *  \param[in] mode_enc mode encoder type
 *  \param[in] mode_con mode connector type
 *  \param[in] width width
 *  \param[in] height height
 *  \param[in] interlace interlace
 *  \param[in] HZ hertz
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 *  \details
 *  select crtc mode from parameters
 *  this function is called when getmode returns false
 */
bool DRMDisplay::setmode(int disp_id, uint32_t enc_id, uint32_t con_id,
	int& width, int& height, bool interlace, int HZ)
{
	bool result = false;
	int  i, j;
	float vrefresh = 60;

	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeModeInfo *mode;
	drmModeEncoder *encoder;

	int matching_size_flag = MATCH_SIZE_OFF;
	int matching_count = 0;
	struct display_fps_t comp_fps[20];
	struct display_fps_t temp_fps;
	uint32_t crt_id;
	int set_flag = 0;
	float preferred_aspect_ratio = 0;
	bool isBootargsPresented = false;

	resources = drmModeGetResources(drm_fd);
	if (!resources) {
		ERR_PRINT("drmModeGetResources error");
		goto err_exit;
	}

	if (disp_id < 0 || disp_id > 2) {
		ERR_PRINT("invalid disp_id");
		goto err_exit;
	}

	for (i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(drm_fd, resources->encoders[i]);
		if (!encoder) {
			ERR_PRINT("drmModeGetEncoder error");
			goto err_exit;
		}
		if (encoder->encoder_id == enc_id) {
			set_flag = 1;
			break;
		}
		drmModeFreeEncoder(encoder);
	}
	if (!set_flag) {
		ERR_PRINT("no matching encoder_type");
		goto err_exit;
	}

	crt_id = encoder->crtc_id;

	if(crt_id == 0) {
		for (j = 0; j < resources->count_crtcs; ++j) {

			if (!(encoder->possible_crtcs & (1 << j)))
				continue;

			crt_id = resources->crtcs[j];
		}
	}
	drmModeFreeEncoder(encoder);

	set_flag = 0;
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!connector) {
			ERR_PRINT("drmModeGetConnector error");
			goto err_exit;
		}
		if (connector->connector_id == con_id) {
			set_flag = 1;
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}
	if (!set_flag) {
		ERR_PRINT("no matching connector_type");
		goto err_exit;
	}

	if (HZ == 0)
		HZ = 60;

	mode = NULL;

	for (i = 0; i < connector->count_modes; i++) {
		drmModeModeInfo *test_mode;
		test_mode = &connector->modes[i];
		ALOGD("mode %d name = %s vrefresh = %u", i, test_mode->name, test_mode->vrefresh);
	}

	if (connector->count_modes > 0) {
		preferred_aspect_ratio = connector->modes[0].hdisplay /
				(float)connector->modes[0].vdisplay;
	}

	for (i = 0; i < connector->count_modes; i++) {
		drmModeModeInfo *test_mode;
		test_mode = &connector->modes[i];
		if(test_mode->type & DRM_MODE_TYPE_PREFERRED) {
			preferred_aspect_ratio = test_mode->hdisplay / (float)test_mode->vdisplay;
			break;
		}
	}

	isBootargsPresented = getResolutionFromBootargs(disp_id, width, height, HZ);

	for (i = 0; i < connector->count_modes; i++) {
		drmModeModeInfo *test;

		test = &connector->modes[i];

		float aspect = test->hdisplay / (float)test->vdisplay;

		if (!isBootargsPresented && (fabs(aspect - preferred_aspect_ratio) > 0.001)) {
			continue;
		}

		if (!isBootargsPresented && (test->hdisplay > 1920)) {
			/* Now we don't support FullHD resolution */
			continue;
		}

		if (isBootargsPresented && (test->hdisplay != width ||
				test->vdisplay != height)) {
			/* display size is not meet request value */
			continue;
		}

		if ((test->type == DRM_MODE_TYPE_USERDEF) && (disp_id != 0)) {
			/* Do this only for external displays */
			/* Skip userdef module because it generated by drm driver and may
			* not support by display */
			continue;
		}

		if (test->vrefresh != (unsigned int) HZ) {
			matching_size_flag = MATCH_SIZE_ON;
			matching_count++;
			if (matching_count > 20) {
				matching_size_flag = MATCH_SIZE_OFF;
				ERR_PRINT("matching count exceed limit");
				break;
			}
			comp_fps[matching_count - 1].mode_num = i;
			comp_fps[matching_count - 1].fps
				= abs(test->vrefresh - HZ);
			continue;
		}
		vrefresh = get_drm_mode_vrefresh(connector, test);

		if (interlace) {
			/* interlace is requested. */
			if (!(test->flags & DRM_MODE_FLAG_INTERLACE)) {
				continue;
			}
		}

		mode = test;
		ALOGD("selected mode %d name = %s vrefresh = %u", i, mode->name, mode->vrefresh);
		width = mode->hdisplay;
		height = mode->vdisplay;
		break;
	}
	/* can't find same (HZ) */
	if (mode == NULL && matching_size_flag == MATCH_SIZE_ON) {
		for (i = 0; i < matching_count; i++) {
			for (j = i + 1; j < matching_count; j++) {
				if (comp_fps[i].fps > comp_fps[j].fps){
					temp_fps = comp_fps[i];
					comp_fps[i] = comp_fps[j];
					comp_fps[j] = temp_fps;
				}
			}
		}
		for (i = 0; i < matching_count; i++) {
			drmModeModeInfo *test;

			test = &connector->modes[comp_fps[i].mode_num];

			vrefresh = get_drm_mode_vrefresh(connector, test);

			if (interlace) {
				/* interlace is requested. */
				if (!(test->flags & DRM_MODE_FLAG_INTERLACE)) {
					continue;
				}
			}

			mode = test;
			break;
		}
	}

	if (mode == NULL) {
		if (HZ) {
			ERR_PRINT("can not found available mode %dx%d @ %d Hz %s", width, height, HZ, interlace ? "interlace":"");
		} else {
			ERR_PRINT("can not found available mode %dx%d %s", width, height, interlace ? "interlace":"");
		}
		goto err_exit;
	} else {
		/* record information. */
		display[disp_id].crtc_id      = crt_id;
		display[disp_id].connector_id = con_id;
		display[disp_id].mode         = *mode;

		/* update attributes */
		setattributes(connector, mode, disp_id, vrefresh);

		result = true;
	}

	display[disp_id].first_draw = true;

err_exit:
	if (connector) {
		/* free connector */
		drmModeFreeConnector(connector);
	}
	if (resources) {
		/* free resource */
		drmModeFreeResources(resources);
	}

	return result;
}

/*! \brief Display plane
 *  \param[in] disp_id  display ID
 *  \param[in] plane_id plane ID    0 is YUV
 *  \param[in] buffer pointer to a IonBuffer class
 *  \param[in] cx  crop position x
 *  \param[in] cy  crop position y
 *  \param[in] cw  crop size width
 *  \param[in] ch  crop size height
 *  \param[in] dx  window position x
 *  \param[in] dy  window position y
 *  \param[in] dw  window size width
 *  \param[in] dh  window size height
 *  \param[out] f  pointer of int
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 *  \details
 *  send a request of display plane to DRM driver
 */
bool DRMDisplay::display_plane(int disp_id, int plane_id, sp<IonBuffer>& buffer, int cx, int cy, int cw, int ch, int dx, int dy, int dw, int dh, int *f)
{
	bool res = false;
#if DEBUG_ATRACE
	ATRACE_CALL();
#endif
	Mutex::Autolock _l(lock_plane);

	if (disp_id < 0 || disp_id > 2) {
		ERR_PRINT("invalid disp_id");
	} else if (plane_id != 0) {
		ERR_PRINT("invalid plane_id");
#if DRMSUPPORT_BLANK_DESKTOP
	} else if (display[disp_id].blank_state) {
		DBG_PRINT("ignore setplane");
#endif
	} else if (buffer.get() == NULL && display[disp_id].plane_buffer[plane_id].get() == NULL) {
		/* nothing to do. */
		if (f) {
			*f = -1;
		}
		res = true;
	} else {
		uint32_t crt   = display[disp_id].crtc_id;
		uint32_t plane = display[disp_id].plane_id[plane_id];
		uint32_t fb_id;
		int      result;

		if (f) {
			*f = -1;
		}
		if (buffer.get()) {
			fb_id = buffer->drm_fbid;
		} else {
			fb_id = 0;
		}

		DBG_PRINT("plane:%d crt:%d fb:%d\n", plane, crt, fb_id);

		{
			result = drmModeSetPlaneFence(drm_fd,plane,crt,fb_id,0,
				dx,dy,dw,dh,cx<<16,cy<<16,cw<<16,ch<<16);
			if (fb_id == 0) {
				/* nothing to do */
			} else {
				/* handle fence */
				if (f) {
					*f = result;
				} else {
					close(result);
				}
				if (result >= 0) {
					result = 0;
				}
			}
			if (result < 0) {
#if DRMSUPPORT_BLANK_DESKTOP == 0
				DBG_PRINT("drmModeSetPlaneFence error [May not be error.]");
#else
				ERR_PRINT("drmModeSetPlaneFence error");
#endif
			}
		}
		display[disp_id].plane_buffer[plane_id] = buffer;

		if (result == 0) {
			res = true;
		}
	}

	return res;
}


/*! \brief Set blank state
 *  \param[in] disp_id  display ID
 *  \param[in] status  nonzero(screen off) zero(screen on)
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 */
bool DRMDisplay::set_blankstate(int disp_id, bool status)
{
#if DISABLE_BLANK_OPERATION == 0
	int mode = (status) ? DRM_MODE_DPMS_OFF: DRM_MODE_DPMS_ON;
#endif
#if DEBUG_ATRACE
	ATRACE_CALL();
#endif

	if (disp_id >= 0 && disp_id <= 2) {
		/* acquire flip lock */
		Mutex::Autolock _l(lock_flip);
		while(!display[disp_id].flip_listners.empty()) {
			Condition dummy;
			/* unlock and wait 16msec */
			dummy.waitRelative(lock_flip, 16*1000000LL);
		}

		if (display[disp_id].blank_state && !status) {
			/* change blank to unblank */
		} else if (!display[disp_id].blank_state && status) {
			/* change unblank to blank */
			sp<IonBuffer> buffer = NULL;
			if (display[disp_id].plane_buffer[0].get()) {
				display_plane(disp_id, 0, buffer);
			}

#if DRMSUPPORT_BLANK_DESKTOP
			/* blank */
			if (drmRcarDU_DesktopPlane(drm_fd, display[disp_id].crtc_id, 0)) {
				ERR_PRINT("drmRcarDU_DesktopPlane error.");
			} else {
				display[disp_id].desktop_visible = false;
			}
#endif
		}

#if DISABLE_BLANK_OPERATION == 0
		int property_id = 2; /* DPMS property */
		if (drmModeConnectorSetProperty(drm_fd, display[disp_id].connector_id, property_id, mode)) {
			ERR_PRINT("display%d: drmModeConnectorSetProperty error", disp_id);
		} else {
#endif
			if (display[disp_id].blank_state && !status) {
				/* change blank to unblank */
				display[disp_id].first_draw = true;
			} else if (!display[disp_id].blank_state && status) {
				/* change unblank to blank */
#if DISABLE_BLANK_OPERATION == 0
				Mutex::Autolock _l(lock_vsync);
				List<VSyncCallback*>::iterator it;

				/* notice listner. goto blank. */
				for(it = vsync_listners.begin(); it != vsync_listners.end(); it++) {
					(*it)->vsync_callback(true, 0ll);
				}
#endif
			}
			display[disp_id].blank_state = status;
#if DISABLE_BLANK_OPERATION == 0
		}
#endif
		usleep(display[disp_id].attributes.display_vsync_period / 1000);
	} else {
		ERR_PRINT("invalid disp_id");
	}

	return true;
}


/*! \brief initialize
 *  \return none
 *  \details
 *  create thread
 */
void DRMDisplay::init(void)
{

        thread.start(DRM_THREAD_NAME, PRIORITY_URGENT_DISPLAY, _threadLoop, (void*)this);

        thread_vsync.start(DRM_VBLANK_THREAD_NAME, PRIORITY_URGENT_DISPLAY, _threadLoop_vsync, (void*)this);


}

/*! \brief request thread terminate
 *  \return none
 *  \details
 *  request thread terminate and return after thread terminate.
 */
void DRMDisplay::deinit(void)
{

	thread.terminate();
	thread_vsync.terminate();

}

/*! \brief DRMDisplay initialize
 */
DRMDisplay::DRMDisplay():
	request_next(false)
{
	int i;

	drm_fd = drmOpen("rcar-du", NULL);
	if (drm_fd < 0) {
		ERR_PRINT("drmOpen error");
	}

	drm_fd_vsync = drmOpen("rcar-du", NULL);
	if (drm_fd_vsync < 0) {
		ERR_PRINT("drmOpen error");
	}

	drmDropMaster(drm_fd_vsync);

	memset(&evctx, 0, sizeof(evctx));
	evctx.version = DRM_EVENT_CONTEXT_VERSION;
	evctx.page_flip_handler = page_flip_handler;

	memset(&evctx_vsync, 0, sizeof(evctx_vsync));
	evctx_vsync.version = DRM_EVENT_CONTEXT_VERSION;
	evctx_vsync.vblank_handler = vblank_handler;

	for (i = 0; i < 3; i++) {

		display[i].crtc_id      = 0;
		display[i].connector_id = 0;
		display[i].plane_id[0]  = 0;
		display[i].high_crtc    = -1;
		memset(&display[i].mode,       0, sizeof(display[i].mode));

		display[i].attributes.display_vsync_period = 0;
		display[i].attributes.display_width = 0;
		display[i].attributes.display_height = 0;
		display[i].attributes.display_dpi_x = 0;
		display[i].attributes.display_dpi_y = 0;
		display[i].attributes.display_stride = 0;

		display[i].blank_state = false;
		display[i].first_draw  = true;
#if DRMSUPPORT_BLANK_DESKTOP
		display[i].desktop_visible = false;
#endif
	}

}

/*! \brief DRMDisplay destructor
 */
DRMDisplay::~DRMDisplay()
{
	int i;

	/* remove listner */
	for (i = 0; i < 3; i++) {
		List< sp<FlipCallback> >::iterator it;
		for (it = display[i].flip_listners.begin(); it != display[i].flip_listners.end(); it++) {
			(*it)->buffer = NULL;
			(*it) = NULL;
		}
		display[i].flip_listners.clear();
	}
	vsync_listners.clear();

	if (drm_fd >= 0) {
		drmClose(drm_fd);
		drm_fd = -1;
	}

	if (drm_fd_vsync >= 0) {
		drmClose(drm_fd_vsync);
		drm_fd_vsync = -1;
	}

	for (i = 0; i < 3; i++) {
		if (display[i].plane_buffer[0].get()) {
			ALOGE("invalid sequence. all plane should turn off.");
			display[i].plane_buffer[0] = NULL;
		}
	}
}


/*! \brief constructor of IonBuffer
 *  \param[in] _dsp      pointer to DRMDisplay class
 *  \param[in] _hnd      pointer to buffer_handle_t structure
 *  \return none
 */
DRMDisplay::IonBuffer::IonBuffer(DRMDisplay *_dsp, buffer_handle_t _hnd)
{
	IMG_native_handle_t *img_hnd = (IMG_native_handle_t *)_hnd;
	IonBuffer(_dsp, img_hnd->fd[0], img_hnd->iFormat, img_hnd->iWidth, img_hnd->iHeight);
}


/*! \brief constructor of IonBuffer
 *  \param[in] _dsp      pointer to DRMDisplay class
 *  \param[in] ion_fd    file descriptor of dma-buf. for example. ion fd
 *  \param[in] format    color format specifeied by hal-code.
 *  \param[in] width     width of image.
 *  \param[in] height    height of image.
 *  \return none
 */
DRMDisplay::IonBuffer::IonBuffer(DRMDisplay *_dsp, int ion_fd, int format, int width, int height)
{
	int      stride_width;
	uint32_t   handle;

	handle   = 0;
	drm_fbid = -1;

	/* keep pointer */
	dsp = _dsp;

	/* configure width with align */
	if (format == HAL_PIXEL_FORMAT_NV12_CUSTOM || format ==HAL_PIXEL_FORMAT_NV21_CUSTOM) {
		stride_width = ALIGN_ROUND_UP(width, 128);
	} else {
		stride_width = ALIGN_ROUND_UP(width, HW_ALIGN);
	}

	/* register ion memory to drm driver */
	if (drmPrimeFDToHandle(dsp->drm_fd, ion_fd, &handle)) {
		ERR_PRINT("drmPrimeFDToHandle error");
	} else {
		uint32_t bo_handles[4] = { 0 };
		uint32_t pitches[4]    = { 0 };
		uint32_t offsets[4]    = { 0 };
		struct drm_gem_close arg = {
			handle: handle,
			pad:    0,
		};

		/* configure format and pitch */
		switch(format) {
		case HAL_PIXEL_FORMAT_BGRX_8888:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 4;
			format = DRM_FORMAT_XRGB8888;
			break;
		case HAL_PIXEL_FORMAT_BGRA_8888:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 4;
			format = DRM_FORMAT_ARGB8888;
			break;
		case HAL_PIXEL_FORMAT_RGBX_8888:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 4;
			format = DRM_FORMAT_XBGR8888;
			break;
		case HAL_PIXEL_FORMAT_RGBA_8888:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 4;
			format = DRM_FORMAT_ABGR8888;
			break;
		case HAL_PIXEL_FORMAT_RGB_888:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 3;
			format = DRM_FORMAT_RGB888;
			break;
		case HAL_PIXEL_FORMAT_RGB_565:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 2;
			format = DRM_FORMAT_RGB565;
			break;
		case HAL_PIXEL_FORMAT_UYVY:
			bo_handles[0] = handle;
			pitches[0] = stride_width * 2;
			format = DRM_FORMAT_UYVY;
			break;
		case HAL_PIXEL_FORMAT_NV12:
		case HAL_PIXEL_FORMAT_NV12_CUSTOM:
			bo_handles[0] = handle;
			bo_handles[1] = handle;
			pitches[0] = stride_width;
			pitches[1] = stride_width;
			offsets[1] = pitches[0] * height;
			format = DRM_FORMAT_NV12;
			break;
		case HAL_PIXEL_FORMAT_NV21:
		case HAL_PIXEL_FORMAT_NV21_CUSTOM:
			bo_handles[0] = handle;
			bo_handles[1] = handle;
			pitches[0] = stride_width;
			pitches[1] = stride_width;
			offsets[1] = pitches[0] * height;
			format = DRM_FORMAT_NV21;
			break;
		case HAL_PIXEL_FORMAT_YV12:
			bo_handles[0] = handle;
			bo_handles[1] = handle;
			bo_handles[2] = handle;
			pitches[0] = stride_width;
			pitches[1] = stride_width/2;
			pitches[2] = stride_width/2;
			offsets[1] =              pitches[0] * height;
			offsets[2] = offsets[0] + pitches[1] * ((height+1)/2);
			format = DRM_FORMAT_YVU420;
			break;
		default:
			format = 0;
		}

		if (drmModeAddFB2(dsp->drm_fd, width, height, format, bo_handles, pitches, offsets, (uint32_t*)&drm_fbid, 0)) {
			ERR_PRINT("drmModeAddFB2 error");
		} else {
			DBG_PRINT("register width:%d height:%d handle:%d fbid:%d ionfd:%d", width, height, handle, drm_fbid, ion_fd);
		}

		/* close import reference */
		if (drmIoctl(dsp->drm_fd, DRM_IOCTL_GEM_CLOSE, &arg)) {
			ERR_PRINT("free handle from drmPrimeFDToHandle");
		}
		handle = 0;
	}
}

/*! \brief deconstructor of IonBuffer
 *  \return none
 */
DRMDisplay::IonBuffer::~IonBuffer()
{
	if (drm_fbid >= 0) {
		DBG_PRINT("unregister fbid:%d", drm_fbid);

		if (drmModeRmFB(dsp->drm_fd, drm_fbid)) {
			ERR_PRINT("drmModeRmFB error");
		}
	}
}

/* default implementation of pageflip listner class */

/*! \brief callback of pageflip
 *  \return none
 */
void DRMDisplay::FlipCallback::flip_callback(void)
{
}

/*! \brief destructor of pageflip listner
 *  \return none
 */
DRMDisplay::FlipCallback::~FlipCallback()
{
}

/*! \brief display buffer using Page flip 
 *  \param[in] dispid  displayid
 *  \param[in] buffer  buffer to display
 *  \param[in] listner pointer to FlipCallback
 *  \return result of processing
 *  \retval false  pageflip listner not available.
 *  \retval true   pageflip listner available.
 *  \details
 *  display specified buffer to display. at first use SetCRTC, lator use pageFlip.
 */
int DRMDisplay::display_pageflip(int dispid, sp<IonBuffer>& buffer, sp<FlipCallback>& listner)
{
	bool result = false;
	Mutex::Autolock _l(lock_flip);

	if (dispid < 0 || dispid > 2) {
		ERR_PRINT("invalid dispid");
#if DRMSUPPORT_BLANK_DESKTOP
	} else if (display[dispid].blank_state) {
		DBG_PRINT("ignore flip");
#endif
	} else if (buffer->drm_fbid <= 0) {
		ERR_PRINT("buffer invalid.");
	} else {
		List< sp<FlipCallback> >::iterator it;
		List< sp<FlipCallback> >& list = display[dispid].flip_listners;

		for(it = list.begin(); it != list.end();it++) {
			if ((*it).get()==listner.get()) break;
		}

		if (it != list.end()) {
			ERR_PRINT("pageflip already requested.");
		} else {
			uint32_t crt_id = display[dispid].crtc_id;
			uint32_t con_id = display[dispid].connector_id;

			if (display[dispid].first_draw) {
				/* set mode and framebuffer */
				if (drmModeSetCrtc(drm_fd, crt_id, buffer->drm_fbid, 0, 0, &con_id, 1, &display[dispid].mode)) {
#if DRMSUPPORT_BLANK_DESKTOP == 0
					DBG_PRINT("drmModeSetCrtc error [May not be error.]");
#else
					ERR_PRINT("drmModeSetCrtc error");
#endif
				} else {
					/* clear first draw flag */
					display[dispid].first_draw = false;
				}
			} else {
				/* register listner */
				listner->dsp = this;
				listner->dispid = dispid;
				listner->buffer = buffer;

				list.push_back(listner);

				/* page flip */
				if (drmModePageFlip(drm_fd, crt_id, buffer->drm_fbid, DRM_MODE_PAGE_FLIP_EVENT, (void *)listner.get())) {
#if DRMSUPPORT_BLANK_DESKTOP == 0
					DBG_PRINT("drmModePageFlip error [May not be error.]");
#else
					ERR_PRINT("drmModePageFlip error");
#endif
					display[dispid].first_draw = true;

					/* unregister listner */
					listner->buffer = NULL;
					it = list.end(); it--;
					list.erase(it);
				} else {
					result = true;
				}
			}

#if DRMSUPPORT_BLANK_DESKTOP
			if (!display[dispid].desktop_visible) {
				/* unblank */
				if (drmRcarDU_DesktopPlane(drm_fd, crt_id, 1)) {
					ERR_PRINT("drmRcarDU_DesktopPlane error.");
				} else {
					display[dispid].desktop_visible = true;
				}
			}
#endif
		}
	}

	return result;
}

/*! \brief display buffer to blank.
 *  \param[in] dispid  displayid
 *  \return result of processing
 *  \retval -1  error
 *  \retval 0   success
 *  \details
 *  display blank buffer that use display_pageflip.
 */
int DRMDisplay::display_pageflip_blank(int dispid)
{
	int res = -1;

	UNUSED(dispid);

#if DRMSUPPORT_BLANK_DESKTOP
	Mutex::Autolock _l(lock_flip);

	if (dispid < 0 || dispid > 2) {
		ERR_PRINT("invalid dispid");
	} else if (!display[dispid].desktop_visible) {
		DBG_PRINT("ignore blank");
	} else {
		if (drmRcarDU_DesktopPlane(drm_fd, display[dispid].crtc_id, 0)) {
			ERR_PRINT("drmRcarDU_DesktopPlane error.");
		} else {
			display[dispid].desktop_visible = false;
		}
		res = 0;
	}
#endif
	return res;
}


/*! \brief Page flip handler
 *  \param[in] fd file descriptor
 *  \param[in] frame frame sequence
 *  \param[in] sec second
 *  \param[in] usec micro second
 *  \param[in] data void pointer
 *  \return none
 *  \details
 *  this handler is called when the page flip finishes
 */
void DRMDisplay::page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
	FlipCallback  *arg   = (FlipCallback *)data;
	int            dispid = arg->dispid;
	DRMDisplay     *dsp   = arg->dsp;
	List< sp<FlipCallback> >::iterator it;
	List< sp<FlipCallback> > current_list;

	UNUSED(fd);
	UNUSED(frame);
	UNUSED(sec);
	UNUSED(usec);

	/* Signal fence */
	dsp->sync_timeline.inc();
#if DEBUG_ATRACE
	const char *name;
	if (dispid) {
		name = "page_flip_external";
	} else {
		name = "page_flip_primary";
	}
	ATRACE_NAME(name);
#endif

	/* copy list */
	{
		Mutex::Autolock _l(dsp->lock_flip);
		current_list = dsp->display[dispid].flip_listners;

		dsp->display[dispid].flip_listners.clear();
	}

	/* handle listner */
	while((it = current_list.begin()) != current_list.end()) {
		(*it)->buffer = NULL;
		(*it)->flip_callback();
		current_list.erase(it);
	}
}

/*! \brief Display thread
 *  \param[in] arg this pointer of C++.
 *  \return NULL
 *  \details
 *  loop is finished by terminate parameter
 */
void* DRMDisplay::_threadLoop(void *arg)
{
	class DRMDisplay *drm_disp = (class DRMDisplay *)arg;

	/* thread loop */
	drm_disp->threadLoop();

	return NULL;
}

/*! \brief Wait for display update
 *  \return none
 *  \details
 *  called from _threadLoop().
 */
void DRMDisplay::threadLoop(void)
{
	int rc;
	struct pollfd fds[2];

	/* main loop */
	fds[0].fd     = drm_fd;
	fds[0].events = POLLIN;
	fds[1].fd     = thread.get_waitevent_fd();
	fds[1].events = POLLIN;

	while (drm_fd >= 0) {
		rc = poll(fds, 2, -1);
		if (rc == -1) {
			ERR_PRINT("error poll drm events");
			continue;
		} else if (rc > 0) {
			if (fds[1].revents & POLLIN) {
				int rc = thread.receive_event();
				if (rc == HWCThread::TERMINATE) {
					/* terminate thread */
					break;
				}
			}
			if (fds[0].revents & POLLIN) {
#if DEBUG_ATRACE
				ATRACE_NAME("drmHandleEvent");
#endif
				DBG_PRINT("drmHandleEvent");
				drmHandleEvent(drm_fd, &evctx);
			}
		}
	}
}

/* default implementation of vsync listner class */

/*! \brief callback of vsync
 *  \param[in] aborted   vsync listner should aborted. [causes blank]
 *  \param[in] time      time of vsync in nano-second.
 *  \return none
 */
void DRMDisplay::VSyncCallback::vsync_callback(bool aborted, int64_t time)
{
	UNUSED(aborted);
	UNUSED(time);
}

/*! \brief destructor of vsync listner
 *  \return none
 */
DRMDisplay::VSyncCallback::~VSyncCallback()
{
}

/*! \brief add vsync listener
 *  \param[in] listner   point to vsync listner.
 *  \return processing result
 *  \retval 0  success.
 *  \retval -1 already registered or vsync not available.
 */
int DRMDisplay::add_vsync_listener(sp<VSyncCallback>& listner)
{
	bool            status;
	struct timespec time;

	/* register */
	Mutex::Autolock _l(lock_vsync);

	List< sp<VSyncCallback> >::iterator it;
	for(it = vsync_listners.begin(); it != vsync_listners.end(); it++) {
		if ((*it).get() == listner.get()) {
			return -1;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &time);
	listner->entry_time = time.tv_sec;
	listner->entry_time *= 1000000000;
	listner->entry_time += time.tv_nsec;

	status = vsync_listners.empty();
	vsync_listners.push_back(listner);

	/* request VSYNC if necessary */
	if (status) {
		request_next_vsync();
	}
	return 0;
}

/*! \brief remove vsync listner
 *  \param[in] listner   pointer to vsync listner.
 *  \return processing result
 *  \retval 0  success.
 *  \retval -1 not registered.
 */
int DRMDisplay::remove_vsync_listener(sp<VSyncCallback>& listner)
{
	List< sp<VSyncCallback> >::iterator it;
	bool status;

	/* unregister */
	status = false;
	Mutex::Autolock _l(lock_vsync);
	for(it = vsync_listners.begin(); it != vsync_listners.end(); it++) {
		if ((*it).get() == listner.get()) {
			*it = NULL;
			vsync_listners.erase(it);
			return 0;
		}
	}

	return -1;
}


/*! \brief Vsync thread
 *  \param[in] arg this pointer of C++.
 *  \return NULL
 *  \details
 *  loop is finished by terminate parameter
 */
void* DRMDisplay::_threadLoop_vsync(void *arg)
{
	class DRMDisplay *drm_disp = (class DRMDisplay *)arg;

	/* thread loop */
	drm_disp->threadLoop_vsync();

	return NULL;
}

/*! \brief Wait for vsync signal
 *  \return none
 *  \details
 *  called from _threadLoop_vsync().
 */
void DRMDisplay::threadLoop_vsync(void)
{
	int rc;
	struct pollfd fds[2];

	/* main loop */
	fds[0].fd     = drm_fd_vsync;
	fds[0].events = POLLIN;
	fds[1].fd     = thread_vsync.get_waitevent_fd();
	fds[1].events = POLLIN;

	while (drm_fd_vsync >= 0) {
		rc = poll(fds, 2, -1);
		if (rc == -1) {
			ERR_PRINT("error poll drm vsync events");
			continue;
		} else if (rc > 0) {
			if (fds[1].revents & POLLIN) {
				int rc = thread_vsync.receive_event();
				if (rc == HWCThread::TERMINATE) {
					/* terminate thread */
					break;
				}
			}
			if (fds[0].revents & POLLIN) {
#if DEBUG_ATRACE
				ATRACE_NAME("drmHandleEvent vsync");
#endif
				DBG_PRINT("drmHandleEvent vsync");
				drmHandleEvent(drm_fd_vsync, &evctx_vsync);
			}
		}
	}
}


/*! \brief Request vsync
 *  \return none
 */
void DRMDisplay::request_next_vsync(void)
{
	/* lock_vsync is already acquired. */

	if (!request_next) {
		drmVBlank       vbl;
		memset(&vbl, 0, sizeof(vbl));
		vbl.request.type = (drmVBlankSeqType) (DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT | (display[VSYNC_PRIM_DISP].high_crtc << DRM_VBLANK_HIGH_CRTC_SHIFT));
		vbl.request.sequence = 1;
		vbl.request.signal = (unsigned long)this;
		if (drmWaitVBlank(drm_fd_vsync, &vbl) == 0) {
			request_next = true;
		}
	}
}

/*! \brief Vsync handler
 *  \param[in] fd file descriptor
 *  \param[in] frame frame sequence
 *  \param[in] sec second
 *  \param[in] usec micro second
 *  \param[in] data void pointer
 *  \return none
 */
void DRMDisplay::vblank_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{

	class DRMDisplay *drm_disp = (DRMDisplay *)data;
	int64_t timestamp;

	timestamp = sec;
	timestamp *= 1000000;
	timestamp += usec;
	timestamp *= 1000;             /* convert usec to nsec */

	DBG_PRINT("vblank_handler frame:%d time:%d.%06d data:%p fd%x", frame, sec, usec, data, fd);

	{
		List< sp<VSyncCallback> > current_list;
		List< sp<VSyncCallback> >::iterator it;
		{
			Mutex::Autolock _l(drm_disp->lock_vsync);

			drm_disp->request_next = false;

			/* request VSYNC if necessary */
			if (!drm_disp->vsync_listners.empty()) {
				drm_disp->request_next_vsync();

				/* copy list */
				current_list = drm_disp->vsync_listners;
			}
		}

		/* handle listner */
		for(it = current_list.begin(); it != current_list.end(); it++) {
			sp<VSyncCallback>& cb = (*it);
			bool          ignore = false;
			if (cb->entry_time) {
				if (timestamp-(cb->entry_time) < 0) {
					/* ignore vsync if entry of lister is after event */
					ignore = true;
				}
				cb->entry_time = 0;
			}
			if (!ignore) {
				cb->vsync_callback(false, timestamp);
			}
		}
	}
}

int DRMDisplay::getfencefd()
{
	return sync_timeline.getFenceFd();
}
