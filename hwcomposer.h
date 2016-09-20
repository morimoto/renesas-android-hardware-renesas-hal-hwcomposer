/*
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
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

#ifndef __HWCOMPOSER_H
#define __HWCOMPOSER_H

/***********************************
 define function switch
***********************************/
#include "config.h"

/***********************************
 define macro
***********************************/

/***********************************
 define structure
***********************************/

/*****************************************************
 structure for hwcomposer
*****************************************************/
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include "base/hwc_notice.h"
#include "base/hwc_base.h"
#include "component/drm_display.h"

#include <utils/Mutex.h>
using namespace android;

typedef struct hwc_context_t {
	hwc_composer_device_1_t device;

	/* mutex object */
	Mutex       mutex;

	/* entry to callback surfaceflinger. */
	hwc_procs_t           *procs;

	/* notice object */
	HWCNotice            notice;

	/* drm object */
	DRMDisplay *drm_disp;

	/* display */
	HWCBase              *base[3 + NUM_OF_VIRTUALDISPLAY];

	/* previous used display */
	int                  prev_used_display;

	/* poll hwcomposer mode */
	bool                 force_updatemode;
	int                  poll_fd;
} hwc_context;



#endif /* __HWCOMPOSER_BASE_H */

