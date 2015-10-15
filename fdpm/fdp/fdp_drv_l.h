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

#ifndef _FDP_DRV_L_H
#define _FDP_DRV_L_H

/*-------------------- global variable structure ----------------------------*/

typedef struct {
	void *userdata1;
	void *userdata2;
	void (*fdp_cb1)(T_FDP_CB1 *);
	void (*fdp_cb2)(void *);
} T_FDPD_SUB_OBJ;

/* for drv_FDP_Open */
typedef struct {
	unsigned char ref_mode;
	unsigned char refbuf_mode;
	T_FDP_REFPREBUF *refbuf;
	unsigned char vmode;
	unsigned char clkmode;
	unsigned short vcnt;
	unsigned char sequence_max; /* add for FDP1 */
} T_FDPD_OPEN;

typedef struct {
	unsigned char	smsk_th;
	unsigned char	cmb_grad;
	unsigned char	cmb_ofst;
	unsigned char	cmb_max;
	unsigned char	mov_coef;
	unsigned char	stl_coef;
	unsigned char	bld_grad;
	unsigned char	bld_ofst;
	unsigned char	bld_max;
	unsigned char	hg_grad;
	unsigned char	hg_ofst;
	unsigned char	hg_max;
	unsigned char	sprs_grad;
	unsigned char	sprs_ofst;
	unsigned char	sprs_max;
	unsigned char	asel45;
	unsigned char	asel22;
	unsigned char	asel15;
	unsigned char	ipix_max45;
	unsigned char	ipix_grad45;
	unsigned char	ipix_max22;
	unsigned char	ipix_grad22;
	unsigned char	ipix_max15;
	unsigned char	ipix_grad15;
	unsigned char	vfq_th;
	unsigned char	hfq_th;
	unsigned char	dif_th;
	unsigned char	sad_th;
	unsigned char	detector_sel;
	unsigned char	comb_th;
	unsigned char	freq_th;
	unsigned char	dif_adj[256];
	unsigned char	sad_adj[256];
	unsigned char	bld_gain[256];
	unsigned char	dif_gain[256];
	unsigned char    mdet[256];
} T_FDPD_INITIPC_REG;

typedef struct {
	void *addr;
	void *addr_c0;
	void *addr_c1;
	unsigned short stride;
	unsigned short stride_c;
	unsigned short height;
	unsigned short height_c;
} T_FDPD_IMGBUF;

/* FDP telecine mode parameter */
typedef struct {
	unsigned char	telecine_out_MODE[2];
	unsigned char	telecine_out_MODE_pre;
} T_FDPD_TELECINE_OUT;

typedef struct {
	unsigned char      first_count_flag;
	unsigned char	telecine_repeat_count;
	unsigned char	telecine_struct[3];
	unsigned char	telecine_mode[3];
} T_FDPD_TELECINE_PAR;

/* for drv_FDP_Start	picture main parameter */
typedef struct {
	unsigned short  picid;
	T_FDPD_PICPAR *pic_par;
	T_FDPD_IMGBUF *in_buf1;
	T_FDPD_IMGBUF *in_buf2;
} T_FDPD_PIC;

/* out state pipeline */
typedef struct {
	unsigned char	out_enable[3];
	unsigned long	out_picid[3];
	unsigned char	out_left[3];
	unsigned char	out_req[1];
	unsigned char	out_enable_pre;
	unsigned long	out_picid_pre;
	unsigned char	out_left_pre;
	unsigned char	out_req_pre;
} T_FDPD_OUT_STATE_PIPE;

/* reference memory pointer */
typedef struct {
	unsigned char	refwr_num;
	unsigned char	refrd0_num;
	unsigned char	refrd1_num;
	unsigned char	refrd2_num;
} T_FDPD_REFPOINTER;

/* FDP logical driver use memory */
typedef struct {
	T_FDPD_SUB_OBJ         fdp_sub;
	T_FDPD_PIC	       fdp_in_pic;
	T_FDPD_PIC	       fdp_in_pic_pre;
	T_FDPD_TELECINE_PAR    fdp_telecine_par;
	T_FDPD_TELECINE_PAR    fdp_telecine_par_pre;
	T_FDPD_OUT_STATE_PIPE  fdp_out_state_pipe;
	unsigned char	       fdp_seq_update_flag;
	unsigned char	       fdp_seq_out_update_flag;
	unsigned char	       fdp_vint_update_flag;
	unsigned char	       fdp_inimg_update_flag;
	unsigned char	       fdp_interrupt_chk_flag;
	int irq;

	struct fdp_independ    fdp_independ;

	void __iomem *mapbase; /* map base */
	resource_size_t start_reg_adr;
	struct resource *mmio_res;
} T_FDPD_MEM;

#endif
