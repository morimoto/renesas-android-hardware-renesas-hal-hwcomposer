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


/*************************/
/* includes              */
/*************************/
#include "displays/hwc_primary.h"
#include "displays/hwc_primary_private.h"
#include "config.h"
#include <cutils/log.h>
#include "component/hwcglobal.h"

#include <img_gralloc_public.h>
#include <linux/fb.h>
#include <ion/ion.h>
#include <sync/sync.h>
#include <linux/ion.h>
#include <sys/mman.h>
#include <utility>

/*************************/
/* defines               */
/*************************/
#define ALIGN_ROUND_UP_4K(X)    (((X)+4095) & ~4095)

#define PRIM_DISP_ID   0
//#define PRIM_CRT_INDEX   DRM_MODE_ENCODER_DAC
#define PRIM_CRT_INDEX   DRM_MODE_ENCODER_TMDS
//#define PRIM_CON_INDEX   DRM_MODE_CONNECTOR_VGA
#define PRIM_CON_INDEX DRM_MODE_CONNECTOR_HDMIA

#if DEBUG_USE_ATRACE
#define ATRACE_TAG  ATRACE_TAG_ALWAYS
#include <utils/Trace.h>

#endif

#define WAIT_TIMEOUT 5000000000LL /* nsec */

#define YUV_PLANEID   0

#define STOP_PLANE_DELAY    0    /* if 1 specified, turn off plane delay. if 0 specified, immediately. */

/*************************/
/* implement DISPLAY     */
/*************************/

/*! \brief callback of pageflip
 *  \return none
 *  \details
 *  make condition signale.
 */
void DisplayPrimary::flip_callback(void)
{
	Mutex::Autolock _l(lock);
	cond_flip_flag.signal();
}

/* virtual function of base class */

/*! \brief Update display
 *  \param[in] buf  pointer to a hwc_disp_buffer structure
 *  \param[in] no_plane  flag of plane use
 *  \return none
 *  \details
 *  set the buffer to primary display and wait for the buffer to be finished
 */
void DisplayPrimary::onUpdateDisplay(hwc_disp_buffer *buf, bool no_plane)
{
#if DEBUG_USE_ATRACE
	char name[40];
#endif
#if STOP_PLANE_DELAY
	bool need_plane_op = no_plane;
	sp<DRMDisplay::IonBuffer> tmp=NULL;
#endif
	struct buffer_t *next = (struct buffer_t *)buf;
	bool   ignore_display = false;

	UNUSED(no_plane);

	Mutex::Autolock _l(lock_blank);
	if (next == NULL) {
		ignore_display = true;
	} else {
#if DRMSUPPORT_BLANK_DESKTOP
		/* if DRM support BLANK_DESKTOP extension,
		 * turn off FB from pageFlip is available. */
		if (blank_state) {
			/* do not update current buffer */
			complete_flip_buffer(&next->buffer);
			ignore_display = true;
		}
#endif
	}


	if (!ignore_display) {
#if DEBUG_USE_ATRACE
		sprintf(name, "Primary-Disp%d", next->buffer.buf_index);
		ATRACE_INT(name, 1);
#endif

		Mutex::Autolock _l(lock);
#if DRMSUPPORT_BLANK_DESKTOP
		if (flag_no_fbt) {
			/* do not page flip when no_fbt */
		} else {
#endif
			if (dsp->display_pageflip(PRIM_DISP_ID, next->drm_buffer, flip)) {
#if STOP_PLANE_DELAY
				if (need_plane_op) {
					dsp->display_plane(PRIM_DISP_ID, YUV_PLANEID, tmp);
					need_plane_op = false;
				}
#endif
				if (cond_flip_flag.waitRelative(lock, WAIT_TIMEOUT) != NO_ERROR) {
					ALOGE("wait flip timeout");
				}
			}
#if DRMSUPPORT_BLANK_DESKTOP
		}
#endif

		if (current) {
#if DEBUG_USE_ATRACE
			sprintf(name, "Primary-Disp%d", current->buffer.buf_index);
			ATRACE_INT(name, 0);
#endif
			/* notice base class the buffer be complete */
			complete_flip_buffer(&current->buffer);
		}
		current = next;
	}
#if STOP_PLANE_DELAY
	if (need_plane_op) {
		dsp->display_plane(PRIM_DISP_ID, YUV_PLANEID, tmp);
		need_plane_op = false;
	}
#endif
}

/*! \brief Update display
 *  \param[in] plane  pointer to a hwc_disp_plane structure
 *  \param[in] no_fbt flag of frame buffer target
 *  \return file descriptor of fence.
 *  \details
 *  update plane of drm.
 */
int DisplayPrimary::onUpdatePlane(hwc_disp_plane *plane, bool no_fbt)
{
	int      f = -1;
	sp<DRMDisplay::IonBuffer> buffer = NULL;

	UNUSED(no_fbt);

	if (plane->sync_fd >= 0) {
	    struct sync_fence_info_data* finfo = sync_fence_info(plane->sync_fd);
	    if (finfo) {
		    if (finfo->status != 1) {
				ALOGE("plane update witout waiting fence.");
			}
	        sync_fence_info_free(finfo);
	    }
	}

	if (plane->ion_fd >= 0) {
		buffer = new DRMDisplay::IonBuffer(dsp, plane->ion_fd, plane->iFormat, plane->iWidth, plane->iHeight);
	}

#if STOP_PLANE_DELAY
	if (buffer.get() != NULL) {
#endif
		dsp->display_plane(PRIM_DISP_ID, YUV_PLANEID, buffer,
			plane->crop_x, plane->crop_y, plane->crop_w, plane->crop_h,
			plane->win_x,  plane->win_y,  plane->win_w,  plane->win_h, &f);
#if STOP_PLANE_DELAY
	}
#endif

#if DRMSUPPORT_BLANK_DESKTOP
	Mutex::Autolock _l(lock);
	flag_no_fbt = no_fbt;
	if (no_fbt) {
		dsp->display_pageflip_blank(PRIM_DISP_ID);
	}
#endif

	return f;
}

/* virtual function of base class */


/*! \brief Set blank state
 *  \param[in] state  nonzero(screen off) zero(screen on)
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayPrimary::setBlank(bool state)
{
	Mutex::Autolock _l(lock_blank);

	dsp->set_blankstate(PRIM_DISP_ID, state);

	blank_state = state;

	return 0;
}

/* virtual function of base class */

/*! \brief Get size from attribute
 *  \param[out] width width
 *  \param[out] height height
 *  \param[out] stride stride
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayPrimary::get_attribute_size(int *width, int *height, int *stride)
{
	*width  = hwc_attr.display_width;
	*height = hwc_attr.display_height;
	*stride = hwc_attr.display_stride;
	return 0;
}

/* virtual function of base class */

/*! \brief Get vsync period from attribute
 *  \param[out] period period
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayPrimary::get_attribute_period(int *period)
{
	/* set vsync period in nano sec. */
	*period = hwc_attr.display_vsync_period;
	return 0;
}

/* virtual function of base class */

/*! \brief Get dots per inch from attribute
 *  \param[out] dpi_x X dots per inch
 *  \param[out] dpi_y Y dots per inch
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayPrimary::get_attribute_dpi(float *dpi_x, float *dpi_y)
{
	*dpi_x = hwc_attr.display_dpi_x;
	*dpi_y = hwc_attr.display_dpi_y;
	return 0;
}

/*! \brief Get dots clock of current display
 *  \param[out] dc    dot clock.
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayPrimary::get_attribute_dotclock(uint64_t *dc)
{
	*dc = hwc_attr.display_dotclock;
	*dc *= 1000;
	return 0;
}

/*! \brief Check whether DRM is available or not
 *  \return result of processing
 *  \retval false       error
 *  \retval true       normal
 */
bool DisplayPrimary::isValid(void)
{
	int i;
	for (i = 0; i < NUM_MAX_PRIMARY_BUFFER; i++) {
		if (!bufdata[i].drm_buffer.get())
			return false;
		if (bufdata[i].drm_buffer->drm_fbid < 0)
			return false;
	}
	return true;
}

/*! \brief notice vsync.
 *  \param[in] aborted  flag of fake notice to abort wait vsync.
 *  \param[in] time     time of vsync in nano second.
 *  \return none
 */
void DisplayPrimary::vsync_callback(bool aborted, int64_t time)
{
	if (!aborted && vsync_enable) {
		notice->vsync(disp, time);
	}
}

/*! \brief Enables or disables Vsync signal
 *  \param[in] state  enables or disables
 *  \retval none
 */
void DisplayPrimary::setEnabled(bool state)
{

	ALOGD_IF(USE_DBGLEVEL(3),
		 "[HWCVsync] setEnabled disp:%d state:%d", disp, state);

	vsync_enable = state;
	if (state) {
		dsp->add_vsync_listener(vsync);
	} else {
		dsp->remove_vsync_listener(vsync);
	}
}


/*! \brief DisplayPrimary initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] display  display type
 *  \param[in] drm_disp  pointer to a DRMDisplay structure
 *  \details
 *  if getmode() returns error, setmode() called with default display size.
 *  default display size is FullHD
 */
DisplayPrimary::DisplayPrimary(HWCNotice *obj, int display, DRMDisplay *drm_disp):
	DisplayBase(obj, display),
	current(NULL),
	dsp(drm_disp),
	notice(obj),
#if DRMSUPPORT_BLANK_DESKTOP
	flag_no_fbt(false),
#endif
	vsync_enable(false)
{
	int width = 1920, height = 1080;
	bool interlace = false;

	int ret_getmode = false;
	int get_width, get_height;
	int i;

	int ion_fd;
	int map_size;
	uint32_t drm_handle;
	int map_fd;

//	ret_getmode = dsp->getmode(PRIM_DISP_ID, PRIM_CRT_INDEX,
//		PRIM_CON_INDEX, &get_width, &get_height);
//
//	if (ret_getmode) {
//		width = get_width;
//		height = get_height;
//	}
	ALOGI("primary display size:%dx%d", width, height);

	if (!ret_getmode) {
		if (!dsp->setmode(PRIM_DISP_ID, PRIM_CRT_INDEX, PRIM_CON_INDEX, width, height, interlace)) {
			ALOGE("can not set mode for primary display CRT:%d CON:%d", PRIM_CRT_INDEX, PRIM_CON_INDEX);
		}
	}

	/* to avoid error in SurfaceFlinger. configure dummy param */
	hwc_attr.display_vsync_period = 1000000000 / 60;
	hwc_attr.display_width = width;
	hwc_attr.display_height = height;
	hwc_attr.display_dpi_x = 0;
	hwc_attr.display_dpi_y = 0;
	hwc_attr.display_stride = 0;

	if (!dsp->getattributes(PRIM_DISP_ID, &hwc_attr)) {
		ALOGE("can not get display attributes\n");
	}

	ion_fd = ion_open();
	map_size = hwc_attr.display_stride*height;
	map_size = ALIGN_ROUND_UP_4K(map_size);

	for (i = 0; i < NUM_MAX_PRIMARY_BUFFER; i++) {
		//const int alloc_flag = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
		const int alloc_flag = 0;
		//const int heap_mask  = 1 << (ION_HEAP_TYPE_CUSTOM);
		const int heap_mask  = ION_HEAP_TYPE_DMA_MASK;
		drm_handle = 0; /* does not use libkms so drm_handle is always zero. */
		map_fd = -1;
		if (ion_alloc_fd(ion_fd, map_size, 4096, heap_mask, alloc_flag, &map_fd) < 0) {
			/* error */
			ALOGE("ion_alloc_fd error\n");
		}

		bufdata[i].buffer.buf_fd    = map_fd;
		bufdata[i].drm_buffer       = new DRMDisplay::IonBuffer(dsp, map_fd, HAL_PIXEL_FORMAT_BGRA_8888, width, height);
		bufdata[i].buffer.phys_addr = 0;

		/* add buffer to base class. */
		add_buffer(&bufdata[i].buffer);
	}

	ion_close(ion_fd);

	/* create vsync listener */
	class VSYNCReceiver:public virtual DRMDisplay::VSyncCallback {
		DisplayPrimary *obj;
	public:
		void vsync_callback(bool aborted, int64_t time) {
			obj->vsync_callback(aborted, time);
		}
		VSYNCReceiver(DisplayPrimary *_obj):obj(_obj) { }
		virtual ~VSYNCReceiver() {};
	};

	vsync = new VSYNCReceiver(this);

	/* create flip listener */
	class FlipReceiver:public virtual DRMDisplay::FlipCallback {
		DisplayPrimary *obj;
	public:
		void flip_callback(void) {
			obj->flip_callback();
		}
		FlipReceiver(DisplayPrimary *_obj):obj(_obj) { }
		virtual ~FlipReceiver() {};
	};

	flip = new FlipReceiver(this);
}

/*! \brief DisplayPrimary destructor
 */
DisplayPrimary::~DisplayPrimary()
{
	int i;

	if (vsync_enable) {
		dsp->remove_vsync_listener(vsync);
	}
	vsync = NULL;
	flip = NULL;

	for (i = 0; i < NUM_MAX_PRIMARY_BUFFER; i++) {
		if (bufdata[i].buffer.buf_fd >= 0) {
			close(bufdata[i].buffer.buf_fd);
		}
		bufdata[i].drm_buffer = NULL;
	}

	/* turn off plane */
	sp<DRMDisplay::IonBuffer> tmp = NULL;
	dsp->display_plane(PRIM_DISP_ID, YUV_PLANEID, tmp);
}

/*****************************/
/* implement PRIMARY display */
/*****************************/

/*! \brief Setup layer select
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 */
bool HWCPrimary::onSetupLayersel(hwc_display_contents_1_t* list)
{
	int num_layer = g.num_overlay[HWC_DISPLAY_PRIMARY];
	int num_yuv   = 1;

	UNUSED(list);

	ALOGD_IF(USE_DBGLEVEL(3),
		"Primary-Disp overlay:%d", num_layer);

	layersel->init_numlayer(num_layer, num_yuv);

	return true;
}

/*! \brief HWCPrimary initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] drm_disp  pointer to a DRMDisplay structure
 */
HWCPrimary::HWCPrimary(HWCNotice *obj, DRMDisplay *drm_disp) :
	notice(obj),
	dsp(drm_disp)
{
	display_type = HWC_DISPLAY_PRIMARY;

	registerDisplay(new DisplayPrimary(notice, display_type, dsp));

	/* primary display always connected. */
	set_connect_state(true);

}

/*! \brief HWCPrimary destructor
 */
HWCPrimary::~HWCPrimary()
{
	/* nothing to do */
}
