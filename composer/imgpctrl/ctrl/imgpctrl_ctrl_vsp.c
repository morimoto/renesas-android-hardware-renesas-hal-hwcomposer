/*************************************************************************/ /*
 imgpctrl_ctrl_vsp.c
   imgpctrl ctrl_vsp main function file

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
#include "imgpctrl_ctrl_vsp.h"



/* imgpctrl_ctrl_exe_vsp -					*/
/* return value : stat	R_IMGPCTRL_OK				*/
/*			R_IMGPCTRL_NG				*/
/*			R_IMGPCTRL_R_PARA_ERR			*/
/*			R_IMGPCTRL_R_SEQ_ERR			*/
/*                      R_IMGPCTRL_R_VSPM_INIT_NG		*/
/*                      R_IMGPCTRL_R_VSPM_ENTRY_NG		*/
/* argument : ctrl_hdl  -- IMGPCTRL control handle information  */
/*            exe_info --  control execute information		*/
/* Description: VSPM setting function				*/

long imgpctrl_ctrl_exe_vsp(u_long ctrl_hdl, struct t_exe_info *exe_info)
{
	u_long vspm_handle = 0;
	long err;
	long q_err;
	long ret = 0;
	u_long  jobid;
	struct t_ctrl_hdl *ct_hdl;
	struct t_ctrl_hdl_vsp *ct_vsp;
	VSPM_IP_PAR *vspm_ip;
	VSPM_VSP_PAR *p_vsp;
	struct t_vspm_vsp	*vspm_vsp;
	struct screen_grap_handle *ghp;
	u_long debug_data = 0;
#if (RT_GRAPHICS_TRACE_ID > 1)
	u_long trace_data[4];
#endif

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	if (ctrl_hdl == ((u_long)NULL)) {
		ret = IMGPCTRL_R_SEQ_ERR;
		MSG_ERROR("[%s] ERR: vspm ctrl_hdl is null.Line:%d\n",
			__func__, __LINE__);
		return ret;
	}

	ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;
	ct_vsp = (struct t_ctrl_hdl_vsp *)ct_hdl->vspm_param;

	ghp = ((struct screen_grap_handle *)(ct_hdl->udata));

	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_IN, 0,
		(u_long *)NULL , ghp);

	if (ct_vsp == ((struct t_ctrl_hdl_vsp *)NULL)) {
		ret = IMGPCTRL_R_SEQ_ERR;
		MSG_ERROR("[%s] ERR: vspm parameter is null.Line:%d\n",
			 __func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
		(u_short)debug_data ,
		ret , HDL_PTR_NULL, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ret;
	}
	if (exe_info == ((struct t_exe_info *)NULL)) {
		ret = IMGPCTRL_R_SEQ_ERR;
		MSG_ERROR("[%s] ERR: exe info parameter is null.Line:%d\n",
			 __func__, __LINE__);
		debug_data = __LINE__;
		/* output trace & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
		(u_short)debug_data ,
		ret , PARAM_NULL, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ret;
	}

	vspm_ip = (VSPM_IP_PAR *)(&(ct_vsp->vspm_vsp.vspm_ip));
	p_vsp = (VSPM_VSP_PAR *)(&(ct_vsp->vspm_vsp.p_vsp));
	vspm_vsp = (struct t_vspm_vsp *)(&(ct_vsp->vspm_vsp));

	vspm_ip->unionIpParam.ptVsp = p_vsp;

	p_vsp->src1_par = &(ct_vsp->vspm_vsp.src_par[0]);
	p_vsp->src2_par = &(ct_vsp->vspm_vsp.src_par[1]);
	p_vsp->src3_par = &(ct_vsp->vspm_vsp.src_par[2]);
	p_vsp->src4_par = &(ct_vsp->vspm_vsp.src_par[3]);
	p_vsp->dst_par  = &(ct_vsp->vspm_vsp.dst_par);
	p_vsp->ctrl_par  = &(ct_vsp->vspm_vsp.ctrl_par);
	p_vsp->src1_par->alpha_blend = &(ct_vsp->vspm_vsp.src_alpha[0]);
	p_vsp->src2_par->alpha_blend = &(ct_vsp->vspm_vsp.src_alpha[1]);
	p_vsp->src3_par->alpha_blend = &(ct_vsp->vspm_vsp.src_alpha[2]);
	p_vsp->src4_par->alpha_blend = &(ct_vsp->vspm_vsp.src_alpha[3]);
	p_vsp->ctrl_par->uds = &(ct_vsp->vspm_vsp.uds_par[0]);
	p_vsp->ctrl_par->uds1 = &(ct_vsp->vspm_vsp.uds_par[1]);
	p_vsp->ctrl_par->uds2 = &(ct_vsp->vspm_vsp.uds_par[2]);

	p_vsp->ctrl_par->bru = &(ct_vsp->vspm_vsp.bru_par);

	p_vsp->ctrl_par->bru->blend_virtual =
		 &(ct_vsp->vspm_vsp.bru_vir_par);

	p_vsp->ctrl_par->bru->blend_control_a =
		 &(ct_vsp->vspm_vsp.bru_ctrl_par[0]);
	p_vsp->ctrl_par->bru->blend_control_b =
		 &(ct_vsp->vspm_vsp.bru_ctrl_par[1]);
	p_vsp->ctrl_par->bru->blend_control_c =
		 &(ct_vsp->vspm_vsp.bru_ctrl_par[2]);
	p_vsp->ctrl_par->bru->blend_control_d =
		 &(ct_vsp->vspm_vsp.bru_ctrl_par[3]);
	p_vsp->ctrl_par->bru->blend_rop =  &(ct_vsp->vspm_vsp.bru_rop);


	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_INIT_BC, 0,
		(u_long *)NULL , ghp);

	err = VSPM_lib_DriverInitialize(&vspm_handle);

	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_INIT_AC, 0,
		(u_long *)NULL , ghp);

	if (err == R_VSPM_NG) {
		MSG_ERROR(
		 "[%s] ERR:VSPM_lib_Driver_Initialize is NG.Line:%d\n"
		 , __func__, __LINE__);
		debug_data = __LINE__;
		ret = IMGPCTRL_R_VSPM_INIT_NG;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
		(u_short)debug_data ,
		ret , VSPM_INIT_NG, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		/* Notify Error  */
		return ret;
	} else {
		/* Save VSPM handle */
		ct_hdl->vspm_hdl = vspm_handle;
	}


	/* VSP parameter setting */
	err = imgpctrl_vsp_para_gen(exe_info, vspm_vsp,
		ghp);


	if (err != IMGPCTRL_R_OK) {
		MSG_ERROR(
		"[%s] ERR: imgpctrl_para_gen is NG.Line:%d ercd:%ld\n",
		 __func__, __LINE__, err);
		/* Notify Error*/
		 ret = err;
		debug_data = __LINE__;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
		(u_short)debug_data ,
		ret , EXE_VSPTDD_ERR, ghp);

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_QUIT_BC, 0,
		(u_long *)NULL , ghp);

		q_err = VSPM_lib_DriverQuit(ct_hdl->vspm_hdl);

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_QUIT_AC, 0,
		(u_long *)NULL , ghp);

		if (q_err != R_VSPM_OK) {
			MSG_FATAL("[%s] ERR: VSPM Quit.Line:%d ercd=%ld\n",
					__func__, __LINE__, q_err);
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
			__LINE__, ret , VSPM_QUIT_ERR, ghp);

		}
		/* output trace log */
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ret;
	}

#if (RT_GRAPHICS_TRACE_ID > 1)
	trace_data[0] = vspm_ip->unionIpParam.ptVsp->rpf_num;
	trace_data[1] = vspm_ip->unionIpParam.ptVsp->use_module;
	trace_data[2] = vspm_ip->unionIpParam.ptVsp->dst_par->width;
	trace_data[3] = vspm_ip->unionIpParam.ptVsp->dst_par->height;


	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_ENTRY_BC, 4,
	&trace_data[0] , ghp);
#endif

	err = VSPM_lib_Entry(
		ct_hdl->vspm_hdl,
		&jobid,
		IMGPCTRL_JOB_PRIO,
		vspm_ip,
		ctrl_hdl,
		&imgpctrl_vspm_callback);

	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_ENTRY_AC, 0,
	(u_long *)NULL , ghp);

	if (err != IMGPCTRL_R_OK) {
		/* Reserve Result */
		ct_hdl->entry_jobid[0] = jobid;
		ct_hdl->entry_ret[0] = err;

		/* log output */
		MSG_ERROR("[%s] ERR: VSPM_lib_Entry is NG.Line:%d,ercd=%ld\n",
			 __func__, __LINE__, err);
		debug_data = __LINE__;
		/* Notify Error */
		ret = IMGPCTRL_R_VSPM_ENTRY_NG;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
		(u_short)debug_data, ret , VSPM_QUIT_ERR, ghp);

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_QUIT_BC, 0,
		(u_long *)NULL , ghp);

		q_err = VSPM_lib_DriverQuit(ct_hdl->vspm_hdl);

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, VSPM_QUIT_AC, 0,
		(u_long *)NULL , ghp);
		if (q_err != R_VSPM_OK) {
			MSG_FATAL("[%s] ERR: VSPM Quit.Line:%d ercd=%ld\n",
					__func__, __LINE__, q_err);
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_VSP,
			__LINE__, ret , VSPM_QUIT_ERR, ghp);
		}

	} else {
		/* Reversed JobID */
		/* entry_num++ */
		ct_hdl->entry_ret[0] = err;
		ct_hdl->entry_jobid[0] = jobid;
		ct_hdl->entry_num++;
	}

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);

	if (ret == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_OUT_NRM, 0,
			(u_long *)NULL , ghp);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_VSP, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
	}

	return ret;
}


