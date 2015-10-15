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

#ifndef __IMGPCTRL_PUBLIC_H__
#define __IMGPCTRL_PUBLIC_H__

/*
 * Definition
 */

/* color_format */
#define RT_GRAPHICS_COLOR_YUV422SP	(1)	/* YUV422 SemiPlanar */
#define RT_GRAPHICS_COLOR_YUV420SP	(2)	/* YUV420 SemiPlanar */
#define RT_GRAPHICS_COLOR_RGB565	(5)	/* RGB565 */
#define RT_GRAPHICS_COLOR_RGB888	(6)	/* RGB888 */
#define RT_GRAPHICS_COLOR_ARGB8888	(7)	/* aRGB8888 */
#define RT_GRAPHICS_COLOR_ABGR8888	(8)	/* aBGR8888 */
#define RT_GRAPHICS_COLOR_YUV420PL	(12)	/* YUV420 Planar */
#define RT_GRAPHICS_COLOR_XRGB8888	(13)	/* xRGB8888 */
#define RT_GRAPHICS_COLOR_XBGR8888	(14)	/* xBGR8888 */
#define RT_GRAPHICS_COLOR_YUV420SP_NV21	(15)	/* YUV420 SemiPlanar(NV21) */
#define RT_GRAPHICS_COLOR_YUV422I_UYVY	(16)	/* YUV422 Interleaved(UYVY) */

/* yuv_format */
#define RT_GRAPHICS_COLOR_BT601		(0)	/* ITU-R BT.601 */
#define RT_GRAPHICS_COLOR_BT709		(1)	/* ITU-R BT.709 */

/* yuv_range */
#define RT_GRAPHICS_COLOR_COMPRESSED	(0)	/* Compressed-Range */
#define RT_GRAPHICS_COLOR_FULLSCALE	(1)	/* Fullscale-Range */

/* Rotation type */
#define RT_GRAPHICS_ROTATE_0		(1)	/* Non-rotation */
#define RT_GRAPHICS_ROTATE_90		(2)	/* Angle of 90 degrees */
#define RT_GRAPHICS_ROTATE_180		(3)	/* Angle of 180 degrees */
#define RT_GRAPHICS_ROTATE_270		(4)	/* Angle of 270 degrees */

/* Mirror type */
#define RT_GRAPHICS_MIRROR_N		(1)	/* Non-inversion */
#define RT_GRAPHICS_MIRROR_V		(2)	/* Up Down inversion */
#define RT_GRAPHICS_MIRROR_H		(4)	/* Right Left inversion */

/* premultiplied */
#define RT_GRAPHICS_PREMULTI_OFF	(1)	/* premultiplied OFF */
#define RT_GRAPHICS_PREMULTI_ON		(2)	/* premultiplied ON */

/* RT-API result code */
#define SMAP_LIB_GRAPHICS_OK		(0)
#define SMAP_LIB_GRAPHICS_NG		(-1)
#define SMAP_LIB_GRAPHICS_PARAERR	(-2)
#define SMAP_LIB_GRAPHICS_SEQERR	(-3)
#define SMAP_LIB_GRAPHICS_MEMERR	(-4)

/*
 * Structure
 */

struct screen_rect {
	unsigned short			x;
	unsigned short			y;
	unsigned short			width;
	unsigned short			height;
};

struct screen_grap_image_param {
	unsigned short			width;
	unsigned short			height;
	unsigned short			stride;
	unsigned short			stride_c;
	unsigned int			format;
	unsigned short			yuv_format;
	unsigned short			yuv_range;
	unsigned char			*address;
	unsigned char			*address_c0;
	unsigned char			*address_c1;
};

struct screen_grap_layer {
	struct screen_grap_image_param	image;
	struct screen_rect		rect;
	unsigned short			alpha;
	unsigned short			rotate;
	unsigned short			mirror;
	unsigned short			dummy;
	long				key_color;
	unsigned short			premultiplied;
	unsigned short			alpha_coef;
	unsigned char			*palette;
	unsigned long			palette_size;
	unsigned char			*alpha_plane;
};

#endif	/* __IMGPCTRL_PUBLIC_H__ */
