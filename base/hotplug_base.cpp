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

#include "base/hotplug_base.h"
#include "config.h"
#include <stdio.h>
#include <poll.h>
#include <sys/resource.h>

#include <cutils/log.h>


#define FIRSTTIME_DELAY      2 /* sec */

#define POLL_TIMEOUT         10000 /* msec */
#define HOTPLUG_THREAD_NAME  "HWCHotplug"


/*! \brief hotplug thread
 *  \param[in] arg this pointer of C++.
 *  \return NULL
 *  \details
 *  loop is finished by terminate parameter
 */
void* HotplugBase::_threadLoop(void *arg)
{
	class HotplugBase *hotplug_base = (class HotplugBase *)arg;

	/* thread loop */
	hotplug_base->threadLoop();

	return NULL;
}

/*! \brief Wait for hotplug events
 *  \return none
 *  \details
 *  called from _threadLoop().
 */
void HotplugBase::threadLoop(void)
{
	int  need_notice = false;
	bool first_time  = true;

	/* thread loop */
	while (!terminate) {
		bool flag = need_notice;

		if (first_time) {
			/* get current states */
			first_time = false;

			sleep(FIRSTTIME_DELAY);

			if (onInitHotplug() == 0) {
				/* success to get current state. */
				flag = true;
				ALOGD_IF(USE_DBGLEVEL(3),
					"[HWChotplug] threadLoop 1 disp:%d connected:%d", disp, connected);
			}
		} else {
			struct pollfd fds[2];
			int ret;
			int num_fds = 2;

			/* wait pollevent 10 sec */

			if (fd < 0) {
				num_fds = 1;
			} else {
				fds[1].fd     = fd;
				fds[1].events = POLLIN;
			}
			fds[0].fd     = thread.get_waitevent_fd();
			fds[0].events = POLLIN;
			ret = poll(fds, num_fds, POLL_TIMEOUT);

			if (ret > 0) {
				if (fds[0].revents & POLLIN) {
					int rc = thread.receive_event();
					if (rc == HWCThread::TERMINATE)
						ALOGD_IF(USE_DBGLEVEL(3),
							 "[HWChotplug] threadLoop 2 disp:%d receive_event:%d", disp, rc);
				}
				if (fds[1].revents & POLLIN) {
					if (onEventHotplug() == 0) {
						flag = true;
						ALOGD_IF(USE_DBGLEVEL(3),
							 "[HWChotplug] threadLoop 2 disp:%d connected:%d", disp, connected);
					}
				}
			}
		}

		if (terminate)
			break;

		/* notice event */
		if (flag) {
			ALOGD_IF(USE_DBGLEVEL(4),
				"[HWChotplug] threadLoop hotplug  disp:%d", disp);
			if (notice->hotplug(disp, connected)) {
				flag = false;
			}

			/* redraw everything */
			if (connected) {
				ALOGD_IF(USE_DBGLEVEL(4),
					"[HWChotplug] threadLoop invalidate");
				notice->invalidate();
			}
		}
		need_notice = flag;

		ALOGD_IF(USE_DBGLEVEL(4),
			"[HWChotplug] threadLoop 1 disp:%d need_notice:%d", disp, need_notice);
	}
}

/*! \brief initialize
 *  \return none
 *  \details
 *  create thread
 */
void HotplugBase::init(void)
{

        char thread_name[64];
        snprintf(thread_name, sizeof(thread_name), "%s-%d",HOTPLUG_THREAD_NAME,disp);
        thread.start(thread_name, PRIORITY_URGENT_DISPLAY, _threadLoop, (void*)this);

}

/*! \brief request thread terminate
 *  \return none
 *  \details
 *  request thread terminate and return after thread terminate.
 */
void HotplugBase::deinit(void)
{

	terminate = true;
	thread.terminate();

}

/*! \brief HotplugBase initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] _disp  display type
 */
HotplugBase::HotplugBase(HWCNotice *obj, int _disp) :
	notice(obj),
	disp(_disp),
	terminate(false),
	connected(false),
	fd(-1)
{

}

/*! \brief HotplugBase destructor
 */
HotplugBase::~HotplugBase()
{
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}
