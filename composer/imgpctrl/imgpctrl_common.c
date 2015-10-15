/*************************************************************************/ /*
  imgpctrl_common.c
   imgpctrl common function file.

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
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/time.h>

#include <linux/sched.h>


#include "imgpctrl_private.h"
#include "imgpctrl_common.h"


/************************************************************************/
/* debug function                                                       */
/************************************************************************/
/************************************************************************/
/* Function     : imgpctrl_cmn_dbg_exe_param                            */
/* Description  : Print parameter of imgpctrl ctrl execution for DEBUG. */
/* Return value : None                                                  */
/* Argument     : *exe_info - imgpctrl ctrl exe data pointer            */
/*                lay - layer number                                    */
/************************************************************************/
void imgpctrl_cmn_dbg_exe_param(struct t_exe_info *exe_info, u_short lay)
{
	struct screen_grap_layer *lyp;

	MSG_DEBUG("%s", "[INFO] ********* t_exe_info *********\n");
	MSG_DEBUG("[INFO] hw_type       =%d\n", exe_info->hw_type);
	MSG_DEBUG("[INFO] rq_type       =%d\n", exe_info->rq_type);
	MSG_DEBUG("[INFO] rq_layer      =%d\n", exe_info->rq_layer);
	MSG_DEBUG("[INFO] bg_color      =%ld\n", exe_info->bg_color);

	lyp = (struct screen_grap_layer *)(&(exe_info->input_layer[lay]));
	MSG_DEBUG("[INFO] ********* input_layer[%d] *********\n", lay);
	MSG_DEBUG("%s", "[INFO] ///// screen_rect /////\n");
	MSG_DEBUG("[INFO] rect.x        =%u\n", lyp->rect.x);
	MSG_DEBUG("[INFO] rect.y        =%u\n", lyp->rect.y);
	MSG_DEBUG("[INFO] rect.width    =%u\n", lyp->rect.width);
	MSG_DEBUG("[INFO] rect.height   =%u\n", lyp->rect.height);
	MSG_DEBUG("%s", "[INFO] ///// screen_grap_image_param /////\n");
	MSG_DEBUG("[INFO] width         =%u\n", lyp->image.width);
	MSG_DEBUG("[INFO] height        =%u\n", lyp->image.height);
	MSG_DEBUG("[INFO] stride        =%u\n", lyp->image.stride);
	MSG_DEBUG("[INFO] stride_c      =%u\n", lyp->image.stride_c);
	MSG_DEBUG("[INFO] format        =%u\n", lyp->image.format);
	MSG_DEBUG("[INFO] yuv_format    =%u\n", lyp->image.yuv_format);
	MSG_DEBUG("[INFO] yuv_range     =%u\n", lyp->image.yuv_range);
	MSG_DEBUG("[INFO] address       =0x%p\n", lyp->image.address);
	MSG_DEBUG("[INFO] address_c0    =0x%p\n", lyp->image.address_c0);
	MSG_DEBUG("[INFO] address_c1    =0x%p\n", lyp->image.address_c1);
	MSG_DEBUG("%s", "[INFO] ///// screen_grap_layer /////\n");
	MSG_DEBUG("[INFO] alpha         =%u\n", lyp->alpha);
	MSG_DEBUG("[INFO] rotate        =%u\n", lyp->rotate);
	MSG_DEBUG("[INFO] mirror        =%u\n", lyp->mirror);
	MSG_DEBUG("[INFO] dummy         =%u\n", lyp->dummy);
	MSG_DEBUG("[INFO] key_color     =%ld\n", lyp->key_color);
	MSG_DEBUG("[INFO] premultiplied =%u\n", lyp->premultiplied);
	MSG_DEBUG("[INFO] alpha_coef    =%u\n", lyp->alpha_coef);
	MSG_DEBUG("[INFO] palette       =0x%p\n", lyp->palette);
	MSG_DEBUG("[INFO] palette_size  =%lu\n", lyp->palette_size);
	MSG_DEBUG("[INFO] alpha_plane   =0x%p\n", lyp->alpha_plane);

	lyp = (struct screen_grap_layer *)(&(exe_info->output_layer[lay]));
	MSG_DEBUG("[INFO] ********* output_layer[%d] *********\n", lay);
	MSG_DEBUG("%s", "[INFO] ///// screen_rect /////\n");
	MSG_DEBUG("[INFO] rect.x        =%u\n", lyp->rect.x);
	MSG_DEBUG("[INFO] rect.y        =%u\n", lyp->rect.y);
	MSG_DEBUG("[INFO] rect.width    =%u\n", lyp->rect.width);
	MSG_DEBUG("[INFO] rect.height   =%u\n", lyp->rect.height);
	MSG_DEBUG("%s", "[INFO] ///// screen_grap_image_param /////\n");
	MSG_DEBUG("[INFO] width         =%u\n", lyp->image.width);
	MSG_DEBUG("[INFO] height        =%u\n", lyp->image.height);
	MSG_DEBUG("[INFO] stride        =%u\n", lyp->image.stride);
	MSG_DEBUG("[INFO] stride_c      =%u\n", lyp->image.stride_c);
	MSG_DEBUG("[INFO] format        =%u\n", lyp->image.format);
	MSG_DEBUG("[INFO] yuv_format    =%u\n", lyp->image.yuv_format);
	MSG_DEBUG("[INFO] yuv_range     =%u\n", lyp->image.yuv_range);
	MSG_DEBUG("[INFO] address       =0x%p\n", lyp->image.address);
	MSG_DEBUG("[INFO] address_c0    =0x%p\n", lyp->image.address_c0);
	MSG_DEBUG("[INFO] address_c1    =0x%p\n", lyp->image.address_c1);
	MSG_DEBUG("%s", "[INFO] ///// screen_grap_layer /////\n");
	MSG_DEBUG("[INFO] alpha         =%u\n", lyp->alpha);
	MSG_DEBUG("[INFO] rotate        =%u\n", lyp->rotate);
	MSG_DEBUG("[INFO] mirror        =%u\n", lyp->mirror);
	MSG_DEBUG("[INFO] dummy         =%u\n", lyp->dummy);
	MSG_DEBUG("[INFO] key_color     =%ld\n", lyp->key_color);
	MSG_DEBUG("[INFO] premultiplied =%u\n", lyp->premultiplied);
	MSG_DEBUG("[INFO] alpha_coef    =%u\n", lyp->alpha_coef);
	MSG_DEBUG("[INFO] palette       =0x%p\n", lyp->palette);
	MSG_DEBUG("[INFO] palette_size  =%lu\n", lyp->palette_size);
	MSG_DEBUG("[INFO] alpha_plane   =0x%p\n", lyp->alpha_plane);

	return;
}

/************************************************************************/
/* Function     : imgpctrl_cmn_dbg_ctrl_hdl_param                       */
/* Description  : Print parameter of imgpctrl ctrl handle for DEBUG.    */
/* Return value : None                                                  */
/* Argument     : *ct_hdl - IMGPCTRL control handle pointer             */
/************************************************************************/
void imgpctrl_cmn_dbg_ctrl_hdl_param(const struct t_ctrl_hdl *ct_hdl)
{
	MSG_DEBUG("%s", "[INFO] ********* t_ctrl_hdl *********\n");
	MSG_DEBUG("[INFO] ctrl_th       =0x%p\n", ct_hdl->ctrl_th);
	MSG_DEBUG("[INFO] fw_ctrl_cb_fp =0x%p\n", ct_hdl->fw_ctrl_cb_fp);
	MSG_DEBUG("[INFO] udata         =%lu\n", ct_hdl->udata);
	MSG_DEBUG("[INFO] rq_layer      =%u\n", ct_hdl->rq_layer);
	MSG_DEBUG("[INFO] vspm_hdl      =%lu\n", ct_hdl->vspm_hdl);
	MSG_DEBUG("[INFO] entry_num     =%u\n", ct_hdl->entry_num);
	MSG_DEBUG("[INFO] entry_jobid[0]=%lu\n", ct_hdl->entry_jobid[0]);
	MSG_DEBUG("[INFO] entry_ret[0]  =%ld\n", ct_hdl->entry_ret[0]);
	MSG_DEBUG("[INFO] entry_jobid[1]=%lu\n", ct_hdl->entry_jobid[1]);
	MSG_DEBUG("[INFO] entry_ret[1]  =%ld\n", ct_hdl->entry_ret[1]);
	MSG_DEBUG("[INFO] cb_num        =%u\n", ct_hdl->cb_num);
	MSG_DEBUG("[INFO] cb_jobid[0]   =%lu\n", ct_hdl->cb_jobid[0]);
	MSG_DEBUG("[INFO] cb_ret[0]     =%ld\n", ct_hdl->cb_ret[0]);
	MSG_DEBUG("[INFO] cb_jobid[1]   =%lu\n", ct_hdl->cb_jobid[1]);
	MSG_DEBUG("[INFO] cb_ret[1]     =%ld\n", ct_hdl->cb_ret[1]);
	MSG_DEBUG("[INFO] vspm_param    =0x%p\n", ct_hdl->vspm_param);

	return;
}

/************************************************************************/
/* Function     : imgpctrl_cmn_dbg_tdd_param                            */
/* Description  : Print parameter of vspm tdd for DEBUG.                */
/* Return value : None                                                  */
/* Argument     : *m2d_par - 2D-DMAC mode setting pointer               */
/*                *req_par - 2D-DMAC request setting pointer            */
/************************************************************************/
void imgpctrl_cmn_dbg_tdd_param(
	const T_TDDMAC_MODE *m2d_par,
	const T_TDDMAC_REQUEST *req_par)
{
	MSG_DEBUG("%s", "[INFO] ********* T_TDDMAC_MODE *********\n");
	MSG_DEBUG("[INFO] renewal       =%u\n", m2d_par->renewal);
	MSG_DEBUG("[INFO] resource      =%u\n", m2d_par->resource);
	MSG_DEBUG("[INFO] p_extend      =0x%p\n", m2d_par->p_extend);

	MSG_DEBUG("%s", "[INFO] ********* T_TDDMAC_REQUEST *********\n");
	MSG_DEBUG("[INFO] src_adr       =0x%p\n", req_par->src_adr);
	MSG_DEBUG("[INFO] src_stride    =%u\n", req_par->src_stride);
	MSG_DEBUG("[INFO] src_x_offset  =%u\n", req_par->src_x_offset);
	MSG_DEBUG("[INFO] src_y_offset  =%u\n", req_par->src_y_offset);
	MSG_DEBUG("[INFO] src_format    =%u\n", req_par->src_format);
	MSG_DEBUG("[INFO] ratio         =%u\n", req_par->ratio);
	MSG_DEBUG("[INFO] dst_adr       =0x%p\n", req_par->dst_adr);
	MSG_DEBUG("[INFO] alpha_ena     =%u\n", req_par->alpha_ena);
	MSG_DEBUG("[INFO] alpha         =%u\n", req_par->alpha);
	MSG_DEBUG("[INFO] dst_format    =%u\n", req_par->dst_format);
	MSG_DEBUG("[INFO] dst_stride    =%u\n", req_par->dst_stride);
	MSG_DEBUG("[INFO] dst_x_offset  =%u\n", req_par->dst_x_offset);
	MSG_DEBUG("[INFO] dst_y_offset  =%u\n", req_par->dst_y_offset);
	MSG_DEBUG("[INFO] dst_width     =%u\n", req_par->dst_width);
	MSG_DEBUG("[INFO] dst_height    =%u\n", req_par->dst_height);
	MSG_DEBUG("[INFO] cb_finished   =0x%p\n", req_par->cb_finished);
	MSG_DEBUG("[INFO] userdata      =0x%p\n", req_par->userdata);
	MSG_DEBUG("[INFO] swap          =0x%08lX\n", req_par->swap);
	MSG_DEBUG("[INFO] mirror        =%u\n", req_par->mirror);
	MSG_DEBUG("[INFO] rotation      =%u\n", req_par->rotation);

	return;
}

/************************************************************************/
/* Function     : imgpctrl_cmn_dbg_vsp_param                            */
/* Description  : Print parameter of vspm tdd for DEBUG.                */
/* Return value : None                                                  */
/************************************************************************/
void imgpctrl_cmn_dbg_vsp_param(struct t_vspm_vsp *vspm_ptr,
	u_short req_lay, u_short uds_num, u_short rq_type)
{

	T_VSP_IN       *src_ptr;
	T_VSP_OUT      *dst_ptr;
	T_VSP_ALPHA *alpha;
	int	i;
	VSPM_VSP_PAR *p_vsp;
	T_VSP_BRU *bru;
	T_VSP_BLEND_VIRTUAL *b_virtual;
	T_VSP_BLEND_CONTROL *b_cntl;
	T_VSP_UDS *uds_param;

	p_vsp = &(vspm_ptr->p_vsp);

	MSG_DEBUG("%s", "[INFO] ********* VSPM SETTING PARAM*********\n");
	MSG_DEBUG("%s", "[INFO] ********* T_VSP_START *********\n");
	MSG_DEBUG("[INFO]uhType  %x\n", vspm_ptr->vspm_ip.uhType);
	MSG_DEBUG("[INFO]rpf_order %lx\n", p_vsp->rpf_order);
	MSG_DEBUG("[INFO]rpf_num  %x\n", p_vsp->rpf_num);
	MSG_DEBUG("[INFO]use_module %lx\n", p_vsp->use_module);

	src_ptr = p_vsp->src1_par;
	if (src_ptr != (T_VSP_IN *)NULL) {
		MSG_DEBUG("%s", "[INFO] ********* T_VSP_IN src1 *********\n");
		MSG_DEBUG("[INFO] in addr      =0x%p\n", src_ptr->addr);
		MSG_DEBUG("[INFO] in addr_c0   =0x%p\n", src_ptr->addr_c0);
		MSG_DEBUG("[INFO] in addr_c1   =0x%p\n", src_ptr->addr_c1);
		MSG_DEBUG("[INFO] in stride    =%u\n", src_ptr->stride);
		MSG_DEBUG("[INFO] in width     =%u\n", src_ptr->width);
		MSG_DEBUG("[INFO] in height    =%u\n", src_ptr->height);
		MSG_DEBUG("[INFO] in width_ex  =%u\n", src_ptr->width_ex);
		MSG_DEBUG("[INFO] in height_ex =%u\n", src_ptr->height_ex);
		MSG_DEBUG("[INFO] in x_offset  =%u\n", src_ptr->x_offset);
		MSG_DEBUG("[INFO] in y_offset  =%u\n", src_ptr->y_offset);
		MSG_DEBUG("[INFO] in format    =%u\n", src_ptr->format);
		MSG_DEBUG("[INFO] in swap      =%u\n", src_ptr->swap);
		MSG_DEBUG("[INFO] in x_position=%u\n", src_ptr->x_position);
		MSG_DEBUG("[INFO] in y_position=%u\n", src_ptr->y_position);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in cipm      =%u\n", src_ptr->cipm);
		MSG_DEBUG("[INFO] in cext      =%u\n", src_ptr->cext);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in csc       =%u\n", src_ptr->csc);
		MSG_DEBUG("[INFO] in itbrbt(yuv_fotmat)=%u\n", src_ptr->iturbt);
		MSG_DEBUG("[INFO] in clrcng(yuv_range) =%u\n", src_ptr->clrcng);
		MSG_DEBUG("[INFO] in vir       =%u\n", src_ptr->vir);
		MSG_DEBUG("[INFO] in vir color =%lu\n", src_ptr->vircolor);
		MSG_DEBUG("[INFO] in osd lut   =0x%p\n", src_ptr->osd_lut);

		alpha = p_vsp->src1_par->alpha_blend;
		MSG_DEBUG("[INFO] in alpha addr    =0x%p\n", alpha->addr_a);
		MSG_DEBUG("[INFO] in alpha alphan  =%u\n", alpha->alphan);
		MSG_DEBUG("[INFO] in alpha alpha1  =%lu\n", alpha->alpha1);
		MSG_DEBUG("[INFO] in alpha alpha2  =%lu\n", alpha->alpha2);
		MSG_DEBUG("[INFO] in alpha astride =%u\n", alpha->astride);
		MSG_DEBUG("[INFO] in alpha aswap   =%u\n", alpha->aswap);
		MSG_DEBUG("[INFO] in alpha asel    =%u\n", alpha->asel);
		MSG_DEBUG("[INFO] in alpha aext    =%u\n", alpha->aext);
		MSG_DEBUG("[INFO] in alpha anum0   =%u\n", alpha->anum0);
		MSG_DEBUG("[INFO] in alpha anum1   =%u\n", alpha->anum1);
		MSG_DEBUG("[INFO] in alpha afix    =%u\n", alpha->afix);
		MSG_DEBUG("[INFO] in alpha irop    =%u\n", alpha->irop);
		MSG_DEBUG("[INFO] in alpha msken   =%u\n", alpha->msken);
		MSG_DEBUG("[INFO] in alpha bsel    =%u\n", alpha->bsel);
		MSG_DEBUG("[INFO] in alpha mgcolor =%lu\n", alpha->mgcolor);
		MSG_DEBUG("[INFO] in alpha mscolor0=%lu\n", alpha->mscolor0);
		MSG_DEBUG("[INFO] in alpha mscolor1=%lu\n", alpha->mscolor1);

		MSG_DEBUG("[INFO] in clrcnv(INDEX8)=0x%p\n", src_ptr->clrcnv);
		MSG_DEBUG("[INFO] in connect = 0x%08lX\n", src_ptr->connect);
	}

	src_ptr = p_vsp->src2_par;
	if ((req_lay >= 2) && (src_ptr != (T_VSP_IN *)NULL)) {
		MSG_DEBUG("%s", "[INFO] ********* T_VSP_IN src2 *********\n");
		MSG_DEBUG("[INFO] in addr      =0x%p\n", src_ptr->addr);
		MSG_DEBUG("[INFO] in addr_c0   =0x%p\n", src_ptr->addr_c0);
		MSG_DEBUG("[INFO] in addr_c1   =0x%p\n", src_ptr->addr_c1);
		MSG_DEBUG("[INFO] in stride    =%u\n", src_ptr->stride);
		MSG_DEBUG("[INFO] in width     =%u\n", src_ptr->width);
		MSG_DEBUG("[INFO] in height    =%u\n", src_ptr->height);
		MSG_DEBUG("[INFO] in width_ex  =%u\n", src_ptr->width_ex);
		MSG_DEBUG("[INFO] in height_ex =%u\n", src_ptr->height_ex);
		MSG_DEBUG("[INFO] in x_offset  =%u\n", src_ptr->x_offset);
		MSG_DEBUG("[INFO] in y_offset  =%u\n", src_ptr->y_offset);
		MSG_DEBUG("[INFO] in format    =%u\n", src_ptr->format);
		MSG_DEBUG("[INFO] in swap      =%u\n", src_ptr->swap);
		MSG_DEBUG("[INFO] in x_position=%u\n", src_ptr->x_position);
		MSG_DEBUG("[INFO] in y_position=%u\n", src_ptr->y_position);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in cipm      =%u\n", src_ptr->cipm);
		MSG_DEBUG("[INFO] in cext      =%u\n", src_ptr->cext);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in csc       =%u\n", src_ptr->csc);
		MSG_DEBUG("[INFO] in itbrbt(yuv_fotmat)=%u\n",
			 src_ptr->iturbt);
		MSG_DEBUG("[INFO] in clrcng(yuv_range) =%u\n",
			 src_ptr->clrcng);
		MSG_DEBUG("[INFO] in vir       =%u\n", src_ptr->vir);
		MSG_DEBUG("[INFO] in vir color =%lu\n", src_ptr->vircolor);
		MSG_DEBUG("[INFO] in osd lut   =0x%p\n", src_ptr->osd_lut);

		alpha = p_vsp->src2_par->alpha_blend;
		MSG_DEBUG("[INFO] in alpha addr    =0x%p\n", alpha->addr_a);
		MSG_DEBUG("[INFO] in alpha alphan  =%u\n", alpha->alphan);
		MSG_DEBUG("[INFO] in alpha alpha1  =%lu\n", alpha->alpha1);
		MSG_DEBUG("[INFO] in alpha alpha2  =%lu\n", alpha->alpha2);
		MSG_DEBUG("[INFO] in alpha astride =%u\n", alpha->astride);
		MSG_DEBUG("[INFO] in alpha aswap   =%u\n", alpha->aswap);
		MSG_DEBUG("[INFO] in alpha asel    =%u\n", alpha->asel);
		MSG_DEBUG("[INFO] in alpha aext    =%u\n", alpha->aext);
		MSG_DEBUG("[INFO] in alpha anum0   =%u\n", alpha->anum0);
		MSG_DEBUG("[INFO] in alpha anum1   =%u\n", alpha->anum1);
		MSG_DEBUG("[INFO] in alpha afix    =%u\n", alpha->afix);
		MSG_DEBUG("[INFO] in alpha irop    =%u\n", alpha->irop);
		MSG_DEBUG("[INFO] in alpha msken   =%u\n", alpha->msken);
		MSG_DEBUG("[INFO] in alpha bsel    =%u\n", alpha->bsel);
		MSG_DEBUG("[INFO] in alpha mgcolor =%lu\n", alpha->mgcolor);
		MSG_DEBUG("[INFO] in alpha mscolor0=%lu\n", alpha->mscolor0);
		MSG_DEBUG("[INFO] in alpha mscolor1=%lu\n", alpha->mscolor1);

		MSG_DEBUG("[INFO] in clrcnv(INDEX8)=0x%p\n", src_ptr->clrcnv);
		MSG_DEBUG("[INFO] in connect = 0x%08lX\n", src_ptr->connect);
	}

	src_ptr = p_vsp->src3_par;
	if ((req_lay >= 3) && (src_ptr != (T_VSP_IN *)NULL)) {
		MSG_DEBUG("%s", "[INFO] ********* T_VSP_IN src3 *********\n");
		MSG_DEBUG("[INFO] in addr      =0x%p\n", src_ptr->addr);
		MSG_DEBUG("[INFO] in addr_c0   =0x%p\n", src_ptr->addr_c0);
		MSG_DEBUG("[INFO] in addr_c1   =0x%p\n", src_ptr->addr_c1);
		MSG_DEBUG("[INFO] in stride    =%u\n", src_ptr->stride);
		MSG_DEBUG("[INFO] in width     =%u\n", src_ptr->width);
		MSG_DEBUG("[INFO] in height    =%u\n", src_ptr->height);
		MSG_DEBUG("[INFO] in width_ex  =%u\n", src_ptr->width_ex);
		MSG_DEBUG("[INFO] in height_ex =%u\n", src_ptr->height_ex);
		MSG_DEBUG("[INFO] in x_offset  =%u\n", src_ptr->x_offset);
		MSG_DEBUG("[INFO] in y_offset  =%u\n", src_ptr->y_offset);
		MSG_DEBUG("[INFO] in format    =%u\n", src_ptr->format);
		MSG_DEBUG("[INFO] in swap      =%u\n", src_ptr->swap);
		MSG_DEBUG("[INFO] in x_position=%u\n", src_ptr->x_position);
		MSG_DEBUG("[INFO] in y_position=%u\n", src_ptr->y_position);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in cipm      =%u\n", src_ptr->cipm);
		MSG_DEBUG("[INFO] in cext      =%u\n", src_ptr->cext);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in csc       =%u\n", src_ptr->csc);
		MSG_DEBUG("[INFO] in itbrbt(yuv_fotmat)=%u\n", src_ptr->iturbt);
		MSG_DEBUG("[INFO] in clrcng(yuv_range) =%u\n", src_ptr->clrcng);
		MSG_DEBUG("[INFO] in vir       =%u\n", src_ptr->vir);
		MSG_DEBUG("[INFO] in vir color =%lu\n", src_ptr->vircolor);
		MSG_DEBUG("[INFO] in osd lut   =0x%p\n", src_ptr->osd_lut);

		alpha = p_vsp->src3_par->alpha_blend;
		MSG_DEBUG("[INFO] in alpha addr    =0x%p\n", alpha->addr_a);
		MSG_DEBUG("[INFO] in alpha alphan  =%u\n", alpha->alphan);
		MSG_DEBUG("[INFO] in alpha alpha1  =%lu\n", alpha->alpha1);
		MSG_DEBUG("[INFO] in alpha alpha2  =%lu\n", alpha->alpha2);
		MSG_DEBUG("[INFO] in alpha astride =%u\n", alpha->astride);
		MSG_DEBUG("[INFO] in alpha aswap   =%u\n", alpha->aswap);
		MSG_DEBUG("[INFO] in alpha asel    =%u\n", alpha->asel);
		MSG_DEBUG("[INFO] in alpha aext    =%u\n", alpha->aext);
		MSG_DEBUG("[INFO] in alpha anum0   =%u\n", alpha->anum0);
		MSG_DEBUG("[INFO] in alpha anum1   =%u\n", alpha->anum1);
		MSG_DEBUG("[INFO] in alpha afix    =%u\n", alpha->afix);
		MSG_DEBUG("[INFO] in alpha irop    =%u\n", alpha->irop);
		MSG_DEBUG("[INFO] in alpha msken   =%u\n", alpha->msken);
		MSG_DEBUG("[INFO] in alpha bsel    =%u\n", alpha->bsel);
		MSG_DEBUG("[INFO] in alpha mgcolor =%lu\n", alpha->mgcolor);
		MSG_DEBUG("[INFO] in alpha mscolor0=%lu\n", alpha->mscolor0);
		MSG_DEBUG("[INFO] in alpha mscolor1=%lu\n", alpha->mscolor1);

		MSG_DEBUG("[INFO] in clrcnv(INDEX8)=0x%p\n", src_ptr->clrcnv);
		MSG_DEBUG("[INFO] in connect = 0x%08lX\n", src_ptr->connect);
	}

	src_ptr = p_vsp->src4_par;
	if ((req_lay >= 4) && (src_ptr != (T_VSP_IN *)NULL)) {
		MSG_DEBUG("%s", "[INFO] ********* T_VSP_IN src4 *********\n");
		MSG_DEBUG("[INFO] in addr      =0x%p\n", src_ptr->addr);
		MSG_DEBUG("[INFO] in addr_c0   =0x%p\n", src_ptr->addr_c0);
		MSG_DEBUG("[INFO] in addr_c1   =0x%p\n", src_ptr->addr_c1);
		MSG_DEBUG("[INFO] in stride    =%u\n", src_ptr->stride);
		MSG_DEBUG("[INFO] in width     =%u\n", src_ptr->width);
		MSG_DEBUG("[INFO] in height    =%u\n", src_ptr->height);
		MSG_DEBUG("[INFO] in width_ex  =%u\n", src_ptr->width_ex);
		MSG_DEBUG("[INFO] in height_ex =%u\n", src_ptr->height_ex);
		MSG_DEBUG("[INFO] in x_offset  =%u\n", src_ptr->x_offset);
		MSG_DEBUG("[INFO] in y_offset  =%u\n", src_ptr->y_offset);
		MSG_DEBUG("[INFO] in format    =%u\n", src_ptr->format);
		MSG_DEBUG("[INFO] in swap      =%u\n", src_ptr->swap);
		MSG_DEBUG("[INFO] in x_position=%u\n", src_ptr->x_position);
		MSG_DEBUG("[INFO] in y_position=%u\n", src_ptr->y_position);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in cipm      =%u\n", src_ptr->cipm);
		MSG_DEBUG("[INFO] in cext      =%u\n", src_ptr->cext);
		MSG_DEBUG("[INFO] in pwd       =%u\n", src_ptr->pwd);
		MSG_DEBUG("[INFO] in csc       =%u\n", src_ptr->csc);
		MSG_DEBUG("[INFO] in itbrbt(yuv_fotmat)=%u\n", src_ptr->iturbt);
		MSG_DEBUG("[INFO] in clrcng(yuv_range) =%u\n", src_ptr->clrcng);
		MSG_DEBUG("[INFO] in vir       =%u\n", src_ptr->vir);
		MSG_DEBUG("[INFO] in vir color =%lu\n", src_ptr->vircolor);
		MSG_DEBUG("[INFO] in osd lut   =0x%p\n", src_ptr->osd_lut);

		alpha = p_vsp->src4_par->alpha_blend;
		MSG_DEBUG("[INFO] in alpha addr    =0x%p\n", alpha->addr_a);
		MSG_DEBUG("[INFO] in alpha alphan  =%u\n", alpha->alphan);
		MSG_DEBUG("[INFO] in alpha alpha1  =%lu\n", alpha->alpha1);
		MSG_DEBUG("[INFO] in alpha alpha2  =%lu\n", alpha->alpha2);
		MSG_DEBUG("[INFO] in alpha astride =%u\n", alpha->astride);
		MSG_DEBUG("[INFO] in alpha aswap   =%u\n", alpha->aswap);
		MSG_DEBUG("[INFO] in alpha asel    =%u\n", alpha->asel);
		MSG_DEBUG("[INFO] in alpha aext    =%u\n", alpha->aext);
		MSG_DEBUG("[INFO] in alpha anum0   =%u\n", alpha->anum0);
		MSG_DEBUG("[INFO] in alpha anum1   =%u\n", alpha->anum1);
		MSG_DEBUG("[INFO] in alpha afix    =%u\n", alpha->afix);
		MSG_DEBUG("[INFO] in alpha irop    =%u\n", alpha->irop);
		MSG_DEBUG("[INFO] in alpha msken   =%u\n", alpha->msken);
		MSG_DEBUG("[INFO] in alpha bsel    =%u\n", alpha->bsel);
		MSG_DEBUG("[INFO] in alpha mgcolor =%lu\n", alpha->mgcolor);
		MSG_DEBUG("[INFO] in alpha mscolor0=%lu\n", alpha->mscolor0);
		MSG_DEBUG("[INFO] in alpha mscolor1=%lu\n", alpha->mscolor1);

		MSG_DEBUG("[INFO] in clrcnv(INDEX8)=0x%p\n", src_ptr->clrcnv);
		MSG_DEBUG("[INFO] in connect = 0x%08lX\n", src_ptr->connect);
	}

	if ((rq_type & IMGPCTRL_REQ_BLEND) == IMGPCTRL_REQ_BLEND) {
		bru = (p_vsp->ctrl_par->bru);

		MSG_DEBUG("%s", "[INFO] ********* T_VSP_CTRL-BRU *********\n");
		MSG_DEBUG("[INFO]bru lay_order %x\n",
			 (u_int)bru->lay_order);
		MSG_DEBUG("[INFO]bru adiv %x\n", (u_int)bru->adiv);
		for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
			MSG_DEBUG("[INFO]bru qnt[%d] %x\n", i,
				 (u_int)bru->qnt[i]);
			MSG_DEBUG("[INFO]bru dith[%d] %x\n", i,
				 (u_int)bru->dith[i]);
		}
		MSG_DEBUG("[INFO] bru connect = 0x%08lX\n", bru->connect);

		b_virtual = (p_vsp->ctrl_par->bru->blend_virtual);

		MSG_DEBUG("%s",
			"[INFO] ********* T_VSP_BLEND_VIRTUAL *********\n");
		MSG_DEBUG("[INFO]virtual width =%d\n", b_virtual->width);
		MSG_DEBUG("[INFO]virtual height = %d\n", b_virtual->height);
		MSG_DEBUG("[INFO]virtual x position = %d\n",
			 b_virtual->x_position);
		MSG_DEBUG("[INFO]virtual y position = %d\n",
			 b_virtual->y_position);
		MSG_DEBUG("[INFO]virtual pwd = %d\n", b_virtual->pwd);
		MSG_DEBUG("[INFO]background color = 0x%08lx\n",
			 b_virtual->color);

		b_cntl = (p_vsp->ctrl_par->bru->blend_control_a);
		if (b_cntl != (T_VSP_BLEND_CONTROL *)NULL) {

			MSG_DEBUG("%s",
				"[INFO] ********* BLEND_CONTROL_A *******\n");

			MSG_DEBUG("[INFO] rbc  =%u\n", b_cntl->rbc);
			MSG_DEBUG("[INFO] crop  =%x\n", b_cntl->crop);
			MSG_DEBUG("[INFO] arop  =%x\n", b_cntl->arop);
			MSG_DEBUG("[INFO] blend formula  =%x\n",
				b_cntl->blend_formula);
			MSG_DEBUG("[INFO] blend coefx  =%x\n",
				b_cntl->blend_coefx);
			MSG_DEBUG("[INFO] blend coefy  =%x\n",
				b_cntl->blend_coefy);
			MSG_DEBUG("[INFO] aformula  =%x\n", b_cntl->aformula);
			MSG_DEBUG("[INFO] acoefx  =%x\n", b_cntl->acoefx);
			MSG_DEBUG("[INFO] acoefy  =%x\n", b_cntl->acoefy);
			MSG_DEBUG("[INFO] acoefx_fix  =%x\n",
				b_cntl->acoefx_fix);
			MSG_DEBUG("[INFO] acoefy_fix  =%x\n",
				b_cntl->acoefy_fix);

			MSG_DEBUG("[INFO]blend rop CROP = %d\n",
				bru->blend_rop->crop);
			MSG_DEBUG("[INFO]blend rop AROP = %d\n",
				bru->blend_rop->arop);
		}

		b_cntl = (p_vsp->ctrl_par->bru->blend_control_b);
		if ((req_lay >= 2) &&
			(b_cntl != (T_VSP_BLEND_CONTROL *)NULL)) {
			MSG_DEBUG("%s",
				"[INFO] ********* BLEND_CONTROL_B *****\n");

			MSG_DEBUG("[INFO] rbc  =%u\n", b_cntl->rbc);
			MSG_DEBUG("[INFO] crop  =%x\n", b_cntl->crop);
			MSG_DEBUG("[INFO] arop  =%x\n", b_cntl->arop);
			MSG_DEBUG("[INFO] blend formula  =%x\n",
				b_cntl->blend_formula);
			MSG_DEBUG("[INFO] blend coefx  =%x\n",
				 b_cntl->blend_coefx);
			MSG_DEBUG("[INFO] blend coefy  =%x\n",
				 b_cntl->blend_coefy);
			MSG_DEBUG("[INFO] aformula  =%x\n", b_cntl->aformula);
			MSG_DEBUG("[INFO] acoefx  =%x\n",
				 b_cntl->acoefx);
			MSG_DEBUG("[INFO] acoefy  =%x\n",
				 b_cntl->acoefy);
			MSG_DEBUG("[INFO] acoefx_fix  =%x\n",
				 b_cntl->acoefx_fix);
			MSG_DEBUG("[INFO] acoefy_fix  =%x\n"
				, b_cntl->acoefy_fix);

			MSG_DEBUG("[INFO]blend rop CROP = %d\n",
				bru->blend_rop->crop);
			MSG_DEBUG("[INFO]blend rop AROP = %d\n",
				 bru->blend_rop->arop);
		}

		b_cntl = (p_vsp->ctrl_par->bru->blend_control_c);
		if ((req_lay >= 3) &&
			(b_cntl != (T_VSP_BLEND_CONTROL *)NULL)) {
			MSG_DEBUG("%s",
				"[INFO] ******* BLEND_CONTROL_C *******\n");

			MSG_DEBUG("[INFO] rbc  =%u\n", b_cntl->rbc);
			MSG_DEBUG("[INFO] crop  =%x\n", b_cntl->crop);
			MSG_DEBUG("[INFO] arop  =%x\n", b_cntl->arop);
			MSG_DEBUG("[INFO] blend formula  =%x\n",
				 b_cntl->blend_formula);
			MSG_DEBUG("[INFO] blend coefx  =%x\n",
				 b_cntl->blend_coefx);
			MSG_DEBUG("[INFO] blend coefy  =%x\n"
				, b_cntl->blend_coefy);
			MSG_DEBUG("[INFO] aformula  =%x\n", b_cntl->aformula);
			MSG_DEBUG("[INFO] acoefx  =%x\n",
				 b_cntl->acoefx);
			MSG_DEBUG("[INFO] acoefy  =%x\n",
				 b_cntl->acoefy);
			MSG_DEBUG("[INFO] acoefx_fix  =%x\n",
				 b_cntl->acoefx_fix);
			MSG_DEBUG("[INFO] acoefy_fix  =%x\n",
				 b_cntl->acoefy_fix);

			MSG_DEBUG("[INFO]blend rop CROP = %d\n",
				 bru->blend_rop->crop);
			MSG_DEBUG("[INFO]blend rop AROP = %d\n",
				 bru->blend_rop->arop);
		}

		if ((req_lay >= 4) &&
			(b_cntl != (T_VSP_BLEND_CONTROL *)NULL)) {
			MSG_DEBUG("%s",
				"[INFO] ******* BLEND_CONTROL_D *******\n");
			b_cntl = (p_vsp->ctrl_par->bru->blend_control_d);

			MSG_DEBUG("[INFO] rbc  =%u\n", b_cntl->rbc);
			MSG_DEBUG("[INFO] crop  =%x\n", b_cntl->crop);
			MSG_DEBUG("[INFO] arop  =%x\n", b_cntl->arop);
			MSG_DEBUG("[INFO] blend formula  =%x\n",
				 b_cntl->blend_formula);
			MSG_DEBUG("[INFO] blend coefx  =%x\n",
				 b_cntl->blend_coefx);
			MSG_DEBUG("[INFO] blend coefy  =%x\n",
				 b_cntl->blend_coefy);
			MSG_DEBUG("[INFO] aformula  =%x\n", b_cntl->aformula);
			MSG_DEBUG("[INFO] acoefx  =%x\n", b_cntl->acoefx);
			MSG_DEBUG("[INFO] acoefy  =%x\n", b_cntl->acoefy);
			MSG_DEBUG("[INFO] acoefx_fix  =%x\n",
				 b_cntl->acoefx_fix);
			MSG_DEBUG("[INFO] acoefy_fix  =%x\n",
				 b_cntl->acoefy_fix);

			MSG_DEBUG("[INFO]blend rop CROP = %d\n",
				 bru->blend_rop->crop);
			MSG_DEBUG("[INFO]blend rop AROP = %d\n",
				 bru->blend_rop->arop);
		}
	}

	if ((rq_type & IMGPCTRL_REQ_RESIZE) == IMGPCTRL_REQ_RESIZE) {

		MSG_DEBUG("%s", "[INFO] ********* T_VSP_UDS *********\n");
		uds_param = (p_vsp->ctrl_par->uds);

		MSG_DEBUG("[INFO] amd  =%d\n", uds_param->amd);
		MSG_DEBUG("[INFO] fmd  =%x\n", uds_param->fmd);
		MSG_DEBUG("[INFO] filcolor  =%x\n",
			 (u_int)uds_param->filcolor);
		MSG_DEBUG("[INFO] clip  =%x\n", uds_param->clip);
		MSG_DEBUG("[INFO] alpha  =%x\n", uds_param->alpha);
		MSG_DEBUG("[INFO] complement  =%x\n",
			 uds_param->complement);
		MSG_DEBUG("[INFO] athres0  =%x\n", uds_param->athres0);
		MSG_DEBUG("[INFO] athres1  =%x\n", uds_param->athres1);
		MSG_DEBUG("[INFO] anum0  =%x\n", uds_param->anum0);
		MSG_DEBUG("[INFO] anum1  =%x\n", uds_param->anum1);
		MSG_DEBUG("[INFO] anum2  =%x\n", uds_param->anum2);
		MSG_DEBUG("[INFO] x_ratio =%x\n", uds_param->x_ratio);
		MSG_DEBUG("[INFO] y_ratio  =%x\n", uds_param->y_ratio);
		MSG_DEBUG("[INFO] after resize width  =%x\n",
			 uds_param->out_cwidth);
		MSG_DEBUG("[INFO] after resize height  =%x\n",
			 uds_param->out_cheight);
		MSG_DEBUG("[INFO] connect  =%x\n",
			 (u_int)uds_param->connect);


		if (uds_num >= 2) {
			MSG_DEBUG("%s",
			"[INFO] ********* T_VSP_UDS 1*********\n");
			uds_param = (p_vsp->ctrl_par->uds1);

			MSG_DEBUG("[INFO] amd  =%d\n", uds_param->amd);
			MSG_DEBUG("[INFO] fmd  =%x\n", uds_param->fmd);
			MSG_DEBUG("[INFO] filcolor  =%x\n",
				(u_int)uds_param->filcolor);
			MSG_DEBUG("[INFO] clip  =%x\n", uds_param->clip);
			MSG_DEBUG("[INFO] alpha  =%x\n", uds_param->alpha);
			MSG_DEBUG("[INFO] complement  =%x\n",
				 uds_param->complement);
			MSG_DEBUG("[INFO] athres0  =%x\n", uds_param->athres0);
			MSG_DEBUG("[INFO] athres1  =%x\n", uds_param->athres1);
			MSG_DEBUG("[INFO] anum0  =%x\n", uds_param->anum0);
			MSG_DEBUG("[INFO] anum1  =%x\n", uds_param->anum1);
			MSG_DEBUG("[INFO] anum2  =%x\n", uds_param->anum2);
			MSG_DEBUG("[INFO] x_ratio =%x\n", uds_param->x_ratio);
			MSG_DEBUG("[INFO] y_ratio  =%x\n", uds_param->y_ratio);
			MSG_DEBUG("[INFO] after resize width  =%x\n",
				 uds_param->out_cwidth);
			MSG_DEBUG("[INFO] after resize height  =%x\n",
				 uds_param->out_cheight);
			MSG_DEBUG("[INFO] connect  =%x\n",
				 (u_int)uds_param->connect);
		}

		if (uds_num >= 3) {
			MSG_DEBUG("%s",
				"[INFO] ********* T_VSP_UDS 2*********\n");
			uds_param = (p_vsp->ctrl_par->uds2);

			MSG_DEBUG("[INFO] amd  =%d\n", uds_param->amd);
			MSG_DEBUG("[INFO] fmd  =%x\n", uds_param->fmd);
			MSG_DEBUG("[INFO] filcolor  =%x\n",
				 (u_int)uds_param->filcolor);
			MSG_DEBUG("[INFO] clip  =%x\n", uds_param->clip);
			MSG_DEBUG("[INFO] alpha  =%x\n", uds_param->alpha);
			MSG_DEBUG("[INFO] complement  =%x\n",
				 uds_param->complement);
			MSG_DEBUG("[INFO] athres0  =%x\n", uds_param->athres0);
			MSG_DEBUG("[INFO] athres1  =%x\n", uds_param->athres1);
			MSG_DEBUG("[INFO] anum0  =%x\n", uds_param->anum0);
			MSG_DEBUG("[INFO] anum1  =%x\n", uds_param->anum1);
			MSG_DEBUG("[INFO] anum2  =%x\n", uds_param->anum2);
			MSG_DEBUG("[INFO] x_ratio =%x\n", uds_param->x_ratio);
			MSG_DEBUG("[INFO] y_ratio  =%x\n", uds_param->y_ratio);
			MSG_DEBUG("[INFO] after resize width  =%x\n",
				 uds_param->out_cwidth);
			MSG_DEBUG("[INFO] after resize height  =%x\n",
				 uds_param->out_cheight);
			MSG_DEBUG("[INFO] connect  =%x\n",
				 (u_int)uds_param->connect);
		}
	}

	dst_ptr =  &(vspm_ptr->dst_par);

	MSG_DEBUG("%s", "[INFO] ********* T_VSP_OUT *********\n");
	MSG_DEBUG("[INFO] out addr     =0x%p\n", dst_ptr->addr);
	MSG_DEBUG("[INFO] out addr_c0  =0x%p\n", dst_ptr->addr_c0);
	MSG_DEBUG("[INFO] out addr_c1  =0x%p\n", dst_ptr->addr_c1);
	MSG_DEBUG("[INFO] out stride   =%u\n", dst_ptr->stride);
	MSG_DEBUG("[INFO] out width    =%u\n", dst_ptr->width);
	MSG_DEBUG("[INFO] out height   =%u\n", dst_ptr->height);
	MSG_DEBUG("[INFO] out x_offset =%u\n", dst_ptr->x_offset);
	MSG_DEBUG("[INFO] out y_offset =%u\n", dst_ptr->y_offset);
	MSG_DEBUG("[INFO] out format   =%u\n", dst_ptr->format);
	MSG_DEBUG("[INFO] out swap     =%u\n", dst_ptr->swap);
	MSG_DEBUG("[INFO] out pxa      =%u\n", dst_ptr->pxa);
	MSG_DEBUG("[INFO] out pad      =%u\n", dst_ptr->pad);
	MSG_DEBUG("[INFO] out x_coffset=%u\n", dst_ptr->x_coffset);
	MSG_DEBUG("[INFO] out y_coffset=%u\n", dst_ptr->y_coffset);
	MSG_DEBUG("[INFO] out csc      =%u\n", dst_ptr->csc);
	MSG_DEBUG("[INFO] out itbrbt(yuv_fotmat)=%u\n", dst_ptr->iturbt);
	MSG_DEBUG("[INFO] out clrcng(yuv_range) =%u\n", dst_ptr->clrcng);
	MSG_DEBUG("[INFO] out cbrm     =%u\n", dst_ptr->cbrm);
	MSG_DEBUG("[INFO] out abrm     =%u\n", dst_ptr->abrm);
	MSG_DEBUG("[INFO] out athres   =%u\n", dst_ptr->athres);
	MSG_DEBUG("[INFO] out clmd     =%u\n", dst_ptr->clmd);
	MSG_DEBUG("[INFO] out dith     =%u\n", dst_ptr->dith);

	return;
}

void imgpctrl_graphdl_log(struct t_exe_info *exe_info ,
			struct screen_grap_handle *bl_hdl)
{
	int i, j, k;
	struct screen_grap_layer  *lyp;
	struct screen_grap_image_param *oimg;
	struct t_chain_info	*chainp;
	struct t_memory_info	*memp;
	struct t_ext_layer	*extp;
	struct t_trace_info	*tinfp;
	struct screen_grap_handle *sghp;

	if (bl_hdl->log_start_ptr == (u_long *)NULL) {
		MSG_ERROR("[%s] ERR: error_log cannot write. Line:%d\n",
			__func__, __LINE__);
		return;
	}

	tinfp = (struct t_trace_info *)bl_hdl->log_start_ptr;
	if ((tinfp->err_hdl_add) != (u_long *)NULL) {
		sghp = (struct screen_grap_handle *)(tinfp->err_hdl_add);
		(void)memcpy(sghp, bl_hdl, sizeof(struct screen_grap_handle));
	}
	MSG_DEBUG("[DBG] grap_hdl RT_GRAPHICS_TRACE_ID = %d\n",
		RT_GRAPHICS_TRACE_ID);
	MSG_DEBUG("%s", "[IMGPCTRL ERROR LOG HDL Detail]");

	MSG_DEBUG("%s", " ********* bl_handle param *********\n");
	MSG_DEBUG(" layer num = %d\n", bl_hdl->layer_num);
	MSG_DEBUG(" layer 1st part bit = %d\n",
		bl_hdl->layer_1st_part_bit);
	MSG_DEBUG(" layer blend bit = %d\n", bl_hdl->layer_blend_bit);
	MSG_DEBUG(" blned req flag = %d\n", bl_hdl->blend_req_flag);
	MSG_DEBUG(" bg color = 0x%08lx\n", bl_hdl->bg_color);
	MSG_DEBUG(" user data = 0x%08lx\n", bl_hdl->user_data);


	MSG_DEBUG("%s", " ********* bl_handle layer param *********\n");
	for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
		lyp = (struct screen_grap_layer  *)(&(bl_hdl->input_layer[i]));
		MSG_DEBUG("****** input_layer[%d] from API = %p******\n",
			i, lyp);
		MSG_DEBUG("%s", " ///// screen_rect /////\n");
		MSG_DEBUG(" rect.x        =%u\n", lyp->rect.x);
		MSG_DEBUG(" rect.y        =%u\n", lyp->rect.y);
		MSG_DEBUG(" rect.width    =%u\n", lyp->rect.width);
		MSG_DEBUG(" rect.height   =%u\n", lyp->rect.height);
		MSG_DEBUG("%s", " ///// screen_grap_image_param /////\n");
		MSG_DEBUG(" width         =%u\n", lyp->image.width);
		MSG_DEBUG(" height        =%u\n", lyp->image.height);
		MSG_DEBUG(" stride        =%u\n", lyp->image.stride);
		MSG_DEBUG(" stride_c      =%u\n", lyp->image.stride_c);
		MSG_DEBUG(" format        =%u\n", lyp->image.format);
		MSG_DEBUG(" yuv_format    =%u\n", lyp->image.yuv_format);
		MSG_DEBUG(" yuv_range     =%u\n", lyp->image.yuv_range);
		MSG_DEBUG(" address       =0x%p\n", lyp->image.address);
		MSG_DEBUG(" address_c0    =0x%p\n", lyp->image.address_c0);
		MSG_DEBUG(" address_c1    =0x%p\n", lyp->image.address_c1);

		MSG_DEBUG("%s", " ///// screen_grap_layer /////\n");
		MSG_DEBUG(" alpha         =%u\n", lyp->alpha);
		MSG_DEBUG(" rotate        =%u\n", lyp->rotate);
		MSG_DEBUG(" mirror        =%u\n", lyp->mirror);
		MSG_DEBUG(" dummy         =%u\n", lyp->dummy);
		MSG_DEBUG(" key_color     =%ld\n", lyp->key_color);
		MSG_DEBUG(" premultiplied =%u\n", lyp->premultiplied);
		MSG_DEBUG(" alpha_coef    =%u\n", lyp->alpha_coef);
		MSG_DEBUG(" palette       =0x%p\n", lyp->palette);
		MSG_DEBUG(" palette_size  =%lu\n", lyp->palette_size);
		MSG_DEBUG(" alpha_plane   =0x%p\n", lyp->alpha_plane);
	}

	oimg = (struct screen_grap_image_param *)(&(bl_hdl->output_image));
	MSG_DEBUG(" ********* output_image from API=%p *********\n",
		oimg);
	MSG_DEBUG(" width         =%u\n", oimg->width);
	MSG_DEBUG(" height        =%u\n", oimg->height);
	MSG_DEBUG(" stride        =%u\n", oimg->stride);
	MSG_DEBUG(" stride_c      =%u\n", oimg->stride_c);
	MSG_DEBUG(" format        =%u\n", oimg->format);
	MSG_DEBUG(" yuv_format    =%u\n", oimg->yuv_format);
	MSG_DEBUG(" yuv_range     =%u\n", oimg->yuv_range);
	MSG_DEBUG(" address       =0x%p\n", oimg->address);
	MSG_DEBUG(" address_c0    =0x%p\n", oimg->address_c0);
	MSG_DEBUG(" address_c1    =0x%p\n", oimg->address_c1);
	MSG_DEBUG("%s", " ********************************\n\n");

	for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
		chainp = (struct t_chain_info *)(&(bl_hdl->stack[i]));
		MSG_DEBUG(" ********* bl_hdl stack[%d] = %p*********\n",
			i, chainp);
		for (j = 0; j < IMGPCTRL_STACK_MAX; j++) {
			MSG_DEBUG(" ******* stack_chain param[%d] *******\n",
				 j);
			MSG_DEBUG(" hw_type       =%d\n",
				chainp->chain[j].param.hw_type);
			MSG_DEBUG(" rq_type       =%d\n",
				chainp->chain[j].param.rq_type);
			MSG_DEBUG(" rq_layer      =%d\n",
				chainp->chain[j].param.rq_layer);
			MSG_DEBUG(" bg_color      =%ld\n",
				chainp->chain[j].param.bg_color);
			for (k = 0; k < IMGPCTRL_LAYER_MAX; k++) {
				lyp = (struct screen_grap_layer  *)
				(&(chainp->chain[j].param.input_layer[k]));
				MSG_DEBUG("***c.input_layer[%d]=%p***\n",
					k, lyp);
				MSG_DEBUG("%s", " ///// screen_rect /////\n");
				MSG_DEBUG(" rect.x        =%u\n", lyp->rect.x);
				MSG_DEBUG(" rect.y        =%u\n", lyp->rect.y);
				MSG_DEBUG(" rect.width    =%u\n",
					lyp->rect.width);
				MSG_DEBUG(" rect.height   =%u\n",
					lyp->rect.height);
				MSG_DEBUG("%s",
					"/// screen_grap_image_param ///\n");
				MSG_DEBUG(" width         =%u\n",
					lyp->image.width);
				MSG_DEBUG(" height        =%u\n",
					lyp->image.height);
				MSG_DEBUG(" stride        =%u\n",
					lyp->image.stride);
				MSG_DEBUG(" stride_c      =%u\n",
					lyp->image.stride_c);
				MSG_DEBUG(" format        =%u\n",
					lyp->image.format);
				MSG_DEBUG(" yuv_format    =%u\n",
					lyp->image.yuv_format);
				MSG_DEBUG(" yuv_range     =%u\n",
					lyp->image.yuv_range);
				MSG_DEBUG(" address       =0x%p\n",
					lyp->image.address);
				MSG_DEBUG(" address_c0    =0x%p\n",
					lyp->image.address_c0);
				MSG_DEBUG(" address_c1    =0x%p\n",
					lyp->image.address_c1);
				MSG_DEBUG("%s",
					" ///// screen_grap_layer /////\n");
				MSG_DEBUG(" alpha         =%u\n", lyp->alpha);
				MSG_DEBUG(" rotate        =%u\n", lyp->rotate);
				MSG_DEBUG(" mirror        =%u\n", lyp->mirror);
				MSG_DEBUG(" dummy         =%u\n", lyp->dummy);
				MSG_DEBUG(" key_color     =%ld\n",
					lyp->key_color);
				MSG_DEBUG(" premultiplied =%u\n",
					lyp->premultiplied);
				MSG_DEBUG(" alpha_coef    =%u\n",
					lyp->alpha_coef);
				MSG_DEBUG(" palette       =0x%p\n",
					lyp->palette);
				MSG_DEBUG(" palette_size  =%lu\n",
					lyp->palette_size);
				MSG_DEBUG(" alpha_plane   =0x%p\n",
					lyp->alpha_plane);
			}
			for (k = 0; k < IMGPCTRL_LAYER_MAX; k++) {
				lyp = (struct screen_grap_layer  *)
				(&(chainp->chain[j].param.output_layer[k]));
				MSG_DEBUG("***c.output_layer[%d]=%p***\n",
					k, lyp);
				MSG_DEBUG("%s", " ///// screen_rect /////\n");
				MSG_DEBUG(" rect.x        =%u\n",
					lyp->rect.x);
				MSG_DEBUG(" rect.y        =%u\n",
					lyp->rect.y);
				MSG_DEBUG(" rect.width    =%u\n",
					lyp->rect.width);
				MSG_DEBUG(" rect.height   =%u\n",
					lyp->rect.height);
				MSG_DEBUG("%s",
					"/// screen_grap_image_param ///\n");
				MSG_DEBUG(" width         =%u\n",
					lyp->image.width);
				MSG_DEBUG(" height        =%u\n",
					lyp->image.height);
				MSG_DEBUG(" stride        =%u\n",
					lyp->image.stride);
				MSG_DEBUG(" stride_c      =%u\n",
					lyp->image.stride_c);
				MSG_DEBUG(" format        =%u\n",
					lyp->image.format);
				MSG_DEBUG(" yuv_format    =%u\n",
					lyp->image.yuv_format);
				MSG_DEBUG(" yuv_range     =%u\n",
					lyp->image.yuv_range);
				MSG_DEBUG(" address       =0x%p\n",
					lyp->image.address);
				MSG_DEBUG(" address_c0    =0x%p\n",
					lyp->image.address_c0);
				MSG_DEBUG(" address_c1    =0x%p\n",
					lyp->image.address_c1);
				MSG_DEBUG("%s",
					" ///// screen_grap_layer /////\n");
				MSG_DEBUG(" alpha         =%u\n", lyp->alpha);
				MSG_DEBUG(" rotate        =%u\n", lyp->rotate);
				MSG_DEBUG(" mirror        =%u\n", lyp->mirror);
				MSG_DEBUG(" dummy         =%u\n", lyp->dummy);
				MSG_DEBUG(" key_color     =%ld\n",
					lyp->key_color);
				MSG_DEBUG(" premultiplied =%u\n",
					lyp->premultiplied);
				MSG_DEBUG(" alpha_coef    =%u\n",
					lyp->alpha_coef);
				MSG_DEBUG(" palette       =0x%p\n",
					lyp->palette);
				MSG_DEBUG(" palette_size  =%lu\n",
					lyp->palette_size);
				MSG_DEBUG(" alpha_plane   =0x%p\n",
					lyp->alpha_plane);
			}
			for (k = 0; k < IMGPCTRL_LAYER_MAX; k++) {
				extp = (struct t_ext_layer  *)
				(&(chainp->chain[j].param.ext_input[k]));
				MSG_DEBUG("x_offset       =%d\n",
					extp->x_offset);
				MSG_DEBUG("y_offset       =%d\n",
					extp->y_offset);
			}
		}
		MSG_DEBUG("chain ptr =%d\n", chainp->ptr);
		MSG_DEBUG("chain num =%d\n", chainp->num);
	}

	MSG_DEBUG("%s", " ********* bl_handle param memory info *********\n");
	memp = (struct t_memory_info *)(&(bl_hdl->mem_info));
	MSG_DEBUG(" buf_info.size = 0x%08lx\n", memp->buf_info.size);
	MSG_DEBUG(" buf_info.p_addr = 0x%08lx\n", memp->buf_info.p_addr);
	MSG_DEBUG(" buf_info.v_addr = 0x%08lx\n", memp->buf_info.v_addr);
	MSG_DEBUG(" buf_info.flag = %d\n", (int)memp->buf_info.flag);
	MSG_DEBUG(" buf_offset = 0x%08lx\n", memp->buf_offset);
	MSG_DEBUG(" start_ptr = 0x%08lx\n", memp->start_ptr);
	MSG_DEBUG(" end_ptr = 0x%08lx\n", memp->end_ptr);
	MSG_DEBUG(" buf use flag = %d\n", memp->buf_use_flag);
	for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
		MSG_DEBUG(" o_width[%d] = %d\n", i, bl_hdl->o_width[i]);
		MSG_DEBUG(" o_height[%d] = %d\n", i, bl_hdl->o_height[i]);
	}


	if (exe_info != (struct t_exe_info *)NULL) {
		MSG_DEBUG("%s", " ********* exe_info param *********\n");
		MSG_DEBUG(" hw_type       =%d\n", exe_info->hw_type);
		MSG_DEBUG(" rq_type       =%d\n", exe_info->rq_type);
		MSG_DEBUG(" rq_layer      =%d\n", exe_info->rq_layer);
		MSG_DEBUG(" bg_color      =%ld\n", exe_info->bg_color);

		for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
			lyp = (struct screen_grap_layer *)
				(&(exe_info->input_layer[i]));
			MSG_DEBUG("*** input_layer[%d] =%p ***\n",
			i, lyp);
			MSG_DEBUG("%s", " ///// screen_rect /////\n");
			MSG_DEBUG(" rect.x        =%u\n", lyp->rect.x);
			MSG_DEBUG(" rect.y        =%u\n", lyp->rect.y);
			MSG_DEBUG(" rect.width    =%u\n", lyp->rect.width);
			MSG_DEBUG(" rect.height   =%u\n", lyp->rect.height);
			MSG_DEBUG("%s",
				" ///// screen_grap_image_param /////\n");
			MSG_DEBUG(" width         =%u\n", lyp->image.width);
			MSG_DEBUG(" height        =%u\n",
				lyp->image.height);
			MSG_DEBUG(" stride        =%u\n",
				lyp->image.stride);
			MSG_DEBUG(" stride_c      =%u\n",
				lyp->image.stride_c);
			MSG_DEBUG(" format        =%u\n",
				lyp->image.format);
			MSG_DEBUG(" yuv_format    =%u\n",
				lyp->image.yuv_format);
			MSG_DEBUG(" yuv_range     =%u\n",
				lyp->image.yuv_range);
			MSG_DEBUG(" address       =0x%p\n", lyp->image.address);
			MSG_DEBUG(" address_c0    =0x%p\n",
					lyp->image.address_c0);
			MSG_DEBUG(" address_c1    =0x%p\n",
					lyp->image.address_c1);
			MSG_DEBUG("%s", " ///// screen_grap_layer /////\n");
			MSG_DEBUG(" alpha         =%u\n", lyp->alpha);
			MSG_DEBUG(" rotate        =%u\n", lyp->rotate);
			MSG_DEBUG(" mirror        =%u\n", lyp->mirror);
			MSG_DEBUG(" dummy         =%u\n", lyp->dummy);
			MSG_DEBUG(" key_color     =%ld\n", lyp->key_color);
			MSG_DEBUG(" premultiplied =%u\n", lyp->premultiplied);
			MSG_DEBUG(" alpha_coef    =%u\n", lyp->alpha_coef);
			MSG_DEBUG(" palette       =0x%p\n", lyp->palette);
			MSG_DEBUG(" palette_size  =%lu\n", lyp->palette_size);
			MSG_DEBUG(" alpha_plane   =0x%p\n", lyp->alpha_plane);
		}

		for (i = 0; i < IMGPCTRL_LAYER_MAX; i++) {
			lyp = (struct screen_grap_layer *)
				(&(exe_info->output_layer[i]));
			MSG_DEBUG("*** output_layer[%d] = 0x%p ***\n", i, lyp);
			MSG_DEBUG("%s", " ///// screen_rect /////\n");
			MSG_DEBUG(" rect.x        =%u\n", lyp->rect.x);
			MSG_DEBUG(" rect.y        =%u\n", lyp->rect.y);
			MSG_DEBUG(" rect.width    =%u\n", lyp->rect.width);
			MSG_DEBUG(" rect.height   =%u\n", lyp->rect.height);
			MSG_DEBUG("%s",
				" ///// screen_grap_image_param /////\n");
			MSG_DEBUG(" width         =%u\n", lyp->image.width);
			MSG_DEBUG(" height        =%u\n", lyp->image.height);
			MSG_DEBUG(" stride        =%u\n", lyp->image.stride);
			MSG_DEBUG(" stride_c      =%u\n", lyp->image.stride_c);
			MSG_DEBUG(" format        =%u\n", lyp->image.format);
			MSG_DEBUG(" yuv_format    =%u\n",
				lyp->image.yuv_format);
			MSG_DEBUG(" yuv_range     =%u\n", lyp->image.yuv_range);
			MSG_DEBUG(" address       =0x%p\n",
				lyp->image.address);
			MSG_DEBUG(" address_c0    =0x%p\n",
				lyp->image.address_c0);
			MSG_DEBUG(" address_c1    =0x%p\n",
				lyp->image.address_c1);
			MSG_DEBUG("%s", " ///// screen_grap_layer /////\n");
			MSG_DEBUG(" alpha         =%u\n", lyp->alpha);
			MSG_DEBUG(" rotate        =%u\n", lyp->rotate);
			MSG_DEBUG(" mirror        =%u\n", lyp->mirror);
			MSG_DEBUG(" dummy         =%u\n", lyp->dummy);
			MSG_DEBUG(" key_color     =%ld\n", lyp->key_color);
			MSG_DEBUG(" premultiplied =%u\n", lyp->premultiplied);
			MSG_DEBUG(" alpha_coef    =%u\n", lyp->alpha_coef);
			MSG_DEBUG(" palette       =0x%p\n", lyp->palette);
			MSG_DEBUG(" palette_size  =%lu\n", lyp->palette_size);
			MSG_DEBUG(" alpha_plane   =0x%p\n", lyp->alpha_plane);
			MSG_DEBUG("%s", "********************************\n");
			MSG_DEBUG("%s", "********************************\n");
		}
	}
}

void imgpctrl_error_log(u_short function_id, u_short line, long errorcode,
	u_long temp, struct screen_grap_handle *bl_hdl)
{
	struct t_err_log  *err_log;
	u_long            ecnt;

	if (bl_hdl == NULL) {
		MSG_ERROR("[%s] ERR: bl_hdl cannot write. Line:%d\n",
			__func__, __LINE__);
		return;
	}

	err_log = (struct t_err_log *)bl_hdl->log_start_ptr;

	down(&(bl_hdl->err_sem.imgpctrl_sem));

	err_log->version = IMGPCTRL_VER;
	ecnt = err_log->err_cnt;

	MSG_DEBUG("error version  = 0x%lx\n", (err_log->version));
	MSG_DEBUG("error trace_flg = 0x%lx\n", (err_log->trace_flg));

	if (err_log->trace_flg == LOG_NOUPDATE) {
		err_log->trace_flg = LOG_UPDATE;
		err_log->err_info[ecnt].id_line =
			(u_long)(((u_long)function_id << IMGPCTRL_16BIT_SHIFT) |
			(u_long)line);
		err_log->err_info[ecnt].err_code = errorcode;
		err_log->err_info[ecnt].temp = temp;
		err_log->err_info[ecnt].time = (u_long)get_jiffies_64();

		err_log->err_cnt = ((ecnt+1)&0x07);

		MSG_DEBUG("error trace_flg = 0x%lx\n", (err_log->trace_flg));

		MSG_DEBUG("err_log->err_info[%u].id_line = 0x%08lx\n",
			(u_int)ecnt,
			(u_long)(err_log->err_info[ecnt].id_line));
		MSG_DEBUG("err_log->err_info[%u].errcode = %d\n",
			(u_int)ecnt,
			(u_int)(err_log->err_info[ecnt].err_code));
		MSG_DEBUG("err_log->err_info[%u].tmp = 0x%08lx\n",
			(u_int)ecnt,
			(u_long)(err_log->err_info[ecnt].temp));
		MSG_DEBUG("err_log->err_info[%u].time = 0x%08lx\n",
			(u_int)ecnt,
			(u_long)(err_log->err_info[ecnt].time));

	}

	MSG_DEBUG("error trace_flg = 0x%lx\n", (err_log->trace_flg));

	up(&(bl_hdl->err_sem.imgpctrl_sem));
}

void imgpctrl_trace_log(u_short function_id, u_char flag, u_char size,
	u_long *ptr, struct screen_grap_handle *bl_hdl)
{
	struct t_trace_info	*t_add;
	u_long logsz;
	long tmpsz;
	u_long *start_ptr;
	u_long *end_ptr;
	u_long *next_ptr;
	u_long *data_ptr;
	int	i;
	u_long time_stamp;
	pid_t tid = 0;

	MSG_DEBUG("[DBG] trace_log RT_GRAPHICS_TRACE_ID = %d\n",
		RT_GRAPHICS_TRACE_ID);
	MSG_DEBUG("%s", "****[DBG] trace log ****\n");
	MSG_DEBUG("bl_hdl add = 0x%p\n", bl_hdl);
	MSG_DEBUG("size = %d\n", size);

	if (bl_hdl == NULL) {
		MSG_ERROR("[%s] ERR: bl_hdl is NULL. Line:%d\n",
			__func__, __LINE__);
		return;
	}
	if (size == 0xFF) {
		MSG_ERROR("[%s] ERR: trace log size over. Line:%d\n",
			__func__, __LINE__);
		return;
	}

	t_add = (struct t_trace_info *)bl_hdl->log_start_ptr;
	time_stamp = (u_long)get_jiffies_64();

	/* for function check add printk */
	if (t_add->trace2_size == 0) {
		start_ptr = (u_long *)(t_add->trace1_add + 1);
		logsz = IMGPCTRL_TRACE1_SIZE/4;	/* default value */
		end_ptr = (u_long *)(t_add->trace1_add + logsz);

		spin_lock(&(bl_hdl->trace_spin.bl_lock));

		data_ptr = (u_long *)(*(t_add->trace1_add));
		next_ptr = data_ptr + (size + 4);
		if (next_ptr >= end_ptr) {
			tmpsz = next_ptr - end_ptr;
			next_ptr = start_ptr + tmpsz;
		}

		*(t_add->trace1_add) = (u_long)next_ptr;

		spin_unlock(&(bl_hdl->trace_spin.bl_lock));

	} else {
		start_ptr = (u_long *)(t_add->trace2_add + 1);
		logsz = t_add->trace2_size/4;
		end_ptr = (u_long *)(t_add->trace2_add + logsz);

		spin_lock(&(bl_hdl->trace_spin.bl_lock));

		data_ptr = (u_long *)(*(t_add->trace2_add));
		next_ptr = data_ptr + (size + 4);
		if (next_ptr >= end_ptr) {
			tmpsz = next_ptr - end_ptr;
			next_ptr = start_ptr + tmpsz;
		}
		*(t_add->trace2_add) = (u_long)next_ptr;

		spin_unlock(&(bl_hdl->trace_spin.bl_lock));
	}

	/* data part write */
	for (i = 0; i < size; i++) {
		*data_ptr = *ptr;
		data_ptr++;
		ptr++;
		if (data_ptr == end_ptr) {
			/* set pointer start position */
			data_ptr = start_ptr;
		}
	}

	/* insert bl_hdr address */
	*data_ptr = (u_long)bl_hdl;
	data_ptr++;
	if (data_ptr == end_ptr) {
		/* set pointer start position */
		data_ptr = start_ptr;
	}

	/* insert thread ID */
	if (bl_hdl->cb_bl_thread != NULL) {
		tid = current->pid;
		*data_ptr = (u_long)tid;
	} else {
		*data_ptr = 0;
	}
	data_ptr++;
	if (data_ptr == end_ptr) {
		/* set pointer start position */
		data_ptr = start_ptr;
	}

	/* process time write */
	*data_ptr = time_stamp;
	data_ptr++;
	if (data_ptr == end_ptr) {
		/* set pointer start position */
		data_ptr = start_ptr;
	}

	/* trace header write */
	*data_ptr = (u_long)(((u_long)function_id << IMGPCTRL_16BIT_SHIFT) |
			((u_long)flag << IMGPCTRL_8BIT_SHIFT) |
			(u_long)(size+3));
}
