/*************************************************************************/ /*
 FDPM

 Copyright (C) 2013 Renesas Electronics Corporation

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

#ifndef _FDP_DRV_HARDW_H
#define _FDP_DRV_HARDW_H

/*-------------------- register define -------------------------------------*/
#define              FD1_CTL_CMD	(0x0000)
#define              FD1_CTL_SGCMD	(0x0004)
#define              FD1_CTL_REGEND	(0x0008)
#define              FD1_CTL_CHACT	(0x000C)
#define              FD1_CTL_OPMODE	(0x0010)
#define              FD1_CTL_VPERIOD	(0x0014)
#define              CLK_CTRL	(0x0018)

#define              FD1_CTL_STATUS	(0x0024)
#define              FD1_CTL_VCYCLE_STAT	(0x0028)

#define              FD1_CTL_IRQENB	(0x0038)
#define              FD1_CTL_IRQSTA	(0x003C)
#define              FD1_CTL_IRQFSET	(0x0040)

#define              FD1_RPF_SIZE	(0x0060)
#define              FD1_RPF_FORMAT	(0x0064)
#define              FD1_RPF_PSTRIDE	(0x0068)
#define              FD1_RPF0_ADDR_Y	(0x006C)
#define              FD1_RPF0_ADDR_C0	(0x0070)
#define              FD1_RPF0_ADDR_C1	(0x0074)
#define              FD1_RPF1_ADDR_Y	(0x0078)
#define              FD1_RPF1_ADDR_C0	(0x007C)
#define              FD1_RPF1_ADDR_C1	(0x0080)
#define              FD1_RPF2_ADDR_Y	(0x0084)
#define              FD1_RPF2_ADDR_C0	(0x0088)
#define              FD1_RPF2_ADDR_C1	(0x008C)
#define              FD1_RPF_SMSK_ADDR	(0x0090)
#define              FD1_RPF_SWAP	(0x0094)

#define              FD1_WPF_FORMAT	(0x00C0)
#define              FD1_WPF_RNDCTL	(0x00C4)
#define              FD1_WPF_PSTRIDE	(0x00C8)
#define              FD1_WPF_ADDR_Y	(0x00CC)
#define              FD1_WPF_ADDR_C0	(0x00D0)
#define              FD1_WPF_ADDR_C1	(0x00D4)
#define              FD1_WPF_SWAP	(0x00D8)

#define              FD1_IPC_MODE	(0x0100)
#define              FD1_IPC_SMSK_THRESH	(0x0104)
#define              FD1_IPC_COMB_DET	(0x0108)
#define              FD1_IPC_MOTDEC	(0x010C)

#define              FD1_IPC_DLI_BLEND	(0x0120)
#define              FD1_IPC_DLI_HGAIN	(0x0124)
#define              FD1_IPC_DLI_SPRS	(0x0128)
#define              FD1_IPC_DLI_ANGLE	(0x012C)
#define              FD1_IPC_DLI_ISOPIX0	(0x0130)
#define              FD1_IPC_DLI_ISOPIX1	(0x0134)

#define              SENSOR_TH0	(0x0140)
#define              SENSOR_TH1	(0x0144)
#define              SNSOR_SAD	(0x0148)
#define              SNSOR_DIF	(0x014C)
#define              SNSOR_HFQ	(0x0150)
#define              SNSOR_VFQ	(0x0154)
#define              SNSOR_FQ	(0x0158)
#define              SNSOR_SM	(0x015C)
#define              SNSOR_CMB	(0x0160)

#define              FD1_SNSOR_CTL0	(0x0170)
#define              FD1_SNSOR_CTL1	(0x0174)
#define              FD1_SNSOR_CTL2	(0x0178)
#define              FD1_SNSOR_CTL3	(0x017C)
#define              FD1_SNSOR_0	(0x0180)
#define              FD1_SNSOR_1	(0x0184)
#define              FD1_SNSOR_2	(0x0188)
#define              FD1_SNSOR_3	(0x018C)
#define              FD1_SNSOR_4	(0x0190)
#define              FD1_SNSOR_5	(0x0194)
#define              FD1_SNSOR_6	(0x0198)
#define              FD1_SNSOR_7	(0x019C)
#define              FD1_SNSOR_8	(0x01A0)
#define              FD1_SNSOR_9	(0x01A4)
#define              FD1_SNSOR_10	(0x01A8)
#define              FD1_SNSOR_11	(0x01AC)
#define              FD1_SNSOR_12	(0x01B0)
#define              FD1_SNSOR_13	(0x01B4)
#define              FD1_SNSOR_14	(0x01B8)
#define              FD1_SNSOR_15	(0x01BC)
#define              FD1_SNSOR_16	(0x01C0)
#define              FD1_SNSOR_17	(0x01C4)

#define              FD1_IPC_LMEM	(0x01E0)

#define              DBG_INFO_TOP0	(0x0700)
#define              DBG_INFO_TOP1	(0x0704)
#define              DBG_INFO_CTL	(0x0708)
#define              DBG_INFO_RREQ	(0x070C)
#define              DBG_INFO_RMIF0	(0x0710)
#define              DBG_INFO_RMIF1	(0x0714)
#define              DBG_INFO_WREQ0	(0x0718)
#define              DBG_INFO_WREQ1	(0x071C)
#define              DBG_INFO_WMIF0	(0x0720)
#define              DBG_INFO_WMIF1	(0x0724)
#define              DBG_INFO_ABIR	(0x0728)
#define              DBG_INFO_ABIW	(0x072C)

#define              IP_VERSION	(0x0800)

typedef struct st_fdp_lut {
	unsigned long              dif_adj[256];
	unsigned long              sad_adj[256];
	unsigned long              bld_gain[256];
	unsigned long              dif_gain[256];
	unsigned long              mdet[256];
} DEF_FDP_LUT;

/*	FDP	register select */
enum {
	FDP_DIF_ADJ = 0,              /* R/W 0x1000  32(8) */
	FDP_SAD_ADJ,                  /* R/W 0x1400  32(8) */
	FDP_BLD_GAIN,                 /* R/W 0x1800  32(8) */
	FDP_DIF_GAIN,                 /* R/W 0x1C00  32(8) */
	FDP_MDET,                     /* R/W 0x2000  32(8) */
	FDP_LUT_MAX
};

void fdp_reg_rwrite(unsigned long wdata, unsigned long mask, void *addr);
void fdp_reg_write(unsigned long wdata, void *addr);
unsigned long fdp_reg_read(void *addr);

#define P_FDP  (FDP_obj->mapbase)
#define P_FDP_LUT ((FDP_obj->mapbase + 0x1000))

#define FEND_STATUS_BIT (0x00000001)
#define FD1_RPF_SWAP_ISWAP_BIT (0x0000000f)
#define FD1_WPF_SWAP_SSWAP_OSWAP_BIT (0x000000ff)
#define FD1_CTL_IRQENB_FRE_BIT (0x00000001)
#define FD1_CTL_OPMODE_PRG_BIT (0x00000010)
#define FD1_IPC_MODE_DLI_BIT (0x00000100)
#define FD1_IPC_SMSK_THRESH_FSM0_BIT (0x00010000)
#define FD1_IPC_LMEM_PNUM_BIT (1024)

#define FD1_CTL_CMD_STRCMD_MSK (0x00000001)
#define FD1_CTL_OPMODE_VIMD_MSK (0x00000003)
#define CLK_CTRL_
#define FD1_IPC_SMSK_THRESH_SMSK_TH_MSK (0x000000ff)

#endif
