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

#include <asm/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include "fdpm_drv.h"
#include "fdpm_if_par.h"
#include "fdp/fdp_depend.h"
#include "fdp/fdp_drv_l.h"
#include "fdp/fdp_drv_lfunc.h"
#include "fdp/fdp_drv.h"
#include "fdp/fdp_drv_hardw.h"
#include "include/fdpm_log.h"

extern void disp_fdp_reg(T_FDPD_MEM *FDP_obj);

#ifdef USER_ASSERT_ON
#include	<assert.h>
extern void abort(void);
#endif


/* drv_FDP_Open============================================================ */
/******************************************************************************
	Function:		fdp_set_RegInitIpc
	Description:	Ipc parameter regster initial set @ drv_FDP_Open
	Parameter:		IN  : *initipc_reg
	Returns:		0
******************************************************************************/
void fdp_set_RegInitIpc(T_FDPD_MEM *FDP_obj, T_FDPD_INITIPC_REG *initipc_reg)
{
	unsigned long reg_data;
	unsigned int rreg_data;
	DPRINT("called\n");

	fdp_reg_write(FD1_RPF_SWAP_ISWAP_BIT, P_FDP+FD1_RPF_SWAP);
	fdp_reg_write(FD1_WPF_SWAP_SSWAP_OSWAP_BIT, P_FDP+FD1_WPF_SWAP);

	fdp_reg_write(FD1_CTL_IRQENB_FRE_BIT, P_FDP+FD1_CTL_IRQENB);

	fdp_reg_rwrite((initipc_reg->smsk_th), ~FD1_IPC_SMSK_THRESH_SMSK_TH_MSK,
		       P_FDP+FD1_IPC_SMSK_THRESH);

	reg_data = (initipc_reg->cmb_ofst<<16) | (initipc_reg->cmb_max<<8) |
		(initipc_reg->cmb_grad);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_COMB_DET);

	reg_data = (initipc_reg->mov_coef<<8) | (initipc_reg->stl_coef);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_MOTDEC);

	reg_data = (initipc_reg->bld_grad<<16) | (initipc_reg->bld_max<<8) |
		(initipc_reg->bld_ofst);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_DLI_BLEND);

	reg_data = (initipc_reg->hg_grad<<16) | (initipc_reg->hg_ofst<<8) |
		(initipc_reg->hg_max);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_DLI_HGAIN);

	reg_data = (initipc_reg->sprs_grad<<16) | (initipc_reg->sprs_ofst<<8) |
		(initipc_reg->sprs_max);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_DLI_SPRS);

	reg_data = (initipc_reg->asel45<<16) | (initipc_reg->asel22<<8) |
		(initipc_reg->asel15);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_DLI_ANGLE);

	reg_data = (initipc_reg->ipix_max45<<24) |
		(initipc_reg->ipix_grad45<<16) | (initipc_reg->ipix_max22<<8) |
		(initipc_reg->ipix_grad22);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_DLI_ISOPIX0);

	reg_data = (initipc_reg->ipix_max15<<8) | (initipc_reg->ipix_grad15);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_DLI_ISOPIX1);

	reg_data = (initipc_reg->vfq_th<<24) | (initipc_reg->hfq_th<<16) |
		(initipc_reg->dif_th<<8) | (initipc_reg->sad_th);
	fdp_reg_write(reg_data, P_FDP+SENSOR_TH0);

	reg_data = (initipc_reg->detector_sel<<16) | (initipc_reg->comb_th<<8) |
		(initipc_reg->freq_th);
	fdp_reg_write(reg_data, P_FDP+SENSOR_TH1);

	fdp_reg_write(FD1_IPC_LMEM_PNUM_BIT, P_FDP+FD1_IPC_LMEM);

	rreg_data = fdp_reg_read(P_FDP+IP_VERSION);
	DPRINT("IPVERSION(%08x):%08x\n", (int)P_FDP+IP_VERSION, rreg_data);

	fdp_set_Lut(FDP_obj,      0, initipc_reg->dif_adj);
	fdp_set_Lut(FDP_obj,  0x400, initipc_reg->sad_adj);
	fdp_set_Lut(FDP_obj,  0x800, initipc_reg->bld_gain);
	fdp_set_Lut(FDP_obj,  0xC00, initipc_reg->dif_gain);
	fdp_set_Lut(FDP_obj, 0x1000, initipc_reg->mdet);

	DPRINT("done\n");

}

/******************************************************************************
	Function:		fdp_set_Lut
	Description:	Ipc parameter lut initial set @ drv_FDP_Open
	Parameter:		IN  : offset, *src_adr
	Returns:		none
******************************************************************************/
void fdp_set_Lut(T_FDPD_MEM *FDP_obj, unsigned long offset,
		 unsigned char *src_adr)
{
	int i;
	unsigned long *dst;
	unsigned long wdata;
	for (i = 0; i < 256; i++) {
		wdata = src_adr[i];
		dst = (unsigned long *)(P_FDP_LUT + offset + i*4);
		iowrite32(wdata, dst);
	}
}

/******************************************************************************
	Function:		fdp_set_VintClkStopMode
	Description:	drv_FDP_Open Vint mode & clock stop mode setting
	Parameter:		IN  : *open_par
	Global:			OUT : FDP_obj->fdp_mode
	Returns:		0
******************************************************************************/
void fdp_set_VintClkStopMode(T_FDPD_MEM *FDP_obj, T_FDP_R_OPEN *open_par)
{
	unsigned long reg_data;

	if (open_par->vmode == FDP_VMODE_NORMAL)/* normal */
		reg_data = 0x2;
	else if (open_par->vmode == FDP_VMODE_VBEST)/* best effort mode */
		reg_data = 0x2;
	else
		reg_data = 0x2;

	fdp_reg_rwrite(reg_data, ~FD1_CTL_OPMODE_VIMD_MSK,
		       P_FDP+FD1_CTL_OPMODE);

	/* Clock stop mode == mode0 */
	if (open_par->clkmode == FDP_CLKMODE_0)
		reg_data = 0x1;
	else
	/* Clock stop mode == mode1,mode3 */
		reg_data = 0x1;
	fdp_reg_rwrite(reg_data, 0xfffffffe, P_FDP+CLK_CTRL);

	return;
}

/* drv_FDP_Close=========================================================== */

/* drv_FDP_Start=========================================================== */
/******************************************************************************
	Function:		fdp_set_StartParm
	Description:	drv_FDP_Start FDP process start parameter setting
	Parameter:		IN  : *start_par
	Returns:		0
******************************************************************************/
void fdp_set_StartParm(T_FDPD_MEM *FDP_obj, T_FDP_R_START *start_par)
{
	/* FDP proccess */
	if (start_par->fdpgo == FDP_GO) { /* fdpgo=1 */
		fdp_reg_rwrite(0x1, 0xfffffffe, P_FDP+FD1_CTL_CMD);
		/* frame parameter setting */
		if (start_par->fproc_par_flg == 1) {
			/* input format parameter set */
			fdp_proc_rpf(FDP_obj, &start_par->fproc_par);
			fdp_proc_ipc(FDP_obj, &start_par->fproc_par);
			/* Picture sequence control process */
			fdp_proc_PictSEQcntrl(FDP_obj, start_par);
		} else {
			DPRINT("fproc_par invalid\n");
		}
	} else { /* fdpgo=0 */
		fdp_reg_rwrite(0x0, ~FD1_CTL_CMD_STRCMD_MSK, P_FDP+FD1_CTL_CMD);
	}
}

/***** componet for fdp_set_StartParm *****/
/******************************************************************************
	Function:		fdp_proc_PictSEQcntrl
	Description:  picture sequence control process @ drv_FDP_Start
	Parameter:    IN  : *start_par
	Global:	      IN  : FDP_obj->fdp_seq_par, FDP_obj->fdp_seq_par_pre,
		      FDP_obj->fdp_seq_par_out, FDP_obj->fdp_seq_par_out_pre,
		      FDP_obj->fdp_sub_pre,
		      FDP_obj->fdp_state, FDP_obj->fdp_out_state_pipe,
		      FDP_obj->fdp_telecine_par, FDP_obj->fdp_telecine_par_pre,
		      FDP_obj->fdp_in_pic, FDP_obj->fdp_outimg_reg_adapt,
	Returns:      none
******************************************************************************/
void fdp_proc_PictSEQcntrl(T_FDPD_MEM *FDP_obj, T_FDP_R_START *start_par)
{
	T_FDP_R_SEQ seq_par_tmp;
	T_FDP_R_FPROC *fproc_par = &start_par->fproc_par;

	unsigned char seq_mode;

	seq_mode = fproc_par->seq_par.seq_mode;
	memcpy(&seq_par_tmp, &fproc_par->seq_par, sizeof(T_FDP_R_SEQ));

	fdp_set_RegSizeIntim(FDP_obj, &seq_par_tmp);
	fdp_set_SeqModeIntim(FDP_obj, &seq_par_tmp, start_par);

	/* Output buf control ( input timing ) */
	if (fproc_par->out_buf_flg == 1)
		fdp_set_outbuf(FDP_obj, fproc_par);
	fdp_set_refbuf(FDP_obj, fproc_par);
}

/***** componet *****/
/******************************************************************************
	Function:		fdp_set_SeqModeIntim
	Description:	seq_mode set ( seq top input timing ) @ drv_FDP_Start
	Parameter:		*seq_par
	Global:		 IN  : FDP_obj->fdp_state, FDP_obj->fdp_out_state_pipe
	Returns:		none
******************************************************************************/
void fdp_set_SeqModeIntim(T_FDPD_MEM *FDP_obj, T_FDP_R_SEQ *seq_par,
			  T_FDP_R_START *start_par)
{
	int smw = 0;
	int wr = 0;
	int smr = 0;
	int rd = 0;
	int dim = 0;
	int dli = 1;
	int fsm0 = 1;

	unsigned long reg_data;
	T_FDP_R_FPROC *fproc_par = &start_par->fproc_par;

	if (fproc_par->f_decodeseq == 1) {/* force decode sequence mode */
		if (seq_par->seq_mode == FDP_SEQ_PROG) {
			/* progressive mode */
			fdp_reg_rwrite(FD1_CTL_OPMODE_PRG_BIT,
				       ~FD1_CTL_OPMODE_PRG_BIT,
				       P_FDP+FD1_CTL_OPMODE);
			fdp_reg_rwrite(FD1_IPC_MODE_DLI_BIT,
				       ~FD1_IPC_MODE_DLI_BIT,
				       P_FDP+FD1_IPC_MODE);
			smw = 0; wr = 1; smr = 0; rd = 2; dim = 1;
		} else {
			fdp_reg_rwrite(0x0, ~FD1_CTL_OPMODE_PRG_BIT,
				       P_FDP+FD1_CTL_OPMODE);
			if ((FDP_obj->fdp_independ.decode_val == 0x13) ||
			    (FDP_obj->fdp_independ.decode_val == 0x14) ||
			    (FDP_obj->fdp_independ.decode_val == 0x15) ||
			    (FDP_obj->fdp_independ.decode_val == 0x16) ||
			    (FDP_obj->fdp_independ.decode_val == 0x17) ||
			    (FDP_obj->fdp_independ.decode_val == 0x18)) {
				if (FDP_obj->fdp_independ.decode_count == 0) {
					smw = 0; wr = 1; smr = 0;
					rd = 6; dim = 4;
				} else {
					smw = 0; wr = 1; smr = 0;
					rd = 3; dim = 3;
				}
			} else {
				smw = 0; wr = 1; smr = 0;
				rd = 2; dim = 0;/* illegal entry */
			}
		}
	} else {/* normal mode */
		if (seq_par->seq_mode == FDP_SEQ_PROG) {
			/* progressive mode */
			fdp_reg_rwrite(FD1_CTL_OPMODE_PRG_BIT,
				       ~FD1_CTL_OPMODE_PRG_BIT,
				       P_FDP+FD1_CTL_OPMODE);
			smw = 0; wr = 1; smr = 0; rd = 2; dim = 1;
		} else if ((seq_par->seq_mode == FDP_SEQ_INTER) ||
			   (seq_par->seq_mode == FDP_SEQ_INTERH)) {
			/* interlace mode */
			fdp_reg_rwrite(0x0, ~FD1_CTL_OPMODE_PRG_BIT,
				       P_FDP+FD1_CTL_OPMODE);
			if (fproc_par->seq_par_flg == 1) {
				smw = 1; wr = 1; smr = 0; rd = 2;/* first */
				dim = 1; fsm0 = 1;
			} else if (fproc_par->last_start == 1) {/* final */
				smw = 0; wr = 1; smr = 0; rd = 2;
				dim = 1;
			} else if (fproc_par->seq_par_flg == 2) {
				smw = 1; wr = 1; smr = 0; rd = 7;/* second */
			} else {
				smw = 1; wr = 1; smr = 1; rd = 7;/* other */
			}
			if ((start_par->telecine_flg > 0) &&
			    (fproc_par->last_start != 1)) {
				smw = 1; wr = 1; smr = 1;
				rd = 7; dim = start_par->next_pattern;
			}
		} else if ((seq_par->seq_mode == FDP_SEQ_INTER_2D) ||
			   (seq_par->seq_mode == FDP_SEQ_INTERH_2D)) {
			/* interlace mode */
			fdp_reg_rwrite(0x0, ~FD1_CTL_OPMODE_PRG_BIT,
				       P_FDP+FD1_CTL_OPMODE);
			smw = 0; wr = 1; smr = 0; rd = 2;/* fixed 2D */
			dim = 1;
		}
	}
	FDP_obj->fdp_independ.smw   = smw;
	FDP_obj->fdp_independ.wr    = wr;
	FDP_obj->fdp_independ.chact = rd;
	reg_data = (fsm0<<16);
	fdp_reg_rwrite(reg_data, ~FD1_IPC_SMSK_THRESH_FSM0_BIT,
		       P_FDP+FD1_IPC_SMSK_THRESH);
	reg_data = (smw<<9) | (wr<<8) | (smr<<3) | (rd);
	fdp_reg_write(reg_data, P_FDP+FD1_CTL_CHACT);
	reg_data = (dli<<8) | (dim);
	fdp_reg_write(reg_data, P_FDP+FD1_IPC_MODE);
}

/******************************************************************************
	Function:		fdp_set_RegSizeIntim
	Description:Input image size register set @ sequence top @drv_FDP_Start
	Parameter:		*seq_par
	Returns:		none
******************************************************************************/
void fdp_set_RegSizeIntim(T_FDPD_MEM *FDP_obj, T_FDP_R_SEQ *seq_par)
{
	int frm_lvl_th = 2;/* frame level thresh */
	int fld_lvl_th = 2;/* field level thresh */
	int fd_en = 1;     /* film detector enable */
	int div_width1, div_width2;
	int hist_width, hist_height;
	int start_pos_x, start_pos_y;
	int end_pos_x, end_pos_y;
	unsigned long reg_data;

	div_width1 = seq_par->in_width/3;/* Area 0/1 border */
	div_width2 = seq_par->in_width*2/3;/* Area 1/2 border */
	hist_width = (seq_par->in_width == 0) ? 0 : seq_par->in_width - 1;

	if (seq_par->in_height == 0) {
		hist_height = 0;
	} else {
		if (seq_par->seq_mode == FDP_SEQ_PROG)
			hist_height = seq_par->in_height - 1;
		else
			hist_height = seq_par->in_height*2 - 1;
	}

	reg_data = (seq_par->in_width<<16) | (seq_par->in_height);
	fdp_reg_write(reg_data, P_FDP+FD1_RPF_SIZE);

	/* Film detection function setting */
	reg_data = (frm_lvl_th<<12) | (fld_lvl_th<<8) | (fd_en);
	fdp_reg_write(reg_data, P_FDP+FD1_SNSOR_CTL0);

	/* start position (0,0) */
	start_pos_x = 0;
	start_pos_y = 0;
	reg_data = ((start_pos_x)<<16) | (start_pos_y);
	fdp_reg_write(reg_data, P_FDP+FD1_SNSOR_CTL1);

	/* end position (hsize,vsize) */
	end_pos_x = hist_width;
	end_pos_y = hist_height;
	reg_data = ((end_pos_x)<<16) | (end_pos_y);
	fdp_reg_write(reg_data, P_FDP+FD1_SNSOR_CTL2);

	reg_data = (div_width1<<16) | (div_width2);/* Area 0/1/2 */
	fdp_reg_write(reg_data, P_FDP+FD1_SNSOR_CTL3);
}

/***** componet *****/
/******************************************************************************
	Function:		fdp_set_refbuf
	Description:	setting for ref_buf @ drv_FDP_Start
	Parameter:		IN  : *fproc_par
	Global:			IN  : FDP_obj->fdp_mode
	Returns:		none
******************************************************************************/
void fdp_set_refbuf(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par)
{
	T_FDP_R_REFBUF *ref_buf;
	unsigned long reg_data;

	ref_buf = &fproc_par->ref_buf;

	/* === Read channel === */
	/* --- Refernce image Y(Back) (ch2 ipcx_in0_rd_y) --- */
	/* start adr */
	if (ref_buf->buf_refrd0_flg == 1) {
		fdp_reg_write((unsigned long)ref_buf->buf_refrd0.addr,
			      P_FDP+FD1_RPF2_ADDR_Y);
		fdp_reg_write((unsigned long)ref_buf->buf_refrd0.addr_c0,
			      P_FDP+FD1_RPF2_ADDR_C0);
		fdp_reg_write((unsigned long)ref_buf->buf_refrd0.addr_c1,
			      P_FDP+FD1_RPF2_ADDR_C1);
	}
	if (ref_buf->buf_refrd1_flg == 1) {
		fdp_reg_write((unsigned long)ref_buf->buf_refrd1.addr,
			      P_FDP+FD1_RPF1_ADDR_Y);
		fdp_reg_write((unsigned long)ref_buf->buf_refrd1.addr_c0,
			      P_FDP+FD1_RPF1_ADDR_C0);
		fdp_reg_write((unsigned long)ref_buf->buf_refrd1.addr_c1,
			      P_FDP+FD1_RPF1_ADDR_C1);
		reg_data = (ref_buf->buf_refrd1.stride<<16) |
			(ref_buf->buf_refrd1.stride_c);
		fdp_reg_write(reg_data, P_FDP+FD1_RPF_PSTRIDE);
	}
	if (ref_buf->buf_refrd2_flg == 1) {
		fdp_reg_write((unsigned long)ref_buf->buf_refrd2.addr,
			      P_FDP+FD1_RPF0_ADDR_Y);
		fdp_reg_write((unsigned long)ref_buf->buf_refrd2.addr_c0,
			      P_FDP+FD1_RPF0_ADDR_C0);
		fdp_reg_write((unsigned long)ref_buf->buf_refrd2.addr_c1,
			      P_FDP+FD1_RPF0_ADDR_C1);
	}

	/* === Write channel === */
	/* --- Refernce image Y(write) (ch0	pcx_fir_wr_y) --- */
	/* start adr */
	if (0) {
		fdp_reg_write((unsigned long)ref_buf->buf_refwr.addr,
			      P_FDP+FD1_WPF_ADDR_Y);
		fdp_reg_write((unsigned long)ref_buf->buf_refwr.addr_c0,
			      P_FDP+FD1_WPF_ADDR_C0);
		fdp_reg_write((unsigned long)ref_buf->buf_refwr.addr_c1,
			      P_FDP+FD1_WPF_ADDR_C1);
		reg_data = (ref_buf->buf_refwr.stride<<16) |
			(ref_buf->buf_refwr.stride_c);
		fdp_reg_write(reg_data, P_FDP+FD1_WPF_PSTRIDE);
	}
}

/******************************************************************************
	Function:		fdp_set_outbuf
	Description:	setting for out_buf @ drv_FDP_Start
	Parameter:		IN  : *out_buf, out_format, *seq_par_out
	Returns:		none
******************************************************************************/
void fdp_set_outbuf(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par)
{
	int pdv = 0xff;
	int csc;
	int wspycs;
	int wspuvs;
	int dith;
	int wrtm = 0;
	int wrfmt;
	unsigned long reg_data;

	/* output format setting */
	switch (fproc_par->out_format) {
	case FDP_YUV420_YV12:
		csc = 0; wspuvs = 0; wspycs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x4c; break;
	case FDP_YUV420_NV21:
		csc = 0; wspuvs = 1; wspycs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x42; break;
	case FDP_YUV422_NV16:
		csc = 0; wspuvs = 0; wspycs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x41; break;
	case FDP_YUV422_YUY2:
		csc = 0; wspuvs = 0; wspycs = 1; dith = 0; wrtm = 0;
		wrfmt = 0x47; break;
	case FDP_YUV422_UYVY:
		csc = 0; wspuvs = 0; wspycs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x47; break;
	case FDP_RGB_332:
		csc = 1; wspycs = 0; wspuvs = 0; dith = 3; wrtm = 0;
		wrfmt = 0x00; break;
	case FDP_RGBA_4444:
		csc = 1; wspycs = 0; wspuvs = 0; dith = 3; wrtm = 0;
		wrfmt = 0x02; break;
	case FDP_RGBA_5551:
		csc = 1; wspycs = 0; wspuvs = 0; dith = 3; wrtm = 0;
		wrfmt = 0x05; break;
	case FDP_RGB_565:
		csc = 1; wspycs = 0; wspuvs = 0; dith = 3; wrtm = 0;
		wrfmt = 0x06; break;
	case FDP_RGB_888:
		csc = 1; wspycs = 0; wspuvs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x15; break;
	case FDP_RGBA_8888:
		csc = 1; wspycs = 0; wspuvs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x14; break;
	default:
		csc = 0; wspuvs = 0; wspycs = 0; dith = 0; wrtm = 0;
		wrfmt = 0x42; break;
	}
	/* format setting */
	reg_data =  (pdv<<24) | (wspycs<<15) | (wspuvs<<14) | (dith<<12) |
		(wrtm<<9) | (csc<<8) | (wrfmt);
	fdp_reg_write(reg_data, P_FDP+FD1_WPF_FORMAT);

	/* stlmsk address setting */
	reg_data = FDP_obj->fdp_independ.stlmsk_adr;
	fdp_reg_write(reg_data, P_FDP+FD1_RPF_SMSK_ADDR);

	/* output address setting */
	reg_data = (fproc_par->out_buf.stride<<16) |
		(fproc_par->out_buf.stride_c);
	fdp_reg_write(reg_data, P_FDP+FD1_WPF_PSTRIDE);
	fdp_reg_write(fproc_par->out_buf.addr, P_FDP+FD1_WPF_ADDR_Y);
	fdp_reg_write(fproc_par->out_buf.addr_c0, P_FDP+FD1_WPF_ADDR_C0);
	fdp_reg_write(fproc_par->out_buf.addr_c1, P_FDP+FD1_WPF_ADDR_C1);
}

/***** componet for fdp_set_StartParm *****/
/* interupt handler========================================================= */
/******************************************************************************
	Function:		fdp_int_update_state
	Description:	interupt handler : update state
	Parameter:		none
	Global:			IN  : FDP_obj->fdp_state,FDP_obj->fdp_state_pre
				OUT : FDP_obj->fdp_state
	Returns:		0
******************************************************************************/
void fdp_int_update_state(T_FDPD_MEM *FDP_obj)
{
	FDP_obj->fdp_independ.fdp_state.vcycle =
		fdp_reg_read(P_FDP+FD1_CTL_VCYCLE_STAT);
	disp_fdp_reg(FDP_obj);
	FDP_obj->fdp_independ.fdp_state.status = ST_FDP_RDY;
}

/******************************************************************************
	Function:		fdp_update_pipline_fdpnogo
	Description:	fdp_update_pipline @ fdpnogo @ interrupt handler
	Parameter:		none
	Global:			IN  : FDP_obj->fdp_out_state_pipe
				OUT : FDP_obj->fdp_out_state_pipe
	Returns:		none
******************************************************************************/
void fdp_update_pipline_fdpnogo(T_FDPD_MEM *FDP_obj)
{
	/* out_enable update ( Pipeline delay 1 stage ) */
	FDP_obj->fdp_out_state_pipe.out_enable[0] =
		FDP_obj->fdp_out_state_pipe.out_enable_pre;
}

/******************************************************************************
	Function:		fdp_proc_rpf
	Description:	RPF/WPF setting at Start timing
	Parameter:		none
	Global:			IN  : start_par->fproc_par
	Returns:		void
******************************************************************************/
void fdp_proc_rpf(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par)
{
	int cipm;
	int rspuvs;
	int rspycs;
	int tff = 1;
	int cf;
	int rdfmt;
	unsigned long reg_data;

	cf = fproc_par->cf;

	cipm = (fproc_par->seq_par.seq_mode == FDP_SEQ_PROG) ? 0 : 1;

	switch (fproc_par->in_pic.pic_par.chroma_format) {
	case FDP_YUV420_YV12:
		rspuvs = 0; rspycs = 0; rdfmt = 0x4c; break;
	case FDP_YUV420_NV21:
		rspuvs = 1; rspycs = 0; rdfmt = 0x42; break;
	case FDP_YUV422_NV16:
		rspuvs = 0; rspycs = 0; rdfmt = 0x41; break;
	case FDP_YUV422_YUY2:
		rspuvs = 0; rspycs = 1; rdfmt = 0x47; break;
	default:
		rspuvs = 0; rspycs = 0; rdfmt = 0x42; break;
	}

	reg_data = (cipm<<16) | (rspycs<<13) | (rspuvs<<12) |
		(tff<<9) | (cf<<8) | (rdfmt);
	fdp_reg_write(reg_data, P_FDP+FD1_RPF_FORMAT);
}

/******************************************************************************
	Function:		fdp_proc_ipc
	Description:	IPC setting at Start timing
	Parameter:		none
	Global:			IN  : start_par->fproc_par
	Returns:		void
******************************************************************************/
void fdp_proc_ipc(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par)
{

	if (fproc_par->in_pic_flg == 1) {
		if (fproc_par->in_pic.pic_par_flg == 1) {
			FDP_obj->fdp_independ.fdp_pic_par.width  =
				fproc_par->in_pic.pic_par.width;
			FDP_obj->fdp_independ.fdp_pic_par.height =
				fproc_par->in_pic.pic_par.height;
			FDP_obj->fdp_independ.fdp_pic_par.chroma_format =
				fproc_par->in_pic.pic_par.chroma_format;
			FDP_obj->fdp_independ.fdp_pic_par.progressive_sequence =
				fproc_par->in_pic.pic_par.progressive_sequence;
			FDP_obj->fdp_independ.fdp_pic_par.progressive_frame =
				fproc_par->in_pic.pic_par.progressive_frame;
			FDP_obj->fdp_independ.fdp_pic_par.picture_structure =
				fproc_par->in_pic.pic_par.picture_structure;
			FDP_obj->fdp_independ.fdp_pic_par.repeat_first_field =
				fproc_par->in_pic.pic_par.repeat_first_field;
			FDP_obj->fdp_independ.fdp_pic_par.top_field_first =
				fproc_par->in_pic.pic_par.top_field_first;
		} else {
			DPRINT("fproc_par->in_pic.pic_par_flg = 0\n");
			memcpy(&FDP_obj->fdp_independ.fdp_pic_par,
			       &FDP_obj->fdp_independ.fdp_pic_par_pre,
			       sizeof(T_FDPD_PICPAR));
		}
	} else {
		DPRINT("fproc_par->in_pic_flg = 0\n");
		memcpy(&FDP_obj->fdp_independ.fdp_pic_par,
		       &FDP_obj->fdp_independ.fdp_pic_par_pre,
		       sizeof(T_FDPD_PICPAR));
	}
}

/******************************************************************************
	Function:		fdp_out_adr_set
	Description:	fdp output address set
	Parameter:		none
	Global:			IN  : start_par->fproc_par
	Returns:		void
******************************************************************************/
void fdp_out_adr_set(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par)
{
	T_FDP_R_REFBUF *ref_buf;
	unsigned long reg_data;

	ref_buf = &fproc_par->ref_buf;

	if (ref_buf->buf_refwr_flg == 1) {
		fdp_reg_write((unsigned long)ref_buf->buf_refwr.addr,
			      P_FDP+FD1_WPF_ADDR_Y);
		fdp_reg_write((unsigned long)ref_buf->buf_refwr.addr_c0,
			      P_FDP+FD1_WPF_ADDR_C0);
		fdp_reg_write((unsigned long)ref_buf->buf_refwr.addr_c1,
			      P_FDP+FD1_WPF_ADDR_C1);
		reg_data = (ref_buf->buf_refwr.stride<<16) |
			(ref_buf->buf_refwr.stride_c);
		fdp_reg_write(reg_data, P_FDP+FD1_WPF_PSTRIDE);
	}
}

/* ========================================================================= */
inline void fdp_reg_rwrite(unsigned long wdata, unsigned long mask, void *addr)
{
	unsigned long reg_data;

	reg_data = ioread32(addr);
	reg_data = (reg_data & mask) | wdata;
	iowrite32(reg_data, addr);
}

inline void fdp_reg_write(unsigned long wdata, void *addr)
{
	iowrite32(wdata, addr);
}

inline unsigned long fdp_reg_read(void *addr)
{
	unsigned long reg_data;

	reg_data = ioread32(addr);
	return reg_data;
}
