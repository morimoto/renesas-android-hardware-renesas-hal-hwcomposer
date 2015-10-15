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

#include "vspm_main.h"
#include "vspm_log.h"
#include "tddmac_drv.h"
#include "tddmac_drv_local.h"

/* control register offset table */
const unsigned short tddmac_tbl_reg_ctrl[TDDMAC_CH_MAX] = {
/*  CH0     CH1     CH2     CH3     CH4     CH5     CH6     CH7 */
	0x0020, 0x0024, 0x0028, 0x002C, 0x0120, 0x0124, 0x0128, 0x012C
};

/* input swap register offset table */
const unsigned short tddmac_tbl_reg_swap[TDDMAC_CH_MAX] = {
/*  CH0     CH1     CH2     CH3     CH4     CH5     CH6     CH7 */
	0x0030, 0x0034, 0x0038, 0x003C, 0x0130, 0x0134, 0x0138, 0x013C
};

/* channel register offset table */
const unsigned short tddmac_tbl_reg_offset[TDDMAC_CH_MAX] = {
/*	CH0     CH1     CH2     CH3     CH4     CH5     CH6     CH7 */
	0x0080, 0x00A0, 0x00C0, 0x00E0, 0x0180, 0x01A0, 0x01C0, 0x01E0
};

const unsigned long tddmac_tbl_ratio[TDDMAC_RATIO_MAX] = {
	0x00000000,
	0x00000400,	/* Magnify X direction */
	0x00000100,	/* Magnify Y direction */
	0x00000500	/* Magnify X and Y direction */
};

const unsigned long tddmac_tbl_rotation[TDDMAC_ROT_MAX] = {
	0x00000000,
	0x00001000,	/* 90 degree */
	0x00002000,	/* 270 degree */
	0x00003000	/* 180 degree */
};

const unsigned long tddmac_tbl_mirror[TDDMAC_MRR_MAX] = {
	0x00000000,
	0x00008000,	/* Horizontal inversion */
	0x00004000,	/* Vertival inversion */
	0x0000C000,	/* Horizontal and vertival inversion */
};

const unsigned long tddmac_tbl_format[TDDMAC_FORMAT_MAX] = {
	0x00000020,	/* Y data */
	0x00000040,	/* CbCr data (YCbCr4:2:0) */
	0x00000060,	/* CbCr data (YCbCr4:2:2) */
	0x00000000,	/* ARGB8888 */
	0x00000001,	/* RGBA8888 */
	0x00000002,	/* RGB888 */
	0x00000003,	/* RGB565 */
	0x00000004,	/* RGB332 */
	0x00000007,	/* pRGB14-666 */
	0x00000008,	/* pRGB4-444 */
	0x00000009,	/* RGB666 */
	0x0000000A,	/* BGR666 */
	0x0000000B,	/* BGR888 */
	0x0000000C,	/* ABGR8888 */
	0x0000000D	/* RGB565 (zero padding) */
};

const unsigned char tddmac_tbl_size_factor[TDDMAC_FORMAT_MAX] = {
	1,			/* Y data */
	1,			/* CbCr data (YCbCr4:2:0) */
	1,			/* CbCr data (YCbCr4:2:2) */
	4,			/* ARGB8888 */
	4,			/* RGBA8888 */
	3,			/* RGB888 */
	2,			/* RGB565 */
	1,			/* RGB332 */
	4,			/* pRGB14-666 */
	2,			/* pRGB4-444 */
	3,			/* RGB666 */
	3,			/* BGR666 */
	3,			/* BGR888 */
	4,			/* ABGR8888 */
	4			/* RGB565 (zero padding) */
};

/*
 * tddmac_ins_allocate_memory - Allocate memory routine
 * @prv:		private data
 *
 */
long tddmac_ins_allocate_memory(TDDMAC_PRIVATE_DATA **prv)
{
	/* allocate memory */
	*prv = kzalloc(sizeof(TDDMAC_PRIVATE_DATA), GFP_KERNEL);
	if (!*prv) {
		APRINT("[%s] allocate memory failed!!\n", __func__);
		return E_TDDMAC_NO_MEM;
	}

	DPRINT("allocate(%08x)\n", (unsigned int)(*prv));
	return 0;
}

/*
 * tddmac_ins_free_memory - Release memory routine
 * @prv:		private data
 *
 */
long tddmac_ins_free_memory(TDDMAC_PRIVATE_DATA *prv)
{
	/* clear memory */
	memset(prv, 0, sizeof(TDDMAC_PRIVATE_DATA));

	/* release memory */
	kfree(prv);

	DPRINT("release(%08x)\n", (unsigned int)prv);
	return 0;
}


/*
 * tddmac_ins_set_reg_table - Make register table routine
 * @prv:		private data
 *
 */
static long tddmac_ins_set_reg_table(TDDMAC_PRIVATE_DATA *prv)
{
	struct tddmac_ch_info *ch_info;
	int i;

	ch_info = &prv->ch_info[0];
	for (i = 0; i < TDDMAC_CH_MAX; i++) {
		ch_info->reg_ctrl
			= prv->base_reg + (unsigned long)tddmac_tbl_reg_ctrl[i];
		ch_info->reg_swap
			= prv->base_reg + (unsigned long)tddmac_tbl_reg_swap[i];

		ch_info->reg_sar = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x0000;
		ch_info->reg_dar = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x0004;
		ch_info->reg_dpxl = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x0008;
		ch_info->reg_sfmt = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x000C;
		ch_info->reg_dfmt = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x0010;
		ch_info->reg_sare = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x0014;
		ch_info->reg_dare = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x0018;
		ch_info->reg_dpxle = prv->base_reg +
			(unsigned long)tddmac_tbl_reg_offset[i] + 0x001C;

		ch_info++;
	}

	return 0;
}

/*
 * tddmac_ins_init_reg - Initialize register routine
 * @pdev:		platform device
 * @prv:		private data
 *
 */
long tddmac_ins_init_reg(
	struct platform_device *pdev, TDDMAC_PRIVATE_DATA *prv)
{
	struct resource *res;
	unsigned long tmp;

	unsigned char lp_ch;

	DPRINT("called\n");

	/* get an I/O memory resource for device */
	res = platform_get_resource(pdev, IORESOURCE_MEM, RESOURCE_TDDMAC);
	if (!res) {
		APRINT("[%s] platform_get_resource() failed!!\n", __func__);
		return E_TDDMAC_DEF_REG;
	}

	/* remap I/O memory */
	prv->base_reg = ioremap_nocache(res->start, resource_size(res));
	if (!prv->base_reg) {
		APRINT("[%s] ioremap_nocache() failed!!\n", __func__);
		return E_TDDMAC_DEF_REG;
	}

	/* make register table */
	(void)tddmac_ins_set_reg_table(prv);

	/* workaround a problem of write request for FIFO control. */
	tmp = ioread32(prv->base_reg + 0x0008);
	iowrite32((tmp | TDDMAC_CHTCTRL_OUT), prv->base_reg + 0x0008);

	/* initialize register */
	for (lp_ch = 0; lp_ch < TDDMAC_CH_MAX; lp_ch++)
		tddmac_ins_clear_dma(&prv->ch_info[lp_ch]);

	DPRINT("done\n");
	return 0;
}

/*
 * tddmac_ins_quit_reg - Finalize register routine
 * @prv:		private data
 *
 */
long tddmac_ins_quit_reg(TDDMAC_PRIVATE_DATA *prv)
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
 * tddmac_ins_ih - Interrupt handler routine
 * @irq:	interrupt number
 * @dev:	device ID supplied during interrupt registration
 *
 */
static irqreturn_t tddmac_ins_ih(int irq, void *dev)
{
	TDDMAC_PRIVATE_DATA *prv = (TDDMAC_PRIVATE_DATA *)dev;
	struct tddmac_ch_info *ch_info;

	volatile unsigned long dmy;
	unsigned long tmp;
	unsigned char lp_ch;

	DPRINT("called\n");

	/* check finished channel */
	ch_info = &prv->ch_info[0];
	for (lp_ch = 0; lp_ch < TDDMAC_CH_MAX; lp_ch++) {
		/* read control register */
		tmp = ioread32(ch_info->reg_ctrl);

		if ((tmp & TDDMAC_CTRL_TR_END) == TDDMAC_CTRL_TR_END) {
			/* disable interrupt */
			tmp &= ~(TDDMAC_CTRL_TR_END);
			iowrite32(tmp, ch_info->reg_ctrl);

			/* dummy read */
			dmy = ioread32(ch_info->reg_ctrl);
			dmy = ioread32(ch_info->reg_ctrl);

			/* callback function */
			tddmac_ins_cb_function(ch_info, lp_ch, 0);

			break;
		}

		ch_info++;
	}

	DPRINT("done\n");
	return IRQ_HANDLED;
}

/*
 * tddmac_ins_reg_ih - Registory interrupt handler routine
 * @pdev:		platform device
 * @prv:		private data
 *
 */
long tddmac_ins_reg_ih(struct platform_device *pdev, TDDMAC_PRIVATE_DATA *prv)
{
	int ercd;

	DPRINT("called\n");

	prv->irq = platform_get_irq(pdev, RESOURCE_TDDMAC);
	if (prv->irq < 0) {
		APRINT("[%s] platform_get_irq failed!! ercd=%d\n",
			__func__, prv->irq);
		return E_TDDMAC_DEF_INH;
	}

	/* registory interrupt handler */
	ercd = request_irq(
		prv->irq, tddmac_ins_ih, IRQF_SHARED, RESNAME "-tddmac", prv);
	if (ercd) {
		APRINT("[%s] request_irq failed!! ercd=%d, irq=%d\n",
			__func__, ercd, prv->irq);
		return E_TDDMAC_DEF_INH;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * tddmac_ins_unreg_ih - Unregistory interrupt handler routine
 * @prv:		private data
 *
 */
long tddmac_ins_unreg_ih(TDDMAC_PRIVATE_DATA *prv)
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
 * tddmac_ins_cb_function - Callback function routine
 * @ch_info:	Channel information
 * @ch:			Channel number
 * @ercd:		Error code(0 or -ECANCELD)
 *
 */
void tddmac_ins_cb_function(
	struct tddmac_ch_info *ch_info, unsigned char ch, long ercd)
{
	void (*cb_func)(T_TDDMAC_CB *);
	T_TDDMAC_CB cb_param;

	if (ch_info->status == TDDMAC_STAT_RUN) {
		/* set callback parameter */
		cb_param.ercd = ercd;
		cb_param.sub_ercd = 0;	/* not used */
		cb_param.channel = (unsigned long)ch;
		cb_param.count =
			(ch_info->val_dpxl - ioread32(ch_info->reg_dpxle))
			& 0xFFFF;
		cb_param.userdata = ch_info->cb_userdata;

		/* set callback function */
		cb_func = ch_info->cb_finished;

		/* update status */
		ch_info->status = TDDMAC_STAT_READY;

		/* callback function */
		if (cb_func)
			cb_func(&cb_param);
	}
}

/*
 * tddmac_ins_check_init_param - Check initialize parameter routine
 * @param:		T_TDDMAC_INIT parameter
 *
 */
long tddmac_ins_check_init_param(T_TDDMAC_INIT *param)
{
	/* no check */
	return 0;
}

/*
 * tddmac_ins_check_mode_param - Check mode parameter routine
 * @param:		T_TDDMAC_MODE parameter
 *
 */
long tddmac_ins_check_mode_param(T_TDDMAC_MODE *param)
{
	if (param->renewal != TDDMAC_RNEW_NORMAL)
		return E_TDDMAC_PARA_RENEWAL;

	if (param->resource != TDDMAC_RES_AUTO)
		return E_TDDMAC_PARA_RESOURCE;

	return 0;
}

/*
 * tddmac_ins_check_request_param - Check request parameter routine
 * @param:		T_TDDMAC_REQUEST parameter
 *
 */
long tddmac_ins_check_request_param(T_TDDMAC_REQUEST *param)
{
	unsigned long tmp;

	/* check src/dst format parameter */
	switch (param->src_format) {
	case TDDMAC_FORMAT_Y:
		if (param->dst_format != TDDMAC_FORMAT_Y)
			return E_TDDMAC_PARA_DST_FORMAT;
		break;
	case TDDMAC_FORMAT_C420:
	case TDDMAC_FORMAT_C422:
		if ((param->dst_format != TDDMAC_FORMAT_C420) &&
			(param->dst_format != TDDMAC_FORMAT_C422)) {
			return E_TDDMAC_PARA_DST_FORMAT;
		}
		break;
	default:
		if ((param->src_format >= TDDMAC_FORMAT_ARGB8888) &&
			(param->src_format <= TDDMAC_FORMAT_RGB0565)) {
			if ((param->dst_format < TDDMAC_FORMAT_ARGB8888) ||
				(param->dst_format > TDDMAC_FORMAT_RGB0565)) {
				return E_TDDMAC_PARA_DST_FORMAT;
			}
		} else {
			return E_TDDMAC_PARA_SRC_FORMAT;
		}
		break;
	}

	/* check source address parameter */
	if (param->src_adr == NULL)
		return E_TDDMAC_PARA_SRC_ADR;

	/* check destination address parameter */
	if ((param->dst_adr == NULL) ||
		((unsigned long)param->dst_adr & 0xF)) {
		return E_TDDMAC_PARA_DST_ADR;
	}

	/* check ratio/mirror/rotation parameter */
	if ((param->mirror == TDDMAC_MRR_OFF) &&
		(param->rotation == TDDMAC_ROT_OFF)) {
		/* check ratio parameter */
		if (param->ratio >= TDDMAC_RATIO_MAX)
			return E_TDDMAC_PARA_RATIO;

		tmp = (unsigned long)tddmac_tbl_size_factor[param->src_format];
	} else {
		/* check mirror parameter */
		if (param->mirror >= TDDMAC_MRR_MAX)
			return E_TDDMAC_PARA_MIRROR;

		/* check rotation parameter */
		if (param->rotation >= TDDMAC_ROT_MAX)
			return E_TDDMAC_PARA_ROTATION;

		/* check ratio parameter */
		if (param->ratio != TDDMAC_RATIO_1_1)
			return E_TDDMAC_PARA_RATIO;

		tmp = 16;
	}

	/* check source stride parameter */
	if ((param->src_stride == 0) ||
		(param->src_stride % tmp)) {
		return E_TDDMAC_PARA_SRC_STRIDE;
	}

	/* check destination stride parameter */
	if ((param->dst_stride == 0) ||
		(param->dst_stride % 16)) {
		return E_TDDMAC_PARA_DST_STRIDE;
	}

	/* check source y offset */
	if (param->src_format == TDDMAC_FORMAT_C420) {
		if (param->src_y_offset & 0x1)
			return E_TDDMAC_PARA_SRC_Y_OFFSET;
	}

	/* check destination width */
	if (param->dst_width == 0)
		return E_TDDMAC_PARA_DST_WIDTH;

	tmp = tddmac_tbl_mirror[param->mirror] |
		tddmac_tbl_rotation[param->rotation];
	if ((param->mirror == TDDMAC_MRR_HV) ||
		(param->rotation == TDDMAC_ROT_180)) {
		if (param->rotation != TDDMAC_ROT_OFF)
			tmp = (~tmp) & TDDMAC_CTRL_MASK;
	}

	switch (tmp) {
	/* case DAR specified point "C" */
	/* case DAR specified point "D" */
	case TDDMAC_CTRL_ROTR:
	case TDDMAC_CTRL_HMRR:
	case TDDMAC_CTRL_VMRR|TDDMAC_CTRL_HMRR:	/* = 180 degree */
	case TDDMAC_CTRL_VMRR|TDDMAC_CTRL_ROTR:
	case TDDMAC_CTRL_HMRR|TDDMAC_CTRL_ROTL:
		tmp = ((unsigned long)param->dst_width) *
		((unsigned long)tddmac_tbl_size_factor[param->dst_format]);
		if (tmp % 16)
			return E_TDDMAC_PARA_DST_WIDTH;
		break;
	/* case DAR specified point "A" */
	/* case DAR specified point "B" */
	default:
		if ((param->dst_format == TDDMAC_FORMAT_C420) ||
			(param->dst_format == TDDMAC_FORMAT_C422)) {
			if (param->dst_width & 0x1)
				return E_TDDMAC_PARA_DST_WIDTH;
		}
		break;
	}

	/* check destination height */
	if (param->dst_height == 0)
		return E_TDDMAC_PARA_DST_HEIGHT;

	if ((param->src_format == TDDMAC_FORMAT_C420) ||
		(param->dst_format == TDDMAC_FORMAT_C420)) {
		if (param->dst_height & 0x1)
			return E_TDDMAC_PARA_DST_HEIGHT;
	}

	/* check destination x offset */
	tmp = ((unsigned long)param->dst_x_offset) *
		((unsigned long)tddmac_tbl_size_factor[param->dst_format]);
	if (tmp % 16)
		return E_TDDMAC_PARA_DST_X_OFFSET;

	/* check destination y offset */
	if (param->dst_format == TDDMAC_FORMAT_C420) {
		if (param->dst_y_offset & 0x1)
			return E_TDDMAC_PARA_DST_Y_OFFSET;
	}

	/* check alpha parameter */
	if (param->alpha_ena >= TDDMAC_SRCALPHA_MAX)
		return E_TDDMAC_PARA_ALPHA_ENA;

	return 0;
}

/*
 * tddmac_ins_change_dar_address - Change DAR address routine
 * @ch_info:	Channel information
 * @param:		T_TDDMAC_REQUEST parameter
 *
 */
static void tddmac_ins_change_address_dar(
	struct tddmac_ch_info *ch_info, T_TDDMAC_REQUEST *param)
{
	unsigned long dst_stride = (unsigned long)param->dst_stride;
	unsigned long dst_height = (unsigned long)param->dst_height;

	unsigned long pic_stride;

	/* calculate */
	if (param->dst_format == TDDMAC_FORMAT_C420)
		dst_height >>= 1;

	pic_stride = ((unsigned long)param->dst_width) *
		((unsigned long)tddmac_tbl_size_factor[param->dst_format]);

	switch (ch_info->val_ctrl & TDDMAC_CTRL_MASK) {
	/* case DAR specified point "B" */
	case TDDMAC_CTRL_ROTL:
	case TDDMAC_CTRL_VMRR:
		ch_info->val_dar += (dst_stride * (dst_height-1));
		break;
	/* case DAR specified point "C" */
	case TDDMAC_CTRL_ROTR:
	case TDDMAC_CTRL_HMRR:
		ch_info->val_dar += pic_stride;
		break;
	/* case DAR specified point "D" */
	case TDDMAC_CTRL_VMRR|TDDMAC_CTRL_HMRR:	/* = 180 degree */
	case TDDMAC_CTRL_VMRR|TDDMAC_CTRL_ROTR:
	case TDDMAC_CTRL_HMRR|TDDMAC_CTRL_ROTL:
		ch_info->val_dar += (dst_stride * (dst_height-1)) + pic_stride;
		break;
	/* case DAR specified point "A" */
	default:
		break;
	}
}

/*
 * tddmac_ins_set_request_param - Set request parameter routine
 * @ch_info:	Channel information
 * @param:		T_TDDMAC_REQUEST parameter
 *
 */
void tddmac_ins_set_request_param(
	struct tddmac_ch_info *ch_info, T_TDDMAC_REQUEST *param)
{
	unsigned long val_tmp;

	/* set callback functions */
	ch_info->cb_finished = param->cb_finished;
	ch_info->cb_userdata = param->userdata;

	/* make control register(CHnCTRL) value */
	ch_info->val_ctrl = 0;

	if (param->cb_finished != NULL)
		ch_info->val_ctrl |= TDDMAC_CTRL_TIE;

	ch_info->val_ctrl |= tddmac_tbl_ratio[param->ratio];
	val_tmp =
		tddmac_tbl_mirror[param->mirror] |
		tddmac_tbl_rotation[param->rotation];
	if ((param->mirror == TDDMAC_MRR_HV) ||
		(param->rotation == TDDMAC_ROT_180)) {
		if (param->rotation != TDDMAC_ROT_OFF)
			val_tmp = (~val_tmp) & TDDMAC_CTRL_MASK;
	}

	ch_info->val_ctrl |= val_tmp;

	/* make destination pixel register(CHnDPXL) value */
	val_tmp = (unsigned long)param->dst_width;
	if ((param->dst_format == TDDMAC_FORMAT_C422) ||
		(param->dst_format == TDDMAC_FORMAT_C420)) {
		val_tmp >>= 1;	/* half pixel */
	}
	ch_info->val_dpxl = val_tmp << 16;

	val_tmp = (unsigned long)param->dst_height;
	if (param->dst_format == TDDMAC_FORMAT_C420)
		val_tmp >>= 1;	/* half pixel */
	ch_info->val_dpxl |= val_tmp;

	/* make source address register(CHnSAR) value */
	val_tmp = (unsigned long)param->src_stride;
	if (param->src_format == TDDMAC_FORMAT_C420)
		val_tmp *= (unsigned long)(param->src_y_offset >> 1);
	else
		val_tmp *= (unsigned long)(param->src_y_offset);
	val_tmp += ((unsigned long)param->src_x_offset) *
		(unsigned long)tddmac_tbl_size_factor[param->src_format];

	ch_info->val_sar = (unsigned long)param->src_adr + val_tmp;

	/* make source format register(CHnDFMT) value */
	ch_info->val_sfmt = ((unsigned long)param->src_stride) << 16;
	ch_info->val_sfmt |= tddmac_tbl_format[param->src_format];

	/* make destination addres register(CHnDAR) value */
	val_tmp = (unsigned long)param->dst_stride;
	if (param->dst_format == TDDMAC_FORMAT_C420)
		val_tmp *= (unsigned long)(param->dst_y_offset >> 1);
	else
		val_tmp *= (unsigned long)(param->dst_y_offset);
	val_tmp += ((unsigned long)param->dst_x_offset) *
		(unsigned long)tddmac_tbl_size_factor[param->dst_format];

	ch_info->val_dar = (unsigned long)param->dst_adr + val_tmp;

	/* change DAR address */
	tddmac_ins_change_address_dar(ch_info, param);

	/* make destination format register(CHnDFMT) value */
	ch_info->val_dfmt = ((unsigned long)param->dst_stride) << 16;
	ch_info->val_dfmt |= tddmac_tbl_format[param->dst_format];
	if (param->alpha_ena == TDDMAC_SRCALPHA_ENABLE)
		ch_info->val_dfmt |= TDDMAC_DFMT_AV;
	ch_info->val_dfmt |= ((unsigned long)param->alpha) << 8;

	/* make swap register(CHnSWAP) */
	ch_info->val_swap = ((unsigned long)param->swap) & TDDMAC_SWAP_MASK;

	return;
}

/*
 * tddmac_ins_start_dma - Set starting DMA register routine
 * @ch_info:	Channel information
 *
 */
long tddmac_ins_start_dma(struct tddmac_ch_info *ch_info)
{
	unsigned long tmp_ctrl;

	/* set register */
	iowrite32(ch_info->val_swap, ch_info->reg_swap);
	DPRINT("CHxSWAP = %08x\n", (unsigned int)ch_info->val_swap);

	iowrite32(ch_info->val_ctrl, ch_info->reg_ctrl);
	DPRINT("CHxCTRL = %08x\n", (unsigned int)ch_info->val_ctrl);
	iowrite32(ch_info->val_sar, ch_info->reg_sar);
	DPRINT("CHxSAR = %08x\n", (unsigned int)ch_info->val_sar);
	iowrite32(ch_info->val_dar, ch_info->reg_dar);
	DPRINT("CHxDAR = %08x\n", (unsigned int)ch_info->val_dar);
	iowrite32(ch_info->val_dpxl, ch_info->reg_dpxl);
	DPRINT("CHxDPXL = %08x\n", (unsigned int)ch_info->val_dpxl);
	iowrite32(ch_info->val_sfmt, ch_info->reg_sfmt);
	DPRINT("CHxSFMT = %08x\n", (unsigned int)ch_info->val_sfmt);
	iowrite32(ch_info->val_dfmt, ch_info->reg_dfmt);
	DPRINT("CHxDFMT = %08x\n", (unsigned int)ch_info->val_dfmt);

	/* start transfer */
	tmp_ctrl = ch_info->val_ctrl | TDDMAC_CTRL_DMAEN;
	iowrite32(tmp_ctrl, ch_info->reg_ctrl);
	DPRINT("CHxCTRL = %08x\n", (unsigned int)tmp_ctrl);

	return 0;
}

/*
 * tddmac_ins_stop_dma - Set stopping DMA register routine
 * @ch_info:	Channel information
 *
 */
long tddmac_ins_stop_dma(struct tddmac_ch_info *ch_info)
{
	volatile unsigned long dmy;
	unsigned long tmp_ctrl;

	/* disable interrupt */
	tmp_ctrl = ch_info->val_ctrl & ~TDDMAC_CTRL_TIE;
	iowrite32(tmp_ctrl, ch_info->reg_ctrl);

	/* dummy read */
	dmy = ioread32(ch_info->reg_ctrl);
	dmy = ioread32(ch_info->reg_ctrl);

	/* stop transfer */
	tmp_ctrl = ch_info->val_ctrl | TDDMAC_CTRL_STP;
	iowrite32(tmp_ctrl, ch_info->reg_ctrl);
	DPRINT("CHxCTRL = %08x\n", (unsigned int)tmp_ctrl);

	return 0;
}

/*
 * tddmac_ins_clear_dma - Set clearing DMA register routine
 * @ch_info:	Channel information
 *
 */
void tddmac_ins_clear_dma(struct tddmac_ch_info *ch_info)
{
	/* clear register */
	iowrite32(0x00000000, ch_info->reg_ctrl);
	iowrite32(0x00000000, ch_info->reg_sar);
	iowrite32(0x00000000, ch_info->reg_dar);
	iowrite32(0x00000000, ch_info->reg_dpxl);
	iowrite32(0x00000000, ch_info->reg_sfmt);
	iowrite32(0x00000000, ch_info->reg_dfmt);
}
