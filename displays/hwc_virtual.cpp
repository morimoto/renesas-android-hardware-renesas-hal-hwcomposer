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
#include "displays/hwc_virtual.h"
#include "img_gralloc_public.h"
#include <cutils/log.h>
#include "config.h"
#include "component/hwcglobal.h"

#if DEBUG_HWC
#if USE_HWC_VERSION1_3
#include <math.h>
#define CROPWIDTH(X)  (floorf(X->sourceCropf.right)  - ceilf(X->sourceCropf.left))
#define CROPHEIGHT(X) (floorf(X->sourceCropf.bottom) - ceilf(X->sourceCropf.top))
#else
#define CROPWIDTH(X)  (X->sourceCropi.right  - X->sourceCropi.left)
#define CROPHEIGHT(X) (X->sourceCropi.bottom - X->sourceCropi.top )
#endif
#define DISPWIDTH(X)  (X->displayFrame.right  - X->displayFrame.left)
#define DISPHEIGHT(X) (X->displayFrame.bottom - X->displayFrame.top )
#endif

/*****************************/
/* implement VIRTUAL display */
/*****************************/


/*! \brief HWCVirtual initialize
 *  \param[in] list  pointer to a hwc_display_contents_1_t structure
 *  \return    result of processing
 *  \retval    true  success. always retrun this value.
 */
bool HWCVirtual::onSetupLayersel(hwc_display_contents_1_t* list) {
	IMG_native_handle_t *handle = (IMG_native_handle_t *)list->outbuf;
	int num_layer = g.num_overlay[HWC_DISPLAY_VIRTUAL];
	int num_yuv   = 1;
	int num_scaler = 1;
	int display_flag = 0;

	/* physical display uses. */
	if (g.st_connect[0] && !g.st_blank[0]) {
		display_flag++;
	}
	if (g.st_connect[1] && !g.st_blank[1]) {
		display_flag++;
	}

	if (display_flag == 0) {
		ALOGD_IF(USE_DBGLEVEL(4),
			"all physical display blanked, all layer handled GLES.");

		num_layer = 0;
		num_yuv   = 0;
	} else if (handle == NULL || composer->settarget(handle->iFormat, handle->iWidth, handle->iHeight)) {
		ALOGD_IF(USE_DBGLEVEL(4),
			"target not available, all layer handled GLES.");

		num_layer = 0;
		num_yuv   = 0;
	}

#if DEBUG_HWC
	/* extra check of design dependency. */
	if (list->numHwLayers > 1) {
		hwc_layer_1_t *fbt_layer = &list->hwLayers[list->numHwLayers-1];
		int           fbt_width  = CROPWIDTH(fbt_layer);
		int           fbt_height = CROPHEIGHT(fbt_layer);
		int           err = 0;

		if (fbt_layer->compositionType != HWC_FRAMEBUFFER_TARGET) {
			ALOGE(
				"target not available, HWC_FRAMEBUFFER_TARGET expected.");
			err = 1;
		}
		else if (fbt_layer->transform != 0) {
			ALOGE(
				"target not available, transform unexpected.");
			err = 1;
		}
		else if (fbt_width != handle->iWidth || fbt_height != handle->iHeight) {
			ALOGE(
				"target not available, dimension of FBTARGET should same outbuf.");
			err = 1;
		}
		if (fbt_width != DISPWIDTH(fbt_layer) || fbt_height != DISPHEIGHT(fbt_layer)) {
			ALOGE(
				"target not available, FBTARGET scaling unexpected.");
			err = 1;
		}

		if (err) {
			num_layer = 0;
			num_yuv   = 0;
		}
	}
#endif

	ALOGD_IF(USE_DBGLEVEL(3),
		"Virtual-Disp overlay:%d", num_layer);

	layersel->init_numlayer(num_layer, num_yuv, num_scaler);

	return true;
}


/*! \brief module for onTargetPrepare
 *  \param[in]     list          pointer to a hwc_display_contents_1_t structure
 *  \param[in,out] info          reference to a target_t structure
 *  \param[in,out] use_opengl    flag of use HWC_FRAMEBUFFER_TARGET
 *  \param[in,out] use_composer  flag of use composer
 *  \param[in]     log_flag      flag of log already generated
 *  \param[in,out] next_log_flag flag of next log_flag
 *  \return nothing
 *  \details
 *  To reduce score of MVG (McCabe's Cyclomatic Number)
 *  some code split to this function.
 *  do not call this function except for onTargetPrepare function.
 */
void HWCVirtual::onTargetPrepare_part1(hwc_display_contents_1_t* list, struct target_t& info, bool& use_opengl, bool& use_composer, const int log_flag, int& next_log_flag)
{
	UNUSED(list);
#if !defined(FORCE_HWC_COPY_FOR_VIRTUAL_DISPLAYS)
	if (!use_composer && !use_opengl) {
		/* to fill outbuf with backgroundColor, forcely enable composer. */
		if (!g.st_disable_hwc) {
			use_composer = true;
		} else {
			ALOGW_IF(log_flag==0, "ignore virtual display update.");
			next_log_flag = 1;
		}
	} else if (!use_composer && use_opengl) {
		/* frame buffer target is not handled HWC. */
		info.use_fbt = false;
		use_opengl = false;
	}
#else
	if (!use_composer && !use_opengl) {
		/* to fill outbuf with backgroundColor, forcely enable composer. */
		if (!g.st_disable_hwc) {
			use_composer = true;
		} else {
			ALOGW_IF(log_flag==0, "ignore virtual display update.");
			next_log_flag = 1;
		}
	} else {
		if (g.st_disable_hwc) {
			IMG_native_handle_t *handle     = (IMG_native_handle_t *)list->outbuf;
			IMG_native_handle_t *fbt_handle = NULL;
			if (list->numHwLayers > 0) {
				fbt_handle = (IMG_native_handle_t *)list->hwLayers[list->numHwLayers-1].handle;
			}
			if (use_opengl && 
				((handle == NULL || fbt_handle == NULL) ||
				 (handle->iFormat != fbt_handle->iFormat))) {
				/* format conversion not available. */
				ALOGW_IF(log_flag==0, "ignore virtual display update.");
				/* frame buffer target is not handled HWC. */
				info.use_fbt = false;
				use_opengl = false;
				use_composer = false;
				next_log_flag = 1;
			}
		}
	}
#endif
}


/*! \brief prepare composition target buffers for physical display
 *  \param[in]     list     pointer to a hwc_display_contents_1_t structure
 *  \param[in,out] info     reference to a target_t structure
 *  \return nothing
 */
void HWCVirtual::onTargetPrepare(
	hwc_display_contents_1_t* list,
	struct target_t& info)
{
	bool   use_opengl   = info.use_fbt;
	bool   use_composer = info.use_ovl;
	static int log_flag = 0;
	int    next_log_flag = log_flag;

	onTargetPrepare_part1(list, info, use_opengl, use_composer, log_flag, next_log_flag);

	if (use_composer || use_opengl) {
		IMG_native_handle_t *handle = (IMG_native_handle_t *)list->outbuf;

		if (!handle || handle->fd[0] < 0) {
			ALOGE("outbuf invalid.");
		} else {
			if ((list->flags & HWC_GEOMETRY_CHANGED) || force_geometry_changed) {
				if (composer->settarget(handle->iFormat, handle->iWidth, handle->iHeight)) {
					ALOGE_IF(USE_DBGLEVEL(4),
						"can not settarget.");
					outbuf_invalid = true;
				} else {
					outbuf_invalid = false;
					next_log_flag = 0;
				}
			}
			if (outbuf_invalid) {
				ALOGE("virtual display outbuf not supported.");
			} else {
				/* sett buffer information. */
				info.fd     = handle->fd[0];
				info.fence  = list->outbufAcquireFenceFd;
				info.cached = !!(handle->usage & (GRALLOC_USAGE_SW_WRITE_MASK | GRALLOC_USAGE_SW_READ_MASK));
			}
		}
	}

	log_flag = next_log_flag;
}


/*! \brief process result of composition target buffers for virtual display.
 *  \param[in]      list               pointer to a hwc_display_contents_1_t structure
 *  \param[in]      composer_fence     fence of composer.
 *  \param[in]      drm_fence          fence of drm.
 *  \param[in]      info               reference to a target_t structure
 *  \param[in,out]  retire_fence       retire fence.
 *  \return nothing
 *  \details
 *  set retire_fence, if composer provide fence.
 */
void HWCVirtual::onTargetExecute(
	hwc_display_contents_1_t* list,
	int              composer_fence,
	int&             drm_fence,
	struct target_t& info,
	int&             retire_fence)
{
	UNUSED(drm_fence);
	UNUSED(info);

	if (composer_fence >= 0) {
		retire_fence = dup(composer_fence);
	} else if (list->outbufAcquireFenceFd >= 0) {
		retire_fence = dup(list->outbufAcquireFenceFd);
	}
}


/*! \brief HWCVirtual initialize
 *  \param[in] obj  pointer to a HWCNotice structure
 */
HWCVirtual::HWCVirtual(HWCNotice *obj) :
	notice(obj), outbuf_invalid(true)
{

	display_type = HWC_DISPLAY_VIRTUAL;

	/* virtual display always connected. */
	set_connect_state(true);

}

/*! \brief HWCVirtual destructor
 */
HWCVirtual::~HWCVirtual()
{
	/* nothing to do */
}


