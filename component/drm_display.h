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

#ifndef __HWCOMPOSER_DRM_H
#define __HWCOMPOSER_DRM_H

#include "config.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include "drm_fourcc.h"

#include "base/hwc_thread.h"
#include "SyncTimeline.h"

#include <hardware/hwcomposer_defs.h>

#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/RefBase.h>
#include <utils/List.h>

using namespace android;

#ifndef UNUSED
#define UNUSED(X) ((void)&X)
#endif

/* set 1, if drm driver support blank desktop extension. */
// #define DRMSUPPORT_BLANK_DESKTOP    1

struct drm_attributes_t{
	int   display_vsync_period;
	int   display_width;
	int   display_height;
	float display_dpi_x;
	float display_dpi_y;
	int   display_stride;
	int   display_dotclock;
};

/*! @class DRMUtil
 *  @brief Interface to DRM utility
 */
class DRMUtil {
public:
	int drmModeSetPlaneFence(int fd, uint32_t plane_id, uint32_t crtc_id,
		    uint32_t fb_id, uint32_t flags,
		    uint32_t crtc_x, uint32_t crtc_y,
		    uint32_t crtc_w, uint32_t crtc_h,
		    uint32_t src_x, uint32_t src_y,
		    uint32_t src_w, uint32_t src_h);
	int drmRcarDU_DesktopPlane(int fd, int crt, int on);
};

/*! @class DRMDisplay
 *  @brief Interface to DRM driver
 */
class DRMDisplay:public DRMUtil {
private:
	void get_mode_part1(int disp_id, drmModeRes *resources);
public:
/*********************/
/* buffer interface  */
/* use smart pointer */
/*********************/
	/*! 
	 *  @brief inner class of DRMDisplay. ion buffer handling.
	 */
	class IonBuffer: public virtual RefBase {
		DRMDisplay *dsp;
	public:
		int drm_fbid;
		IonBuffer(DRMDisplay *_dsp, int ion_fd, int format, int width, int height);
		IonBuffer(DRMDisplay *_dsp, buffer_handle_t _hnd);
		virtual ~IonBuffer();
	};
/********************/
/* vsync interface  */
/********************/
	/*! 
	 *  @brief inner class of DRMDisplay. vsync callback.
	 */
	class VSyncCallback: public virtual RefBase {
	public:
		int64_t entry_time;
		virtual void vsync_callback(bool aborted, int64_t time);
		VSyncCallback():entry_time(0) {};
		virtual ~VSyncCallback();
	};
	int add_vsync_listener(sp<VSyncCallback>& listner);
	int remove_vsync_listener(sp<VSyncCallback>& listner);
/**************************/
/* pageflip interface     */
/**************************/
	/*! 
	 *  @brief inner class of DRMDisplay. flip callback.
	 */
	class FlipCallback: public virtual RefBase {
	public:
		DRMDisplay    *dsp;
		int           dispid;
		sp<IonBuffer> buffer;
		virtual void flip_callback(void);
		virtual ~FlipCallback();
	};
	int display_pageflip(int dispid, sp<IonBuffer>& buffer, sp<FlipCallback>& listner);
	int display_pageflip_blank(int dispid);
/**************************/
/* plane interface        */
/**************************/
	bool display_plane(int disp_id, int plane_id, sp<IonBuffer>& buffer, int cx=0, int cy=0, int cw=0, int ch=0, int dx=0, int dy=0, int dw=0, int dh=0, int *f=NULL);


private:
	List< sp<VSyncCallback> > vsync_listners;

	HWCThread       thread;
	HWCThread       thread_vsync;

	int             drm_fd;
	int             drm_fd_vsync;
	drmEventContext evctx;
	drmEventContext evctx_vsync;
	Mutex           lock_plane;
	Mutex           lock_flip;
	Mutex           lock_vsync;
	int             request_next;
	SyncTimeline    sync_timeline;

	struct {
		List< sp<FlipCallback> >  flip_listners;

		uint32_t        crtc_id;
		uint32_t        connector_id;
		uint32_t        plane_id[1];
		sp<IonBuffer>   plane_buffer[1];

		drmModeModeInfo mode;
		struct drm_attributes_t attributes;

#if DRMSUPPORT_BLANK_DESKTOP
		bool            desktop_visible;
#endif
		bool            blank_state;
		bool            first_draw;
		int             high_crtc;
	} display[NUM_DISPLAYS];

	static void* _threadLoop(void *arg);
	static void* _threadLoop_vsync(void *arg);
	void         threadLoop(void);
	void         threadLoop_vsync(void);
	static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data);
	static void vblank_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data);

protected:
	/* return vrefresh */
	float get_drm_mode_vrefresh(drmModeConnector *connector, drmModeModeInfo *mode);
	/* return pitch */
	uint32_t get_drm_fb_pitch(int width, int fmt);
	/* set attributes */
	void setattributes(drmModeConnector *connector, drmModeModeInfo *mode, int disp_id, float vrefresh);

	void request_next_vsync(void);
	bool getResolutionFromBootargs(int disp_id, int& width, int& height, int& refresh);

public:
	bool setmode(int disp_id, uint32_t enc_id, uint32_t con_id,
		int& width, int& height, bool interlace = false, int HZ = 0);
	bool getattributes(int disp_id, struct drm_attributes_t *hwc_attr);

	bool getmode(int disp_id, uint32_t enc_type, uint32_t con_id, int *width, int *height);

	/* support blank */
	bool set_blankstate(int disp_id, bool status);

	/*! cheak whether DRM driver is available */
	inline bool isValid(void)
	{
		return drm_fd >= 0 && drm_fd_vsync >= 0;
	}

	int getfencefd();

	/* initialize */
	void init(void);
	void deinit(void);

	DRMDisplay();
	~DRMDisplay();
};

#endif
