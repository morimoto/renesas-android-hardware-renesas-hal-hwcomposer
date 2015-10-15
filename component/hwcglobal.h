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
#ifndef HWC_GLOBAL_H_
#define HWC_GLOBAL_H_

#include <hardware/hwcomposer.h>
#include <stdint.h>

/*! @class HWCGlobal
 *  @brief Global parameter for control hwcomposer.
 *         this object will access by variable g.
 */
class HWCGlobal {
public:
	bool     st_disable_hwc;
	bool     st_connect[HWC_NUM_PHYSICAL_DISPLAY_TYPES];
	bool     st_blank[HWC_NUM_PHYSICAL_DISPLAY_TYPES];
	uint64_t st_dotclock[HWC_NUM_PHYSICAL_DISPLAY_TYPES];

	/* assignment result */
	int      num_overlay[HWC_NUM_DISPLAY_TYPES];

	void init(void);
	HWCGlobal();
};

extern HWCGlobal g;


#endif
