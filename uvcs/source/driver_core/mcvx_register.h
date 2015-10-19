/*************************************************************************/ /*
 VCP driver

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

#ifndef	MCVX_REGISTER_H
#define	MCVX_REGISTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MCVX_REG_CE_OFFSET			( 0x00000200UL )

/* VCID */
#define MCVX_VCID					( 0x000055F4UL )	/* "VPU" */

/* vlc_cmd */							
#define	MCVX_CMD_VLC_START			( 0x00000001UL )
#define	MCVX_CMD_VLC_RESET_SOFT		( 0x00003600UL )	/* reserved */
#define	MCVX_CMD_VLC_RESET_MODULE	( 0x00003b00UL )

/* ce_cmd */							
#define	MCVX_CMD_CE_START			( 0x00000001UL )
#define MCVX_CMD_FCV_START			( 0x00000010UL )
#define	MCVX_CMD_CE_RESET_SOFT		( 0x00003600UL )	/* reserved */
#define	MCVX_CMD_CE_RESET_MODULE	( 0x00003b00UL )

/* register size */
#define MCVX_REG_TABLE_SIZE			( 16UL )

#define MCVX_REGS_SINGLE			( 1UL )
#define MCVX_REGS_RES_CE			( 14UL )
#define MCVX_REGS_RES_VLC			( 8UL )

/* VLC-module irq flag */
#define	MCVX_IRQ_VLC_END			( 0x00000001UL )
#define	MCVX_IRQ_VLC_EDT			( 0x00000002UL )
#define	MCVX_IRQ_VLC_ALL			( MCVX_IRQ_VLC_END | MCVX_IRQ_VLC_EDT )
#define	MCVX_IRQ_VLC_OFF			( 0x00000000UL )

/* CE-module irq flag (including FCV) */							
#define	MCVX_IRQ_CE_END				( 0x00000001UL )
#define	MCVX_IRQ_CE_EDT				( 0x00000002UL )
#define	MCVX_IRQ_FCV_END			( 0x00000100UL )
#define	MCVX_IRQ_CE_ALL				( MCVX_IRQ_CE_END | MCVX_IRQ_CE_EDT | MCVX_IRQ_FCV_END )

/* irq enable flags */
#define MCVX_VEDE					( 0x00000002UL )	/* VLC system error detection interrupt		*/
#define MCVX_VLE					( 0x00000001UL )	/* VLC end interrupt						*/
#define MCVX_CEDE					( 0x00000002UL )	/* CE system error detection interrupt		*/
#define MCVX_CEE					( 0x00000001UL )	/* CE end interrupt							*/
#define MCVX_FCVE					( 0x00000100UL )	/* end interrupt of FCV process				*/

/* hw event */
#define MCVX_NO_EVENT				( 0x00000000UL )
#define MCVX_VLC_PROC_END_EVENT		( MCVX_VLE  )
#define MCVX_VLC_HUNG_END_EVENT		( MCVX_VEDE )
#define MCVX_FCV_PROC_END_EVENT		( MCVX_FCVE )
#define MCVX_CE_PROC_END_EVENT		( MCVX_CEE  )
#define MCVX_CE_HUNG_END_EVENT		( MCVX_CEDE )

/* endian (128bit width) */								
#define	MCVX_ENDIAN_0000			( 0x00UL )	/* BIG */
#define	MCVX_ENDIAN_0001			( 0x01UL )
#define	MCVX_ENDIAN_0010			( 0x02UL )
#define	MCVX_ENDIAN_0011			( 0x03UL )
#define	MCVX_ENDIAN_0100			( 0x04UL )
#define	MCVX_ENDIAN_0101			( 0x05UL )
#define	MCVX_ENDIAN_0110			( 0x06UL )
#define	MCVX_ENDIAN_0111			( 0x07UL )
#define	MCVX_ENDIAN_1000			( 0x08UL )
#define	MCVX_ENDIAN_1001			( 0x09UL )
#define	MCVX_ENDIAN_1010			( 0x0aUL )
#define	MCVX_ENDIAN_1011			( 0x0bUL )
#define	MCVX_ENDIAN_1100			( 0x0cUL )
#define	MCVX_ENDIAN_1101			( 0x0dUL )
#define	MCVX_ENDIAN_1110			( 0x0eUL )
#define	MCVX_ENDIAN_1111			( 0x0fUL )	/* LIT */

/* mask_bits */
#define MCVX_M_00BIT 				( 0x00000000UL )
#define MCVX_M_01BIT 				( 0x00000001UL )
#define MCVX_M_02BIT 				( 0x00000003UL )
#define MCVX_M_03BIT 				( 0x00000007UL )
#define MCVX_M_04BIT 				( 0x0000000fUL )
#define MCVX_M_05BIT 				( 0x0000001fUL )
#define MCVX_M_06BIT 				( 0x0000003fUL )
#define MCVX_M_07BIT 				( 0x0000007fUL )
#define MCVX_M_08BIT 				( 0x000000ffUL )
#define MCVX_M_09BIT				( 0x000001ffUL )
#define MCVX_M_10BIT				( 0x000003ffUL )
#define MCVX_M_11BIT				( 0x000007ffUL )
#define MCVX_M_12BIT			 	( 0x00000fffUL )
#define MCVX_M_13BIT				( 0x00001fffUL )
#define MCVX_M_14BIT				( 0x00003fffUL )
#define MCVX_M_15BIT				( 0x00007fffUL )
#define MCVX_M_16BIT				( 0x0000ffffUL )
#define MCVX_M_17BIT				( 0x0001ffffUL )
#define MCVX_M_18BIT				( 0x0003ffffUL )
#define MCVX_M_19BIT				( 0x0007ffffUL )
#define MCVX_M_20BIT				( 0x000fffffUL )
#define MCVX_M_21BIT				( 0x001fffffUL )
#define MCVX_M_22BIT				( 0x003fffffUL )
#define MCVX_M_23BIT				( 0x007fffffUL )
#define MCVX_M_24BIT				( 0x00ffffffUL )
#define MCVX_M_25BIT				( 0x01ffffffUL )
#define MCVX_M_26BIT				( 0x03ffffffUL )
#define MCVX_M_27BIT				( 0x07ffffffUL )
#define MCVX_M_28BIT				( 0x0fffffffUL )
#define MCVX_M_29BIT				( 0x1fffffffUL )
#define MCVX_M_30BIT				( 0x3fffffffUL )
#define MCVX_M_31BIT				( 0x7fffffffUL )
#define MCVX_M_32BIT 				( 0xffffffffUL )

/* VCP VLC register */
typedef struct{							/*	OFFSET	*/
	MCVX_REG	VP_VLC_VCRH;			/*	0000	*/
	MCVX_REG	VP_VLC_VCRL;			/*	0004	*/
	MCVX_REG	VP_VLC_CMD;				/*	0008	*/
	MCVX_REG	VP_VLC_CTRL;			/*	000C	*/
	MCVX_REG	VP_VLC_IRQ_ENB;			/*	0010	*/
	MCVX_REG	VP_VLC_IRQ_STA;			/*	0014	*/
	MCVX_REG	RESERVE_0018;			/*	0018	*/
	MCVX_REG	VP_VLC_CLK_STOP;		/*	001C	*/
	MCVX_REG	VP_VLC_STATUS;			/*	0020	*/
	MCVX_REG	RESERVE_0024;			/*	0024	*/
	MCVX_REG	VP_VLC_IND_WDHH;		/*	0028	*/
	MCVX_REG	VP_VLC_IND_WDHL;		/*	002C	*/
	MCVX_REG	VP_VLC_IND_WDLH;		/*	0030	*/
	MCVX_REG	VP_VLC_IND_WDLL;		/*	0034	*/
	MCVX_REG	VP_VLC_IND_WA;			/*	0038	*/
	MCVX_REG	VP_VLC_IND_RA;			/*	003C	*/
	MCVX_REG	VP_VLC_IND_RDHH;		/*	0040	*/
	MCVX_REG	VP_VLC_IND_RDHL;		/*	0044	*/
	MCVX_REG	VP_VLC_IND_RDLH;		/*	0048	*/
	MCVX_REG	VP_VLC_IND_RDLL;		/*	004C	*/
	MCVX_REG	RESERVE_0050;			/*	0050	*/
	MCVX_REG	VP_VLC_DMON;			/*	0054	*/
	MCVX_REG	RESERVE_0058;			/*	0058	*/
	MCVX_REG	RESERVE_005C;			/*	005C	*/
	MCVX_REG	VP_VLC_LIST_INIT;		/*	0060	*/
	MCVX_REG	VP_VLC_LIST_EN;			/*	0064	*/
	MCVX_REG	VP_VLC_LIST_LDEN;		/*	0068	*/
	MCVX_REG	RESERVE_006C;			/*	006C	*/
	MCVX_REG	RESERVE_0070;			/*	0070	*/
	MCVX_REG	RESERVE_0074;			/*	0074	*/
	MCVX_REG	RESERVE_0078;			/*	0078	*/
	MCVX_REG	RESERVE_007C;			/*	007C	*/
	MCVX_REG	RESERVE_0080;			/*	0080	*/
	MCVX_REG	VP_VLC_PBAH;			/*	0084	*/
	MCVX_REG	VP_VLC_PBAL;			/*	0088	*/
	MCVX_REG	VP_VLC_EDT;				/*	008C	*/
	MCVX_REG	RESERVE_0090;			/*	0090	*/
	MCVX_REG	RESERVE_0094;			/*	0094	*/
	MCVX_REG	RESERVE_0098;			/*	0098	*/
	MCVX_REG	RESERVE_009C;			/*	009C	*/
	MCVX_REG	VP_VLC_MB0H_RD;			/*	00A0	*/
	MCVX_REG	VP_VLC_MB0L_RD;			/*	00A4	*/
	MCVX_REG	RESERVE_00A8;			/*	00A8	*/
	MCVX_REG	RESERVE_00AC;			/*	00AC	*/
	MCVX_REG	VP_VLC_CNT;				/*	00B0	*/
	MCVX_REG	RESERVE_00B4;			/*	00B4	*/
	MCVX_REG	RESERVE_00B8;			/*	00B8	*/
	MCVX_REG	RESERVE_00BC;			/*	00BC	*/
	MCVX_REG	RESERVE_00C0;			/*	00C0	*/
	MCVX_REG	VP_VLC_TB;				/*	00C4	*/
	MCVX_REG	VP_VLC_ERR0H;			/*	00C8	*/
	MCVX_REG	VP_VLC_ERR0L;			/*	00CC	*/
	MCVX_REG	VP_VLC_TB_IS0;			/*	00D0	*/
	MCVX_REG	RESERVE_00D4;			/*	00D4	*/
	MCVX_REG	VP_VLC_CODEC_INFO;		/*	00D8	*/
	MCVX_REG	VP_VLC_ERR1H;			/*	00DC	*/
	MCVX_REG	VP_VLC_ERR1L;			/*	00E0	*/
	MCVX_REG	RESERVE_00E4;			/*	00E4	*/
	MCVX_REG	RESERVE_00E8;			/*	00E8	*/
	MCVX_REG	RESERVE_00EC;			/*	00EC	*/
	MCVX_REG	RESERVE_00F0;			/*	00F0	*/
	MCVX_REG	RESERVE_00F4;			/*	00F4	*/
	MCVX_REG	RESERVE_00F8;			/*	00F8	*/
	MCVX_REG	RESERVE_00FC;			/*	00FC	*/
	MCVX_REG	VP_VLC_ACC_CTRL;		/*	0100	*/
	MCVX_REG	VP_VLC_ACC_DIEN;		/*	0104	*/
	MCVX_REG	RESERVE_0108;			/*	0108	*/
	MCVX_REG	VP_VLC_ACC_STA;			/*	010C	*/
	MCVX_REG	VP_VLC_ACC_BDCTX;		/*	0110	*/
	MCVX_REG	VP_VLC_ACC_PROB;		/*	0114	*/
	MCVX_REG	VP_VLC_ACC_SC_MODE0;	/*	0118	*/
	MCVX_REG	VP_VLC_ACC_SC_MODE1;	/*	011C	*/
	MCVX_REG	VP_VLC_ACC_SC_MODE2;	/*	0120	*/
	MCVX_REG	VP_VLC_ACC_SC_LIMIT;	/*	0124	*/
	MCVX_REG	VP_VLC_ACC_BYTEPOS;		/*	0128	*/
	MCVX_REG	RESERVE_012C;			/*	012C	*/
	MCVX_REG	VP_VLC_ACC_READ;		/*	0130	*/
	MCVX_REG	VP_VLC_ACC_SFT;			/*	0134	*/
	MCVX_REG	VP_VLC_ACC_GET1;		/*	0138	*/
	MCVX_REG	VP_VLC_ACC_GET2;		/*	013C	*/
	MCVX_REG	VP_VLC_ACC_GET3;		/*	0140	*/
	MCVX_REG	VP_VLC_ACC_GET4;		/*	0144	*/
	MCVX_REG	VP_VLC_ACC_GET5;		/*	0148	*/
	MCVX_REG	VP_VLC_ACC_GET6;		/*	014C	*/
	MCVX_REG	VP_VLC_ACC_GET7;		/*	0150	*/
	MCVX_REG	VP_VLC_ACC_GET8;		/*	0154	*/
	MCVX_REG	VP_VLC_ACC_GET9;		/*	0158	*/
	MCVX_REG	VP_VLC_ACC_GET10;		/*	015C	*/
	MCVX_REG	VP_VLC_ACC_GET11;		/*	0160	*/
	MCVX_REG	VP_VLC_ACC_GET12;		/*	0164	*/
	MCVX_REG	VP_VLC_ACC_GET13;		/*	0168	*/
	MCVX_REG	VP_VLC_ACC_GET14;		/*	016C	*/
	MCVX_REG	VP_VLC_ACC_GET15;		/*	0170	*/
	MCVX_REG	VP_VLC_ACC_GET16;		/*	0174	*/
	MCVX_REG	VP_VLC_ACC_GET32;		/*	0178	*/
	MCVX_REG	VP_VLC_ACC_UGLM;		/*	017C	*/
	MCVX_REG	VP_VLC_ACC_SGLM;		/*	0180	*/
	MCVX_REG	VP_VLC_ACC_BP;			/*	0184	*/
	MCVX_REG	VP_VLC_ACC_B1;			/*	0188	*/
	MCVX_REG	VP_VLC_ACC_B2;			/*	018C	*/
	MCVX_REG	VP_VLC_ACC_B3;			/*	0190	*/
	MCVX_REG	VP_VLC_ACC_B4;			/*	0194	*/
	MCVX_REG	VP_VLC_ACC_B5;			/*	0198	*/
	MCVX_REG	VP_VLC_ACC_B6;			/*	019C	*/
	MCVX_REG	VP_VLC_ACC_B7;			/*	01A0	*/
	MCVX_REG	VP_VLC_ACC_B8;			/*	01A4	*/
	MCVX_REG	VP_VLC_ACC_T9;			/*	01A8	*/
	MCVX_REG	VP_VLC_ACC_BP_PARAM0;	/*	01AC	*/
	MCVX_REG	VP_VLC_ACC_BP_PARAM1;	/*	01B0	*/
	MCVX_REG	RESERVE_01B4;			/*	01B4	*/
	MCVX_REG	RESERVE_01B8;			/*	01B8	*/
	MCVX_REG	RESERVE_01BC;			/*	01BC	*/
	MCVX_REG	VP_VLC_ACC_DCHR0;		/*	01C0	*/
	MCVX_REG	VP_VLC_ACC_DCHR1;		/*	01C4	*/
	MCVX_REG	VP_VLC_ACC_DCHR2;		/*	01C8	*/
	MCVX_REG	VP_VLC_ACC_BA0;			/*	01CC	*/
	MCVX_REG	VP_VLC_ACC_BA1;			/*	01D0	*/
	MCVX_REG	VP_VLC_ACC_BA2;			/*	01D4	*/
	MCVX_REG	VP_VLC_ACC_BA3;			/*	01D8	*/
	MCVX_REG	VP_VLC_ACC_BA4;			/*	01DC	*/
	MCVX_REG	VP_VLC_ACC_LITEM_0H;	/*	01E0	*/
	MCVX_REG	VP_VLC_ACC_LITEM_0L;	/*	01E4	*/
	MCVX_REG	VP_VLC_ACC_LITEM_1H;	/*	01E8	*/
	MCVX_REG	VP_VLC_ACC_LITEM_1L;	/*	01EC	*/
	MCVX_REG	VP_VLC_ACC_LITEM_2H;	/*	01F0	*/
	MCVX_REG	VP_VLC_ACC_LITEM_2L;	/*	01F4	*/
	MCVX_REG	VP_VLC_ACC_LITEM_3H;	/*	01F8	*/
	MCVX_REG	VP_VLC_ACC_LITEM_3L;	/*	01FC	*/
} MCVX_REG_VLC_T;

/* VCP CE register */
typedef struct{							/*	OFFSET	*/
	MCVX_REG	VP_CE_VCRH;				/*	0200	*/
	MCVX_REG	VP_CE_VCRL;				/*	0204	*/
	MCVX_REG	VP_CE_CMD;				/*	0208	*/
	MCVX_REG	VP_CE_CTRL;				/*	020C	*/
	MCVX_REG	VP_CE_IRQ_ENB;			/*	0210	*/
	MCVX_REG	VP_CE_IRQ_STA;			/*	0214	*/
	MCVX_REG	RESERVE_0218;			/*	0218	*/
	MCVX_REG	VP_CE_CLK_STOP;			/*	021C	*/
	MCVX_REG	VP_CE_STATUS;			/*	0220	*/
	MCVX_REG	VP_CE_DSTATUS;			/*	0224	*/
	MCVX_REG	VP_CE_IND_WDHH;			/*	0228	*/
	MCVX_REG	VP_CE_IND_WDHL;			/*	022C	*/
	MCVX_REG	VP_CE_IND_WDLH;			/*	0230	*/
	MCVX_REG	VP_CE_IND_WDLL;			/*	0234	*/
	MCVX_REG	VP_CE_IND_WA;			/*	0238	*/
	MCVX_REG	VP_CE_IND_RA;			/*	023C	*/
	MCVX_REG	VP_CE_IND_RDHH;			/*	0240	*/
	MCVX_REG	VP_CE_IND_RDHL;			/*	0244	*/
	MCVX_REG	VP_CE_IND_RDLH;			/*	0248	*/
	MCVX_REG	VP_CE_IND_RDLL;			/*	024C	*/
	MCVX_REG	VP_CE_DEBUG;			/*	0250	*/
	MCVX_REG	VP_CE_DMON;				/*	0254	*/
	MCVX_REG	RESERVE_0258;			/*	0258	*/
	MCVX_REG	RESERVE_025C;			/*	025C	*/
	MCVX_REG	VP_CE_MV_NUM_FW;		/*	0260	*/
	MCVX_REG	VP_CE_MV_NUM_BW;		/*	0264	*/
	MCVX_REG	VP_CE_MVH_SUM_FW;		/*	0268	*/
	MCVX_REG	VP_CE_MVV_SUM_FW;		/*	026C	*/
	MCVX_REG	VP_CE_MVH_SUM_BW;		/*	0270	*/
	MCVX_REG	VP_CE_MVV_SUM_BW;		/*	0274	*/
	MCVX_REG	VP_CE_REF_LOG;			/*	0278	*/
	MCVX_REG	VP_CE_PBAH;				/*	027C	*/
	MCVX_REG	VP_CE_PBAL;				/*	0280	*/
	MCVX_REG	RESERVE_0284;			/*	0284	*/
	MCVX_REG	RESERVE_0288;			/*	0288	*/
	MCVX_REG	VP_CE_EDT;				/*	028C	*/
	MCVX_REG	VP_CE_PAR0H_RD;			/*	0290	*/
	MCVX_REG	VP_CE_PAR0L_RD;			/*	0294	*/
	MCVX_REG	VP_CE_PAR1H_RD;			/*	0298	*/
	MCVX_REG	VP_CE_PAR1L_RD;			/*	029C	*/
	MCVX_REG	VP_CE_MB0H_RD;			/*	02A0	*/
	MCVX_REG	VP_CE_MB0L_RD;			/*	02A4	*/
	MCVX_REG	RESERVE_02A8;			/*	02A8	*/
	MCVX_REG	RESERVE_02AC;			/*	02AC	*/
	MCVX_REG	VP_CE_CNT;				/*	02B0	*/
	MCVX_REG	RESERVE_02B4;			/*	02B4	*/
	MCVX_REG	VP_CE_PAR2H_RD;			/*	02B8	*/
	MCVX_REG	VP_CE_PAR2L_RD;			/*	02BC	*/
	MCVX_REG	VP_CE_ERRH;				/*	02C0	*/
	MCVX_REG	VP_CE_ERRL;				/*	02C4	*/
	MCVX_REG	VP_CE_TB_IS0;			/*	02C8	*/
	MCVX_REG	RESERVE_02CC;			/*	02CC	*/
	MCVX_REG	VP_CE_INTRA_NUM;		/*	02D0	*/
	MCVX_REG	RESERVE_02D4;			/*	02D4	*/
	MCVX_REG	VP_CE_I16X16_NUM;		/*	02D8	*/
	MCVX_REG	VP_CE_INXN_NUM;			/*	02DC	*/
	MCVX_REG	RESERVE_02E0;			/*	02E0	*/
	MCVX_REG	VP_CE_16X16_NUM;		/*	02E4	*/
	MCVX_REG	VP_CE_16X8_NUM;			/*	02E8	*/
	MCVX_REG	VP_CE_8X16_NUM;			/*	02EC	*/
	MCVX_REG	RESERVE_02F0;			/*	02F0	*/
	MCVX_REG	VP_CE_SKIP_NUM;			/*	02F4	*/
	MCVX_REG	VP_CE_FWBK_NUM;			/*	02F8	*/
	MCVX_REG	VP_CE_BWBK_NUM;			/*	02FC	*/
	MCVX_REG	RESERVE_0300;			/*	0300	*/
	MCVX_REG	VP_CE_FRMMB_NUM;		/*	0304	*/
	MCVX_REG	RESERVE_0308;			/*	0308	*/
	MCVX_REG	RESERVE_030C;			/*	030C	*/
	MCVX_REG	VP_CE_QP_SUM;			/*	0310	*/
	MCVX_REG	VP_CE_QPO_SUM;			/*	0314	*/
	MCVX_REG	VP_CE_ACP8_SUM;			/*	0318	*/
	MCVX_REG	VP_CE_DHVP4_SUM;		/*	031C	*/
	MCVX_REG	RESERVE_0320;			/*	0320	*/
	MCVX_REG	RESERVE_0324;			/*	0324	*/
	MCVX_REG	VP_CE_SAD0_SUM;			/*	0328	*/
	MCVX_REG	VP_CE_SAD1_SUM;			/*	032C	*/
	MCVX_REG	VP_CE_SAD2_SUM;			/*	0330	*/
	MCVX_REG	VP_CE_QS_SUM;			/*	0334	*/
	MCVX_REG	VP_CE_BOCJ;				/*	0338	*/
	MCVX_REG	RESERVE_033C;			/*	033C	*/
	MCVX_REG	RESERVE_0340;			/*	0340	*/
	MCVX_REG	RESERVE_0344;			/*	0344	*/
	MCVX_REG	RESERVE_0348;			/*	0348	*/
	MCVX_REG	RESERVE_034C;			/*	034C	*/
	MCVX_REG	RESERVE_0350;			/*	0350	*/
	MCVX_REG	RESERVE_0354;			/*	0354	*/
	MCVX_REG	RESERVE_0358;			/*	0358	*/
	MCVX_REG	RESERVE_035C;			/*	035C	*/
	MCVX_REG	VP_CE_LIST_INIT;		/*	0360	*/
	MCVX_REG	VP_CE_LIST_EN;			/*	0364	*/
	MCVX_REG	VP_CE_LIST_LDEN;		/*	0368	*/
	MCVX_REG	RESERVE_036C;			/*	036C	*/
	MCVX_REG	RESERVE_0370;			/*	0370	*/
	MCVX_REG	RESERVE_0374;			/*	0374	*/
	MCVX_REG	RESERVE_0378;			/*	0378	*/
	MCVX_REG	RESERVE_037C;			/*	037C	*/
	MCVX_REG	VP_CE_FCV_PICSIZE;		/*	0380	*/
	MCVX_REG	VP_CE_FCV_INCTRL;		/*	0384	*/
	MCVX_REG	VP_CE_FCV_OUTCTRL;		/*	0388	*/
	MCVX_REG	VP_CE_FCV_DCHR_YR;		/*	038C	*/
	MCVX_REG	VP_CE_FCV_DCHR_CR;		/*	0390	*/
	MCVX_REG	VP_CE_FCV_DCHR_YW;		/*	0394	*/
	MCVX_REG	VP_CE_FCV_DCHR_CW;		/*	0398	*/
	MCVX_REG	VP_CE_FCV_BA_YR;		/*	039C	*/
	MCVX_REG	VP_CE_FCV_BA_CR;		/*	03A0	*/
	MCVX_REG	VP_CE_FCV_BA_YW;		/*	03A4	*/
	MCVX_REG	VP_CE_FCV_BA_C0W;		/*	03A8	*/
	MCVX_REG	VP_CE_FCV_BA_C1W;		/*	03AC	*/
	MCVX_REG	VP_CE_FCV_STATUS;		/*	03B0	*/
	MCVX_REG	VP_CE_FCV_BA_YRH;		/*	03B4	*/
	MCVX_REG	VP_CE_FCV_BA_CRH;		/*	03B8	*/
	MCVX_REG	VP_CE_FCV_BA_YWH;		/*	03BC	*/
	MCVX_REG	VP_CE_FCV_BA_C0WH;		/*	03C0	*/
	MCVX_REG	RESERVE_03C4;			/*	03C4	*/
	MCVX_REG	RESERVE_03C8;			/*	03C8	*/
	MCVX_REG	RESERVE_03CC;			/*	03CC	*/
	MCVX_REG	RESERVE_03D0;			/*	03D0	*/
	MCVX_REG	RESERVE_03D4;			/*	03D4	*/
	MCVX_REG	RESERVE_03D8;			/*	03D8	*/
	MCVX_REG	RESERVE_03DC;			/*	03DC	*/
	MCVX_REG	RESERVE_03E0;			/*	03E0	*/
	MCVX_REG	RESERVE_03E4;			/*	03E4	*/
	MCVX_REG	RESERVE_03E8;			/*	03E8	*/
	MCVX_REG	RESERVE_03EC;			/*	03EC	*/
	MCVX_REG	RESERVE_03F0;			/*	03F0	*/
	MCVX_REG	RESERVE_03F4;			/*	03F4	*/
	MCVX_REG	RESERVE_03F8;			/*	03F8	*/
	MCVX_REG	RESERVE_03FC;			/*	03FC	*/
} MCVX_REG_CE_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* MCVX_REGISTER_H */
