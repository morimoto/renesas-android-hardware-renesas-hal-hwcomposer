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

#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "vspm_main.h"
#include "vspm_log.h"
#include "vsp_drv.h"
#include "vsp_drv_local.h"


/* VPS resource number */
const unsigned int vsp_tbl_resource_num[VSP_IP_MAX] = {
	RESOURCE_VSPS,
#ifdef USE_VSPR
	RESOURCE_VSPR,
#endif /* USE_VSPR */
#ifdef USE_VSPD0
	RESOURCE_VSPD0,
#endif /* USE_VSPD0 */
#ifdef USE_VSPD1
	RESOURCE_VSPD1,
#endif /* USE_VSPD1 */
};


const char *vsp_tbl_resource_name[VSP_IP_MAX] = {
	RESNAME "-vsps",
#ifdef USE_VSPR
	RESNAME "-vspr",
#endif /* USE_VSPR */
#ifdef USE_VSPD0
	RESNAME "-vspd0",
#endif /* USE_VSPD0 */
#ifdef USE_VSPD1
	RESNAME "-vspd1",
#endif /* USE_VSPD1 */
};


/* VPS usable module bits */
const unsigned long vsp_tbl_usable_module[VSP_IP_MAX] = {
	/* VSPS */
	VSP_SRU_USE|
	VSP_UDS_USE|
#ifdef USE_VSPS_UDS
	VSP_UDS1_USE|
	VSP_UDS2_USE|
#endif /* USE_VSPS_UDS */
	VSP_LUT_USE|
	VSP_CLU_USE|
	VSP_HST_USE|
	VSP_HSI_USE|
	VSP_BRU_USE|
	VSP_HGO_USE|
	VSP_HGT_USE,
#ifdef USE_VSPR
	/* VSPR */
	VSP_SRU_USE|
	VSP_UDS_USE|
	VSP_HST_USE|
	VSP_HSI_USE|
	VSP_BRU_USE,
#endif /* USE_VSPR */
#ifdef USE_VSPD0
	/* VSPD0 */
	VSP_UDS_USE|
	VSP_LUT_USE|
	VSP_HST_USE|
	VSP_HSI_USE|
	VSP_BRU_USE|
	VSP_HGO_USE,
#endif /* USE_VSPD0 */
#ifdef USE_VSPD1
	/* VSPD0 */
	VSP_UDS_USE|
	VSP_LUT_USE|
	VSP_HST_USE|
	VSP_HSI_USE|
	VSP_BRU_USE|
	VSP_HGO_USE,
#endif /* USE_VSPD1 */
};


/* VPS usable RPF bits */
const unsigned long vsp_tbl_usable_rpf[VSP_IP_MAX] = {
	/* VSPS */
	VSP_RPF0_USE|
	VSP_RPF1_USE|
	VSP_RPF2_USE|
	VSP_RPF3_USE|
	VSP_RPF4_USE,
#ifdef USE_VSPR
	/* VSPR */
	VSP_RPF0_USE|
	VSP_RPF1_USE|
	VSP_RPF2_USE|
	VSP_RPF3_USE|
	VSP_RPF4_USE,
#endif /* USE_VSPR */
#ifdef USE_VSPD0
	/* VSPD0 */
	VSP_RPF0_USE|
	VSP_RPF1_USE|
	VSP_RPF2_USE|
	VSP_RPF3_USE,
#endif /* USE_VSPD0 */
#ifdef USE_VSPD1
	/* VSPD1 */
	VSP_RPF0_USE|
	VSP_RPF1_USE|
	VSP_RPF2_USE|
	VSP_RPF3_USE,
#endif /* USE_VSPD1 */
};


/* VPS usable WPF bits */
const unsigned long vsp_tbl_usable_wpf[VSP_IP_MAX] = {
	/* VSPS */
	VSP_WPF0_USE|
	VSP_WPF1_USE|
	VSP_WPF2_USE|
	VSP_WPF3_USE,
#ifdef USE_VSPR
	/* VSPR */
	VSP_WPF0_USE|
	VSP_WPF1_USE|
	VSP_WPF2_USE|
	VSP_WPF3_USE,
#endif /* USE_VSPR */
#ifdef USE_VSPD0
	/* VSPD0 */
	VSP_WPF0_USE,
#endif /* USE_VSPD0 */
#ifdef USE_VSPD1
	/* VSPD1 */
	VSP_WPF0_USE,
#endif /* USE_VSPD1 */
};


/* WPF register offset table */
const unsigned short vsp_tbl_wpf_reg_offset[VSP_WPF_MAX][VSP_WPF_REG_MAX] = {
	{	/* WPF0 */
		VSP_WPF0_CMD,			/* CMD */
		VSP_WPF0_IRQ_ENB,		/* IRQ_ENB */
		VSP_WPF0_IRQ_STA,		/* IRQ_STA */
		VSP_WPF0_OFFSET,		/* CTRL */
		VSP_DPR_WPF0_FPORCH		/* FPORCH */
	},
	{	/* WPF1 */
		VSP_WPF1_CMD,			/* CMD */
		VSP_WPF1_IRQ_ENB,		/* IRQ_ENB */
		VSP_WPF1_IRQ_STA,		/* IRQ_STA */
		VSP_WPF1_OFFSET,		/* CTRL */
		VSP_DPR_WPF1_FPORCH		/* FPORCH */
	},
	{	/* WPF2 */
		VSP_WPF2_CMD,			/* CMD */
		VSP_WPF2_IRQ_ENB,		/* IRQ_ENB */
		VSP_WPF2_IRQ_STA,		/* IRQ_STA */
		VSP_WPF2_OFFSET,		/* CTRL */
		VSP_DPR_WPF2_FPORCH		/* FPORCH */
	},
	{	/* WPF3 */
		VSP_WPF3_CMD,			/* CMD */
		VSP_WPF3_IRQ_ENB,		/* IRQ_ENB */
		VSP_WPF3_IRQ_STA,		/* IRQ_STA */
		VSP_WPF3_OFFSET,		/* CTRL */
		VSP_DPR_WPF3_FPORCH		/* FPORCH */
	},
};


/* RPF register offset table */
const unsigned short vsp_tbl_rpf_reg_offset[VSP_RPF_MAX][VSP_REG_MAX] = {
	{	/* RPF0 */
		VSP_RPF0_OFFSET,		/* CTRL */
		VSP_RPF0_CLUT_OFFSET,	/* CLUT TABLE */
		VSP_DPR_RPF0_ROUTE		/* ROUTE */
	},
	{	/* RPF1 */
		VSP_RPF1_OFFSET,		/* CTRL */
		VSP_RPF1_CLUT_OFFSET,	/* CLUT TABLE */
		VSP_DPR_RPF1_ROUTE		/* ROUTE */
	},
	{	/* RPF2 */
		VSP_RPF2_OFFSET,		/* CTRL */
		VSP_RPF2_CLUT_OFFSET,	/* CLUT TABLE */
		VSP_DPR_RPF2_ROUTE		/* ROUTE */
	},
	{	/* RPF3 */
		VSP_RPF3_OFFSET,		/* CTRL */
		VSP_RPF3_CLUT_OFFSET,	/* CLUT TABLE */
		VSP_DPR_RPF3_ROUTE		/* ROUTE */
	},
	{	/* RPF4 */
		VSP_RPF4_OFFSET,		/* CTRL */
		VSP_RPF4_CLUT_OFFSET,	/* CLUT TABLE */
		VSP_DPR_RPF4_ROUTE		/* ROUTE */
	},
};


/* software reset mask bit table */
const unsigned long vsp_tbl_sreset_mskbit[VSP_WPF_MAX] = {
/*	WPF0		WPF1		WPF2		WPF3 */
	0x00000001, 0x00000002, 0x00000004, 0x00000008
};

/* status mask bit table */
const unsigned long vsp_tbl_status_mskbit[VSP_WPF_MAX] = {
/*	WPF0		WPF1		WPF2		WPF3 */
	0x00000100, 0x00000200, 0x00000400, 0x00000800
};

/* SRU parameter table */
const unsigned long vsp_tbl_sru_param[VSP_SCL_LEVEL_MAX][3] = {
/*   CTRL0,      CTRL1       CTRL2 */
	{0x01000400, 0x000007FF, 0x001828FF},	/* level1(weak) */
	{0x01000400, 0x000007FF, 0x000810FF},	/* level2 */
	{0x01800500, 0x000007FF, 0x00243CFF},	/* level3 */
	{0x01800500, 0x000007FF, 0x000C1BFF},	/* level4 */
	{0x02000600, 0x000007FF, 0x003050FF},	/* level5 */
	{0x02000600, 0x000007FF, 0x001024FF},	/* level6(strong) */
};


/*
 * vsp_ins_get_ch_cnt - Get max channel routine
 * @bits:		channel table
 *
 */
static unsigned char vsp_ins_get_ch_cnt(unsigned long bits)
{
	unsigned char cnt = 0;

	if (bits == 0)
		return cnt;

	if (bits & 0xFFFF0000UL) {
		cnt += 16;
		bits >>= 16;
	}

	if (bits & 0x0000FF00UL) {
		cnt += 8;
		bits >>= 8;
	}

	if (bits & 0x000000F0UL) {
		cnt += 4;
		bits >>= 4;
	}

	if (bits & 0x0000000CUL) {
		cnt += 2;
		bits >>= 2;
	}

	if (bits & 0x00000002UL) {
		cnt += 1;
		bits >>= 1;
	}

	return cnt+1;
}


/*
 * vsp_ins_set_lookup_table_register - Set Color lookup table register routine
 * @tbl:		register address
 * @data:		color data
 * @size:		table size
 *
 */
static long vsp_ins_set_lookup_table_register(
	void __iomem *tbl, unsigned long *data, short size)
{
	void __iomem *addr = tbl;
	short i;

	for (i = 0; i < size; i++) {
		iowrite32(*data++, addr);
		addr += 4;
	}

	return 0;
}


/*
 * vsp_ins_set_rpf_register - Set RPF parameter routine
 * @rpf_info:	RPF information
 * @param:		input parameter
 *
 */
static long vsp_ins_set_rpf_register(
	struct vsp_rpf_info *rpf_info, T_VSP_IN *param)
{
	unsigned long reg_temp;

	/* basic read size register */
	reg_temp = ((unsigned long)param->width) << 16;
	reg_temp |= (unsigned long)param->height;
	iowrite32(reg_temp, rpf_info->reg_ctrl + VSP_RPF_SRC_BSIZE);
	DPRINT("VI6_RPF_SRC_BSIZE = %08x\n", (unsigned int)reg_temp);

	/* extended read size register */
	iowrite32(rpf_info->val_esize, rpf_info->reg_ctrl + VSP_RPF_SRC_ESIZE);
	DPRINT("VI6_RPF_SRC_ESIZE = %08x\n", (unsigned int)rpf_info->val_esize);

	/* input format register */
	iowrite32(rpf_info->val_infmt, rpf_info->reg_ctrl + VSP_RPF_INFMT);
	DPRINT("VI6_RPF_INFMT = %08x\n", (unsigned int)rpf_info->val_infmt);

	/* data swapping register */
	iowrite32(rpf_info->val_dswap, rpf_info->reg_ctrl + VSP_RPF_DSWAP);
	DPRINT("VI6_RPF_DSWAP = %08x\n", (unsigned int)rpf_info->val_dswap);

	/* display location register */
	iowrite32(rpf_info->val_loc, rpf_info->reg_ctrl + VSP_RPF_LOC);
	DPRINT("VI6_RPF_LOC = %08x\n", (unsigned int)rpf_info->val_loc);

	/* alpha plane selection control register */
	iowrite32(
		rpf_info->val_alpha_sel, rpf_info->reg_ctrl + VSP_RPF_ALPH_SEL);
	DPRINT("VI6_RPF_ALPH_SEL = %08x\n",
		(unsigned int)rpf_info->val_alpha_sel);

	/* virtual plane color information register */
	iowrite32(
		rpf_info->val_vrtcol, rpf_info->reg_ctrl + VSP_RPF_VRTCOL_SET);
	DPRINT("VI6_RPF_VRTCOL_SET = %08x\n",
		(unsigned int)rpf_info->val_vrtcol);

	/* mask control register */
	iowrite32(rpf_info->val_mskctrl, rpf_info->reg_ctrl + VSP_RPF_MSKCTRL);
	DPRINT("VI6_RPF_MSKCTRL = %08x\n", (unsigned int)rpf_info->val_mskctrl);

	iowrite32(
		rpf_info->val_mskset[0], rpf_info->reg_ctrl + VSP_RPF_MSKSET0);
	DPRINT("VI6_RPF_MSKSET0 = %08x\n",
		(unsigned int)rpf_info->val_mskset[0]);

	iowrite32(
		rpf_info->val_mskset[1], rpf_info->reg_ctrl + VSP_RPF_MSKSET1);
	DPRINT("VI6_RPF_MSKSET1 = %08x\n",
		(unsigned int)rpf_info->val_mskset[1]);

	/* color keying control register */
	iowrite32(
		rpf_info->val_ckey_ctrl,
		rpf_info->reg_ctrl + VSP_RPF_CKEY_CTRL);
	DPRINT("VI6_RPF_CKEY_CTRL = %08x\n",
		(unsigned int)rpf_info->val_ckey_ctrl);

	iowrite32(rpf_info->val_ckey_set[0],
		rpf_info->reg_ctrl + VSP_RPF_CKEY_SET0);
	DPRINT("VI6_RPF_CKEY_SET0 = %08x\n",
		(unsigned int)rpf_info->val_ckey_set[0]);

	iowrite32(rpf_info->val_ckey_set[1],
		rpf_info->reg_ctrl + VSP_RPF_CKEY_SET1);
	DPRINT("VI6_RPF_CKEY_SET1 = %08x\n",
		(unsigned int)rpf_info->val_ckey_set[1]);

	/* source picture memory stride setting register */
	reg_temp = ((unsigned long)param->stride) << 16;
	reg_temp |= (unsigned long)param->stride_c;
	iowrite32(reg_temp, rpf_info->reg_ctrl + VSP_RPF_SRCM_PSTRIDE);
	DPRINT("VI6_RPF_SRCM_PSTRIDE = %08x\n", (unsigned int)reg_temp);

	iowrite32(rpf_info->val_astride,
		rpf_info->reg_ctrl + VSP_RPF_SRCM_ASTRIDE);
	DPRINT("VI6_RPF_SRCM_ASTRIDE = %08x\n",
		(unsigned int)rpf_info->val_astride);

	iowrite32(
		rpf_info->val_addr_y,
		rpf_info->reg_ctrl + VSP_RPF_SRCM_ADDR_Y);
	DPRINT("VI6_RPF_SRCM_ADD_Y = %08x\n",
		(unsigned int)rpf_info->val_addr_y);

	iowrite32(
		rpf_info->val_addr_c0,
		rpf_info->reg_ctrl + VSP_RPF_SRCM_ADDR_C0);
	DPRINT("VI6_RPF_SRCM_ADD_C0 = %08x\n",
		(unsigned int)rpf_info->val_addr_c0);

	iowrite32(
		rpf_info->val_addr_c1,
		rpf_info->reg_ctrl + VSP_RPF_SRCM_ADDR_C1);
	DPRINT("VI6_RPF_SRCM_ADD_C1 = %08x\n",
		(unsigned int)rpf_info->val_addr_c1);

	iowrite32(
		rpf_info->val_addr_ai,
		rpf_info->reg_ctrl + VSP_RPF_SRCM_ADDR_AI);
	DPRINT("VI6_RPF_SRCM_ADD_AI = %08x\n",
		(unsigned int)rpf_info->val_addr_ai);

	/* lookup table register */
	if ((param->format == VSP_IN_RGB_CLUT_DATA) ||
		(param->format == VSP_IN_YUV_CLUT_DATA)) {
		if (param->osd_lut) {
			(void)vsp_ins_set_lookup_table_register(
				rpf_info->reg_clut,
				param->osd_lut->clut,
				param->osd_lut->size);
		}
	}

	/* routing register */
	iowrite32(rpf_info->val_dpr, rpf_info->reg_route);
	DPRINT("VI6_DPR_RPF_ROUTE = %08x\n", (unsigned int)rpf_info->val_dpr);

	return 0;
}

/*
 * vsp_ins_set_wpf_register - Set WPF parameter routine
 * @ch_info:	channel(WPF) information
 * @param:		output parameter
 *
 */
static long vsp_ins_set_wpf_register(
	struct vsp_ch_info *ch_info, T_VSP_OUT *param)
{
	unsigned long reg_temp;

	/* source RPF register */
	iowrite32(ch_info->val_srcrpf, ch_info->reg_ctrl + VSP_WPF_SRCRPFL);
	DPRINT("VI6_WPF_SRCRPF = %08x\n", (unsigned int)ch_info->val_srcrpf);

	/* input size clipping register */
	reg_temp = VSP_WPF_HSZCLIP_HCEN;
	reg_temp |= (((unsigned long)param->x_coffset) << 16);
	reg_temp |= ((unsigned long)param->width);
	iowrite32(reg_temp, ch_info->reg_ctrl + VSP_WPF_HSZCLIP);
	DPRINT("VI6_WPF_HSZCLIP = %08x\n", (unsigned int)reg_temp);

	reg_temp = VSP_WPF_VSZCLIP_VCEN;
	reg_temp |= (((unsigned long)param->y_coffset) << 16);
	reg_temp |= ((unsigned long)param->height);
	iowrite32(reg_temp, ch_info->reg_ctrl + VSP_WPF_VSZCLIP);
	DPRINT("VI6_WPF_VSZCLIP = %08x\n", (unsigned int)reg_temp);

	/* output format register */
	iowrite32(ch_info->val_outfmt, ch_info->reg_ctrl + VSP_WPF_OUTFMT);
	DPRINT("VI6_WPF_OUTFMT = %08x\n", (unsigned int)ch_info->val_outfmt);

	/* data swapping register */
	iowrite32(
		(unsigned long)param->swap, ch_info->reg_ctrl + VSP_WPF_DSWAP);
	DPRINT("VI6_WPF_DSWAP = %08x\n", (unsigned int)param->swap);

	/* rounding control register */
	reg_temp = (((unsigned long)param->cbrm) << 28);
	reg_temp |= (((unsigned long)param->abrm) << 24);
	reg_temp |= (((unsigned long)param->athres) << 16);
	reg_temp |= (((unsigned long)param->clmd) << 12);
	iowrite32(reg_temp, ch_info->reg_ctrl + VSP_WPF_RNDCTRL);
	DPRINT("VI6_WPF_RNDCTRL = %08x\n", (unsigned int)reg_temp);

	/* memory stride register */
	iowrite32((unsigned long)param->stride,
		ch_info->reg_ctrl + VSP_WPF_DSTM_STRIDE_Y);
	DPRINT("VI6_WPF_DSTM_STRIDE_Y = %08x\n",
		(unsigned int)param->stride);

	iowrite32((unsigned long)param->stride_c,
		ch_info->reg_ctrl + VSP_WPF_DSTM_STRIDE_C);
	DPRINT("VI6_WPF_DSTM_STRIDE_C = %08x\n",
		(unsigned int)param->stride_c);

	/* address register */
	iowrite32(
		ch_info->val_addr_y, ch_info->reg_ctrl + VSP_WPF_DSTM_ADDR_Y);
	DPRINT("VI6_WPF_DSTM_ADDR_Y = %08x\n",
		(unsigned int)ch_info->val_addr_y);

	iowrite32(
		ch_info->val_addr_c0, ch_info->reg_ctrl + VSP_WPF_DSTM_ADDR_C0);
	DPRINT("VI6_WPF_DSTM_ADDR_C0 = %08x\n",
		(unsigned int)ch_info->val_addr_c0);

	iowrite32(
		ch_info->val_addr_c1, ch_info->reg_ctrl + VSP_WPF_DSTM_ADDR_C1);
	DPRINT("VI6_WPF_DSTM_ADDR_C1 = %08x\n",
		(unsigned int)ch_info->val_addr_c1);

	return 0;
}

/*
 * vsp_ins_set_sru_register - Set SRU parameter routine
 * @sru_info:	SRU information
 * @param:		SRU parameter
 *
 */
static long vsp_ins_set_sru_register(
	struct vsp_sru_info *sru_info, T_VSP_SRU *param)
{
	unsigned long reg_temp;

	if (sru_info->reg_ctrl != NULL) {
		/* super resolution control register 0 */
		reg_temp = vsp_tbl_sru_param[param->enscl][0];
		reg_temp |= param->mode;
		reg_temp |= param->param;
		reg_temp |= VSP_SRU_CTRL_EN;
		iowrite32(reg_temp, sru_info->reg_ctrl + VSP_SRU_CTRL0);
		DPRINT("VI6_SRU_CTRL0 = %08x\n", (unsigned int)reg_temp);

		/* super resolution control register 1 */
		reg_temp = vsp_tbl_sru_param[param->enscl][1];
		iowrite32(reg_temp, sru_info->reg_ctrl + VSP_SRU_CTRL1);
		DPRINT("VI6_SRU_CTRL1 = %08x\n", (unsigned int)reg_temp);

		/* super resolution control register 2 */
		reg_temp = vsp_tbl_sru_param[param->enscl][2];
		iowrite32(reg_temp, sru_info->reg_ctrl + VSP_SRU_CTRL2);
		DPRINT("VI6_SRU_CTRL2 = %08x\n", (unsigned int)reg_temp);

		/* routing register */
		iowrite32(sru_info->val_dpr, sru_info->reg_route);
		DPRINT("VI6_DPR_SRU_ROUTE = %08x\n",
			(unsigned int)sru_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_uds_register - Set UDS parameter routine
 * @uds_info:	UDS information
 * @param:		UDS parameter
 *
 */
static long vsp_ins_set_uds_register(
	struct vsp_uds_info *uds_info, T_VSP_UDS *param)
{
	unsigned long reg_temp;

	if (uds_info->reg_ctrl != NULL) {
		/* scaling control register */
		iowrite32(
			uds_info->val_ctrl, uds_info->reg_ctrl + VSP_UDS_CTRL);
		DPRINT("VI6_UDSn_CTRL = %08x\n",
			(unsigned int)uds_info->val_ctrl);

		/* scaling factor register */
		reg_temp = ((unsigned long)param->x_ratio) << 16;
		reg_temp |= (unsigned long)param->y_ratio;
		iowrite32(reg_temp, uds_info->reg_ctrl + VSP_UDS_SCALE);
		DPRINT("VI6_UDSn_SCALE = %08x\n", (unsigned int)reg_temp);

		/* alpha data threshold setting register(AON=1) */
		reg_temp = ((unsigned long)param->athres1) << 8;
		reg_temp |= (unsigned long)param->athres0;
		iowrite32(reg_temp, uds_info->reg_ctrl + VSP_UDS_ALPTH);
		DPRINT("VI6_UDSn_ALPTH = %08x\n", (unsigned int)reg_temp);

		/* alpha data replacing value setting register(AON=1) */
		reg_temp = ((unsigned long)param->anum2) << 16;
		reg_temp |= ((unsigned long)param->anum1) << 8;
		reg_temp |= (unsigned long)param->anum0;
		iowrite32(reg_temp, uds_info->reg_ctrl + VSP_UDS_ALPVAL);
		DPRINT("VI6_UDSn_ALPVAL = %08x\n", (unsigned int)reg_temp);

		/* passband register */
		iowrite32(uds_info->val_pass,
			uds_info->reg_ctrl + VSP_UDS_PASS_BWIDTH);
		DPRINT("VI6_UDSn_PASS_BWIDTH = %08x\n",
			(unsigned int)uds_info->val_pass);

		/* 2D IPC setting register(not used) */
		iowrite32(0x00000000, uds_info->reg_ctrl + VSP_UDS_IPC);

		/* output size clipping register */
		iowrite32(
			uds_info->val_clip,
			uds_info->reg_ctrl + VSP_UDS_CLIP_SIZE);
		DPRINT("VI6_UDSn_CLIP_SIZE = %08x\n",
			(unsigned int)uds_info->val_clip);

		/* color fill register */
		iowrite32(
			param->filcolor,
			uds_info->reg_ctrl + VSP_UDS_FILL_COLOR);
		DPRINT("VI6_UDSn_FILL_COLOR = %08x\n",
			(unsigned int)param->filcolor);

		/* routing register */
		iowrite32(uds_info->val_dpr, uds_info->reg_route);
		DPRINT("VI6_DPR_UDS_ROUTE = %08x\n",
			(unsigned int)uds_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_lut_register - Set LUT parameter routine
 * @lut_info:	LUT information
 * @param:		LUT parameter
 *
 */
static long vsp_ins_set_lut_register(
	struct vsp_lut_info *lut_info, T_VSP_LUT *param)
{
	if (lut_info->reg_ctrl != NULL) {
		/* LUT table register */
		(void)vsp_ins_set_lookup_table_register(
			lut_info->reg_table, param->lut, param->size);

		/* control register */
		iowrite32(VSP_LUT_CTRL_EN, lut_info->reg_ctrl);
		DPRINT("VI6_LUT_CTRL = %08x\n", (unsigned int)VSP_LUT_CTRL_EN);

		/* routing register */
		iowrite32(lut_info->val_dpr, lut_info->reg_route);
		DPRINT("VI6_DPR_LUT_ROUTE = %08x\n",
			(unsigned int)lut_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_clu_register - Set CLU parameter routine
 * @clu_info:	CLU information
 * @param:		CLU parameter
 *
 */
static long vsp_ins_set_clu_register(
	struct vsp_clu_info *clu_info, T_VSP_CLU *param)
{
	unsigned long *addr = param->clu_addr;
	unsigned long *data = param->clu_data;
	short i;

	if (clu_info->reg_ctrl != NULL) {
		/* control register */
		iowrite32(clu_info->val_ctrl, clu_info->reg_ctrl);
		DPRINT("VI6_CLU_CTRL = %08x\n",
			(unsigned int)clu_info->val_ctrl);

		/* CLU table register */
		if (clu_info->val_ctrl & VSP_CLU_CTRL_AAI) {
			/* automatic address increment mode */
			iowrite32(0, clu_info->reg_addr);

			for (i = 0; i < param->size; i++)
				iowrite32(*data++, clu_info->reg_data);
		} else {
			/* manual address increment mode */
			for (i = 0; i < param->size; i++) {
				iowrite32(*addr++, clu_info->reg_addr);
				iowrite32(*data++, clu_info->reg_data);
			}
		}

		/* routing register */
		iowrite32(clu_info->val_dpr, clu_info->reg_route);
		DPRINT("VI6_DPR_CLU_ROUTE = %08x\n",
			(unsigned int)clu_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_hst_register - Set HST parameter routine
 * @hst_info:	HST information
 *
 */
static long vsp_ins_set_hst_register(struct vsp_hst_info *hst_info)
{
	if (hst_info->reg_ctrl != NULL) {
		/* control register */
		iowrite32(VSP_HST_CTRL_EN, hst_info->reg_ctrl);
		DPRINT("VI6_HST_CTRL = %08x\n", (unsigned int)VSP_HST_CTRL_EN);

		/* routing register */
		iowrite32(hst_info->val_dpr, hst_info->reg_route);
		DPRINT("VI6_DPR_HST_ROUTE = %08x\n",
			(unsigned int)hst_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_hsi_register - Set HSI parameter routine
 * @hsi_info:	HSI information
 *
 */
static long vsp_ins_set_hsi_register(struct vsp_hsi_info *hsi_info)
{
	if (hsi_info->reg_ctrl != NULL) {
		/* control register */
		iowrite32(VSP_HSI_CTRL_EN, hsi_info->reg_ctrl);
		DPRINT("VI6_HSI_CTRL = %08x\n", (unsigned int)VSP_HSI_CTRL_EN);

		/* routing register */
		iowrite32(hsi_info->val_dpr, hsi_info->reg_route);
		DPRINT("VI6_DPR_HSI_ROUTE = %08x\n",
			(unsigned int)hsi_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_bru_register - Set BRU parameter routine
 * @bru_info:	BRU information
 * @param:		BRU parameter
 *
 */
static long vsp_ins_set_bru_register(
	struct vsp_bru_info *bru_info, T_VSP_BRU *param)
{
	if (bru_info->reg_ctrl != NULL) {
		/* input control register */
		iowrite32(
			bru_info->val_inctrl,
			bru_info->reg_ctrl + VSP_BRU_INCTRL);
		DPRINT("VI6_BRU_INCTRL = %08x\n",
			(unsigned int)bru_info->val_inctrl);

		/* input virtual address */
		iowrite32(
			bru_info->val_vir_size,
			bru_info->reg_ctrl + VSP_BRU_VIRRPF_SIZE);
		DPRINT("VI6_BRU_VIRRPF_SIZE = %08x\n",
			(unsigned int)bru_info->val_vir_size);

		/* display location register */
		iowrite32(
			bru_info->val_vir_loc,
			bru_info->reg_ctrl + VSP_BRU_VIRRPF_LOC);
		DPRINT("VI6_BRU_VIRRPF_LOC = %08x\n",
			(unsigned int)bru_info->val_vir_loc);

		/* color information register */
		iowrite32(
			bru_info->val_vir_color,
			bru_info->reg_ctrl + VSP_BRU_VIRRPF_COL);
		DPRINT("VI6_BRU_VIRRPF_COL = %08x\n",
			(unsigned int)bru_info->val_vir_color);

		/* Blend/ROP UNIT A control register */
		iowrite32(
			bru_info->val_ctrl[0],
			bru_info->reg_ctrl + VSP_BRUA_CTRL);
		DPRINT("VI6_BRUA_CTRL = %08x\n",
			(unsigned int)bru_info->val_ctrl[0]);
		iowrite32(
			bru_info->val_bld[0],
			bru_info->reg_ctrl + VSP_BRUA_BLD);
		DPRINT("VI6_BRUA_BLD = %08x\n",
			(unsigned int)bru_info->val_bld[0]);

		/* Blend/ROP UNIT B control register */
		iowrite32(
			bru_info->val_ctrl[1],
			bru_info->reg_ctrl + VSP_BRUB_CTRL);
		DPRINT("VI6_BRUB_CTRL = %08x\n",
			(unsigned int)bru_info->val_ctrl[1]);
		iowrite32(
			bru_info->val_bld[1],
			 bru_info->reg_ctrl + VSP_BRUB_BLD);
		DPRINT("VI6_BRUB_BLD = %08x\n",
			(unsigned int)bru_info->val_bld[1]);

		/* Blend/ROP UNIT C control register */
		iowrite32(
			bru_info->val_ctrl[2],
			bru_info->reg_ctrl + VSP_BRUC_CTRL);
		DPRINT("VI6_BRUC_CTRL = %08x\n",
			(unsigned int)bru_info->val_ctrl[2]);
		iowrite32(
			bru_info->val_bld[2],
			bru_info->reg_ctrl + VSP_BRUC_BLD);
		DPRINT("VI6_BRUC_BLD = %08x\n",
			(unsigned int)bru_info->val_bld[2]);

		/* Blend/ROP UNIT D control register */
		iowrite32(
			bru_info->val_ctrl[3],
			bru_info->reg_ctrl + VSP_BRUD_CTRL);
		DPRINT("VI6_BRUD_CTRL = %08x\n",
			(unsigned int)bru_info->val_ctrl[3]);
		iowrite32(
			bru_info->val_bld[3],
			bru_info->reg_ctrl + VSP_BRUD_BLD);
		DPRINT("VI6_BRUD_BLD = %08x\n",
			(unsigned int)bru_info->val_bld[3]);

		/* ROP UNIT control register */
		iowrite32(bru_info->val_rop, bru_info->reg_ctrl + VSP_BRU_ROP);
		DPRINT("VI6_BRU_ROP = %08x\n", (unsigned int)bru_info->val_rop);

		/* routing register */
		iowrite32(bru_info->val_dpr, bru_info->reg_route);
		DPRINT("VI6_DPR_BRU_ROUTE = %08x\n",
			(unsigned int)bru_info->val_dpr);
	}

	return 0;
}

/*
 * vsp_ins_set_hgo_register - Set HGO parameter routine
 * @hgo_info:	HGO information
 * @param:		HGO parameter
 *
 */
static long vsp_ins_set_hgo_register(
	struct vsp_hgo_info *hgo_info, T_VSP_HGO *param)
{
	unsigned long reg_temp;

	if (hgo_info->reg_ctrl != NULL) {
		/* detection window offset register */
		reg_temp = ((unsigned long)param->x_offset) << 16;
		reg_temp |= (unsigned long)param->y_offset;
		iowrite32(reg_temp, hgo_info->reg_ctrl);
		DPRINT("VI6_HGO_OFFSET = %08x\n",
			(unsigned int)reg_temp);

		/* detection window size register */
		reg_temp = ((unsigned long)param->width) << 16;
		reg_temp |= (unsigned long)param->height;
		iowrite32(reg_temp, hgo_info->reg_ctrl + VSP_HGO_SIZE);
		DPRINT("VI6_HGO_SIZE = %08x\n",
			(unsigned int)reg_temp);

		/* mode register */
		reg_temp = (unsigned long)param->binary_mode;
		reg_temp |= (unsigned long)param->maxrgb_mode;
		reg_temp |= ((unsigned long)param->x_skip) << 2;
		reg_temp |= (unsigned long)param->y_skip;
		iowrite32(reg_temp, hgo_info->reg_ctrl + VSP_HGO_MODE);
		DPRINT("VI6_HGO_MODE = %08x\n",
			(unsigned int)reg_temp);

		/* smppt register */
		iowrite32(hgo_info->val_dpr, hgo_info->reg_smppt);
		DPRINT("VI6_DPR_HGO_SMPPT = %08x\n",
			(unsigned int)hgo_info->val_dpr);

		/* reset */
		iowrite32(
			VSP_HGO_REGRST_SET,
			hgo_info->reg_ctrl + VSP_HGO_REGRST);
		DPRINT("VI6_HGO_REGRST = %08x\n",
			(unsigned int)VSP_HGO_REGRST_SET);
	}

	return 0;
}

/*
 * vsp_ins_set_hgt_register - Set HGT parameter routine
 * @hgt_info:	HGT information
 * @param:		HGT parameter
 *
 */
static long vsp_ins_set_hgt_register(
	struct vsp_hgt_info *hgt_info, T_VSP_HGT *param)
{
	unsigned long reg_temp;

	if (hgt_info->reg_ctrl != NULL) {
		/* detection window offset register */
		reg_temp = ((unsigned long)param->x_offset) << 16;
		reg_temp |= (unsigned long)param->y_offset;
		iowrite32(reg_temp, hgt_info->reg_ctrl);
		DPRINT("VI6_HGT_OFFSET = %08x\n",
			(unsigned int)reg_temp);

		/* detection window size register */
		reg_temp = ((unsigned long)param->width) << 16;
		reg_temp |= (unsigned long)param->height;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_SIZE);
		DPRINT("VI6_HGT_SIZE = %08x\n",
			(unsigned int)reg_temp);

		/* mode register */
		reg_temp = ((unsigned long)param->x_skip) << 2;
		reg_temp |= (unsigned long)param->y_skip;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_MODE);
		DPRINT("VI6_HGT_MODE = %08x\n",
			(unsigned int)reg_temp);

		/* heu area register */
		reg_temp = ((unsigned long)param->area[0].lower) << 16;
		reg_temp |= (unsigned long)param->area[0].upper;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_HUE_AREA0);
		DPRINT("VI6_HGT_AREA0 = %08x\n",
			(unsigned int)reg_temp);

		reg_temp = ((unsigned long)param->area[1].lower) << 16;
		reg_temp |= (unsigned long)param->area[1].upper;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_HUE_AREA1);
		DPRINT("VI6_HGT_AREA1 = %08x\n",
			(unsigned int)reg_temp);

		reg_temp = ((unsigned long)param->area[2].lower) << 16;
		reg_temp |= (unsigned long)param->area[2].upper;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_HUE_AREA2);
		DPRINT("VI6_HGT_AREA2 = %08x\n",
			(unsigned int)reg_temp);

		reg_temp = ((unsigned long)param->area[3].lower) << 16;
		reg_temp |= (unsigned long)param->area[3].upper;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_HUE_AREA3);
		DPRINT("VI6_HGT_AREA3 = %08x\n",
			(unsigned int)reg_temp);

		reg_temp = ((unsigned long)param->area[4].lower) << 16;
		reg_temp |= (unsigned long)param->area[4].upper;
		iowrite32(reg_temp, hgt_info->reg_ctrl + VSP_HGT_HUE_AREA4);
		DPRINT("VI6_HGT_AREA4 = %08x\n",
			(unsigned int)reg_temp);

		reg_temp = ((unsigned long)param->area[5].lower) << 16;
		reg_temp |= (unsigned long)param->area[5].upper;
		iowrite32(
			reg_temp,
			hgt_info->reg_ctrl + VSP_HGT_HUE_AREA5);
		DPRINT("VI6_HGT_AREA5 = %08x\n",
			(unsigned int)reg_temp);

		/* smppt register */
		iowrite32(hgt_info->val_dpr, hgt_info->reg_smppt);
		DPRINT("VI6_DPR_HGT_SMPPT = %08x\n",
			(unsigned int)hgt_info->val_dpr);

		/* reset */
		iowrite32(
			VSP_HGT_REGRST_SET,
			hgt_info->reg_ctrl + VSP_HGT_REGRST);
		DPRINT("VI6_HGT_REGRST = %08x\n",
			(unsigned int)VSP_HGT_REGRST_SET);
	}

	return 0;
}

/*
 * vsp_ins_get_hgo_register - Get HGO parameter routine
 * @hgo_info:	HGO information
 *
 */
static long vsp_ins_get_hgo_register(struct vsp_hgo_info *hgo_info)
{
	unsigned long *r_addr = (unsigned long *)(hgo_info->val_addr);
	unsigned long *g_addr = (unsigned long *)(hgo_info->val_addr + 0x100);
	unsigned long *b_addr = (unsigned long *)(hgo_info->val_addr + 0x200);
	int i;

	for (i = 0; i < 64; i++) {
		*r_addr++ = ioread32(hgo_info->reg_hist[0] + i*4);
		*g_addr++ = ioread32(hgo_info->reg_hist[1] + i*4);
		*b_addr++ = ioread32(hgo_info->reg_hist[2] + i*4);
	}

	return 0;
}

/*
 * vsp_ins_get_hgt_register - Get HGT parameter routine
 * @hgt_info:	HGT information
 *
 */
static long vsp_ins_get_hgt_register(struct vsp_hgt_info *hgt_info)
{
	unsigned long *dst = (unsigned long *)(hgt_info->val_addr);
	void __iomem *src = hgt_info->reg_hist;
	int i;

	for (i = 0; i < 192; i++) {
		*dst++ = ioread32(src);
		src += 4;
	}

	return 0;
}

/*
 * vsp_ins_set_module_register - Set module register routine
 * @prv:		private data
 * @ch:			channel number
 * @ctrl_param:	module parameter
 *
 */
static long vsp_ins_set_module_register(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_CTRL *ctrl_param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	unsigned long module = ch_info->reserved_module;
	long ercd;

	/* set super-resolution parameter */
	if (module & VSP_SRU_USE) {
		ercd = vsp_ins_set_sru_register(
			&prv->sru_info[0], ctrl_param->sru);
		if (ercd)
			return ercd;
	}

	/* set up down scaler parameter */
	if (module & VSP_UDS_USE) {
		ercd = vsp_ins_set_uds_register(
			&prv->uds_info[0], ctrl_param->uds);
		if (ercd)
			return ercd;
	}

	if (module & VSP_UDS1_USE) {
		ercd = vsp_ins_set_uds_register(
			&prv->uds_info[1], ctrl_param->uds1);
		if (ercd)
			return ercd;
	}

	if (module & VSP_UDS2_USE) {
		ercd = vsp_ins_set_uds_register(
			&prv->uds_info[2], ctrl_param->uds2);
		if (ercd)
			return ercd;
	}

	/* set look up table parameter */
	if (module & VSP_LUT_USE) {
		ercd = vsp_ins_set_lut_register(
			&prv->lut_info[0], ctrl_param->lut);
		if (ercd)
			return ercd;
	}

	/* set CLU parameter */
	if (module & VSP_CLU_USE) {
		ercd = vsp_ins_set_clu_register(
			&prv->clu_info[0], ctrl_param->clu);
		if (ercd)
			return ercd;
	}

	/* set hue saturation value transform parameter */
	if (module & VSP_HST_USE) {
		ercd = vsp_ins_set_hst_register(&prv->hst_info[0]);
		if (ercd)
			return ercd;
	}

	/* set hue saturation value inverse transform parameter */
	if (module & VSP_HSI_USE) {
		ercd = vsp_ins_set_hsi_register(&prv->hsi_info[0]);
		if (ercd)
			return ercd;
	}

	/* set blend ROP parameter */
	if (module & VSP_BRU_USE) {
		ercd = vsp_ins_set_bru_register(
			&prv->bru_info[0], ctrl_param->bru);
		if (ercd)
			return ercd;
	}

	/* set histogram generator-one dimension parameter */
	if (module & VSP_HGO_USE) {
		ercd = vsp_ins_set_hgo_register(
			&prv->hgo_info[0], ctrl_param->hgo);
		if (ercd)
			return ercd;
	}

	/* set histogram generator-two dimension parameter */
	if (module & VSP_HGT_USE) {
		ercd = vsp_ins_set_hgt_register(
			&prv->hgt_info[0], ctrl_param->hgt);
		if (ercd)
			return ercd;
	}

	return 0;
}

/*
 * vsp_ins_get_module_register - Get module register routine
 * @prv:		private data
 * @module:		module ID
 *
 */
static long vsp_ins_get_module_register(
	VSP_PRIVATE_DATA * prv, unsigned long module)
{
	long ercd;

	/* set histogram generator-one dimension parameter */
	if (module & VSP_HGO_USE) {
		ercd = vsp_ins_get_hgo_register(&prv->hgo_info[0]);
		if (ercd)
			return ercd;
	}

	/* set histogram generator-two dimension parameter */
	if (module & VSP_HGT_USE) {
		ercd = vsp_ins_get_hgt_register(&prv->hgt_info[0]);
		if (ercd)
			return ercd;
	}

	return 0;
}

/*
 * vsp_ins_clear_rpf_register - Clear RPF register routine
 * @prv:		private data
 * @rpf_bits:	RPF channel bits
 *
 */
static long vsp_ins_clear_rpf_register(
	VSP_PRIVATE_DATA * prv, unsigned long rpf_bits)
{
	unsigned long bit = 0x00000001UL;
	unsigned char ch;

	for (ch = 0; ch < VSP_RPF_MAX; ch++) {
		if (rpf_bits & bit) {
			/* clear RPF routing register */
			iowrite32(
				VSP_DPR_ROUTE_NOT_USE,
				prv->rpf_info[ch].reg_route);
		}
		bit <<= 1;
	}

	return 0;
}

/*
 * vsp_ins_clear_wpf_register - Clear WPF register routine
 * @prv:		private data
 * @wpf_bits:	WPF channel bits
 *
 */
static long vsp_ins_clear_wpf_register(
	VSP_PRIVATE_DATA * prv, unsigned long wpf_bits)
{
	unsigned long bit = 0x00000001UL;
	unsigned char ch;

	for (ch = 0; ch < VSP_WPF_MAX; ch++) {
		if (wpf_bits & bit) {
			/* clear WPF source-RPF register */
			iowrite32(
				0x00000000,
				prv->ch_info[ch].reg_ctrl + VSP_WPF_SRCRPFL);

			/* clear WPF timing control register */
			iowrite32(0x00000500, prv->ch_info[ch].reg_fporch);
		}
		bit <<= 1;
	}

	return 0;
}

/*
 * vsp_ins_clear_module_register - Clear module register routine
 * @prv:		private data
 * @module:		target module bits
 *
 */
static long vsp_ins_clear_module_register(
	VSP_PRIVATE_DATA * prv, unsigned long module)
{
	/* clear super-resolution parameter */
	if (module & VSP_SRU_USE) {
		/* clear SRU routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->sru_info[0].reg_route);
	}

	/* clear up down scaler parameter */
	if (module & VSP_UDS_USE) {
		/* clear UDS0 routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->uds_info[0].reg_route);
	}

	if (module & VSP_UDS1_USE) {
		/* clear UDS1 routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->uds_info[1].reg_route);
	}

	if (module & VSP_UDS2_USE) {
		/* clear UDS2 routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->uds_info[2].reg_route);
	}

	/* clear look up table parameter */
	if (module & VSP_LUT_USE) {
		/* clear LUT routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->lut_info[0].reg_route);
	}

	/* clear CLU parameter */
	if (module & VSP_CLU_USE) {
		/* clear CLU routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->clu_info[0].reg_route);
	}

	/* clear hue saturation value transform parameter */
	if (module & VSP_HST_USE) {
		/* clear HST routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->hst_info[0].reg_route);
	}

	/* clear hue saturation value inverse transform parameter */
	if (module & VSP_HSI_USE) {
		/* clear HSI routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->hsi_info[0].reg_route);
	}

	/* clear blend ROP parameter */
	if (module & VSP_BRU_USE) {
		/* clear BRU routing register */
		iowrite32(VSP_DPR_ROUTE_NOT_USE, prv->bru_info[0].reg_route);
	}

	/* set histogram generator-one dimension parameter */
	if (module & VSP_HGO_USE) {
		/* clear HGO sampling point register */
		iowrite32(VSP_DPR_SMPPT_NOT_USE, prv->hgo_info[0].reg_smppt);
	}

	/* set histogram generator-two dimension parameter */
	if (module & VSP_HGT_USE) {
		/* clear HGT sampling point register */
		iowrite32(VSP_DPR_SMPPT_NOT_USE, prv->hgt_info[0].reg_smppt);
	}

	return 0;
}

/*
 * vsp_ins_set_start_parameter - Set start parameter routine
 * @prv:		private data
 * @ch:			channel number
 * @param:		starting parameter
 *
 */
long vsp_ins_set_start_parameter(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_START *param)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	T_VSP_IN *(in_param[VSP_SRC_MAX]);

	unsigned long rpf_ch;
	unsigned char rpf_lp;

	unsigned long rpf_order = param->rpf_order;

	long ercd;

	/* set input parameter */
	in_param[0] = param->src1_par;
	in_param[1] = param->src2_par;
	in_param[2] = param->src3_par;
	in_param[3] = param->src4_par;

	/* input module */
	for (rpf_lp = 0; rpf_lp < param->rpf_num; rpf_lp++) {
		/* get RPF channel */
		rpf_ch = (unsigned char)(rpf_order & 0xff);

		ercd = vsp_ins_set_rpf_register(
			&prv->rpf_info[rpf_ch], in_param[rpf_lp]);
		if (ercd)
			return ercd;

		rpf_order >>= 8;
	}

	/* processing module */
	ercd = vsp_ins_set_module_register(prv, ch, param->ctrl_par);
	if (ercd)
		return ercd;

	/* output module */
	ercd = vsp_ins_set_wpf_register(ch_info, param->dst_par);
	if (ercd)
		return ercd;

	return 0;
}

/*
 * vsp_ins_start_processing - Start processing routine
 * @ch_info:		channel information
 *
 */
void vsp_ins_start_processing(struct vsp_ch_info *ch_info)
{
	/* clear interrupt status */
	iowrite32(~VSP_IRQ_FRMEND, ch_info->reg_irq_sta);
	DPRINT("VI6_WPF%d_IRQ_STA = %08x\n",
		ch_info->wpf_ch, (unsigned int)~VSP_IRQ_FRMEND);

	/* enable interrupt */
	iowrite32(VSP_IRQ_FRMEND, ch_info->reg_irq_enb);
	DPRINT("VI6_WPF%d_IRQ_ENB = %08x\n",
		ch_info->wpf_ch, (unsigned int)VSP_IRQ_FRMEND);

	/* start */
	iowrite32(VSP_CMD_STRCMD, ch_info->reg_cmd);
	DPRINT("VI6_CMD%d = %08x\n",
		ch_info->wpf_ch, (unsigned int)VSP_CMD_STRCMD);
}

/*
 * vsp_ins_stop_processing - Stop processing routine
 * @prv:		private data
 * @ch:			channel number
 *
 */
long vsp_ins_stop_processing(VSP_PRIVATE_DATA *prv, unsigned char ch)
{
	struct vsp_ch_info *ch_info = &prv->ch_info[ch];
	volatile unsigned long dmy;

	unsigned long status;
	unsigned long loop_cnt = VSP_STATUS_LOOP_CNT;

	/* disable interrupt */
	iowrite32(0x00000000, ch_info->reg_irq_enb);

	/* clear interrupt */
	iowrite32(0x00000000, ch_info->reg_irq_sta);

	/* dummy read */
	dmy = ioread32(ch_info->reg_irq_sta);
	dmy = ioread32(ch_info->reg_irq_sta);

	/* read status register */
	status = ioread32(prv->base_reg + VSP_STATUS);

	if (status & vsp_tbl_status_mskbit[ch]) {
		/* software reset */
		iowrite32(
			vsp_tbl_sreset_mskbit[ch], prv->base_reg + VSP_SRESET);
		DPRINT("VI6_SRESET = %08x\n",
			(unsigned int)vsp_tbl_sreset_mskbit[ch]);

		/* waiting reset process */
		do {
			/* sleep */
			msleep(1);		/* 1ms */

			/* read status register */
			status = ioread32(prv->base_reg + VSP_STATUS);
		} while ((status & vsp_tbl_status_mskbit[ch]) &&
			(--loop_cnt > 0));
	}

	/* disable callback function */
	ch_info->cb_func = NULL;

	/* callback function */
	if (loop_cnt != 0)
		vsp_ins_cb_function(prv, ch, -ECANCELED);
	else
		vsp_ins_cb_function(prv, ch, -ETIMEDOUT);

	return 0;
}


/*
 * vsp_ins_software_reset - Software reset routine
 * @prv:		private data
 * @wpf_bits:	WPF channel bits
 *
 */
static void vsp_ins_software_reset(
	VSP_PRIVATE_DATA *prv, unsigned long wpf_bits)
{
	unsigned long bit = 0x00000001UL;
	unsigned char ch;

	for (ch = 0; ch < VSP_WPF_MAX; ch++) {
		if (wpf_bits & bit)
			(void)vsp_ins_stop_processing(prv, ch);

		bit <<= 1;
	}
}

/*
 * vsp_ins_allocate_memory - Allocate memory routine
 * @prv:		private data
 *
 */
long vsp_ins_allocate_memory(VSP_PRIVATE_DATA **prv)
{
	/* allocate memory */
	*prv = kzalloc(sizeof(VSP_PRIVATE_DATA), GFP_KERNEL);
	if (!*prv) {
		APRINT("[%s] allocate memory failed!!\n", __func__);
		return E_VSP_NO_MEM;
	}

	DPRINT("allocate(%08x)\n", (unsigned int)(*prv));
	return 0;
}

/*
 * vsp_ins_free_memory - Release memory routine
 * @prv:		private data
 *
 */
long vsp_ins_free_memory(VSP_PRIVATE_DATA *prv)
{
	/* clear memory */
	memset(prv, 0, sizeof(VSP_PRIVATE_DATA));

	/* release memory */
	kfree(prv);

	DPRINT("release(%08x)\n", (unsigned int)prv);
	return 0;
}


/*
 * vsp_ins_set_rpf_reg_table - Make RPF register table routine
 * @prv:		private data
 * @rpf_bits:	RPF channel bits
 *
 */
static void vsp_ins_set_rpf_reg_table(
	VSP_PRIVATE_DATA *prv, unsigned long rpf_bits)
{
	struct vsp_rpf_info *rpf_info;

	unsigned long bit = 0x00000001UL;
	unsigned char ch;

	/* set RPF register */
	rpf_info = &prv->rpf_info[0];
	for (ch = 0; ch < VSP_RPF_MAX; ch++) {
		if (rpf_bits & bit) {
			rpf_info->reg_ctrl = prv->base_reg +
				vsp_tbl_rpf_reg_offset[ch][VSP_REG_CTRL];
			rpf_info->reg_clut = prv->base_reg +
				vsp_tbl_rpf_reg_offset[ch][VSP_REG_CLUT];
			rpf_info->reg_route = prv->base_reg +
				vsp_tbl_rpf_reg_offset[ch][VSP_REG_ROUTE];
		}
		bit <<= 1;
		rpf_info++;
	}
}


/*
 * vsp_ins_set_wpf_reg_table - Make WPF register table routine
 * @prv:		private data
 * @wpf_bits:	WPF channel bits
 *
 */
static void vsp_ins_set_wpf_reg_table(
	VSP_PRIVATE_DATA *prv, unsigned long wpf_bits)
{
	struct vsp_ch_info *ch_info;

	unsigned long bit = 0x00000001UL;
	unsigned char ch;

	/* set WPF register */
	ch_info = &prv->ch_info[0];
	for (ch = 0; ch < VSP_WPF_MAX; ch++) {
		if (wpf_bits & bit) {
			ch_info->reg_cmd = prv->base_reg +
				vsp_tbl_wpf_reg_offset[ch][VSP_WPF_REG_CMD];
			ch_info->reg_irq_enb = prv->base_reg +
				vsp_tbl_wpf_reg_offset[ch][VSP_WPF_REG_IRQ_ENB];
			ch_info->reg_irq_sta = prv->base_reg +
				vsp_tbl_wpf_reg_offset[ch][VSP_WPF_REG_IRQ_STA];

			ch_info->reg_ctrl = prv->base_reg +
				vsp_tbl_wpf_reg_offset[ch][VSP_WPF_REG_CTRL];
			ch_info->reg_fporch = prv->base_reg +
				vsp_tbl_wpf_reg_offset[ch][VSP_WPF_REG_FPORCH];
		}
		bit <<= 1;
		ch_info++;
	}
}


/*
 * vsp_ins_set_module_reg_table - Make module register table routine
 * @prv:		private data
 * @module:		VSP usable module bits
 *
 */
static void vsp_ins_set_module_reg_table(
	VSP_PRIVATE_DATA *prv, unsigned long module)
{
	/* set SRU register */
	if (module & VSP_SRU_USE) {
		prv->sru_info[0].reg_ctrl  = prv->base_reg + VSP_SRU_OFFSET;
		prv->sru_info[0].reg_route = prv->base_reg + VSP_DPR_SRU_ROUTE;
	}

	/* set UDS register */
	if (module & VSP_UDS_USE) {
		prv->uds_info[0].reg_ctrl  = prv->base_reg + VSP_UDS0_OFFSET;
		prv->uds_info[0].reg_route = prv->base_reg + VSP_DPR_UDS0_ROUTE;
	}

	if (module & VSP_UDS1_USE) {
		prv->uds_info[1].reg_ctrl  = prv->base_reg + VSP_UDS1_OFFSET;
		prv->uds_info[1].reg_route = prv->base_reg + VSP_DPR_UDS1_ROUTE;
	}

	if (module & VSP_UDS2_USE) {
		prv->uds_info[2].reg_ctrl  = prv->base_reg + VSP_UDS2_OFFSET;
		prv->uds_info[2].reg_route = prv->base_reg + VSP_DPR_UDS2_ROUTE;
	}

	/* set LUT register */
	if (module & VSP_LUT_USE) {
		prv->lut_info[0].reg_ctrl  = prv->base_reg + VSP_LUT_OFFSET;
		prv->lut_info[0].reg_route = prv->base_reg + VSP_DPR_LUT_ROUTE;
		prv->lut_info[0].reg_table = prv->base_reg + VSP_LUT_TBL_OFFSET;
	}

	/* set CLU register */
	if (module & VSP_CLU_USE) {
		prv->clu_info[0].reg_ctrl  = prv->base_reg + VSP_CLU_OFFSET;
		prv->clu_info[0].reg_route = prv->base_reg + VSP_DPR_CLU_ROUTE;
		prv->clu_info[0].reg_addr  = prv->base_reg + VSP_CLU_TBL_ADDR;
		prv->clu_info[0].reg_data  = prv->base_reg + VSP_CLU_TBL_DATA;
	}

	/* set HST register */
	if (module & VSP_HST_USE) {
		prv->hst_info[0].reg_ctrl  = prv->base_reg + VSP_HST_OFFSET;
		prv->hst_info[0].reg_route = prv->base_reg + VSP_DPR_HST_ROUTE;
	}

	/* set HSI register */
	if (module & VSP_HSI_USE) {
		prv->hsi_info[0].reg_ctrl  = prv->base_reg + VSP_HSI_OFFSET;
		prv->hsi_info[0].reg_route = prv->base_reg + VSP_DPR_HSI_ROUTE;
	}

	/* set BRU register */
	if (module & VSP_BRU_USE) {
		prv->bru_info[0].reg_ctrl  = prv->base_reg + VSP_BRU_OFFSET;
		prv->bru_info[0].reg_route = prv->base_reg + VSP_DPR_BRU_ROUTE;
	}

	/* set HGO register */
	if (module & VSP_HGO_USE) {
		prv->hgo_info[0].reg_ctrl  = prv->base_reg + VSP_HGO_OFFSET;
		prv->hgo_info[0].reg_smppt = prv->base_reg + VSP_DPR_HGO_SMPPT;
		prv->hgo_info[0].reg_hist[0] =
			prv->base_reg + VSP_HGO_R_HISTO_OFFSET;
		prv->hgo_info[0].reg_hist[1] =
			prv->base_reg + VSP_HGO_G_HISTO_OFFSET;
		prv->hgo_info[0].reg_hist[2] =
			prv->base_reg + VSP_HGO_B_HISTO_OFFSET;
	}

	/* set HGT register */
	if (module & VSP_HGT_USE) {
		prv->hgt_info[0].reg_ctrl  = prv->base_reg + VSP_HGT_OFFSET;
		prv->hgt_info[0].reg_smppt = prv->base_reg + VSP_DPR_HGT_SMPPT;
		prv->hgt_info[0].reg_hist =
			prv->base_reg + VSP_HGT_HIST_OFFSET;
	}
}


/*
 * vsp_ins_init_reg - Initialize register routine
 * @pdev:		platform device
 * @prv:		private data
 * @vsp_ch:		VSP IP channel
 *
 */
long vsp_ins_init_reg(
	struct platform_device *pdev,
	VSP_PRIVATE_DATA *prv,
	unsigned char vsp_ch)
{
	struct resource *res;

	DPRINT("called\n");

	/* get an I/O memory resource for device */
	res = platform_get_resource(
		pdev, IORESOURCE_MEM, vsp_tbl_resource_num[vsp_ch]);
	if (!res) {
		APRINT("[%s] platform_get_resource() failed!!\n", __func__);
		return E_VSP_DEF_REG;
	}

	/* remap I/O memory */
	prv->base_reg = ioremap_nocache(res->start, resource_size(res));
	if (!prv->base_reg) {
		APRINT("[%s] ioremap_nocache() failed!!\n", __func__);
		return E_VSP_DEF_REG;
	}

	/* make RPF register table */
	vsp_ins_set_rpf_reg_table(prv, vsp_tbl_usable_rpf[vsp_ch]);

	/* make WPF register table */
	vsp_ins_set_wpf_reg_table(prv, vsp_tbl_usable_wpf[vsp_ch]);

	/* make module register table */
	vsp_ins_set_module_reg_table(prv, vsp_tbl_usable_module[vsp_ch]);

	/* software reset */
	(void)vsp_ins_software_reset(prv, vsp_tbl_usable_wpf[vsp_ch]);

	/* initialize RPF register */
	(void)vsp_ins_clear_rpf_register(prv, vsp_tbl_usable_rpf[vsp_ch]);

	/* initialize WPF register */
	(void)vsp_ins_clear_wpf_register(prv, vsp_tbl_usable_wpf[vsp_ch]);

	/* initialize module register */
	(void)vsp_ins_clear_module_register(prv, vsp_tbl_usable_module[vsp_ch]);

	DPRINT("done\n");
	return 0;
}

/*
 * vsp_ins_quit_reg - Finalize register routine
 * @prv:		private data
 *
 */
long vsp_ins_quit_reg(VSP_PRIVATE_DATA *prv)
{
	DPRINT("called\n");

	/* unmap register */
	if (prv->base_reg) {
		iounmap(prv->base_reg);
		prv->base_reg = 0;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * vsp_ins_cb_function - Callback function routine
 * @prv:		private data
 * @ch:			channel number
 * @ercd:		error code
 *
 */
void vsp_ins_cb_function(VSP_PRIVATE_DATA *prv, unsigned char ch, long ercd)
{
	struct vsp_ch_info *ch_info;
	void (*cb_func)(T_VSP_CB *);
	T_VSP_CB cb_param;

	if (prv == NULL) {
		APRINT("[%s] prv pointer is NULL!!\n", __func__);
		return;
	}

	ch_info = &prv->ch_info[ch];
	if (ch_info->status == VSP_STAT_RUN) {
		/* clear RPF register */
		(void)vsp_ins_clear_rpf_register(prv, ch_info->reserved_rpf);

		/* clear WPF register */
		(void)vsp_ins_clear_wpf_register(prv, 0x00000001UL << ch);

		/* clear module register */
		(void)vsp_ins_clear_module_register(
			prv, ch_info->reserved_module);

		/* clear using flag */
		prv->use_rpf &= ~(ch_info->reserved_rpf);
		prv->use_module &= ~(ch_info->reserved_module);

		/* set callback parameter */
		cb_param.ercd = ercd;
		cb_param.ch = (unsigned long)ch;
		cb_param.userdata = ch_info->cb_userdata;

		/* copy histgram */
		(void)vsp_ins_get_module_register(
			prv, ch_info->reserved_module);

		/* set callback function */
		cb_func = ch_info->cb_func;

		ch_info->reserved_rpf = 0;
		ch_info->reserved_module = 0;

		/* update status */
		ch_info->status = VSP_STAT_READY;

		/* callback function*/
		if (cb_func)
			cb_func(&cb_param);
	}
}

/*
 * vsp_ins_ih - Interrupt handler routine
 * @irq:	interrupt number
 * @dev:	device ID supplied during interrupt registration
 *
 */
static irqreturn_t vsp_ins_ih(int irq, void *dev)
{
	VSP_PRIVATE_DATA *prv = (VSP_PRIVATE_DATA *)dev;
	struct vsp_ch_info *ch_info;

	volatile unsigned long dmy;
	unsigned long tmp;
	unsigned char ch;

	DPRINT("called\n");

	/* check finished channel */
	ch_info = &prv->ch_info[0];
	for (ch = 0; ch < VSP_WPF_MAX; ch++) {
		if (ch_info->status == VSP_STAT_RUN) {
			/* read control register */
			tmp = ioread32(ch_info->reg_irq_sta);

			if ((tmp & VSP_IRQ_FRMEND) == VSP_IRQ_FRMEND) {
				/* disable interrupt */
				iowrite32(0x00000000, ch_info->reg_irq_enb);

				/* clear interrupt */
				iowrite32(0x00000000, ch_info->reg_irq_sta);

				/* dummy read */
				dmy = ioread32(ch_info->reg_irq_sta);
				dmy = ioread32(ch_info->reg_irq_sta);

				/* callback function */
				vsp_ins_cb_function(prv, ch, 0);

				break;
			}
		}

		ch_info++;
	}

	DPRINT("done\n");
	return IRQ_HANDLED;
}

/*
 * vsp_ins_reg_ih - Registory interrupt handler routine
 * @pdev:		platform device
 * @prv:		private data
 * @vsp_ch:		VSP IP channel
 *
 */
long vsp_ins_reg_ih(
	struct platform_device *pdev,
	VSP_PRIVATE_DATA *prv,
	unsigned char vsp_ch)
{
	int ercd;

	DPRINT("called\n");

	prv->irq = platform_get_irq(pdev, vsp_tbl_resource_num[vsp_ch]);
	if (prv->irq < 0) {
		APRINT("[%s] platform_get_irq failed!! ercd=%d\n",
			__func__, prv->irq);
		return E_VSP_DEF_INH;
	}

	/* registory interrupt handler */
	ercd = request_irq(
		prv->irq,
		vsp_ins_ih,
		IRQF_SHARED,
		vsp_tbl_resource_name[vsp_ch], prv);
	if (ercd) {
		APRINT("[%s] request_irq failed!! ercd=%d, irq=%d\n",
			__func__, ercd, prv->irq
		);
		return E_VSP_DEF_INH;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * vsp_ins_unreg_ih - Unregistory interrupt handler routine
 * @prv:		private data
 *
 */
long vsp_ins_unreg_ih(VSP_PRIVATE_DATA *prv)
{
	DPRINT("called\n");

	/* release interrupt handler */
	if (prv->irq) {
		free_irq(prv->irq, prv);
		prv->irq = 0;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * vsp_ins_get_vsp_ip_num - Get VSP IP and WPF channel routine
 * @prv:		private data
 *
 */
long vsp_ins_get_vsp_ip_num(
	unsigned char *vsp, unsigned char *wpf, unsigned char ch)
{
	unsigned long bit;
	unsigned char vsp_ch, wpf_cnt;

	for (vsp_ch = 0; vsp_ch < VSP_IP_MAX; vsp_ch++) {
		wpf_cnt = vsp_ins_get_ch_cnt(vsp_tbl_usable_wpf[vsp_ch]);

		if (wpf_cnt > ch) {
			bit = 0x00000001UL << ch;
			if (vsp_tbl_usable_wpf[vsp_ch] & bit) {
				*vsp = vsp_ch;
				*wpf = ch;
				return 0;
			} else {
				break;
			}
		}
		ch -= wpf_cnt;
	}

	return E_VSP_PARA_CH;
}
