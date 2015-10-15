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

#ifndef _HWC_INTERFACE_HWC_BASE
#define _HWC_INTERFACE_HWC_BASE

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <utils/String8.h>
using namespace android;

#include "base/disp_base.h"

#include "component/composer.h"
#include "component/layersel.h"

#include <fcntl.h>

/* configure the num of available native buffers, handled in HWC */
#if defined(TARGET_BOARD_KOELSCH) || defined(TARGET_BOARD_ALT)
#define MAX_NATIVEBUFFER_USED  2
#else
#define MAX_NATIVEBUFFER_USED  3
#endif

#ifndef UNUSED
#define UNUSED(X) ((void)&X)
#endif

/*! @class HWCBase
 *  @brief Control H/W composer event
 */
class HWCBase {
private:
	void shift_composer_fence(void);
	void prepare_config(hwc_display_contents_1_t* list, struct HWCLayerSelect::layer_statistics_t *info, struct HWCLayerSelect::layer_select_t *sel);
protected:
	/* interface */
	DisplayBase      *disp;
	HWCComposer          *composer;
	HWCLayerSelect       *layersel;

	/* states */
	int                   display_type;
	bool                  state;
	bool                  state_connect;
	bool                  state_blank;
	bool                  state_vsync;

	/* fence sync to wait */
	int                   composer_fence[MAX_NATIVEBUFFER_USED];
	unsigned int          max_composer_fence;

	bool                  force_geometry_changed;

	void registerDisplay(DisplayBase *obj);

	virtual bool onSetupLayersel(hwc_display_contents_1_t* list) {
		UNUSED(list);
		return true;
	}

	struct target_t {
		bool          use_fbt;
		bool          use_ovl;
		unsigned long physaddr;
		int           fd;
		int           fence;
		int           cached;
		void          *option;
	};

	virtual void onTargetPrepare(hwc_display_contents_1_t* list, struct target_t& info);
	virtual void onTargetExecute(hwc_display_contents_1_t* list, int composer_fence, int& drm_fence, struct target_t& info, int&  retire_fence);

	struct attributes_t {
		int hwc_display_vsync_period;
		int hwc_display_width;
		int hwc_display_height;
		int hwc_display_dpi_x;
		int hwc_display_dpi_y;
	}attributes;

public:
	int  set_connect_state(bool connect_state);
	bool register_display(void);

	void update_layerselmode(void);

	int wait_composer_fence(void);
	static void clear_acquire_fence(hwc_display_contents_1_t *list);
	static void set_release_fence(hwc_display_contents_1_t *list, int fence, int retire, bool use_fbt, struct HWCLayerSelect::layer_select_t *sel, int fence2);
	void dump_layerlog(hwc_display_contents_1_t *list);

	int prepare(hwc_display_contents_1_t* list);
	int set(hwc_display_contents_1_t* list);
	int eventControl(int event, int enabled);
	int blank(int blank);
	void dump(String8& msg);
	int DisplayConfigs(uint32_t* configs, size_t* numConfigs);
	int DisplayAttributes(uint32_t config, const uint32_t* attr, int32_t* values);

	/*! return state */
	inline int isValid(void) {
		return state;
	}

	HWCBase();
	virtual ~HWCBase();
};

#endif
