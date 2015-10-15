/*************************************************************************/ /*
 VSPM

 Copyright (C) 2013-2014 Renesas Electronics Corporation

 License        Dual MIT/GPLv2

 The contents of this file are subject to the MIT license as set out below.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 Alternatively, the contents of this file may be used under the terms of
 the GNU General Public License Version 2 ("GPL") in which case the provisions
 of GPL are applicable instead of those above.

 If you wish to allow use of your version of this file only under the terms of
 GPL, and not to allow others to use your version of this file under the terms
 of the MIT license, indicate your decision by deleting the provisions above
 and replace them with the notice and other provisions required by GPL as set
 out in the file called "GPL-COPYING" included in this distribution. If you do
 not delete the provisions above, a recipient may use your version of this file
 under the terms of either the MIT license or GPL.

 This License is also included in this distribution in the file called
 "MIT-COPYING".

 EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
 PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


 GPLv2:
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

#include "vspm_main.h"
#include "vspm_log.h"
#include "vsp_drv.h"
#include "vsp_drv_local.h"

/* WPF module route table */
const unsigned long vsp_tbl_wpf_route[VSP_WPF_MAX] = {
	VSP_DPR_ROUTE_WPF0,
	VSP_DPR_ROUTE_WPF1,
	VSP_DPR_ROUTE_WPF2,
	VSP_DPR_ROUTE_WPF3,
};

/* BRU module route table */
const unsigned long vsp_tbl_bru_route[VSP_BRU_CH_MAX] = {
	VSP_DPR_ROUTE_BRU0,
	VSP_DPR_ROUTE_BRU1,
	VSP_DPR_ROUTE_BRU2,
	VSP_DPR_ROUTE_BRU3,
};

/* VPS usable module bits */
extern const unsigned long vsp_tbl_usable_module[VSP_IP_MAX];

/* VPS usable RPF bits */
extern const unsigned long vsp_tbl_usable_rpf[VSP_IP_MAX];

/* VPS usable WPF bits */
extern const unsigned long vsp_tbl_usable_wpf[VSP_IP_MAX];

/*
 * vsp_ins_check_init_parameter - Check initialise parameter routine
 * @param:		T_VSP_INIT parameter
 *
 */
long vsp_ins_check_init_parameter(T_VSP_INIT *param)
{
	/* no check */
	return 0;
}

/*
 * vsp_ins_get_dpr_route - Get DPR route value routine
 * @ch_info:	channel information
 * @connect:	connect module ID
 * @fxa:		fixed alpha output value
 *
 */
static unsigned long vsp_ins_get_dpr_route(
	struct vsp_ch_info *ch_info, unsigned long connect, unsigned char fxa)
{
	unsigned long route;

	/* set fixed alpha output value */
	route = ((unsigned long)fxa) << 16;

	/* get target node value */
	switch (connect) {
	case 0:
		route |= vsp_tbl_wpf_route[ch_info->wpf_ch];
		ch_info->wpf_cnt++;
		break;
	case VSP_SRU_USE:
		route |= VSP_DPR_ROUTE_SRU;
		break;
	case VSP_UDS_USE:
		route |= VSP_DPR_ROUTE_UDS0;
		break;
	case VSP_UDS1_USE:
		route |= VSP_DPR_ROUTE_UDS1;
		break;
	case VSP_UDS2_USE:
		route |= VSP_DPR_ROUTE_UDS2;
		break;
	case VSP_LUT_USE:
		route |= VSP_DPR_ROUTE_LUT;
		break;
	case VSP_CLU_USE:
		route |= VSP_DPR_ROUTE_CLU;
		break;
	case VSP_HST_USE:
		route |= VSP_DPR_ROUTE_HST;
		break;
	case VSP_HSI_USE:
		route |= VSP_DPR_ROUTE_HSI;
		break;
	case VSP_BRU_USE:
		if (ch_info->bru_cnt < VSP_BRU_CH_MAX) {
			route |= vsp_tbl_bru_route[ch_info->bru_cnt];
		} else {
			/* BRU channel empty */
			route = VSP_DPR_ROUTE_NOT_USE;
		}
		ch_info->bru_cnt++;
		break;
	default:
		route = VSP_DPR_ROUTE_NOT_USE;
		break;
	}

	ch_info->next_module = connect;

	return route;
}

/*
 * vsp_ins_get_dpr_smppt - Get DPR smppt value routine
 * @ch_info:	channel information
 * @smpling:	sampling point module ID
 *
 */
static unsigned long vsp_ins_get_dpr_smppt(
	struct vsp_ch_info *ch_info, unsigned long sampling)
{
	unsigned long smppt;

	/* set target wpf index */
	smppt = ((unsigned long)ch_info->wpf_ch) << 8;

	/* get target node index */
	switch (sampling) {
	case VSP_SMPPT_SRC1:
	case VSP_SMPPT_SRC2:
	case VSP_SMPPT_SRC3:
	case VSP_SMPPT_SRC4:
		if (sampling < ch_info->src_cnt)
			smppt |= ch_info->src_info[sampling].rpf_ch;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_SRU:
		if (ch_info->reserved_module & VSP_SRU_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_UDS:
		if (ch_info->reserved_module & VSP_UDS_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_UDS1:
		if (ch_info->reserved_module & VSP_UDS1_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_UDS2:
		if (ch_info->reserved_module & VSP_UDS2_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_LUT:
		if (ch_info->reserved_module & VSP_LUT_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_BRU:
		if (ch_info->reserved_module & VSP_BRU_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_CLU:
		if (ch_info->reserved_module & VSP_CLU_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_HST:
		if (ch_info->reserved_module & VSP_HST_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	case VSP_SMPPT_HSI:
		if (ch_info->reserved_module & VSP_HSI_USE)
			smppt |= sampling;
		else
			smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	default:
		smppt = VSP_DPR_SMPPT_NOT_USE;
		break;
	}

	return smppt;
}

/*
 * vsp_ins_get_passband_bwidth - Get passband value routine
 * @ratio:		HMANT[15:12],HFRAC[11:0]
 *
 */
static unsigned long vsp_ins_get_passband_bwidth(unsigned short ratio)
{
	unsigned long bwidth;

	if (ratio & VSP_UDS_SCALE_MANT) {
		if (ratio < 0x4000)
			bwidth = 0x40000;
		else if (ratio < 0x8000)
			bwidth = 0x80000;
		else
			bwidth = 0x100000;

		bwidth = bwidth / ratio;
	} else {
		bwidth = 64;
	}

	return bwidth;
}


/*
 * vsp_ins_check_input_color_space_of_bru - Check input color space routine
 * @ch_info:		channel information
 *
 */
static long vsp_ins_check_input_color_space_of_bru(struct vsp_ch_info *ch_info)
{
	struct vsp_src_info *src_info;
	unsigned char base_color;
	unsigned char i;

	if (ch_info->src_cnt > 0) {
		src_info = &ch_info->src_info[0];

		/* set base color space */
		base_color = src_info->color;

		for (i = 0; i < ch_info->src_cnt; i++) {
			if (src_info->color == VSP_COLOR_HSV)
				return E_VSP_PARA_BRU_INHSV;

			if (src_info->color != base_color)
				return E_VSP_PARA_BRU_INCOLOR;

			src_info++;
		}
	}

	return 0;
}

/*
 * vsp_ins_check_master_layer - Check master layer routine
 * @ch_info:		channel information
 *
 */
static long vsp_ins_check_master_layer(struct vsp_ch_info *ch_info)
{
	struct vsp_src_info *src_info;
	unsigned char master_idx;
	unsigned char master_cnt = 0;
	unsigned long srcrpf = 0;

	unsigned char color = VSP_COLOR_NO;

	unsigned char i;

	/* check input source counter */
	if (ch_info->src_cnt == 0) {
		/* no input source */
		return E_VSP_PARA_NOINPUT;
	}

	/* search and increment master counter */
	src_info = &ch_info->src_info[0];
	for (i = 0; i < ch_info->src_cnt; i++) {
		if (src_info->master == VSP_LAYER_PARENT) {
			master_idx = i;
			master_cnt++;
		}

		/* set WPFn_SRCRPF register value */
		srcrpf |= src_info->master << (src_info->rpf_ch * 2);

		/* set color space */
		if (src_info->color != VSP_COLOR_NO)
			color = src_info->color;

		src_info++;
	}

	/* check master counter */
	if (master_cnt != 1) {
		/* no master or over masters */
		return E_VSP_PARA_NOPARENT;
	}

	/* check position parameter */
	src_info = &ch_info->src_info[0];
	for (i = 0; i < ch_info->src_cnt; i++) {
		if (i != master_idx) {
			if (src_info->x_position >=
					ch_info->src_info[master_idx].width) {
				if (src_info->rpf_ch == 14)
					return E_VSP_PARA_VIR_XPOSI;
				else
					return E_VSP_PARA_IN_XPOSI;
			}

			if (src_info->y_position >=
					ch_info->src_info[master_idx].height) {
				if (src_info->rpf_ch == 14)
					return E_VSP_PARA_VIR_YPOSI;
				else
					return E_VSP_PARA_IN_YPOSI;
			}
		}

		src_info++;
	}

	/* set master layer */
	ch_info->src_idx = master_idx;
	ch_info->src_info[master_idx].master = srcrpf;
	ch_info->src_info[master_idx].color = color;

	return 0;
}

/*
 * vsp_ins_check_rpf_format - Check RPF format parameter routine
 * @rpf_info:	RPF information
 * @in_param:	input parameter
 *
 */
static long vsp_ins_check_rpf_format(
	struct vsp_rpf_info *rpf_info, T_VSP_IN *in_param)
{
	unsigned long x_offset = (unsigned long)in_param->x_offset;
	unsigned long x_offset_c = (unsigned long)in_param->x_offset;
	unsigned long y_offset = (unsigned long)in_param->y_offset;
	unsigned long y_offset_c = (unsigned long)in_param->y_offset;
	unsigned long stride = (unsigned long)in_param->stride;
	unsigned long stride_c = (unsigned long)in_param->stride_c;
	unsigned long temp;

	if (in_param->vir == VSP_NO_VIR) {
		/* check RGB(Y) address pointer */
		if (in_param->addr == NULL)
			return E_VSP_PARA_IN_ADR;

		switch (in_param->format) {
		case VSP_IN_RGB332:
		case VSP_IN_XRGB4444:
		case VSP_IN_RGBX4444:
		case VSP_IN_XRGB1555:
		case VSP_IN_RGBX5551:
		case VSP_IN_RGB565:
		case VSP_IN_AXRGB86666:
		case VSP_IN_RGBXA66668:
		case VSP_IN_XRGBA66668:
		case VSP_IN_ARGBX86666:
		case VSP_IN_AXXXRGB82666:
		case VSP_IN_XXXRGBA26668:
		case VSP_IN_ARGBXXX86662:
		case VSP_IN_RGBXXXA66628:
		case VSP_IN_XRGB6666:
		case VSP_IN_RGBX6666:
		case VSP_IN_XXXRGB2666:
		case VSP_IN_RGBXXX6662:
		case VSP_IN_ARGB8888:
		case VSP_IN_RGBA8888:
		case VSP_IN_RGB888:
		case VSP_IN_XXRGB7666:
		case VSP_IN_XRGB14666:
		case VSP_IN_BGR888:
		case VSP_IN_ARGB4444:
		case VSP_IN_RGBA4444:
		case VSP_IN_ARGB1555:
		case VSP_IN_RGBA5551:
		case VSP_IN_ABGR4444:
		case VSP_IN_BGRA4444:
		case VSP_IN_ABGR1555:
		case VSP_IN_BGRA5551:
		case VSP_IN_XXXBGR2666:
		case VSP_IN_ABGR8888:
		case VSP_IN_XRGB16565:
		case VSP_IN_RGB_CLUT_DATA:
		case VSP_IN_YUV_CLUT_DATA:
			temp = (unsigned long)
				((in_param->format & 0x0f00) >> 8);

			/* set address */
			rpf_info->val_addr_y =
				(unsigned long)in_param->addr +
				(y_offset * stride) +
				(x_offset * temp);

			rpf_info->val_addr_c0 = 0;
			rpf_info->val_addr_c1 = 0;
			break;
		/* YUV interleaved */
		case VSP_IN_YUV420_INTERLEAVED:
			/* check height */
			if (in_param->height & 0x1)
				return E_VSP_PARA_IN_HEIGHT;

			/* check height_ex */
			if (in_param->height_ex & 0x1)
				return E_VSP_PARA_IN_HEIGHTEX;

			/* check y_offset */
			if (in_param->y_offset & 0x1)
				return E_VSP_PARA_IN_YOFFSET;

			x_offset <<= 1;
			y_offset >>= 1;
			/* break; */
		case VSP_IN_YUV422_INTERLEAVED0:
		case VSP_IN_YUV422_INT0_YUY2:
		case VSP_IN_YUV422_INT0_YVYU:
		case VSP_IN_YUV422_INTERLEAVED1:
			/* check width */
			if (in_param->width & 0x1)
				return E_VSP_PARA_IN_WIDTH;

			/* check width_ex */
			if (in_param->width_ex & 0x1)
				return E_VSP_PARA_IN_WIDTHEX;

			/* check x_offset */
			if (in_param->x_offset & 0x1)
				return E_VSP_PARA_IN_XOFFSET;

			x_offset_c >>= 1;
			/* break; */
		case VSP_IN_YUV444_INTERLEAVED:

			temp = (y_offset * stride) +
				(x_offset + x_offset_c + x_offset_c);

			rpf_info->val_addr_y =
				(unsigned long)in_param->addr + temp;

			rpf_info->val_addr_c0 = 0;
			rpf_info->val_addr_c1 = 0;
			break;
		/* YUV semi planer */
		case VSP_IN_YUV420_SEMI_PLANAR:
		case VSP_IN_YUV420_SEMI_NV21:
			/* check height */
			if (in_param->height & 0x1)
				return E_VSP_PARA_IN_HEIGHT;

			/* check height_ex */
			if (in_param->height_ex & 0x1)
				return E_VSP_PARA_IN_HEIGHTEX;

			/* check y_offset */
			if (in_param->y_offset & 0x1)
				return E_VSP_PARA_IN_YOFFSET;

			y_offset_c >>= 1;
			/* break; */
		case VSP_IN_YUV422_SEMI_PLANAR:
		case VSP_IN_YUV422_SEMI_NV61:
			/* check width */
			if (in_param->width & 0x1)
				return E_VSP_PARA_IN_WIDTH;

			/* check width_ex */
			if (in_param->width_ex & 0x1)
				return E_VSP_PARA_IN_WIDTHEX;

			/* check x_offset */
			if (in_param->x_offset & 0x1)
				return E_VSP_PARA_IN_XOFFSET;

			x_offset_c >>= 1;
			/* break; */
		case VSP_IN_YUV444_SEMI_PLANAR:
			/* check Cb address pointer */
			if (in_param->addr_c0 == NULL)
				return E_VSP_PARA_IN_ADRC0;

			/* set address */
			rpf_info->val_addr_y =
				(unsigned long)in_param->addr +
				(y_offset * stride) +
				x_offset;

			temp = (y_offset_c * stride_c) +
				(x_offset_c + x_offset_c);

			rpf_info->val_addr_c0 =
				(unsigned long)in_param->addr_c0 + temp;
			rpf_info->val_addr_c1 = 0;
			break;
		/* YUV planar */
		case VSP_IN_YUV420_PLANAR:
			/* check height */
			if (in_param->height & 0x1)
				return E_VSP_PARA_IN_HEIGHT;

			/* check height_ex */
			if (in_param->height_ex & 0x1)
				return E_VSP_PARA_IN_HEIGHTEX;

			/* check y_offset */
			if (in_param->y_offset & 0x1)
				return E_VSP_PARA_IN_YOFFSET;

			y_offset_c >>= 1;
			/* break; */
		case VSP_IN_YUV422_PLANAR:
			/* check width */
			if (in_param->width & 0x1)
				return E_VSP_PARA_IN_WIDTH;

			/* check width_ex */
			if (in_param->width_ex & 0x1)
				return E_VSP_PARA_IN_WIDTHEX;

			/* check x_offset */
			if (in_param->x_offset & 0x1)
				return E_VSP_PARA_IN_XOFFSET;

			x_offset_c >>= 1;
			/* break; */
		case VSP_IN_YUV444_PLANAR:
			/* check CbCr address pointer */
			if (in_param->addr_c0 == NULL)
				return E_VSP_PARA_IN_ADRC0;

			if (in_param->addr_c1 == NULL)
				return E_VSP_PARA_IN_ADRC1;

			/* set address */
			rpf_info->val_addr_y =
				(unsigned long)in_param->addr +
				(y_offset * stride) +
				x_offset;

			temp = (y_offset_c * stride_c) + x_offset_c;

			rpf_info->val_addr_c0 =
				(unsigned long)in_param->addr_c0 + temp;
			rpf_info->val_addr_c1 =
				(unsigned long)in_param->addr_c1 + temp;
			break;
		default:
			return E_VSP_PARA_IN_FORMAT;
			break;
		}
	} else {	/* in_param->vir == VSP_VIR */
		if ((in_param->format != VSP_IN_ARGB8888) &&
			(in_param->format != VSP_IN_YUV444_SEMI_PLANAR)) {
			return E_VSP_PARA_IN_FORMAT;
		}

		rpf_info->val_addr_y = 0;
		rpf_info->val_addr_c0 = 0;
		rpf_info->val_addr_c1 = 0;
	}

	return 0;
}

/*
 * vsp_ins_check_wpf_format - Check WPF format parameter routine
 * @ch_info:	channel(WPF) information
 * @out_param:	output parameter
 *
 */
static long vsp_ins_check_wpf_format(
	struct vsp_ch_info *ch_info, T_VSP_OUT *out_param)
{
	unsigned long x_offset = (unsigned long)out_param->x_offset;
	unsigned long x_offset_c = (unsigned long)out_param->x_offset;
	unsigned long y_offset = (unsigned long)out_param->y_offset;
	unsigned long y_offset_c = (unsigned long)out_param->y_offset;
	unsigned long stride = (unsigned long)out_param->stride;
	unsigned long stride_c = (unsigned long)out_param->stride_c;
	unsigned long temp;

	/* check RGB(Y) address pointer */
	if (out_param->addr == NULL)
		return E_VSP_PARA_OUT_ADR;

	switch (out_param->format) {
	case VSP_OUT_RGB332:
	case VSP_OUT_XRGB4444:
	case VSP_OUT_RGBX4444:
	case VSP_OUT_XRGB1555:
	case VSP_OUT_RGBX5551:
	case VSP_OUT_RGB565:
	case VSP_OUT_PXRGB86666:
	case VSP_OUT_RGBXP66668:
	case VSP_OUT_XRGBP66668:
	case VSP_OUT_PRGBX86666:
	case VSP_OUT_PXXXRGB82666:
	case VSP_OUT_XXXRGBP26668:
	case VSP_OUT_PRGBXXX86662:
	case VSP_OUT_RGBXXXP66628:
	case VSP_OUT_XRGB6666:
	case VSP_OUT_RGBX6666:
	case VSP_OUT_XXXRGB2666:
	case VSP_OUT_RGBXXX6662:
	case VSP_OUT_PRGB8888:
	case VSP_OUT_RGBP8888:
	case VSP_OUT_RGB888:
	case VSP_OUT_XXRGB7666:
	case VSP_OUT_XRGB14666:
	case VSP_OUT_BGR888:
	case VSP_OUT_PRGB4444:
	case VSP_OUT_RGBP4444:
	case VSP_OUT_PRGB1555:
	case VSP_OUT_RGBP5551:
	case VSP_OUT_PBGR4444:
	case VSP_OUT_BGRP4444:
	case VSP_OUT_PBGR1555:
	case VSP_OUT_BGRP5551:
	case VSP_OUT_XXXBGR2666:
	case VSP_OUT_PBGR8888:
	case VSP_OUT_XRGB16565:
		temp = (unsigned long)((out_param->format & 0x0f00) >> 8);

		/* set address */
		ch_info->val_addr_y =
			(unsigned long)out_param->addr +
			(y_offset * stride) +
			(x_offset * temp);

		ch_info->val_addr_c0 = 0;
		ch_info->val_addr_c1 = 0;
		break;
	/* YUV interleaved */
	case VSP_OUT_YUV420_INTERLEAVED:
		/* check height */
		if (out_param->height & 0x1)
			return E_VSP_PARA_OUT_HEIGHT;

		/* check y_offset */
		if (out_param->y_offset & 0x1)
			return E_VSP_PARA_OUT_YOFFSET;

		x_offset <<= 1;
		y_offset >>= 1;
		/* break; */
	case VSP_OUT_YUV422_INTERLEAVED0:
	case VSP_OUT_YUV422_INT0_YUY2:
	case VSP_OUT_YUV422_INT0_YVYU:
	case VSP_OUT_YUV422_INTERLEAVED1:
		/* check width */
		if (out_param->width & 0x1)
			return E_VSP_PARA_OUT_WIDTH;

		/* check x_offset */
		if (out_param->x_offset & 0x1)
			return E_VSP_PARA_OUT_XOFFSET;

		x_offset_c >>= 1;
		/* break; */
	case VSP_OUT_YUV444_INTERLEAVED:

		temp = (y_offset * stride) +
			(x_offset + x_offset_c + x_offset_c);

		ch_info->val_addr_y = (unsigned long)out_param->addr + temp;

		ch_info->val_addr_c0 = 0;
		ch_info->val_addr_c1 = 0;
		break;
	/* YUV semi planer */
	case VSP_OUT_YUV420_SEMI_PLANAR:
	case VSP_OUT_YUV420_SEMI_NV21:
		/* check height */
		if (out_param->height & 0x1)
			return E_VSP_PARA_OUT_HEIGHT;

		/* check y_offset */
		if (out_param->y_offset & 0x1)
			return E_VSP_PARA_OUT_YOFFSET;

		y_offset_c >>= 1;
		/* break; */
	case VSP_OUT_YUV422_SEMI_PLANAR:
	case VSP_OUT_YUV422_SEMI_NV61:
		/* check width */
		if (out_param->width & 0x1)
			return E_VSP_PARA_OUT_WIDTH;

		/* check x_offset */
		if (out_param->x_offset & 0x1)
			return E_VSP_PARA_OUT_XOFFSET;

		x_offset_c >>= 1;
		/* break; */
	case VSP_OUT_YUV444_SEMI_PLANAR:
		/* check Cb address pointer */
		if (out_param->addr_c0 == NULL)
			return E_VSP_PARA_OUT_ADRC0;

		/* set address */
		ch_info->val_addr_y =
			(unsigned long)out_param->addr +
			(y_offset * stride) +
			x_offset;

		temp = (y_offset_c * stride_c) + (x_offset_c + x_offset_c);

		ch_info->val_addr_c0 = (unsigned long)out_param->addr_c0 + temp;
		ch_info->val_addr_c1 = 0;
		break;
	/* YUV planar */
	case VSP_OUT_YUV420_PLANAR:
		/* check height */
		if (out_param->height & 0x1)
			return E_VSP_PARA_OUT_HEIGHT;

		/* check y_offset */
		if (out_param->y_offset & 0x1)
			return E_VSP_PARA_OUT_YOFFSET;

		y_offset_c >>= 1;
		/* break; */
	case VSP_OUT_YUV422_PLANAR:
		/* check width */
		if (out_param->width & 0x1)
			return E_VSP_PARA_OUT_WIDTH;

		/* check x_offset */
		if (out_param->x_offset & 0x1)
			return E_VSP_PARA_OUT_XOFFSET;

		x_offset_c >>= 1;
		/* break; */
	case VSP_OUT_YUV444_PLANAR:
		/* check CbCr address pointer */
		if (out_param->addr_c0 == NULL)
			return E_VSP_PARA_OUT_ADRC0;

		if (out_param->addr_c1 == NULL)
			return E_VSP_PARA_OUT_ADRC1;

		/* set address */
		ch_info->val_addr_y =
			(unsigned long)out_param->addr +
			(y_offset * stride) +
			x_offset;

		temp = (y_offset_c * stride_c) + x_offset_c;

		ch_info->val_addr_c0 = (unsigned long)out_param->addr_c0 + temp;
		ch_info->val_addr_c1 = (unsigned long)out_param->addr_c1 + temp;
		break;
	default:
		return E_VSP_PARA_OUT_FORMAT;
		break;
	}

	return 0;
}

/*
 * vsp_ins_checK_rpf_clut_param - Check RPF color lookup table parameter routine
 * @rpf_info:	RPF information
 * @param:		input parameter
 *
 */
static long vsp_ins_checK_rpf_clut_param(
	struct vsp_rpf_info *rpf_info, T_VSP_IN *param)
{
	T_VSP_OSDLUT *clut_param = param->osd_lut;

	if ((param->format == VSP_IN_RGB_CLUT_DATA) ||
		(param->format == VSP_IN_YUV_CLUT_DATA)) {

		if (clut_param != NULL) {
			/* check pointer */
			if (clut_param->clut == NULL)
				return E_VSP_PARA_OSD_CLUT;

			/* check size */
			if ((clut_param->size < 1) || (clut_param->size > 256))
				return E_VSP_PARA_OSD_SIZE;
		}
	}

	return 0;
}

/*
 * vsp_ins_check_alpha_blend_param - Check alpha blend parameter routine
 * @rpf_info:		RPF information
 * @param:			alpha blend parameter
 *
 */
static long vsp_ins_check_alpha_blend_param(
	struct vsp_rpf_info *rpf_info, T_VSP_IN *param)
{
	T_VSP_ALPHA *alpha_param = param->alpha_blend;
	T_VSP_CLRCNV *clrcnv_param = param->clrcnv;

	unsigned char use_alpha_plane = 0;

	/* check pointer */
	if (alpha_param == NULL)
		return E_VSP_PARA_IN_ALPHA;

	/* initialise value */
	rpf_info->val_mskctrl = 0;
	rpf_info->val_mskset[0] = 0;
	rpf_info->val_mskset[1] = 0;

	rpf_info->val_ckey_ctrl = 0;
	rpf_info->val_ckey_set[0] = 0;
	rpf_info->val_ckey_set[1] = 0;

	rpf_info->val_addr_ai = 0;
	rpf_info->val_astride = 0;

	/* check virtual input parameter */
	if (param->vir == VSP_NO_VIR) {
		if (alpha_param->alphan == VSP_ALPHA_NO) {
			if (clrcnv_param != NULL) {
				rpf_info->val_ckey_ctrl |= VSP_RPF_CKEY_CTRL_CV;
				rpf_info->val_ckey_set[0] =
					clrcnv_param->color1;
				rpf_info->val_ckey_set[1] =
					clrcnv_param->color2;
			}
		} else {
			rpf_info->val_ckey_ctrl =
				(alpha_param->alphan & VSP_RPF_CKEY_CTRL_MSK);
			if (rpf_info->val_ckey_ctrl == 0)
				return E_VSP_PARA_ALPHA_ALPHAN;

			if (alpha_param->alphan & VSP_ALPHA_AL1)
				rpf_info->val_ckey_set[0] = alpha_param->alpha1;

			if (alpha_param->alphan & VSP_ALPHA_AL2)
				rpf_info->val_ckey_set[1] = alpha_param->alpha2;
		}

		if (alpha_param->asel == VSP_ALPHA_NUM5) {
			rpf_info->val_vrtcol =
				((unsigned long)alpha_param->afix) << 24;
		}

	} else {	/* param->vir == VSP_VIR */
		if (alpha_param->asel != VSP_ALPHA_NUM5)
			return E_VSP_PARA_ALPHA_ASEL;
	}

	/* check alpha blend parameter */
	rpf_info->val_alpha_sel = ((unsigned long)alpha_param->asel) << 28;
	switch (alpha_param->asel) {
	case VSP_ALPHA_NUM1:	/* 1,4 or 8-bit packed alpha + plane alpha */
	case VSP_ALPHA_NUM3:	/* 1-bit packed alpha + plane alpha */
		if (alpha_param->asel == VSP_ALPHA_NUM1) {
			/* check 8bit transparent-alpha generator */
			if ((alpha_param->aext != VSP_AEXT_EXPAN) &&
				(alpha_param->aext != VSP_AEXT_COPY) &&
				(alpha_param->aext != VSP_AEXT_EXPAN_MAX)) {
				return E_VSP_PARA_ALPHA_AEXT;
			}

			/* set 8bit transparent-alpha generator */
			rpf_info->val_alpha_sel |=
				((unsigned long)alpha_param->aext) << 18;
		} else {	/* alpha_param->asel == VSP_ALPHA_NUM3 */
			/* set 8bit transparent-alpha generator */
			rpf_info->val_alpha_sel |=
				((unsigned long)alpha_param->anum1) << 8;
			rpf_info->val_alpha_sel |=
				(unsigned long)alpha_param->anum0;
		}

		/* check 1bit mask-alpha generator */
		if (alpha_param->msken == VSP_MSKEN_ALPHA) {
			if (alpha_param->bsel == VSP_ALPHA_8BIT)
				use_alpha_plane = 1;
			else if (alpha_param->bsel == VSP_ALPHA_1BIT)
				use_alpha_plane = 8;
			else
				return E_VSP_PARA_ALPHA_BSEL;

			rpf_info->val_alpha_sel |=
				((unsigned long)alpha_param->bsel) << 23;

			if ((alpha_param->irop == VSP_IROP_NOP) ||
				(alpha_param->irop == VSP_IROP_CLEAR) ||
				(alpha_param->irop == VSP_IROP_INVERT) ||
				(alpha_param->irop == VSP_IROP_SET)) {
				/* don't use external memory */
				use_alpha_plane = 0;
			}
		} else if (alpha_param->msken == VSP_MSKEN_COLOR) {
			rpf_info->val_mskctrl =
				VSP_RPF_MSKCTRL_EN | alpha_param->mgcolor;
		} else {
			return E_VSP_PARA_ALPHA_MSKEN;
		}
		rpf_info->val_mskset[0] = alpha_param->mscolor0;
		rpf_info->val_mskset[1] = alpha_param->mscolor1;

		/* check IROP-SRC input parameter */
		if (alpha_param->irop >= VSP_IROP_MAX)
			return E_VSP_PARA_ALPHA_IROP;

		rpf_info->val_alpha_sel |=
			((unsigned long)alpha_param->irop) << 24;

		break;
	case VSP_ALPHA_NUM2:	/* 8-bit plane alpha */
	case VSP_ALPHA_NUM4:	/* 1-bit plane alpha */
		if (alpha_param->asel == VSP_ALPHA_NUM2) {
			use_alpha_plane = 1;
		} else {	/* alpha_param->asel == VSP_ALPHA_NUM4 */
			/* set 8bit transparent-alpha generator */
			rpf_info->val_alpha_sel |=
				((unsigned long)alpha_param->anum1) << 8;
			rpf_info->val_alpha_sel |=
				(unsigned long)alpha_param->anum0;

			use_alpha_plane = 8;
		}

		/* check 1bit mask-alpha generator */
		if (alpha_param->msken == VSP_MSKEN_ALPHA) {
			if (alpha_param->irop != VSP_IROP_NOP)
				return E_VSP_PARA_ALPHA_IROP;
		} else if (alpha_param->msken == VSP_MSKEN_COLOR) {
			rpf_info->val_mskctrl =
				VSP_RPF_MSKCTRL_EN | alpha_param->mgcolor;
		} else {
			return E_VSP_PARA_ALPHA_MSKEN;
		}
		rpf_info->val_mskset[0] = alpha_param->mscolor0;
		rpf_info->val_mskset[1] = alpha_param->mscolor1;

		/* check IROP-SRC input parameter */
		if (alpha_param->irop >= VSP_IROP_MAX)
			return E_VSP_PARA_ALPHA_IROP;

		rpf_info->val_alpha_sel |=
			((unsigned long)alpha_param->irop) << 24;

		break;
	case VSP_ALPHA_NUM5:	/* fixed alpha */
		/* check irop operation parameter */
		if (alpha_param->irop != VSP_IROP_NOP)
			return E_VSP_PARA_ALPHA_IROP;
		break;
	default:
		return E_VSP_PARA_ALPHA_ASEL;
		break;
	}

	if (use_alpha_plane) {
		/* check alpha plane buffer pointer */
		if (alpha_param->addr_a == NULL)
			return E_VSP_PARA_ALPHA_ADR;

		/* check x_offset parameter */
		if (param->x_offset % use_alpha_plane)
			return E_VSP_PARA_IN_XOFFSET;

		/* set address and stride value */
		rpf_info->val_addr_ai =
			(unsigned long)alpha_param->addr_a +
			((unsigned long)alpha_param->astride) *
			((unsigned long)param->y_offset) +
			(unsigned long)(param->x_offset / use_alpha_plane);
		rpf_info->val_astride = (unsigned long)alpha_param->astride;

		/* update swap value */
		rpf_info->val_dswap |= ((unsigned long)alpha_param->aswap) << 8;
	}

	return 0;
}

/*
 * vsp_ins_check_rpf_param - Check RPF parameter routine
 * @ch_info:		channel information
 * @rpf_info:		RPF information
 * @in_param:		input parameter
 *
 */
static long vsp_ins_check_rpf_param(struct vsp_ch_info *ch_info,
	struct vsp_rpf_info *rpf_info, T_VSP_IN *in_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];
	unsigned long width, height;
	long ercd;

	/* initialise */
	rpf_info->val_infmt = 0;
	rpf_info->val_vrtcol = 0;

	/* check pointer */
	if (in_param == NULL)
		return E_VSP_PARA_NOIN;

	/* check virtual parameter */
	if (in_param->vir == VSP_NO_VIR) {
		/* check color space conversion parameter */
		if (in_param->csc == VSP_CSC_OFF) {
			/* no process */
		} else if (in_param->csc == VSP_CSC_ON) {
			rpf_info->val_infmt |= VSP_RPF_INFMT_CSC;

			if (in_param->iturbt == VSP_ITURBT_601) {
				/* no process */
				/* dummy line */
			} else if (in_param->iturbt == VSP_ITURBT_709) {
				rpf_info->val_infmt |= VSP_RPF_INFMT_RDTM1;
			} else {
				return E_VSP_PARA_IN_ITURBT;
			}

			if (in_param->clrcng == VSP_ITU_COLOR) {
				/* no process */
				/* dummy line */
			} else if (in_param->clrcng == VSP_FULL_COLOR) {
				rpf_info->val_infmt |= VSP_RPF_INFMT_RDTM0;
			} else {
				return E_VSP_PARA_IN_CLRCNG;
			}
		} else {
			return E_VSP_PARA_IN_CSC;
		}
	} else if (in_param->vir == VSP_VIR) {
		rpf_info->val_infmt |= VSP_RPF_INFMT_VIR;
		rpf_info->val_vrtcol = in_param->vircolor;

		/* check color space conversion parameter */
		if (in_param->csc != VSP_CSC_OFF)
			return E_VSP_PARA_IN_CSC;
	} else {
		return E_VSP_PARA_IN_VIR;
	}

	/* check horizontal chrominance interpolation method parameter */
	if ((in_param->cipm != VSP_CIPM_0_HOLD) &&
		(in_param->cipm != VSP_CIPM_BI_LINEAR))
			return E_VSP_PARA_IN_CIPM;
	rpf_info->val_infmt |= ((unsigned long)in_param->cipm) << 16;

	/* check lower-bit color data extension method parameter */
	if ((in_param->cext != VSP_CEXT_EXPAN) &&
		(in_param->cext != VSP_CEXT_COPY) &&
		(in_param->cext != VSP_CEXT_EXPAN_MAX))
			return E_VSP_PARA_IN_CEXT;
	rpf_info->val_infmt |= ((unsigned long)in_param->cext) << 12;

	/* check format parameter */
	ercd = vsp_ins_check_rpf_format(rpf_info, in_param);
	if (ercd)
		return ercd;

	rpf_info->val_infmt |=
		(unsigned long)(in_param->format & VSP_RPF_INFMT_MSK);

	/* set color space */
	if (((in_param->format & 0x40) == 0x40) ^
		 (in_param->csc == VSP_CSC_ON))
		src_info->color = VSP_COLOR_YUV;
	else
		src_info->color = VSP_COLOR_RGB;

	/* set data swapping parameter */
	rpf_info->val_dswap = (unsigned long)(in_param->swap);

	/* check basic area */
	if ((in_param->width < 1) || (in_param->width > 8190))
		return E_VSP_PARA_IN_WIDTH;

	if ((in_param->height < 1) || (in_param->height > 8190))
		return E_VSP_PARA_IN_HEIGHT;

	/* check extended area */
	if (in_param->width_ex == 0) {
		width = (unsigned long)in_param->width;
	} else if ((in_param->width_ex < in_param->width) ||
			(in_param->width_ex > 8190)) {
		return E_VSP_PARA_IN_WIDTHEX;
	} else {
		width = (unsigned long)in_param->width_ex;
	}

	if (in_param->height_ex == 0) {
		height = (unsigned long)in_param->height;
	} else if ((in_param->height_ex < in_param->height) ||
			(in_param->height_ex > 8190)) {
		return E_VSP_PARA_IN_HEIGHTEX;
	} else {
		height = (unsigned long)in_param->height_ex;
	}
	rpf_info->val_esize = (width << 16) | height;

	src_info->width = width;
	src_info->height = height;

	/* check source RPF register */
	if (in_param->pwd == VSP_LAYER_PARENT) {
		width = 0;
		height = 0;
	} else if (in_param->pwd == VSP_LAYER_CHILD) {
		width = (unsigned long)in_param->x_position;
		height = (unsigned long)in_param->y_position;
	} else {
		return E_VSP_PARA_IN_PWD;
	}
	rpf_info->val_loc = (width << 16) | height;

	src_info->master = (unsigned long)in_param->pwd;
	src_info->x_position = width;
	src_info->y_position = height;

	/* check input color look up table parameter */
	ercd = vsp_ins_checK_rpf_clut_param(rpf_info, in_param);
	if (ercd)
		return ercd;

	/* check alpha blend and color converter parameter */
	ercd = vsp_ins_check_alpha_blend_param(rpf_info, in_param);
	if (ercd)
		return ercd;

	/* check connect parameter */
	if (in_param->connect & ~VSP_RPF_USABLE_DPR)
		return E_VSP_PARA_IN_CONNECT;

	/* get DPR value */
	rpf_info->val_dpr =
		vsp_ins_get_dpr_route(ch_info, in_param->connect, 0);
	if (rpf_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_IN_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_wpf_param - Check output(WPF) parameter routine
 * @ch_info:		channel information
 * @out_param:		output parameter
 *
 */
static long vsp_ins_check_wpf_param(
	struct vsp_ch_info *ch_info, T_VSP_OUT *out_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];
	unsigned char exp_color;
	long ercd;

	/* check pointer */
	if (out_param == NULL)
		return E_VSP_PARA_OUTPAR;

	/* check input color space */
	if (src_info->color == VSP_COLOR_HSV)
		return E_VSP_PARA_OUT_INHSV;

	/* check input image size */
	if (src_info->width > 2048)
		return E_VSP_PARA_OUT_INWIDTH;

	if (src_info->height > 2048)
		return E_VSP_PARA_OUT_INHEIGHT;

	/* check format parameter */
	ercd = vsp_ins_check_wpf_format(ch_info, out_param);
	if (ercd)
		return ercd;

	ch_info->val_srcrpf = src_info->master;
	ch_info->val_outfmt
		= (unsigned long)(out_param->format & VSP_WPF_OUTFMT_MSK);

	/* check PAD data parameter */
	if (out_param->pxa == VSP_PAD_P)
		ch_info->val_outfmt |= ((unsigned long)(out_param->pad)) << 24;
	else if (out_param->pxa == VSP_PAD_IN)
		ch_info->val_outfmt |= VSP_WPF_OUTFMT_PXA;
	else
		return E_VSP_PARA_OUT_PXA;

	/* check dithering parameter */
	if (out_param->dith == VSP_NO_DITHER) {
		/* no process */
		/* dummy line */
	} else if (out_param->dith == VSP_DITHER) {
		ch_info->val_outfmt |= VSP_WPF_OUTFMT_DITH;
	} else {
		return E_VSP_PARA_OUT_DITH;
	}

	/* check color space conversion parameter */
	if (out_param->csc == VSP_CSC_OFF) {
		/* no process */
	} else if (out_param->csc == VSP_CSC_ON) {
		ch_info->val_outfmt |= VSP_WPF_OUTFMT_CSC;

		if (out_param->iturbt == VSP_ITURBT_601) {
			/* no process */
			/* dummy line */
		} else if (out_param->iturbt == VSP_ITURBT_709) {
			ch_info->val_outfmt |= VSP_WPF_OUTFMT_WRTM1;
		} else {
			return E_VSP_PARA_OUT_ITURBT;
		}

		if (out_param->clrcng == VSP_ITU_COLOR) {
			/* no process */
			/* dummy line */
		} else if (out_param->clrcng == VSP_FULL_COLOR) {
			ch_info->val_outfmt |= VSP_WPF_OUTFMT_WRTM0;
		} else {
			return E_VSP_PARA_OUT_CLRCNG;
		}
	} else {
		return E_VSP_PARA_OUT_CSC;
	}

	/* set expectation color space */
	if (((out_param->format & 0x40) == 0x40) ^
		 (out_param->csc == VSP_CSC_ON))
		exp_color = VSP_COLOR_YUV;
	else
		exp_color = VSP_COLOR_RGB;

	/* check expectation color space */
	if (src_info->color != VSP_COLOR_NO) {
		if (src_info->color != exp_color)
			return E_VSP_PARA_OUT_NOTCOLOR;
	}

	/* check clipping paremeter */
	if (out_param->x_coffset > 255)
		return E_VSP_PARA_OUT_XCOFFSET;

	if (out_param->width == 0)
		return E_VSP_PARA_OUT_WIDTH;

	if (out_param->x_coffset + out_param->width > src_info->width)
		return E_VSP_PARA_OUT_XCLIP;

	if (out_param->y_coffset > 255)
		return E_VSP_PARA_OUT_YCOFFSET;

	if (out_param->height == 0)
		return E_VSP_PARA_OUT_HEIGHT;

	if (out_param->y_coffset + out_param->height > src_info->height)
		return E_VSP_PARA_OUT_YCLIP;

	/* check rounding control parameter */
	if ((out_param->cbrm != VSP_CSC_ROUND_DOWN) &&
		(out_param->cbrm != VSP_CSC_ROUND_OFF))
			return E_VSP_PARA_OUT_CBRM;

	if ((out_param->abrm != VSP_CONVERSION_ROUNDDOWN) &&
		(out_param->abrm != VSP_CONVERSION_ROUNDING) &&
		(out_param->abrm != VSP_CONVERSION_THRESHOLD))
			return E_VSP_PARA_OUT_ABRM;

	if ((out_param->clmd != VSP_CLMD_NO) &&
		(out_param->clmd != VSP_CLMD_MODE1) &&
		(out_param->clmd != VSP_CLMD_MODE2))
			return E_VSP_PARA_OUT_CLMD;

	return 0;
}

/*
 * vsp_ins_check_sru_param - check SRU parameter routine
 * @ch_info:		channel information
 * @sru_info:		SRU information
 * @sru_param:		SRU parameter
 *
 */
static long vsp_ins_check_sru_param(struct vsp_ch_info *ch_info,
	struct vsp_sru_info *sru_info, T_VSP_SRU *sru_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];

	/* check pointer */
	if (sru_param == NULL)
		return E_VSP_PARA_NOSRU;

	/* check input color space */
	if (src_info->color == VSP_COLOR_HSV)
		return E_VSP_PARA_SRU_INHSV;

	/* check mode */
	if (sru_param->mode == VSP_SRU_MODE1) {
		/* no process */
	} else if (sru_param->mode == VSP_SRU_MODE2) {
		src_info->width <<= 1;
		src_info->height <<= 1;
	} else {
		return E_VSP_PARA_SRU_MODE;
	}

	/* check image size (after scale-up) */
	if ((src_info->width < 4) || (src_info->width > 2048))
		return E_VSP_PARA_SRU_WIDTH;

	if ((src_info->height < 4) || (src_info->height > 8190))
		return E_VSP_PARA_SRU_HEIGHT;

	/* check param */
	if (sru_param->param & (~(VSP_SRU_RCR|VSP_SRU_GY|VSP_SRU_BCB)))
		return E_VSP_PARA_SRU_PARAM;

	/* check enscl */
	if (sru_param->enscl >= VSP_SCL_LEVEL_MAX)
		return E_VSP_PARA_SRU_ENSCL;

	/* check connect parameter */
	if (sru_param->connect & ~VSP_SRU_USABLE_DPR)
		return E_VSP_PARA_SRU_CONNECT;

	/* check connect parameter */
	sru_info->val_dpr = vsp_ins_get_dpr_route(
		ch_info, sru_param->connect, sru_param->fxa);
	if (sru_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_SRU_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_uds_param - check UDS parameter routine
 * @ch_info:		channel information
 * @uds_info:		UDS information
 * @uds_param:		UDS parameter
 *
 */
static long vsp_ins_check_uds_param(struct vsp_ch_info *ch_info,
	struct vsp_uds_info *uds_info, T_VSP_UDS *uds_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];

	/* initialise */
	uds_info->val_ctrl = 0;
	uds_info->val_pass = 0;
	uds_info->val_clip = 0;

	/* check pointer */
	if (uds_param == NULL)
		return E_VSP_PARA_NOUDS;

	/* check input image size */
	if (src_info->width < 4)
		return E_VSP_PARA_UDS_INWIDTH;

	if ((src_info->height < 4) || (src_info->height > 8190))
		return E_VSP_PARA_UDS_INHEIGHT;

	/* check scaling factor parameter */
	if (uds_param->x_ratio < VSP_UDS_SCALE_16_1) {
		return E_VSP_PARA_UDS_XRATIO;
	} else if (uds_param->x_ratio <= VSP_UDS_SCALE_1_1) {
		/* scale up or same size */
		if (src_info->width > 2048)
			return E_VSP_PARA_UDS_INWIDTH;
	} else {
		/* scale down */
		if (src_info->width > 8190)
			return E_VSP_PARA_UDS_INWIDTH;
	}

	if (uds_param->y_ratio < VSP_UDS_SCALE_16_1)
		return E_VSP_PARA_UDS_YRATIO;

	/* check clipping size */
	if ((uds_param->out_cwidth < 4) || (uds_param->out_cwidth > 2048))
		return E_VSP_PARA_UDS_OUTCWIDTH;

	if ((uds_param->out_cheight < 4) || (uds_param->out_cheight > 2048))
		return E_VSP_PARA_UDS_OUTCHEIGHT;

	uds_info->val_clip = ((unsigned long)uds_param->out_cwidth) << 16;
	uds_info->val_clip |= (unsigned long)uds_param->out_cheight;

	/* update image size */
	src_info->width = uds_param->out_cwidth;
	src_info->height = uds_param->out_cheight;

	/* calculate passband parameter */
	uds_info->val_pass =
		vsp_ins_get_passband_bwidth(uds_param->x_ratio) << 16;
	uds_info->val_pass |= vsp_ins_get_passband_bwidth(uds_param->y_ratio);

	/* check pixel count at scale-up parameter */
	if (uds_param->amd == VSP_AMD_NO) {
		/* no process */
		/* dummy line */
	} else if (uds_param->amd == VSP_AMD) {
		uds_info->val_ctrl |= VSP_UDS_CTRL_AMD;
	} else {
		return E_VSP_PARA_UDS_AMD;
	}

	/* check padding for insufficient clipping size parameter */
	if (uds_param->fmd == VSP_FMD_NO) {
		/* no process */
		/* dummy line */
	} else if (uds_param->fmd == VSP_FMD) {
		uds_info->val_ctrl |= VSP_UDS_CTRL_FMD;
	} else {
		return E_VSP_PARA_UDS_FMD;
	}

	/* check scale-up/down of alpha plane parameter */
	if (uds_param->alpha == VSP_ALPHA_OFF) {
		/* no process */
	} else if (uds_param->alpha == VSP_ALPHA_ON) {
		uds_info->val_ctrl |= VSP_UDS_CTRL_AON;

		/* alpha output threshold comparison parameter */
		if (uds_param->clip == VSP_CLIP_OFF) {
			/* no process */
			/* dummy line */
		} else if (uds_param->clip == VSP_CLIP_ON) {
			uds_info->val_ctrl |= VSP_UDS_CTRL_ATHON;
		} else {
			return E_VSP_PARA_UDS_CLIP;
		}
	} else {
		return E_VSP_PARA_UDS_ALPHA;
	}

	if (uds_param->complement == VSP_COMPLEMENT_BIL) {
		/* no process */
	} else if (uds_param->complement == VSP_COMPLEMENT_NN) {
		/* when under quarter, cannot use nearest neighbor */
		if ((uds_param->x_ratio > VSP_UDS_SCALE_1_4) ||
			(uds_param->y_ratio > VSP_UDS_SCALE_1_4))
			return E_VSP_PARA_UDS_COMP;

		uds_info->val_ctrl |= VSP_UDS_CTRL_NN;
	} else if (uds_param->complement == VSP_COMPLEMENT_BC) {
		/* when alpha scale up/down is performed, can't use multi tap */
		if (uds_param->alpha == VSP_ALPHA_ON)
			return E_VSP_PARA_UDS_COMP;

		uds_info->val_ctrl |= VSP_UDS_CTRL_BC;
	} else {
		return E_VSP_PARA_UDS_COMP;
	}

	/* check connect parameter */
	if (uds_param->connect & ~VSP_UDS_USABLE_DPR)
		return E_VSP_PARA_UDS_CONNECT;

	/* check connect parameter */
	uds_info->val_dpr =
		vsp_ins_get_dpr_route(ch_info, uds_param->connect, 0);
	if (uds_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_UDS_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_lut_param - check LUT parameter routine
 * @ch_info:		channel information
 * @lut_info:		LUT information
 * @lut_param:		LUT parameter
 *
 */
static long vsp_ins_check_lut_param(struct vsp_ch_info *ch_info,
	struct vsp_lut_info *lut_info, T_VSP_LUT *lut_param)
{
	/* check pointer */
	if (lut_param == NULL)
		return E_VSP_PARA_NOLUT;

	/* check lut pointer */
	if (lut_param->lut == NULL)
		return E_VSP_PARA_LUT;

	/* check size */
	if ((lut_param->size < 1) || (lut_param->size > 256))
		return E_VSP_PARA_LUT_SIZE;

	/* check connect parameter */
	if (lut_param->connect & ~VSP_LUT_USABLE_DPR)
		return E_VSP_PARA_LUT_CONNECT;

	/* check connect parameter */
	lut_info->val_dpr = vsp_ins_get_dpr_route(
		ch_info, lut_param->connect, lut_param->fxa);
	if (lut_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_LUT_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_clu_param - check CLU parameter routine
 * @ch_info:		channel information
 * @clu_info:		CLU information
 * @clu_param:		CLU parameter
 *
 */
static long vsp_ins_check_clu_param(struct vsp_ch_info *ch_info,
	struct vsp_clu_info *clu_info, T_VSP_CLU *clu_param)
{
	unsigned long *clu_addr;
	unsigned long *clu_data;
	short i;

	/* initialise */
	clu_info->val_ctrl = 0;

	/* check pointer */
	if (clu_param == NULL)
		return E_VSP_PARA_NOCLU;

	clu_addr = clu_param->clu_addr;
	clu_data = clu_param->clu_data;

	/* check clu data pointer */
	if (clu_data == NULL)
		return E_VSP_PARA_CLU_DATA;

	/* check mode */
	if ((clu_param->mode == VSP_CLU_MODE_3D) ||
		(clu_param->mode == VSP_CLU_MODE_3D_AUTO)) {

		/* check size */
		if ((clu_param->size < 1) || (clu_param->size > 4913))
			return E_VSP_PARA_CLU_SIZE;

		/* check address */
		if (clu_param->mode == VSP_CLU_MODE_3D) {
			/* check clu addr pointer */
			if (clu_addr == NULL)
				return E_VSP_PARA_CLU_ADR;

			/* check clu addr value */
			for (i = 0; i < clu_param->size; i++) {
				volatile unsigned long tmp;
				tmp = *clu_addr++;
				if (((tmp & 0xff) > 0x10) ||
					((tmp & 0xff00) > 0x1000) ||
					((tmp & 0xff0000) > 0x100000) ||
					((tmp & 0xff000000) != 0)) {
					return E_VSP_PARA_CLU_ADR;
				}
			}
		} else {
			/* enable Automatic table Address Increment */
			clu_info->val_ctrl |= VSP_CLU_CTRL_AAI;
		}

		/* check data */
		for (i = 0; i < clu_param->size; i++) {
			volatile unsigned long tmp;
			tmp = *clu_data++;
			if ((tmp & 0xff000000) != 0)
				return E_VSP_PARA_CLU_DATA;
		}

		clu_info->val_ctrl |=
			(VSP_CLU_CTRL_MVS | VSP_CLU_CTRL_3D | VSP_CLU_CTRL_EN);
	} else if ((clu_param->mode == VSP_CLU_MODE_2D) ||
		(clu_param->mode == VSP_CLU_MODE_2D_AUTO)) {

		/* check size */
		if ((clu_param->size < 1) || (clu_param->size > 289))
			return E_VSP_PARA_CLU_SIZE;

		/* check address */
		if (clu_param->mode == VSP_CLU_MODE_2D) {
			/* check clu addr pointer */
			if (clu_addr == NULL)
				return E_VSP_PARA_CLU_ADR;

			/* check clu addr value */
			for (i = 0; i < clu_param->size; i++) {
				volatile unsigned long tmp;
				tmp = *clu_addr++;
				if (((tmp & 0xff00) > 0x1000) ||
					((tmp & 0xff0000) > 0x100000) ||
					((tmp & 0xff0000ff) != 0)) {
					return E_VSP_PARA_CLU_ADR;
				}
			}
		} else {
			/* enable Automatic table Address Increment */
			clu_info->val_ctrl |= VSP_CLU_CTRL_AAI;
		}

		/* check data */
		for (i = 0; i < clu_param->size; i++) {
			volatile unsigned long tmp;
			tmp = *clu_data++;
			if ((tmp & 0xffff00ff) != 0)
				return E_VSP_PARA_CLU_DATA;
		}

		clu_info->val_ctrl |=
			(VSP_CLU_CTRL_MVS | VSP_CLU_CTRL_2D | VSP_CLU_CTRL_EN);
	} else {
		return E_VSP_PARA_CLU_MODE;
	}

	/* check connect parameter */
	if (clu_param->connect & ~VSP_CLU_USABLE_DPR)
		return E_VSP_PARA_CLU_CONNECT;

	/* check connect parameter */
	clu_info->val_dpr = vsp_ins_get_dpr_route(
		ch_info, clu_param->connect, clu_param->fxa);
	if (clu_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_CLU_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_hst_param - check HST parameter routine
 * @ch_info:		channel information
 * @hst_info:		HST information
 * @hst_param:		HST parameter
 *
 */
static long vsp_ins_check_hst_param(struct vsp_ch_info *ch_info,
	struct vsp_hst_info *hst_info, T_VSP_HST *hst_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];

	/* check pointer */
	if (hst_param == NULL)
		return E_VSP_PARA_NOHST;

	/* check input color space */
	if (src_info->color != VSP_COLOR_RGB)
		return E_VSP_PARA_HST_NOTRGB;

	/* check connect parameter */
	if (hst_param->connect & ~VSP_HST_USABLE_DPR)
		return E_VSP_PARA_HST_CONNECT;

	/* check connect parameter */
	hst_info->val_dpr = vsp_ins_get_dpr_route(
		ch_info, hst_param->connect, hst_param->fxa);
	if (hst_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_HST_CONNECT;

	/* change color space */
	src_info->color = VSP_COLOR_HSV;

	return 0;
}

/*
 * vsp_ins_check_hsi_param - check HSI parameter routine
 * @ch_info:		channel information
 * @hsi_info:		HSI information
 * @hsi_param:		HSI parameter
 *
 */
static long vsp_ins_check_hsi_param(struct vsp_ch_info *ch_info,
	struct vsp_hsi_info *hsi_info, T_VSP_HSI *hsi_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];

	/* check pointer */
	if (hsi_param == NULL)
		return E_VSP_PARA_NOHSI;

	/* check input color space */
	if (src_info->color != VSP_COLOR_HSV)
		return E_VSP_PARA_HSI_NOTHSV;

	/* check connect parameter */
	if (hsi_param->connect & ~VSP_HSI_USABLE_DPR)
		return E_VSP_PARA_HSI_CONNECT;

	/* check connect parameter */
	hsi_info->val_dpr = vsp_ins_get_dpr_route(
		ch_info, hsi_param->connect, hsi_param->fxa);
	if (hsi_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_HSI_CONNECT;

	/* change color space */
	src_info->color = VSP_COLOR_RGB;

	return 0;
}

/*
 * vsp_ins_check_blend_virtual_param - check BRU blend virtual parameter routine
 * @ch_info:		channel information
 * @bru_info:		BRU information
 * @vir_param:		Blend virtual parameter
 *
 */
static long vsp_ins_check_blend_virtual_param(struct vsp_ch_info *ch_info,
	struct vsp_bru_info *bru_info, T_VSP_BLEND_VIRTUAL *vir_param)
{
	struct vsp_src_info *src_info = &ch_info->src_info[ch_info->src_idx];
	unsigned long x_pos, y_pos;

	/* check pointer */
	if (vir_param == NULL)
		return E_VSP_PARA_VIR_ADR;

	/* check width */
	if ((vir_param->width < 1) || (vir_param->width > 8190))
		return E_VSP_PARA_VIR_WIDTH;

	/* check height */
	if ((vir_param->height < 1) || (vir_param->height > 8190))
		return E_VSP_PARA_VIR_HEIGHT;

	bru_info->val_vir_size = ((unsigned long)vir_param->width) << 16;
	bru_info->val_vir_size |= (unsigned long)vir_param->height;
	bru_info->val_vir_color = vir_param->color;

	/* check pwd */
	if (vir_param->pwd == VSP_LAYER_PARENT) {
		x_pos = 0;
		y_pos = 0;
	} else if (vir_param->pwd == VSP_LAYER_CHILD) {
		x_pos = (unsigned long)vir_param->x_position;
		y_pos = (unsigned long)vir_param->y_position;
	} else {
		return E_VSP_PARA_VIR_PWD;
	}
	bru_info->val_vir_loc = (x_pos << 16) | y_pos;

	src_info->color = VSP_COLOR_NO;
	src_info->master = (unsigned long)vir_param->pwd;
	src_info->width = (unsigned long)vir_param->width;
	src_info->height = (unsigned long)vir_param->height;
	src_info->x_position = x_pos;
	src_info->y_position = y_pos;

	/* update source counter */
	src_info->rpf_ch = 14;		/* virtual source in BRU */
	ch_info->src_idx++;
	ch_info->src_cnt++;

	return 0;
}

/*
 * vsp_ins_check_blend_control_param - check BRU blend control parameter routine
 * @bru_ctrl:		BRU control register value
 * @bru_bld:		BRU blend control register value
 * @dst_layer:		destination layer(VSP_LAY_NO/1/2/3/4/VIRTUAL)
 * @src_layer:		source layer(VSP_LAY_NO/1/2/3/4/VIRTUAL)
 * @ctrl_param:		Blend control parameter
 *
 */
static long vsp_ins_check_blend_control_param(
	unsigned long *bru_ctrl, unsigned long *bru_bld,
	unsigned char dst_layer, unsigned char src_layer,
	T_VSP_BLEND_CONTROL *ctrl_param)
{
	/* initialise */
	if (dst_layer != VSP_LAY_NO)
		*bru_ctrl = ((unsigned long)(dst_layer-1)) << 20;
	else
		*bru_ctrl = 0;

	*bru_bld = 0;

	if ((ctrl_param != NULL) && (src_layer != VSP_LAY_NO)) {
		/* check rbc */
		if ((ctrl_param->rbc != VSP_RBC_ROP) &&
			(ctrl_param->rbc != VSP_RBC_BLEND))
			return E_VSP_PARA_BLEND_RBC;

		/* check crop */
		if (ctrl_param->crop >= VSP_IROP_MAX)
			return E_VSP_PARA_BLEND_CROP;

		/* check arop */
		if (ctrl_param->arop >= VSP_IROP_MAX)
			return E_VSP_PARA_BLEND_AROP;

		/* check blend_formula */
		if ((ctrl_param->blend_formula != VSP_FORM_BLEND0) &&
			(ctrl_param->blend_formula != VSP_FORM_BLEND1))
				return E_VSP_PARA_BLEND_FORM;

		/* check blend_corfx */
		if (ctrl_param->blend_coefx >= VSP_COEFFICIENT_BLENDX_MAX)
			return E_VSP_PARA_BLEND_COEFX;

		/* check blend_corfy */
		if (ctrl_param->blend_coefy >= VSP_COEFFICIENT_BLENDY_MAX)
			return E_VSP_PARA_BLEND_COEFY;

		/* check aformula */
		if ((ctrl_param->aformula != VSP_FORM_ALPHA0) &&
			(ctrl_param->aformula != VSP_FORM_ALPHA1))
				return E_VSP_PARA_BLEND_AFORM;

		/* check acoefx */
		if (ctrl_param->acoefx >= VSP_COEFFICIENT_ALPHAX_MAX)
			return E_VSP_PARA_BLEND_ACOEFX;

		/* check acoefy */
		if (ctrl_param->acoefy >= VSP_COEFFICIENT_ALPHAY_MAX)
			return E_VSP_PARA_BLEND_ACOEFY;

		*bru_ctrl |= ((unsigned long)ctrl_param->rbc) << 31;
		*bru_ctrl |= ((unsigned long)(src_layer-1)) << 16;
		*bru_ctrl |= ((unsigned long)ctrl_param->crop) << 4;
		*bru_ctrl |=  (unsigned long)ctrl_param->arop;

		*bru_bld |= ((unsigned long)ctrl_param->blend_formula) << 31;
		*bru_bld |= ((unsigned long)ctrl_param->blend_coefx) << 28;
		*bru_bld |= ((unsigned long)ctrl_param->blend_coefy) << 24;
		*bru_bld |= ((unsigned long)ctrl_param->aformula) << 23;
		*bru_bld |= ((unsigned long)ctrl_param->acoefx) << 20;
		*bru_bld |= ((unsigned long)ctrl_param->acoefy) << 16;
		*bru_bld |= ((unsigned long)ctrl_param->acoefx_fix) << 8;
		*bru_bld |=  (unsigned long)ctrl_param->acoefy_fix;
	}

	return 0;
}

/*
 * vsp_ins_check_blend_rop_param - check BRU Raster Opration parameter routine
 * @bru_rop:		Raster opration control register value
 * @layer:			layer(VSP_LAY_NO/1/2/3/4/VIRTUAL)
 * @rop_param:		Raster opration parameter
 *
 */
static long vsp_ins_check_blend_rop_param(unsigned long *bru_rop,
	unsigned char layer, T_VSP_BLEND_ROP *rop_param)
{
	/* initialise */
	if (layer != VSP_LAY_NO)
		*bru_rop = ((unsigned long)(layer-1)) << 20;
	else
		*bru_rop = 0;

	if (rop_param != NULL) {
		/* check crop */
		if (rop_param->crop >= VSP_IROP_MAX)
			return E_VSP_PARA_ROP_CROP;

		/* check arop */
		if (rop_param->arop >= VSP_IROP_MAX)
			return E_VSP_PARA_ROP_AROP;

		*bru_rop |= ((unsigned long)rop_param->crop) << 4;
		*bru_rop |=  (unsigned long)rop_param->arop;
	};

	return 0;
}

/*
 * vsp_ins_check_bru_param - check BRU parameter routine
 * @ch_info:		channel information
 * @bru_info:		BRU information
 * @bru_param:		BRU parameter
 *
 */
static long vsp_ins_check_bru_param(struct vsp_ch_info *ch_info,
	struct vsp_bru_info *bru_info, T_VSP_BRU *bru_param)
{
	unsigned long order_tmp;
	unsigned long order_bit;
	unsigned char layer[VSP_BROP_MAX];

	unsigned char *qnt;
	unsigned char *dith;

	unsigned char i;
	long ercd;

	/* initialise */
	bru_info->val_inctrl = 0;

	bru_info->val_vir_loc = 0;
	bru_info->val_vir_color = 0;
	bru_info->val_vir_size = 0;

	/* check pointer */
	if (bru_param == NULL)
		return E_VSP_PARA_NOBRU;

	/* check input color space */
	ercd = vsp_ins_check_input_color_space_of_bru(ch_info);
	if (ercd)
		return ercd;

	/* check lay_order */
	order_tmp = bru_param->lay_order;
	order_bit = 0;
	for (i = 0; i < VSP_BROP_MAX; i++) {
		layer[i] = (unsigned char)(order_tmp & 0xf);
		order_bit |= (0x00000001UL << layer[i]); /* layer max 15 */

		order_tmp >>= 4;
	}
	order_bit >>= 1;

	if ((order_bit & 0xf) != ((0x00000001UL << ch_info->bru_cnt) - 1))
		return E_VSP_PARA_BRU_LAYORDER;

	if (layer[VSP_BROP_DST_A] == VSP_LAY_NO)
		return E_VSP_PARA_BRU_LAYORDER;

	/* check adiv */
	if ((bru_param->adiv != VSP_DIVISION_OFF) &&
		(bru_param->adiv != VSP_DIVISION_ON))
		return E_VSP_PARA_BRU_ADIV;

	bru_info->val_inctrl = ((unsigned long)bru_param->adiv) << 28;

	/* set dithering parameter */
	qnt = &bru_param->qnt[0];
	dith = &bru_param->dith[0];
	for (i = 0; i < ch_info->bru_cnt; i++) {
		if (*qnt == VSP_QNT_OFF) {
			/* no process */
		} else if (*qnt == VSP_QNT_ON) {
			if ((*dith != VSP_DITH_OFF) &&
				(*dith != VSP_DITH_18BPP) &&
				(*dith != VSP_DITH_16BPP) &&
				(*dith != VSP_DITH_15BPP) &&
				(*dith != VSP_DITH_12BPP) &&
				(*dith != VSP_DITH_8BPP)) {
				return E_VSP_PARA_BRU_DITH;
			}
			bru_info->val_inctrl |=
				(((unsigned long)*qnt) << (i+16));
			bru_info->val_inctrl |=
				(((unsigned long)*dith) << (i*4));
		} else {
			return E_VSP_PARA_BRU_QNT;
		}

		qnt++;
		dith++;
	}

	/* check blend virtual parameter */
	if (order_bit & 0x10) {
		ercd = vsp_ins_check_blend_virtual_param(
			ch_info, bru_info, bru_param->blend_virtual);
		if (ercd)
			return ercd;
	}

	/* check blend control UNIT A parameter */
	ercd = vsp_ins_check_blend_control_param(
		&bru_info->val_ctrl[0],
		&bru_info->val_bld[0],
		layer[VSP_BROP_DST_A],
		layer[VSP_BROP_SRC_A],
		bru_param->blend_control_a
	);
	if (ercd)
		return ercd;

	/* check blend ROP UNIT parameter */
	ercd = vsp_ins_check_blend_rop_param(
		&bru_info->val_rop,
		layer[VSP_BROP_DST_R],
		bru_param->blend_rop
	);
	if (ercd)
		return ercd;

	/* check blend control UNIT B parameter */
	if (layer[VSP_BROP_DST_R] != VSP_LAY_NO) {
		ercd = vsp_ins_check_blend_control_param(
			&bru_info->val_ctrl[1],
			&bru_info->val_bld[1],
			1,
			1,
			bru_param->blend_control_b
		);
		if (ercd)
			return ercd;
	} else {
		bru_info->val_ctrl[1] = 0;
		bru_info->val_bld[1] = 0;
	}

	/* check blend control UNIT C parameter */
	ercd = vsp_ins_check_blend_control_param(
		&bru_info->val_ctrl[2],
		&bru_info->val_bld[2],
		VSP_LAY_NO,
		layer[VSP_BROP_SRC_C],
		bru_param->blend_control_c
	);
	if (ercd)
		return ercd;

	/* check blend control UNIT D parameter */
	ercd = vsp_ins_check_blend_control_param(
		&bru_info->val_ctrl[3],
		&bru_info->val_bld[3],
		VSP_LAY_NO,
		layer[VSP_BROP_SRC_D],
		bru_param->blend_control_d
	);
	if (ercd)
		return ercd;

	/* check connect parameter */
	if (bru_param->connect & ~VSP_BRU_USABLE_DPR)
		return E_VSP_PARA_BRU_CONNECT;

	/* check connect parameter */
	bru_info->val_dpr =
		vsp_ins_get_dpr_route(ch_info, bru_param->connect, 0);
	if (bru_info->val_dpr == VSP_DPR_ROUTE_NOT_USE)
		return E_VSP_PARA_BRU_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_hgo_param - check HGO parameter routine
 * @ch_info:		channel information
 * @hgo_info:		HGO information
 * @hgo_param:		HGO parameter
 *
 */
static long vsp_ins_check_hgo_param(struct vsp_ch_info *ch_info,
	struct vsp_hgo_info *hgo_info, T_VSP_HGO *hgo_param)
{
	if (hgo_param == NULL)
		return E_VSP_PARA_NOHGO;

	/* check address pointer */
	if (hgo_param->addr == NULL)
		return E_VSP_PARA_HGO_ADR;

	if (((unsigned long)hgo_param->addr) & 0x3)
		return E_VSP_PARA_HGO_ADR;

	hgo_info->val_addr = (unsigned long)hgo_param->addr;

	/* check detection window area */
	if ((hgo_param->width < 1) || (hgo_param->width > 8190))
		return E_VSP_PARA_HGO_WIDTH;

	if ((hgo_param->height < 1) || (hgo_param->height > 8190))
		return E_VSP_PARA_HGO_HEIGHT;

	if ((hgo_param->width + hgo_param->x_offset) > 8190)
		return E_VSP_PARA_HGO_XOFFSET;

	if ((hgo_param->height + hgo_param->y_offset) > 8190)
		return E_VSP_PARA_HGO_YOFFSET;

	/* check binary mode */
	if ((hgo_param->binary_mode != VSP_STRAIGHT_BINARY) &&
		(hgo_param->binary_mode != VSP_OFFSET_BINARY))
			return E_VSP_PARA_HGO_BINMODE;

	/* check */
	if ((hgo_param->maxrgb_mode != VSP_MAXRGB_OFF) &&
		(hgo_param->maxrgb_mode != VSP_MAXRGB_ON))
			return E_VSP_PARA_HGO_MAXRGB;

	/* check skip mode */
	if ((hgo_param->x_skip != VSP_SKIP_OFF) &&
		(hgo_param->x_skip != VSP_SKIP_1_2) &&
		(hgo_param->x_skip != VSP_SKIP_1_4))
			return E_VSP_PARA_HGO_XSKIP;

	if ((hgo_param->y_skip != VSP_SKIP_OFF) &&
		(hgo_param->y_skip != VSP_SKIP_1_2) &&
		(hgo_param->y_skip != VSP_SKIP_1_4))
			return E_VSP_PARA_HGO_YSKIP;

	/* check sampling point */
	hgo_info->val_dpr = vsp_ins_get_dpr_smppt(ch_info, hgo_param->sampling);
	if (hgo_info->val_dpr == VSP_DPR_SMPPT_NOT_USE)
		return E_VSP_PARA_HGO_SMMPT;

	return 0;
}

/*
 * vsp_ins_check_hue_area_param - check HGT HUE area parameter routine
 * @hue_area:		Hue area parameter
 *
 */
static long vsp_ins_check_hue_area_param(T_VSP_HUE_AREA *hue_area)
{
	unsigned char val = hue_area[0].upper;
	int i;

	for (i = 1; i < 6; i++) {
		if (val > hue_area[i].lower)
			return E_VSP_PARA_HGT_AREA;

		if (hue_area[i].lower > hue_area[i].upper)
			return E_VSP_PARA_HGT_AREA;

		val = hue_area[i].upper;
	}

	if ((hue_area[0].upper < hue_area[0].lower) &&
		(hue_area[0].lower < hue_area[5].upper)) {
		return E_VSP_PARA_HGT_AREA;
	}

	return 0;
}

/*
 * vsp_ins_check_hgt_param - check HGT parameter routine
 * @ch_info:		channel information
 * @hgt_info:		HGT information
 * @hgt_param:		HGT parameter
 *
 */
static long vsp_ins_check_hgt_param(struct vsp_ch_info *ch_info,
	struct vsp_hgt_info *hgt_info, T_VSP_HGT *hgt_param)
{
	long ercd;

	if (hgt_param == NULL)
		return E_VSP_PARA_NOHGT;

	/* check address pointer */
	if (hgt_param->addr == NULL)
		return E_VSP_PARA_HGT_ADR;

	if (((unsigned long)hgt_param->addr) & 0x3)
		return E_VSP_PARA_HGT_ADR;

	hgt_info->val_addr = (unsigned long)hgt_param->addr;

	/* check detection window area */
	if ((hgt_param->width < 1) || (hgt_param->width > 8190))
		return E_VSP_PARA_HGT_WIDTH;

	if ((hgt_param->height < 1) || (hgt_param->height > 8190))
		return E_VSP_PARA_HGT_HEIGHT;

	if ((hgt_param->width + hgt_param->x_offset) > 8190)
		return E_VSP_PARA_HGT_XOFFSET;

	if ((hgt_param->height + hgt_param->y_offset) > 8190)
		return E_VSP_PARA_HGT_YOFFSET;

	/* check hue area */
	ercd = vsp_ins_check_hue_area_param(&hgt_param->area[0]);
	if (ercd)
		return ercd;

	/* check skip mode */
	if ((hgt_param->x_skip != VSP_SKIP_OFF) &&
		(hgt_param->x_skip != VSP_SKIP_1_2) &&
		(hgt_param->x_skip != VSP_SKIP_1_4)) {
		return E_VSP_PARA_HGO_XSKIP;
	}

	if ((hgt_param->y_skip != VSP_SKIP_OFF) &&
		(hgt_param->y_skip != VSP_SKIP_1_2) &&
		(hgt_param->y_skip != VSP_SKIP_1_4)) {
		return E_VSP_PARA_HGO_YSKIP;
	}

	/* check sampling point */
	hgt_info->val_dpr = vsp_ins_get_dpr_smppt(ch_info, hgt_param->sampling);
	if (hgt_info->val_dpr == VSP_DPR_SMPPT_NOT_USE)
		return E_VSP_PARA_HGT_SMMPT;

	return 0;
}

/*
 * vsp_ins_check_module_param - check module parameter routine
 * @prv:			private data
 * @ch:				channel number
 * @ctrl_param:		module parameter
 *
 */
static long vsp_ins_check_module_param(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_CTRL *ctrl_param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	unsigned long processing_module;

	long ercd;

	do {
		processing_module = ch_info->next_module;

		/* check duplicate */
		if (processing_module != VSP_BRU_USE) {
			if (ch_info->reserved_module & processing_module)
				return E_VSP_PARA_CONNECT;
		}

		/* check serial connecting UDS module */
		if (processing_module &
				(VSP_UDS_USE|VSP_UDS1_USE|VSP_UDS2_USE)) {
			if (ch_info->use_uds_flag == 1)
				return E_VSP_PARA_UDS_SERIAL;
			ch_info->use_uds_flag = 1;
		}

		switch (processing_module) {
		case VSP_SRU_USE:
			/* check sru parameter */
			ercd = vsp_ins_check_sru_param(
				ch_info, &prv->sru_info[0], ctrl_param->sru);
			if (ercd)
				return ercd;
			break;
		case VSP_UDS_USE:
			/* check uds parameter */
			ercd = vsp_ins_check_uds_param(
				ch_info, &prv->uds_info[0], ctrl_param->uds);
			if (ercd)
				return ercd;
			break;
		case VSP_UDS1_USE:
			/* check uds parameter */
			ercd = vsp_ins_check_uds_param(
				ch_info, &prv->uds_info[1], ctrl_param->uds1);
			if (ercd)
				return ercd;
			break;
		case VSP_UDS2_USE:
			/* check uds parameter */
			ercd = vsp_ins_check_uds_param(
				ch_info, &prv->uds_info[2], ctrl_param->uds2);
			if (ercd)
				return ercd;
			break;
		case VSP_LUT_USE:
			/* check lut parameter */
			ercd = vsp_ins_check_lut_param(
				ch_info, &prv->lut_info[0], ctrl_param->lut);
			if (ercd)
				return ercd;
			break;
		case VSP_CLU_USE:
			/* check clu parameter */
			ercd = vsp_ins_check_clu_param(
				ch_info, &prv->clu_info[0], ctrl_param->clu);
			if (ercd)
				return ercd;
			break;
		case VSP_HST_USE:
			/* check hst parameter */
			ercd = vsp_ins_check_hst_param(
				ch_info, &prv->hst_info[0], ctrl_param->hst);
			if (ercd)
				return ercd;
			break;
		case VSP_HSI_USE:
			/* check hsi parameter */
			ercd = vsp_ins_check_hsi_param(
				ch_info, &prv->hsi_info[0], ctrl_param->hsi);
			if (ercd)
				return ercd;
			break;
		case VSP_BRU_USE:
		default:	/* WPF */
			processing_module = 0;
			break;
		}

		ch_info->reserved_module |= processing_module;
	} while (processing_module);

	return 0;
}

/*
 * vsp_ins_check_connection_module_from_rpf - check connection parameter routine
 * @prv:			private data
 * @vsp:			IP number
 * @ch:				channel number
 * @param:			start parameter
 *
 */
static long vsp_ins_check_connection_module_from_rpf(
	VSP_PRIVATE_DATA * prv,
	unsigned char vsp,
	unsigned char ch,
	T_VSP_START *param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];

	T_VSP_IN *(in_param[VSP_SRC_MAX]);
	unsigned char rpf_ch;
	unsigned char i;

	unsigned long order = param->rpf_order;
	unsigned long module;

	unsigned char uds_flag = 0;

	long ercd;

	/* set input parameter */
	in_param[0] = param->src1_par;
	in_param[1] = param->src2_par;
	in_param[2] = param->src3_par;
	in_param[3] = param->src4_par;

	/* check RPF number */
	if (param->rpf_num > VSP_SRC_MAX)
		return E_VSP_PARA_RPFNUM;

	/* check module parameter pointer */
	if (param->ctrl_par == NULL)
		return E_VSP_PARA_CTRLPAR;

	for (i = 0; i < param->rpf_num; i++) {
		/* inisialise */
		ch_info->next_module = 0;
		ch_info->use_uds_flag = 0;

		/* get RPF channel */
		rpf_ch = (unsigned char)(order & 0xff);
		module = (0x00000001UL << rpf_ch);

		/* check valid RPF channel */
		if ((vsp_tbl_usable_rpf[vsp] & module) != module)
			return E_VSP_PARA_RPFORDER;

		/* check using other channel */
		if (prv->use_rpf & module) {
			/* already using */
			return E_VSP_BUSY_RPF_OVER;
		}

		ch_info->reserved_rpf |= module;

		/* check RPF parameter */
		ercd = vsp_ins_check_rpf_param(
			ch_info, &prv->rpf_info[rpf_ch], in_param[i]);
		if (ercd)
			return ercd;

		/* check module parameter */
		ercd = vsp_ins_check_module_param(prv, ch, param->ctrl_par);
		if (ercd)
			return ercd;

		/* update source counter */
		ch_info->src_info[ch_info->src_idx].rpf_ch = rpf_ch;
		ch_info->src_idx++;
		ch_info->src_cnt++;

		/* update using UDS flag */
		uds_flag |= ch_info->use_uds_flag;

		order >>= 8;
	}

	/* check connection */
	if ((ch_info->wpf_cnt > 0) && (ch_info->bru_cnt > 0))
		return E_VSP_PARA_CONNECT;

	/* feedback using UDS flag */
	ch_info->use_uds_flag = uds_flag;

	return 0;
}

/*
 * vsp_ins_check_connection_module_from_bru - check connection parameter routine
 * @prv:			private data
 * @ch:				channel number
 * @param:			start parameter
 *
 */
static long vsp_ins_check_connection_module_from_bru(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_START *param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	long ercd;

	/* initialise */
	ch_info->next_module = 0;

	/* check module parameter pointer */
	/* already checked */

	if (param->use_module & VSP_BRU_USE) {
		/* check BRU parameter */
		ercd = vsp_ins_check_bru_param(
			ch_info, &prv->bru_info[0], param->ctrl_par->bru);
		if (ercd)
			return ercd;

		ch_info->reserved_module |= VSP_BRU_USE;
	}

	/* check master layer */
	ercd = vsp_ins_check_master_layer(ch_info);
	if (ercd)
		return ercd;

	/* check module parameter */
	ercd = vsp_ins_check_module_param(prv, ch, param->ctrl_par);
	if (ercd)
		return ercd;

	/* check connection */
	if (ch_info->wpf_cnt != 1)
		return E_VSP_PARA_CONNECT;

	return 0;
}

/*
 * vsp_ins_check_independent_module - check independent parameter routine
 * @prv:			private data
 * @ch:				channel number
 * @param:			start parameter
 *
 */
static long vsp_ins_check_independent_module(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_START *param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];

	long ercd;

	/* check module parameter pointer */
	/* already checked */

	/* check HGO parameter */
	if (param->use_module & VSP_HGO_USE) {
		ercd = vsp_ins_check_hgo_param(
			ch_info, &prv->hgo_info[0], param->ctrl_par->hgo);
		if (ercd)
			return ercd;

		ch_info->reserved_module |= VSP_HGO_USE;
	}

	/* check HGT parameter */
	if (param->use_module & VSP_HGT_USE) {
		ercd = vsp_ins_check_hgt_param(
			ch_info, &prv->hgt_info[0], param->ctrl_par->hgt);
		if (ercd)
			return ercd;

		ch_info->reserved_module |= VSP_HGT_USE;
	}

	return 0;
}

/*
 * vsp_ins_check_output_module - check output module parameter routine
 * @prv:			private data
 * @ch:				channel number
 * @param:			start parameter
 *
 */
static long vsp_ins_check_output_module(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_START *param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	long ercd;

	/* checl WPF parameter */
	ercd = vsp_ins_check_wpf_param(ch_info, param->dst_par);
	if (ercd)
		return ercd;

	return 0;
}

/*
 * vsp_ins_check_start_parameter - check start parameter routine
 * @prv:			VSP driver private data
 * @vsp:			IP number
 * @ch:				channel number
 * @param:			stating parameter
 *
 */
long vsp_ins_check_start_parameter(
	VSP_PRIVATE_DATA * prv,
	unsigned char vsp,
	unsigned char ch,
	T_VSP_START *param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	long ercd;

	/* initialise */
	ch_info->wpf_ch = ch;

	ch_info->wpf_cnt = 0;
	ch_info->bru_cnt = 0;

	ch_info->reserved_rpf = 0;
	ch_info->reserved_module = 0;

	memset(ch_info->src_info, 0, sizeof(ch_info->src_info));
	ch_info->src_idx = 0;
	ch_info->src_cnt = 0;

	/* check connection module parameter (RPF->BRU or WPF) */
	ercd = vsp_ins_check_connection_module_from_rpf(prv, vsp, ch, param);
	if (ercd)
		return ercd;

	/* check connection module parameter (BRU->WPF) */
	ercd = vsp_ins_check_connection_module_from_bru(prv, ch, param);
	if (ercd)
		return ercd;

	/* check independent module parameter (HGO, HGT) */
	ercd = vsp_ins_check_independent_module(prv, ch, param);
	if (ercd)
		return ercd;

	/* check use_module parameter */
	if (param->use_module != ch_info->reserved_module)
		return E_VSP_PARA_USEMODULE;

	/* check valid WPF channel */
	if ((vsp_tbl_usable_module[vsp] & ch_info->reserved_module) !=
		ch_info->reserved_module) {
		return E_VSP_PARA_USEMODULE;
	}

	/* check using other channel */
	if (prv->use_module & ch_info->reserved_module) {
		/* already using */
		return E_VSP_BUSY_MODULE_OVER;
	}

	/* check WPF module parameter */
	ercd = vsp_ins_check_output_module(prv, ch, param);
	if (ercd)
		return ercd;

	DPRINT("use_rpf_bits = %08x\n",
		(unsigned int)ch_info->reserved_rpf);
	DPRINT("use_module_bits = %08x\n",
		(unsigned int)ch_info->reserved_module);
	return 0;
}

