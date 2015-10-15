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

#ifndef _TDDMAC_DRV_LOCAL_H_
#define _TDDMAC_DRV_LOCAL_H_

/* define status */
#define TDDMAC_STAT_NOT_INIT	0
#define TDDMAC_STAT_INIT		1
#define TDDMAC_STAT_READY		2
#define TDDMAC_STAT_RUN			3

/* define regiter value */
#define TDDMAC_CTRL_HMRR	(0x00008000)	/* horizontal inversion */
#define TDDMAC_CTRL_VMRR	(0x00004000)	/* vertical inversion */
#define TDDMAC_CTRL_ROTL	(0x00002000)	/* 270 degree rotation */
#define TDDMAC_CTRL_ROTR	(0x00001000)	/* 90 degree rotation */
#define TDDMAC_CTRL_TIE		(0x00000020)
#define TDDMAC_CTRL_TE		(0x00000010)
#define TDDMAC_CTRL_STP		(0x00000002)
#define TDDMAC_CTRL_DMAEN	(0x00000001)
#define TDDMAC_CTRL_MRR_ROL	\
	(TDDMAC_CTRL_HMRR|TDDMAC_CTRL_VMRR|\
	 TDDMAC_CTRL_ROTL|TDDMAC_CTRL_ROTR)
#define TDDMAC_CTRL_TR_END	(TDDMAC_CTRL_TIE|TDDMAC_CTRL_TE)

#define TDDMAC_DFMT_AV		(0x00000080)

#define TDDMAC_CHTCTRL_OUT	(0x00000020)

#define TDDMAC_CTRL_MASK	(TDDMAC_CTRL_MRR_ROL)
#define TDDMAC_SWAP_MASK \
	(TDDMAC_SWAP_OLS|TDDMAC_SWAP_OWS|TDDMAC_SWAP_OBS|\
	 TDDMAC_SWAP_ILS|TDDMAC_SWAP_IWS|TDDMAC_SWAP_IBS)


/* channel information structure */
struct tddmac_ch_info {
	unsigned char status;
	void (*cb_finished)(T_TDDMAC_CB *);
	void *cb_userdata;

	unsigned long val_ctrl;		/* control register */
	unsigned long val_swap;		/* input swap register */
	unsigned long val_sar;		/* source address register */
	unsigned long val_dar;		/* destination address register */
	unsigned long val_dpxl;		/* destination pixel register */
	unsigned long val_sfmt;		/* source format register */
	unsigned long val_dfmt;		/* destination format register */

	void __iomem *reg_ctrl;		/* control register */
	void __iomem *reg_swap;		/* input swap register */
	void __iomem *reg_sar;		/* source address register */
	void __iomem *reg_dar;		/* destination address register */
	void __iomem *reg_dpxl;		/* destination pixel register */
	void __iomem *reg_sfmt;		/* source format register */
	void __iomem *reg_dfmt;		/* destination format register */
	void __iomem *reg_sare;		/* source line address register */
	void __iomem *reg_dare;		/* destination line address register */
	void __iomem *reg_dpxle;	/* dest pixel processing register */
};

/* private data structure */
typedef struct TDDMAC_PRIVATE_DATA_T {
	void __iomem *base_reg;
	int irq;

	struct tddmac_ch_info ch_info[TDDMAC_CH_MAX];
} TDDMAC_PRIVATE_DATA;

/* define local functions */
long tddmac_ins_allocate_memory(TDDMAC_PRIVATE_DATA **prv);
long tddmac_ins_free_memory(TDDMAC_PRIVATE_DATA *prv);

long tddmac_ins_init_reg(
	struct platform_device *pdev, TDDMAC_PRIVATE_DATA *prv);
long tddmac_ins_quit_reg(TDDMAC_PRIVATE_DATA *prv);

long tddmac_ins_reg_ih(struct platform_device *pdev, TDDMAC_PRIVATE_DATA *prv);
long tddmac_ins_unreg_ih(TDDMAC_PRIVATE_DATA *prv);

void tddmac_ins_cb_function(
	struct tddmac_ch_info *ch_info, unsigned char ch, long ercd);

long tddmac_ins_check_init_param(T_TDDMAC_INIT *param);
long tddmac_ins_check_mode_param(T_TDDMAC_MODE *param);
long tddmac_ins_check_request_param(T_TDDMAC_REQUEST *param);

void tddmac_ins_set_request_param(
	struct tddmac_ch_info *ch_info, T_TDDMAC_REQUEST *param);

long tddmac_ins_start_dma(struct tddmac_ch_info *ch_info);
long tddmac_ins_stop_dma(struct tddmac_ch_info *ch_info);
void tddmac_ins_clear_dma(struct tddmac_ch_info *ch_info);

#endif
