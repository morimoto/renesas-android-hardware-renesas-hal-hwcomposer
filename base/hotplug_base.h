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

#ifndef _HWC_INTERFACE_HOTPLUG_BASE
#define _HWC_INTERFACE_HOTPLUG_BASE

#include <hardware/hwcomposer.h>

#include "base/hwc_notice.h"
#include "base/hwc_thread.h"

/* base class */

/*! @class HotplugBase
 *  @brief Hotplug processes for external display
 */
class HotplugBase {
/* private */
	HWCNotice     *notice;
	HWCThread     thread;

	int            disp;
	volatile bool  terminate;

	static void*   _threadLoop(void *arg);
	void           threadLoop(void);

/* protected */
protected:
	int            connected;
	int            fd;

	virtual int onInitHotplug(void)  = 0;
	virtual int onEventHotplug(void) = 0;

/* public */
public:

	/*! Get display type for debug */
	inline int get_display(void)
	{
		return disp;
	}

	void init(void);
	void deinit(void);

	HotplugBase(HWCNotice *obj, int disp);
	virtual ~HotplugBase();
};


#endif
