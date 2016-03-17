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

#include "base/disp_base.h"
#include "config.h"
#include <sync/sync.h>
#include <ion/ion.h>
#include <stdio.h>

#include <sys/resource.h>
#include <system/graphics.h>

#include <cutils/log.h>
#include "img_gralloc_public.h"


/* timeout of fence recomment to set never.
 * but it is not able to detect location of operation blocked. */
#define MAX_SYNC_WAIT_TIME    12000 /* msec */

#define DISPLAY_THREAD_NAME   "HWCDisp"

#if DEBUG_USE_ATRACE && USE_DBGLEVEL(1)
#define ATRACE_TAG  ATRACE_TAG_ALWAYS
#include <utils/Trace.h>
#endif

/*! \brief Display thread
 *  \param[in] arg this pointer of C++.
 *  \return NULL
 *  \details
 *  loop is finished by terminate parameter
 */
void *DisplayBase::_threadLoop(void *arg)
{
	class DisplayBase *disp_base = (class DisplayBase *)arg;

	/* thread loop */
	disp_base->threadLoop();

	return NULL;
}

/*! \brief Wait for display update
 *  \return none
 *  \details
 *  called from _threadLoop().
 */
void DisplayBase::threadLoop(void)
{
	/* thread loop */
	while (!terminate) {
		{
			Mutex::Autolock _l(lock_disp);
			while (!terminate && !flipqueue.size()) {
				cond_queue.wait(lock_disp);
			}
		}

		if (terminate)
			break;

		if (flipqueue.size()) {
			hwc_disp_buffer *buffer = NULL;
			int             sync_fd = -1;
			bool            no_plane = false;

			/* get data from flipqueue */
			flipqueue.get(&buffer, &sync_fd, &no_plane);

			ALOGD_IF(USE_DBGLEVEL(4),
				"[HWCDisp] threadLoop disp:%d buffer:%p sync_fd:%d", disp, buffer, sync_fd);

			/* wait composition complete */
			if (sync_fd >= 0) {
#if DEBUG_USE_ATRACE && USE_DBGLEVEL(1)
				ATRACE_NAME("syncwait");
#endif
				if (sync_wait(sync_fd, MAX_SYNC_WAIT_TIME) < 0) {
					ALOGE("sync_wait error");
				}
				close(sync_fd);
				sync_fd = -1;
			}

			/* update display buffer. */
			{
#if DEBUG_USE_ATRACE && USE_DBGLEVEL(1)
				ATRACE_NAME("disp");
#endif
				onUpdateDisplay(buffer, no_plane);
			}
		}
	}
}

/*! \brief Add display buffer
 *  \param[in] disp_buffer  pointer to a hwc_disp_buffer structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  only called when each display class is initialized
 */
int DisplayBase::add_buffer(hwc_disp_buffer *disp_buffer)
{
	int res = -1;

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCDisp] add_buffer disp:%d buffer:%p", disp, buffer);

	Mutex::Autolock _l(lock_disp);

	if (num_buffer >= NUM_MAX_DISPLAY_BUFFER) {
		ALOGE("num of buffer invalid");
	} else {
		disp_buffer->buf_index = num_buffer;
		buffer[num_buffer++] = disp_buffer;
		res = 0;
	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCDisp] add_buffer disp:%d num_buffer:%d", disp, num_buffer);

	return res;
}

/*! \brief Process of completed buffer
 *  \param[in] p_buffer  pointer to a hwc_disp_buffer structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 */
int DisplayBase::complete_flip_buffer(hwc_disp_buffer *p_buffer)
{
	int res = -1;
	int i;

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCDisp] complete_flip disp:%d buf_index:%d", disp, p_buffer->buf_index);

	Mutex::Autolock _l(lock_disp);

#if USE_DBGLEVEL(1)
	for (i = 0; i < num_buffer; i++) {
		if (p_buffer == buffer[i]) {
			/* find out valid buffer */
			res = 0;
			break;
		}
	}
	ALOGE_IF(i != p_buffer->buf_index, "[HWCDisp] buf_index invalid.");
#else
	res = 0;
	i = p_buffer->buf_index;
#endif

	if (res == 0) {
		num_dequeuebuffer--;
		if (buffer_usedmask & (1 << i)) {
			/* clear queuemask flag */
			buffer_queuemask &= ~(1 << i);
			/* clear buffer use flag */
			buffer_usedmask &= ~(1 << i);

			cond_dequeue.signal();
		} else {
			ALOGE("buffer is not displayed.");
		}
	} else {
		ALOGE("buffer invalid.");
	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCDisp] dequeue disp:%d buffer_usedmask:0x%x 0x%x", disp, buffer_usedmask, buffer_queuemask);

	return res;
}

/* virtual function. */

/*! \brief Update display
 *  \param[in] buffer    pointer to a hwc_disp_buffer structure
 *  \param[in] no_plane  flag of no plane.
 *  \return none
 *  \details
 *  this is virtual function
 */
void DisplayBase::onUpdateDisplay(hwc_disp_buffer *buffer, bool no_plane)
{
	UNUSED(no_plane);
	/* fake implementation. */
	complete_flip_buffer(buffer);
}

/*! \brief Get display buffer
 *  \return pointer to buffer
 *  \details
 *  called from set()
 */
hwc_disp_buffer *DisplayBase::dequeue(void)
{
	int free_index = -1;

	Mutex::Autolock _l(lock_disp);

	while (!terminate) {
		int tmp_index = 0;
		int i;

		for (i = 0; i < num_buffer; i ++) {
			if ((buffer_usedmask & (1 << tmp_index)) == 0)
				break;

			tmp_index = tmp_index+1;
			if (tmp_index >= num_buffer)
				tmp_index = 0;
		}

		if (i >= num_buffer) {
#if FLIPWAIT_LOG
			/* generate log */
			if (msg_flipwaitlog == 0) {
				if (disp == 0) {
					ALOGI("wait primary flip.");
				} else if (disp == 1) {
					ALOGI("wait external flip.");
				}
			}
			/* set log generate flag. */
			msg_flipwaitlog = 2;
#endif
			/* wait complete display */
			cond_dequeue.wait(lock_disp);
		} else {
#if FLIPWAIT_LOG
			if (msg_flipwaitlog > 0) {
				msg_flipwaitlog--;
			}
#endif
			free_index = tmp_index;
			break;
		}
	}
	if (free_index >= 0) {
		buffer_usedmask |= (1 << free_index);
		num_dequeuebuffer++;
	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCDisp] dequeue disp:%d buffer_usedmask:0x%x 0x%x", disp, buffer_usedmask, buffer_queuemask);


	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCDisp] dequeue disp:%d buf_index:%d", disp, free_index);


	if (free_index >= 0) {
		return buffer[free_index];
	} else {
		return NULL;
	}
}

/*! \brief Release display buffer
 *  \param[in] buffer  pointer to a hwc_disp_buffer structure
 *  \param[in] sync_fd file descriptor
 *  \param[in] no_plane flag of no plane uses
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  called from set()
 */
int DisplayBase::queue(hwc_disp_buffer *buffer, int sync_fd, bool no_plane)
{
	int bufid = buffer->buf_index;
	int res = -1;

	Mutex::Autolock _l(lock_disp);

	if (bufid < 0 || bufid >= num_buffer) {
		ALOGE("invalid buffer_index");
	} else if (!(buffer_usedmask & (1 << bufid))) {
		ALOGE("not dequeue");
	} else if (buffer_queuemask & (1 << bufid)) {
		ALOGE("already queue");
	} else {
		int dup_sync_fd = dup(sync_fd);

		ALOGD_IF(USE_DBGLEVEL(3),
			"[HWCDisp] queue disp:%d buf_index:%d sync_fd:%d", disp, bufid, sync_fd);

		buffer_queuemask |= (1 << bufid);
		flipqueue.put(buffer, dup_sync_fd, no_plane);
		cond_queue.signal();

		res = 0;
	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCDisp] queue disp:%d buffer_usedmask:0x%x 0x%x", disp, buffer_usedmask, buffer_queuemask);

	return res;
}

/*! \brief Cancel dequeue
 *  \param[in] buffer   pointer to a hwc_disp_buffer structure
 *  \param[in] no_plane flag of no plane uses
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  called when dequeue() return NULL
 */
int DisplayBase::dequeue_cancel(hwc_disp_buffer *buffer, bool no_plane)
{
	int bufid = buffer->buf_index;
	int res = -1;

	Mutex::Autolock _l(lock_disp);

	if (bufid < 0 || bufid >= num_buffer) {
		ALOGE("invalid buffer_index");
	} else if (!(buffer_usedmask & (1 << bufid))) {
		ALOGE("not dequeue");
	} else {
		ALOGD_IF(USE_DBGLEVEL(3),
			"[HWCDisp] dequeue_cancel disp:%d buf_index:%d", disp, bufid);

		buffer_usedmask &= ~(1 << bufid);
		cond_dequeue.signal();
		res = 0;
		num_dequeuebuffer--;

		flipqueue.put(NULL, -1, no_plane);
		cond_queue.signal();
	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCDisp] dequeue_cancel disp:%d buffer_usedmask:0x%x 0x%x", disp, buffer_usedmask, buffer_queuemask);

	return res;
}

/*! \brief Set blank flag
 *  \param[in] state  nonzero(screen off) zero(screen on)
 *  \return result of processing
 *  \retval 0   always return this value.
 */
int DisplayBase::setBlank(bool state)
{
	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCDisp] setBlank disp:%d state:%d", disp, state);

	if (blank_state != state) {
		/* register blank_state */
		blank_state = state;
	}
	return 0;
}

/*! \brief Enables or disables Vsync signal
 *  \param[in] state  enables or disables
 *  \retval none
 */
void DisplayBase::setEnabled(bool state)
{

	ALOGD_IF(USE_DBGLEVEL(3),
		 "[HWCVsync] setEnabled disp:%d state:%d", disp, state);

}

/*! \brief Update display
 *  \param[in] plane   pointer to a hwc_disp_plane structure
 *  \param[in] no_fbt  flag of presents desktop
 *  \return file descriptor of fence.
 *  \details
 *  this is virtual function
 */
int DisplayBase::onUpdatePlane(hwc_disp_plane *plane, bool no_fbt)
{
	/* fake implementation. */
	UNUSED(plane);
	UNUSED(no_fbt);
	return -1;
}

/*! \brief Update display
 *  \param[in] _handle  pointer to a buffer_handle_t structure
 *  \param[in] fence    fence of plane
 *  \param[in] crop     pointer to a hwc_rect_t structure
 *  \param[in] win      pointer to a hwc_rect_t structure
 *  \param[in] no_fbt   flag to turn off fbt plane.
 *  \return result of processing
 *  \retval grater_equal_to_0  fence file descriptor
 *  \retval -1                 error
 *  \details
 *  this is virtual function
 */
int DisplayBase::show_plane(buffer_handle_t _handle, int fence, hwc_rect_t *crop, hwc_rect_t *win, bool no_fbt)
{
	hwc_disp_plane      data;

	/* default */
	memset(&data, 0, sizeof(data));
	data.ion_fd = -1;
	data.sync_fd = -1;

	if (_handle) {
		IMG_native_handle_t *handle = (IMG_native_handle_t*)_handle;

		if (handle->fd[0] >= 0) {
			/* do not dup file descriptor, because fd is used in this function. */
			data.sync_fd = fence;
			data.ion_fd  = handle->fd[0];
			data.iFormat = handle->iFormat;
			data.iWidth  = handle->iWidth;
			data.iHeight = handle->iHeight;

			data.crop_x  = crop->left;
			data.crop_y  = crop->top;
			data.crop_w  = crop->right - crop->left;
			data.crop_h  = crop->bottom - crop->top;

			data.win_x  = win->left;
			data.win_y  = win->top;
			data.win_w  = win->right - win->left;
			data.win_h  = win->bottom - win->top;

			if ((handle->usage & GRALLOC_USAGE_SW_WRITE_MASK) != 0) {
				int fd = ion_open();
				ion_sync_fd(fd, handle->fd[0]);
				ion_close(fd);
			}
		}
	}

	return onUpdatePlane(&data, no_fbt);
}

/*! \brief initialize
 *  \return none
 *  \details
 *  create thread
 */
void DisplayBase::init(void)
{

	char thread_name[64];
	snprintf(thread_name, sizeof(thread_name), "%s-%d",DISPLAY_THREAD_NAME, disp);
	thread.start(thread_name, PRIORITY_URGENT_DISPLAY, _threadLoop, (void*)this);

}

/*! \brief request thread terminate
 *  \return none
 *  \details
 *  request thread terminate and return after thread terminate.
 */
void DisplayBase::deinit(void)
{
	{
		Mutex::Autolock _l(lock_disp);
		terminate = true;
		cond_queue.signal();
		cond_dequeue.signal();
	}

	thread.terminate();
}

/*! \brief DisplayBase initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] _disp  display type
 */
DisplayBase::DisplayBase(HWCNotice *obj, int _disp) :
	notice(obj),
	terminate(false),
#if FLIPWAIT_LOG
	msg_flipwaitlog(0),
#endif
	buffer_usedmask(0),
	num_buffer(0),
	num_dequeuebuffer(0),
	buffer_queuemask(0),
	blank_state(false),
	disp(_disp)
{

}

/* virtual function */

/*! \brief DisplayBase destructor
 */
DisplayBase::~DisplayBase()
{

}
