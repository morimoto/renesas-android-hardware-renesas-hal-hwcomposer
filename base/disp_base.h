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

#ifndef _HWC_INTERFACE_DISPLAY_BASE
#define _HWC_INTERFACE_DISPLAY_BASE

#include <hardware/hwcomposer.h>

typedef struct hwc_disp_buffer_t {
	int           buf_index;  /* reserved */
	unsigned long phys_addr;
	int           buf_fd;
} hwc_disp_buffer;

typedef struct hwc_disp_plane_t {
	int           sync_fd;
	int           ion_fd;
	int           iFormat;
	int           iWidth;
	int           iHeight;
	int           crop_x;
	int           crop_y;
	int           crop_w;
	int           crop_h;
	int           win_x;
	int           win_y;
	int           win_w;
	int           win_h;
} hwc_disp_plane;

#ifndef UNUSED
#define UNUSED(X) ((void)&X)
#endif

#define FLIPWAIT_LOG             0

#define NUM_MAX_DISPLAY_BUFFER   4

#include "base/hwc_notice.h"
#include "base/hwc_thread.h"

#include <utils/Mutex.h>
#include <utils/Condition.h>

using namespace android;

/*! @class DisplayBase
 *  @brief Control display buffer
 */
class DisplayBase {
	HWCNotice     *notice;
	HWCThread     thread;

	Mutex   lock_disp;
	Condition    cond_queue;
	Condition    cond_dequeue;

	volatile bool     terminate;
#if FLIPWAIT_LOG
	int               msg_flipwaitlog;
#endif

	/* buffer information */
	hwc_disp_buffer *buffer[NUM_MAX_DISPLAY_BUFFER];
	int              buffer_usedmask;
	int              num_buffer;
	int              num_dequeuebuffer;

	/*! @brief FIFO class for handling display update information */
	class fifo {
		Mutex fifo_lock;
		int count;
		struct {
			hwc_disp_buffer *bufid;
			int             sync_fd;
			bool            no_plane;
		} info[NUM_MAX_DISPLAY_BUFFER];
	public:
		fifo() {
			count = 0;
		}
		inline int size(void)
		{
			return count;
		}
		int put(hwc_disp_buffer *bufid, int sync_fd, bool no_plane) {
			int ret;
			Mutex::Autolock _l(fifo_lock);
			if (count >= NUM_MAX_DISPLAY_BUFFER) {
				/* queue overflow */
				ret = -1;
			} else {
				info[count].bufid   = bufid;
				info[count].sync_fd = sync_fd;
				info[count].no_plane = no_plane;
				count++;
				ret = 0;
			}
			return ret;
		}
		int get(hwc_disp_buffer **bufid, int *sync_fd, bool *no_plane) {
			int ret;
			Mutex::Autolock _l(fifo_lock);
			if (count == 0) {
				/* queue underflow */
				ret = -1;
			} else {
				int i;
				*bufid   = info[0].bufid;
				*sync_fd = info[0].sync_fd;
				*no_plane = info[0].no_plane;
				for (i = 1; i < count; i++)
					info[i-1] = info[i];
				count--;
				ret = 0;
			}
			return ret;
		}
	} flipqueue;

	int          buffer_queuemask;
	static void* _threadLoop(void *arg);
	void         threadLoop(void);

/* protected */
protected:
	Mutex             lock_blank;
	bool              blank_state;
	int               disp;
	Mutex             page_flip;

	int add_buffer(hwc_disp_buffer *buffer);
	int complete_flip_buffer(hwc_disp_buffer *buffer);

	virtual void onUpdateDisplay(hwc_disp_buffer *buffer, bool no_plane);
	virtual int  onUpdatePlane(hwc_disp_plane *plane, bool no_fbt);

public:
/* public virtual */
	hwc_disp_buffer *dequeue(void);
	int dequeue_cancel(hwc_disp_buffer *buffer, bool no_plane);
	int queue(hwc_disp_buffer *buffer, int sync_fd, bool no_plane);
	int show_plane(buffer_handle_t _handle, int fence, hwc_rect_t *crop, hwc_rect_t *win, bool no_fbt);

	virtual int setBlank(bool state);

	/*! Get display type for debug */
	inline int get_display(void) {
		return disp;
	}

	virtual int get_attribute_size(int *width, int *height, int *stride) = 0;
	virtual int get_attribute_period(int *period) = 0;
	virtual int get_attribute_dpi(float *dpi_x, float *dpi_y) = 0;
	virtual int get_attribute_dotclock(uint64_t *clock) = 0;

	virtual void setEnabled(bool state);

	/* initialize */
	void init(void);
	void deinit(void);

	DisplayBase(HWCNotice *obj, int disp);
	virtual ~DisplayBase();
};

#endif
