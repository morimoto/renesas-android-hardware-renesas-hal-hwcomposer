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

#ifndef __HWC_VIRTUAL_H
#define __HWC_VIRTUAL_H

#include "base/hwc_base.h"
#include "hwcomposer.h"

/***********************************/
/* declare virtual display handler */
/***********************************/

/*! @class HWCVirtual
 *  @brief Initialize virtual display information
 */
class HWCVirtual : public HWCBase {
private:
	void onTargetPrepare_part1(hwc_display_contents_1_t* list, struct target_t& info, bool& use_opengl, bool& use_composer, const int log_flag, int& next_log_flag);
private:
	bool       outbuf_invalid;

	/* virtual function */
	bool onSetupLayersel(hwc_display_contents_1_t* list);
	void onTargetPrepare(hwc_display_contents_1_t* list, struct target_t& info);
	void onTargetExecute(hwc_display_contents_1_t* list, int composer_fence, int& drm_fence, struct target_t& info, int&  retire_fence);

public:
	HWCVirtual(HWCNotice *obj);
	~HWCVirtual();
};

#endif
