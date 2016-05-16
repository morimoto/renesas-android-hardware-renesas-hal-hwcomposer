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

#include "component/layersel.h"
#include "config.h"
#include <math.h>
#include <img_gralloc_public.h>
#include <cutils/log.h>
#include <utils/misc.h>

#define hwc_width(x)  ((x).right - (x).left)
#define hwc_height(x) ((x).bottom - (x).top)
#define min(x, y)     ((int)(x) < (int)(y) ? (x):(y))
#define min3(x, y, z) (min(min(x, y), z))

#define DEBUG_FLAG    USE_DBGLEVEL(4)

#define DO_NOT_SELECT_SWUSAGE    0

/* define for floatClipping. */
/* If crop values have decimal places, the layer is composited with OpenGL.*/
#define USE_GFX_DECIMALPLACES_CROP    0

/* define for support correct blending of coverage. */
/* if use this function, secure buffer of coverage may not displayed. */
#define USE_EXPERIMENTAL_COVERAGE_SUPPORT  1

/* define for return code of is_support_crop */
#define SUPPORT_CROP_INVALID   0
#define SUPPORT_CROP_VALID     1

/* define for return code of is_support_scaling function. */
#define SUPPORT_SCALING_TYPE_NOTSUPPORT    0
#define SUPPORT_SCALING_TYPE_NOSCALE       1
#define SUPPORT_SCALING_TYPE_SCALING       2


/*! \brief get crop from layer
 *  \param[out] crop  sourceCrop of integer type
 *  \param[in]  layer pointer to a hwc_layer_1_t structure
 *  \return none
 */
void HWCLayerSelect::get_sourcecropinfo(hwc_rect_t &crop, const hwc_layer_1_t *layer)
{
#if USE_HWC_VERSION1_3
	crop.top  = int(ceilf(layer->sourceCropf.top));
	crop.left = int(ceilf(layer->sourceCropf.left));
	crop.bottom = int(floorf(layer->sourceCropf.bottom));
	crop.right  = int(floorf(layer->sourceCropf.right));
#else
	crop = layer->sourceCropi;
#endif
}

/*! \brief Check displayFrame positions
 *  \param[out] crop  sourceCrop
 *  \param[out] win  displayFrame
 *  \param[in] layer_info  pointer to a hwc_layer_1_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  get crop/win values from layer_info
 */
int HWCLayerSelect::found_valid_displayrect(
	hwc_rect_t&          crop,         /* output */
	hwc_rect_t&          win,          /* output */
	hwc_layer_1_t        *layer_info)  /* input */
{
	get_sourcecropinfo(crop, layer_info);
	win  = layer_info->displayFrame;

	/* check strictly */
#if USE_GFX_DECIMALPLACES_CROP && USE_HWC_VERSION1_3
	if (layer_info->sourceCropf.left   != (float)crop.left  ||
		layer_info->sourceCropf.top    != (float)crop.top   ||
		layer_info->sourceCropf.right  != (float)crop.right ||
		layer_info->sourceCropf.bottom != (float)crop.bottom) {
		/* can not handle crop rect. */
		ALOGE_IF(DEBUG_FLAG,
			"sourceCrop (%.1f,%.1f,%.1f,%.1f) not integer", layer_info->sourceCropf.left, layer_info->sourceCropf.top, layer_info->sourceCropf.right, layer_info->sourceCropf.bottom);
		return -1;
	}
#endif

	/* check displayrect. */
	if ((win.left < 0) || (win.top < 0)
		|| (win.right > mode.fb_width) || (win.bottom > mode.fb_height)) {
		/* do not display images. */
		ALOGE_IF(DEBUG_FLAG,
			 "displayFrame (%d,%d,%d,%d) invalid", win.left, win.top, win.right, win.bottom);
		return -1;
	}
	return 0;
}


/*! \brief Adjust displayFrame position for aspect
 *  \param[out] display  displayFrame
 *  \return result of processing
 *  \retval 0       always return this value
 *  \details
 *  modify displayed range of displayFrame with aspect parameter
 */
int  HWCLayerSelect::adjust_displayrect_with_aspect(
	hwc_rect_t& display)
{
	int lines;
	int fb_width  = mode.fb_width;
	int fb_height = mode.fb_height;

	if (fb_height * aspect_param >= fb_width) {
		/* adjust vertically */
		lines = fb_width / aspect_param;
		ALOGE_IF(DEBUG_FLAG,
			"display rect with aspect (%d,%d)", fb_width, lines);

		if (lines >= fb_height) {
			/* nothing to do */
		} else {
			int offset = (fb_height - lines) / 2;
			display.top    = display.top    * lines / fb_height + offset;
			display.bottom = display.bottom * lines / fb_height + offset;
		}
	} else {
		/* adjust horizontally */
		lines = fb_height * aspect_param;
		ALOGE_IF(DEBUG_FLAG,
			"display rect with aspect (%d,%d)", lines, fb_height);

		if (lines >= fb_width) {
			/* nothing to do */
		} else {
			int offset = (fb_width - lines) / 2;
			display.left  = display.left  * lines / fb_width + offset;
			display.right = display.right * lines / fb_width + offset;
		}
	}
	ALOGE_IF(DEBUG_FLAG,
		"after displayFrame (%d,%d,%d,%d)", display.left, display.top, display.right, display.bottom);

	return 0;
}


/*! \brief Adjust displayFrame position for fullscreen
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval 0      not adjust position
 *  \retval 1      adjust position
 *  \details
 *  modify displayed range of displayFrame with fullscreen size
 */
int HWCLayerSelect::adjust_displayrect_fullscreen(hwc_display_contents_1_t *list)
{
	hwc_layer_1_t*      layer;
	int                 fb_width  = mode.fb_width;
	int                 fb_height = mode.fb_height;
	hwc_rect_t          temp_sourceCrop;
	hwc_rect_t          temp_displayFrame;
	float               scale;
	int                 display_w, display_h;
	IMG_native_handle_t *handle;
	int                 status;

	if (select.num_overlay != 1) {
		ALOGD_IF(DEBUG_FLAG,
			"not adjust position, because multiple-layer selected.");
		return 0;
	}

	layer = &list->hwLayers[select.ovl_index[0]];
	get_sourcecropinfo(temp_sourceCrop, layer);
	handle = (IMG_native_handle_t *)layer->handle;

	if (layer->transform & HAL_TRANSFORM_ROT_90) {
		/* set minimum scaling with pixel aspect 1:1 */
		if (hwc_height(temp_sourceCrop) * fb_height > hwc_width(temp_sourceCrop) * fb_width)
			scale = (float)fb_height / hwc_width(temp_sourceCrop);
		else
			scale = (float)fb_width / hwc_height(temp_sourceCrop);

		/* set display size */
		display_w = scale * hwc_height(temp_sourceCrop);
		display_h = scale * hwc_width(temp_sourceCrop);
	} else {
		/* set minimum scaling with pixel aspect 1:1 */
		if (hwc_width(temp_sourceCrop) * fb_height > hwc_height(temp_sourceCrop) * fb_width)
			scale = (float)fb_height / hwc_height(temp_sourceCrop);
		else
			scale = (float)fb_width / hwc_width(temp_sourceCrop);

		/* set display size */
		display_w = scale * hwc_width(temp_sourceCrop);
		display_h = scale * hwc_height(temp_sourceCrop);
	}

	/* set display position */
	temp_displayFrame.left  = (fb_width - display_w) / 2;
	temp_displayFrame.right = temp_displayFrame.left + display_w;
	temp_displayFrame.top   = (fb_height - display_h) / 2;
	temp_displayFrame.right = temp_displayFrame.top + display_h;

	/* adjust aspect ratio */
	if (mode.flags & MODE_FLAG_ASPECT) {
		adjust_displayrect_with_aspect(temp_displayFrame);
	}

	if (adjust_for_device(handle->iFormat, layer->transform, temp_sourceCrop, temp_displayFrame)) {
		ALOGD_IF(DEBUG_FLAG,
			"not adjust position, because can not display image.");
		return 0;
	}

	status = is_support_scaling(handle->iFormat, layer->transform, temp_sourceCrop, temp_displayFrame);
	if (status == SUPPORT_SCALING_TYPE_NOTSUPPORT) {
		/* can not support scaling */
		ALOGD_IF(DEBUG_FLAG,
			"not adjust position, because scaling not supported.");
		return 0;
	}

	ALOGE_IF(DEBUG_FLAG,
		"before crop (%d,%d,%d,%d)",
			select.ovl_sourceCrop[0].left, select.ovl_sourceCrop[0].top, select.ovl_sourceCrop[0].right, select.ovl_sourceCrop[0].bottom);
	ALOGE_IF(DEBUG_FLAG,
		"before displayFrame (%d,%d,%d,%d)",
			select.ovl_displayFrame[0].left, select.ovl_displayFrame[0].top, select.ovl_displayFrame[0].right, select.ovl_displayFrame[0].bottom);

	/* overwrite display position. */

	select.ovl_sourceCrop[0] = temp_sourceCrop;
	select.ovl_displayFrame[0] = temp_displayFrame;

	ALOGE_IF(DEBUG_FLAG,
		"after crop (%d,%d,%d,%d)",
			select.ovl_sourceCrop[0].left, select.ovl_sourceCrop[0].top, select.ovl_sourceCrop[0].right, select.ovl_sourceCrop[0].bottom);
	ALOGE_IF(DEBUG_FLAG,
		"after displayFrame (%d,%d,%d,%d)",
			select.ovl_displayFrame[0].left, select.ovl_displayFrame[0].top, select.ovl_displayFrame[0].right, select.ovl_displayFrame[0].bottom);

	return 1;
}

/*! \brief Check the format type whether it is YUV or not
 *  \param[in] format  format
 *  \return result of processing
 *  \retval 0      not YUV format
 *  \retval 1      YUV format
 */
int HWCLayerSelect::is_YUV_pixel_format(int format)
{
	return (format == HAL_PIXEL_FORMAT_YCbCr_422_SP) || \
		(format == HAL_PIXEL_FORMAT_NV12) ||
		(format == HAL_PIXEL_FORMAT_NV12_CUSTOM) ||
		(format == HAL_PIXEL_FORMAT_YV12) ||
		(format == HAL_PIXEL_FORMAT_NV21) ||
		(format == HAL_PIXEL_FORMAT_NV21_CUSTOM) ||
		(format == HAL_PIXEL_FORMAT_UYVY);
}

/*! \brief Check the format type whether it is available for imgpctrl driver
 *  \param[in] format  format
 *  \param[in] blend  blend
 *  \param[in] alpha  alpha
 *  \return result of processing
 *  \retval 0      not available
 *  \retval 1      available
 *  \details
 *  When this alpha isn't 0xFF, the layer can be handled by HWC(API v1.2).
 *  but if the image has an alpha channel, it is composited with OpenGL.
 */
int HWCLayerSelect::is_support_pixel_format(int format, int blend, int alpha)
{
	int n_entry= 0;
	const int *table = NULL;

	/* composer module doesn't support yuv formats so skip them */
	if (is_YUV_pixel_format(format))
		return false;

	if (mode.flags & MODE_FLAG_VSPDMODE) {
		/* only support UYVY withoutu alpha */
		if (alpha == 0xFF && blend == HWC_BLENDING_NONE) {
			static const int array[] = {
				HAL_PIXEL_FORMAT_NV12,
				HAL_PIXEL_FORMAT_NV12_CUSTOM,
//				HAL_PIXEL_FORMAT_YV12,
				HAL_PIXEL_FORMAT_NV21,
				HAL_PIXEL_FORMAT_NV21_CUSTOM,
				HAL_PIXEL_FORMAT_UYVY };
			n_entry = NELEM(array);
			table   = &array[0];
		}
	} else {
		if (alpha != 0xFF) {
			static const int array[] = {
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
				HAL_PIXEL_FORMAT_BGRX_8888,
#endif
				HAL_PIXEL_FORMAT_RGBX_8888,
				HAL_PIXEL_FORMAT_RGB_888,
				HAL_PIXEL_FORMAT_RGB_565,
				HAL_PIXEL_FORMAT_NV12,
				HAL_PIXEL_FORMAT_NV12_CUSTOM,
				HAL_PIXEL_FORMAT_YV12,
				HAL_PIXEL_FORMAT_NV21,
				HAL_PIXEL_FORMAT_NV21_CUSTOM,
				HAL_PIXEL_FORMAT_UYVY };
			n_entry = NELEM(array);
			table   = &array[0];
		} else {
			if (blend == HWC_BLENDING_PREMULT) {
				static const int array[] = {
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
					HAL_PIXEL_FORMAT_BGRX_8888,
#endif
					HAL_PIXEL_FORMAT_RGBX_8888,
					HAL_PIXEL_FORMAT_BGRA_8888,
					HAL_PIXEL_FORMAT_RGBA_8888 };
				n_entry = NELEM(array);
				table   = &array[0];
			} else if (blend == HWC_BLENDING_COVERAGE) {
				static const int array[] = {
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
					HAL_PIXEL_FORMAT_BGRX_8888,
#endif
					HAL_PIXEL_FORMAT_RGBX_8888,
					HAL_PIXEL_FORMAT_BGRA_8888,
					HAL_PIXEL_FORMAT_RGBA_8888 };
				n_entry = NELEM(array);
				table   = &array[0];
			} else if (blend == HWC_BLENDING_NONE) {
				static const int array[] = {
#ifdef HAL_PIXEL_FORMAT_BGRX_8888
					HAL_PIXEL_FORMAT_BGRX_8888,
#endif
					HAL_PIXEL_FORMAT_RGBA_8888,
					HAL_PIXEL_FORMAT_RGBX_8888,
					HAL_PIXEL_FORMAT_BGRA_8888,
					HAL_PIXEL_FORMAT_RGB_888,
					HAL_PIXEL_FORMAT_RGB_565,
					HAL_PIXEL_FORMAT_NV12,
					HAL_PIXEL_FORMAT_NV12_CUSTOM,
					HAL_PIXEL_FORMAT_YV12,
					HAL_PIXEL_FORMAT_NV21,
					HAL_PIXEL_FORMAT_NV21_CUSTOM,
					HAL_PIXEL_FORMAT_UYVY };
				n_entry = NELEM(array);
				table   = &array[0];
			}
		}
	}

	if (table) {
		int i;
		for (i = 0; i < n_entry; i++) {
			if (table[i] == format) {
				return true;
			}
		}
	}
	return false;
}

/*! \brief confirm vsp mode available.
 *  \param[in] width         source width  without crop.
 *  \param[in] height        source height without crop.
 *  \param[in] win           destination of window.
 *  \return processing result
 *  \retval 0   supproted
 *  \retval -1  not supported
 *  \details
 *  confirm vsp mode to check available.
 */
int HWCLayerSelect::is_support_vspmode(int width, int height, hwc_rect_t& win)
{
	if (mode.flags & MODE_FLAG_VSPDMODE) {
		int dst_width  = hwc_width(win);
		int dst_height = hwc_height(win);
		const unsigned long B=width    *height;
		const unsigned long C=dst_width*dst_height;


		/* check destination size */
		if (dst_width < 4 || dst_height < 4) {
			/* too small */
			return false;
		}

		if (dst_width > 2048 || dst_height > 2048) {
			/* too large */
			return false;
		}

		double tmp1, tmp2;
		tmp1 = mode.vsp_paramA1;
		tmp1 /= B;
		tmp1 *= mode.vsp_paramA2;
		tmp1 /= mode.vsp_dotclock;

		tmp2 = B;
		tmp2 /= C;
		if (tmp1 <= tmp2) {
			/* meet unable to handle VSP-D */
			return -1;
		}
	}
	return 0;
}

/*! \brief confirm destination and scaling parameters
 *  \param[in] dst_width      destination width
 *  \param[in] dst_height     destination height
 *  \param[in] src_width      source width
 *  \param[in] src_height     source height
 *  \param[in] scale_up       upper limit of scale up
 *  \param[in] scale_down     upper limit of scale down
 *  \return processing result
 *  \retval true   supproted
 *  \retval false  not supported
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for is_support_scaling function.
 */
bool HWCLayerSelect::is_support_scaling_dest_scale(int dst_width, int dst_height, int src_width, int src_height, int format)
{
	int scale_down, scale_up;

	/* for composer scalineg restriction. */
	switch (format) {
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_YV12:
	case HAL_PIXEL_FORMAT_NV12:
	case HAL_PIXEL_FORMAT_NV12_CUSTOM:
	case HAL_PIXEL_FORMAT_NV21:
	case HAL_PIXEL_FORMAT_NV21_CUSTOM:
	case HAL_PIXEL_FORMAT_UYVY:
		scale_down = 16;
		scale_up = 16;
		break;
	default:
		scale_down = 16;
		scale_up = 4;  /* scale is limited to get better image when partial image display. */
		break;
	}

	/* check destination size */
	if (dst_width < 4 || dst_height < 4) {
		/* too small */
		return false;
	}

	if (dst_width > mode.max_size || dst_height > mode.max_size || dst_width * dst_height > mode.max_area) {
		/* too large */
		return false;
	}

	/* check scaling */
	if (dst_width  > src_width  * scale_up   ||
		src_width  > dst_width  * scale_down ||
		dst_height > src_height * scale_up   ||
		src_height > dst_height * scale_down) {
		/* not supported scaling. */
		return false;
	}
	return true;
}

/*! \brief Check the scaling whether it's supported by imgpctrl driver
 *  \param[in] format  format
 *  \param[in] transform  transform
 *  \param[in] crop  sourceCrop
 *  \param[in] win  displayFrame
 *  \return result of processing
 *  \retval SUPPORT_SCALING_TYPE_NOTSUPPORT      not supported
 *  \retval SUPPORT_SCALING_TYPE_NOSCALE      supported (no scaling)
 *  \retval SUPPORT_SCALING_TYPE_SCALING      supported
 */
int HWCLayerSelect::is_support_scaling(int format, int transform, hwc_rect_t& crop, hwc_rect_t& win)
{
	int src_width, src_height;
	int dst_width, dst_height;

	src_width  = hwc_width(crop);
	src_height = hwc_height(crop);
	if (transform & HWC_TRANSFORM_ROT_90)
	{
		dst_height = hwc_width(win);
		dst_width  = hwc_height(win);
	} else {
		dst_width  = hwc_width(win);
		dst_height = hwc_height(win);
	}

	/* check source size */
	if (src_width < 4 || src_height < 4) {
		/* too small */
		return SUPPORT_SCALING_TYPE_NOTSUPPORT;
	}

	if (src_width > mode.max_size || src_height > mode.max_size || src_width * src_height > mode.max_area) {
		/* too large */
		return SUPPORT_SCALING_TYPE_NOTSUPPORT;
	}

	if (src_width == dst_width && src_height == dst_height) {
		/* same size */
		return SUPPORT_SCALING_TYPE_NOSCALE;
	}

	/* check destination size and scaling */

	if (!is_support_scaling_dest_scale(dst_width, dst_height, src_width, src_height, format)) {
		/* not supported scaling. */
		return SUPPORT_SCALING_TYPE_NOTSUPPORT;
	}

	return SUPPORT_SCALING_TYPE_SCALING;
}

/*! \brief Check the crop whether it's supported by imgpctrl driver
 *  \param[in] format  format
 *  \param[in] width  width
 *  \param[in] height  height
 *  \param[in] crop  sourceCrop
 *  \return result of processing
 *  \retval SUPPORT_CROP_INVALID      not supported
 *  \retval SUPPORT_CROP_VALID      supported
 */
int HWCLayerSelect::is_support_crop(int format, int width, int height, hwc_rect_t& crop)
{
	UNUSED(format);

	/* confirm left */
	if (crop.left < 0) {
		return SUPPORT_CROP_INVALID;
	}
	/* confirm top */
	if (crop.top < 0) {
		return SUPPORT_CROP_INVALID;
	}
	/* confirm right */
	if (crop.right > width) {
		return SUPPORT_CROP_INVALID;
	}
	/* confirm bottom */
	if (crop.bottom > height) {
		return SUPPORT_CROP_INVALID;
	}
	/* confirm order. */
	if (crop.left >= crop.right ||
		crop.top  >= crop.bottom) {
		return SUPPORT_CROP_INVALID;
	}

	return SUPPORT_CROP_VALID;
}

/*! \brief Adjust sourceCrop position for device by rounding up left/top crop and rounding down right/bottom crop
 *  \param[in] format  format
 *  \param[in] transform  transform
 *  \param[out] crop  sourceCrop
 *  \param[out] win  displayFrame
 *  \return result of processing
 *  \retval 0      not adjust position
 *  \retval 1      adjust position
 */
int HWCLayerSelect::adjust_for_device(int format, int transform, hwc_rect_t& crop, hwc_rect_t& win)
{
	int src_width_alignment, src_height_alignment;
	int mask;

	UNUSED(transform);
	UNUSED(win);

	switch (format) {
	case HAL_PIXEL_FORMAT_YV12:
	case HAL_PIXEL_FORMAT_NV12:
	case HAL_PIXEL_FORMAT_NV12_CUSTOM:
	case HAL_PIXEL_FORMAT_NV21:
	case HAL_PIXEL_FORMAT_NV21_CUSTOM:
		src_width_alignment  = 2;
		src_height_alignment = 2;
		break;
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_UYVY:
		src_width_alignment  = 2;
		src_height_alignment = 1;
		break;
	default:
		src_width_alignment  = 1;
		src_height_alignment = 1;
		break;
	}

	/* handle crop restriction. */
	mask = src_width_alignment-1;
	if (crop.left & mask) {
		crop.left = (crop.left + mask) & ~mask;
	}
	if (crop.right & mask) {
		crop.right = (crop.right)      & ~mask;
	}
	mask = src_height_alignment-1;
	if (crop.top & mask) {
		crop.top = (crop.top + mask)   & ~mask;
	}
	if (crop.bottom & mask) {
		crop.bottom = (crop.bottom)    & ~mask;
	}

	if (crop.left >= crop.right ||
		crop.top  >= crop.bottom)
	{
		ALOGD_IF(DEBUG_FLAG,
			"sourceCrop (%d,%d,%d,%d) invalid.", crop.left, crop.top, crop.right, crop.bottom);
		return -1;
	}
	return 0;
}

/*! \brief module for confirm layer supportable
 *  \param[in] layer      reference to hwc_layer_1_t structure
 *  \return processing result
 *  \retval true   supproted
 *  \retval false  not supported
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for is_supported function.
 */
bool HWCLayerSelect::is_supported_part1(hwc_layer_1_t &layer)
{
	IMG_native_handle_t *handle = (IMG_native_handle_t *)layer.handle;

	if (layer.flags & HWC_SKIP_LAYER) /* layer is handled by SurfaceFlinger. */
	{
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because SKIP_LAYER flag is on.");
		return false;
	}

	if (!handle) {
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because handle is NULL.");
		return false;
	}

	if ((handle->usage & GRALLOC_USAGE_HW_COMPOSER) == 0) {
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because usage is not for HWC.");
		return false;
	}
#if DO_NOT_SELECT_SWUSAGE
	if ((handle->usage & GRALLOC_USAGE_SW_WRITE_MASK) != 0) {
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because usage is using SW_WRITE.");
		return false;
	}
#endif

	{
		int alpha = 0xFF;
#if USE_HWC_VERSION1_2
		alpha = layer.planeAlpha;
#endif
		if (is_support_pixel_format(handle->iFormat, layer.blending, alpha) == 0) {
			ALOGD_IF(DEBUG_FLAG,
				 "format:%d, blending:%d alpha:%d not supported.", handle->iFormat, layer.blending, alpha);
			return false;
		}
	}
	hwc_rect_t temp_sourceCrop;
	get_sourcecropinfo(temp_sourceCrop, &layer);
	if (is_support_crop(handle->iFormat, handle->iWidth, handle->iHeight, temp_sourceCrop) == SUPPORT_CROP_INVALID) {
		ALOGE("crop (%d,%d)-(%d,%d) invalid. image size %dx%d.",
			temp_sourceCrop.left, temp_sourceCrop.top, temp_sourceCrop.right, temp_sourceCrop.bottom,
			handle->iWidth, handle->iHeight);
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because not supported crop pattern.");
		return false;
	}

	if (mode.flags & MODE_FLAG_EXTONLY) {
		/* need external disp flag */
		if (!(handle->usage & GRALLOC_USAGE_EXTERNAL_DISP)) {
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, because GRALLOC_USAGE_EXTERNAL_DISP is not uses.");
			return false;
		}
	}
	return true;
}

/*! \brief module for confirm layer supportable
 *  \param[in] i           index of layer
 *  \param[in] list        pointer to hwc_display_contents_1_t structure
 *  \param[in,out] dummy_i index of dummy layer
 *  \return none
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for is_supported_part2 function.
 */
void HWCLayerSelect::is_supported_part2a(int i, hwc_display_contents_1_t *list, int& dummy_i)
{
	size_t j;

	/* serach dummy layer */
	for (j = 0; j < list->numHwLayers - 1; j++) {
		size_t k;

		/* found valid index of dummy layer */
		if ((int)j == i) {
			/* same index is not dummy layer. */
			continue;
		}
		for (k = 0; k < select.num_overlay; k++) {
			if ((int)j == select.ovl_index[k]) {
				break;
			}
		}
		if (k != select.num_overlay) {
			/* already selcet overlay. */
			continue;
		}

		/* confirm dummy layer requirements */
		hwc_layer_1_t&      tmp_layer   = list->hwLayers[j];
		IMG_native_handle_t *tmp_handle = (IMG_native_handle_t *)tmp_layer.handle;

		/* layer information available. */
		if (tmp_layer.compositionType != HWC_FRAMEBUFFER || tmp_handle == NULL) {
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, because dummy layer %d not available.", j);
			continue;
		}

		/* format available. */
		switch(tmp_handle->iFormat) {
		case HAL_PIXEL_FORMAT_NV12:
		case HAL_PIXEL_FORMAT_NV12_CUSTOM:
		case HAL_PIXEL_FORMAT_NV21:
		case HAL_PIXEL_FORMAT_NV21_CUSTOM:
		case HAL_PIXEL_FORMAT_UYVY:
			k = 1;
			break;
		default:
			k = 0;
		}
		if (k == 0) {
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, format of dummy layer %d unexpected.", j);
			continue;
		}

		/* found fake plane */
		dummy_i = j;
		break;
	}
}

/*! \brief module for confirm layer supportable
 *  \param[in] i           index of layer
 *  \param[in] list        pointer to hwc_display_contents_1_t structure
 *  \param[out] use_crop   sourceCrop
 *  \param[out] use_win    displayFrame
 *  \param[in,out] dummy_i index of dummy layer
 *  \return processing result
 *  \retval true   supproted
 *  \retval false  not supported
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for is_supported function.
 */
bool HWCLayerSelect::is_supported_part2(int i, hwc_display_contents_1_t *list, hwc_rect_t &use_crop, hwc_rect_t& use_win, int& dummy_i)
{
	hwc_layer_1_t&      layer = list->hwLayers[i];
	IMG_native_handle_t *handle = (IMG_native_handle_t *)layer.handle;
	size_t j;

	/* check current target */
	for (j = 0; j < select.num_overlay; j++) {
		if (i == select.ovl_index[j]) {
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, layer %d already handled as HWC_OVERLAY.", i);
			return false;
		}
	}
	/* YUV plane is RIGHT-BOTTOM, and 1x1 size */
	if (layer.displayFrame.left   != mode.fb_width-1 ||
	    layer.displayFrame.top    != mode.fb_height-1 ||
	    layer.displayFrame.right  != mode.fb_width ||
	    layer.displayFrame.bottom != mode.fb_height) {
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, layer %d not satisfy fake plane requirement.", i);
		return false;
	}

	/* serach dummy layer */
	is_supported_part2a(i, list, dummy_i);

	if (dummy_i >= 0) {
		hwc_layer_1_t       tmp_layer = list->hwLayers[dummy_i];

#if USE_HWC_VERSION1_3
		tmp_layer.sourceCropf.left = 0.0f;
		tmp_layer.sourceCropf.top  = 0.0f;
		tmp_layer.sourceCropf.right  = handle->iWidth;
		tmp_layer.sourceCropf.bottom = handle->iHeight;
#else
		tmp_layer.sourceCropi.left = 0;
		tmp_layer.sourceCropi.top  = 0;
		tmp_layer.sourceCropi.right  = handle->iWidth;
		tmp_layer.sourceCropi.bottom = handle->iHeight;
#endif
		if (found_valid_displayrect(use_crop, use_win, &tmp_layer)) {
			/* failed to set actual use crop and display area. */
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, because no image displayed.");
			return false;
		}
		return true;
	}
	return false;
}

/*! \brief check the layer whether it's supported
 *  \param[in] i          index of layer
 *  \param[in] list       pointer to hwc_display_contents_1_t structure
 *  \param[out] use_crop  sourceCrop
 *  \param[out] use_win   displayFrame
 *  \param[out] dummy_i   index of dummy layer
 *  \return result of processing
 *  \retval 0      not supported
 *  \retval SUPPORT_SCALING_TYPE_NOSCALE      supported (no scaling)
 *  \retval SUPPORT_SCALING_TYPE_SCALING      supported (with scaling)
 */
int HWCLayerSelect::is_supported(int i, hwc_display_contents_1_t *list, hwc_rect_t &use_crop, hwc_rect_t& use_win, int& dummy_i)
{
	hwc_layer_1_t&      layer = list->hwLayers[i];
	IMG_native_handle_t *handle = (IMG_native_handle_t *)layer.handle;
	int status;

	/* no dummy layer */
	dummy_i = -1;

	if (!is_supported_part1(layer)) {
		/* layer not supported */
		return 0;
	}

	/* correct positions before confirm scaling. */
	if ((statistics.num_YUV > 1) && (mode.flags & MODE_FLAG_VSPDMODE) && (mode.flags & MODE_FLAG_FAKEPLANE)) {
		if (!is_supported_part2(i, list, use_crop, use_win, dummy_i)) {
			/* failed to set actual use crop and display area. */
			return 0;
		}
	} else {
		if (found_valid_displayrect(use_crop, use_win, &layer)) {
			/* failed to set actual use crop and display area. */
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, because no image displayed.");
			return 0;
		}
	}

	if (mode.flags & MODE_FLAG_ASPECT) {
		adjust_displayrect_with_aspect(use_win);
	}

	/* handle alignment depending pixel format */
	if (adjust_for_device(handle->iFormat, layer.transform, use_crop, use_win)) {
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because can not display image.");
		return 0;
	}

	/* confirm VSPDMODE */
	if (mode.flags & MODE_FLAG_VSPDMODE) {
		if (is_support_vspmode(hwc_width(use_crop), hwc_height(use_crop), use_win)) {
			ALOGD_IF(DEBUG_FLAG,
				"overlay not available, because meet VSP-D unusable condition.");
			return 0;
		}
	}

	/* confirm scaling */
	status = is_support_scaling(handle->iFormat, layer.transform, use_crop, use_win);
	if (status == SUPPORT_SCALING_TYPE_NOTSUPPORT) {
		/* can not support scaling */
		ALOGD_IF(DEBUG_FLAG,
			"overlay not available, because scaling not supported.");
		return 0;
	}

	return status;
}

/*! \brief Gather layer informations
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval statistics      layer_statistics_t structure
 *  \details
 *  count number of layer for YUV, RGB, other and external
 */
struct HWCLayerSelect::layer_statistics_t *HWCLayerSelect::gather_layerinfo(hwc_display_contents_1_t *list)
{
	size_t i;

	/* clear statistics */
	memset(&statistics, 0, sizeof(statistics));

	/* set default composition type. and counts. */
	for (i = 0; list && i < list->numHwLayers; i++) {
		hwc_layer_1_t       *layer  = &list->hwLayers[i];
		IMG_native_handle_t *handle = (IMG_native_handle_t *)layer->handle;

		/* check valid layer */
		if (layer->compositionType == HWC_BACKGROUND) {
			/* layer is not necessary to compose. */
			continue;
		}
		if (layer->compositionType == HWC_FRAMEBUFFER_TARGET) {
			/* update fb size */
			hwc_rect_t          temp_sourceCrop;
			get_sourcecropinfo(temp_sourceCrop, layer);
			mode.fb_width  = hwc_width(temp_sourceCrop);
			mode.fb_height = hwc_height(temp_sourceCrop);
			continue;
		}


		/* confirm blend type */
		if (layer->blending == HWC_BLENDING_COVERAGE) {
			statistics.num_COVERAGE++;
		}

		/* confirm layer format */
		if ((layer->flags & HWC_SKIP_LAYER) || /* layer is handled by SurfaceFlinger. */
			(handle == NULL)                || /* layer is not use valid buffer_handle_t */
			(handle->usage & GRALLOC_USAGE_HW_COMPOSER) == 0) {
			/* use default layer */
			statistics.num_OTHER ++;
		} else {
			int format = handle->iFormat;
			int alpha = 0xFF;
#if USE_HWC_VERSION1_2
			alpha = layer->planeAlpha;
#endif
			/* num of layers with EXTERNAL_DISP */
			if (handle->usage & GRALLOC_USAGE_EXTERNAL_DISP) {
				statistics.num_EXTDISP++;
			}
			/* num of layers for YUV, RGB */
			if (is_support_pixel_format(format, layer->blending, alpha)) {
				if (is_YUV_pixel_format(format)) {
					statistics.num_YUV++;
				} else {
					statistics.num_RGB++;
				}
			} else {
				statistics.num_OTHER ++;
			}
		}
		statistics.num_layer++;
	}
	ALOGD_IF(DEBUG_FLAG,
		"num of layers (YUV:%d, RGB:%d, OTHER:%d) sum:%d, YUV_EXT:%d",
		statistics.num_YUV, statistics.num_RGB, statistics.num_OTHER, statistics.num_layer, statistics.num_EXTDISP);

	return &statistics;
}


/*! \brief module for select layer
 *  \param[in] i              index of layer, only used for debug
 *  \param[in] list           pointer to hwc_display_contents_1_t structure
 *  \param[in] num_layer      reference to count of available layer
 *  \param[in] num_scaler     reference to count of available scaler
 *  \param[in] num_yuv        reference to count of available yuv layer (with scaler)
 *  \param[in] num_rotbuf     reference to count of available rotate buffer
 *  \param[in,out] layer_type type of layer.
 *  \return nothing
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for select_layer function.
 */
void HWCLayerSelect::select_layer_overlay(int i, hwc_display_contents_1_t *list, int& num_layer, int& num_scaler, int& num_yuv, int& num_rotbuf, int& layer_type)
{
	hwc_rect_t          temp_sourceCrop;
	hwc_rect_t          temp_displayFrame;
	int                 support_type;
	hwc_layer_1_t&      layer = list->hwLayers[i];
	IMG_native_handle_t *handle = (IMG_native_handle_t *)layer.handle;
	int                 dummy_i;

	UNUSED(i);

	support_type = is_supported(i, list, temp_sourceCrop, temp_displayFrame, dummy_i);
	if (!support_type) {
		ALOGD_IF(DEBUG_FLAG,
			"layer %d use HWC_FRAMEBUFFER.", i);
	} else if (num_rotbuf == 0 && layer.transform != 0) {
		ALOGD_IF(DEBUG_FLAG,
			"layer %d use HWC_FRAMEBUFFER. because resource for transform not enough.", i);
	} else if (select.use_fb && layer.blending != HWC_BLENDING_NONE) {
		/* after decided to use opengles, all overlay only available BLENDIND_NONE. */
		ALOGD_IF(DEBUG_FLAG,
			 "overlay not available for layer %d, because lower layer draw by opengl.", i);
	} else if (is_YUV_pixel_format(handle->iFormat)) {
		/* current layer is YUV format */

		if (num_yuv > 0) {
			/* select this layer to OVERLAY */
			layer_type = HWC_OVERLAY;

			select.ovl_index       [ select.num_overlay   ] = i;
			select.ovl_engine      [ select.num_overlay   ] = (mode.flags & MODE_FLAG_VSPDMODE) ? BLENDER_DRM:BLENDER_COMPOSER;
			select.ovl_sourceCrop  [ select.num_overlay   ] = temp_sourceCrop;
			select.ovl_displayFrame[ select.num_overlay++ ] = temp_displayFrame;

			if (dummy_i>=0) {
				/* use fake plane. */
				select.ovl_engine      [ select.num_overlay-1 ] = BLENDER_NOP;
				select.ovl_engine      [ select.num_overlay   ] = BLENDER_DRM;
				select.ovl_index       [ select.num_overlay   ] = dummy_i;
				select.ovl_sourceCrop  [ select.num_overlay   ] = select.ovl_sourceCrop  [ select.num_overlay-1 ];
				select.ovl_displayFrame[ select.num_overlay   ] = select.ovl_displayFrame[ select.num_overlay-1 ];
				select.num_overlay++;
				ALOGD_IF(DEBUG_FLAG,
					"layer %d's blend type select HWC_OVERLAY, for fake plane", i);
			}

			num_yuv--;
			if (layer.transform) {
				num_rotbuf--;
			}
		} else {
			ALOGD_IF(DEBUG_FLAG,
				"layer %d use HWC_FRAMEBUFFER. because resource for blend not enough.", i);
		}
	} else {
		/* current layer is RGB format */
		if (num_layer > 0 &&
			(support_type == SUPPORT_SCALING_TYPE_NOSCALE ||
			(num_scaler > 0 && (support_type == SUPPORT_SCALING_TYPE_SCALING)))) {
			/* select this layer to OVERLAY (RGB) */
			layer_type = HWC_OVERLAY;

			select.ovl_index       [ select.num_overlay   ] = i;
			select.ovl_engine      [ select.num_overlay   ] = (mode.flags & MODE_FLAG_VSPDMODE) ? BLENDER_DRM:BLENDER_COMPOSER;
			select.ovl_sourceCrop  [ select.num_overlay   ] = temp_sourceCrop;
			select.ovl_displayFrame[ select.num_overlay++ ] = temp_displayFrame;

			num_layer--;
			if (support_type == SUPPORT_SCALING_TYPE_SCALING) {
				num_scaler--;
			}
			if (layer.transform) {
				num_rotbuf--;
			}
		} else {
			ALOGD_IF(DEBUG_FLAG,
				"layer %d use HWC_FRAMEBUFFER. because resource for blend not enough.", i);
		}
	}
}


/*! \brief module for select layer
 *  \param[in] list               pointer to hwc_display_contents_1_t structure
 *  \param[in,out] num_layer      num of layer for overlay.
 *  \param[in,out] num_scaler     num of scaler for overlay
 *  \param[in,out] num_yuvlayer   num of yuv layer for overlay
 *  \return nothing
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for select_layer function.
 */
void HWCLayerSelect::select_layer_numlayer(hwc_display_contents_1_t *list, int& num_layer, int& num_scaler, int &num_yuvlayer)
{
	UNUSED(list);

	if ((int)statistics.num_layer > num_layer) {
		/* one layer reserved for frame buffer target */
		if (num_layer > 0)
			num_layer--;
	}
	/* one scaler reserved if display position adjust aspect. except num_layer is 1 */
	if (mode.flags & MODE_FLAG_ASPECT) {
		if (statistics.num_layer > 1) {
			if (num_scaler > 0)
				num_scaler--;
		}
	}

	if (statistics.num_COVERAGE) {
#if USE_EXPERIMENTAL_COVERAGE_SUPPORT
		/* there is blending type of COVERAGE,   */
		/* disable all overlays.                 */
		num_layer = 0;
#endif
	}

	if (mode.flags & MODE_FLAG_DISABLE) {
		/* layer not supported */
		num_layer = 0;
		num_scaler = 0;
	}

	if (statistics.num_YUV) {
		int min_available_yuvlayers = min3(statistics.num_YUV, num_scaler, num_layer);

		/* there is YUV layer */
		if (num_yuvlayer > min_available_yuvlayers) {
			num_yuvlayer = min_available_yuvlayers;
		}
		num_layer  -= num_yuvlayer;
		num_scaler -= num_yuvlayer;
	}

	/* layer select mode is for VSP-D */
	if (mode.flags & MODE_FLAG_VSPDMODE) {
		/* disable all overlay execept for yuv */
		num_layer = 0;
		num_scaler = 0;
		num_yuvlayer = 1;
	}
}


/*! \brief Select layer
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval select      layer_statistics_t structure
 *  \details
 *  select layer which is sent to imgpctrl driver
 */
struct HWCLayerSelect::layer_select_t *HWCLayerSelect::select_layer(hwc_display_contents_1_t *list)
{
	size_t i;
	int    num_layer  = mode.num_layer;
	int    num_scaler = mode.num_scaler;
	int    num_yuv    = mode.num_yuv;
	int    num_rotbuf = mode.max_rotbuf;

	/* update num_layer variable */
	select_layer_numlayer(list, num_layer, num_scaler, num_yuv);

	ALOGD_IF(DEBUG_FLAG,
		"num_layer:%d num_scaler:%d num_yuv:%d.", num_layer, num_scaler, num_yuv);

	/* reset select information. */
	select.num_overlay = 0;
	select.fbt_index   = -1;
	select.bglayer     = NULL;
	select.use_fb      = 0;

	for (i = 0; i < list->numHwLayers; i++) {
		hwc_layer_1_t&      layer = list->hwLayers[i];
		int                 layer_type = HWC_FRAMEBUFFER;
		hwc_rect_t          temp_sourceCrop;

		if (layer.compositionType == HWC_BACKGROUND) {
			select.bglayer = &layer;
			ALOGE_IF(i != 0, "HWC_BACKGROUND shoule present at first.");
			continue;
		}
		if (layer.compositionType == HWC_FRAMEBUFFER_TARGET) {
			if (select.fbt_index >= 0) {
				ALOGE_IF(DEBUG_FLAG,
					"FRAME_BUFFER_TARGET already found.");
			} else {
				select.fbt_index = i;

				get_sourcecropinfo(temp_sourceCrop, &layer);
				select.fb_sourceCrop   = temp_sourceCrop;
				select.fb_displayFrame = layer.displayFrame;
				if (mode.flags & MODE_FLAG_ASPECT) {
					adjust_displayrect_with_aspect(select.fb_displayFrame);
				}
			}
			continue;
		}

		/* try to assign overlay */
		select_layer_overlay(i, list, num_layer, num_scaler, num_yuv, num_rotbuf, layer_type);

		if (layer_type == HWC_FRAMEBUFFER) {
			/* some layer use OpenGL for blending. */
			select.use_fb = 1;
		}
		ALOGD_IF(DEBUG_FLAG,
			"layer %d's blend type select %d (%s)", i, layer_type,
			(layer_type == 0) ? "HWC_FRAMEBUFFER":"HWC_OVERLAY");
	}

	if ((mode.flags & MODE_FLAG_EXTONLY) && select.num_overlay) {
		/* reset flag of use_fb. */
		select.use_fb = 0;

		/* overwrite display position if available */
		adjust_displayrect_fullscreen(list);
	}

#if DEBUG_FLAG
	ALOGD("input layer");
	for (i = 0; i < list->numHwLayers; i++) {
		hwc_layer_1_t const* l = &list->hwLayers[i];

		ALOGD("\tpointer:%p type=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%.1f,%.1f,%.1f,%.1f}, {%d,%d,%d,%d}",
			l, l->compositionType, l->flags, l->handle, l->transform, l->blending,
#if USE_HWC_VERSION1_3
			l->sourceCropf.left, l->sourceCropf.top, l->sourceCropf.right, l->sourceCropf.bottom,
#else
			(float)l->sourceCropi.left, (float)l->sourceCropi.top, (float)l->sourceCropi.right, (float)l->sourceCropi.bottom,
#endif
			l->displayFrame.left, l->displayFrame.top, l->displayFrame.right, l->displayFrame.bottom);
	}
	ALOGD("select result");
	ALOGD("\tnum overlay:%d", select.num_overlay);
	for (i = 0; i < select.num_overlay; i++) {
		hwc_layer_1_t const* l = &list->hwLayers[select.ovl_index[i]];
		hwc_rect_t    const* c = &select.ovl_sourceCrop[i];
		hwc_rect_t    const* w = &select.ovl_displayFrame[i];
		ALOGD("\toverlay%d:%p {%d,%d,%d,%d}, {%d,%d,%d,%d}", i, l,
			c->left, c->top, c->right, c->bottom, w->left, w->top, w->right, w->bottom);
	}
	ALOGD("\tfbt_index:%d bglayer:%p", select.fbt_index, select.bglayer);
	ALOGD("\tuse_fb:%d", select.use_fb);
	if (select.fbt_index >= 0) {
		hwc_rect_t const* c = &select.fb_sourceCrop;
		hwc_rect_t const* w = &select.fb_displayFrame;
		ALOGD("\tfb_target {%d,%d,%d,%d}, {%d,%d,%d,%d}",
			c->left, c->top, c->right, c->bottom, w->left, w->top, w->right, w->bottom);
	}
#endif

	return &select;
}


/*! \brief Initialize layer and YUV number
 *  \param[in] num_layers  number of layers
 *  \param[in] num_yuv  number of YUVs
 *  \return none
 */
void HWCLayerSelect::init_numlayer(int num_layers, int num_yuv)
{
	mode.num_layer = num_layers;
	mode.num_yuv   = num_yuv;
}

/*! \brief Initialize flag
 *  \param[in] new_mode  mode flag
 *  \return none
 */
void HWCLayerSelect::init_flag(int new_mode)
{
	mode.flags = new_mode;
}

/*! \brief Initialize aspect
 *  \param[in] aspect_w  width aspect
 *  \param[in] aspect_h  height aspect
 *  \param[in] dpi_x  X dots per inch
 *  \param[in] dpi_y  Y dots per inch
 *  \return none
 */
void HWCLayerSelect::init_aspect(int aspect_w, int aspect_h, int dpi_x, int dpi_y)
{
	if (dpi_x <= 0 || dpi_y <= 0) {
		ALOGE("invalid dpi\n");
		dpi_x = 1;
		dpi_y = 1;
	}
	if (aspect_h * dpi_y <= 0 || aspect_w * dpi_x <= 0) {
		ALOGE("invalid aspect parameter\n");
		aspect_param = 1.0;
	} else {
		aspect_param = (float) (aspect_w * dpi_x) / (float) (aspect_h * dpi_y);
	}
}

/*! \brief Initialize flag
 *  \param[in] dotclock  dot clock for confirm vsp-d scaling
 *  \return none
 */
void HWCLayerSelect::init_vspparam(int dotclock)
{
	if (dotclock <= 0) {
		/* invalid param, use default */
		dotclock = mode.vsp_paramA1;
	}
	mode.vsp_dotclock = dotclock;
}

/*! \brief HWCLayerSelect initialize
 */
HWCLayerSelect::HWCLayerSelect(int size, int area, int rotbuf)
{
	/* initialize work */
	memset(&mode, 0, sizeof(mode));
	memset(&statistics, 0, sizeof(statistics));

	/* set default */
	mode.max_size = size;
	mode.max_area = area; /* total pixels */
	mode.max_rotbuf = rotbuf;
	mode.flags    = MODE_FLAG_DEFAULT;
	init_aspect(16, 9);

#if defined(TARGET_BOARD_LAGER) || defined(TARGET_BOARD_KOELSCH) || defined(TARGET_BOARD_ALT) || \
	defined(TARGET_BOARD_SALVATOR_M3) || defined(TARGET_BOARD_SALVATOR_H3)
	mode.num_layer  = 4;
	mode.num_scaler = 0;
	mode.num_yuv    = 1;
	mode.vsp_paramA1 = 1920*1080;
	mode.vsp_paramA2 = 148500000;
	mode.vsp_dotclock = mode.vsp_paramA1;
#else
	#error unknown target.
#endif
}
