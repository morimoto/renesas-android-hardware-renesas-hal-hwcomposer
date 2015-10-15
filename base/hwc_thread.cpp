/*
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hwc_thread.h"
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cutils/log.h>

#include <errno.h>
#include <string.h>

/*! \brief Receive event
 *  \return result of processing
 *  \retval event code
 */
int HWCThread::receive_event(void)
{
	unsigned char event = 0;

	int ret = read(pipe_fd[0], &event, 1);
	if (ret < 0) {
		int eno = errno;
		ALOGD("errstr:%s", strerror(eno));
	}

	return event - '0';
}

/*! \brief Send event
 *  \param[in] code  event code
 *  \return none
 */

void HWCThread::send_event(int code)
{
	unsigned char event = 0;

	event = code + '0';
	write(pipe_fd[1], &event, 1);

}

/*! \brief Thread terminate
 *  \return none
 */
void HWCThread::terminate(void)
{

	/* request terminate and wait.*/
	if (mThread.get()) {
		send_event(TERMINATE);

		if (mThread->requestExitAndWait()) {
			ALOGE("requestExitAndWait failed");
		}

		mThread.clear();
	}
	return;
}

/*! \brief Thread start
 *  \param[in] name  thread name
 *  \param[in] priority thread priority
 *  \param[in] routine pointer to thread function
 *  \param[in] arg pointer to thread argument
 *  \return none
 */
void HWCThread::start(const char *name, int priority, void *(*routine)(void *), void *arg)
{
	if (mThread.get()) {
		ALOGE("already start thread.");
	} else {
		strlcpy(thread_name, name, sizeof(thread_name));
		thread_entry = routine;
		thread_arg   = arg;

		mThread = new ImplThread(this);
		mThread->run(name, priority);
	}

}

/*! \brief HWCThread initialize
 */
HWCThread::HWCThread()
: thread_entry(NULL),thread_arg(NULL)
{
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	if (pipe2(pipe_fd, O_CLOEXEC) == -1) {
		ALOGE("pipe2 error.");
	}
}

/*! \brief HWCThread destructor
 */
HWCThread::~HWCThread()
{

	if (isstart()) {
		ALOGE("HWCThread invoke terminate. %s", thread_name);
		terminate();
	}
	if (pipe_fd[0] >= 0)
		close(pipe_fd[0]);

	if (pipe_fd[1] >= 0)
		close(pipe_fd[1]);
};
