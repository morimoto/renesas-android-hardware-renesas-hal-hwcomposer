/*************************************************************************/ /*
 imgpctrl_ctrl_vsp_para_gen.c
   imgpctrl ctrl_vsp parameter generate file

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

/****************************************************************/
/* Internal function						*/
/****************************************************************/

static long imgpctrl_vsp_blend_0(struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_blend(struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p, u_char req_lay,
	u_short uds_num, struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_zoom(struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p, struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_conversion(struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p, struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_format_get(u_int informat,
	u_short *outformat, u_char *swap, u_char *cspace,
	struct screen_grap_handle *bl_hdl);
static void imgpctrl_vsp_in_csc_set(u_char inputformat, u_char outputformat,
	u_char  *middleformat,
	const struct screen_grap_image_param *in_image,
	const struct screen_grap_image_param *out_image,
	T_VSP_IN *src_ptr, u_short rq_type,
	struct screen_grap_handle *bl_hdl);
static void imgpctrl_vsp_t_vsp_in_set(
	const struct screen_grap_layer *in_layer,
	const struct t_ext_layer *ext_input,
	u_char informat,
	u_short vsp_in_format,
	T_VSP_IN *src_ptr,
	u_short rq_type,
	u_char swap,
	struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_in_alpha_set(
	const struct screen_grap_layer *in_layer,
	T_VSP_IN *src_ptr,
	struct screen_grap_handle *bl_hdl);
static void imgpctrl_vsp_t_vsp_out_set(
	u_char informat,  u_short vsp_out_format,
	const struct screen_grap_image_param *out_image,
	u_char outformat,
	T_VSP_OUT *dst_ptr,
	u_char swap,
	struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_blend_check_param(
	struct t_exe_info *exe_info,
	u_char *req_lay, u_short *uds_num, struct screen_grap_handle *bl_hdl);
static void imgpctrl_vsp_t_vsp_bru_set(
	struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p, u_char req_lay,
	struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_t_uds_set(T_VSP_UDS *uds_param,
	const struct screen_grap_layer *layer,
	struct screen_grap_handle *bl_hdl);
static long imgpctrl_vsp_uds_get_scaling_ratio(
	u_short ush_in_size,
	u_short ush_out_size,
	u_short *ush_ratio,
	struct screen_grap_handle *bl_hdl);
static u_long imgpctrl_vsp_uds_scale_down_process(
	u_short ush_in_size,
	u_short ush_out_size,
	struct screen_grap_handle *bl_hdl);

/* imgpctrl_vsp_para_gen						*/
/* return value : status IMGPCTRL_R_OK					*/
/*                       IMGPCTRL_R_PARA_ERR				*/
/* argument     :   *exe_info -- IMGPCTRL control execute information	*/
/*		*vspm_p  -- VSPM parameter address			*/
/*		*bl_hdl -- log start pointer				*/
/* Description: blend/zoom/convirsion call				*/

long imgpctrl_vsp_para_gen(
	struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	struct screen_grap_handle *bl_hdl)
{
	long err;
	u_char req_lay = 0;
	u_short use_num = 0;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_PARA_GEN, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if ((exe_info->hw_type & IMGPCTRL_TYPE_VSP) != 0) {
		/* HW connect type = VSP? */
		if ((exe_info->rq_type & IMGPCTRL_REQ_ROTATE) ==
		 IMGPCTRL_REQ_ROTATE) {
			/* Is valid the  rotate/mirror bit? */
			err = IMGPCTRL_R_PARA_ERR;
			MSG_ERROR("[%s] ERR: para err.Line:%d ercd: %ld\n",
				 __func__, __LINE__, err);
			debug_data = __LINE__;
			/* output error & trace log */
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_VSP_PARA_GEN,
			(u_short)debug_data, err , INVALID_REQUEST, bl_hdl);

			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_PARA_GEN,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data , bl_hdl);
			return err;
		} else if ((exe_info->rq_type & IMGPCTRL_REQ_BLEND) ==
		 IMGPCTRL_REQ_BLEND) {
			/* Is valid the blend bit ? */
			/* Parameter check <- only blend */
			err = imgpctrl_vsp_blend_check_param(exe_info,
				 &req_lay, &use_num, bl_hdl);
			if (err == IMGPCTRL_R_OK) {
				if (req_lay == 0) {
					/* blend function call */
					err = imgpctrl_vsp_blend_0(
					exe_info, vspm_p,
					bl_hdl);
				} else {
					/* blend function call */
					err = imgpctrl_vsp_blend(
					exe_info, vspm_p,
					req_lay, use_num, bl_hdl);
				}
			} else
				debug_data = __LINE__;
		} else if (((exe_info->rq_type) & IMGPCTRL_REQ_RESIZE) ==
			IMGPCTRL_REQ_RESIZE) {
			/*Is valid the resize bit? */
			/* resize function call */
			err = imgpctrl_vsp_zoom(exe_info, vspm_p, bl_hdl);
			if (err != IMGPCTRL_R_OK) {
				debug_data = __LINE__;

				MSG_ERROR("[%s] err: %d Line:%d\n",
				 __func__, (u_int) err, __LINE__);
			}

			/* The number of layers =1 */
			req_lay = 1;

		} else if ((((exe_info->rq_type) & IMGPCTRL_REQ_COLOR_CONVERT)
			== IMGPCTRL_REQ_COLOR_CONVERT) ||
			(((exe_info->rq_type) & IMGPCTRL_REQ_TRIMMING) ==
					IMGPCTRL_REQ_TRIMMING)) {
			/*Is the valid the trimming/color convert bit */
			/* color convert function call */
			err = imgpctrl_vsp_conversion(exe_info, vspm_p, bl_hdl);
			if (err != IMGPCTRL_R_OK)
				debug_data = __LINE__;

			/* The number of layers =1 */
			req_lay = 1;

		} else {
			err = IMGPCTRL_R_PARA_ERR;
			MSG_ERROR("[%s] ERR: para err.Line:%d ercd: %ld\n",
				 __func__, __LINE__, err);
			debug_data = __LINE__;
			IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_VSP_PARA_GEN,
			(u_short)debug_data, err , INVALID_REQUEST, bl_hdl);
		}
	} else {
		/* HW connect type = TDD(Error) */
		err = IMGPCTRL_R_PARA_ERR;
		MSG_ERROR("[%s] ERR: para err.Line:%d ercd: %ld\n",
			 __func__, __LINE__, err);
		debug_data = __LINE__;
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_VSP_PARA_GEN,
		(u_short)debug_data, err , INVALID_REQUEST, bl_hdl);
	}

	/* Debug Output */
	imgpctrl_cmn_dbg_vsp_param(
		vspm_p,
		req_lay,
		use_num,
		exe_info->rq_type);

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (err == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_PARA_GEN, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_PARA_GEN, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return err;
}

/* imgpctrl_ctrl_vsp_blend_check_param					*/
/* return value : status IMGPCTRL_R_OK					*/
/*                               IMGPCTRL_R_PARA_ERR			*/
/* argument     :   *exe_info -- IMGPCTRL control execute information	*/
/*		*req_lay  -- using layer numbers			*/
/*		*uds_num  -- using uds number				*/
/*		*bl_hdl -- log start pointer				*/
/* Description: Unnecessary parameter is 0 and Blend parameter check	*/
static long imgpctrl_vsp_blend_check_param(
	struct t_exe_info *exe_info,
	 u_char *req_lay,
	 u_short *uds_num,
	struct screen_grap_handle *bl_hdl)
{

	long  err = IMGPCTRL_R_OK;

	int i;
	struct screen_grap_layer *in_layer;
	u_long debug_data;

	*req_lay = 0;
	*uds_num = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM, FUNC_IN, 0,
		(u_long *)NULL, bl_hdl);

	/* use layer check */
	if (exe_info->rq_layer == 0) {
		*req_lay = 0;
	} else if (exe_info->rq_layer == IMGPCTRL_REQ_LAYER_0) {
		*req_lay = 1;
	} else if (exe_info->rq_layer ==
		(IMGPCTRL_REQ_LAYER_0 | IMGPCTRL_REQ_LAYER_1)) {
		*req_lay = 2;
	} else if (exe_info->rq_layer ==
		(IMGPCTRL_REQ_LAYER_0 | IMGPCTRL_REQ_LAYER_1
		| IMGPCTRL_REQ_LAYER_2)) {
		*req_lay = 3;
	} else if (exe_info->rq_layer ==
		(IMGPCTRL_REQ_LAYER_0 | IMGPCTRL_REQ_LAYER_1
		| IMGPCTRL_REQ_LAYER_2 | IMGPCTRL_REQ_LAYER_3)) {
		*req_lay = 4;
	} else {
		err = IMGPCTRL_R_PARA_ERR;
		MSG_ERROR("[%s]ERR: imgpctrl_brend.Line:%d err: %ld\n",
		 __func__, __LINE__, err);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM,
			(u_short)debug_data, err , LAY_OVER, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		 return err;
	}




	for (i = 0; i < *req_lay; i++) {
		in_layer =
		 ((struct screen_grap_layer *)
			 (&(exe_info->input_layer[i])));

		if (((exe_info->rq_type) & IMGPCTRL_REQ_RESIZE) ==
		IMGPCTRL_REQ_RESIZE) {
			if ((in_layer->rect.width != 0) ||
			 (in_layer->rect.height != 0)) {
				/* resize valid */
				(*uds_num)++;

				if (*uds_num > IMGPCTRL_MAX_UDS) {
					err = IMGPCTRL_R_PARA_ERR;
					MSG_ERROR
					("[%s] ERR:uds.Line:%d uds_num:%d\n",
					 __func__, __LINE__,
					 (u_int)*uds_num);
					debug_data = __LINE__;
					/* output error & trace log */
					IMGPCTRL_ERROR_LOG(
					FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM,
					(u_short)debug_data,
					err , UDS_ERR, bl_hdl);
					IMGPCTRL_TRACE_LOG(
					FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM,
					FUNC_OUT_ERR, 1,
					(u_long *)&debug_data , bl_hdl);
					 return err;
				}
			}
		}

	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return err;
}

/* imgpctrl_vsp_blend_0							*/
/* return value : status	IMGPCTRL_R_OK				*/
/*			IMGPCTRL_R_PARA_ERR				*/
/* argument     :   *exe_info -- IMGPCTRL control execute information	*/
/*		*vspm_p    -- VSPM parameter address			*/
/*		*bl_hdl -- log start pointer				*/
/* Description: layer 0 only Blend(Background color)			*/
/*		 setting check and vspm setting called function		*/
static long imgpctrl_vsp_blend_0(
	struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	struct screen_grap_handle *bl_hdl)
{

	long  err = IMGPCTRL_R_OK;

	u_int checkformat;
	long  infmt;
	long  outfmt;
	struct screen_grap_image_param *out_image;
	T_VSP_OUT *dst_ptr;
	VSPM_VSP_PAR *p_vsp;
	u_short  vsp_in_format;
	u_short  vsp_out_format;
	u_char  in_swap;
	u_char  out_swap;
	u_char  in_c_space;
	u_char  out_c_space;
	u_long debug_data;


	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND0, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	p_vsp = &(vspm_p->p_vsp);

	/* param initialize */
	p_vsp->ctrl_par->sru = (T_VSP_SRU *)NULL;
	p_vsp->ctrl_par->lut = (T_VSP_LUT *)NULL;
	p_vsp->ctrl_par->clu = (T_VSP_CLU *)NULL;
	p_vsp->ctrl_par->hst = (T_VSP_HST *)NULL;
	p_vsp->ctrl_par->hsi = (T_VSP_HSI *)NULL;
	p_vsp->ctrl_par->hgo = (T_VSP_HGO *)NULL;
	p_vsp->ctrl_par->hgt = (T_VSP_HGT *)NULL;

	/* IP type: VSP */
	vspm_p->vspm_ip.uhType = exe_info->hw_type;

	/* rpf_order: 0(unused) */
	p_vsp->rpf_order = 0;

	/* RPF num: The request numbers of layer */
	p_vsp->rpf_num = 0;

	/* use module:blend */
	p_vsp->use_module = VSP_BRU_USE;

	/* unused src setting NULL set */
	p_vsp->src1_par = (T_VSP_IN *)NULL;
	p_vsp->src2_par = (T_VSP_IN *)NULL;
	p_vsp->src3_par = (T_VSP_IN *)NULL;
	p_vsp->src4_par = (T_VSP_IN *)NULL;

	/* unused uds setting NULL set */
	p_vsp->ctrl_par->uds = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->uds1 = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->uds2 = (T_VSP_UDS *)NULL;

	/* the number of layer is 0.(the blend only background) */
	out_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->output_layer[0].image));

	/* Input format check */
	checkformat = RT_GRAPHICS_COLOR_ARGB8888;
	infmt = imgpctrl_vsp_format_get(checkformat,
	 &vsp_in_format, &in_swap, &in_c_space, bl_hdl);

	/* output format check */
	checkformat = out_image->format;
	outfmt = imgpctrl_vsp_format_get(checkformat,
	 &vsp_out_format, &out_swap, &out_c_space, bl_hdl);

	if ((infmt == IMGPCTRL_R_PARA_ERR) ||
		(outfmt == IMGPCTRL_R_PARA_ERR)) {
		err = IMGPCTRL_R_PARA_ERR;
		MSG_ERROR
		("ERR:[%s] unvalid format.Line:%d\n",
		  __func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_BLEND0,
			(u_short)debug_data, err , UNVALID_FORMAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND0,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);

		return err;
	}

	/* The image format input pointer set */
	dst_ptr = p_vsp->dst_par;


	/* Background only setting */
	imgpctrl_vsp_t_vsp_bru_set(exe_info, vspm_p, 0, bl_hdl);

	/* connect HW: BRU->WPF */
	vspm_p->bru_par.connect = 0;

	/*  VSP_OUT set */
	imgpctrl_vsp_t_vsp_out_set(
		 in_c_space,
		 vsp_out_format,
		 out_image,
		 out_c_space,
		 dst_ptr,
		 out_swap,
		 bl_hdl);


	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND0, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return err;
}

/* imgpctrl_vsp_blend							*/
/* return value : status	IMGPCTRL_R_OK				*/
/*			IMGPCTRL_R_PARA_ERR				*/
/* argument     :   *exe_info -- IMGPCTRL control execute information	*/
/*		*vspm_p    -- VSPM parameter address			*/
/*		req_lay     -- request layer numbers			*/
/*		uds_num  -- request uds numbers				*/
/*		*bl_hdl -- log start pointer				*/
/* Description: Blend setting check and vspm setting called function	*/
static long imgpctrl_vsp_blend(
	struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	u_char req_lay,
	u_short uds_num,
	struct screen_grap_handle *bl_hdl)
{

	long  err = IMGPCTRL_R_OK;

	int i;		/* blend loop */
	int j = 0;		/* uds loop */
	u_int checkformat;
	u_char middleformat = 0;
	long  infmt;
	long  outfmt;
	struct screen_grap_image_param *in_image;
	struct screen_grap_image_param *out_image;
	struct t_ext_layer *ext_input;
	T_VSP_IN *src_ptr;
	T_VSP_OUT *dst_ptr;
	struct screen_grap_layer *in_layer;
	VSPM_VSP_PAR *p_vsp;
	u_short  vsp_in_format;
	u_short  vsp_out_format;
	T_VSP_UDS *uds_ptr;
	u_char  in_swap;
	u_char  out_swap;
	u_short rq_type;
	u_char  in_c_space = 0;
	u_char  out_c_space;
	u_long debug_data;



	rq_type = exe_info->rq_type;
	dst_ptr = (T_VSP_OUT *)NULL;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);


	p_vsp = &(vspm_p->p_vsp);

	/* param initialize */
	p_vsp->ctrl_par->sru = (T_VSP_SRU *)NULL;
	p_vsp->ctrl_par->lut = (T_VSP_LUT *)NULL;
	p_vsp->ctrl_par->clu = (T_VSP_CLU *)NULL;
	p_vsp->ctrl_par->hst = (T_VSP_HST *)NULL;
	p_vsp->ctrl_par->hsi = (T_VSP_HSI *)NULL;
	p_vsp->ctrl_par->hgo = (T_VSP_HGO *)NULL;
	p_vsp->ctrl_par->hgt = (T_VSP_HGT *)NULL;


	/* IP type: VSP */
	vspm_p->vspm_ip.uhType = exe_info->hw_type;

	/* rpf_order: 0(unused) */
	p_vsp->rpf_order = 0;

	/* RPF num: The request numbers of layer */
	p_vsp->rpf_num = req_lay;


	/* unused src setting NULL set */
	p_vsp->src1_par = (T_VSP_IN *)NULL;
	p_vsp->src2_par = (T_VSP_IN *)NULL;
	p_vsp->src3_par = (T_VSP_IN *)NULL;
	p_vsp->src4_par = (T_VSP_IN *)NULL;

	/* the number of blend >= 1 */
	if (uds_num == 0) {
		/* use module: blend(not resize) */
		p_vsp->use_module = VSP_BRU_USE;
		p_vsp->ctrl_par->uds = (T_VSP_UDS *)NULL;
		p_vsp->ctrl_par->uds1 = (T_VSP_UDS *)NULL;
		p_vsp->ctrl_par->uds2 = (T_VSP_UDS *)NULL;
	} else if (uds_num == 1) {
		/* use module:reseize(1 layer)|blend */
		p_vsp->use_module = VSP_UDS_USE | VSP_BRU_USE;
		p_vsp->ctrl_par->uds1 = (T_VSP_UDS *)NULL;
		p_vsp->ctrl_par->uds2 = (T_VSP_UDS *)NULL;
	} else if (uds_num == 2) {
		/* use module : resize(2 layer)| blend */
		p_vsp->use_module = VSP_UDS_USE |
			 VSP_UDS1_USE | VSP_BRU_USE;
		p_vsp->ctrl_par->uds2 = (T_VSP_UDS *)NULL;
	} else {
		/* use module : resize(3 layer)| blend*/
		p_vsp->use_module = VSP_UDS_USE | VSP_UDS1_USE |
			 VSP_UDS2_USE | VSP_BRU_USE;
	}

	/* Set Output */
	out_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->output_layer[0].image));

	/* output format check */
	checkformat = out_image->format;
	outfmt = imgpctrl_vsp_format_get(
	 checkformat,
	 &vsp_out_format,
	 &out_swap,
	 &out_c_space,
	 bl_hdl);

	/* the unvalid format */
	if (outfmt == IMGPCTRL_R_PARA_ERR) {
		err = IMGPCTRL_R_PARA_ERR;
		MSG_ERROR
		("[%s]ERR: unvalid format.Line:%d\n",
		  __func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_BLEND,
			(u_short)debug_data, err , UNVALID_FORMAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	/* loop of the number of blend */
	for (i = 0; i < req_lay; i++) {
		/* Get layer of input */
		in_layer =
		 (struct screen_grap_layer *)
		 (&(exe_info->input_layer[i]));
		in_image =
		 (struct screen_grap_image_param *)
		 (&(exe_info->input_layer[i].image));
		ext_input =
		(struct t_ext_layer *)(&(exe_info->ext_input[i]));

		/* Input format check */
		checkformat = in_image->format;
		infmt = imgpctrl_vsp_format_get(
		 checkformat,
		 &vsp_in_format,
		 &in_swap,
		 &in_c_space,
		 bl_hdl);

		/* the unvalid format */
		if (infmt == IMGPCTRL_R_PARA_ERR) {
			err = IMGPCTRL_R_PARA_ERR;
			MSG_ERROR
			("[%s] ERR:unvalid forma.Line:%d\n",
			  __func__, __LINE__);
			debug_data = __LINE__;
			/* output error & trace log */
			IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_BLEND,
			(u_short)debug_data, err , UNVALID_FORMAT, bl_hdl);
			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data , bl_hdl);

			return err;
		}

		/* The image format input pointer set */
		src_ptr = &(vspm_p->src_par[i]);
		dst_ptr = p_vsp->dst_par;

		/* CSC set */
		imgpctrl_vsp_in_csc_set(
			 in_c_space,
			 out_c_space,
			 &middleformat,
			 in_image,
			 out_image,
			 src_ptr,
			 rq_type,
			 bl_hdl);

		/* VSP_IN set */
		imgpctrl_vsp_t_vsp_in_set(
			 in_layer,
			 ext_input,
			 in_c_space,
			 vsp_in_format,
			 src_ptr,
			 rq_type,
			 in_swap,
			 bl_hdl);

		/* reserved the input format of hw setting format */
		in_c_space = middleformat;

		/* alpha set */
		err = imgpctrl_vsp_in_alpha_set(
			in_layer, src_ptr, bl_hdl);
		if (err != IMGPCTRL_R_OK) {
			debug_data = __LINE__;
			/* output error & trace log */
			IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_BLEND,
			(u_short)debug_data, err ,
			UDS_ERR, bl_hdl);
			IMGPCTRL_TRACE_LOG(
				FID_IMGPCTRL_VSP_BLEND,
				FUNC_OUT_ERR, 1,
				(u_long *)&debug_data ,
				bl_hdl);
			return err;
		}

		if ((in_layer->rect.width != 0)
		 || (in_layer->rect.height != 0)) {
			/* resize */

			/* uds_par ptr setting */
			uds_ptr = &(vspm_p->uds_par[j]);

			/* UDS set */
			err = imgpctrl_vsp_t_uds_set(
				uds_ptr, in_layer, bl_hdl);
			if (err != IMGPCTRL_R_OK) {
				debug_data = __LINE__;
				/* output error & trace log */
				IMGPCTRL_ERROR_LOG(
				FID_IMGPCTRL_VSP_BLEND,
				(u_short)debug_data, err ,
				UDS_ERR, bl_hdl);
				IMGPCTRL_TRACE_LOG(
					FID_IMGPCTRL_VSP_BLEND,
					FUNC_OUT_ERR, 1,
					(u_long *)&debug_data ,
					bl_hdl);
				return err;
			}

			/* connect HW: UDS->BRU */
			uds_ptr->connect = VSP_BRU_USE;

			if (j == 0) {
				/* connect HW:RPF->UDS */
				src_ptr->connect = VSP_UDS_USE;

				/* UDS set */
				p_vsp->ctrl_par->uds = uds_ptr;
			} else if (j == 1) {
				/* connect HW:RPF->UDS1 */
				src_ptr->connect = VSP_UDS1_USE;

				/* UDS1 set */
				p_vsp->ctrl_par->uds1 = uds_ptr;
			} else {
				/* connect HW:RPF->UDS2 */
				src_ptr->connect = VSP_UDS2_USE;

				/* UDS2 set */
				p_vsp->ctrl_par->uds2 = uds_ptr;
			}

			j++;	/* uds_par address +1 */
		} else {
			/* connect HW:RPF->BRU */
			src_ptr->connect = VSP_BRU_USE;
		}

		if (i == 0)
			p_vsp->src1_par = src_ptr;
		else if (i == 1)
			p_vsp->src2_par = src_ptr;
		else if (i == 2)
			p_vsp->src3_par = src_ptr;
		else
			p_vsp->src4_par = src_ptr;

		/* blend set */
		imgpctrl_vsp_t_vsp_bru_set(
			exe_info, vspm_p, req_lay, bl_hdl);

		/* connect HW: BRU->WPF */
		vspm_p->bru_par.connect = 0;
	}


	/*  VSP_OUT set */
	imgpctrl_vsp_t_vsp_out_set(
		 in_c_space,
		 vsp_out_format,
		 out_image,
		 out_c_space,
		 dst_ptr,
		 out_swap,
		 bl_hdl);


	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_BLEND, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return err;
}


/* imgpctrl_vsp_zoom							*/
/* return value : status IMGPCTRL_R_OK					*/
/*                       IMGPCTRL_R_PARA_ERR				*/
/* argument     :   *exe_info -- IMGPCTRL control execute information	*/
/*		*vspm_p    -- VSPM parameter address			*/
/*		*bl_hdl -- log start pointer				*/
/* Description: Zoom in/out setting function				*/
static long imgpctrl_vsp_zoom(struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	struct screen_grap_handle *bl_hdl)
{
	long   err;
	u_short req_lay;
	u_int checkformat;
	u_char middleformat = 0;
	long  infmt, outfmt;
	struct screen_grap_image_param *in_image;
	struct screen_grap_image_param *out_image;
	struct t_ext_layer *ext_input;
	T_VSP_IN *src_ptr;
	T_VSP_OUT *dst_ptr;
	struct screen_grap_layer *in_layer;
	VSPM_VSP_PAR *p_vsp;
	u_short  vsp_in_format;
	u_short  vsp_out_format;
	T_VSP_UDS *uds_ptr;
	u_char  in_swap;
	u_char  out_swap;
	u_short rq_type;

	u_char  in_c_space;
	u_char  out_c_space;
	u_long debug_data;

	rq_type = exe_info->rq_type;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_ZOOM, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	p_vsp = &(vspm_p->p_vsp);

	/* IP type: VSP */
	vspm_p->vspm_ip.uhType = exe_info->hw_type;

	/* RPF use: 1 */
	p_vsp->rpf_num = 1;

	/* rpf_order: 0(unused) */
	p_vsp->rpf_order = 0;

	/* use module: resize */
	p_vsp->use_module = VSP_UDS_USE;

	/* Initialize set of unnecessary parameter(NULL pointer setting) */
	p_vsp->src2_par = (T_VSP_IN *)NULL;
	p_vsp->src3_par = (T_VSP_IN *)NULL;
	p_vsp->src4_par = (T_VSP_IN *)NULL;
	p_vsp->ctrl_par->sru = (T_VSP_SRU *)NULL;
	p_vsp->ctrl_par->uds1 = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->uds2 = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->lut = (T_VSP_LUT *)NULL;
	p_vsp->ctrl_par->clu = (T_VSP_CLU *)NULL;
	p_vsp->ctrl_par->hst = (T_VSP_HST *)NULL;
	p_vsp->ctrl_par->hsi = (T_VSP_HSI *)NULL;
	p_vsp->ctrl_par->bru = (T_VSP_BRU *)NULL;
	p_vsp->ctrl_par->hgo = (T_VSP_HGO *)NULL;
	p_vsp->ctrl_par->hgt = (T_VSP_HGT *)NULL;

	switch (exe_info->rq_layer) {
	case IMGPCTRL_REQ_LAYER_0:
		req_lay = 0;
		break;
	case IMGPCTRL_REQ_LAYER_1:
		req_lay = 1;
		break;
	case IMGPCTRL_REQ_LAYER_2:
		req_lay = 2;
		break;
	case IMGPCTRL_REQ_LAYER_3:
		req_lay = 3;
		break;
	default:
		/*the unvalid layer */
		err = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_ZOOM,
			(u_short)debug_data, err ,
			INVALID_LAYER, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_ZOOM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	in_layer =
	 (struct screen_grap_layer *)
	 (&(exe_info->input_layer[req_lay]));
	in_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->input_layer[req_lay].image));
	out_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->output_layer[req_lay].image));
	ext_input =
	(struct t_ext_layer *)
	(&(exe_info->ext_input[req_lay]));

	/* Input format check */
	checkformat = in_image->format;
	infmt = imgpctrl_vsp_format_get(
	 checkformat,
	 &vsp_in_format,
	 &in_swap,
	 &in_c_space,
	 bl_hdl);

	/* output format check */
	checkformat = out_image->format;
	outfmt = imgpctrl_vsp_format_get(
	 checkformat,
	 &vsp_out_format,
	 &out_swap,
	 &out_c_space,
	 bl_hdl);

	if ((infmt == IMGPCTRL_R_PARA_ERR) ||
		(outfmt == IMGPCTRL_R_PARA_ERR)) {
		err = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_ZOOM,
			(u_short)debug_data, err ,
			UNVALID_FORMAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_ZOOM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	/* The image format input pointer set */
	src_ptr = &(vspm_p->src_par[0]);
	dst_ptr = p_vsp->dst_par;

	/* CSC set */
	imgpctrl_vsp_in_csc_set(
		 in_c_space,
		 out_c_space,
		 &middleformat,
		 in_image,
		 out_image,
		 src_ptr,
		 rq_type,
		 bl_hdl);

	/* VSP_IN set */
	imgpctrl_vsp_t_vsp_in_set(
		 in_layer,
		 ext_input,
		 in_c_space,
		 vsp_in_format,
		 src_ptr,
		 rq_type,
		 in_swap,
		 bl_hdl);

	/* reserved the input format of hw setting format */
	in_c_space = middleformat;

	/* alpha set */
	err = imgpctrl_vsp_in_alpha_set(in_layer, src_ptr, bl_hdl);
	if (err != IMGPCTRL_R_OK) {
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_ZOOM,
			(u_short)debug_data, err ,
			UDS_ERR, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_ZOOM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	p_vsp->src1_par = src_ptr;

	/*  connect HW: RPF->UDS */
	src_ptr->connect = VSP_UDS_USE;

	/* call resize function */
	uds_ptr = p_vsp->ctrl_par->uds;
	err = imgpctrl_vsp_t_uds_set(uds_ptr, in_layer, bl_hdl);
	if (err != IMGPCTRL_R_OK) {
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_ZOOM,
			(u_short)debug_data, err ,
			UDS_ERR, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_ZOOM, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	/* connect HW: UDS->WPF*/
	uds_ptr->connect = 0;


	/*  VSP_OUT set */
	imgpctrl_vsp_t_vsp_out_set(
		 in_c_space,
		 vsp_out_format,
		 out_image,
		 out_c_space,
		 dst_ptr,
		 out_swap,
		 bl_hdl);

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_ZOOM, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return err;
}

/* imgpctrl_vsp_conversion						*/
/* return value : status	IMGPCTRL_R_OK				*/
/*			IMGPCTRL_R_PARA_ERR				*/
/* argument     :   *exe_info -- IMGPCTRL control execute information	*/
/*		*Vspm  -- VSPM parameter address			*/
/*		*bl_hdl -- log start pointer				*/
/* Description: Colorconversion and trimming setting function		*/
static long imgpctrl_vsp_conversion(
	struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	struct screen_grap_handle *bl_hdl)
{

	long   err;
	u_short req_lay;
	u_short checkformat;
	u_char middleformat = 0;
	long  infmt, outfmt;
	struct screen_grap_image_param *in_image;
	struct screen_grap_image_param *out_image;
	struct t_ext_layer *ext_input;
	T_VSP_IN *src_ptr;
	T_VSP_OUT *dst_ptr;
	struct screen_grap_layer *in_layer;
	VSPM_VSP_PAR *p_vsp;
	u_short  vsp_in_format;
	u_short  vsp_out_format;
	u_char  in_swap;
	u_char  out_swap;

	u_short rq_type;

	u_char  in_c_space;
	u_char  out_c_space;

	u_long debug_data;

	rq_type = exe_info->rq_type;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_CONVERSION, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);


	p_vsp = &(vspm_p->p_vsp);

	/* IP type: VSP */
	vspm_p->vspm_ip.uhType = exe_info->hw_type;

	/* rpf_num: 1 */
	p_vsp->rpf_num = 1;

	/* rpf_order: 0(unused) */
	p_vsp->rpf_order = 0;

	/* usemodule: nothing(connect WPF) */
	p_vsp->use_module = 0;

	/* Initialize set of unnecessary parameter(NULL pointer setting) */
	p_vsp->src2_par = (T_VSP_IN *)NULL;
	p_vsp->src3_par = (T_VSP_IN *)NULL;
	p_vsp->src4_par = (T_VSP_IN *)NULL;
	p_vsp->ctrl_par->sru = (T_VSP_SRU *)NULL;
	p_vsp->ctrl_par->uds = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->uds1 = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->uds2 = (T_VSP_UDS *)NULL;
	p_vsp->ctrl_par->lut = (T_VSP_LUT *)NULL;
	p_vsp->ctrl_par->clu = (T_VSP_CLU *)NULL;
	p_vsp->ctrl_par->hst = (T_VSP_HST *)NULL;
	p_vsp->ctrl_par->hsi = (T_VSP_HSI *)NULL;
	p_vsp->ctrl_par->bru = (T_VSP_BRU *)NULL;
	p_vsp->ctrl_par->hgo = (T_VSP_HGO *)NULL;
	p_vsp->ctrl_par->hgt = (T_VSP_HGT *)NULL;

	switch (exe_info->rq_layer) {
	case IMGPCTRL_REQ_LAYER_0:
		req_lay = 0;
		break;
	case IMGPCTRL_REQ_LAYER_1:
		req_lay = 1;
		break;
	case IMGPCTRL_REQ_LAYER_2:
		req_lay = 2;
		break;
	case IMGPCTRL_REQ_LAYER_3:
		req_lay = 3;
		break;
	default:
		/* the unvalid layer */
		err = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_CONVERSION,
			(u_short)debug_data, err ,
			INVALID_LAYER, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_CONVERSION, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}


	in_layer =
	 (struct screen_grap_layer *)
	 (&(exe_info->input_layer[req_lay]));
	in_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->input_layer[req_lay].image));
	out_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->output_layer[req_lay].image));
	ext_input =
	(struct t_ext_layer *)
	(&(exe_info->ext_input[req_lay]));

	/* Input format check */
	checkformat = (u_short)(in_image->format);
	infmt = imgpctrl_vsp_format_get(
	 checkformat,
	 &vsp_in_format,
	 &in_swap,
	 &in_c_space,
	 bl_hdl);

	/* output format check */
	checkformat = (u_short)(out_image->format);
	outfmt = imgpctrl_vsp_format_get(
	 checkformat,
	 &vsp_out_format,
	 &out_swap,
	 &out_c_space,
	 bl_hdl);

	if ((infmt == IMGPCTRL_R_PARA_ERR) ||
		(outfmt == IMGPCTRL_R_PARA_ERR)) {
		err = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_CONVERSION,
			(u_short)debug_data, err ,
			UNVALID_FORMAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_CONVERSION, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	/* The image format input pointer set */
	src_ptr = &(vspm_p->src_par[0]);
	dst_ptr = p_vsp->dst_par;

	/* CSC set */
	imgpctrl_vsp_in_csc_set(
		 in_c_space,
		 out_c_space,
		 &middleformat,
		 in_image,
		 out_image,
		 src_ptr,
		 rq_type,
		 bl_hdl);

	/* VSP_IN set */
	imgpctrl_vsp_t_vsp_in_set(
		 in_layer,
		 ext_input,
		 in_c_space,
		 vsp_in_format,
		 src_ptr,
		 rq_type,
		 in_swap,
		 bl_hdl);
	/* reserved the input format of hw setting format */
	in_c_space = middleformat;

	/* alpha set */
	err = imgpctrl_vsp_in_alpha_set(in_layer, src_ptr, bl_hdl);
	if (err != IMGPCTRL_R_OK) {
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_CONVERSION,
			(u_short)debug_data, err ,
			UNVALID_FORMAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_CONVERSION, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return err;
	}

	p_vsp->src1_par = src_ptr;

	/*  connect HW: RPF->WPF */
	src_ptr->connect = 0;

	/*  VSP_OUT set */
	imgpctrl_vsp_t_vsp_out_set(
		 in_c_space,
		 vsp_out_format,
		 out_image,
		 out_c_space,
		 dst_ptr,
		 out_swap,
		 bl_hdl);

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_CONVERSION, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return err;
}


/* imgpctrl_vsp_format_get					*/
/* return value : status	IMGPCTRL_R_OK			*/
/*				IMGPCTRL_R_PARA_ERR		*/
/* argument     :	informat  -- API format setting		*/
/*			outformat -- VSP format setting		*/
/*			swap      -- swap			*/
/*			csapce    -- color space		*/
/*			*bl_hdl   -- log start pointer		*/
/* Description: Check and get vsp format, swap ,color space	*/
static long imgpctrl_vsp_format_get(u_int informat, u_short *outformat,
	u_char *swap, u_char *cspace, struct screen_grap_handle *bl_hdl)
{
	long ret = IMGPCTRL_R_OK;
	u_long debug_data = 0;
	*cspace = 0xFF;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_FORMAT_GET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	/* VSP_IN_XXXX = VSP_OUT_XXXX */
	/* Therefore set to VSP_IN_XXXX in here */
	switch (informat) {
	case RT_GRAPHICS_COLOR_RGB565:
		*outformat = VSP_IN_RGB565;
		*swap = VSP_SWAP_LL | VSP_SWAP_L | VSP_SWAP_W;
		*cspace = IMGPCTRL_VSP_RGB;
		break;
	case RT_GRAPHICS_COLOR_RGB888:
		*outformat = VSP_IN_RGB888;
		*swap = VSP_SWAP_LL | VSP_SWAP_L |
			VSP_SWAP_W | VSP_SWAP_B;
		*cspace = IMGPCTRL_VSP_RGB;
		break;
	case RT_GRAPHICS_COLOR_ARGB8888:
		*outformat = VSP_IN_ARGB8888;
		*swap = VSP_SWAP_LL | VSP_SWAP_L;
		*cspace = IMGPCTRL_VSP_RGB;
		break;
	case RT_GRAPHICS_COLOR_XRGB8888:
		*outformat = VSP_IN_ARGB8888;
		*swap = VSP_SWAP_LL | VSP_SWAP_L;
		*cspace = IMGPCTRL_VSP_RGB;
		break;
	case RT_GRAPHICS_COLOR_ABGR8888:
		*outformat = VSP_IN_ABGR8888;
		*swap = VSP_SWAP_LL | VSP_SWAP_L;
		*cspace = IMGPCTRL_VSP_RGB;
		break;
	case RT_GRAPHICS_COLOR_XBGR8888:
		*outformat = VSP_IN_ABGR8888;
		*swap = VSP_SWAP_LL | VSP_SWAP_L;
		*cspace = IMGPCTRL_VSP_RGB;
		break;
	case RT_GRAPHICS_COLOR_YUV420SP:
		*outformat = VSP_IN_YUV420_SEMI_PLANAR;
		*swap = VSP_SWAP_LL | VSP_SWAP_L |
			VSP_SWAP_W | VSP_SWAP_B;
		*cspace = IMGPCTRL_VSP_YUV;
		break;
	case RT_GRAPHICS_COLOR_YUV420SP_NV21:
		*outformat = VSP_IN_YUV420_SEMI_NV21;
		*swap = VSP_SWAP_LL | VSP_SWAP_L |
			VSP_SWAP_W | VSP_SWAP_B;
		*cspace = IMGPCTRL_VSP_YUV;
		break;
	case RT_GRAPHICS_COLOR_YUV422SP:
		*outformat = VSP_IN_YUV422_SEMI_PLANAR;
		*swap = VSP_SWAP_LL | VSP_SWAP_L |
			VSP_SWAP_W | VSP_SWAP_B;
		*cspace = IMGPCTRL_VSP_YUV;
		break;
	case RT_GRAPHICS_COLOR_YUV420PL:
		*outformat = VSP_IN_YUV420_PLANAR;
		*swap = VSP_SWAP_LL | VSP_SWAP_L |
			VSP_SWAP_W | VSP_SWAP_B;
		*cspace = IMGPCTRL_VSP_YUV;
		break;
	case RT_GRAPHICS_COLOR_YUV422I_UYVY:
		*outformat = VSP_IN_YUV422_INTERLEAVED0;
		*swap = VSP_SWAP_LL | VSP_SWAP_L |
			VSP_SWAP_W | VSP_SWAP_B;
		*cspace = IMGPCTRL_VSP_YUV;
		break;
	default:
		ret = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_FORMAT_GET,
			(u_short)debug_data, ret ,
			UNVALID_FORMAT, bl_hdl);
		break;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_FORMAT_GET, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);
	if (ret == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_FORMAT_GET, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_FORMAT_GET, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ret;
}


/*  T_VSP_IN:CSC only set		*/
/* internal function			*/
static void imgpctrl_vsp_in_csc_set(
	u_char inputformat,
	u_char outputformat,
	u_char  *middleformat,
	const struct screen_grap_image_param *in_image,
	const struct screen_grap_image_param *out_image,
	T_VSP_IN *src_ptr,
	u_short rq_type,
	struct screen_grap_handle *bl_hdl)
{
	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_IN_CSC_SET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if ((rq_type & IMGPCTRL_REQ_BLEND) != IMGPCTRL_REQ_BLEND) {
		/* not blend */
		if (inputformat == IMGPCTRL_VSP_RGB) {
			/* Input:RGB */
			if (inputformat != outputformat) {
				/* Output:YUV CSC:ON,VSP:YUV */
				src_ptr->csc =  VSP_CSC_ON;
				*middleformat = IMGPCTRL_VSP_YUV;
			} else {
				/* Output:RGB CSC:OFF,VSP=RGB */
				src_ptr->csc =  VSP_CSC_OFF;
				*middleformat = IMGPCTRL_VSP_RGB;
			}
		} else if (inputformat == IMGPCTRL_VSP_YUV) {
			/* Input:YUV */
			if (inputformat != outputformat) {
				/* Output:RGB CSC:ON,VSP=RGB */
				src_ptr->csc =  VSP_CSC_ON;
				*middleformat = IMGPCTRL_VSP_RGB;
			} else {
			/* Output:YUV */
				if (in_image->yuv_format !=
					 out_image->yuv_format) {
					/* ITU-R BT format different */
					/* CSC:ON,VSP=RGB*/
					src_ptr->csc =  VSP_CSC_ON;
					*middleformat = IMGPCTRL_VSP_RGB;
				} else if (in_image->yuv_range !=
					 out_image->yuv_range) {
					/* scale is different */
					/* CSC:ON,VSP=RGB*/
					src_ptr->csc =  VSP_CSC_ON;
					*middleformat = IMGPCTRL_VSP_RGB;
				} else {
					/* CSC:OFF,VSP=YUV */
					src_ptr->csc =  VSP_CSC_OFF;
					*middleformat = IMGPCTRL_VSP_YUV;
				}
			}
		} else {
			/* No Operation */
			/* blank */
		}
	} else {
		/*blend yes*/
		if (inputformat == IMGPCTRL_VSP_YUV) {
			/* Input:YUV CSC:ON,VSP=RGB*/
			src_ptr->csc =  VSP_CSC_ON;
			*middleformat = IMGPCTRL_VSP_RGB;
		} else {
			/* Input:RGB CSC:OFF,VSP=RGB*/
			src_ptr->csc =  VSP_CSC_OFF;
			*middleformat = IMGPCTRL_VSP_RGB;
		}

	}
	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_IN_CSC_SET, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

}


/*  T_VSP_IN set */
static void imgpctrl_vsp_t_vsp_in_set(
	const struct screen_grap_layer *in_layer,
	const struct t_ext_layer *ext_input,
	u_char informat,
	u_short vsp_in_format,
	T_VSP_IN *src_ptr,
	u_short rq_type,
	u_char swap,
	struct screen_grap_handle *bl_hdl)
{

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_VSP_IN_SET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	/* address set */
	src_ptr->addr = in_layer->image.address;
	if (informat == IMGPCTRL_VSP_YUV) {
		src_ptr->addr_c0 = in_layer->image.address_c0;
		src_ptr->stride_c = in_layer->image.stride_c;
	} else {
		src_ptr->addr_c0 = (void *)NULL;
		src_ptr->stride_c = 0;
	}

	if (in_layer->image.format == RT_GRAPHICS_COLOR_YUV420PL)
		src_ptr->addr_c1 = in_layer->image.address_c1;
	 else
		src_ptr->addr_c1 = (void *)NULL;

	/* stride set */
	src_ptr->stride = in_layer->image.stride;

	/*width/height set */
	src_ptr->width = in_layer->image.width;
	src_ptr->height = in_layer->image.height;

	src_ptr->width_ex  = 0;
	src_ptr->height_ex = 0;

	/* trimming set */
	src_ptr->x_offset  = ext_input->x_offset;
	src_ptr->y_offset = ext_input->y_offset;

	/* VSP set */
	src_ptr->format = vsp_in_format;

	/* SWAP */
	src_ptr->swap = swap;

	if ((rq_type & IMGPCTRL_REQ_BLEND) == IMGPCTRL_REQ_BLEND) {
		/* blend status = ON */
		/* setting the poition of blend */
		src_ptr->x_position = in_layer->rect.x;
		src_ptr->y_position = in_layer->rect.y;

		src_ptr->pwd = VSP_LAYER_CHILD;

	} else {
		/* blend status = OFF */
		/* set potision 0,0 */
		src_ptr->x_position = 0;
		src_ptr->y_position = 0;

		src_ptr->pwd = VSP_LAYER_PARENT;
	}

	src_ptr->cipm = VSP_CIPM_0_HOLD;
	src_ptr->cext  = VSP_CEXT_EXPAN;

	/* csc setting ->  imgpctrl_vsp_IN_CSC_SET() set */

	src_ptr->iturbt = (u_char)in_layer->image.yuv_format;
	src_ptr->clrcng = (u_char)in_layer->image.yuv_range;

	src_ptr->vir = VSP_NO_VIR;

	src_ptr->vircolor = 0x000000000;

	src_ptr->osd_lut = (T_VSP_OSDLUT *)NULL;

	/* alpha_blend -> imgpctrl_vsp_IN_ALPHA() set */

	src_ptr->clrcnv = (T_VSP_CLRCNV *)NULL;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_VSP_IN_SET, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);
}



/*  T_VSP_IN:ALPHA only set */
static long imgpctrl_vsp_in_alpha_set(
	const struct screen_grap_layer *in_layer,
	T_VSP_IN *src_ptr,
	struct screen_grap_handle *bl_hdl)
{
	T_VSP_ALPHA *alpha;
	u_short format;
	u_long debug_data;
	long   err = IMGPCTRL_R_OK;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_IN_ALPHA_SET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	alpha = src_ptr->alpha_blend;
	format = (u_short)in_layer->image.format;

	alpha->addr_a = (void *)NULL;
	alpha->alphan = VSP_ALPHA_NO;
	alpha->alpha1 = 0;
	alpha->alpha2 = 0;
	alpha->astride = 0;
	alpha->aswap = 0;

	if ((format == RT_GRAPHICS_COLOR_ARGB8888) ||
		(format == RT_GRAPHICS_COLOR_ABGR8888))
		/* set alpha */
		alpha->asel =  VSP_ALPHA_NUM1;
	else
		/* the fix alpha value(0xff) use */
		alpha->asel = VSP_ALPHA_NUM5;

	alpha->aext = VSP_AEXT_EXPAN_MAX;

	alpha->anum0 = 0;
	alpha->anum1 = 0;

	if ((in_layer->alpha & 0xFF00) != 0) {
		/* parameter error */
		err = IMGPCTRL_R_PARA_ERR;
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_IN_ALPHA_SET,
			(u_short)debug_data, err ,
			UNVALID_FORMAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_IN_ALPHA_SET, FUNC_OUT_ERR,
			1, (u_long *)&debug_data , bl_hdl);
		return err;
	} else {
		if (alpha->asel == VSP_ALPHA_NUM5)
			alpha->afix = (u_char)(in_layer->alpha & 0x00FF);
		else
			alpha->afix = IMGPCTRL_FULL_OPAQUE;
	}

	alpha->irop = VSP_IROP_NOP;
	alpha->msken = VSP_MSKEN_ALPHA;
	alpha->bsel = VSP_ALPHA_8BIT;
	alpha->mgcolor = 0;
	alpha->mscolor0 = 0;
	alpha->mscolor1 = 0;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_IN_ALPHA_SET, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return err;
}


/*  T_VSP_OUT:set */
static void imgpctrl_vsp_t_vsp_out_set(
	u_char informat,
	u_short vsp_out_format,
	const struct screen_grap_image_param *out_image,
	u_char outformat,
	T_VSP_OUT *dst_ptr,
	u_char swap,
	struct screen_grap_handle *bl_hdl)
{
	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_VSP_OUT_SET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	/*output address set */
	dst_ptr->addr = out_image->address;
	if (outformat == IMGPCTRL_VSP_YUV) {
		dst_ptr->addr_c0 = out_image->address_c0;
		dst_ptr->stride_c = out_image->stride_c;
	} else {
		dst_ptr->addr_c0 = NULL;
		dst_ptr->stride_c = 0;
	}

	if (out_image->format == RT_GRAPHICS_COLOR_YUV420PL)
		dst_ptr->addr_c1 = out_image->address_c1;
	else
		dst_ptr->addr_c1 = NULL;

	/* stride set */
	dst_ptr->stride = out_image->stride;

	/* width/height set */
	dst_ptr->width = out_image->width;
	dst_ptr->height = out_image->height;

	/* output offset =0 fix */
	dst_ptr->x_offset  = 0;
	dst_ptr->y_offset = 0;

	dst_ptr->format = vsp_out_format;

	/* SWAP */
	dst_ptr->swap = swap;


	if ((out_image->format == RT_GRAPHICS_COLOR_XRGB8888) ||
		(out_image->format == RT_GRAPHICS_COLOR_XBGR8888)) {
		dst_ptr->pxa = VSP_PAD_P;
		dst_ptr->pad = IMGPCTRL_FULL_OPAQUE;
	} else {
		dst_ptr->pxa = VSP_PAD_IN;
		dst_ptr->pad = IMGPCTRL_FULL_TRANSPARENT;
	}


	dst_ptr->x_coffset = 0;
	dst_ptr->y_coffset = 0;

	if (informat == outformat)
		dst_ptr->csc = VSP_CSC_OFF;
	else
		dst_ptr->csc = VSP_CSC_ON;


	dst_ptr->iturbt = (u_char)out_image->yuv_format;
	dst_ptr->clrcng = (u_char)out_image->yuv_range;

	dst_ptr->cbrm = VSP_CSC_ROUND_DOWN;
	dst_ptr->abrm = VSP_CONVERSION_ROUNDDOWN;
	dst_ptr->athres = 0;

	if (outformat == IMGPCTRL_VSP_YUV) {
		if ((out_image->yuv_format == RT_GRAPHICS_COLOR_BT601) &&
		 (out_image->yuv_range == RT_GRAPHICS_COLOR_FULLSCALE)) {
			dst_ptr->clmd = VSP_CLMD_NO;
		} else
			dst_ptr->clmd = VSP_CLMD_MODE1;
	}  else
		dst_ptr->clmd = VSP_CLMD_NO;

	dst_ptr->dith = VSP_NO_DITHER;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_VSP_OUT_SET, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

}


/* BRU setting */
static void imgpctrl_vsp_t_vsp_bru_set(
	 struct t_exe_info *exe_info,
	 struct t_vspm_vsp *vspm_p,
	 u_char req_lay,
	struct screen_grap_handle *bl_hdl)
{
	int i;
	T_VSP_BRU *bru;
	T_VSP_BLEND_VIRTUAL *b_virtual;
	T_VSP_BLEND_CONTROL *b_cntl;
	struct screen_grap_layer *in_layer;
	struct screen_grap_layer *out_layer;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_VSP_BRU_SET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	bru = &(vspm_p->bru_par);
	b_virtual = &(vspm_p->bru_vir_par);

	if (req_lay == 0) {
		/*  virtual layer setting */
		bru->lay_order = VSP_LAY_VIRTUAL;

		/* Set NULL unused all blend control  */
		bru->blend_control_a = (T_VSP_BLEND_CONTROL *)NULL;
		bru->blend_control_b = (T_VSP_BLEND_CONTROL *)NULL;
		bru->blend_control_c = (T_VSP_BLEND_CONTROL *)NULL;
		bru->blend_control_d = (T_VSP_BLEND_CONTROL *)NULL;

	} else if (req_lay == 1) {
		/* virtual | input layer1 */
		bru->lay_order = VSP_LAY_VIRTUAL |
			 (VSP_LAY_1 << IMGPCTRL_4BIT_SHIFT);

		/* Set NULL unused blend control */
		bru->blend_control_b = (T_VSP_BLEND_CONTROL *)NULL;
		bru->blend_control_c = (T_VSP_BLEND_CONTROL *)NULL;
		bru->blend_control_d = (T_VSP_BLEND_CONTROL *)NULL;

	} else if (req_lay == 2) {
		/* virtual | input layer1 | input layer2 */
		bru->lay_order = VSP_LAY_VIRTUAL |
		 (VSP_LAY_1 << IMGPCTRL_4BIT_SHIFT) |
		 (VSP_LAY_2 << IMGPCTRL_8BIT_SHIFT);

		/* Set NULL unused blend control */
		bru->blend_control_c = (T_VSP_BLEND_CONTROL *)NULL;
		bru->blend_control_d = (T_VSP_BLEND_CONTROL *)NULL;

	} else if (req_lay == 3) {
		/* virtual | input layer1 | input layer2 | input layer3 */
		bru->lay_order = VSP_LAY_VIRTUAL |
		 (VSP_LAY_1 << IMGPCTRL_4BIT_SHIFT) |
		 (VSP_LAY_2 << IMGPCTRL_8BIT_SHIFT) |
		  (VSP_LAY_3 << IMGPCTRL_12BIT_SHIFT);


		/* Set NULL unused blend control */
		bru->blend_control_d = (T_VSP_BLEND_CONTROL *)NULL;
	} else {
		/* virtual | input layer1 | input layer2 | */
		/* input layer3 | input layer4 */
		bru->lay_order = VSP_LAY_VIRTUAL |
		 (VSP_LAY_1 << IMGPCTRL_4BIT_SHIFT) |
		 (VSP_LAY_2 << IMGPCTRL_8BIT_SHIFT) |
		  (VSP_LAY_3 << IMGPCTRL_12BIT_SHIFT)|
		  (VSP_LAY_4 << IMGPCTRL_16BIT_SHIFT);
	}

	out_layer =
	 (struct screen_grap_layer *)(&(exe_info->output_layer[0]));

	bru->adiv = VSP_DIVISION_OFF;

	for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
		bru->qnt[i] = VSP_QNT_OFF;
		bru->dith[i] = VSP_DITH_OFF;
	}

	/* set virtual(background) image */
	b_virtual->width = out_layer->image.width;
	b_virtual->height = out_layer->image.height;

	b_virtual->x_position = 0;
	b_virtual->y_position = 0;

	/* set virtual parent */
	b_virtual->pwd = VSP_LAYER_PARENT;

	/* set background color */
	if ((out_layer->image.format == RT_GRAPHICS_COLOR_ARGB8888) ||
		(out_layer->image.format == RT_GRAPHICS_COLOR_ABGR8888)) {
		/* bg color = setting value */
		b_virtual->color = exe_info->bg_color;
	} else {
		/* bg color = alpha(0xff)+ color setting value */
		b_virtual->color = (IMGPCTRL_FIXALPHA
			| exe_info->bg_color);
	}

	/* max check */
	if (req_lay > IMGPCTRL_LAYER_MAX) {
		MSG_ERROR("[%s] ERR: Max layer over.Line:%d ercd=%d\n",
			 __func__, __LINE__, req_lay);
		req_lay = IMGPCTRL_LAYER_MAX;
	}

	for (i = 0; i < req_lay; i++) {
		/* get every layer the blend setting */
		b_cntl = &(vspm_p->bru_ctrl_par[i]);
		in_layer =
		 (struct screen_grap_layer *)(&(exe_info->input_layer[i]));

		b_cntl->rbc = VSP_RBC_BLEND;
		b_cntl->crop = VSP_IROP_NOP;
		b_cntl->arop = VSP_IROP_NOP;
		b_cntl->blend_formula = VSP_FORM_BLEND0;
		b_cntl->blend_coefx = VSP_COEFFICIENT_BLENDX4;

		if (in_layer->premultiplied == RT_GRAPHICS_PREMULTI_ON)
			b_cntl->blend_coefy = VSP_COEFFICIENT_BLENDY5;
		else
			b_cntl->blend_coefy = VSP_COEFFICIENT_BLENDY3;

		b_cntl->aformula = VSP_FORM_ALPHA0;

		b_cntl->acoefx = VSP_COEFFICIENT_ALPHAX4;
		b_cntl->acoefy = VSP_COEFFICIENT_ALPHAY5;

		b_cntl->acoefx_fix = IMGPCTRL_FULL_TRANSPARENT;
		b_cntl->acoefy_fix = IMGPCTRL_FULL_OPAQUE;

		if (i == 0)
			bru->blend_control_a  = b_cntl;
		else if (i == 1)
			bru->blend_control_b = b_cntl;
		else if (i == 2)
			bru->blend_control_c = b_cntl;
		else
			bru->blend_control_d = b_cntl;
	}

	bru->blend_rop->crop = VSP_IROP_NOP;
	bru->blend_rop->arop = VSP_IROP_NOP;

	MSG_TRACE("[%s] ST : out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_VSP_BRU_SET, FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

}

/* imgpctrl_vsp_t_uds_set					*/
/* return value :	IMGPCTRL_R_OK				*/
/*			IMGPCTRL_R_PARA_ERR			*/
/* argument     :	uds_parame -- setting structure		*/
/*			uds_param -- scaling setting structure	*/
/*			layer -- layer				*/
/*			*bl_hdl -- log start pointer		*/
/* Description: Set parameter of UDS Structure			*/
static long imgpctrl_vsp_t_uds_set(T_VSP_UDS *uds_param,
	const struct screen_grap_layer *layer,
	struct screen_grap_handle *bl_hdl)
{
	long ret = IMGPCTRL_R_OK;
	long uds_ret_w;
	long uds_ret_h;
	u_short scale;
	u_long debug_data = 0;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_UDS_SET, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	uds_param->amd = VSP_AMD_NO;
	uds_param->fmd = VSP_FMD_NO;
	/* if fmd is VSP_FMD_NO, filcolor is invalid */
	uds_param->filcolor = 0x00000000;
	uds_param->clip = VSP_CLIP_OFF;
	uds_param->alpha = VSP_ALPHA_ON;
	uds_param->complement = VSP_COMPLEMENT_BIL;
	uds_param->athres0 = 0x00;
	uds_param->athres1 = 0x00;
	uds_param->anum0 = 0x00;
	uds_param->anum1 = 0x00;
	uds_param->anum2 = 0x00;
	uds_ret_w = imgpctrl_vsp_uds_get_scaling_ratio(layer->image.width,
		layer->rect.width, &scale, bl_hdl);
	uds_param->x_ratio = scale;
	uds_ret_h = imgpctrl_vsp_uds_get_scaling_ratio(layer->image.height,
		layer->rect.height, &scale, bl_hdl);
	uds_param->y_ratio = scale;
	uds_param->out_cwidth = layer->rect.width;
	uds_param->out_cheight = layer->rect.height;

	if ((uds_ret_w != IMGPCTRL_R_OK)
		|| (uds_ret_h != IMGPCTRL_R_OK)) {
		debug_data = __LINE__;
		ret = IMGPCTRL_R_PARA_ERR;
	}

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	if (ret == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_UDS_SET, FUNC_OUT_NRM, 0,
			(u_long *)NULL , bl_hdl);
	} else {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_T_UDS_SET, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
	}

	return ret;
}

/**
 * imgpctrl_vsp_uds_get_scaling_ratio - Calculating UDS ratio
 * ush_in_size:  input size
 * ush_out_size: output size
 * ush_ratio:	pointer of UDS ratio
 * bl_hdl : log start pointer
 * Description: Calculate UDS ratio(AMD=0) and check error.
 * Returns: On success IMGPCTRL_R_OK is returned.
 *          On error IMGPCTRL_R_PARA_ERR is returned.
 */
static long imgpctrl_vsp_uds_get_scaling_ratio(
	u_short ush_in_size,
	u_short ush_out_size,
	u_short *ush_ratio,
	struct screen_grap_handle *bl_hdl)
{
	u_long alpha;
	u_short round_value;
	u_long debug_data;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO, FUNC_IN, 0,
		(u_long *)NULL , bl_hdl);

	if ((ush_in_size < ush_out_size) &&
		((ush_in_size * 16) < ush_out_size)) {
		MSG_ERROR("[%s] ERR: expand ratio is over 16x.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		/* output error &trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			(u_short)debug_data, IMGPCTRL_R_PARA_ERR ,
			OUT_OF_RANGE_RAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_PARA_ERR;
	}
	if ((ush_in_size > ush_out_size) &&
		(ush_in_size > (ush_out_size * 16))) {
		MSG_ERROR("[%s] ERR: expand ratio is over 1/16x.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			(u_short)debug_data, IMGPCTRL_R_PARA_ERR ,
			OUT_OF_RANGE_RAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_PARA_ERR;
	}
	if (ush_out_size < 4) {
		MSG_ERROR("[%s] ERR: ush_out_size is smaller than 4.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			(u_short)debug_data, IMGPCTRL_R_PARA_ERR ,
			OUT_OF_RANGE_RAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_PARA_ERR;
	}
	if (ush_ratio == NULL) {
		MSG_ERROR("[%s] ERR: ush_ratio is Null pointer.Line:%d\n",
			__func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(
			FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			(u_short)debug_data, IMGPCTRL_R_PARA_ERR,
			OUT_OF_RANGE_RAT, bl_hdl);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
			FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , bl_hdl);
		return IMGPCTRL_R_PARA_ERR;
	}

	if (ush_in_size < ush_out_size) {
		round_value = ush_out_size / 2;	/* round off */
		/* expanding */
		alpha = (u_long)((((ush_in_size - 1)
				* 4096) + round_value) / (ush_out_size - 1));
		if (alpha < IMGPCTRL_RF_ALFAMIN)
			alpha = IMGPCTRL_RF_ALFAMIN;
	} else if (ush_out_size < ush_in_size) {
		alpha = imgpctrl_vsp_uds_scale_down_process(
				ush_in_size,
				ush_out_size,
				bl_hdl);

	} else {
		/* same size */
		alpha = IMGPCTRL_RF_ALFACEN;
	}
	*ush_ratio = (u_short)alpha;
	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO,
		FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return IMGPCTRL_R_OK;
}

/**
 * imgpctrl_vsp_uds_scale_down_process - Calculating UDS ScaleDown Rate
 * ush_in_size:  input size
 * ush_out_size: output size
 * ush_ratio:	pointer of UDS ratio
 * log_ptr : log start pointer
 * Description: Calculate UDS Scale Down Ratio.
 * Returns: On success IMGPCTRL_R_OK is returned.
 *          On error IMGPCTRL_R_PARA_ERR is returned.
 */
static u_long imgpctrl_vsp_uds_scale_down_process(
	u_short ush_in_size,
	u_short ush_out_size,
	struct screen_grap_handle *bl_hdl)
{
	u_short	mant;
	u_short	mant_pre;
	u_short	mant_pre_r;
	u_long alpha;
	u_short round_value;

	MSG_TRACE("[%s] ST: in. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_SCALEDOWNPROCESS,
		FUNC_IN, 0, (u_long *)NULL , bl_hdl);

	mant = ush_in_size / ush_out_size;
	round_value = ush_out_size / 2;	/* round off */

	if (mant <= IMGPCTRL_RF_MNTMAX) {
		if ((1 <= mant) && (mant < 4))
			mant_pre	= IMGPCTRL_MANTPRE_MIN;	/* mantpre:1 */
		else if ((4 <= mant) && (mant < 8))
			mant_pre	= IMGPCTRL_MANTPRE_MID;	/* mantpre:2 */
		else
			mant_pre	= IMGPCTRL_MANTPRE_MAX;	/* mantpre:4 */

		alpha = (u_long)((((((((mant_pre + ush_in_size) - 1)
				/ mant_pre) - 1) * mant_pre)
				* 4096) + round_value)
				/ (ush_out_size - 1));

		if (IMGPCTRL_RF_ALFAMAX < alpha)
			alpha = IMGPCTRL_RF_ALFAMAX; /* overflow */

		mant = (u_short)(0x000F & (alpha >> 12));

		if ((1 <= mant) && (mant < 4)) {
			/* mantpreR:1 */
			mant_pre_r = IMGPCTRL_MANTPRE_MIN;
		} else if ((4 <= mant) && (mant < 8)) {
			/* mantpreR:2 */
			mant_pre_r = IMGPCTRL_MANTPRE_MID;
		} else {
			/* mantpreR:4 */
			mant_pre_r = IMGPCTRL_MANTPRE_MAX;
		}

		if (mant_pre != mant_pre_r) {
			alpha = (u_long)((((((((mant_pre_r + ush_in_size) - 1)
					/ mant_pre_r) - 1) * mant_pre_r)
					* 4096) + round_value)
					/ (ush_out_size - 1));

			if (mant_pre < mant_pre_r) {
				if (alpha < (mant_pre_r << 12))
					alpha = mant_pre_r << 12;
			} else {
				if ((mant_pre << 12) <= alpha)
					alpha = mant_pre << 12;
			}
		}
	} else
		alpha = IMGPCTRL_RF_ALFAMAX;

	if (IMGPCTRL_RF_ALFAMAX < alpha)
		alpha = IMGPCTRL_RF_ALFACEN;

	MSG_TRACE("[%s] ST: out. Line:%d\n", __func__, __LINE__);
	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_VSP_UDS_SCALEDOWNPROCESS,
		FUNC_OUT_NRM, 0,
		(u_long *)NULL , bl_hdl);

	return alpha;
}

