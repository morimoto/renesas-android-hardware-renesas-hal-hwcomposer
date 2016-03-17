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

#ifndef __HWC_PRIMARY_PRIVATE_H
#define __HWC_PRIMARY_PRIVATE_H

#include "component/drm_display.h"

/***************************/
/* declare display handler */
/***************************/
#include "base/disp_base.h"

#define NUM_MAX_PRIMARY_BUFFER 3

/*! @class DisplayPrimary
 *  @brief Update Primary display
 */
class DisplayPrimary : public DisplayBase {
	sp<DRMDisplay::VSyncCallback> vsync;
	sp<DRMDisplay::FlipCallback>  flip;

public:
	struct buffer_t {
		hwc_disp_buffer           buffer;
		sp<DRMDisplay::IonBuffer> drm_buffer;
	} bufdata[NUM_MAX_PRIMARY_BUFFER];

private:
	struct buffer_t *current;

	DRMDisplay  *dsp;
	struct drm_attributes_t hwc_attr;

	HWCNotice               *notice;
	Mutex                   lock;

#if DRMSUPPORT_BLANK_DESKTOP
	bool                    flag_no_fbt;
#endif

	Condition               cond_flip_flag;
	bool                    vsync_enable;

protected:
	/* virtual function of DRMDisplay class */
	void flip_callback(void);
	void vsync_callback(bool aborted, int64_t time);

	/* virtual function of base class */
	void onUpdateDisplay(hwc_disp_buffer *buf, bool no_plane);
	int  onUpdatePlane(hwc_disp_plane *plane, bool no_fbt);

public:
	/* virtual function of base class */
	int setBlank(bool state);
	int get_attribute_size(int *width, int *height, int *stride);
	int get_attribute_period(int *period);
	int get_attribute_dpi(float *dpi_x, float *dpi_y);
	int get_attribute_dotclock(uint64_t *dc);

	/* check valid or not */
	bool isValid(void);

	void setEnabled(bool state);

	DisplayPrimary(HWCNotice *obj, int display, DRMDisplay *drm_disp);
	~DisplayPrimary();
};

#endif
