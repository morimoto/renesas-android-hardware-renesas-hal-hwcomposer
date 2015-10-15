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

#ifndef _HWC_INTERFACE_LAYERSELECT
#define _HWC_INTERFACE_LAYERSELECT

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#ifndef UNUSED
#define UNUSED(X) ((void)&X)
#endif

/*! @class HWCLayerSelect
 *  @brief Select display methods from each layer
 */
class HWCLayerSelect {
private:
	void select_layer_numlayer(hwc_display_contents_1_t *list, int& num_layer, int& num_scaler, int& num_yuvlayer);
	void select_layer_overlay(int i, hwc_display_contents_1_t *list, int& num_layer, int& num_scaler, int& num_yuv, int& num_rotbuf, int& layer_type);
	bool is_support_scaling_dest_scale(int dst_width, int dst_height, int src_width, int src_height, int format);
	bool is_supported_part1(hwc_layer_1_t &layer);
	bool is_supported_part2(int i, hwc_display_contents_1_t *list, hwc_rect_t &use_crop, hwc_rect_t& use_win, int &dummy_i);
	void is_supported_part2a(int i, hwc_display_contents_1_t *list, int &dummy_i);

protected:
	/* mode parameter for layer select */
	struct resource_assign_t {
		/* initialize at constructor */
		int max_size;
		int max_area;
		int max_rotbuf;

		/* initialize at gather_layerinfo */
		int fb_width;
		int fb_height;

		/* parameter for layer select */
		int num_layer;
		int num_scaler;
		int num_yuv;
		int vsp_paramA1;
		int vsp_paramA2;
		int vsp_dotclock;
		int flags;
	} mode;

public:
	enum {
		MODE_FLAG_DEFAULT = 0,
		MODE_FLAG_EXTONLY = (1 << 0),
		MODE_FLAG_ASPECT  = (1 << 1),
		MODE_FLAG_DISABLE = (1 << 2),
		MODE_FLAG_VSPDMODE = (1 << 3),
		MODE_FLAG_FAKEPLANE = (1 << 4) /* depend on MODE_FLAG_VSPDMODE */
	};
	enum {
		BLENDER_COMPOSER,
		BLENDER_DRM,
		BLENDER_NOP
	};

	struct layer_statistics_t {
		unsigned int num_layer;
		unsigned int num_YUV;
		unsigned int num_EXTDISP;
		unsigned int num_RGB;
		unsigned int num_OTHER;
		unsigned int num_COVERAGE;
	} statistics;

	struct layer_select_t {
		unsigned int  num_overlay;
		int           ovl_index[4];
		int           ovl_engine[4];
		hwc_rect_t    ovl_sourceCrop[4];
		hwc_rect_t    ovl_displayFrame[4];

		hwc_layer_1_t *bglayer;

		int           fbt_index;
		hwc_rect_t    fb_sourceCrop;
		hwc_rect_t    fb_displayFrame;

		int           use_fb;
	} select;

	float aspect_param; /* desired_aspect_w / desired_aspect_h * dpi_x / dpi_y */

protected:
	void get_sourcecropinfo(hwc_rect_t &crop, const hwc_layer_1_t *layer);
	int found_valid_displayrect(
		hwc_rect_t&          crop,         /* input/ output */
		hwc_rect_t&          display,      /* input/ output */
		hwc_layer_1_t        *layer_info); /* input */

	int adjust_displayrect_with_aspect(
		hwc_rect_t&          display);     /* input/ output */

	int adjust_displayrect_fullscreen(hwc_display_contents_1_t *list);

	int is_YUV_pixel_format(int format);
	int is_support_pixel_format(int format, int blend, int alpha);
	int is_support_scaling(int format, int transform, hwc_rect_t& crop, hwc_rect_t& win);
	int is_support_crop(int format, int width, int height, hwc_rect_t& crop);
	int is_support_vspmode(int width, int height, hwc_rect_t& win);

	int adjust_for_device(int format, int transform, hwc_rect_t& crop, hwc_rect_t& win);

	int is_supported(int i, hwc_display_contents_1_t *list, hwc_rect_t &use_crop, hwc_rect_t& use_win, int &dummy_i);

public:
	struct layer_statistics_t *gather_layerinfo(hwc_display_contents_1_t *list);
	struct layer_select_t *select_layer(hwc_display_contents_1_t *list);

	void init_numlayer(int num_layers, int num_yuv);
	void init_flag(int mode);
	void init_aspect(int aspect_w, int aspect_x, int dpi_x = 1, int dpi_y = 1);
	void init_vspparam(int dotclock);

	HWCLayerSelect(int size, int area, int rotbuf);
};

#endif
