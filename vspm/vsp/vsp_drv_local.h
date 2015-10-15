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

#ifndef _VSP_DRV_L_H_
#define _VSP_DRV_L_H_

/* define IP number */
enum {
	VSP_IP_VSPS = 0,
#ifdef USE_VSPR
	VSP_IP_VSPR,
#endif /* USE_VSPR */
#ifdef USE_VSPD0
	VSP_IP_VSPD0,
#endif /* USE_VSPD0 */
#ifdef USE_VSPD1
	VSP_IP_VSPD1,
#endif /* USE_VSPD1 */
	VSP_IP_MAX
};

/* define status read counter */
#define VSP_STATUS_LOOP_CNT		(1000)

/* define module maximum */
#define VSP_WPF_MAX				(4)
#define VSP_SRU_MAX				(1)
#define VSP_UDS_MAX				(3)
#define VSP_LUT_MAX				(1)
#define VSP_CLU_MAX				(1)
#define VSP_HST_MAX				(1)
#define VSP_HSI_MAX				(1)
#define VSP_BRU_MAX				(1)
#define VSP_HGO_MAX				(1)
#define VSP_HGT_MAX				(1)

#define VSP_SRC_MAX				(4)
#define VSP_BRU_CH_MAX			(4)

/* define status */
#define VSP_STAT_NOT_INIT		0
#define VSP_STAT_INIT			1
#define VSP_STAT_READY			2
#define VSP_STAT_RUN			3

/* define */
#define VSP_FALSE				0
#define VSP_TRUE				1

/* define color space */
#define VSP_COLOR_NO			(0)
#define VSP_COLOR_RGB			(1)
#define VSP_COLOR_YUV			(2)
#define VSP_COLOR_HSV			(3)

/* define channel bits */
#define VSP_RPF0_USE			(0x0001)
#define VSP_RPF1_USE			(0x0002)
#define VSP_RPF2_USE			(0x0004)
#define VSP_RPF3_USE			(0x0008)
#define VSP_RPF4_USE			(0x0010)

#define VSP_WPF0_USE			(0x0001)
#define VSP_WPF1_USE			(0x0002)
#define VSP_WPF2_USE			(0x0004)
#define VSP_WPF3_USE			(0x0008)

/* define usable pass route */
#define VSP_RPF_USABLE_DPR \
	(VSP_SRU_USE|	\
	 VSP_UDS_USE|	\
	 VSP_UDS1_USE|	\
	 VSP_UDS2_USE|	\
	 VSP_LUT_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HST_USE|	\
	 VSP_BRU_USE)

#define VSP_SRU_USABLE_DPR \
	(VSP_UDS_USE|	\
	 VSP_UDS1_USE|	\
	 VSP_UDS2_USE|	\
	 VSP_LUT_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HST_USE|	\
	 VSP_BRU_USE)

#define VSP_UDS_USABLE_DPR \
	(VSP_SRU_USE|	\
	 VSP_LUT_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HST_USE|	\
	 VSP_BRU_USE)

#define VSP_LUT_USABLE_DPR \
	(VSP_SRU_USE|	\
	 VSP_UDS_USE|	\
	 VSP_UDS1_USE|	\
	 VSP_UDS2_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HST_USE|	\
	 VSP_HSI_USE|	\
	 VSP_BRU_USE)

#define VSP_CLU_USABLE_DPR \
	(VSP_SRU_USE|	\
	 VSP_UDS_USE|	\
	 VSP_UDS1_USE|	\
	 VSP_UDS2_USE|	\
	 VSP_LUT_USE|	\
	 VSP_HST_USE|	\
	 VSP_HSI_USE|	\
	 VSP_BRU_USE)

#define VSP_HST_USABLE_DPR \
	(VSP_LUT_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HSI_USE)

#define VSP_HSI_USABLE_DPR \
	(VSP_SRU_USE|	\
	 VSP_UDS_USE|	\
	 VSP_UDS1_USE|	\
	 VSP_UDS2_USE|	\
	 VSP_LUT_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HST_USE|	\
	 VSP_BRU_USE)

#define VSP_BRU_USABLE_DPR \
	(VSP_SRU_USE|	\
	 VSP_UDS_USE|	\
	 VSP_UDS1_USE|	\
	 VSP_UDS2_USE|	\
	 VSP_LUT_USE|	\
	 VSP_CLU_USE|	\
	 VSP_HST_USE)

/* define register offset */
#define VSP_WPF0_CMD			(0x0000)
#define VSP_WPF1_CMD			(0x0004)
#define VSP_WPF2_CMD			(0x0008)
#define VSP_WPF3_CMD			(0x000C)

#define VSP_SRESET				(0x0028)
#define VSP_STATUS				(0x0038)

#define VSP_WPF0_IRQ_ENB		(0x0048)
#define VSP_WPF0_IRQ_STA		(0x004C)
#define VSP_WPF1_IRQ_ENB		(0x0054)
#define VSP_WPF1_IRQ_STA		(0x0058)
#define VSP_WPF2_IRQ_ENB		(0x0060)
#define VSP_WPF2_IRQ_STA		(0x0064)
#define VSP_WPF3_IRQ_ENB		(0x006C)
#define VSP_WPF3_IRQ_STA		(0x0070)

/* RPFn control registers offset */
#define VSP_RPF0_OFFSET			(0x0300)
#define VSP_RPF1_OFFSET			(0x0400)
#define VSP_RPF2_OFFSET			(0x0500)
#define VSP_RPF3_OFFSET			(0x0600)
#define VSP_RPF4_OFFSET			(0x0700)

#define VSP_RPF_SRC_BSIZE		(0x0000)
#define VSP_RPF_SRC_ESIZE		(0x0004)
#define VSP_RPF_INFMT			(0x0008)
#define VSP_RPF_DSWAP			(0x000C)
#define VSP_RPF_LOC				(0x0010)
#define VSP_RPF_ALPH_SEL		(0x0014)
#define VSP_RPF_VRTCOL_SET		(0x0018)
#define VSP_RPF_MSKCTRL			(0x001C)
#define VSP_RPF_MSKSET0			(0x0020)
#define VSP_RPF_MSKSET1			(0x0024)
#define VSP_RPF_CKEY_CTRL		(0x0028)
#define VSP_RPF_CKEY_SET0		(0x002C)
#define VSP_RPF_CKEY_SET1		(0x0030)
#define VSP_RPF_SRCM_PSTRIDE	(0x0034)
#define VSP_RPF_SRCM_ASTRIDE	(0x0038)
#define VSP_RPF_SRCM_ADDR_Y		(0x003C)
#define VSP_RPF_SRCM_ADDR_C0	(0x0040)
#define VSP_RPF_SRCM_ADDR_C1	(0x0044)
#define VSP_RPF_SRCM_ADDR_AI	(0x0048)
#define VSP_RPF_CHPRI_CTRL		(0x0050)

#define VSP_RPF0_CLUT_OFFSET	(0x4000)
#define VSP_RPF1_CLUT_OFFSET	(0x4400)
#define VSP_RPF2_CLUT_OFFSET	(0x4800)
#define VSP_RPF3_CLUT_OFFSET	(0x4C00)
#define VSP_RPF4_CLUT_OFFSET	(0x5000)

/* WPF control registers offset */
#define VSP_WPF0_OFFSET			(0x1000)
#define VSP_WPF1_OFFSET			(0x1100)
#define VSP_WPF2_OFFSET			(0x1200)
#define VSP_WPF3_OFFSET			(0x1300)

#define VSP_WPF_SRCRPFL			(0x0000)
#define VSP_WPF_HSZCLIP			(0x0004)
#define VSP_WPF_VSZCLIP			(0x0008)
#define VSP_WPF_OUTFMT			(0x000C)
#define VSP_WPF_DSWAP			(0x0010)
#define VSP_WPF_RNDCTRL			(0x0014)
#define VSP_WPF_DSTM_STRIDE_Y	(0x001C)
#define VSP_WPF_DSTM_STRIDE_C	(0x0020)
#define VSP_WPF_DSTM_ADDR_Y		(0x0024)
#define VSP_WPF_DSTM_ADDR_C0	(0x0028)
#define VSP_WPF_DSTM_ADDR_C1	(0x002C)

/* DPR control registers offset */
#define VSP_DPR_RPF0_ROUTE		(0x2000)
#define VSP_DPR_RPF1_ROUTE		(0x2004)
#define VSP_DPR_RPF2_ROUTE		(0x2008)
#define VSP_DPR_RPF3_ROUTE		(0x200C)
#define VSP_DPR_RPF4_ROUTE		(0x2010)
#define VSP_DPR_WPF0_FPORCH		(0x2014)
#define VSP_DPR_WPF1_FPORCH		(0x2018)
#define VSP_DPR_WPF2_FPORCH		(0x201C)
#define VSP_DPR_WPF3_FPORCH		(0x2020)
#define VSP_DPR_SRU_ROUTE		(0x2024)
#define VSP_DPR_UDS0_ROUTE		(0x2028)
#define VSP_DPR_UDS1_ROUTE		(0x202C)
#define VSP_DPR_UDS2_ROUTE		(0x2030)
#define VSP_DPR_LUT_ROUTE		(0x203C)
#define VSP_DPR_CLU_ROUTE		(0x2040)
#define VSP_DPR_HST_ROUTE		(0x2044)
#define VSP_DPR_HSI_ROUTE		(0x2048)
#define VSP_DPR_BRU_ROUTE		(0x204C)
#define VSP_DPR_HGO_SMPPT		(0x2054)
#define VSP_DPR_HGT_SMPPT		(0x2058)

/* SRU control registers offset */
#define VSP_SRU_OFFSET			(0x2200)

#define VSP_SRU_CTRL0			(0x0000)
#define VSP_SRU_CTRL1			(0x0004)
#define VSP_SRU_CTRL2			(0x0008)

/* UDS control registers offset */
#define VSP_UDS0_OFFSET			(0x2300)
#define VSP_UDS1_OFFSET			(0x2400)
#define VSP_UDS2_OFFSET			(0x2500)

#define VSP_UDS_CTRL			(0x0000)
#define VSP_UDS_SCALE			(0x0004)
#define VSP_UDS_ALPTH			(0x0008)
#define VSP_UDS_ALPVAL			(0x000C)
#define VSP_UDS_PASS_BWIDTH		(0x0010)
#define VSP_UDS_IPC				(0x0018)
#define VSP_UDS_HSZCLIP			(0x001C)
#define VSP_UDS_VSZCLIP			(0x0020)
#define VSP_UDS_CLIP_SIZE		(0x0024)
#define VSP_UDS_FILL_COLOR		(0x0028)

/* LUT control registers offset */
#define VSP_LUT_OFFSET			(0x2800)
#define VSP_LUT_TBL_OFFSET		(0x7000)

/* CLU control registers offset */
#define VSP_CLU_OFFSET			(0x2900)
#define VSP_CLU_TBL_ADDR		(0x7400)
#define VSP_CLU_TBL_DATA		(0x7404)

/* HST control registers offset */
#define VSP_HST_OFFSET			(0x2A00)

/* HSI control registers offset */
#define VSP_HSI_OFFSET			(0x2B00)

/* BRU control registers offset */
#define VSP_BRU_OFFSET			(0x2C00)

#define VSP_BRU_INCTRL			(0x0000)
#define VSP_BRU_VIRRPF_SIZE		(0x0004)
#define VSP_BRU_VIRRPF_LOC		(0x0008)
#define VSP_BRU_VIRRPF_COL		(0x000C)
#define VSP_BRUA_CTRL			(0x0010)
#define VSP_BRUA_BLD			(0x0014)
#define VSP_BRUB_CTRL			(0x0018)
#define VSP_BRUB_BLD			(0x001C)
#define VSP_BRUC_CTRL			(0x0020)
#define VSP_BRUC_BLD			(0x0024)
#define VSP_BRUD_CTRL			(0x0028)
#define VSP_BRUD_BLD			(0x002C)
#define VSP_BRU_ROP				(0x0030)

/* HGO control registers offset */
#define VSP_HGO_OFFSET			(0x3000)

#define VSP_HGO_SIZE			(0x0004)
#define VSP_HGO_MODE			(0x0008)
#define VSP_HGO_R_HISTO_OFFSET	(0x3030)
#define VSP_HGO_G_HISTO_OFFSET	(0x3140)
#define VSP_HGO_B_HISTO_OFFSET	(0x3250)
#define VSP_HGO_REGRST			(0x03FC)

/* HGT control registers offset */
#define VSP_HGT_OFFSET			(0x3400)

#define VSP_HGT_SIZE			(0x0004)
#define VSP_HGT_MODE			(0x0008)
#define VSP_HGT_HUE_AREA0		(0x000C)
#define VSP_HGT_HUE_AREA1		(0x0010)
#define VSP_HGT_HUE_AREA2		(0x0014)
#define VSP_HGT_HUE_AREA3		(0x0018)
#define VSP_HGT_HUE_AREA4		(0x001C)
#define VSP_HGT_HUE_AREA5		(0x0020)
#define VSP_HGT_HIST_OFFSET		(0x3450)
#define VSP_HGT_REGRST			(0x03FC)

/* define register set value */
#define VSP_CMD_STRCMD			(0x00000001)
#define VSP_IRQ_FRMEND			(0x00000001)

#define VSP_RPF_INFMT_VIR		(0x10000000)
#define VSP_RPF_INFMT_SPYCS		(0x00008000)
#define VSP_RPF_INFMT_SPUVS		(0x00004000)
#define VSP_RPF_INFMT_RDTM1		(0x00000400)
#define VSP_RPF_INFMT_RDTM0		(0x00000200)
#define VSP_RPF_INFMT_CSC		(0x00000100)
#define VSP_RPF_INFMT_RDFMT		(0x0000007F)

#define VSP_RPF_INFMT_MSK \
	(VSP_RPF_INFMT_SPYCS|\
	 VSP_RPF_INFMT_SPUVS|\
	 VSP_RPF_INFMT_RDFMT)

#define VSP_RPF_MSKCTRL_EN		(0x01000000)

#define VSP_RPF_CKEY_CTRL_CV	(0x00000010)
#define VSP_RPF_CKEY_CTRL_MSK	(VSP_ALPHA_AL1|VSP_ALPHA_AL2)

#define VSP_WPF_HSZCLIP_HCEN	(0x10000000)
#define VSP_WPF_VSZCLIP_VCEN	(0x10000000)

#define VSP_WPF_OUTFMT_PXA		(0x00800000)
#define VSP_WPF_OUTFMT_SPYCS	(0x00008000)
#define VSP_WPF_OUTFMT_SPUVS	(0x00004000)
#define VSP_WPF_OUTFMT_DITH		(0x00003000)
#define VSP_WPF_OUTFMT_WRTM1	(0x00000400)
#define VSP_WPF_OUTFMT_WRTM0	(0x00000200)
#define VSP_WPF_OUTFMT_CSC		(0x00000100)
#define VSP_WPF_OUTFMT_WRFMT	(0x0000007F)

#define VSP_WPF_OUTFMT_MSK \
	(VSP_WPF_OUTFMT_SPYCS| \
	 VSP_WPF_OUTFMT_SPUVS| \
	 VSP_WPF_OUTFMT_WRFMT)


#define VSP_DPR_ROUTE_SRU		(0x00000010)
#define VSP_DPR_ROUTE_UDS0		(0x00000011)
#define VSP_DPR_ROUTE_UDS1		(0x00000012)
#define VSP_DPR_ROUTE_UDS2		(0x00000013)
#define VSP_DPR_ROUTE_LUT		(0x00000016)
#define VSP_DPR_ROUTE_CLU		(0x0000001D)
#define VSP_DPR_ROUTE_HST		(0x0000001E)
#define VSP_DPR_ROUTE_HSI		(0x0000001F)
#define VSP_DPR_ROUTE_BRU0		(0x00000017)
#define VSP_DPR_ROUTE_BRU1		(0x00000018)
#define VSP_DPR_ROUTE_BRU2		(0x00000019)
#define VSP_DPR_ROUTE_BRU3		(0x0000001A)
#define VSP_DPR_ROUTE_WPF0		(0x00000038)
#define VSP_DPR_ROUTE_WPF1		(0x00000039)
#define VSP_DPR_ROUTE_WPF2		(0x0000003A)
#define VSP_DPR_ROUTE_WPF3		(0x0000003B)
#define VSP_DPR_ROUTE_NOT_USE	(0x0000003F)

#define VSP_DPR_SMPPT_RPF0		(0x00000000)
#define VSP_DPR_SMPPT_RPF1		(0x00000001)
#define VSP_DPR_SMPPT_RPF2		(0x00000002)
#define VSP_DPR_SMPPT_RPF3		(0x00000003)
#define VSP_DPR_SMPPT_RPF4		(0x00000004)
#define VSP_DPR_SMPPT_SRU		(0x00000010)
#define VSP_DPR_SMPPT_UDS0		(0x00000011)
#define VSP_DPR_SMPPT_UDS1		(0x00000012)
#define VSP_DPR_SMPPT_UDS2		(0x00000013)
#define VSP_DPR_SMPPT_LUT		(0x00000016)
#define VSP_DPR_SMPPT_BRU		(0x0000001B)
#define VSP_DPR_SMPPT_CLU		(0x0000001D)
#define VSP_DPR_SMPPT_HST		(0x0000001E)
#define VSP_DPR_SMPPT_HSI		(0x0000001F)
#define VSP_DPR_SMPPT_LIF		(0x00000037)
#define VSP_DPR_SMPPT_NOT_USE	(0x0000073F)

#define VSP_SRU_CTRL_EN			(0x00000001)

#define VSP_UDS_CTRL_AMD		(0x40000000)
#define VSP_UDS_CTRL_FMD		(0x20000000)
#define VSP_UDS_CTRL_BLADV		(0x10000000)
#define VSP_UDS_CTRL_AON		(0x02000000)
#define VSP_UDS_CTRL_ATHON		(0x01000000)
#define VSP_UDS_CTRL_BC			(0x00100000)	/* multi tap */
#define VSP_UDS_CTRL_NN			(0x000F0000)	/* nearest neighbor */

#define VSP_UDS_SCALE_1_4		(0x4000)	/* quarter */
#define VSP_UDS_SCALE_1_2		(0x2000)	/* half */
#define VSP_UDS_SCALE_1_1		(0x1000)
#define VSP_UDS_SCALE_2_1		(0x0800)	/* double */
#define VSP_UDS_SCALE_4_1		(0x0400)	/* quadruple */
#define VSP_UDS_SCALE_16_1		(0x0100)	/* maximum scale */
#define VSP_UDS_SCALE_MANT		(0xF000)
#define VSP_UDS_SCALE_FRAC		(0x0FFF)

#define VSP_LUT_CTRL_EN			(0x00000001)

#define VSP_CLU_CTRL_3D			(0x00000000)
#define VSP_CLU_CTRL_2D			(0x0000D372)
#define VSP_CLU_CTRL_AAI		(0x10000000)
#define VSP_CLU_CTRL_MVS		(0x01000000)
#define VSP_CLU_CTRL_EN			(0x00000001)

#define VSP_HST_CTRL_EN			(0x00000001)

#define VSP_HSI_CTRL_EN			(0x00000001)

#define VSP_HGO_REGRST_SET		(0x00000001)

#define VSP_HGT_REGRST_SET		(0x00000001)

enum {
	VSP_WPF_REG_CMD = 0,
	VSP_WPF_REG_IRQ_ENB,
	VSP_WPF_REG_IRQ_STA,
	VSP_WPF_REG_CTRL,
	VSP_WPF_REG_FPORCH,
	VSP_WPF_REG_MAX
};

enum {
	VSP_REG_CTRL = 0,
	VSP_REG_CLUT,
	VSP_REG_ROUTE,
	VSP_REG_MAX
};

enum {
	VSP_BROP_DST_A = 0,
	VSP_BROP_SRC_A,
	VSP_BROP_DST_R,
	VSP_BROP_SRC_C,
	VSP_BROP_SRC_D,
	VSP_BROP_MAX
};

/* RPF information structure */
struct vsp_rpf_info {
	unsigned long val_esize;
	unsigned long val_infmt;
	unsigned long val_dswap;
	unsigned long val_loc;
	unsigned long val_alpha_sel;
	unsigned long val_vrtcol;
	unsigned long val_mskctrl;
	unsigned long val_mskset[2];
	unsigned long val_ckey_ctrl;
	unsigned long val_ckey_set[2];
	unsigned long val_astride;
	unsigned long val_addr_y;
	unsigned long val_addr_c0;
	unsigned long val_addr_c1;
	unsigned long val_addr_ai;

	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_clut;
	void __iomem *reg_route;
};

/* SRU information structure */
struct vsp_sru_info {
	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
};

/* UDS information structure */
struct vsp_uds_info {
	unsigned long val_ctrl;
	unsigned long val_pass;
	unsigned long val_clip;

	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
};

/* LUT information structure */
struct vsp_lut_info {
	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
	void __iomem *reg_table;
};

/* CLU information structure */
struct vsp_clu_info {
	unsigned long val_ctrl;

	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
	void __iomem *reg_addr;
	void __iomem *reg_data;
};

/* HST information structure */
struct vsp_hst_info {
	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
};

/* HSI information structure */
struct vsp_hsi_info {
	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
};

/* BRU information structure */
struct vsp_bru_info {
	unsigned long val_inctrl;

	unsigned long val_vir_loc;
	unsigned long val_vir_color;
	unsigned long val_vir_size;

	unsigned long val_ctrl[4];
	unsigned long val_bld[4];
	unsigned long val_rop;

	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_route;
};

/* HGO information structure */
struct vsp_hgo_info {
	unsigned long val_addr;
	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_smppt;

	void __iomem *reg_hist[3];
};

/* HGT information structure */
struct vsp_hgt_info {
	unsigned long val_addr;
	unsigned long val_dpr;

	void __iomem *reg_ctrl;
	void __iomem *reg_smppt;

	void __iomem *reg_hist;
};

struct vsp_src_info {
	unsigned char rpf_ch;
	unsigned char color;
	unsigned long width;
	unsigned long height;
	unsigned long x_position;
	unsigned long y_position;
	unsigned long master;
};

/* channel information structure */
struct vsp_ch_info {
	unsigned char status;
	unsigned char wpf_ch;

	void (*cb_func)(T_VSP_CB *);	/* callback function */
	void *cb_userdata;				/* callback user data */

	unsigned long reserved_rpf;
	unsigned long reserved_module;

	unsigned long next_module;

	struct vsp_src_info src_info[VSP_SRC_MAX + 1];
	unsigned char src_idx;
	unsigned char src_cnt;

	unsigned char wpf_cnt;
	unsigned char bru_cnt;
	unsigned char use_uds_flag;

	unsigned long val_srcrpf;		/* WPF_SRCRPF */
	unsigned long val_outfmt;		/* WPF_OUTFMT */
	unsigned long val_addr_y;		/* WPF_DSTM_ADDR_Y */
	unsigned long val_addr_c0;		/* WPF_DSTM_ADDR_C0 */
	unsigned long val_addr_c1;		/* WPF_DSTM_ADDR_C1 */

	void __iomem *reg_cmd;
	void __iomem *reg_irq_enb;
	void __iomem *reg_irq_sta;
	void __iomem *reg_ctrl;
	void __iomem *reg_fporch;
};


/* private data structure */
typedef struct VSP_PRIVATE_DATA_T {
	void __iomem *base_reg;
	int irq;

	unsigned long use_rpf;
	unsigned long use_module;

	struct vsp_ch_info ch_info[VSP_WPF_MAX];

	struct vsp_rpf_info rpf_info[VSP_RPF_MAX];
	struct vsp_sru_info sru_info[VSP_SRU_MAX];
	struct vsp_uds_info uds_info[VSP_UDS_MAX];
	struct vsp_lut_info lut_info[VSP_LUT_MAX];
	struct vsp_clu_info clu_info[VSP_CLU_MAX];
	struct vsp_hst_info hst_info[VSP_HST_MAX];
	struct vsp_hsi_info hsi_info[VSP_HSI_MAX];
	struct vsp_bru_info bru_info[VSP_BRU_MAX];
	struct vsp_hgo_info hgo_info[VSP_HGO_MAX];
	struct vsp_hgt_info hgt_info[VSP_HGT_MAX];
} VSP_PRIVATE_DATA;


/* define local function */
long vsp_ins_check_init_parameter(T_VSP_INIT *param);
long vsp_ins_check_start_parameter(
	VSP_PRIVATE_DATA * prv,
	unsigned char vsp,
	unsigned char ch,
	T_VSP_START *param);

long vsp_ins_set_start_parameter(
	VSP_PRIVATE_DATA * prv, unsigned char ch, T_VSP_START *param);
void vsp_ins_start_processing(struct vsp_ch_info *ch_info);
long vsp_ins_stop_processing(VSP_PRIVATE_DATA *prv, unsigned char ch);

long vsp_ins_allocate_memory(VSP_PRIVATE_DATA **prv);
long vsp_ins_free_memory(VSP_PRIVATE_DATA *prv);

long vsp_ins_init_reg(
	struct platform_device *pdev,
	VSP_PRIVATE_DATA *prv,
	unsigned char vsp_ch);
long vsp_ins_quit_reg(VSP_PRIVATE_DATA *prv);

void vsp_ins_cb_function(VSP_PRIVATE_DATA *prv, unsigned char ch, long ercd);

long vsp_ins_reg_ih(
	struct platform_device *pdev,
	VSP_PRIVATE_DATA *prv,
	unsigned char vsp_ch);
long vsp_ins_unreg_ih(VSP_PRIVATE_DATA *prv);

long vsp_ins_get_vsp_ip_num(
	unsigned char *vsp, unsigned char *wpf, unsigned char ch);

#endif
