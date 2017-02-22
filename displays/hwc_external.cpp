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
#include "displays/hwc_external.h"
#include "displays/hwc_external_private.h"
#include "config.h"
#include <cutils/log.h>
#include "component/hwcglobal.h"

#include <ion/ion.h>
#include <linux/ion.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <fcntl.h>

/*************************/
/* defines               */
/*************************/
#if USE_EXTERNAL

#define ALIGN_ROUND_UP_4K(X)    (((X)+4095) & ~4095)
#define ALIGN_ROUND_UP_128(X)    (((X)+127) & ~127)

#if defined(TARGET_BOARD_LAGER) || \
	defined(TARGET_BOARD_SALVATOR)

#define EXTERNAL_WIDTH              1920
#define EXTERNAL_HEIGHT             1080

#define EXT_CRT_INDEX               DRM_MODE_ENCODER_TMDS
#define EXT_CON_INDEX               DRM_MODE_CONNECTOR_HDMIA

#else
#error unknown TARGET_BOARD_xxx
#endif

#define USE_UEVENT           1

#if USE_UEVENT
#include <hardware_legacy/uevent.h>
#include <string.h>
#endif

#if DEBUG_USE_ATRACE
#define ATRACE_TAG  ATRACE_TAG_ALWAYS
#include <utils/Trace.h>
#endif

#define WAIT_TIMEOUT 5000000000LL /* nsec */

/*************************/
/* implement DISPLAY     */
/*************************/
/* virtual function of base class */

/*! \brief callback of pageflip
 *  \return none
 *  \details
 *  make condition signale.
 */
void DisplayExternal::flip_callback(void)
{
	Mutex::Autolock _l(lock);
	cond_flip_flag.signal();
}

/*! \brief Update display
 *  \param[in] buf  pointer to a hwc_disp_buffer structure
 *  \param[in] no_plane  flag of plane use
 *  \return none
 *  \details
 *  set the buffer to external display and wait for the buffer to be finished
 */
void DisplayExternal::onUpdateDisplay(hwc_disp_buffer *buf, bool no_plane)
{
#if DEBUG_USE_ATRACE
	char name[40];
#endif
	struct buffer_t *next = (struct buffer_t *)buf;

	UNUSED(no_plane);

	if (next == NULL) {
		/* ignore display */
	} else if (next->drm_buffer.get() == NULL) {
		/* ignore display */
		complete_flip_buffer(&next->buffer);
	} else {
		Mutex::Autolock _l(lock_blank);

#if DRMSUPPORT_BLANK_DESKTOP
		/* if DRM support BLANK_DESKTOP extension,
		 * turn off FB from pageFlip is available. */
		if (blank_state) {
			/* do not update current buffer */
			complete_flip_buffer(&next->buffer);
			return;
		}
#endif

#if DEBUG_USE_ATRACE
		sprintf(name, "External-Disp%d", next->buffer.buf_index);
		ATRACE_INT(name, 1);
#endif
		{
			Mutex::Autolock _l(lock);
			if (dsp->display_pageflip(disp_id, next->drm_buffer, flip)) {
				if (cond_flip_flag.waitRelative(lock, WAIT_TIMEOUT) != NO_ERROR) {
					ALOGE("wait flip timeout");
				}
			}
		}

		if (current) {
#if DEBUG_USE_ATRACE
			sprintf(name, "External-Disp%d", current->buffer.buf_index);
			ATRACE_INT(name, 0);
#endif
			/* notice base class the buffer be complete */
			complete_flip_buffer(&current->buffer);
		}
		current = next;
	}
}

/* virtual function of base class */

/*! \brief Set blank state
 *  \param[in] state  nonzero(screen off) zero(screen on)
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayExternal::setBlank(bool state)
{
	Mutex::Autolock _l(lock_blank);

	dsp->set_blankstate(disp_id, state);

	blank_state = state;

	return  0;
}

/* virtual function of base class */

/*! \brief Get size from attribute
 *  \param[out] width width
 *  \param[out] height height
 *  \param[out] stride stride
 *  \return result of processing
 *  \retval 0       always return this value
 */
int DisplayExternal::get_attribute_size(int *width, int *height, int *stride)
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
int DisplayExternal::get_attribute_period(int *period) {
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
int DisplayExternal::get_attribute_dpi(float *dpi_x, float *dpi_y)
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
int DisplayExternal::get_attribute_dotclock(uint64_t *dc)
{
	*dc = hwc_attr.display_dotclock;
	*dc *= 1000;
	return 0;
}

/*! \brief Set display size
 *  \param[in] width  width
 *  \param[in] height  height
 *  \return result of processing
 *  \retval false       error
 *  \retval true       normal
 *  \details
 *  if getmode() returns error, setmode() called with default display size.
 *  default display size is FullHD
 */
bool DisplayExternal::set_displaysize(int width, int height)
{
	sp<DRMDisplay::IonBuffer> drm_data[NUM_MAX_EXTERNAL_BUFFER];
	int i;
	int get_width, get_height;

	int encoder_id = hwdisplays[disp_id].encoder_id;
	int connector_id = hwdisplays[disp_id].connector_id;

	if (!dsp->setmode(disp_id, encoder_id, connector_id, width, height)) {
		ALOGE("can not set mode for external display ENC:%d CON:%d", encoder_id, connector_id);
	}

	ALOGD("external display size:%dx%d", width, height);

	clear_buffers();
	freeDisplayBuffers();
	allocateDisplayBuffers(width, height);

	/* to avoid error in SurfaceFlinger. configure dummy param */
	hwc_attr.display_vsync_period = 1000000000 / 60;
	hwc_attr.display_width = width;
	hwc_attr.display_height = height;
	hwc_attr.display_dpi_x = 150;
	hwc_attr.display_dpi_y = 150;
	hwc_attr.display_stride = width * 4;

	/* create new display_buffer. */
	for (i = 0; i < NUM_MAX_EXTERNAL_BUFFER; i++) {
		int map_fd = bufdata[i].buffer.buf_fd;

		drm_data[i] = new DRMDisplay::IonBuffer(dsp, map_fd, HAL_PIXEL_FORMAT_BGRA_8888, width, height);
		if (drm_data[i]->drm_fbid < 0) {
			ALOGE("can not change displaysize");
			while (i >= 0) {
				drm_data[i] = NULL;
				i--;
			}
			return false;
		}
	}

	if (!dsp->getattributes(disp_id, &hwc_attr)) {
		ALOGE("can not get display attributes\n");
		/* to avoid problem overwrite attributes. */
		hwc_attr.display_width  = width;
		hwc_attr.display_height = height;
	}

	/* swap registered information. */
	for (i = 0; i < NUM_MAX_EXTERNAL_BUFFER; i++) {
		bufdata[i].drm_buffer = NULL;
		bufdata[i].drm_buffer = drm_data[i];
	}

	return true;
}

/*! \brief Check whether DRM is available or not
 *  \return result of processing
 *  \retval false       error
 *  \retval true       normal
 */
bool DisplayExternal::isValid(void)
{
	int i;
	for (i = 0; i < NUM_MAX_EXTERNAL_BUFFER; i++) {
		if (!bufdata[i].drm_buffer.get())
			return false;
		if (bufdata[i].drm_buffer->drm_fbid < 0)
			return false;
	}
	return true;
}

/*! \brief DisplayExternal initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] display  display type
 *  \param[in] drm_disp  pointer to a DRMDisplay structure
 */
DisplayExternal::DisplayExternal(HWCNotice *obj, int display, DRMDisplay *drm_disp, int id):
	DisplayBase(obj, display),
	current(NULL),
	dsp(drm_disp),
	disp_id(id)
{
	/* fake buffer register */
	int i;

	/* to avoid error in SurfaceFlinger. configure dummy param */
	hwc_attr.display_vsync_period = 1000000000 / 60;
	hwc_attr.display_width = EXTERNAL_WIDTH;
	hwc_attr.display_height = EXTERNAL_HEIGHT;
	hwc_attr.display_dpi_x = 0;
	hwc_attr.display_dpi_y = 0;
	hwc_attr.display_stride = 0;

	/* actual hwc_attr is configured in set_displaysize function. */
	/* set default display size */
#if 0
	if (!set_displaysize(EXTERNAL_WIDTH, EXTERNAL_HEIGHT)) {
		ALOGE("can not set display size");
	}
#endif

	/* create flip listener */
	class FlipReceiver:public virtual DRMDisplay::FlipCallback {
		DisplayExternal *obj;
	public:
		void flip_callback(void) {
			obj->flip_callback();
		}
		FlipReceiver(DisplayExternal *_obj):obj(_obj) { }
		virtual ~FlipReceiver() {};
	};

	flip = new FlipReceiver(this);

	for (int i = 0; i < NUM_MAX_EXTERNAL_BUFFER; i++) {
		bufdata[i].buffer.buf_fd = -1;
	}
}

/*! \brief DisplayExternal destructor
 */
DisplayExternal::~DisplayExternal()
{
	flip = NULL;

	freeDisplayBuffers();
}

int DisplayExternal::allocateDisplayBuffers(int width, int height)
{
	uint32_t drm_handle;
	int map_fd;

	int ion_fd = ion_open();
	int map_size = ALIGN_ROUND_UP_128(width * 4) * height;
	map_size = ALIGN_ROUND_UP_4K(map_size);

	for (int i = 0; i < NUM_MAX_EXTERNAL_BUFFER; i++) {
		const int alloc_flag = 0;
		const int heap_mask  = ION_HEAP_TYPE_DMA_MASK;
		drm_handle = 0; /* does not use libkms so drm_handle is always zero. */
		map_fd = -1;
		if (ion_alloc_fd(ion_fd, map_size, 4096, heap_mask, alloc_flag, &map_fd) < 0) {
			/* error */
			ALOGE("ion_alloc_fd error\n");
		}

		bufdata[i].buffer.buf_fd = map_fd;
		bufdata[i].drm_buffer    = NULL;
		bufdata[i].buffer.phys_addr = 0;

		/* add buffer to base class. */
		add_buffer(&bufdata[i].buffer);
	}

	/* close ion_device. */
	ion_close(ion_fd);

	return 0;
}

void DisplayExternal::freeDisplayBuffers()
{
	for (int i = 0; i < NUM_MAX_EXTERNAL_BUFFER; i++) {
		if (bufdata[i].buffer.buf_fd >= 0) {
			close(bufdata[i].buffer.buf_fd);
		}
		bufdata[i].drm_buffer = NULL;
	}
}

/*************************/
/* implement HOTPLUG     */
/*************************/

/*! \brief Initialize hotplug
 *  \return result of processing
 *  \retval 0       always return this value
 */
int HWCHotplug::onInitHotplug(void)
{
	int fd = open(hwdisplays[disp_id].status, O_RDONLY);

	connected = false;
	if (fd < 0) {
		ALOGE("error open hdmi state");
	} else {
		char value;

		read(fd, &value, 1);
		close(fd);

		if (value == 'c') {
			connected = true;
		}
	}

	if (connected) {
		DisplayExternal *disp = (DisplayExternal*)ext_base->disp;

		if (!disp) {
			ALOGE("can not get display handle");
		} else {
			/* update display class config if necessary. */

			disp->set_displaysize(EXTERNAL_WIDTH, EXTERNAL_HEIGHT);

			ext_base->set_connect_state(true);
		}
	} else {
		ext_base->set_connect_state(false);
	}

	return 0;
}

/*! \brief Event hotplug
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 */
int HWCHotplug::onEventHotplug(void)
{
	int result = -1;

#if USE_UEVENT
	char uevent_desc[4096];
	const char *dev = "change@/devices/platform/soc/feb00000.display/drm/card0";
	const char *s;
	int        len;
	int state = 0;

	bool curr_connected = false;

	len = uevent_next_event(uevent_desc, sizeof(uevent_desc) - 2);
	s   = &uevent_desc[0];

	if (strcmp(s, dev) == 0) {

		int fd = open(hwdisplays[disp_id].status, O_RDONLY);

		if (fd < 0) {
			ALOGE("error open hdmi state");
		} else {
			char value;

			read(fd, &value, 1);
			close(fd);

			if (value == 'c') {
				curr_connected = true;
			}
		}

		if (curr_connected != connected) {
			connected = curr_connected;
			result = 0;
			if (connected) {
				DisplayExternal *disp = (DisplayExternal*)ext_base->disp;

				if (!disp) {
					ALOGE("can not get display handle");
				} else {
					/* update display class config if necessary. */

					disp->set_displaysize(EXTERNAL_WIDTH, EXTERNAL_HEIGHT);

					ext_base->set_connect_state(true);
				}
			} else {
				ext_base->set_connect_state(false);
			}
		}
	}
#endif

	return result;
}

/*! \brief Check whether DRM is available or not
 *  \return result of processing
 *  \retval false       error
 *  \retval true       normal
 */
bool HWCHotplug::isValid(void)
{
#if USE_UEVENT
	return (fd >= 0);
#else
	return true;
#endif
}

/*! \brief HWCHotplug initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] disp  display type
 *  \param[in] base  pointer to a HWCExternal structure
 */
HWCHotplug::HWCHotplug(HWCNotice *obj, int disp, HWCExternal *base, int disp_id): HotplugBase(obj, disp_id), ext_base(base),
		disp_id(disp_id)
{
#if USE_UEVENT
	uevent_init();
	fd = uevent_get_fd();
#endif
}

HWCHotplug::~HWCHotplug()
{
	/* nothing to do */
}

/*****************************/
/* implement EXTERNAL display */
/*****************************/

/* virtual function of base class */

/*! \brief Setup layer select
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 */
bool HWCExternal::onSetupLayersel(hwc_display_contents_1_t* list)
{
	int num_layer = g.num_overlay[HWC_DISPLAY_EXTERNAL];
	int num_yuv   = 1;
	int num_scaler   = 0;

	UNUSED(list);

	ALOGD_IF(USE_DBGLEVEL(3),
		"External-Disp overlay:%d", num_layer);

	layersel->init_numlayer(num_layer, num_yuv, num_scaler);

	return true;
}

/*! \brief HWCExternal initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 *  \param[in] drm_disp  pointer to a DRMDisplay structure
 */
HWCExternal::HWCExternal(HWCNotice *obj, DRMDisplay *drm_disp, int id) :
	notice(obj),
	dsp(drm_disp),
	disp_id(id)
{

	display_type = id;

	registerDisplay(new DisplayExternal(notice, display_type, dsp, disp_id));

	/* external display always dis-connected on init. */
	set_connect_state(false);

	hotplug = new HWCHotplug(notice, HWC_DISPLAY_EXTERNAL, this, disp_id);

	if (hotplug) {
		hotplug->init();
	}

}

/*! \brief HWCExternal destructor
 */
HWCExternal::~HWCExternal()
{
	if (hotplug) {
		hotplug->deinit();
		delete hotplug;
	}
}

#endif
