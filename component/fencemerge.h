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

#ifndef _HWC_FENCEMERGE
#define _HWC_FENCEMERGE

#include <sync/sync.h>

/*! @class FenceMerge
 *  @brief Interface to merge fences.
 */
class FenceMerge {
/* private */
	static int sequence;
	char       name[32];

	int        fence;
public:
	void  merge(int f1);
	int   get(void);
	void  clear(void);

	FenceMerge();
	~FenceMerge();
};

#endif
