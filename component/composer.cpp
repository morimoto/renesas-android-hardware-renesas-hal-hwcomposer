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

#include "component/composer.h"
#include "config.h"

#include <fcntl.h>


#include <cutils/log.h>
#include "hal_public.h"
#include "gralloc_custom.h"

#define FLAG_FBTARGET_CONFIGURED   0x80

#define USE_NVCUSTOM_ALIGNMENT     1

/**********************************************
 utility to setup layerinformation
**********************************************/

/*! \brief Initialize geometry parameter
 *  \param[in] geometry  pointer to a geometry_t structure
 *  \param[in] layer  pointer to a hwc_layer_1_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 */
int HWCComposer::initialize_geometry(struct geometry_t& geometry, const hwc_layer_1_t& layer)
{
	int status = -EINVAL;
	IMG_native_handle_t *handle;
	int stride   = 0;
	int stride_c = 0;

	if (!layer.handle) {
		ALOGE("handle is NULL");
		goto err_exit;
	}

	int w, h;

	handle = (IMG_native_handle_t*)layer.handle;

	geometry.iFormat     = handle->iFormat;
	geometry.iWidth  = w = handle->iWidth;
	geometry.iHeight = h = handle->iHeight;

	w = ALIGN(w, HW_ALIGN);

	switch (handle->iFormat) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
	case HAL_PIXEL_FORMAT_BGRA_8888:
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
	case HAL_PIXEL_FORMAT_BGRX_8888:
#endif
		stride  = w * 4;
		break;
	case HAL_PIXEL_FORMAT_RGB_888:
		stride  = w * 3;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
		stride  = w * 2;
		break;
	case HAL_PIXEL_FORMAT_NV12_CUSTOM:
	case HAL_PIXEL_FORMAT_NV21_CUSTOM:
#if USE_NVCUSTOM_ALIGNMENT
		w = ALIGN(w, 128);
#endif
		/* fall through */
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_NV12:
	case HAL_PIXEL_FORMAT_NV21:
		stride   = w;
		stride_c = ((w + 1) / 2) * 2;
		break;
	case HAL_PIXEL_FORMAT_YV12:
		stride   = w;
		stride_c = ((w + 1) / 2);
		break;
	case HAL_PIXEL_FORMAT_UYVY:
		stride   = w * 2;
		break;
	default:
		ALOGE("format %d not supported.", handle->iFormat);
		goto err_exit;
	}

	geometry.iStride   = stride;
	geometry.iStride_C = stride_c;

	status = 0;

err_exit:
	return status;
}


/*! \brief Initialize geometry parameter
 *  \param[in] geometry  pointer to a geometry_t structure
 *  \param[in] fmt  format
 *  \param[in] w  width
 *  \param[in] h  height
 *  \param[in] stride  luminance stride
 *  \param[in] stride_c  chrominance stride
 *  \return result of processing
 *  \retval 0       always return this value
 */
int HWCComposer::initialize_geometry(struct geometry_t& geometry, int fmt, int w, int h, int stride, int stride_c)
{
	geometry.iFormat  = fmt;
	geometry.iHeight  = h;
	geometry.iWidth   = w;
	geometry.iStride   = stride;
	geometry.iStride_C = stride_c;

	return 0;
}


/*! \brief prepare execute hwc_setup_layerinformation of geometry parameter
 *  \param[in] fmt              format
 *  \param[in] w                width
 *  \param[in] h                height
 *  \param[in] stride           luminance stride
 *  \param[in] stride_c         chrominance stride
 *  \param[in] layer            reference to hwc_layer_1_t structure
 *  \param[out] offset          offset for blend
 *  \param[out] rtapi_fmt       format for blend
 *  \param[out] rtapi_yuv_type  yuv type for blend
 *  \param[out] rtapi_yuv_range yuv range for blend
 *  \param[out] num_plane       num of plane for blend
 *  \param[out] rtapi_premulti  premulti for blend
 *  \return result of processing
 *  \retval false   error
 *  \retval true    normal
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for hwc_setup_layerinformation function.
 */
bool HWCComposer::hwc_setup_layerinformation_prepare(
		int fmt, int w, int h, int stride, int stride_c, const hwc_layer_1_t& layer,
		unsigned long offset[3], int& rtapi_fmt, int& rtapi_yuv_type, int& rtapi_yuv_range, int& num_plane, int& rtapi_premulti)
{
	bool ret = true;

	rtapi_fmt = 0;

	rtapi_premulti = RT_GRAPHICS_PREMULTI_OFF;

	/* set offset */
	offset[0] = offset[1] = offset[2] = 0;

	/* set parameter for PREMULT */
	if (layer.blending == HWC_BLENDING_PREMULT) {
		if (fmt == HAL_PIXEL_FORMAT_RGBA_8888 ||
		    fmt == HAL_PIXEL_FORMAT_BGRA_8888) {
			rtapi_premulti = RT_GRAPHICS_PREMULTI_ON;
		}
	}

	/* use default yuv type */
	if (w * h >= 1280 * 720) {
		rtapi_yuv_type  = RT_GRAPHICS_COLOR_BT709;
		rtapi_yuv_range = RT_GRAPHICS_COLOR_COMPRESSED;
	} else {
		rtapi_yuv_type  = RT_GRAPHICS_COLOR_BT601;
		rtapi_yuv_range = RT_GRAPHICS_COLOR_COMPRESSED;
	}

	num_plane = 1;

	switch (fmt) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
		rtapi_fmt = RT_GRAPHICS_COLOR_ABGR8888;
		if (layer.blending == HWC_BLENDING_NONE) {
			/* alpha channel not used. */
			rtapi_fmt = RT_GRAPHICS_COLOR_XBGR8888;
		}
		break;
	case HAL_PIXEL_FORMAT_RGBX_8888:
		rtapi_fmt = RT_GRAPHICS_COLOR_XBGR8888;
		break;
	case HAL_PIXEL_FORMAT_BGRA_8888:
		rtapi_fmt = RT_GRAPHICS_COLOR_ARGB8888;
		if (layer.blending == HWC_BLENDING_NONE) {
			/* alpha channel not used. */
			rtapi_fmt = RT_GRAPHICS_COLOR_XRGB8888;
		}
		break;
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
	case HAL_PIXEL_FORMAT_BGRX_8888:
		rtapi_fmt = RT_GRAPHICS_COLOR_XRGB8888;
		break;
#endif
	case HAL_PIXEL_FORMAT_RGB_888:
		rtapi_fmt = RT_GRAPHICS_COLOR_RGB888;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
		rtapi_fmt = RT_GRAPHICS_COLOR_RGB565;
		break;
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
		rtapi_fmt = RT_GRAPHICS_COLOR_YUV422SP;
		num_plane = 2;

		/* apply offset for one FDs */
		offset[1] = offset[0] + stride * h;

		break;
	case HAL_PIXEL_FORMAT_NV12:
	case HAL_PIXEL_FORMAT_NV12_CUSTOM:
		w = ALIGN(w, HW_ALIGN);
		rtapi_fmt = RT_GRAPHICS_COLOR_YUV420SP;
		num_plane = 2;

		/* apply offset for one FDs */
		offset[1] = offset[0] + stride * h;

		break;
	case HAL_PIXEL_FORMAT_NV21:
	case HAL_PIXEL_FORMAT_NV21_CUSTOM:
		rtapi_fmt = RT_GRAPHICS_COLOR_YUV420SP_NV21;
		num_plane = 2;

		/* apply offset for one FDs */
		offset[1] = offset[0] + stride * h;

		break;
	case HAL_PIXEL_FORMAT_YV12:
		rtapi_fmt = RT_GRAPHICS_COLOR_YUV420PL;
		num_plane = 3;

		/* apply offset for one FDs */
		offset[2] = offset[0] + stride * h;
		offset[1] = offset[0] + stride * h + stride_c *((h+1)/2);

		/*********************************************
		The Cb/Cr address is replaced
		to deal with YV12 format using I420 format.
		*********************************************/

		break;
	case HAL_PIXEL_FORMAT_UYVY:
		rtapi_fmt = RT_GRAPHICS_COLOR_YUV422I_UYVY;
		break;
	default:
		ALOGE("format %d not supported.", fmt);
		ret = false;
	}
	return ret;
}

/*! \brief prepare execute hwc_setup_layerinformation of transform parameter
 *  \param[in] layer   reference to hwc_layer_1_t structure
 *  \param[out] rotate rotate for blend
 *  \param[out] mirror mirror for blend
 *  \return result of processing
 *  \retval false   error. this error only for detect syntax changed.
 *  \retval true    normal
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for hwc_setup_layerinformation function.
 */
bool HWCComposer::hwc_setup_layerinformation_prepare_transform(const hwc_layer_1_t& layer, int& rotate, int& mirror)
{
	switch (layer.transform & 7) {
	default:
	case 0:
		rotate = RT_GRAPHICS_ROTATE_0;
		mirror = RT_GRAPHICS_MIRROR_N;
		break;
	case HWC_TRANSFORM_ROT_90:
		rotate = RT_GRAPHICS_ROTATE_90;
		mirror = RT_GRAPHICS_MIRROR_N;
		break;
	case HWC_TRANSFORM_ROT_180:
		rotate = RT_GRAPHICS_ROTATE_180;
		mirror = RT_GRAPHICS_MIRROR_N;
		break;
	case HWC_TRANSFORM_ROT_270:
		rotate = RT_GRAPHICS_ROTATE_270;
		mirror = RT_GRAPHICS_MIRROR_N;
		break;
	case 0                     ^HWC_TRANSFORM_FLIP_H:
		rotate = RT_GRAPHICS_ROTATE_0;
		mirror = RT_GRAPHICS_MIRROR_H;
		break;
	case HWC_TRANSFORM_ROT_90  ^HWC_TRANSFORM_FLIP_H:
		rotate = RT_GRAPHICS_ROTATE_270;
		mirror = RT_GRAPHICS_MIRROR_H;
		break;
	case HWC_TRANSFORM_ROT_180 ^HWC_TRANSFORM_FLIP_H:
		rotate = RT_GRAPHICS_ROTATE_180;
		mirror = RT_GRAPHICS_MIRROR_H;
		break;
	case HWC_TRANSFORM_ROT_270 ^HWC_TRANSFORM_FLIP_H:
		rotate = RT_GRAPHICS_ROTATE_90;
		mirror = RT_GRAPHICS_MIRROR_H;
		break;
	}
	if (layer.transform & ~7) {
		/* transform syntax changed. */
		return false;
	} else {
		return true;
	}
}

/*! \brief prepare execute hwc_setup_layerinformation of offset parameter
 *  \param[in] fmt              format
 *  \param[in] stride           luminance stride
 *  \param[in] stride_c         chrominance stride
 *  \param[in] crop_x           x position
 *  \param[in] crop_y           y position
 *  \param[out] offset          offset for blend
 *  \return result of processing
 *  \retval false   error
 *  \retval true    normal
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for hwc_setup_layerinformation function.
 */
bool HWCComposer::hwc_setup_layerinformation_prepare_offset(int fmt, int stride, int stride_c, int crop_x, int crop_y, unsigned long offset[3])
{
	bool ret = true;

	switch (fmt) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
	case HAL_PIXEL_FORMAT_BGRX_8888:
#endif
	case HAL_PIXEL_FORMAT_BGRA_8888:
		offset[0] += crop_y * stride + crop_x * 4;
		break;
	case HAL_PIXEL_FORMAT_RGB_888:
		offset[0] += crop_y * stride + crop_x * 3;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
		offset[0] += crop_y * stride + crop_x * 2;
		break;
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
		offset[0] += crop_y * stride   + crop_x;
		offset[1] += crop_y * stride_c + (crop_x / 2) * 2;
		break;
	case HAL_PIXEL_FORMAT_NV12:
	case HAL_PIXEL_FORMAT_NV12_CUSTOM:
	case HAL_PIXEL_FORMAT_NV21:
	case HAL_PIXEL_FORMAT_NV21_CUSTOM:
		offset[0] += crop_y     * stride   + crop_x;
		offset[1] += crop_y / 2 * stride_c + (crop_x / 2) * 2;
		break;
	case HAL_PIXEL_FORMAT_YV12:
		offset[0] += crop_y     * stride   + crop_x;
		offset[1] += crop_y / 2 * stride_c + (crop_x / 2);
		offset[2] += crop_y / 2 * stride_c + (crop_x / 2);
		break;
	case HAL_PIXEL_FORMAT_UYVY:
		offset[0] += crop_y * stride + crop_x * 2;
		break;
	default:
		ALOGE("format %d not supported.", fmt);
		ret = false;
		break;
	}
	return ret;
}


/*! \brief Set layer information
 *  \param[in] image  pointer to a screen_grap_layer structure
 *  \param[in] crop  crop information
 *  \param[in] win  window information
 *  \param[in] layer  pointer to a hwc_layer_1_t structure
 *  \param[in] geometry  pointer to a geometry_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 *  \details
 *  set information to imgpctrl driver style
 */
int HWCComposer::hwc_setup_layerinformation(
	screen_grap_layer    *image,
	const hwc_rect_t&    crop,
	const hwc_rect_t&    win,
	const hwc_layer_1_t& layer,
	const struct geometry_t& geometry)
{
	int status = 0;
	unsigned long base_offset[3];

	/* default to initialize zero. */
	memset(image, 0, sizeof(*image));

	int fmt;
	int stride;
	int stride_c;
	int num_plane;

	{
		int w;
		int h;
		int crop_w;
		int crop_h;
		int rtapi_fmt;
		int rtapi_yuv_type;
		int rtapi_yuv_range;
		int rtapi_premulti;

		fmt       = geometry.iFormat;
		w         = geometry.iWidth;
		h         = geometry.iHeight;
		stride    = geometry.iStride;
		stride_c  = geometry.iStride_C;

		if (!hwc_setup_layerinformation_prepare(fmt, w, h, stride, stride_c, layer,
			base_offset, rtapi_fmt, rtapi_yuv_type, rtapi_yuv_range, num_plane, rtapi_premulti)) {
			/* not supported param */
			status = -EINVAL;
		}

		/* update crop */
		crop_w = hwc_width(crop);
		crop_h = hwc_height(crop);

		/* set layer image parameter */
		image->image.width      = crop_w;
		image->image.height     = crop_h;
		image->image.stride     = stride;
		image->image.stride_c   = stride_c;
		image->image.yuv_format = rtapi_yuv_type;
		image->image.yuv_range  = rtapi_yuv_range;
		image->image.format     = rtapi_fmt;
		image->premultiplied    = rtapi_premulti;

		/* set display position */
		{
			int win_width  = hwc_width(win);
			int win_height = hwc_height(win);

			image->rect.x      = win.left;
			image->rect.y      = win.top;
			image->rect.width  = win_width;
			image->rect.height = win_height;
			if (layer.transform & HWC_TRANSFORM_ROT_90) {
				/* turn 90 */
				if (crop_h == win_width && crop_w == win_height) {
					/* no need scaling. */
					image->rect.width  = 0;
					image->rect.height = 0;
				}
			} else {
				/* no turn */
				if (crop_w == win_width && crop_h == win_height) {
					/* no need scaling. */
					image->rect.width  = 0;
					image->rect.height = 0;
				}
			}
		}
	}

#if USE_HWC_VERSION1_2
	image->alpha = layer.planeAlpha;
#else
	/* set defaults */
	image->alpha = 0xFF;
#endif

	/* set transform parameter */
	{
		int rotate;
		int mirror;

		if (!hwc_setup_layerinformation_prepare_transform(layer, rotate, mirror)) {
			/* should not generate this error */
			ALOGE("design error about transform handling.");
		}

		image->rotate = rotate;
		image->mirror = mirror;
	}

	{
		int crop_x = crop.left;
		int crop_y = crop.top;

		/* adjust base address */
		if (!hwc_setup_layerinformation_prepare_offset(fmt, stride, stride_c, crop_x, crop_y, base_offset)) {
			ALOGE("format %d not supported.", fmt);
			status = -EINVAL;
		}

		if (num_plane >= 1) {
			image->image.address    = (unsigned char*)base_offset[0];
		}
		if (num_plane >= 2) {
			image->image.address_c0 = (unsigned char*)base_offset[1];
		}
		if (num_plane >= 3) {
			image->image.address_c1 = (unsigned char*)base_offset[2];
		}
	}

	return status;
}


/*! \brief pre process of settarget
 *  \param[in] format     format
 *  \param[in] width      width
 *  \param[in] height     height
 *  \param[in] arg_stride stride
 *  \param[out] stride    stride used to composition target.
 *  \return result of processing
 *  \retval false  error
 *  \retval true   normal
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for settarget function.
 */
bool HWCComposer::settarget_getstride(const int format, const int width, const int height, const int arg_stride, int& stride)
{
	/* confirm valid range */
	/* currently minimum of width and height is not necessary to confirm condition. */
#if 0
	if (width < 4 || height < 4) {
		ALOGD_IF(USE_DBGLEVEL(4),
			"target size %dx%d invalid.", width, height);
		return false;
	}
#endif
	if (width > max_size || height > max_size ||
		width * height > max_area) {
		ALOGD_IF(USE_DBGLEVEL(4),
			"target size %dx%d invalid.", width, height);
		return false;
	}

	/* confirm format */
	/* set default stride */
	if (arg_stride == 0) {
		switch(format) {
		case HAL_PIXEL_FORMAT_RGBA_8888:
		case HAL_PIXEL_FORMAT_RGBX_8888:
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
		case HAL_PIXEL_FORMAT_BGRX_8888:
#endif
		case HAL_PIXEL_FORMAT_BGRA_8888:
			stride = ALIGN(width, HW_ALIGN) * 4;
			break;
		case HAL_PIXEL_FORMAT_RGB_888:
			stride = ALIGN(width, HW_ALIGN) * 3;
			break;
		case HAL_PIXEL_FORMAT_UYVY:
		case HAL_PIXEL_FORMAT_RGB_565:
			stride = ALIGN(width, HW_ALIGN) * 2;
			break;
		case HAL_PIXEL_FORMAT_NV12_CUSTOM:
		case HAL_PIXEL_FORMAT_NV21_CUSTOM:
#if USE_NVCUSTOM_ALIGNMENT
			stride = ALIGN(width, 128);
			break;
#else
			/* fall through */
#endif
		case HAL_PIXEL_FORMAT_YCbCr_422_SP:
		case HAL_PIXEL_FORMAT_NV12:
		case HAL_PIXEL_FORMAT_NV21:
		case HAL_PIXEL_FORMAT_YV12:
			stride   = ALIGN(width, HW_ALIGN);
			break;
		default:
			ALOGD_IF(USE_DBGLEVEL(4),
				"format %d not supported.", format);
			return false;
		}
	} else {
		stride = arg_stride;
	}

	return true;
}


/*! \brief Set blend target information
 *  \param[in] format     format
 *  \param[in] width      width
 *  \param[in] height     height
 *  \param[in] arg_stride stride
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 */
int HWCComposer::settarget(const int format, const int width, const int height, const int arg_stride)
{
	int res = -1;
	geometry_t    geometry;
	hwc_layer_1_t layer;
	hwc_rect_t    sourceCropi;
	int stride_c;
	int stride;

	if (!settarget_getstride(format, width, height, arg_stride, stride)) {
		/* can not get stirde from argument. */
		return res;
	}

	/* set default stride_c from stride. */
	switch(format) {
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_NV12:
	case HAL_PIXEL_FORMAT_NV12_CUSTOM:
	case HAL_PIXEL_FORMAT_NV21:
	case HAL_PIXEL_FORMAT_NV21_CUSTOM:
		stride_c = stride;
		break;
	case HAL_PIXEL_FORMAT_YV12:
		stride_c = stride / 2;
		break;
	default:
		/* do not use stride_c */
		stride_c = 0;
		break;
	}

	layer.transform         = 0;
	layer.blending          = HWC_BLENDING_PREMULT;
#if USE_HWC_VERSION1_2
	layer.planeAlpha        = 0xFF;
#endif
	sourceCropi.left   = 0;
	sourceCropi.top    = 0;
	sourceCropi.right  = width;
	sourceCropi.bottom = height;
	layer.displayFrame.left   = 0;
	layer.displayFrame.top    = 0;
	layer.displayFrame.right  = width;
	layer.displayFrame.bottom = height;

	initialize_geometry(geometry, format, width, height, stride, stride_c);

	if (hwc_setup_layerinformation(&data.buffer[0], sourceCropi, layer.displayFrame, layer, geometry)) {
		ALOGD_IF(USE_DBGLEVEL(4),
			"layer parameter not support");
	} else {
		data.physAddress[0] = 0;
		data.buffer_fd[0]   = -1;
		res = 0;
	}
	return res;
}

bool HWCComposer::is_samebuffer(buffer_handle_t p)
{
	bool result = false;

	if (p == NULL) {
		/* invalid argument, assume not same buffer. */
		ALOGE("handle is NULL");
	} else {
		size_t i;
		for (i = 0; i < num_layer; i++) {
			if (data_handle[i] == p) {
				result = true;
				break;
			}
		}
	}
	return result;
}

/*! \brief Set background color
 *  \param[in] color  hwc_color_t structure
 *  \return result of processing
 *  \retval 0       always return this value
 */
int HWCComposer::setbgcolor(hwc_color_t &color)
{
	int a = color.a;
	int r = color.r;
	int g = color.g;
	int b = color.b;
	if (a != 255) {
		r = r * a / 255;
		g = g * a / 255;
		b = b * a / 255;
	}
	data.bgcolor = b | (g << 8) | (r << 16) | (0xff << 24);

	/* bgcolor changed. */
	prev_drawbgcolor = false;
	return 0;
}

/*! \brief Set blend target buffer
 *  \param[in] phys   physical address
 *  \param[in] fd     file descriptor
 *  \param[in] fence  file descriptor
 *  \param[in] cached cache flag
 *  \return result of processing
 *  \retval 0       always return this value
 */
int HWCComposer::settarget_buffer(unsigned long phys, int fd,  int fence, int cached)
{
	data.physAddress[0]   = phys;
	data.buffer_fd[0]     = fd;
	data.acquire_fd       = fence;
	data.buffer_cached[0] = cached;
	return 0;
}


/*! \brief initialize target
 *  \return none
 */
void HWCComposer::init(void)
{
	data.num_buffer = 1;
	num_layer = 0;
	fbtarget_layer = -1;
	data.bgcolor = 0xff000000;
	memset(&data_handle[0], 0, sizeof(data_handle));
}

/*! \brief Add layer
 *  \param[in] list   hwc_display_contents_1_t structure
 *  \param[in] index  layer index
 *  \param[in] crop   crop
 *  \param[in] disp  displayframe
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  add layerinformation as many as number of overlay
 */
int HWCComposer::addlayer(hwc_display_contents_1_t *list, int index, hwc_rect_t& crop, hwc_rect_t& disp)
{
	hwc_layer_1_t& layer = list->hwLayers[index];
	int res = -1;

	if (num_layer >= COMPOSER_SRC) {
		ALOGE("too much layer");
	} else {
		data.physAddress[1+num_layer] = 0;
		data.buffer_fd[1+num_layer]   = -1;

		composition_index[num_layer]  = index;

		if (layer.compositionType == HWC_FRAMEBUFFER ||
			layer.compositionType == HWC_OVERLAY) {
			if (layer.handle == NULL) {
				ALOGE("handle is not available");
			} else {
				IMG_native_handle_t *handle = (IMG_native_handle_t*)layer.handle;
				geometry_t  geometry;

				initialize_geometry(geometry, layer);

				if (hwc_setup_layerinformation(&data.buffer[1+num_layer], crop, disp, layer, geometry)) {
					ALOGE("layer parameter not support");
				}

				data.buffer_fd[1+num_layer]   = handle->fd[0];
			}
		} else if (layer.compositionType == HWC_FRAMEBUFFER_TARGET) {
			fbtarget_layer = num_layer;
			fb_crop    = crop;
			fb_display = disp;
		} else {
			ALOGE("compositiontype shoule be HWC_OVERLAY or FRAMEBUFFER_TARGET");
		}
		num_layer++;
		res = 0;
	}
	return res;
}

/*! \brief set parameter for request
 *  \param[in,out] index      counter
 *  \param[in]     layer      pointer to hwc_layer_1_t structure
 *  \param[in,out] blendlayer pointer to array of hwc_layer_1_t structure
 *  \param[in,out] need_blend flag indicate need blend
 *  \return result of processing
 *  \retval false   error
 *  \retval true    normal
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for request function.
 */
bool HWCComposer::request_setparam(
		unsigned int& index,
		hwc_layer_1_t *layer,
		hwc_layer_1_t *blendlayer[COMPOSER_SRC],
		bool&         need_blend)
{
	bool ret = true;

	/* if HWC access to buffer to debug functionality,
	 * at that case should set GRALLOC_USAGE_SW_READ_MASK */
	const int sw_usage_mask = GRALLOC_USAGE_SW_WRITE_MASK;

	IMG_native_handle_t *handle = (IMG_native_handle_t*)layer->handle;

	if (layer->compositionType == HWC_FRAMEBUFFER_TARGET) {
		if (fbtarget_layer < 0) {
			/* continue processing */
		} else if (handle == NULL) {
			ALOGE("handle is not available");
			/* continue processing */
		} else if (index >= COMPOSER_SRC) {
			ALOGE("invalid range of index");
			ret =false;
		} else {
			if (!(fbtarget_layer & FLAG_FBTARGET_CONFIGURED)) {
				geometry_t geometry;

				initialize_geometry(geometry, *layer);

				if (hwc_setup_layerinformation(&data.buffer[1+fbtarget_layer], fb_crop, fb_display, *layer, geometry)) {
					ALOGE("layer parameter not support");
				}
				fbtarget_layer |= FLAG_FBTARGET_CONFIGURED;
			}

			blendlayer[index] = layer;

			data.buffer_fd[1+index] = handle->fd[0];
			usefence.merge(layer->acquireFenceFd);

			/* handle of FRAMEBUFFER_TARGET is always different. */
			need_blend = true;
			data.buffer_cached[1+index] = !!(handle->usage & sw_usage_mask);

			index++;
		}
	} else if (layer->compositionType == HWC_OVERLAY) {
		if (index >= COMPOSER_SRC) {
			ALOGE("invalid range of index");
			ret =false;
		} else {
			blendlayer[index] = layer;

			data.buffer_fd[1+index] = handle->fd[0];
			usefence.merge(layer->acquireFenceFd);

			/* handle of OVERLAY may be unchanged. */
			if (is_samebuffer(layer->handle)) {
				/* compose the same buffer previously used. */
				data.buffer_cached[1+index] = false;
			} else {
				need_blend = true;
				data.buffer_cached[1+index] = !!(handle->usage & sw_usage_mask);
			}

			index++;
		}
	}
	return ret;
}

/*! \brief Request composition
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval fence file descriptor       normal
 *  \retval -1      error
 *  \details
 *  send a request of composition to composer driver
 */
int HWCComposer::request(hwc_display_contents_1_t *list)
{
	int release_fd = -1;
	unsigned int i, index;
	hwc_layer_1_t *blendlayer[COMPOSER_SRC];
	bool need_blend;

	if (!allow_bgcolor_only && num_layer==0) {
		return -1;
	}

	/* update buffer handle and sync fence. */
	index = 0;

	/* to keep blending order, pass previous fence to composer. */
	usefence.merge(prev_fence);

	/* merge fence of target buffer */
	usefence.merge(data.acquire_fd);

	if (num_layer == 0) {
		if (!prev_drawbgcolor) {
			/* draw bgcolor */
			need_blend = true;
		} else {
			/* already draw bgcolor */
			need_blend = false;
		}
	} else {
		/* may be use composition result. */
		need_blend = false;
	}

	if (list->outbuf) {
		/* for virtual, always need blend */
		need_blend = true;
	}

	for (i = 0; i < num_layer; i++) {
		if (composition_index[i] >= (int)list->numHwLayers) {
			/* abort processing */
			break;
		} else {
			hwc_layer_1_t *layer = &list->hwLayers[composition_index[i]];

			if (!request_setparam(index, layer, blendlayer, need_blend)) {
				/* abort processing */
				break;
			}
		}
	}

	/* set merged fence */
	data.acquire_fd = usefence.get();

	if (num_layer != index) {
		ALOGE("interface problem between prepare to set.");
		num_layer = 0;
	}
	data.num_buffer = 1 + num_layer;

	/* do request */
	if (need_blend) {
		release_fd = ioctl(fd, CMP_IOC_POST, &data);
		if (release_fd < 0) {
			ALOGE("error CMP_IOC_POST");
		} else {
			prev_drawbgcolor = (num_layer == 0);
		}
	}

	/* clear merged fence */
	usefence.clear();
	data.acquire_fd = -1;

	if (prev_fence >= 0) {
		close(prev_fence);
		prev_fence = -1;
	}

	/* handle fence object. releaseFence. */
	if (release_fd >= 0) {
		/* record previous fence */
		prev_fence = dup(release_fd);
	}

	for (i = 0; i < index; i++) {
		hwc_layer_1_t *layer = blendlayer[i];

		/* record handle */
		if (release_fd >= 0) {
			data_handle[i] = layer->handle;
		} else {
			data_handle[i] = NULL;
		}
	}

	return release_fd;
}

/*! \brief Dump layer information
 *  \param[in] msg  massage
 *  \return none
 */
void HWCComposer::dump(String8 &msg)
{
	int i;
	screen_grap_layer *layer;

	msg.appendFormat("    size     |strides   |fmt-yuv|window             |tra|alp|offset  |handle    |C|fds|\n");

	layer = &data.buffer[0];

	if (data.num_buffer > 0) {
		msg.appendFormat("    %4d,%4d|%5d,%4d|%3d,%d,%d|-------------------|---|---|%8p|----------|-|---|\n",
			layer->image.width,  layer->image.height,
			layer->image.stride, layer->image.stride_c,
			layer->image.format, layer->image.yuv_format, layer->image.yuv_range,
			layer->image.address);
	}

	for (i = 1; i < data.num_buffer; i++) {
		hwc_rect win;
		int w, h;

		layer = &data.buffer[i];

		win.left = layer->rect.x;
		win.top  = layer->rect.y;
		w        = layer->rect.width;
		h        = layer->rect.height;

		if (w == 0 && h == 0) {
			if (layer->rotate == RT_GRAPHICS_ROTATE_90 || layer->rotate == RT_GRAPHICS_ROTATE_270) {
				h = layer->image.width;
				w = layer->image.height;
			} else {
				w = layer->image.width;
				h = layer->image.height;
			}
		}
		win.right = win.left + w;
		win.bottom = win.top + h;

		msg.appendFormat("    %4d,%4d|%5d,%4d|%3d,%d,%d|%4d,%4d-%4d,%4d|%d,%d|%3d|%8p|%10p|%1d|%3d|\n",
			layer->image.width,  layer->image.height,
			layer->image.stride, layer->image.stride_c,
			layer->image.format, layer->image.yuv_format, layer->image.yuv_range,
			win.left, win.top, win.right, win.bottom,
			layer->rotate, layer->mirror, layer->alpha,
			layer->image.address,
			data_handle[i-1], data.buffer_cached[i], data.buffer_fd[i]);
	}
}

/*! \brief control enable of composition "blending"
 *  \param[in] flag  blending allow if true.
 *  \return none
 */
void HWCComposer::enable(bool flag)
{
	if (flag) {
		data.operation_type = OPTYPE_BLENDONLY;
	} else {
		data.operation_type = OPTYPE_DMACOPY;
	}
}

/*! \brief control enable of composition "back ground color only"
 *  \param[in] flag  only bgcolor allow if true.
 *  \return none
 */
void HWCComposer::enable_bgonly(bool flag)
{
	allow_bgcolor_only = flag;
}


/*! \brief HWCComposer initialize
 */
HWCComposer::HWCComposer():
	allow_bgcolor_only(true),
	num_layer(0),
	fbtarget_layer(-1),
	prev_drawbgcolor(false),
	prev_fence(-1)
{
	fd = open("/dev/composer", O_RDWR);

	memset(&data, 0 , sizeof(data));
	data.operation_type = OPTYPE_BLENDONLY;

	{
		hwc_register_composerinfo info;

		memset(&info, 0, sizeof(info));
		if (ioctl(fd, CMP_IOCGS_REGISTER, &info)) {
			ALOGE("error CMP_IOCGS_REGISTER");
			max_size = 1920;
			max_area = 1920*1088;
			max_rotbuf = 0;
		} else {
			max_size = info.max_size;
			max_area = info.max_area;
			max_rotbuf = info.max_rotbuf;
		}
	}
}

/*! \brief HWCComposer destructor
 */
HWCComposer::~HWCComposer()
{
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
	if (prev_fence >= 0) {
		close(prev_fence);
		prev_fence = -1;
	}
}

