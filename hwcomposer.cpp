/*
 *
 * Copyright (C) 2012-2014 Renesas Electronics Corporation
 *
 * This file is based on the hardware/libhardware/modules/hwcomposer/hwcomposer.cpp
 *
 * Copyright (C) 2010 The Android Open Source Project
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

#include <hardware/hardware.h>

#include <cutils/log.h>

#include "hwcomposer.h"
#include "component/hwcglobal.h"

#include <cutils/properties.h>
#include <utils/misc.h>
#include <utils/String8.h>
#include <img_gralloc_public.h>

#include "displays/hwc_primary.h"
#include "displays/hwc_external.h"
#if USE_HWC_VERSION1_3
#include "displays/hwc_virtual.h"
#endif

/*****************************************************************************/
/* DEBUG_FRAMERATE
 * report frame rate every 10 second.
 *  1   effective.
 *  0   unavailable.
 */
#define DEBUG_FRAMERATE     1 /* report frame rate if 1 specified. */

/*****************************************************************************/
/* DEBUG_PROCESSTIME
 * report processing time between prepare to set.
 *  1   effective.
 *  0   unavailable.
 */
#define DEBUG_PROCESSTIME    0 /* report time if 1 specified. */

/*****************************************************************************/
/* CONFIRM_COMPLETE
 * wait complete drawing of composer, this is need to get correct time information for debug.
 *  1   wait hwc_set till composition running (MAX_NATIVEBUFFER_USED must be defined as 1.)
 *  0   no waiting.
 */
#define CONFIRM_COMPLETE     0  /* wait complete blending                   */

/*****************************************************************************/
/* MONITOR_THREAD
 * handle close buffer used for composer and report information to detect deadlock.
 *  2   use monitor of in/out only
 *  0   not use monitor thread.
 */
#define MONITOR_THREAD      0  /* monitor information. */

/*****************************************************************************/
/* NUM_ASSIGN_OVERLAY
 * layer for overlay. valid range from 2 to 4.
 *  3   use three layer for overlay.
 *  2   use two layer for overlay.
 */
#if defined(TARGET_BOARD_SALVATOR_H3)
#define NUM_ASSIGN_OVERLAY    4
#else
#define NUM_ASSIGN_OVERLAY    2
#endif

/*****************************************************************************/
/*  debug functions                                                          */
/*****************************************************************************/
#if DEBUG_PROCESSTIME
#if CONFIRM_COMPLETE == 0
#error not available
#endif
static struct timeval process_time[2];

#define PLACE_PREPARE   0
#define PLACE_SET       1

static void process_time_handle(int place);
#endif

#if DEBUG_FRAMERATE
static struct timeval fps_start_time;
static int            fps_frame_count;

static void fps_report(void);
#endif

#if DEBUG_USE_ATRACE
#define ATRACE_TAG  ATRACE_TAG_ALWAYS
#include <utils/Trace.h>
#endif

#if USE_HWC_VERSION1_3
#if USE_HWC_VERSION1_2 == 0
#error for API version 1.3 necessary support API version1.2
#endif
#if NUM_OF_VIRTUALDISPLAY == 0
#ifdef FORCE_HWC_COPY_FOR_VIRTUAL_DISPLAYS
#error if FORCE_HWC_COPY_FOR_VIRTUAL_DISPLAYS is defined NUM_OF_VIRTUALDISPLAY should over 1.
#else
#error If comment out this line, virtual display works correct. except for no layer available case.
#endif
#endif
#endif

#define max2(A,B) ((A) > (B) ? (A):(B));

#if MONITOR_THREAD
#define MONITOR_THREAD_NAME "HWCMonitor"
#include <utils/List.h>
#include "base/hwc_thread.h"

using namespace android;

static HWCThread                            monitor_thread;
static void* monitor_thread_function(void *);

static Mutex                                monitor_inout_lock;
static int                                  monitor_inout_count[16][2];
class MonitorInout {
public:
	int        use_index;
	MonitorInout(int x) {
		use_index = x;
		Mutex::Autolock _l(monitor_inout_lock);

		monitor_inout_count[use_index][0]++;
	};
	~MonitorInout() {
		Mutex::Autolock _l(monitor_inout_lock);

		monitor_inout_count[use_index][1]++;
	};
};

static void monitor_inout_log(void);


#define MONITOR_INOUT(X) class MonitorInout _mon_##__LINE__(X)
#else
#define MONITOR_INOUT(X)
#endif


/*****************************************************************************/
/*  prototype                                                                */
/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
	struct hw_device_t** device);

/*****************************************************************************/
/*  variables                                                                */
/*****************************************************************************/
static struct hw_module_methods_t hwc_module_methods = {
	open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
	common: {
		tag: HARDWARE_MODULE_TAG,
		module_api_version: HWC_MODULE_API_VERSION_0_1,
		hal_api_version: HARDWARE_HAL_API_VERSION,
		id: HWC_HARDWARE_MODULE_ID,
		name: "R-Car hwcomposer module",
		author: "The Android Open Source Project",
		methods: &hwc_module_methods,
		dso: NULL,
		reserved: {0},
	}
};

/******************************************************************************
 Processing for prepare
******************************************************************************/

/*! \brief update num of overlay allowable.
 *  \param[in] numDisplays  number of display
 *  \param[in] displays  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval false   num of allowable overlay not changed.
 *  \retval true    num of allowable overlay changed.
 */
static bool update_global(size_t numDisplays, hwc_display_contents_1_t** displays)
{
	int  max_ovl;
	bool changed;

	UNUSED(numDisplays);
	UNUSED(displays);

	ALOGD_IF(USE_DBGLEVEL(3),
		"connected:pri=%d ext=%d", g.st_connect[0], g.st_connect[1]);
	ALOGD_IF(USE_DBGLEVEL(3),
		"dotclock:pri=%lld ext=%lld", g.st_dotclock[0], g.st_dotclock[1]);

	/* num of available overlay for composer. */
	max_ovl = 0;

	if (!g.st_disable_hwc) {
		max_ovl = NUM_ASSIGN_OVERLAY;
	}

	ALOGD_IF(USE_DBGLEVEL(3),
		"st_disable_hwc:%d max_ovl:%d", g.st_disable_hwc, max_ovl);

	/* update global state */
	changed = (g.num_overlay[0] != max_ovl);

	g.num_overlay[0] = max_ovl; /* num of overlays for primary display  */
	g.num_overlay[1] = max_ovl; /* num of overlays for external display */
	g.num_overlay[2] = max_ovl; /* num of overlays for virtual display  */

	return changed;
}

/*! \brief Prepares for updating the framebuffer
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] numDisplays  number of display
 *  \param[in] displays  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  hwc_prepare calls each display prepare()
 */
static int hwc_prepare(hwc_composer_device_1_t *dev,
	size_t numDisplays, hwc_display_contents_1_t** displays)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	size_t i;
	bool   vsp_disable = false;
	bool   update_mode = false;
	MONITOR_INOUT(0);

#if DEBUG_USE_ATRACE
	ATRACE_CALL();
#endif

	if (!ctx) {
		ALOGE("hwc_prepare invalid context");
		return -1;
	}

#if DEBUG_PROCESSTIME
	process_time_handle(PLACE_PREPARE);
#endif

	/* acquire lock */
	Mutex::Autolock _l(ctx->mutex);

	/* update hwc state */
	update_mode = ctx->force_updatemode;
	ctx->force_updatemode = false;

	if (ctx->poll_fd >= 0) {
		char buf[16];
		int  len;

		lseek(ctx->poll_fd, 0, SEEK_SET);
		len = read(ctx->poll_fd, &buf[0], sizeof(buf)-1);
		if (len >0) {
			buf[len] = 0;
			if (atoi(buf) == 0) {
				/* disable HWC */
				vsp_disable = true;
			}
		}
	}

	if (g.st_disable_hwc != vsp_disable) {
		update_mode = true;
	}

	g.st_disable_hwc = vsp_disable;

	if (update_global(numDisplays, displays)) {
		update_mode = true;
	}

	for (i = 0; i < numDisplays; i++) {
		if (i < NELEM(ctx->base)) {
			if (ctx->base[i]) {
				if (update_mode) {
					ctx->base[i]->update_layerselmode();
				}
				ctx->base[i]->prepare(displays[i]);
			}
		} else {
			ALOGE_IF(USE_DBGLEVEL(3),
				"display %d not supported.", i);
		}
	}

	return 0;
}

/******************************************************************************
 Processing for set
******************************************************************************/

/*! \brief Updates the framebuffer
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] numDisplays  number of display
 *  \param[in] displays  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  hwc_set calls each display set()
 */
static int hwc_set(hwc_composer_device_1_t *dev,
		size_t numDisplays, hwc_display_contents_1_t** displays)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	int rc;
	int result = 0;
	size_t i;
	MONITOR_INOUT(1);

#if DEBUG_USE_ATRACE
	ATRACE_CALL();
#endif

	if (!ctx) {
		ALOGE("hwc_set invalid context");
		return -1;
	}

	{
		/* acquire lock */
		Mutex::Autolock _l(ctx->mutex);

		ctx->prev_used_display = 0;
		for (i = 0; i < numDisplays; i++) {
			if (i < NELEM(ctx->base)) {
				if (displays[i]) {
					ctx->prev_used_display |= (1<<i);
				}
				if (ctx->base[i]) {
					rc = ctx->base[i]->set(displays[i]);
					if (rc) {
						/* mark error of display */
						result |= 0x01 << i;
					}
				}
			} else {
				ALOGE_IF(USE_DBGLEVEL(3),
					"display %d not supported.", i);
				if (displays[i]) {
					/* handle default fence operation. */
					HWCBase::set_release_fence(displays[i],-1,-1,false,NULL,-1);
					HWCBase::clear_acquire_fence(displays[i]);
				}
			}
		}

		if (result) {
			ALOGD_IF(USE_DBGLEVEL(3),
				 "result: 0x%x.", result);
		}

		/* wait fence signal */
#if CONFIRM_COMPLETE
#if MAX_NATIVEBUFFER_USED != 1
#error MAX_NATIVEBUFFER_USED not configured as 1. can not use CONFIRM_COMPLETE
#else
		for (i = 0; i < numDisplays; i++) {
			if (i < NELEM(ctx->base)) {
				if (ctx->base[i])
					ctx->base[i]->wait_composer_fence();
			} else {
				ALOGE_IF(USE_DBGLEVEL(3),
					"display %d not supported.", i);
			}
		}
#endif
#endif

	}

#if DEBUG_PROCESSTIME
	process_time_handle(PLACE_SET);
#endif
#if DEBUG_FRAMERATE
	char value[PROPERTY_VALUE_MAX];
	property_get("debug.hwc.showfps", value, "0");
	if (atoi(value))
		fps_report();
#endif

	return 0;
}

/******************************************************************************
 Processing for syncFence
******************************************************************************/

/*! \brief Enables or disables H/W composer events
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] disp  display type
 *  \param[in] event  event type
 *  \param[in] enabled  enables or disables
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 *  \details
 *  hwc_eventControl calls eventControl()
 */
static int hwc_eventControl(struct hwc_composer_device_1* dev, int disp, int event, int enabled)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	HWCBase             *base = NULL;
	int    rc = -EINVAL;
	MONITOR_INOUT(2);

#if DEBUG_USE_ATRACE
	ATRACE_CALL();
#endif

	/* acquire lock */
	Mutex::Autolock _l(ctx->mutex);

	ALOGD_IF(USE_DBGLEVEL(3),
		" eventControl disp:%d event:%d enabled:%d", disp, event, enabled);

	/* select display */
	if (disp >=0 && disp < NELEM(ctx->base))
		base = ctx->base[disp];
	else
		ALOGE("disp %d not supported.", disp);

	if (base) {
		rc = base->eventControl(event, enabled);
	}

	return rc;
}

/*! \brief Set blank state to display
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] disp  display type
 *  \param[in] blank  nonzero(screen off) zero(screen on)
 *  \return result of processing
 *  \retval 0   always return this value
 *  \details
 *  not supported
 */
static int hwc_blank(struct hwc_composer_device_1* dev, int disp, int blank)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	HWCBase             *base= NULL;
	int    rc = -EINVAL;
	MONITOR_INOUT(3);

#if DEBUG_USE_ATRACE
	ATRACE_CALL();
#endif

	ALOGD_IF(USE_DBGLEVEL(3),
		"blank request disp:%d blank:%d\n", disp, blank);

	/* acquire lock */
	Mutex::Autolock _l(ctx->mutex);

	/* select display */
	if (disp >=0 && disp < NELEM(ctx->base))
		base = ctx->base[disp];
	else
		ALOGE("disp %d not supported.", disp);

	if (base) {
		rc = base->blank(blank);
	}

	return rc;
}

/*! \brief Used to retrieve information about the H/W composer
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] what  kind of retrieve information
 *  \param[out] value pointer to store information
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 *  \details
 *  Supported 'what's are HWC_BACKGROUND_LAYER_SUPPORTED and HWC_DISPLAY_TYPES_SUPPORTED
 */
static int hwc_query(struct hwc_composer_device_1* dev, int what, int* value)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	int    rc = -EINVAL;
	MONITOR_INOUT(4);

	switch (what) {
	case HWC_BACKGROUND_LAYER_SUPPORTED:
		/* BACKGROUND_LAYER is unsupported,
		 * because it isn't used on the SurfaceFlinger side. */
		*value = 0;
		rc = 0;
		break;
	case HWC_DISPLAY_TYPES_SUPPORTED:
		/* only support primary layer. */

		*value = 0;
		if (ctx->base[0])
			*value |= HWC_DISPLAY_PRIMARY_BIT;
		if (ctx->base[1])
			*value |= HWC_DISPLAY_EXTERNAL_BIT;
#if USE_HWC_VERSION1_3 && NUM_OF_VIRTUALDISPLAY
		if (ctx->base[2])
			*value |= HWC_DISPLAY_VIRTUAL_BIT;
#endif

		rc = 0;
		break;
	default:
		/* unsupported query */
		ALOGE_IF(USE_DBGLEVEL(3),
			"query(%d) not supported.", what);
	}
	ALOGD_IF(USE_DBGLEVEL(3),
		" query what:%d value:%d", what, *value);

	return rc;
}

/*! \brief Dump message
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[out] buff  buffer for message
 *  \param[in] buff_len buffer length
 *  \return none
 */
static void hwc_dump(struct hwc_composer_device_1* dev, char *buff, int buff_len)
{
	hwc_context_t *ctx = (hwc_context_t *) dev;
	size_t i;
	MONITOR_INOUT(5);

	String8 msg;

	for (i = 0; i < NELEM(ctx->base); i++) {
		if (ctx->prev_used_display & (1<<i)) {
			if (ctx->base[i])
				ctx->base[i]->dump(msg);
		}
	}
	msg.appendFormat("  [variable]\n");
	msg.appendFormat("    disble:%d\n", g.st_disable_hwc);
	if (g.st_connect[0]) {
		msg.appendFormat("    primary  connect:%d blank:%d dotclock:%llu num_overlay:%d\n",
			g.st_connect[0], g.st_blank[0], (long long unsigned int)g.st_dotclock[0], g.num_overlay[0]);
	}
	if (g.st_connect[1]) {
		msg.appendFormat("    external connect:%d blank:%d dotclock:%llu num_overlay:%d\n",
			g.st_connect[1], g.st_blank[1], (long long unsigned int)g.st_dotclock[1], g.num_overlay[1]);
	}
#if USE_HWC_VERSION1_3
	msg.appendFormat("    virtual  num_overlay:%d\n", g.num_overlay[2]);
#endif

	if (!msg.isEmpty()) {
		snprintf(buff, buff_len-1, "%s", msg.string());
	} else {
		*buff = 0;
	}
}

/*! \brief Register hwc_procs_t pointer
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] procs pointer to a hwc_procs_t structure
 *  \return none
 */
static void hwc_register_procs(struct hwc_composer_device_1* dev,
	hwc_procs_t const* procs)
{
	hwc_context_t *hwc_dev = (hwc_context_t *) dev;

	ALOGD_IF(USE_DBGLEVEL(3),
		"register Procs");

	hwc_dev->procs = (typeof(hwc_dev->procs)) procs;
}


/*! \brief hwc_getDisplayConfigs returns handles for the configurations available
 *  on the connected display
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] disp  display type
 *  \param[out] configs pointer to configuration handles
 *  \param[in,out] numConfigs pointer to configuration number
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 */
static int hwc_getDisplayConfigs(struct hwc_composer_device_1* dev, int disp,
	uint32_t* configs, size_t* numConfigs)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	HWCBase             *base= NULL;
	int    rc = -EINVAL;
	MONITOR_INOUT(6);

	ALOGD_IF(USE_DBGLEVEL(3),
		"getDisplayConfigs disp:%d config:%p, size:%d", disp, configs, *numConfigs);

	/* select display */
	if (disp >=0 && disp < NELEM(ctx->base))
		base = ctx->base[disp];
	else
		ALOGE("disp %d not supported.", disp);

	if (base) {
		rc = base->DisplayConfigs(configs, numConfigs);
	}

	return rc;
}

/*! \brief hwc_getDisplayAttributes returns attributes for a specific config of a
 *  connected display
 *  \param[in] dev  pointer to a hwc_composer_device_1_t structure
 *  \param[in] disp  display type
 *  \param[in] config  configuration handles
 *  \param[in] attributes pointer to attributes array
 *  \param[out] values pointer to values array
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 */
static int hwc_getDisplayAttributes(struct hwc_composer_device_1* dev, int disp,
	uint32_t config, const uint32_t* attributes, int32_t* values)
{
	struct hwc_context_t *ctx = (struct hwc_context_t*)dev;
	HWCBase             *base= NULL;
	int    rc         = -EINVAL;
	MONITOR_INOUT(7);

	ALOGD_IF(USE_DBGLEVEL(3),
		"getDisplayAttributes disp:%d config:%d attributes:%p values:%p", disp, config, attributes, values);

	/* select display */
	if (disp >=0 && disp < NELEM(ctx->base))
		base = ctx->base[disp];
	else
		ALOGE("disp %d not supported.", disp);

	if (base) {
		rc = base->DisplayAttributes(config, attributes, values);
	}

	return rc;
}

#if DEBUG_PROCESSTIME

/*! \brief Get time information
 *  \param[in] place  set PLACE_SET or PLACE_PREPARE
 *  \return none
 */
static void process_time_handle(int place)
{
	gettimeofday(&process_time[place], NULL);

	if (place == PLACE_SET) {
		int  usec, sec;
		sec  = process_time[1].tv_sec  - process_time[0].tv_sec;
		usec = process_time[1].tv_usec - process_time[0].tv_usec;
		usec+= sec*1000000;
		ALOGD("%6d usec", usec);
	}
}
#endif

#if DEBUG_FRAMERATE

/*! \brief Report FPS
 *  \return none
 */
static void fps_report(void)
{
	if (fps_frame_count == 0) {
		gettimeofday(&fps_start_time, NULL);
		fps_frame_count++;
	} else {
		struct timeval current;
		int    sec;
		int    usec;
		int    time;
		gettimeofday(&current, NULL);
		sec  = current.tv_sec  - fps_start_time.tv_sec;
		usec = current.tv_usec - fps_start_time.tv_usec;

		time  = usec;
		time += sec * 1000000;
		if (sec > 2 || time >= 1*1000000) {
			float float_sec;

			float_sec = (float)usec / (float)1000000;
			float_sec += sec;

			ALOGD("fps:%5.1f (%d frame per %f sec)", (float)fps_frame_count / float_sec, fps_frame_count, float_sec);
			fps_start_time  = current;
			fps_frame_count = 0;
		}
		fps_frame_count++;
	}
}
#endif

#if MONITOR_THREAD

/*! \brief Report in/out
 *  \return none
 */
static void monitor_inout_log(void)
{
	int i;
	String8 msg;

	ALOGD_IF(USE_DBGLEVEL(3),
		"report in/out count\n");

	for (i = 0; i < 16; i++) {
		const char *idname;

		if (monitor_inout_count[i][0] == 0)
			continue;

		switch (i) {
		case 0:
			idname = "prepare"; break;
		case 1:
			idname = "set"; break;
		case 2:
			idname = "event"; break;
		case 3:
			idname = "blank"; break;
		case 4:
			idname = "query"; break;
		case 5:
			idname = "dump"; break;
		case 6:
			idname = "getConfig"; break;
		case 7:
			idname = "getAttribute"; break;
		default:
			idname = NULL; break;
		}
		if (idname) {
			msg.appendFormat("%s: [%d/%d] ", idname, monitor_inout_count[i][0], monitor_inout_count[i][1]);
		} else {
			msg.appendFormat("%d: [%d/%d] ", i, monitor_inout_count[i][0], monitor_inout_count[i][1]);
		}
	}
	ALOGD("%s", msg.string());
}

/*! \brief Report in/out thread
 *  \return none
 */
static void* monitor_thread_function(void *)
{
	int period_inout = 1000;
	while (1) {
		/* forever loop */

		/* wait next event */
		usleep(10*1000);
		period_inout--;

		if (period_inout == 0) {
			period_inout = 1000;
			monitor_inout_log();
		}
	}
	return NULL;
}
#endif

/*****************************************************************************/

/*! \brief Check gralloc handle
 *  \param[in] ctx  pointer to a hwc_context_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 */
static int check_gralloc_handle(struct hwc_context_t *ctx)
{
	const struct hw_module_t *module;
	int result;
	gralloc_module_t *gralloc;

#if defined(TARGET_BOARD_KOELSCH)
	const char *hal_name = "IMG SGX Graphics HAL";
#elif defined(TARGET_BOARD_LAGER) || defined(TARGET_BOARD_SALVATOR_M3) || \
	defined(TARGET_BOARD_SALVATOR_H3)
	const char *hal_name = "IMG Rogue Graphics HAL";
#elif defined(TARGET_BOARD_ALT)
	const char *hal_name = "IMG SGX Graphics HAL";
#endif

	UNUSED(ctx);

	result = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);

	if (result) {
		ALOGD_IF(USE_DBGLEVEL(1),
			"hw_get_module return %d", result);
		goto err;
	}

	gralloc = (gralloc_module_t *) module;
	if (strcmp(gralloc->common.name, hal_name)) {
		ALOGD_IF(USE_DBGLEVEL(1),
			"gralloc name %s not supported.", gralloc->common.name);
		goto err;
	}

	return 0;

err:
	ALOGE("error in get gralloc handle");
	return -1;
}


/*****************************************************************************/

/*! \brief Close module
 *  \param[in] dev  pointer to a hw_device_t structure
 *  \return result of processing
 *  \retval 0       always return this value
 */
static int hwc_device_close(struct hw_device_t *dev)
{
	struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
	ALOGD_IF(USE_DBGLEVEL(1),
		"close");
	if (ctx) {
		int i;
		for (i = 0; i < NELEM(ctx->base); i++) {
			if (ctx->base[i])
				delete ctx->base[i];
		}

		ctx->drm_disp->deinit();
		delete ctx->drm_disp;

		if (ctx->poll_fd >=0) {
			close(ctx->poll_fd);
		}
		free(ctx);
	}

#if MONITOR_THREAD
	monitor_thread.terminate();
#endif

	return 0;
}


/*****************************************************************************/

/*! \brief Open module
 *  \param[in] module  pointer to a hw_module_t structure
 *  \param[in] name  pointer to device name
 *  \param[in] device  pointer to a hw_device_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 *  \retval -ENOMEM      error
 */
static int hwc_device_open(const struct hw_module_t* module, const char* name,
		struct hw_device_t** device)
{
	int status = -EINVAL;
	struct hwc_context_t *dev = NULL;
	int i;

	ALOGD_IF(USE_DBGLEVEL(1),
		"open");
	if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
		dev = (hwc_context_t*)malloc(sizeof(*dev));
		if (dev == NULL) {
			/* no memory */
			status = -ENOMEM;
			goto err;
		}

		/* initialize our state here */
		memset(dev, 0, sizeof(*dev));
		dev->poll_fd = -1;

		if (check_gralloc_handle(dev)) {
			/* gralloc open error */
			goto err;
		}

		dev->notice.proc_register(const_cast<const hwc_procs_t**>(&dev->procs));

		dev->drm_disp = new DRMDisplay();
		if (dev->drm_disp == NULL || !dev->drm_disp->isValid()) {
			ALOGE("can not create direct render handle");
			goto err;
		}

		dev->drm_disp->init();

		/* initialize global object of HWCBase */
		g.init();
		dev->force_updatemode = true;
		dev->poll_fd = open("/sys/module/rcar_composer/parameters/enable_vsp", O_RDWR);
		ALOGE_IF(dev->poll_fd <= 0 && USE_DBGLEVEL(3),
			"can not open /sys/module/rcar_composer/parameters/enable_vsp");

		/* register primary display */
		dev->base[0] = new HWCPrimary(&dev->notice, dev->drm_disp);
		if (dev->base[0] == NULL || !dev->base[0]->isValid()) {
			ALOGE("can not create primary handle");
			goto err;
		}
#if USE_EXTERNAL
		dev->base[1] = new HWCExternal(&dev->notice, dev->drm_disp);
		if (dev->base[1] == NULL || !dev->base[1]->isValid()) {
			ALOGE("can not create external handle");
			goto err;
		}
#endif
#if USE_HWC_VERSION1_3 && NUM_OF_VIRTUALDISPLAY
		for (i = 2; i < NELEM(dev->base); i++) {
			dev->base[i] = new HWCVirtual(&dev->notice);
			if (dev->base[i] == NULL || !dev->base[i]->isValid()) {
				ALOGE("can not create virtual handle");
				goto err;
			}
		}
#endif

		/* initialize the procs */
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
#if USE_HWC_VERSION1_3
		dev->device.common.version = HWC_DEVICE_API_VERSION_1_3;
#elif USE_HWC_VERSION1_2
		dev->device.common.version = HWC_DEVICE_API_VERSION_1_2;
#else
		dev->device.common.version = HWC_DEVICE_API_VERSION_1_1;
#endif
		dev->device.common.module = const_cast<hw_module_t*>(module);
		dev->device.common.close = hwc_device_close;

		dev->device.prepare = hwc_prepare;
		dev->device.set = hwc_set;
		dev->device.eventControl = hwc_eventControl;
		dev->device.blank = hwc_blank;
		dev->device.query = hwc_query;
		dev->device.dump = hwc_dump;
		dev->device.registerProcs = hwc_register_procs;
		dev->device.getDisplayConfigs = hwc_getDisplayConfigs;
		dev->device.getDisplayAttributes = hwc_getDisplayAttributes;
#if MONITOR_THREAD
		monitor_thread.start(MONITOR_THREAD_NAME, PRIORITY_DEFAULT, monitor_thread_function, NULL);
#endif
		*device = &dev->device.common;
		status = 0;
	}
err:
	if (status) {
		if (dev) {
			for (i = 0; i < NELEM(dev->base); i++) {
				if (dev->base[i])
					delete dev->base[i];
			}

			if (dev->drm_disp) {
				dev->drm_disp->deinit();
				delete dev->drm_disp;
			}

			if (dev->poll_fd >= 0) {
				close(dev->poll_fd);
			}

			/* free memory */
			free(dev);
		}
	}

	return status;
}
