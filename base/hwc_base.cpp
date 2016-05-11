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

#include "hwc_base.h"
#include "config.h"
#include <sync/sync.h>
#include <img_gralloc_public.h>
#include <cutils/log.h>
#include "component/hwcglobal.h"

using namespace android;

#define DEBUG_FLAG    USE_DBGLEVEL(4)

#define WAIT_FENCE_TIME 12000

#if MAX_NATIVEBUFFER_USED == 0
#error MAX_NATIVEBUFFER_USED should over 1
#endif

/*! \brief Set connect information
 *  \param[in] con_state  true or false
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 */
int HWCBase::set_connect_state(bool con_state)
{
	int ret;

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCbase] set_connect_state disp:%d state:%d", display_type, con_state);

	if (!disp) {
		/* for virtual display */
		state_connect = con_state;
		ret = 0;
	} else if (con_state) {
		/* connect */
		int width, height, period, stride;
		float xdpi, ydpi;
		uint64_t clock;
		disp->get_attribute_size(&width, &height, &stride);
		disp->get_attribute_period(&period);
		disp->get_attribute_dpi(&xdpi, &ydpi);
		disp->get_attribute_dotclock(&clock);

		if (width == 0 || height == 0 || period == 0) {
			ALOGE("display attribute invalid.");
			state = false;
		}

		ALOGD_IF(USE_DBGLEVEL(4),
			"[HWCbase] disp:%d width:%d height:%d period:%d xdpi:%f ydpi:%f",
			display_type, width, height, period, xdpi, ydpi);

		/* update attributes */
		attributes.hwc_display_width        = width;
		attributes.hwc_display_height       = height;
		attributes.hwc_display_vsync_period       = period;
		attributes.hwc_display_dpi_x        = 1000 * xdpi;
		attributes.hwc_display_dpi_y        = 1000 * ydpi;

		/* update composer configuration */
		if (composer) {
			composer->settarget(HAL_PIXEL_FORMAT_BGRA_8888, width, height, stride);
		}
		if (layersel) {
			layersel->init_vspparam(clock);
		}

		/* update physical display information */
		g.st_dotclock[display_type] = clock;
		g.st_connect[display_type] = true;

		/* update states */
		state_connect = true;
		ret = 0;
	} else {
		/* update physical display information */
		g.st_dotclock[display_type] = 0;
		g.st_connect[display_type] = false;

		/* dis-connect */
		state_connect = false;
		ret = 0;
	}

	return ret;
}

/*! \brief  update layer select mode
 *  \return nothing
 *  \details
 *  update layer select mode.
 */
void HWCBase::update_layerselmode(void)
{
	if (g.st_disable_hwc) {
		/* composition use VSP disabled. all layer handled GLES */
		if (display_type == HWC_DISPLAY_PRIMARY) {
			layersel->init_flag(HWCLayerSelect::MODE_FLAG_VSPDMODE);
			composer->enable_bgonly(false);
		} else {
			layersel->init_flag(HWCLayerSelect::MODE_FLAG_DISABLE);
		}
		composer->enable(false);
	} else {
		/* composition use VSP enabled. layer can handle overlay. */
		layersel->init_flag(HWCLayerSelect::MODE_FLAG_DEFAULT);
		composer->enable(true);
		composer->enable_bgonly(true);
	}

	force_geometry_changed = true;
}

/*! \brief Wait for fence signal
 *  \return result of processing
 *  \retval 0       always return this value
 */
int HWCBase::wait_composer_fence(void)
{
	int w_fd = composer_fence[0];
#if USE_DBGLEVEL(4)
	String8 msg("[HWCbase] fence_list ");
	for (size_t i = 0; i < max_composer_fence; i++) {
		msg.appendFormat("%d ", composer_fence[i]);
	}
#endif

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCbase] wait_composer_fence disp:%d fd:%d", display_type, w_fd);

	if (w_fd >= 0) {
		sync_wait(w_fd, WAIT_FENCE_TIME);
		close(w_fd);
		composer_fence[0] = -1;
	}

#if USE_DBGLEVEL(4)
	ALOGD("%s", msg.string());
#endif
	return 0;
}

/*! \brief shift composer fence.
 *  \return nothing
 *  \details
 *  shift composer fence.
 */
void HWCBase::shift_composer_fence(void)
{
	size_t i;

	if (composer_fence[0] >= 0)
		close(composer_fence[0]);

	/* shift fence */
	for (i = 0; i < max_composer_fence-1; i++) {
		composer_fence[i] = composer_fence[i+1];
	}
	composer_fence[i] = -1;
}

/*! \brief Clear fence information
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return none
 */
void HWCBase::clear_acquire_fence(hwc_display_contents_1_t *list)
{
	size_t i;

	/* clear acquire_fence */
	if (list->outbufAcquireFenceFd >= 0) {
		close(list->outbufAcquireFenceFd);
		list->outbufAcquireFenceFd = -1;
	}

	for (i = 0; i < list->numHwLayers; i++) {
		hwc_layer_1_t *layer = &list->hwLayers[i];
		if (layer->compositionType == HWC_BACKGROUND) {
			/* nothing to do */
			continue;
		}
		if (layer->acquireFenceFd >= 0) {
			close(layer->acquireFenceFd);
			layer->acquireFenceFd = -1;
		}
	}
}

/*! \brief Clear fence information
 *  \param[in] list   pointer to a hwc_display_contents_1_t structure
 *  \param[in] fence  set release fence to a hwc_display_contents_1_t structure
 *  \param[in] retire set retire fence to a hwc_display_contents_1_t structure
 *  \param[in] usefbt flags current composition used HWC_FRAMEBUFFER_TARGET
 *  \param[in] sel    pointer to a to a HWCLayerSelect::layer_select_t structure
 *  \param[in] fence2 set release fence of drm plane to a hwc_display_contents_1_t structure
 *  \return none
 */
void HWCBase::set_release_fence(hwc_display_contents_1_t *list, int fence, int retire, bool usefbt, struct HWCLayerSelect::layer_select_t *sel, int fence2)
{
	int    fd;
	size_t i;

	/* set retire fence */
	fd = -1;
	if (retire >= 0)
		fd = dup(retire);
	list->retireFenceFd = fd;

	/* set release fence */
	for (i = 0; i < list->numHwLayers; i++) {
		hwc_layer_1_t *layer = &list->hwLayers[i];

		if (layer->compositionType == HWC_BACKGROUND) {
			/* nothing to do */
			continue;
		}
		if (layer->compositionType == HWC_FRAMEBUFFER) {
			/* no need release_fence */
			layer->releaseFenceFd = -1;
			continue;
		}

		fd = -1;

		if (!usefbt && layer->compositionType == HWC_FRAMEBUFFER_TARGET) {
			/* HWC not use FRAMEBUFFER_TARGET */
		} else {
			/* HWC use FRAMEBUFFER_TARGET */
			/* HWC use OVERLAY */
			if (sel && fence2 >=0) {
				size_t j;
				for (j =0; j < sel->num_overlay;j++) {
					if ((size_t)sel->ovl_index[j] == i && sel->ovl_engine[j]) {
						fd = dup(fence2);
						break;
					}
				}
			}

			if (fd < 0 && fence >= 0)
				fd = dup(fence);
		}

		layer->releaseFenceFd = fd;
	}
}

/*! \brief dump layer information for debug.
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return none
 */
void HWCBase::dump_layerlog(hwc_display_contents_1_t *list)
{
	size_t i;
	String8 msg;
	IMG_native_handle_t *handle;

#if USE_HWC_VERSION1_3
	handle = (IMG_native_handle_t *)list->outbuf;
	msg.clear();
	msg.appendFormat("\toutbuf=%p", handle);
	if (handle) {
		msg.appendFormat("(%d,%4dx%4d)", handle->iFormat, handle->iWidth, handle->iHeight);
#if USE_DBGLEVEL(1)
		msg.appendFormat(", outbufAcquireFenceFd=%d", list->outbufAcquireFenceFd);
#endif
	}
	ALOGD("%s\n", msg.string());
#endif

	ALOGD("\tretireFenceFd=%d", list->retireFenceFd);
	for (i = 0; i < list->numHwLayers; i++) {
		hwc_layer_1_t const* l = &list->hwLayers[i];
		handle = (IMG_native_handle_t *) l->handle;
		msg.clear();
		if (l->compositionType == HWC_BACKGROUND) {
			/* nothing to do */
			msg.appendFormat("\ttype=%d, flags=%08x, r:%d, g:%d, b:%d, a:%d", l->compositionType, l->flags,
				l->backgroundColor.r, l->backgroundColor.g, l->backgroundColor.b, l->backgroundColor.a);
		} else {
			msg.appendFormat("\ttype=%d, flags=%08x, handle=%p", l->compositionType, l->flags, handle);
			if (handle) {
				msg.appendFormat("(%d,%4dx%4d)", handle->iFormat, handle->iWidth, handle->iHeight);
			}
			msg.appendFormat(", tr=%02x, blend=%04x", l->transform, l->blending);
#if USE_HWC_VERSION1_2
			msg.appendFormat(", alpha=%3d", l->planeAlpha);
#endif
#if USE_HWC_VERSION1_3
			msg.appendFormat(", {%.1f,%.1f,%.1f,%.1f}", l->sourceCropf.left, l->sourceCropf.top, l->sourceCropf.right, l->sourceCropf.bottom);
#else
			msg.appendFormat(", {%d,%d,%d,%d}", l->sourceCropi.left, l->sourceCropi.top, l->sourceCropi.right, l->sourceCropi.bottom);
#endif
			msg.appendFormat(", {%d,%d,%d,%d}", l->displayFrame.left, l->displayFrame.top, l->displayFrame.right, l->displayFrame.bottom);
#if USE_DBGLEVEL(1)
			msg.appendFormat(", ac_fence=%d, re_fence=%d", l->acquireFenceFd, l->releaseFenceFd);
#endif
		}
		ALOGD("%s\n", msg.string());
	}
}

/*! \brief post process of prepare
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \param[in] info  pointer to HWCLayerSelect::layer_statistics_t structure
 *  \param[in] sel   pointer to HWCLayerSelect::layer_select_t structure
 *  \return none
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for prepare function.
 */
void HWCBase::prepare_config(hwc_display_contents_1_t* list, struct HWCLayerSelect::layer_statistics_t *info, struct HWCLayerSelect::layer_select_t *sel)
{
	size_t i;
	int                                        use_opengl;
	int                                        use_composer;

	/*************************************************************/
	/* selected layer changed to HWC_OVERLAY                     */
	/*************************************************************/
	use_opengl   = 0;
	use_composer = 0;

	if (sel->num_overlay > 0)
		use_composer = 1;

	if (info->num_layer > sel->num_overlay) {
		/* some layer will composed by FBTARGET */
		if (sel->fbt_index >= 0) {
			use_opengl = sel->use_fb;
		} else {
			ALOGE("HWC_FRAMEBUFFER_TARGET necessary, but it is not found.");
		}
	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"use_opengl:%d use_composer:%d.", use_opengl, use_composer);

	for (i = 0; i < sel->num_overlay; i++) {
		hwc_layer_1_t        *layer = &list->hwLayers[sel->ovl_index[i]];

		if (layer->compositionType == HWC_FRAMEBUFFER) {
			layer->compositionType = HWC_OVERLAY;

			switch(sel->ovl_engine[i]) {
			case HWCLayerSelect::BLENDER_COMPOSER:
				if (use_opengl)
					layer->hints = HWC_HINT_CLEAR_FB;
				break;
			case HWCLayerSelect::BLENDER_DRM:
				layer->hints = HWC_HINT_TRIPLE_BUFFER;
				if (use_opengl)
					layer->hints |= HWC_HINT_CLEAR_FB;
				break;
			case HWCLayerSelect::BLENDER_NOP:
				layer->hints = HWC_HINT_TRIPLE_BUFFER;
				break;
			default:
				break; /* do not clear FB */
			}
		}
	}

	/*************************************************************/
	/* create composition parameter                              */
	/*************************************************************/
	composer->init();
	if (sel->bglayer) {
		composer->setbgcolor(sel->bglayer->backgroundColor);
	}
	for (i = 0; i < sel->num_overlay; i++) {
		if (sel->ovl_engine[i] == HWCLayerSelect::BLENDER_COMPOSER) {
			composer->addlayer(list, sel->ovl_index[i],
				sel->ovl_sourceCrop[i],
				sel->ovl_displayFrame[i]);
		}
	}
	if (use_opengl) {
		composer->addlayer(list, sel->fbt_index,
			sel->fb_sourceCrop,
			sel->fb_displayFrame);
	}
}

/*! \brief Prepare for updating the framebuffer
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 *  \details
 *  select composition type (using Gfx or VSP)
 */
int HWCBase::prepare(hwc_display_contents_1_t* list)
{
	size_t i;
	int    layer_change_flag;
	struct HWCLayerSelect::layer_statistics_t *info;
	struct HWCLayerSelect::layer_select_t     *sel;

	if (list == NULL) {
		/* layer is not usable */
		return 0;
	}
	if (!state) {
		ALOGE("HWCBase init failed.");
		return -1;
	}

	/* waiting previous fence signaled */
	wait_composer_fence();

	/* shift composer fence. */
	shift_composer_fence();

	if (!state_connect) {
		ALOGE_IF(USE_DBGLEVEL(4),
			"not connected.");
		return -1;
	}

#if USE_DBGLEVEL(1)
	ALOGD("prepare%d\n", display_type);
	dump_layerlog(list);
#endif

	layer_change_flag = (list->flags & HWC_GEOMETRY_CHANGED) || force_geometry_changed;

	if (layer_change_flag) {
		/* HWC OVERLAY layer presents, at that case, reset to HWC_FRAMEBUFFER */
		for (i = 0; i < list->numHwLayers; i++) {
			hwc_layer_1_t        *layer  = &list->hwLayers[i];

			if (layer->compositionType == HWC_OVERLAY) {
				layer->hints = 0;
				layer->compositionType = HWC_FRAMEBUFFER;
			}
		}
	} else {
		/* nothing to do                                  */
		return 0;
	}

	/*************************************************************/
	/* get layer information                                     */
	/*************************************************************/
	info = layersel->gather_layerinfo(list);

	onSetupLayersel(list);

	/*************************************************************/
	/* select layer to use OVERLAY                               */
	/*************************************************************/
	sel = layersel->select_layer(list);

	ALOGD_IF(USE_DBGLEVEL(4),
		"num overlay %d.", sel->num_overlay);

	prepare_config(list, info, sel);

	/* force_geometry_changed is necessary for virtual display. *
	 * so, do not clean-up this flag at this point.             */

	return 0;
}

/*! \brief Update the framebuffer
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -1      error
 */
int HWCBase::set(hwc_display_contents_1_t* list)
{
	int    ret = -1;
	int    release_fd = -1;
	int    drm_release_fd = -1;
	int    retire_fd  = -1;
	struct target_t target_info = {
		use_fbt:  false,
		use_ovl:  false,
		physaddr: 0,
		fd:       -1,
		fence:    -1,
		cached:   0,
		option:   NULL};

	if (list == NULL) {
		/* layer is not usable */
		return 0;
	}
	if (!state) {
		ALOGE("HWCBase init failed.");
		goto err_exit;
	}
	if (!state_connect) {
		/* ignore display. */
		ALOGE_IF(USE_DBGLEVEL(4),
			"not connected.");
		goto err_exit;
	}

	target_info.use_fbt = (layersel->select.use_fb != 0);
	target_info.use_ovl = (layersel->select.num_overlay > 0);

#if USE_DBGLEVEL(1)
	{
		if (!target_info.use_ovl && !target_info.use_fbt) {
			ALOGD("set%d: -", display_type);
		} else if (target_info.use_ovl && !target_info.use_fbt) {
			ALOGD("set%d: all-OVL", display_type);
		} else if (!target_info.use_ovl && target_info.use_fbt) {
			ALOGD("set%d: all-EGL", display_type);
		} else {
			ALOGD("set%d: OVL+EGL", display_type);
		}
		dump_layerlog(list);
	}
#endif

	/*************************************************************/
	/* get composition target                                    */
	/*************************************************************/
	onTargetPrepare(list, target_info);

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCbase] set disp:%d composition_target(fd:%d, phys:0x%lx, fence:%d, cache:%d)",
			display_type, target_info.fd, target_info.physaddr, target_info.fence, target_info.cached);

	/*************************************************************/
	/* request to start composition                              */
	/*************************************************************/
	if (target_info.fd != -1 || target_info.physaddr != 0) {
		composer->settarget_buffer(target_info.physaddr, target_info.fd, target_info.fence, target_info.cached);

		/* request composition */
		release_fd = composer->request(list);

	}

	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCbase] set disp:%d release_fd:%d", display_type, release_fd);

	/*************************************************************/
	/* recored composer fence                                    */
	/*************************************************************/
	/* fail-safe, expire old fence */
	if (composer_fence[max_composer_fence-1] >= 0) {
		close(composer_fence[max_composer_fence-1]);
	}
	composer_fence[max_composer_fence-1] = release_fd;
	/* release_fd is closed in wait_composer_fence */

	/*************************************************************/
	/* post operation after composition.                         */
	/*************************************************************/
	onTargetExecute(list, release_fd, drm_release_fd, target_info, retire_fd);

	ret = 0;

err_exit:
	/* handle fence */
	set_release_fence(list, release_fd, retire_fd, target_info.use_fbt, &layersel->select, drm_release_fd);
	clear_acquire_fence(list);

	if (retire_fd >= 0) {
		close(retire_fd);
		retire_fd = -1;
	}
	if (drm_release_fd >= 0) {
		close(drm_release_fd);
		drm_release_fd = -1;
	}

	/* clean-up geometry change flag */
	force_geometry_changed = false;

	return ret;
}

/*! \brief Enables or disables VSYNC event
 *  \param[in] event  event type
 *  \param[in] enabled  enables or disables
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 *  \details
 *  Supported event is only HWC_EVENT_VSYNC.
 */
int HWCBase::eventControl(int event, int enabled)
{
	int rc = -EINVAL;

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCbase] eventControl disp:%d event:%d enable:%d", display_type, event, enabled);

	switch (event) {
	case HWC_EVENT_VSYNC:
		if (enabled == 0 || enabled == 1) {
			if (disp) {
				disp->setEnabled(enabled != 0);
				rc = 0;

				state_vsync = enabled;
			} else {
				ALOGD_IF(USE_DBGLEVEL(4),
					"[HWCbase] eventControl disp:%d no vsync interface", display_type);
			}
		}
		break;
	default:
		/* nothing to do */;
		ALOGD_IF(USE_DBGLEVEL(1),
			"[HWCbase] eventControl(%d) ignored.", event);
		break;
	}

	return rc;
}

/*! \brief Set blank state to display
 *  \param[in] blank  nonzero(screen off) zero(screen on)
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 */
int HWCBase::blank(int blank)
{
	int rc = -EINVAL;

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCbase] blank request disp%d blank:%d\n", display_type, blank);

	if (disp) {
		/* update physical display information */
		g.st_blank[display_type] = !!blank;

		disp->setBlank(blank);
		rc = 0;

		state_blank = blank;
	}

	return rc;
}

/*! \brief Dump message
 *  \param[in] msg  message
 *  \return none
 */
void HWCBase::dump(String8& msg)
{
	switch (display_type) {
	case HWC_DISPLAY_PRIMARY:
		msg.appendFormat("  [primary]\n");
		break;
	case HWC_DISPLAY_EXTERNAL:
		msg.appendFormat("  [external]\n");
		break;
	default:
		msg.appendFormat("  [virtual%d]\n", display_type-HWC_DISPLAY_VIRTUAL);
		break;
	}
	msg.appendFormat("    blank:%d vsync:%d connect:%d\n", state_blank, state_vsync, state_connect);
	composer->dump(msg);

	if (g.st_disable_hwc) {
		size_t i;
		struct HWCLayerSelect::layer_select_t *sel = &layersel->select;
		for (i = 0; i < sel->num_overlay; i++) {
			if (sel->ovl_engine[i] == HWCLayerSelect::BLENDER_DRM) {
				msg.appendFormat("    drm plane: crop:%d,%d,%d,%d, frame:%d,%d,%d,%d\n",
					sel->ovl_sourceCrop[i].left, sel->ovl_sourceCrop[i].top, sel->ovl_sourceCrop[i].right, sel->ovl_sourceCrop[i].bottom,
					sel->ovl_displayFrame[i].left, sel->ovl_displayFrame[i].top, sel->ovl_displayFrame[i].right, sel->ovl_displayFrame[i].bottom);
			}
		}
	}
}

/*! \brief DisplayConfigs returns handles for the configurations available
 *  on the connected display
 *  \param[out] configs pointer to configuration handles
 *  \param[in,out] numConfigs pointer to configuration number
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 */
int HWCBase::DisplayConfigs(uint32_t* configs, size_t* numConfigs)
{
	int num;
	int ret;

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCbase] DisplayConfigs disp%d configs:%p numConfigs:%p\n", display_type, configs, numConfigs);
	ALOGD_IF(USE_DBGLEVEL(4),
		"[HWCbase] DisplayConfigs disp%d state_connect:%d\n", display_type, state_connect);

	if (!state_connect) {
		/* not connected. */
		ret = -EINVAL;
	} else if (!disp) {
		/* no display. */
		ret = -EINVAL;
	} else {
		num = *numConfigs;

		if (num >= 1) {
			configs[0]  = 0;
			*numConfigs = 1;
		} else {
			*numConfigs = 0;
		}
		ret = 0;
	}
	return ret;
}

/*! \brief Display attributes
 *  \param[in] config  configuration handles
 *  \param[in] attr pointer to attributes array
 *  \param[out] values pointer to values array
 *  \return result of processing
 *  \retval 0       normal
 *  \retval -EINVAL      error
 *  \details
 *  return attributes for a specific config of a
 *  connected display
 */
int HWCBase::DisplayAttributes(uint32_t config, const uint32_t* attr, int32_t* values)
{
	int ret;

	UNUSED(config);

	ALOGD_IF(USE_DBGLEVEL(3),
		"[HWCbase] getDisplayAttributes disp%d configs:%d\n", display_type, config);

	if (!state_connect) {
		/* not connected. */
		ret = -EINVAL;
	} else if (!disp) {
		/* no display. */
		ret = -EINVAL;
	} else {
		while (*attr != HWC_DISPLAY_NO_ATTRIBUTE) {
			switch (*attr) {
			case HWC_DISPLAY_VSYNC_PERIOD:
				*values = attributes.hwc_display_vsync_period;
				break;
			case HWC_DISPLAY_WIDTH:
				*values = attributes.hwc_display_width;
				break;
			case HWC_DISPLAY_HEIGHT:
				*values = attributes.hwc_display_height;
				break;
			case HWC_DISPLAY_DPI_X:
				*values = attributes.hwc_display_dpi_x;
				break;
			case HWC_DISPLAY_DPI_Y:
				*values = attributes.hwc_display_dpi_y;
				break;
			default:
				ALOGE("attribute %d not supported", *attr);
				*values = 0;
				break;
			}

			ALOGD_IF(USE_DBGLEVEL(4),
				"[HWCbase] getDisplayAttributes disp%d attr:%d value:%d\n", display_type, *attr, *values);

			attr++;
			values ++;
		}
		ret = 0;
	}
	return ret;
}

/*! \brief Register display
 *  \param[in] obj  pointer to a DisplayBase structure
 *  \return none
 */
void HWCBase::registerDisplay(DisplayBase *obj)
{

	disp = obj;
	if (disp == NULL) {
		ALOGE_IF(DEBUG_FLAG,
			"display not available.");
	} else {
		display_type = disp->get_display();
		disp->init();
	}
}

/*! \brief prepare composition target buffers for physical display
 *  \param[in]     list     pointer to a hwc_display_contents_1_t structure
 *  \param[in,out] info     reference to a target_t structure
 *  \return nothing
 *  \details
 *  Return composition target parameters.
 *  if this functino can not provide target buffer,
 *  at that case, composition will be skipped.
 */
void HWCBase::onTargetPrepare(
	hwc_display_contents_1_t* list,
	struct target_t& info)
{
	hwc_disp_buffer *composition_target = NULL;
	static int log_flag = 0;

	UNUSED(list);

	if (!disp) {
		ALOGE("display not registered.");
	} else if (g.st_disable_hwc && !info.use_fbt) {
		if (!info.use_ovl) {
			ALOGW_IF(log_flag==0, "ignore display update.");
			log_flag = 1;
		}
	} else {
		log_flag = 0;
		/* dequeue target */
		composition_target = disp->dequeue();

		info.option = (void *)composition_target;

		if (composition_target) {
			info.physaddr       = composition_target->phys_addr;
			info.fd             = composition_target->buf_fd;
		}
	}
}

/*! \brief process result of composition target buffers for physical display.
 *  \param[in]      list               pointer to a hwc_display_contents_1_t structure
 *  \param[in]      composer_fence     fence of composer.
 *  \param[in]      drm_fence          fence of drm.
 *  \param[in]      info               reference to a target_t structure
 *  \param[in,out]  retire_fence       retire fence.
 *  \return nothing
 *  \details
 *  if composer_fence is -1, it indicate skip or error at composer.
 *  if retire_fence is not necessary, do not change input parameters.
 */
void HWCBase::onTargetExecute(
	hwc_display_contents_1_t* list,
	int              composer_fence,
	int&             drm_fence,
	struct target_t& info,
	int&             retire_fence)
{
	hwc_disp_buffer *composition_target = (hwc_disp_buffer*)info.option;
	bool            no_fbt = (composer->get_numoverlay()==0);
	bool            no_plane = !(g.st_disable_hwc && info.use_ovl);

	UNUSED(list);
	UNUSED(retire_fence);

	/* display using composer */
	if (composition_target) {
		/* for physical display */
		if (composer_fence >= 0) {
			/* queue target to display */
			disp->queue(composition_target, composer_fence, no_plane);
		} else {
			/* cancel dequeue */
			disp->dequeue_cancel(composition_target, no_plane);
		}
	}

	/* display using drm plane. */
	if (!no_plane) {
		struct HWCLayerSelect::layer_select_t *sel = &layersel->select;
		size_t i;

		/* turn on plane */
		i = 0;

		{
			hwc_layer_1_t        *layer = &list->hwLayers[sel->ovl_index[i]];

			drm_fence = disp->show_plane(layer->handle, layer->acquireFenceFd, &sel->ovl_sourceCrop[i], &sel->ovl_displayFrame[i], no_fbt);
		}
	} else {
		/* turn off plane */
		{
			disp->show_plane(NULL, -1, NULL, NULL, no_fbt);
		}
	}
}

/*! \brief HWCBase initialize
 */
HWCBase::HWCBase(): disp(NULL), composer(NULL), layersel(NULL), display_type(0), state(false), state_connect(false), state_blank(false), state_vsync(false), max_composer_fence(MAX_NATIVEBUFFER_USED), force_geometry_changed(false)
{
	size_t i;

	for (i = 0; i < MAX_NATIVEBUFFER_USED; i++) {
		composer_fence[i] = -1;
	}

	composer = new HWCComposer();
	if (composer == NULL || !composer->isValid()) {
		ALOGE("composer not available.");
		goto err;
	}

	/* vsync and display interface is created in create_display */

	layersel = new HWCLayerSelect(composer->get_maxSize(), composer->get_maxArea(), composer->get_maxRotbuffer());
	if (layersel == NULL) {
		ALOGE("layerselect not available");
		goto err;
	}

	/* set default layer select mode */
	layersel->init_flag(HWCLayerSelect::MODE_FLAG_DEFAULT);

	/* configuration of composer blend target is processed in set_connect_state. */
	/* set_connect_state(true);*/

	/* no-problem. */
	state = true;

err:

	ALOGE_IF(!state, "HWCBase initialize failed.");
}

/*! \brief HWCBase destructor
 */
HWCBase::~HWCBase()
{
	if (disp) {
		disp->deinit();
		delete disp;
	}
	if (composer)
		delete composer;
	if (layersel)
		delete layersel;
}

