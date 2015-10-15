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

#ifndef __FDP_DRV_LFUNC_H_
#define __FDP_DRV_LFUNC_H_

/*-------------------- local function prototype declaration -----------------*/
/* parameters check */
#ifdef FDP_PAR_CHECK_MODE
#endif
/* drv_FDP_Init */
void fdp_initialize(void);
/* drv_FDP_Open */
long fdp_GetInitIpcParam(T_FDPD_INITIPC_REG *initipc_reg);
void fdp_set_RefBufMode(T_FDPD_OPEN *open_par);
void fdp_set_RegInitIpc(T_FDPD_MEM *FDP_obj, T_FDPD_INITIPC_REG *initipc_reg);
void fdp_set_Lut(T_FDPD_MEM *FDP_obj, unsigned long offset,
		 unsigned char *src_adr);
void fdp_set_VintClkStopMode(T_FDPD_MEM *FDP_obj, T_FDP_R_OPEN *open_par);
void fdp_set_VintReg(T_FDPD_OPEN *open_par);
/* drv_FDP_Close */
void fdp_set_VintStop(void);
/* drv_FDP_Start */
void fdp_proc_rpf(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par);
void fdp_proc_ipc(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par);
void fdp_adr_set(T_FDPD_PIC *in_pic, int top, int direction);
void fdp_out_adr_set(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par);
void fdp_proc_PictSEQcntrl(T_FDPD_MEM *FDP_obj, T_FDP_R_START *start_par);
void fdp_clr_FdpCb1Clr(T_FDP_CB1 *fdp_cb1);
void fdp_out_PrevFdpCb1Out(T_FDP_CB1 *fdp_cb1);

void fdp_store_SeqPar(T_FDPD_SEQ *seq_par);
void fdp_set_SeqModeIntim(T_FDPD_MEM *FDP_obj, T_FDP_R_SEQ *seq_par,
			  T_FDP_R_START *start_par);
void fdp_set_RegSizeIntim(T_FDPD_MEM *FDP_obj, T_FDP_R_SEQ *seq_par);
void fdp_clr_PipelineClr(T_FDPD_SEQ *seq_par);

void fdp_cntl_PictSeqCntlPrm(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par);
void fdp_store_SeqParOuttim(T_FDPD_SEQ *seq_par);
void fdp_set_SeqModeOuttim(T_FDPD_SEQ *seq_par_out);
void fdp_set_RegSizeOuttim(T_FDPD_SEQ *seq_par_out);
void fdp_set_RegRatio(T_FDPD_SEQ *seq_par_out);
void fdp_store_InPic(T_FDPD_PIC *in_pic);
void fdp_cntl_TeleCineChk(T_FDPD_PIC *in_pic,
			  T_FDPD_TELECINE_PAR *fdp_telecine_par);
void fdp_get_TeleCinePictMode(unsigned char *telecine_pict_mode,
			      unsigned char *telecine_out_par,
			      unsigned char *telecine_repeat_field_count,
			      T_FDPD_TELECINE_PAR *fdp_telecine_par);
void fdp_set_refbuf(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par);
void fdp_set_outbuf(T_FDPD_MEM *FDP_obj, T_FDP_R_FPROC *fproc_par);
void fdp_set_RegTeleCineOuttim(unsigned char ipc_mode,
			       unsigned char telecine_out_mode,
			       T_FDPD_SEQ *seq_par_out);
void fdp_init_outimg_reg_pipe(void);
void fdp_update_state_prev_fdpnogo(void);
/* drv_FDP_Status */
/*  none */
/* interupt handler */
void fdp_update_SeqParStore(void);
void fdp_update_SeqParOuttimStore(void);
void fdp_update_FdpSub(void);
void fdp_update_TelecinePar(void);
void fdp_update_FdpCb1(void);
void fdp_int_update_state(T_FDPD_MEM *FDP_obj);
void fdp_update_pipline(T_FDPD_MEM *FDP_obj);
void fdp_update_pipline_fdpnogo(T_FDPD_MEM *FDP_obj);
void fdp_set_RegTeleCineOuttimPipline(void);
#endif
