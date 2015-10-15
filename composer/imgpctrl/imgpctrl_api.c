/*************************************************************************/ /*
 imgpctrl_api.c
 imgpctrl API file.

 Copyright (C) 2013 Renesas Electronics Corporation

 License        GPLv2

 If you wish to use this file under the terms of GPL, following terms are
 effective.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ /*************************************************************************/

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>

#include "imgpctrl_private.h"
#include "imgpctrl_common.h"

#define IMGPCTRL_BLEND_SEM_TIMEOUT msecs_to_jiffies(10000)

void *screen_graphics_new(struct screen_grap_new *grap_new)
{
	struct screen_grap_handle *grap_handle;
	int ret;

	MSG_TRACE("[APIK] IN |[%s]. Line:%d\n", __func__, __LINE__);

	if (grap_new == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%d] parameter NULL\n",
			__LINE__);
		return NULL;
	}

	if (grap_new->notify_graphics_image_blend == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%d] callback func NULL\n",
			__LINE__);
		return NULL;
	}

	grap_handle = kzalloc(sizeof(struct screen_grap_handle), GFP_KERNEL);

	if (grap_handle == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%s] kzalloc() grap_handle NULL error\n",
			__func__);
		return NULL;
	}

	grap_handle->notify_graphics_image_blend =
		(IMGPCTRL_BLEND_CB)grap_new->notify_graphics_image_blend;

	ret = alloc_heap_buf(grap_handle);
	if (ret != SMAP_LIB_GRAPHICS_OK) {
		MSG_ERROR(
			"[APIK] ERR|[%s][%d] alloc_heap_buf() error\n",
			__func__,
			__LINE__);
		kfree(grap_handle);
		return NULL;
	}
	IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_NEW, FUNC_IN, 0,
		(u_long *)NULL , grap_handle);

	sema_init(&(grap_handle->mem_info.mem_sem), 1);

	sema_init(&(grap_handle->imgpctrl_info.imgpctrl_sem), 1);
	spin_lock_init(&(grap_handle->spin_info.bl_lock));

	spin_lock_init(&(grap_handle->trace_spin.bl_lock));
	sema_init(&(grap_handle->err_sem.imgpctrl_sem), 1);

	MSG_TRACE("[APIK] OUT|[%s]. Line:%d.\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_NEW, FUNC_OUT_NRM, 0,
		(u_long *)NULL , grap_handle);

	return (void *)grap_handle;
}
EXPORT_SYMBOL(screen_graphics_new);

int screen_graphics_image_blend(struct screen_grap_image_blend *grap_blend)
{
	short layer;
	bool null_f;
	int ret;
	u_long dbg_info;

	struct screen_grap_handle *bl_hdl;

	MSG_TRACE("[APIK] IN |[%s]. Line:%d\n", __func__, __LINE__);

	if (grap_blend == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%d] screen_grap_image_blend is NULL.\n",
			__LINE__);
		return SMAP_LIB_GRAPHICS_PARAERR;
	}

	if (grap_blend->handle == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%d] imgpctrl handle is NULL.\n",
			__LINE__);
		return SMAP_LIB_GRAPHICS_PARAERR;
	}

	bl_hdl = (struct screen_grap_handle *)grap_blend->handle;

	IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_IMAGE_BLEND, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	/* Forbidden calling screen_graphics_image_blend in blending, */
	/* when same handle is used.                                  */
	ret = down_timeout(
		&(bl_hdl->imgpctrl_info.imgpctrl_sem),
		(long)IMGPCTRL_BLEND_SEM_TIMEOUT);
	if (ret != 0) {
		dbg_info = __LINE__;
		MSG_ERROR(
			"[APIK] ERR|[%d] TIMEOUT to get imgpctrl semaphore.\n",
			__LINE__);
		IMGPCTRL_ERROR_LOG(
			FID_SCREEN_GRAPHICS_IMAGE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_IMAGE_BLEND,
			FUNC_OUT_ERR, 1, (u_long *)&dbg_info , bl_hdl);
		return SMAP_LIB_GRAPHICS_SEQERR;
	}

	null_f = 0;
	bl_hdl->layer_num = 0;
	for (layer = 0; layer < IMGPCTRL_LAYER_MAX; layer++) {
		if ((null_f == 0) && (grap_blend->input_layer[layer] != NULL)) {
			bl_hdl->layer_num++;
			(void)memcpy(
				&bl_hdl->input_layer[layer],
				grap_blend->input_layer[layer],
				sizeof(struct screen_grap_layer));
		} else if ((null_f == 1) &&
			   (grap_blend->input_layer[layer] != NULL)) {
			dbg_info = __LINE__;
			MSG_ERROR(
				"[APIK] ERR|[%d] input_layer[%d] NOT NULL.\n",
				__LINE__, layer);
			IMGPCTRL_ERROR_LOG(
				FID_SCREEN_GRAPHICS_IMAGE_BLEND,
				(u_short)dbg_info, ret, 0, bl_hdl);
			IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_IMAGE_BLEND,
				FUNC_OUT_ERR, 1, (u_long *)&dbg_info , bl_hdl);
			up(&(bl_hdl->imgpctrl_info.imgpctrl_sem));
			return SMAP_LIB_GRAPHICS_PARAERR;
		} else {
			null_f = 1;
			(void)memset(
				&bl_hdl->input_layer[layer],
				0,
				sizeof(struct screen_grap_layer));
		}
	}

	(void)memcpy(
		&bl_hdl->output_image,
		&grap_blend->output_image,
		sizeof(struct screen_grap_image_param));

	bl_hdl->bg_color = grap_blend->background_color;

	bl_hdl->user_data = grap_blend->user_data;

	bl_hdl->mode = grap_blend->mode;

	ret = make_blend_chain(bl_hdl);
	if (ret != SMAP_LIB_GRAPHICS_OK) {
		dbg_info = __LINE__;
		MSG_ERROR(
			"[APIK] ERR|[%d] make_blend_chain ret = %d\n",
			__LINE__,
			ret);
		IMGPCTRL_ERROR_LOG(
			FID_SCREEN_GRAPHICS_IMAGE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_IMAGE_BLEND,
			FUNC_OUT_ERR, 1, (u_long *)&dbg_info , bl_hdl);
		up(&(bl_hdl->imgpctrl_info.imgpctrl_sem));
		return ret;
	}

	ret = main_blend(bl_hdl);
	if (ret != SMAP_LIB_GRAPHICS_OK) {
		dbg_info = __LINE__;
		MSG_ERROR(
			"[APIK] ERR|[%d] main_blend ret = %d\n",
			__LINE__,
			ret);
		IMGPCTRL_ERROR_LOG(
			FID_SCREEN_GRAPHICS_IMAGE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_IMAGE_BLEND,
			FUNC_OUT_ERR, 1, (u_long *)&dbg_info , bl_hdl);
		up(&(bl_hdl->imgpctrl_info.imgpctrl_sem));
		return ret;
	}

	MSG_TRACE("[APIK] OUT|[%s]. Line:%d.\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_IMAGE_BLEND, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return SMAP_LIB_GRAPHICS_OK;
}
EXPORT_SYMBOL(screen_graphics_image_blend);

void screen_graphics_delete(struct screen_grap_delete *grap_delete)
{
	struct screen_grap_handle *grap_handle;
	int ret;

	MSG_TRACE("[APIK] IN |[%s]. Line:%d\n", __func__, __LINE__);

	if (grap_delete == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%d] grap_delete NULL error\n",
			__LINE__);
		return;
	}

	if (grap_delete->handle == NULL) {
		MSG_ERROR(
			"[APIK] ERR|[%d] handle NULL error\n",
			__LINE__);
	} else {
		grap_handle = grap_delete->handle;

		IMGPCTRL_TRACE_LOG(FID_SCREEN_GRAPHICS_DELETE, FUNC_IN, 0,
			(u_long *)NULL , grap_handle);

		ret = free_heap_buf(grap_handle);
		if (ret != SMAP_LIB_GRAPHICS_OK) {
			MSG_ERROR(
				"[APIK] ERR|[%d] free_heap_buf ret = %d\n",
				__LINE__,
				ret);
		}
		kfree(grap_handle);
	}

	MSG_TRACE("[APIK] OUT|[%s]. Line:%d.\n", __func__, __LINE__);
}
EXPORT_SYMBOL(screen_graphics_delete);
