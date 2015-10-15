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

#include "component/fencemerge.h"
#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <cutils/log.h>



/*! @class FenceMerge
 *  @brief Interface to merge fences.
 */

int FenceMerge::sequence = 0;

/*! \brief merge fence
 *  \param[in] f1  sync fence
 *  \return none
 */
void FenceMerge::merge(int f1)
{
	/* sync_merge is always successed, except following cases */
	/* - not enough kernel memory */
	/* - fd is not sync object    */
	if (f1 < 0) {
		ALOGE_IF(USE_DBGLEVEL(4),
			"invalid fence. ignore merge.");
	} else if (fence < 0) {
		/* duplicate fence */
		fence = dup(f1);
		ALOGE_IF(fence < 0, "failed to dup fence.");
	} else {
		int merge_fence = sync_merge(name, fence, f1);
		if (merge_fence >= 0) {
			close(fence);
			fence = merge_fence;
		} else {
			/* keep previous fence */
			ALOGE_IF(USE_DBGLEVEL(4),
				"%s", strerror(errno));
		}
		ALOGE_IF(fence < 0, "failed to merge fence.");
	}
}

/*! \brief return merged fence
 *  \return result of processing
 *  \retval -1                    no valid fence or fence is not necessary.
 *  \retval grater_than_equal_0   valid fence
 */
int  FenceMerge::get(void)
{
	return fence;
}

/*! \brief clear merged fence
 *  \return nothing
 */
void FenceMerge::clear(void)
{
	if (fence >=0) {
		close(fence);
		fence = -1;
	}
	return;
}


/*! \brief FenceMerge initialize
 */
FenceMerge::FenceMerge()
{
	snprintf(name, sizeof(name), "HWC_acfence%d", sequence++);
	fence = -1;
}

/*! \brief FenceMerge destructor
 */
FenceMerge::~FenceMerge()
{
	if (fence>=0)
		close(fence);
}
