/*
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
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

#include "component/hwcglobal.h"
#include <utils/misc.h>

HWCGlobal g;

/*! \brief HWCGlobal initialize
 */
void HWCGlobal::init(void)
{
	int i;
	st_disable_hwc = true;

	for (i = 0; i < HWC_NUM_PHYSICAL_DISPLAY_TYPES;i++) {
		st_connect[i] = false;
		st_blank[i] = false;
		st_dotclock[i] = 0;
	}

	for (i = 0; i < HWC_NUM_DISPLAY_TYPES; i++) {
		num_overlay[i] = 0; /* available layer for overlay is zero. */
	}
}

/*! \brief HWCGlobal initialize
 */
HWCGlobal::HWCGlobal()
{
	init();
}

