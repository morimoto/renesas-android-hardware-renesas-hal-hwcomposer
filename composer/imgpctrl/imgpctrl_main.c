/*************************************************************************/ /*
 imgpctrl_main.c
 imgpctrl main file.

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

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>


#include "imgpctrl_private.h"
#include "imgpctrl_common.h"
#include "mmngr_public.h"

/******************************************************************************/
/* define prototype                                                           */
/******************************************************************************/

/* Functions for blend stack management                  */
static void imgpctrl_push(
	struct t_chain_info *stack,
	u_short command,
	struct screen_grap_handle *bl_hdl);
static struct t_exe_extinfo *imgpctrl_pop(
	struct t_chain_info *stack,
	struct screen_grap_handle *bl_hdl);

/* Functions for making blend chain functions            */
static u_short get_use_tdd(
	struct screen_grap_layer *input_layer,
	struct screen_grap_handle *bl_hdl);
static u_short get_use_resize(
	struct screen_grap_layer *input_layer,
	struct screen_grap_handle *bl_hdl);

/* Functions for Phase 3 management                      */
static void calc_xy_offset(struct screen_grap_handle *bl_hdl, u_short layer);
static int main_th_blend(void *arg);
static long make_blend_bg_param(struct screen_grap_handle *bl_hdl);
static long make_blend_param(struct screen_grap_handle *bl_hdl);
static long exe_blend(struct screen_grap_handle *bl_hdl);

/******************************************************************************/
/* imgpctrl Framework functions                                               */
/******************************************************************************/

/*********************************************************/
/* Functions for making blend chain functions            */
/*********************************************************/
int make_blend_chain(struct screen_grap_handle *bl_hdl)
{
	u_short layer;
	u_short resize_kind;
	u_long dbg_info;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAKE_CHAIN_BLEND, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	for (layer = 0; layer < IMGPCTRL_LAYER_MAX; layer++) {
		/* Init each layer[i] status. */
		(void)memset(
			&bl_hdl->info[layer],
			IMGPCTRL_R_OK,
			sizeof(struct t_layer_info));

		/* Init stack[i] */
		(void)memset(
			&bl_hdl->stack[layer],
			0,
			sizeof(struct t_chain_info));
	}

	bl_hdl->layer_1st_part_bit = 0;
	bl_hdl->layer_blend_bit = 0;
	bl_hdl->blend_req_flag = 1;
	if (bl_hdl->layer_num == 0) {
		/* only background blend is pushed. */
		imgpctrl_push(&bl_hdl->stack[0], IMGPCTRL_REQ_BLEND, bl_hdl);
		bl_hdl->stack[0].num++;
	}

	for (layer = 0; layer < bl_hdl->layer_num; layer++) {
		bl_hdl->layer_blend_bit |= (IMGPCTRL_1BIT<<layer);
		resize_kind = get_use_resize(
			&bl_hdl->input_layer[layer], bl_hdl);

		if (get_use_tdd(&bl_hdl->input_layer[layer], bl_hdl) != 0) {

			MSG_ERROR(
			"[%s] ERR: request 2DDMAC for layer[%d]. Line:%d\n",
				__func__, layer, __LINE__);

			dbg_info = __LINE__;
			IMGPCTRL_ERROR_LOG(
				FID_MAKE_CHAIN_BLEND, (u_short)dbg_info,
				SMAP_LIB_GRAPHICS_PARAERR, 0, bl_hdl);
			IMGPCTRL_TRACE_LOG(
				FID_MAKE_CHAIN_BLEND, FUNC_OUT_ERR, 1,
				(u_long *)&dbg_info, bl_hdl);
			IMGPCTRL_GRAPHDL_LOG(NULL, bl_hdl);
			return SMAP_LIB_GRAPHICS_PARAERR;
		} else {
			/* Phase3 */
			imgpctrl_push(
				&bl_hdl->stack[layer],
				((u_short)(IMGPCTRL_REQ_BLEND | resize_kind)),
				bl_hdl);
			bl_hdl->stack[layer].num++;
		}
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAKE_CHAIN_BLEND, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return SMAP_LIB_GRAPHICS_OK;
}

/******************************************************************************/
/* main blend functions                                                       */
/* If an error is SYNC, imgpctrl semaphore is freed in api.                   */
/* If an error is ASYNC, imgpctrl semaphore is freed after CB to composer.    */
/******************************************************************************/
int main_blend(struct screen_grap_handle *bl_hdl)
{
	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAIN_BLEND, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	/* initialize Phase3 management event queue */
	init_waitqueue_head(&(bl_hdl->fw_bl_queue));

	/* Run Phase3 management thread */
	bl_hdl->cb_bl_thread = kthread_run(
		&main_th_blend, bl_hdl, "imgpctrl_blend");
	if (IS_ERR(bl_hdl->cb_bl_thread) != 0) {
		MSG_FATAL("[%s] ERR: kthread_run(main_th_blend)\n", __func__);
		IMGPCTRL_ERROR_LOG(
			FID_MAIN_BLEND, __LINE__, IS_ERR(bl_hdl->cb_bl_thread),
			KTHREAD_RUN_ERR, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_MAIN_BLEND, FUNC_OUT_ERR, 0,
			(u_long *)NULL, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(NULL, bl_hdl);
		return SMAP_LIB_GRAPHICS_NG;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAIN_BLEND, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return SMAP_LIB_GRAPHICS_OK;
}


/*********************************************************/
/* Callback function for Phase 3                         */
/*********************************************************/
void fw_blend_cb(u_long ctrl_hdl, u_long udata, long c_ret)
{
	struct screen_grap_handle	*bl_hdl;
	struct t_ctrl_hdl		*ct_hdl;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);

	/* Assumed "c_ret" is following:                                */
	/*   IMGPCTRL_R_OK      : VSPM command was success.             */
	/*   IMGPCTRL_R_NG      : Error occured to VSPM_lib_DriverQuit. */
	/*   IMGPCTRL_R_TIME_OUT: VSPM command was timeout.             */

	bl_hdl = (struct screen_grap_handle *)udata;
	ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;

	IMGPCTRL_TRACE_LOG(FID_FW_BLEND_CB, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	/* save return value. */
	bl_hdl->info[0].sts = c_ret;

	/* save callback return value. */
	bl_hdl->info[0].cb_sts = ct_hdl->cb_ret[0];

	bl_hdl->blend_req_flag = 0;

	IMGPCTRL_TRACE_LOG(FID_FW_BLEND_CB, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	wake_up(&(bl_hdl->fw_bl_queue));

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
}


/*********************************************************/
/* Functions for Phase 3 management                      */
/*********************************************************/
static int main_th_blend(void *arg)
{
	long ret;

	struct screen_grap_handle *bl_hdl;
	int bl_ret = SMAP_LIB_GRAPHICS_OK;

	bl_hdl = (struct screen_grap_handle *)arg;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAIN_TH_BLEND, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	if (bl_hdl->layer_num == 0) {
		/* Only backgroung blending */
		ret = make_blend_bg_param(bl_hdl);
	} else {
		/* Normal blending */
		ret = make_blend_param(bl_hdl);
	}

	/* Assumed "ret" is IMGPCTRL_R_OK or IMGPCTRL_R_NG */
	if (ret != IMGPCTRL_R_OK)
		bl_ret = SMAP_LIB_GRAPHICS_NG;

	/* Execute blending */
	if (bl_ret != SMAP_LIB_GRAPHICS_OK) {
		/* DO NOT execute blending */
	} else {
		ret = exe_blend(bl_hdl);
		if (ret != IMGPCTRL_R_OK) {
			/* Error "ret" is */
			if (ret == IMGPCTRL_R_PARA_ERR)
				bl_ret = SMAP_LIB_GRAPHICS_PARAERR;
			else
				bl_ret = SMAP_LIB_GRAPHICS_NG;
		}
	}

	IMGPCTRL_TRACE_LOG(FID_MAIN_TH_BLEND, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	up(&(bl_hdl->imgpctrl_info.imgpctrl_sem));

	bl_hdl->notify_graphics_image_blend(bl_ret, bl_hdl->user_data);

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);

	return 0;
}

static long make_blend_bg_param(struct screen_grap_handle *bl_hdl)
{
	struct t_exe_extinfo *p_exe;

	long ret = IMGPCTRL_R_OK;
	u_long dbg_info;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAKE_BLEND_BG_PARAM, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	p_exe = imgpctrl_pop(&bl_hdl->stack[0], bl_hdl);
	if (p_exe == NULL) {
		MSG_ERROR(
		"[%s] ERR: stack of layer[0] is empty. Line:%d\n",
			__func__, __LINE__);
		ret = IMGPCTRL_R_NG;
		bl_hdl->info[0].sts = ret;

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_MAKE_BLEND_BG_PARAM, (u_short)dbg_info, ret,
			STACK_LAY_EMPTY, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_MAKE_BLEND_BG_PARAM, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(NULL, bl_hdl);

		return ret;
	}

	/* set hw_type */
	p_exe->param.hw_type = IMGPCTRL_TYPE_VSPS;

	p_exe->param.rq_layer = bl_hdl->layer_blend_bit;
	(void)memcpy(
		&p_exe->param.output_layer[0],
		&bl_hdl->output_image,
		sizeof(struct screen_grap_image_param));
	p_exe->param.bg_color = bl_hdl->bg_color;

	p_exe->status = IMGPCTRL_ST_READY;

	imgpctrl_push(&bl_hdl->stack[0], IMGPCTRL_REQ_BLEND, bl_hdl);

	bl_hdl->info[0].sts = ret;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAKE_BLEND_BG_PARAM, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return ret;
}

static long make_blend_param(struct screen_grap_handle *bl_hdl)
{
	u_short layer;
	short last_ptr;
	long ret;
	u_long dbg_info;

	struct t_exe_extinfo *p_exe;
	struct t_exe_extinfo *p_last_exe;
	struct screen_grap_layer *p_src, *p_dst;
	u_short blend_req_type;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAKE_BLEND_PARAM, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	blend_req_type = 0;
	for (layer = 0; layer < bl_hdl->layer_num; layer++) {

		if (bl_hdl->stack[layer].num == 1) {
			/* stack of Layer[i] : only blending.      */
			/* Phase 1-1, 1-2 or 2 NOT executed.       */
			p_last_exe = NULL;
		} else {
			/* stack of Layer[i] : NOT only blending.  */
			/* Phase 1-1, 1-2 or 2 are executed.       */
			/* Get Layer[i] stack pointer              */
			last_ptr = bl_hdl->stack[layer].ptr;
			p_last_exe = &bl_hdl->stack[layer].chain[last_ptr];
		}

		p_exe = imgpctrl_pop(&bl_hdl->stack[layer], bl_hdl);
		if (p_exe == NULL) {
			MSG_ERROR(
			"[%s] ERR: stack of layer[%d] is empty. Line:%d\n",
				__func__, layer, __LINE__);

			/* stack of Layer[i] is empty. */
			ret = IMGPCTRL_R_NG;
			bl_hdl->info[layer].sts = ret;

			/* output error & trace log */
			dbg_info = __LINE__;
			IMGPCTRL_ERROR_LOG(
				FID_MAKE_BLEND_PARAM, (u_short)dbg_info, ret,
				STACK_LAY_EMPTY, bl_hdl);
			IMGPCTRL_TRACE_LOG(
				FID_MAKE_BLEND_PARAM, FUNC_OUT_ERR, 1,
				(u_long *)&dbg_info, bl_hdl);
			IMGPCTRL_GRAPHDL_LOG(NULL, bl_hdl);

			return ret;
		}

		blend_req_type |= p_exe->param.rq_type;

		if (bl_hdl->stack[layer].num == 1) {
			/* stack of Layer[i] : only BLEND.            */
			/* -> Layer[i] Phase1-1,1-2,2 NOT executed.   */
			/* -> use API input parameters.               */
			p_src = &bl_hdl->input_layer[layer];
		} else {
			/* then Layer[i] Phase1-1,1-2,2 are executed. */
			/* -> use last output parameters.             */
			p_src = &p_last_exe->param.output_layer[layer];

			/* This copied inbuf info is only used        */
			/* to judge free tmpbuf.                      */
			(void)memcpy(
				&p_exe->inbuf,
				&p_last_exe->outbuf,
				sizeof(struct t_buf_info));
		}

		/* Blend input parameters are copied         */
		/* at stack[0].chain[0].param.input_layer[i] */
		p_dst = &bl_hdl->stack[0].chain[0].param.input_layer[layer];
		(void)memcpy(p_dst, p_src, sizeof(struct screen_grap_layer));

		calc_xy_offset(bl_hdl, layer);
	}

	/* Blend output parameters are copied         */
	/* at stack[0].chain[0].param.output_layer[0] */
	p_exe = &bl_hdl->stack[0].chain[0];

	p_exe->param.rq_layer = bl_hdl->layer_blend_bit;

	/* set hw_type */
	p_exe->param.hw_type  = IMGPCTRL_TYPE_VSPS;
	if (bl_hdl->mode == RT_GRAPHICS_ALLGPU_MODE) {

		/* In case of RT_GRAPHICS_ALLGPU_MODE, */
		/* num of input_layer is always 1.     */

		/* use SYS-DMAC */
		p_exe->param.hw_type = IMGPCTRL_TYPE_SYSDMAC;
	}

	p_exe->param.bg_color = bl_hdl->bg_color;

	p_dst = &p_exe->param.output_layer[0];
	(void)memcpy(
		p_dst,
		&bl_hdl->output_image,
		sizeof(struct screen_grap_image_param));

	p_exe->status = IMGPCTRL_ST_READY;

	imgpctrl_push(&bl_hdl->stack[0], blend_req_type, bl_hdl);

	ret = IMGPCTRL_R_OK;
	bl_hdl->info[0].sts = ret;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_MAKE_BLEND_PARAM, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return ret;
}

static long exe_blend(struct screen_grap_handle *bl_hdl)
{
	short last_ptr;
	u_long ctrl_hdl;
	long ret;
	u_long dbg_info;
	long err_ret;

#if (RT_GRAPHICS_TRACE_ID > 1)
	short i, ptr;
	u_long exe_param[3+20*4+8];
#endif

	struct t_exe_extinfo *p_exe;
	struct t_exe_extinfo *p_last_exe;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_EXE_BLEND, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	p_exe = imgpctrl_pop(&bl_hdl->stack[0], bl_hdl);
	if (p_exe == NULL) {
		MSG_ERROR(
			"[%s] ERR: stack of layer[%d] Empty. Line:%d\n",
			__func__, 0, __LINE__);
		ret = IMGPCTRL_R_NG;
		bl_hdl->info[0].sts = ret;

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_EXE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_EXE_BLEND, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(NULL, bl_hdl);

		return ret;
	}

	p_exe->status = IMGPCTRL_ST_EXE;

	ret = imgpctrl_ctrl_new(&ctrl_hdl, &fw_blend_cb, bl_hdl);
	p_exe->ct_hdl = ctrl_hdl;
	if (ret != IMGPCTRL_R_OK) {
		/* Error "ret" is                                     */
		/* IMGPCTRL_R_NG      : kzalloc NG.                   */
		/* IMGPCTRL_R_PARA_ERR: handle or CB pointer was NULL.*/
		/* -> VSPM for Layer[i] does not run,                 */
		/*    so clear Layer[i] request simply.               */
		MSG_ERROR("[%s] ERR: imgpctrl_ctrl_new:%ld\n", __func__, ret);
		bl_hdl->info[0].sts = ret;

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_EXE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_EXE_BLEND, FUNC_OUT_ERR, 1,
			(u_long *)&ctrl_hdl, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(&(p_exe->param), bl_hdl);

		return ret;
	}

#if (RT_GRAPHICS_TRACE_ID > 1)
	/* Save Phase3 Parameters for Trace Log *******************************/
	ptr = 0;
	exe_param[ptr++] = p_exe->param.hw_type;
	exe_param[ptr++] = p_exe->param.rq_type;
	exe_param[ptr++] = p_exe->param.rq_layer;
	for (i = 0; i < bl_hdl->layer_num; i++) {
		exe_param[ptr++] = p_exe->param.input_layer[i].image.width;
		exe_param[ptr++] = p_exe->param.input_layer[i].image.height;
		exe_param[ptr++] = p_exe->param.input_layer[i].image.stride;
		exe_param[ptr++] = p_exe->param.input_layer[i].image.stride_c;
		exe_param[ptr++] = p_exe->param.input_layer[i].image.format;
		exe_param[ptr++] =
			(u_long)(p_exe->param.input_layer[i].image.address);
		exe_param[ptr++] =
			(u_long)(p_exe->param.input_layer[i].image.address_c0);
		exe_param[ptr++] =
			(u_long)(p_exe->param.input_layer[i].image.address_c1);
		exe_param[ptr++] = p_exe->param.input_layer[i].rect.x;
		exe_param[ptr++] = p_exe->param.input_layer[i].rect.y;
		exe_param[ptr++] = p_exe->param.input_layer[i].rect.width;
		exe_param[ptr++] = p_exe->param.input_layer[i].rect.height;
		exe_param[ptr++] = p_exe->param.input_layer[i].alpha;
		exe_param[ptr++] = p_exe->param.input_layer[i].rotate;
		exe_param[ptr++] = p_exe->param.input_layer[i].mirror;
		exe_param[ptr++] =
			(u_long)p_exe->param.input_layer[i].key_color;
		exe_param[ptr++] = p_exe->param.input_layer[i].premultiplied;
		exe_param[ptr++] = p_exe->param.input_layer[i].alpha_coef;
		exe_param[ptr++] = p_exe->param.ext_input[i].x_offset;
		exe_param[ptr++] = p_exe->param.ext_input[i].y_offset;
	}
	exe_param[ptr++] = p_exe->param.output_layer[0].image.width;
	exe_param[ptr++] = p_exe->param.output_layer[0].image.height;
	exe_param[ptr++] = p_exe->param.output_layer[0].image.stride;
	exe_param[ptr++] = p_exe->param.output_layer[0].image.stride_c;
	exe_param[ptr++] = p_exe->param.output_layer[0].image.format;
	exe_param[ptr++] =
		(u_long)(p_exe->param.output_layer[0].image.address);
	exe_param[ptr++] =
		(u_long)(p_exe->param.output_layer[0].image.address_c0);
	exe_param[ptr++] =
		(u_long)(p_exe->param.output_layer[0].image.address_c1);
	IMGPCTRL_TRACE_LOG(
		FID_EXE_BLEND, CTRL_EXE_BC, (u_char)ptr, &exe_param[0], bl_hdl);
	/**********************************************************************/
#endif

	ret = imgpctrl_ctrl_exe(
		ctrl_hdl, &p_exe->param, (u_long)bl_hdl);
	if (ret != IMGPCTRL_R_OK) {
		/* Error "ret" is                                     */
		/* IMGPCTRL_R_NG      : kzalloc was NG for vsp_hdl.   */
		/* IMGPCTRL_R_SEQ_ERR : ctrl_hdl was NULL.            */
		/* IMGPCTRL_R_PARA_ERR: 1. p_exe->param was NULL.     */
		/*                      2. hw_type was illigal.       */
		/* IMGPCTRL_R_VSPM_INIT_NG : init fail for VSPM.      */
		/* IMGPCTRL_R_VSPM_ENTRY_NG: entry fail for VSPM.     */
		/* -> VSPM for Layer[i] does not run,                 */
		/*    so clear Layer[i] request simply.               */
		MSG_ERROR("[%s] ERR: imgpctrl_ctrl_exe:%ld\n", __func__, ret);
		bl_hdl->info[0].sts = ret;

		err_ret = imgpctrl_ctrl_delete(ctrl_hdl, bl_hdl);
		if (err_ret != IMGPCTRL_R_OK) {
			/* Error "ret" is IMGPCTRL_R_NG or IMGPCTRL_R_SEQ_ERR */
			MSG_ERROR(
				"[%s] ERR: Layer[0] imgpctrl_ctrl_delete %ld\n",
				__func__, ret);

			/* output error & trace log */
			dbg_info = __LINE__;
			IMGPCTRL_ERROR_LOG(
				FID_EXE_BLEND, (u_short)dbg_info, ret,
				0, bl_hdl);
		}

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_EXE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_EXE_BLEND, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(&(p_exe->param), bl_hdl);

		return ret;
	}

	/* wait blend *************************************************/
	(void)wait_event_interruptible(
		(bl_hdl->fw_bl_queue),
		(bl_hdl->blend_req_flag == 0));
	/**************************************************************/

	last_ptr = bl_hdl->stack[0].ptr;
	p_last_exe = &bl_hdl->stack[0].chain[last_ptr];
	p_last_exe->status = IMGPCTRL_ST_NEXT;

	ret = imgpctrl_ctrl_delete(p_last_exe->ct_hdl, bl_hdl);
	if (ret != IMGPCTRL_R_OK) {
		/* Error "ret" is IMGPCTRL_R_NG or IMGPCTRL_R_SEQ_ERR. */
		MSG_ERROR(
			"[%s] ERR: Layer[0] imgpctrl_ctrl_delete %ld\n",
			__func__, ret);

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_EXE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_EXE_BLEND, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(&(p_last_exe->param), bl_hdl);

		return ret;
	}

	ret = bl_hdl->info[0].sts;
	if (ret != IMGPCTRL_R_OK) {
		MSG_ERROR(
			"[%s] ERR: (ASYNC) Layer[0] ctrl_main : %ld\n",
			__func__, ret);

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_EXE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_EXE_BLEND, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(&(p_last_exe->param), bl_hdl);

		return ret;
	}

	ret = bl_hdl->info[0].cb_sts;
	if (ret != R_VSPM_OK) {
		MSG_ERROR(
			"[%s] ERR: (ASYNC) Layer[0] Callback(VSPM) : %ld\n",
			__func__, ret);

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_EXE_BLEND, (u_short)dbg_info, ret,
			0, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_EXE_BLEND, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(&(p_last_exe->param), bl_hdl);

		return IMGPCTRL_R_NG;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_EXE_BLEND, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return IMGPCTRL_R_OK;
}

/*********************************************************/
/* Functions for MMNGR I/F control                       */
/*********************************************************/
int alloc_heap_buf(struct screen_grap_handle *bl_hdl)
{
	u_long v_start_addr;
	u_long log_addr;
	u_long offset;
	u_long trace2_size;
	u_long phys_trace1_addr;
	u_long prev_phys_addr;

	u_long *log_bgn_addr;
	u_long *trace_log1_addr;
	u_long *trace_log2_addr;
	u_long *err_log_addr;
	u_long next_ptr_addr;

	if ((RT_GRAPHICS_TRACE_ID < 0)
		|| (4 < RT_GRAPHICS_TRACE_ID)) {
		MSG_FATAL(
			"[%s] FATAL ERR: Trace ID error.\n",
			__func__);
		return SMAP_LIB_GRAPHICS_NG;
	}

	/*********************************************/
	/* alloc memory                              */
	/*********************************************/

	(bl_hdl->mem_info.buf_info.size) =
	(4096 * RT_GRAPHICS_EXTRA_BUFF_NUM);

	if (RT_GRAPHICS_EXTRA_BUFF_NUM > 0) {	/*  log on */

#ifdef LINUX_USE
		(bl_hdl->log_start_ptr) =
			kzalloc((4096 * RT_GRAPHICS_EXTRA_BUFF_NUM),
			GFP_KERNEL);
#else
		if (bl_hdl->mem_info.buf_info.v_addr == (u_long)NULL) {
			(bl_hdl->log_start_ptr) =
			kzalloc((4096 * RT_GRAPHICS_EXTRA_BUFF_NUM),
			GFP_KERNEL);
			MSG_ERROR(
			"[%s] line:%d logs tart ptr for trace = %p.\n",
			__func__, __LINE__, bl_hdl->log_start_ptr);
		} else {
			bl_hdl->log_start_ptr =
			(u_long *)(bl_hdl->mem_info.buf_info.v_addr);
			MSG_ERROR(
			"[%s] line:%d logs tart ptr for trace = %p.\n",
			__func__, __LINE__, bl_hdl->log_start_ptr);
		}
#endif

		if (bl_hdl->log_start_ptr == NULL) {
			bl_hdl->mem_info.buf_info.v_addr = 0;
			MSG_FATAL(
				"[%s] FATAL ERR: virtula addr is NULL.\n",
				__func__);
			return SMAP_LIB_GRAPHICS_NG;
		}

		v_start_addr = (u_long)(bl_hdl->log_start_ptr);
		MSG_DEBUG("[%s] : virtual start_addr = 0x%lx\n", __func__,
			v_start_addr);

		bl_hdl->mem_info.buf_info.v_addr = v_start_addr;

		/* set debug infomation address */
		log_addr = v_start_addr;

		/* 16byte align */
		log_addr += 15;
		log_addr &= 0xFFFFFFF0U;

		bl_hdl->log_start_ptr = (u_long *)log_addr;
		MSG_DEBUG("[%s] : log_start_addr = 0x%lx\n", __func__,
			log_addr);

		/*********************************************/
		/* set log address infomation                */
		/*********************************************/

		/* log_bgn_addr : beginning address of log address */
		log_bgn_addr = bl_hdl->log_start_ptr;

		/* trace_log1_addr : beginning address of trace log1 address */
		trace_log1_addr = (u_long *)(log_addr +
				IMGPCTRL_LOG_ADDRESS_SIZE
				+ IMGPCTRL_ERROR_LOG_SIZE);

		/* set trace log1 address (trace1_add) */
		*log_bgn_addr = log_addr
				+ IMGPCTRL_LOG_ADDRESS_SIZE
				+ IMGPCTRL_ERROR_LOG_SIZE;

		MSG_DEBUG("[%s] : trace log1 addr = 0x%lx\n", __func__,
			*log_bgn_addr);

		/* set hdl infomation address (err_hdl_add) */
		offset = ((5 <= RT_GRAPHICS_EXTRA_BUFF_NUM) ?
				v_start_addr +
				+ 4096 : 0x0);

		*(log_bgn_addr + 1) = offset;

		MSG_DEBUG("[%s] : hdl info addr = 0x%lx\n", __func__,
			offset);

		/* set trace log2 infomation (trace2_add/trace2_size) */
		if ((RT_GRAPHICS_TRACE_ID == 4)
			&& (7 <= RT_GRAPHICS_EXTRA_BUFF_NUM)) {
			offset = v_start_addr + (4096 * 5);
			trace2_size = 4096 * (RT_GRAPHICS_EXTRA_BUFF_NUM - 5);
		} else {
			offset = 0x0;
			trace2_size = 0x0;
		}
		*(log_bgn_addr + 2) = offset;
		*(log_bgn_addr + 3) = trace2_size;

		trace_log2_addr = (u_long *)offset;

		MSG_DEBUG("[%s] : trace log2 addr = 0x%lx\n", __func__,
			offset);
		MSG_DEBUG("[%s] : trace log2 size = 0x%lx\n", __func__,
			trace2_size);

		/* get previous physical tracelog1 addr */
		prev_phys_addr = *(log_bgn_addr + 4);

		/* get current physical tracelog1 addr */
		phys_trace1_addr = ((bl_hdl->mem_info.buf_info.p_addr) << 12)
				+ (IMGPCTRL_LOG_ADDRESS_SIZE)
				+ (IMGPCTRL_ERROR_LOG_SIZE);

		MSG_DEBUG("[%s] : previous phys_addr = 0x%lx\n", __func__,
			prev_phys_addr);
		MSG_DEBUG("[%s] : current phys_addr = 0x%lx\n", __func__,
			phys_trace1_addr);

		/*********************************************/
		/* check first setting                       */
		/*********************************************/

		if (prev_phys_addr != phys_trace1_addr) {
			/*********************************************/
			/* first setting prosess                     */
			/*********************************************/

			/* set physical address (trace1_add_phys) */
			*(log_bgn_addr + 4) = phys_trace1_addr;

			/* initialise error log information */
			err_log_addr = (u_long *)(log_addr +
					IMGPCTRL_LOG_ADDRESS_SIZE);

			/* clear error trace change flag */
			*(err_log_addr + 1) = 0x0;

			/* clear error code index */
			*(err_log_addr + 2) = 0x0;

			/* set next_ptr of trace log1 */
			*trace_log1_addr = (u_long)(trace_log1_addr + 1);

			if (trace_log2_addr != (u_long *)NULL) {
				/* set next_ptr of trace log2 */
				/* trace_log2_addr : beginning address */
				/* of trace log2 */
				*trace_log2_addr =
					(u_long)(trace_log2_addr + 1);
			}

			/* save virtual address */
			*(log_bgn_addr + 5) = v_start_addr;
			MSG_DEBUG("[%s] : save virtual address = 0x%lx\n",
				 __func__, v_start_addr);

		} else {

			MSG_DEBUG("[%s] : current virtual address = 0x%lx\n",
				__func__, v_start_addr);
			MSG_DEBUG("[%s] : previous virtual address = 0x%lx\n",
				 __func__, *(log_bgn_addr + 5));

			/* set next_ptr of trace log1 */
			next_ptr_addr = *trace_log1_addr;
			MSG_DEBUG("[%s] : previous log1 next_ptr = 0x%lx\n",
				__func__, next_ptr_addr);

			*trace_log1_addr =
				((v_start_addr > *(log_bgn_addr + 5)) ?
				(next_ptr_addr + (v_start_addr -
					*(log_bgn_addr + 5))) :
				(next_ptr_addr - (*(log_bgn_addr + 5) -
					v_start_addr)));

			MSG_DEBUG("[%s] : next log1 next_ptr = 0x%lx\n",
				__func__, next_ptr_addr);

			if (trace_log2_addr != (u_long *)NULL) {
				/* set next_ptr of trace log2 */
				/* trace_log2_addr : beginning address of */
				/* trace log2 */
				next_ptr_addr = *trace_log2_addr;
				MSG_DEBUG(
				"[%s] : previous log2 next_ptr = 0x%lx\n",
					__func__, next_ptr_addr);

				*trace_log2_addr =
				((v_start_addr > *(log_bgn_addr + 5)) ?
				next_ptr_addr +
				(v_start_addr - *(log_bgn_addr + 5)) :
				next_ptr_addr -
				(*(log_bgn_addr + 5) - v_start_addr));

				MSG_DEBUG(
				"[%s] : next log2 next_ptr = 0x%lx\n",
					__func__, next_ptr_addr);
			}

			/* save virtual address */
			*(log_bgn_addr + 5) = v_start_addr;

		}
	} else { /* log off */
		bl_hdl->log_start_ptr = NULL;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);

	return SMAP_LIB_GRAPHICS_OK;
}

int free_heap_buf(struct screen_grap_handle *bl_hdl)
{
	int ret = IMGPCTRL_R_OK;
	u_long *addr;
	u_long tmp_addr;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);

	MSG_DEBUG("[%s] : size = %ld\n",
		__func__, bl_hdl->mem_info.buf_info.size);
	MSG_DEBUG("[%s] : p_addr = 0x%lx\n",
		__func__, bl_hdl->mem_info.buf_info.p_addr);
	MSG_DEBUG("[%s] : h_addr = 0x%lx\n",
		__func__, bl_hdl->mem_info.buf_info.h_addr);
	MSG_DEBUG("[%s] : v_addr = 0x%lx\n",
		__func__, bl_hdl->mem_info.buf_info.v_addr);
	MSG_DEBUG("[%s] : flag = %ld\n",
		__func__, bl_hdl->mem_info.buf_info.flag);

	IMGPCTRL_TRACE_LOG(FID_FREE_HEAP_BUF, MMNGR_ALLOC_BC, 0,
		(u_long *)NULL , bl_hdl);

	if (bl_hdl->mem_info.buf_info.p_addr != (u_long)NULL) {
		addr = (u_long *)((u_long)bl_hdl->log_start_ptr +
				IMGPCTRL_LOG_ADDRESS_SIZE);
		MSG_DEBUG("[%s] : log address = 0x%p\n", __func__,
			addr);

		/* clear error trace change flag */
		*(addr + 1) = 0x0;

		/* release virtual memory */
		tmp_addr = bl_hdl->mem_info.buf_info.v_addr;
		if (tmp_addr != (u_long)NULL)
			kfree((void *)tmp_addr);
	}


	bl_hdl->mem_info.buf_info.size = 0;
	bl_hdl->mem_info.buf_info.p_addr = (u_long)NULL;
	bl_hdl->mem_info.buf_info.h_addr = (u_long)NULL;
	bl_hdl->mem_info.buf_info.v_addr = (u_long)NULL;
	bl_hdl->mem_info.buf_info.flag = 0;

	MSG_DEBUG("[%s] : free heap buf end.\n", __func__);

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);

	return ret;
}

static inline u_short get_use_tdd(
	struct screen_grap_layer *input_layer,
	struct screen_grap_handle *bl_hdl)
{
	u_short ret = 0x0;
	u_short mirror_bit =
		((u_short)RT_GRAPHICS_MIRROR_N
		| (u_short)RT_GRAPHICS_MIRROR_V
		| (u_short)RT_GRAPHICS_MIRROR_H);

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_GET_USE_TDD, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	if ((input_layer->rotate < RT_GRAPHICS_ROTATE_0)
	 || (input_layer->rotate > RT_GRAPHICS_ROTATE_270)) {
		/* Out of range rotate parameter.   */
		/* Output error message. And go on. */
		MSG_ERROR(
			"[%s] ERR: Out of range input_layer->rotate. %d\n",
			__func__, input_layer->rotate);
	} else if ((input_layer->rotate == RT_GRAPHICS_ROTATE_90)
		|| (input_layer->rotate == RT_GRAPHICS_ROTATE_180)
		|| (input_layer->rotate == RT_GRAPHICS_ROTATE_270)) {
		ret |= 0x1;
	} else {
		/* No Operation */
	}

	if ((input_layer->mirror | mirror_bit) != mirror_bit) {
		/* Out of range mirror parameter.   */
		/* Output error message. And go on. */
		MSG_ERROR(
			"[%s] ERR: Out of range input_layer->mirror. %d\n",
			__func__, input_layer->mirror);
	} else if ((input_layer->mirror & RT_GRAPHICS_MIRROR_N) != 0) {
		/* No Operation */
	} else if (((input_layer->mirror) &
		(RT_GRAPHICS_MIRROR_V | RT_GRAPHICS_MIRROR_H)) != 0) {
		ret |= 0x2;
	} else {
		/* No Operation */
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_GET_USE_TDD, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return ret;
}

static inline u_short get_use_resize(
	struct screen_grap_layer *input_layer,
	struct screen_grap_handle *bl_hdl)
{
	u_short ret = 0x0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_GET_USE_RESIZE, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	if ((input_layer->rect.width != 0)
	|| (input_layer->rect.height != 0)) {
		ret = IMGPCTRL_REQ_RESIZE;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_GET_USE_RESIZE, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return ret;
}

/* calc x_offset/y_offset for trimming blending */
static inline void calc_xy_offset(
	struct screen_grap_handle *bl_hdl,
	u_short layer)
{
	struct t_exe_info	*hdl_exe;
	struct t_ext_layer	*ext_in;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_CALC_XY_OFFSET, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	hdl_exe = &(bl_hdl->stack[0].chain[0].param);
	ext_in = &(hdl_exe->ext_input[layer]);
	ext_in->x_offset = 0;
	ext_in->y_offset = 0;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_CALC_XY_OFFSET, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);
}

/*********************************************************/
/* Functions for blend stack management                  */
/*********************************************************/
static inline void imgpctrl_push(
	struct t_chain_info *stack,
	u_short command,
	struct screen_grap_handle *bl_hdl)
{
	u_long dbg_info;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_PUSH, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	if (stack->ptr == IMGPCTRL_STACK_MAX) {
		MSG_ERROR(
			"[%s] ERR: stack is full. Line:%d\n",
			__func__, __LINE__);

		/* output error & trace log */
		dbg_info = __LINE__;
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_PUSH, (u_short)dbg_info, 0,
			STACK_FULL, bl_hdl);
		IMGPCTRL_TRACE_LOG(
			FID_IMGPCTRL_PUSH, FUNC_OUT_ERR, 1,
			(u_long *)&dbg_info, bl_hdl);
		IMGPCTRL_GRAPHDL_LOG(NULL, bl_hdl);
		return;
	} else {
		/* command is pushed. */
		stack->chain[stack->ptr].param.rq_type = command;
		/* set next pointer. */
		stack->ptr++;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_PUSH, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return;
}

static inline struct t_exe_extinfo *imgpctrl_pop(
	struct t_chain_info *stack,
	struct screen_grap_handle *bl_hdl)
{
	struct t_exe_extinfo	*p_exe;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_POP, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	if (stack->ptr == 0) {
		p_exe = NULL;
	} else {
		/* decreament stack pointer. */
		stack->ptr--;
		/* command is popped. */
		p_exe = &stack->chain[stack->ptr];
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_POP, FUNC_OUT_NRM, 0,
		(u_long *)NULL, bl_hdl);

	return p_exe;
}
