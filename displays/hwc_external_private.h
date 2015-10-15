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

#ifndef __HWC_EXTERNAL_PRIVATE_H
#define __HWC_EXTERNAL_PRIVATE_H

/* USE_EXTERNAL declare in hwc_external.h */
#include "displays/hwc_external.h"

#if USE_EXTERNAL

#define USE_EXTERNAL_VSYNC 0 /* surface flinger not supported. */

#include "component/drm_display.h"

/***************************/
/* declare display handler */
/***************************/
#include "base/disp_base.h"

#define NUM_MAX_EXTERNAL_BUFFER 3

/*! @class DisplayExternal
 *  @brief Update external display
 */
class DisplayExternal : public DisplayBase {
	sp<DRMDisplay::FlipCallback>  flip;

	struct buffer_t {
		hwc_disp_buffer           buffer;
		sp<DRMDisplay::IonBuffer> drm_buffer;
	} bufdata[NUM_MAX_EXTERNAL_BUFFER];

	struct buffer_t *current;

	DRMDisplay *dsp;
	struct drm_attributes_t hwc_attr;

	HWCNotice               *notice;
	Mutex                   lock;
	Condition               cond_flip_flag;
	bool                    vsync_enable;

protected:
	/* virtual function of base class */
	void onUpdateDisplay(hwc_disp_buffer *buf, bool no_plane);
	/* virtual function of DRMDisplay class */
	void flip_callback(void);

public:

	/* virtual function of base class */
	int setBlank(bool state);
	int get_attribute_size(int *width, int *height, int *stride);
	int get_attribute_period(int *period);
	int get_attribute_dpi(float *dpi_x, float *dpi_y);
	int get_attribute_dotclock(uint64_t *dc);

	bool set_displaysize(int width, int height);

	bool isValid(void);

	DisplayExternal(HWCNotice *obj, int display, DRMDisplay *drm_disp);
	~DisplayExternal();
};

/****************************/
/* declare hot-plug handler */
/****************************/
#include "base/hotplug_base.h"

class HWCExternal;

/*! @class HWCHotplug
 *  @brief Control external Hotplug event
 */
class HWCHotplug : public HotplugBase {
	HWCExternal *ext_base;
/* protected */
protected:
	int onInitHotplug(void);
	int onEventHotplug(void);

/* public */
public:
	bool isValid(void);

	HWCHotplug(HWCNotice *obj, int disp, HWCExternal *base);
	~HWCHotplug();
};

#endif

#endif
