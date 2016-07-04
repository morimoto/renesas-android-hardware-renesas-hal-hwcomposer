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

#ifndef __HWC_EXTERNAL_H
#define __HWC_EXTERNAL_H


#if defined(TARGET_BOARD_LAGER) || defined(TARGET_BOARD_SALVATOR_M3) || \
		defined(TARGET_BOARD_SALVATOR_H3)
#define USE_EXTERNAL (1 && defined(USE_EXTERNAL_DISPLAY))
#else
#define USE_EXTERNAL 0
#endif

#if USE_EXTERNAL

#include "base/hwc_base.h"
#include "hwcomposer.h"
#include "component/drm_display.h"

class HWCHotplug;

/*! @class HWCExternal
 *  @brief Initialize external display information
 */
class HWCExternal : public HWCBase {
	friend class HWCHotplug;

	HWCNotice   *notice;
	HWCHotplug  *hotplug;

	DRMDisplay  *dsp;

protected:
	/* virtual function of base class */
	bool onSetupLayersel(hwc_display_contents_1_t* list);

public:
	HWCExternal(HWCNotice *obj, DRMDisplay *drm_disp);
	~HWCExternal();
};

#endif

#endif
