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

#include "base/hwc_notice.h"
#include <cutils/log.h>
#include <inttypes.h>
#include "config.h"

#define DEBUG_HWCNOTICE_ATRACE   0

#if DEBUG_USE_ATRACE && DEBUG_HWCNOTICE_ATRACE
#define ATRACE_TAG  ATRACE_TAG_ALWAYS
#include <utils/Trace.h>
#endif

/*! \brief Register proc handle
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 */

/* to disable multiple register declare member as static. */
bool HWCNotice::proc_register(const hwc_procs_t **entry)
{
	if (procs) {
		ALOGE("proc handle already registered");
		return false;
	} else {
		procs = entry;
		return true;
	}
}

/*! \brief Invalidate notice functions
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 */
bool HWCNotice::invalidate(void)
{
	if (procs && (*procs) && (*procs)->invalidate) {

		Mutex::Autolock _l(notice_mutex);

		ALOGD_IF(USE_DBGLEVEL(3), "notice invalidate");
		(*procs)->invalidate(*procs);
		return true;
	} else {
		ALOGE("invalidate not registered");
		return false;
	}
}

/*! \brief Operation of vsync
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 */
bool HWCNotice::vsync(int disp, int64_t timestamp)
{
	bool result = false;

	if (procs && (*procs)->vsync) {

		Mutex::Autolock _l(notice_mutex);

		ALOGD_IF(USE_DBGLEVEL(4), "notice vsync disp:%d at %" PRId64 " nsec", disp, timestamp);
		(*procs)->vsync(*procs, disp, timestamp);
		result = true;
	} else {
		ALOGE("vsync not registered");
	}
	return result;
}

/*! \brief Operation of hotplug
 *  \return result of processing
 *  \retval true       normal
 *  \retval false      error
 */
bool HWCNotice::hotplug(int disp, int connected)
{
	if (procs && (*procs)->hotplug) {
		Mutex::Autolock _l(notice_mutex);
		ALOGD_IF(USE_DBGLEVEL(3), "notice hotplug disp:%d connect:%d", disp, connected);
		(*procs)->hotplug(*procs, disp, connected);
		return true;
	} else {
		ALOGE("hotplug not registered");
		return false;
	}
}

/*! \brief HWCNotice initialize
 */
HWCNotice::HWCNotice():
	procs(NULL)        /* callback entry.          */
{

}
