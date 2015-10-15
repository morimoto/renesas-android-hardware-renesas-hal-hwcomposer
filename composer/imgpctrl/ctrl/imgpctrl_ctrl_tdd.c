/*************************************************************************/ /*
    imgpctrl_ctrl_tdd.c
    imgpctrl ctrl tdd function file.

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

#include "imgpctrl_private.h"
#include "imgpctrl_common.h"

/**********************************************************************/
/* Internal function                                                  */
/**********************************************************************/
static long imgpctrl_tdd_vspm_entry(u_long ctrl_hdl, u_short lay,
					u_short e_cnt,
					struct t_exe_info *exe_info);
static long imgpctrl_tdd_set_param(u_short lay, u_short e_cnt,
					struct t_exe_info *exe_info,
					T_TDDMAC_MODE *m2d_par,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl);
static long imgpctrl_tdd_get_layer_num(u_short rq_layer, u_short *lay,
					struct screen_grap_handle *bl_hdl);
static long imgpctrl_tdd_get_loop_num(u_int image_format, u_short *loop_num,
					struct screen_grap_handle *bl_hdl);
static long imgpctrl_tdd_set_format(u_int image_format,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl);
static long imgpctrl_tdd_set_format_c(u_int image_format,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl);
static long imgpctrl_tdd_set_rot_mir(u_short mirror, u_short rotation,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl);

/**********************************************************************/
/* Function for IMGPCTRL Control main                                 */
/**********************************************************************/
/**********************************************************************/
/* Function     : imgpctrl_ctrl_exe_tdd                               */
/* Description  : Generate parameter for 2D-DMAC.                     */
/*                Request of Initialize and Entry to the VSPM.        */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/*                 - IMGPCTRL_R_SEQ_ERR                               */
/*                 - IMGPCTRL_R_VSPM_INIT_NG                          */
/*                 - IMGPCTRL_R_VSPM_ENTRY_NG                         */
/* Argument     : ctrl_hdl - IMGPCTRL control handle address          */
/*                *exe_info - IMGPCTRL ctrl exe data pointer          */
/**********************************************************************/
long imgpctrl_ctrl_exe_tdd(u_long ctrl_hdl, struct t_exe_info *exe_info)
{
	u_long vspm_handle = 0;
	long ercd;

	struct t_ctrl_hdl *ct_hdl;
	u_short lay = 0;
	u_short loop_num = 0;
	u_short e_cnt;
	struct screen_grap_handle *ghp;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	/* Check argument */
	if (0 == ctrl_hdl) {
		MSG_ERROR("[%s] ERR: imgpctrl ctrl handle is null.Line:%d\n",
			__func__, __LINE__);
		ercd = IMGPCTRL_R_SEQ_ERR;
		return ercd;
	}
	if (0 == exe_info) {
		MSG_ERROR("[%s] ERR: imgpctrl exe info is null.Line:%d\n",
			__func__, __LINE__);
		ercd = IMGPCTRL_R_SEQ_ERR;
		return ercd;
	}

	/* Set imgpctrl ctrl handle */
	ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;

	ghp = ((struct screen_grap_handle *)(ct_hdl->udata));
	/* output trace log */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_IN, 0,
		(u_long *)NULL , ghp);

	/* Check imgpctrl ctrl tdd handle */
	if (0 == ct_hdl->vspm_param) {
		MSG_ERROR("[%s] ERR: vspm_param is null.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_SEQ_ERR;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_TDD,
		(u_short)debug_data, ercd, PARAM_NULL, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ercd;
	}

	/* Get and Check request information */
	ercd = imgpctrl_tdd_get_layer_num(exe_info->rq_layer, &lay,
		ghp);
	if (IMGPCTRL_R_OK != ercd) {
		/* GetLayerNum Err */
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_TDD,
		(u_short)debug_data, ercd, INVALID_LAYER, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ercd;
	}
	if (IMGPCTRL_REQ_ROTATE !=
		(exe_info->rq_type & IMGPCTRL_REQ_ROTATE)) {
		MSG_ERROR("[%s] ERR: invalid request\n.Line:%d",
			__func__, __LINE__);
		ercd = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_TDD,
		(u_short)debug_data, ercd, INVALID_REQUEST, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ercd;
	}

	/* Get loop times (by color-format) */
	ercd = imgpctrl_tdd_get_loop_num(
		exe_info->input_layer[lay].image.format,
		&loop_num, ghp);
	if (IMGPCTRL_R_OK != ercd) {
		/* GetLoopNum Err */
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_TDD,
		(u_short)debug_data, ercd, INVALID_LAYER, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ercd;
	}

	/* Debug information */
	imgpctrl_cmn_dbg_exe_param(exe_info, lay);

	/* Initialize VSPM */
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, VSPM_INIT_BC, 0,
		(u_long *)NULL , ghp);
	ercd = VSPM_lib_DriverInitialize(&vspm_handle);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, VSPM_INIT_AC, 0,
		(u_long *)NULL , ghp);
	if (ercd != R_VSPM_OK) {
		/* VSPM Init NG */
		MSG_FATAL("[%s] ERR: VSPM Init NG.Line:%d\n",
			__func__, __LINE__);
		ercd = IMGPCTRL_R_VSPM_INIT_NG;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_TDD,
		(u_short)debug_data, ercd, VSPM_INIT_NG, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		return ercd;
	}

	/* Save VSPM handle */
	ct_hdl->vspm_hdl = vspm_handle;

	/* Entry to VSPM for loop times (by color-format) */
	for (e_cnt = 0; e_cnt < loop_num; e_cnt++) {
		ercd = imgpctrl_tdd_vspm_entry(
				ctrl_hdl,
				lay,
				e_cnt,
				exe_info);
		if (ercd != IMGPCTRL_R_OK) {
			/* Err VSPM Entry */
			debug_data = __LINE__;
			break;
		}
	}

	/* Debug information */
	imgpctrl_cmn_dbg_ctrl_hdl_param(ct_hdl);

	/* output trace log */
	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_NRM, 0,
			(u_long *)NULL , ghp);
	} else {
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY,
			(u_short)debug_data, ercd , VSPM_ENTRY_NG, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_TDD, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
	}

	return ercd;
}

/**********************************************************************/
/* Internal function                                                  */
/**********************************************************************/
/**********************************************************************/
/* Function     : imgpctrl_tdd_vspm_entry                             */
/* Description  : Generate parameter for 2D-DMAC.                     */
/*                Request of Entry to the VSPM.                       */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/*                 - IMGPCTRL_R_VSPM_ENTRY_NG                         */
/* Argument     : ctrl_hdl - IMGPCTRL control handle address          */
/*                lay - layer number                                  */
/*                e_cnt - entry exe count (RGB=0,Y=0,CbCr=1)          */
/*                *exe_info - IMGPCTRL ctrl exe data pointer          */
/**********************************************************************/
static long imgpctrl_tdd_vspm_entry(u_long ctrl_hdl, u_short lay,
					u_short e_cnt,
					struct t_exe_info *exe_info)
{
	long ercd;
	long q_err;
	u_long jobid = IMGPCTRL_INVALID_JOBID;
	u_long debug_data = 0;

	struct t_ctrl_hdl *ct_hdl;
	struct t_ctrl_hdl_tdd *ctrl_hdl_tdd;
	VSPM_IP_PAR *ip_param;
	VSPM_2DDMAC_PAR *vspm_tdd_par;
	T_TDDMAC_MODE *m2d_par;
	T_TDDMAC_REQUEST *req_par;

	struct screen_grap_handle *ghp;
#if (RT_GRAPHICS_TRACE_ID > 1)
	u_long trace_data[5];
#endif

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);

	/* Set imgpctrl ctrl handle */
	ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;

	ghp = (struct screen_grap_handle *)ct_hdl->udata;
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, FUNC_IN, 0,
		(u_long *)NULL , ghp);

	/* Get imgpctrl ctrl tdd handle */
	ctrl_hdl_tdd = (struct t_ctrl_hdl_tdd *)ct_hdl->vspm_param;

	/* Set pointer for VSPM parameter */
	m2d_par = (T_TDDMAC_MODE *)
		(&(ctrl_hdl_tdd->vspm_tdd[e_cnt].p_tddmac_mode));
	req_par = (T_TDDMAC_REQUEST *)
		(&(ctrl_hdl_tdd->vspm_tdd[e_cnt].p_tddmac_request));

	vspm_tdd_par = (VSPM_2DDMAC_PAR *)
		(&(ctrl_hdl_tdd->vspm_tdd[e_cnt].pt2dDmac));
	vspm_tdd_par->ptTdDmacMode = m2d_par;
	vspm_tdd_par->ptTdDmacRequest = req_par;

	/* Set VSPM ip select parameter */
	ip_param = (VSPM_IP_PAR *)(&(ctrl_hdl_tdd->vspm_tdd[e_cnt].vspm_ip));
	ip_param->uhType = VSPM_TYPE_2DDMAC_AUTO;
	ip_param->unionIpParam.pt2dDmac = vspm_tdd_par;

	/* Set VSPM execute parameter */
	ercd = imgpctrl_tdd_set_param(lay, e_cnt, exe_info, m2d_par, req_par,
		ghp);

	if (ercd == IMGPCTRL_R_OK) {
		/* Entry to VSPM */
#if (RT_GRAPHICS_TRACE_ID > 1)
		trace_data[0] = lay;
		trace_data[1] = req_par->dst_width;
		trace_data[2] = req_par->dst_height;
		trace_data[3] = req_par->mirror;
		trace_data[4] = req_par->rotation;

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, VSPM_ENTRY_BC,
			5, &trace_data[0] , ghp);
#endif
		ercd = VSPM_lib_Entry(
				ct_hdl->vspm_hdl,
				&jobid,
				IMGPCTRL_JOB_PRIO,
				ip_param,
				ctrl_hdl,
				&imgpctrl_vspm_callback);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, VSPM_ENTRY_AC,
			0, (u_long *)NULL , ghp);
		ct_hdl->entry_ret[e_cnt] = ercd;
		ct_hdl->entry_jobid[e_cnt] = jobid;

		/* Debug information */
		imgpctrl_cmn_dbg_tdd_param(m2d_par, req_par);

		/* Check return value from VSPM */
		if (ercd != IMGPCTRL_R_OK) {
			MSG_ERROR("[%s] ERR: VSPM Entry.Line:%d ercd=%ld\n",
					__func__, __LINE__, ercd);
			debug_data = __LINE__;
			ercd = IMGPCTRL_R_VSPM_ENTRY_NG;
			/* output error log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY,
			(u_short)debug_data, ercd, VSPM_ENTRY_NG, ghp);
		} else {
			/* Count up Entry times */
			ct_hdl->entry_num++;
		}
	}

	if ((ercd != IMGPCTRL_R_OK) && (ct_hdl->entry_num == 0)) {
		/* Quit VSPM */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, VSPM_QUIT_BC,
			0, (u_long *)NULL , ghp);
		q_err = VSPM_lib_DriverQuit(ct_hdl->vspm_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, VSPM_QUIT_AC,
			0, (u_long *)NULL , ghp);
		if (q_err != R_VSPM_OK) {
			MSG_FATAL("[%s] ERR: VSPM Quit.Line:%d ercd=%ld\n",
						__func__, __LINE__, q_err);
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY,
			__LINE__ , q_err , VSPM_QUIT_ERR, ghp);
		}
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, FUNC_OUT_NRM, 0,
			(u_long *)NULL , ghp);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_VSPM_ENTRY, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
	}

	return ercd;
}
/**********************************************************************/
/* Function     : imgpctrl_tdd_set_param                              */
/* Description  : Generate parameter for 2D-DMAC.                     */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : lay - layer number                                  */
/*                e_cnt - entry exe count (RGB=0,Y=0,CbCr=1)          */
/*                *exe_info - IMGPCTRL ctrl exe data pointer          */
/*                *m2d_par - 2D-DMAC mode setting pointer             */
/*                *req_par - 2D-DMAC request setting pointer          */
/*		*bl_hdl -- log start pointer			      */
/**********************************************************************/
static long imgpctrl_tdd_set_param(u_short lay, u_short e_cnt,
					struct t_exe_info *exe_info,
					T_TDDMAC_MODE *m2d_par,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl)
{
	long ercd;
	struct screen_grap_layer *lyp_i;
	struct screen_grap_layer *lyp_o;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETPARAM, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);


	lyp_i = (struct screen_grap_layer *)(&(exe_info->input_layer[lay]));
	lyp_o = (struct screen_grap_layer *)(&(exe_info->output_layer[lay]));

	/* Set VSPM param (m2d_par) */
	m2d_par->renewal	= TDDMAC_RNEW_NORMAL;
	m2d_par->resource	= TDDMAC_RES_AUTO;
	m2d_par->p_extend	= NULL;

	/* Set VSPM param (req_par) */
	req_par->src_x_offset	= 0;
	req_par->src_y_offset	= 0;
	req_par->ratio		= TDDMAC_RATIO_1_1;
	req_par->alpha_ena	= TDDMAC_SRCALPHA_ENABLE;
	req_par->alpha		= 0xFF;
	req_par->dst_x_offset	= 0;
	req_par->dst_y_offset	= 0;
	req_par->cb_finished	= NULL;
	req_par->userdata	= NULL;
	req_par->dst_width	= lyp_o->image.width;
	req_par->dst_height	= lyp_o->image.height;

	/* Set color-format, address and stride  */
	if (e_cnt == 0) {
		ercd = imgpctrl_tdd_set_format(
			lyp_i->image.format, req_par, bl_hdl);
		if (ercd != IMGPCTRL_R_OK) {
			ercd = IMGPCTRL_R_PARA_ERR;
			debug_data = __LINE__;
			/* output error & trace log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETPARAM,
				(u_short)debug_data, ercd, UNVALID_FORMAT,
				bl_hdl);
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETPARAM,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data , bl_hdl);


			return ercd;
		}
		req_par->src_adr = lyp_i->image.address;
		req_par->src_stride = lyp_i->image.stride;
		req_par->dst_adr = lyp_o->image.address;
		req_par->dst_stride = lyp_o->image.stride;
	} else {
		ercd = imgpctrl_tdd_set_format_c(
			lyp_i->image.format, req_par, bl_hdl);
		if (ercd != IMGPCTRL_R_OK) {
			ercd = IMGPCTRL_R_PARA_ERR;
			debug_data = __LINE__;
			/* output error & trace log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETPARAM,
				(u_short)debug_data, ercd , UNVALID_FORMAT,
				bl_hdl);
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETPARAM,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data , bl_hdl);

			return ercd;
		}
		req_par->src_adr = lyp_i->image.address_c0;
		req_par->src_stride = lyp_i->image.stride_c;
		req_par->dst_adr = lyp_o->image.address_c0;
		req_par->dst_stride = lyp_o->image.stride_c;
	}

	/* Set rotate and mirror */
	ercd = imgpctrl_tdd_set_rot_mir(lyp_i->mirror,
				lyp_i->rotate,
				req_par, bl_hdl);
	if (ercd != IMGPCTRL_R_OK) {
		ercd = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETPARAM,
		(u_short)debug_data, ercd , OUT_OF_RANGE_ROT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETPARAM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return ercd;
	}

	/* Check parameter */
	if ((req_par->src_adr == NULL) || (req_par->dst_adr == NULL)) {
		MSG_ERROR("[%s] ERR: invalid address.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETPARAM,
		(u_short)debug_data, ercd , PARAM_NULL, bl_hdl);
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETPARAM, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETPARAM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ercd;
}
/**********************************************************************/
/* Function     : imgpctrl_tdd_get_layer_num                          */
/* Description  : Get layer number from rq_layer.                     */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : rq_layer - target layer(bit information:logical sum)*/
/*                *lay - layer number data pointer                    */
/*		*bl_hdl -- log start pointer			      */
/**********************************************************************/
static long imgpctrl_tdd_get_layer_num(u_short rq_layer, u_short *lay,
	struct screen_grap_handle *bl_hdl)
{
	long ercd = IMGPCTRL_R_OK;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_GETLAYERNUM, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	switch (rq_layer) {
	case IMGPCTRL_REQ_LAYER_0:
		*lay = 0;
		break;
	case IMGPCTRL_REQ_LAYER_1:
		*lay = 1;
		break;
	case IMGPCTRL_REQ_LAYER_2:
		*lay = 2;
		break;
	case IMGPCTRL_REQ_LAYER_3:
		*lay = 3;
		break;
	default:
		MSG_ERROR("[%s] ERR: invalid layer.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_GETLAYERNUM,
		(u_short)debug_data, ercd , INVALID_LAYER, bl_hdl);
		ercd = IMGPCTRL_R_PARA_ERR;
		break;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_GETLAYERNUM,
			FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_GETLAYERNUM,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ercd;
}
/**********************************************************************/
/* Function     : imgpctrl_tdd_get_loop_num                           */
/* Description  : Get loop times from image_format.                   */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : image_format - image color-format                   */
/*                *loop_num - loop times data pointer                 */
/*		*bl_hdl -- log start pointer			      */
/**********************************************************************/
static long imgpctrl_tdd_get_loop_num(u_int image_format, u_short *loop_num,
	struct screen_grap_handle *bl_hdl)
{
	long ercd = IMGPCTRL_R_OK;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_GETLOOPNUM, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	switch (image_format) {
	case RT_GRAPHICS_COLOR_RGB565:
	case RT_GRAPHICS_COLOR_RGB888:
	case RT_GRAPHICS_COLOR_ARGB8888:
	case RT_GRAPHICS_COLOR_XRGB8888:
	case RT_GRAPHICS_COLOR_ABGR8888:
	case RT_GRAPHICS_COLOR_XBGR8888:
		*loop_num = 1;
		break;
	case RT_GRAPHICS_COLOR_YUV422SP:
	case RT_GRAPHICS_COLOR_YUV420SP:
	case RT_GRAPHICS_COLOR_YUV420SP_NV21:
		*loop_num = 2;
		break;
	case RT_GRAPHICS_COLOR_YUV420PL:
	case RT_GRAPHICS_COLOR_YUV422I_UYVY:
			MSG_ERROR("[%s] ERR: unsupported format.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_GETLOOPNUM,
		(u_short)debug_data , ercd , UNVALID_FORMAT, bl_hdl);
		break;
	default:
		MSG_ERROR("[%s] ERR: invalid format.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_GETLOOPNUM,
		(u_short)debug_data , ercd , UNVALID_FORMAT, bl_hdl);
		break;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_GETLOOPNUM, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_GETLOOPNUM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ercd;
}
/**********************************************************************/
/* Function     : imgpctrl_tdd_set_format                             */
/* Description  : Set parameter of color format and swap value.       */
/*                (RGB format, Y plane of YCbCr format)               */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : image_format - image color-format                   */
/*                *req_par - 2D-DMAC request setting pointer          */
/*		*bl_hdl -- log start pointer			      */
/**********************************************************************/
static long imgpctrl_tdd_set_format(u_int image_format,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl)
{
	long ercd = IMGPCTRL_R_OK;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETFORMAT, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	switch (image_format) {
	case RT_GRAPHICS_COLOR_RGB565:
		req_par->src_format	= TDDMAC_FORMAT_RGB565;
		req_par->dst_format	= TDDMAC_FORMAT_RGB565;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_IWS |
					TDDMAC_SWAP_OLS |
					TDDMAC_SWAP_OWS);
		break;
	case RT_GRAPHICS_COLOR_RGB888:
		req_par->src_format	= TDDMAC_FORMAT_RGB888;
		req_par->dst_format	= TDDMAC_FORMAT_RGB888;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_IWS |
					TDDMAC_SWAP_IBS |
					TDDMAC_SWAP_OLS |
					TDDMAC_SWAP_OWS |
					TDDMAC_SWAP_OBS);
		break;
	case RT_GRAPHICS_COLOR_ARGB8888:
	case RT_GRAPHICS_COLOR_XRGB8888:
		req_par->src_format	= TDDMAC_FORMAT_ARGB8888;
		req_par->dst_format	= TDDMAC_FORMAT_ARGB8888;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_OLS);
		break;
	case RT_GRAPHICS_COLOR_ABGR8888:
	case RT_GRAPHICS_COLOR_XBGR8888:
		req_par->src_format	= TDDMAC_FORMAT_ABGR8888;
		req_par->dst_format	= TDDMAC_FORMAT_ABGR8888;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_OLS);
		break;
	case RT_GRAPHICS_COLOR_YUV422SP:
	case RT_GRAPHICS_COLOR_YUV420SP:
	case RT_GRAPHICS_COLOR_YUV420SP_NV21:
		req_par->src_format	= TDDMAC_FORMAT_Y;
		req_par->dst_format	= TDDMAC_FORMAT_Y;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_IWS |
					TDDMAC_SWAP_IBS |
					TDDMAC_SWAP_OLS |
					TDDMAC_SWAP_OWS |
					TDDMAC_SWAP_OBS);
		break;
	case RT_GRAPHICS_COLOR_YUV420PL:
	case RT_GRAPHICS_COLOR_YUV422I_UYVY:
		MSG_ERROR("[%s] ERR: unsupported format.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETFORMAT,
		(u_short)debug_data , ercd , UNVALID_FORMAT, bl_hdl);
		break;
	default:
		MSG_ERROR("[%s] ERR: invalid format.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETFORMAT,
		(u_short)debug_data , ercd , UNVALID_FORMAT, bl_hdl);
		break;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETFORMAT, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETFORMAT, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ercd;
}
/**********************************************************************/
/* Function     : imgpctrl_tdd_set_format_c                           */
/* Description  : Set parameter of color-format and swap value.       */
/*                (C plane of YCbCr format)                           */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : image_format - image color-format                   */
/*                *req_par - 2D-DMAC request setting pointer          */
/*		*bl_hdl -- log start pointer			      */
/**********************************************************************/
static long imgpctrl_tdd_set_format_c(u_int image_format,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl)
{
	long ercd = IMGPCTRL_R_OK;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETFORMATC, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	switch (image_format) {
	case RT_GRAPHICS_COLOR_YUV422SP:
		req_par->src_format	= TDDMAC_FORMAT_C422;
		req_par->dst_format	= TDDMAC_FORMAT_C422;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_IWS |
					TDDMAC_SWAP_IBS |
					TDDMAC_SWAP_OLS |
					TDDMAC_SWAP_OWS |
					TDDMAC_SWAP_OBS);
		break;
	case RT_GRAPHICS_COLOR_YUV420SP:
	case RT_GRAPHICS_COLOR_YUV420SP_NV21:
		req_par->src_format	= TDDMAC_FORMAT_C420;
		req_par->dst_format	= TDDMAC_FORMAT_C420;
		req_par->swap		= (TDDMAC_SWAP_ILS |
					TDDMAC_SWAP_IWS |
					TDDMAC_SWAP_IBS |
					TDDMAC_SWAP_OLS |
					TDDMAC_SWAP_OWS |
					TDDMAC_SWAP_OBS);
		break;
	case RT_GRAPHICS_COLOR_YUV420PL:
	case RT_GRAPHICS_COLOR_YUV422I_UYVY:
		MSG_ERROR("[%s] ERR: unsupported format.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETFORMATC,
		(u_short)debug_data , ercd , UNVALID_FORMAT, bl_hdl);
		break;
	default:
		MSG_ERROR("[%s] ERR: invalid format.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETFORMATC,
		(u_short)debug_data, ercd , UNVALID_FORMAT, bl_hdl);
		break;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETFORMATC, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETFORMATC, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ercd;
}
/**********************************************************************/
/* Function     : imgpctrl_tdd_set_rot_mir                            */
/* Description  : Set parameter of mirror and rotation value.         */
/* Return value : ercd                                                */
/*                 - IMGPCTRL_R_OK                                    */
/*                 - IMGPCTRL_R_PARA_ERR                              */
/* Argument     : mirror - mirror value                               */
/*                rotation - rotation value                           */
/*                *req_par - 2D-DMAC request setting pointer          */
/*		*bl_hdl -- log start pointer			      */
/**********************************************************************/
static long imgpctrl_tdd_set_rot_mir(u_short mirror, u_short rotation,
					T_TDDMAC_REQUEST *req_par,
					struct screen_grap_handle *bl_hdl)
{
	long ercd = IMGPCTRL_R_OK;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETROTMIR, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if ((mirror & RT_GRAPHICS_MIRROR_N) != 0) {
		/* No-mirror bit is top priority */
		req_par->mirror = TDDMAC_MRR_OFF;
	} else if (((mirror & RT_GRAPHICS_MIRROR_V) != 0)
		 && ((mirror & RT_GRAPHICS_MIRROR_H) != 0)) {
		/* Left<=>Right and Up<=>Down */
		req_par->mirror = TDDMAC_MRR_HV;
	} else if ((mirror & RT_GRAPHICS_MIRROR_V) != 0) {
		/* Up<=>Down */
		req_par->mirror = TDDMAC_MRR_V;
	} else if ((mirror & RT_GRAPHICS_MIRROR_H) != 0) {
		/* Left<=>Right */
		req_par->mirror = TDDMAC_MRR_H;
	} else {
		MSG_ERROR("[%s] ERR: invalid mirror pattern.Line:%d\n",
			__func__, __LINE__);
		req_par->mirror = TDDMAC_MRR_OFF;
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETROTMIR,
		(u_short)debug_data , ercd , OUT_OF_RANGE_MIR, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETROTMIR,
		FUNC_OUT_ERR, 1, (u_long *)&debug_data , bl_hdl);
		return ercd;
	}

	switch (rotation) {
	case RT_GRAPHICS_ROTATE_0:
		req_par->rotation = TDDMAC_ROT_OFF;
		break;
	case RT_GRAPHICS_ROTATE_90:
		req_par->rotation = TDDMAC_ROT_90;
		break;
	case RT_GRAPHICS_ROTATE_180:
		req_par->rotation = TDDMAC_ROT_180;
		break;
	case RT_GRAPHICS_ROTATE_270:
		req_par->rotation = TDDMAC_ROT_270;
		break;
	default:
		MSG_ERROR("[%s] ERR: invalid rotate pattern.Line:%d\n",
			__func__, __LINE__);
		req_par->rotation = TDDMAC_ROT_OFF;
		debug_data = __LINE__;
		ercd = IMGPCTRL_R_PARA_ERR;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_TDD_SETROTMIR,
		(u_short)debug_data , ercd , OUT_OF_RANGE_ROT, bl_hdl);
		break;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ercd == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETROTMIR, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_TDD_SETROTMIR, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ercd;
}
