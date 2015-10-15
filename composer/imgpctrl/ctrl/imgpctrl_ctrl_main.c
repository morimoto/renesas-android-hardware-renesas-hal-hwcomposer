/*************************************************************************/ /*
    imgpctrl_ctrl_main.c
    imgpctrl ctrl main function file.

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

#include <linux/slab.h>
#include <linux/jiffies.h>

#include <linux/dmaengine.h>

#include "imgpctrl_private.h"
#include "imgpctrl_common.h"

static int imgpctrl_ctrl_wait(void *ctrl_hdl);

#define IMGPCTRL_CTRL_MAIN_TIMEOUT	msecs_to_jiffies(10000)

/**********************************************************************/
/* Function     : imgpctrl_ctrl_new                                   */
/* Description  : Store the handle of the IMGPCTRL control section    */
/*                  in the destination address of the handle.         */
/* Return value : ret                                                 */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_NG                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : *p_ctrl_hdl - IMGPCTRL contrl handle pointer        */
/*                fw_ctrl_cb_fp - callback function address           */
/*                *bl_hdl - screen grap handle pointer                */
/**********************************************************************/
long imgpctrl_ctrl_new(u_long *p_ctrl_hdl, FW_CTRL_CB fw_ctrl_cb_fp,
	struct screen_grap_handle *bl_hdl)
{
	struct t_ctrl_hdl	*ct_hdl;
	u_long debug_data;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if (p_ctrl_hdl == NULL) {
		MSG_ERROR("[%s] ERR: handle pointer is null.Line:%d\n",
		 __func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_NEW, (u_short)debug_data,
			IMGPCTRL_R_PARA_ERR, HDL_PTR_NULL,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		return IMGPCTRL_R_PARA_ERR;
	}

	if (fw_ctrl_cb_fp == NULL) {
		MSG_ERROR("[%s] ERR: callback pointer is null.Line:%d\n",
		 __func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_NEW, (u_short)debug_data,
			IMGPCTRL_R_PARA_ERR, CB_PTR_NULL,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_PARA_ERR;
	}

	debug_data = sizeof(struct t_ctrl_hdl);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, KZALLOC_BC, 1,
		(u_long *)&debug_data , bl_hdl);
	ct_hdl = kzalloc(sizeof(struct t_ctrl_hdl), GFP_KERNEL);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, KZALLOC_AC, 0,
		(u_long *)NULL , bl_hdl);

	if (ct_hdl == NULL) {
		MSG_FATAL("[%s] ERR: handle can't allocate.Line:%d\n",
			 __func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_NEW, (u_short)debug_data,
			IMGPCTRL_R_NG, HDL_ALLOC_ERR,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_NG;
	} else {
		ct_hdl->fw_ctrl_cb_fp = fw_ctrl_cb_fp;
		*p_ctrl_hdl = (u_long)ct_hdl;
	}

	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_NEW, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	return IMGPCTRL_R_OK;
}

/**********************************************************************/
/* Function     : imgpctrl_ctrl_exe                                   */
/* Description  : Create a kernel thread and execute processing of    */
/*                the IMGPCTRL control section directed by the uhType.*/
/* Return value : ret                                                 */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_NG                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/*                 - IMGPCTRL_R_SEQ_ERR                               */
/*                 - IMGPCTRL_R_VSPM_INIT_NG                          */
/*                 - IMGPCTRL_R_VSPM_ENTRY_NG                         */
/* Argument     : ctrl_hdl - IMGPCTRL contrl handle pointer           */
/*                *exe_info - IMGPCTRL ctrl exe data pointer          */
/*                udata - user data of framework                      */
/**********************************************************************/
long imgpctrl_ctrl_exe(
	u_long ctrl_hdl,
	struct t_exe_info *exe_info,
	u_long udata)
{
	long	ret;
	long	vspm_ret;
	struct t_ctrl_hdl	*ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;
	void *vspm_para_h;
	struct screen_grap_handle *bl_hdl;
	u_long debug_data;

	bl_hdl = (struct screen_grap_handle *)udata;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if (ct_hdl == NULL) {
		MSG_ERROR("[%s] ERR: handle pointer is null.Line:%d\n",
		 __func__, __LINE__);
		ret = IMGPCTRL_R_SEQ_ERR;
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE, (u_short)debug_data,
			IMGPCTRL_R_SEQ_ERR, HDL_PTR_NULL,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		return ret;
	}

	if (exe_info == NULL) {
		MSG_ERROR("[%s] ERR: exe_info pointer is null.Line:%d\n",
		 __func__, __LINE__);
		ret = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE, (u_short)debug_data,
			IMGPCTRL_R_SEQ_ERR, EXEINFO_PTR_NULL,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		return ret;
	}

	ct_hdl->rq_layer	= exe_info->rq_layer;
	ct_hdl->udata		= udata;
	ct_hdl->ctrl_th		= NULL;
	/* initialise event queue */
	init_waitqueue_head(&(ct_hdl->ctrl_main_queue));

	if ((exe_info->hw_type & IMGPCTRL_TYPE_VSP) != 0) {
		vspm_para_h = kzalloc(sizeof(struct t_ctrl_hdl_vsp),
					GFP_KERNEL);
		ct_hdl->vspm_param = vspm_para_h;
		ret = imgpctrl_ctrl_exe_vsp(ctrl_hdl, exe_info);
	} else if (exe_info->hw_type == IMGPCTRL_TYPE_TDD) {
		MSG_ERROR("[%s] ERR: 2D-DMAC is not supported.Line:%d\n",
		__func__, __LINE__);
		ret = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE, (u_short)debug_data,
			IMGPCTRL_R_PARA_ERR, USE_TDD_ERR,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		return ret;
	} else if (exe_info->hw_type == IMGPCTRL_TYPE_SYSDMAC) {
		vspm_para_h = kzalloc(sizeof(struct t_ctrl_hdl_vsp),
					GFP_KERNEL);
		ct_hdl->vspm_param = vspm_para_h;
		ret = imgpctrl_ctrl_exe_dma(ctrl_hdl, exe_info);
	} else {
		MSG_ERROR("[%s] ERR: hard ware type error.Line:%d\n",
		__func__, __LINE__);
		ret = IMGPCTRL_R_PARA_ERR;
		ct_hdl->vspm_param = (void *)NULL;
		debug_data = __LINE__;
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		return ret;
	}

	if (ret == IMGPCTRL_R_OK)	{
		MSG_DEBUG("[%s] start kthread_run\n", __func__);
		ct_hdl->ctrl_th = kthread_run(&imgpctrl_ctrl_wait,
			(struct t_ctrl_hdl *)ctrl_hdl, "imgpctrl_wait");
		MSG_DEBUG("[%s] end kthread_run\n", __func__);
	} else {
		if (ct_hdl->entry_num == 0) {
			/* execute error. no wait callback */
			MSG_ERROR("[%s] ERR: execute error.Line:%d\n",
				 __func__, __LINE__);
			debug_data = __LINE__;
			/* output error log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE,
				(u_short)debug_data,
				ret, EXE_VSPTDD_ERR,
				bl_hdl);

			/* output trace log */
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data , bl_hdl);

			return ret;
		} else {
			/* execute error. wait all callback return.*/
			MSG_ERROR("[%s] ERR:execute error.Line:%d\n",
				__func__, __LINE__);
			/* output error log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE, __LINE__,
				ret, EXE_VSPTDD_ERR,
				bl_hdl);

			vspm_ret = wait_event_timeout((ct_hdl->ctrl_main_queue),
				(int)(ct_hdl->cb_num >= ct_hdl->entry_num),
				(int)IMGPCTRL_CTRL_MAIN_TIMEOUT);
			if (vspm_ret == 0) {
				MSG_FATAL("[%s]ERR:VSPM exe timeout.Line:%d\n",
				 __func__, __LINE__);
				/* output error log */
				IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE,
					__LINE__,
					vspm_ret, VSPM_EXE_TMOUT,
					bl_hdl);
			}

			MSG_DEBUG("[%s] start vspm quit\n", __func__);
			/* output trace log */
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE,
				VSPM_QUIT_BC, 0,
				(u_long *)NULL , bl_hdl);

			if (exe_info->hw_type != IMGPCTRL_TYPE_SYSDMAC) {
				/* execute quit process and return error to
					framework */
				vspm_ret =
				VSPM_lib_DriverQuit(ct_hdl->vspm_hdl);
				if (vspm_ret != 0) {
					MSG_FATAL("[%s] vspm quit error\n",
						 __func__);
				}
			}

			MSG_DEBUG("[%s] end vspm quit\n", __func__);
			/* output trace log */
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE,
				VSPM_QUIT_AC, 0,
				(u_long *)NULL , bl_hdl);

			debug_data = __LINE__;
			/* output trace log */
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data , bl_hdl);

			return ret;
		}
	}
	ret = IMGPCTRL_R_OK;

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return ret;
}

/**********************************************************************/
/* Function     : imgpctrl_ctrl_cancel                                */
/* Description  : Cancel the IMGPCTRL control section directed        */
/*                by the ctrl_hdl.                                    */
/* Return value : ret                                                 */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_NG                                    */
/*                 - IMGPCTRL_R_SEQ_ERR                               */
/* Argument     : ctrl_hdl - IMGPCTRL contrl handle pointer           */
/*                *bl_hdl - screen grap handle pointer                */
/**********************************************************************/
long imgpctrl_ctrl_cancel(u_long ctrl_hdl,
	struct screen_grap_handle *bl_hdl)
{
	long	ret = IMGPCTRL_R_OK;
	long	vspm_ret;
	struct t_ctrl_hdl	*ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;
	short	i;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if (ct_hdl == NULL) {
		MSG_ERROR("[%s] ERR: handle pointer is null.Line:%d\n",
		__func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_CANCEL,
			(u_short)debug_data,
			IMGPCTRL_R_SEQ_ERR, HDL_PTR_NULL,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_SEQ_ERR;
	}

	/* safety code */
	if (ct_hdl->entry_num > 2) {
		/* fatal error! */
		MSG_FATAL("[%s] entry_num over 2. fatal error.Line:%d\n",
		__func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_CANCEL,
			(u_short)debug_data,
			IMGPCTRL_R_NG, ENTRY_NUM_OVER,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_NG;
	}

	for (i = 0; i < ct_hdl->entry_num; i++) {
		MSG_DEBUG("[%s] start VSPM_lib_Cancel. jobid:%ld\n",
			__func__, ct_hdl->entry_jobid[i]);
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, VSPM_QUIT_BC, 0,
			(u_long *)NULL , bl_hdl);

		vspm_ret = VSPM_lib_Cancel(ctrl_hdl, ct_hdl->entry_jobid[i]);
		MSG_DEBUG("[%s] end VSPM_lib_Cancel. ret:%ld\n",
			__func__, vspm_ret);
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, VSPM_QUIT_AC, 0,
			(u_long *)NULL , bl_hdl);

		if (vspm_ret != R_VSPM_OK) {
			MSG_FATAL("[%s]ERR:VSPM cancel. entry:%d. Line:%d\n",
				__func__, i, __LINE__);
			debug_data = __LINE__;
			/* output error log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_CANCEL,
				(u_short)debug_data,
				IMGPCTRL_R_NG, VSPM_CANCEL_NG,
				bl_hdl);
			ret = IMGPCTRL_R_NG;
		}
	}

	if (ret == IMGPCTRL_R_OK) {
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_CANCEL, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);

	return ret;
}

/**********************************************************************/
/* Function     : imgpctrl_ctrl_delete                                */
/* Description  : free the handle of the IMGPCTRL control section     */
/*                directed by the imgpctrl_ctrl_handle.               */
/* Return value : ret                                                 */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_NG                                    */
/*                 - IMGPCTRL_R_SEQ_ERR                               */
/* Argument     : ctrl_hdl - IMGPCTRL contrl handle pointer           */
/*                *bl_hdl - screen grap handle pointer                */
/**********************************************************************/
long imgpctrl_ctrl_delete(u_long ctrl_hdl,
	struct screen_grap_handle *bl_hdl)
{
	struct t_ctrl_hdl	*ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;
	u_long debug_data;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if (ct_hdl == NULL) {
		MSG_ERROR("[%s] ERR: handle pointer is null.Line:%d\n",
		 __func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_DELETE,
			(u_short)debug_data,
			IMGPCTRL_R_SEQ_ERR, HDL_PTR_NULL,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_SEQ_ERR;
	}

	if (waitqueue_active(&(ct_hdl->ctrl_main_queue)) != 0) {
		MSG_ERROR("[%s] ERR:handle delete.Line:%d\n",
			__func__, __LINE__);
		/* thread process is running. so can't stop thread. */
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_DELETE,
			(u_short)debug_data,
			IMGPCTRL_R_NG, HDL_DEL_ERR,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_NG;
	}

	if (ct_hdl->vspm_param != NULL) {
		MSG_DEBUG("[%s] start free vspm_param\n", __func__);
		debug_data = (u_long)(ct_hdl->vspm_param);
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, KFREE_BC, 1,
		(u_long *)&debug_data , bl_hdl);

		kfree(ct_hdl->vspm_param);

		MSG_DEBUG("[%s] end free vspm_param\n", __func__);

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, KFREE_AC, 0,
			(u_long *)NULL , bl_hdl);
	}

	MSG_DEBUG("[%s] start free handle\n", __func__);
	debug_data = (u_long)ct_hdl;
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, KFREE_BC, 1,
	(u_long *)&debug_data , bl_hdl);

	kfree(ct_hdl);

	MSG_DEBUG("[%s] end free handle\n", __func__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, KFREE_AC, 0,
		(u_long *)NULL , bl_hdl);

	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_DELETE, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	return IMGPCTRL_R_OK;
}

/**********************************************************************/
/* Function     : imgpctrl_ctrl_wait                                  */
/* Description  : wait all VSPM execution.                            */
/*                if all process is done, terminate VSPM.             */
/* Return value : ret                                                 */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_NG                                    */
/*                 - IMGPCTRL_R_TIME_OUT                              */
/* Argument     : ctrl_hdl - IMGPCTRL contrl handle pointer           */
/**********************************************************************/

static int imgpctrl_ctrl_wait(void *ctrl_hdl)
{
	long	ret;
	long	vspm_ret;
	struct t_ctrl_hdl	*ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;
	FW_CTRL_CB fw_cb_func = ct_hdl->fw_ctrl_cb_fp;
	u_long debug_data = 0;
	struct screen_grap_handle *bl_hdl;
	int	i;
	struct dma_chan		*channel;
	dma_cookie_t		cookie;

	bl_hdl = (struct screen_grap_handle *)ct_hdl->udata;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	ret = wait_event_timeout((ct_hdl->ctrl_main_queue),
		(int)(ct_hdl->cb_num >= ct_hdl->entry_num),
		(int)IMGPCTRL_CTRL_MAIN_TIMEOUT);
	MSG_DEBUG("[%s] cb_num=%d. entry_num=%d\n",
	 __func__, ct_hdl->cb_num, ct_hdl->entry_num);
	if (ret == 0) {
		MSG_ERROR(
		"[%s]ERR: callback wait timeout.Line:%d hwtype:%x\n",
			 __func__, __LINE__,
			 bl_hdl->stack[0].chain[0].param.hw_type);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_WAIT, (u_short)debug_data,
			IMGPCTRL_R_TIME_OUT, CB_ERROR,
			bl_hdl);
		ret = IMGPCTRL_R_TIME_OUT;
	} else {
		MSG_DEBUG("[%s] callback wait remaining time : [%ld]\n",
			 __func__, ret);
		ret = IMGPCTRL_R_OK;
	}


	if (bl_hdl->stack[0].chain[0].param.hw_type ==
			IMGPCTRL_TYPE_SYSDMAC) {

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, DMA_QUIT_BC, 0,
			(u_long *)NULL , bl_hdl);

		for (i = 0; i < bl_hdl->use_chan_num; i++)  {
			channel = (bl_hdl->dma_info_p[i].chan);
			cookie =  bl_hdl->cookie[i];

			vspm_ret = (long)dma_async_is_tx_complete(
				channel, cookie, NULL, NULL);
			if (vspm_ret != DMA_SUCCESS) {
				MSG_ERROR(
				"[%s]ERR: DMAC Line:%d hwtype:%x ret:%d\n",
				 __func__, __LINE__,
				 bl_hdl->stack[0].chain[0].param.hw_type,
				 (int)vspm_ret);
				debug_data = __LINE__;
				/* output error log */
				IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_WAIT,
				 (u_short)debug_data,
				IMGPCTRL_R_NG, CB_ERROR,
				bl_hdl);
				ret = IMGPCTRL_R_NG;
			} else {
			MSG_DEBUG
			("[%s] callback wait remaining time : [%ld]\n",
				 __func__, ret);
				ret = IMGPCTRL_R_OK;
			}
			dma_release_channel(channel);
		}
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, DMA_QUIT_AC, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, VSPM_QUIT_BC, 0,
			(u_long *)NULL , bl_hdl);

		vspm_ret = VSPM_lib_DriverQuit(ct_hdl->vspm_hdl);

		if (vspm_ret != 0) {
			MSG_FATAL
			("[%s]ERR: vspm quit process error.Line:%d\n",
			 __func__, __LINE__);
			debug_data = __LINE__;
			/* output error log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_WAIT,
				(u_short)debug_data,
				vspm_ret, VSPM_QUIT_ERR,
				bl_hdl);
			if (ret == IMGPCTRL_R_OK) {
				debug_data = __LINE__;
				ret = IMGPCTRL_R_NG;
			}
		}
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, VSPM_QUIT_AC, 0,
			(u_long *)NULL , bl_hdl);

	}


	if (ret == IMGPCTRL_R_OK) {
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_WAIT, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	/* callback */
	fw_cb_func((u_long)ctrl_hdl, ct_hdl->udata, ret);

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	return ret;
}

/**********************************************************************/
/* Function     : imgpctrl_vspm_callback                              */
/* Description  : receive callback from vspm, and notify              */
/*                imgpctrl_ctrl_wait function of vspm process end.    */
/* Return value : -                                                   */
/* Argument     : ctrl_hdl - IMGPCTRL contrl handle pointer           */
/**********************************************************************/

void imgpctrl_vspm_callback(
	u_long uw_job_id, long w_result, u_long uw_user_data)
{
	struct t_ctrl_hdl	*ct_hdl = (struct t_ctrl_hdl *)uw_user_data;
	struct screen_grap_handle *bl_hdl;
	u_long debug_data;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	bl_hdl = (struct screen_grap_handle *)ct_hdl->udata;

	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSPM_CALLBACK, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if (ct_hdl->cb_num >= 2) {
		MSG_ERROR("[%s]ERR:cb_num over 1.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_VSPM_CALLBACK,
			(u_short)debug_data,
			0, ENTRY_NUM_OVER,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSPM_CALLBACK, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		wake_up(&(ct_hdl->ctrl_main_queue));
		return;
	}

	ct_hdl->cb_jobid[ct_hdl->cb_num] = uw_job_id;
	MSG_DEBUG("[%s] jobId=%ld\n", __func__, uw_job_id);

	ct_hdl->cb_ret[ct_hdl->cb_num] = w_result;
	MSG_DEBUG("[%s] cb_ret=%ld\n", __func__, w_result);

	ct_hdl->cb_num++;
	MSG_DEBUG("[%s] cb_num=%d\n", __func__, ct_hdl->cb_num);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSPM_CALLBACK, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	wake_up(&(ct_hdl->ctrl_main_queue));


	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	return;
}
/**********************************************************************/
/* Function     : imgpctrl_dma_callback                               */
/* Description  : receive callback from dma , and notify              */
/*                imgpctrl_ctrl_wait function of dma  process end.    */
/* Return value : -                                                   */
/* Argument     : ctrl_hdl - IMGPCTRL contrl handle pointer           */
/**********************************************************************/

void imgpctrl_dma_callback(void *completion)
{
	struct t_ctrl_hdl	*ct_hdl = (struct t_ctrl_hdl *)completion;
	struct screen_grap_handle *bl_hdl;
	u_long debug_data;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	bl_hdl = (struct screen_grap_handle *)ct_hdl->udata;


	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_DMA_CALLBACK, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if (ct_hdl->cb_num >= 3) {
		MSG_ERROR("[%s]ERR:cb_num over 1.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_DMA_CALLBACK,
			(u_short)debug_data,
			0, ENTRY_NUM_OVER,
			bl_hdl);

		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_DMA_CALLBACK, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		wake_up(&(ct_hdl->ctrl_main_queue));
		return;
	}


	ct_hdl->cb_num++;
	MSG_DEBUG("[%s] cb_num=%d\n", __func__, ct_hdl->cb_num);
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_DMA_CALLBACK, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);


	wake_up(&(ct_hdl->ctrl_main_queue));

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	return;
}


