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

#ifndef __FDPM_IF_PAR_H_
#define __FDPM_IF_PAR_H_

typedef struct {
	unsigned long addr;
	unsigned long addr_c0;
	unsigned long addr_c1;
	unsigned short stride;
	unsigned short stride_c;
	unsigned short height;
	unsigned short height_c;
} T_FDP_R_IMGBUF;

typedef struct {
	unsigned char   buf_ref0_flg;
	T_FDP_R_IMGBUF buf_ref0;
	unsigned char   buf_ref1_flg;
	T_FDP_R_IMGBUF buf_ref1;
	unsigned char   buf_ref2_flg;
	T_FDP_R_IMGBUF buf_ref2;
	unsigned char   buf_refprg_flg;
	T_FDP_R_IMGBUF buf_refprg;
	unsigned long   stlmsk_adr;
	unsigned char   buf_iir_flg;
	T_FDP_R_IMGBUF buf_iir;
} T_FDP_R_REFPREBUF;

typedef struct {
	unsigned short width;
	unsigned short height;
} T_FDP_R_IMGSIZE;


typedef struct {
	unsigned char ref_mode;
	unsigned char refbuf_mode;
	unsigned char refbuf_flg;
	T_FDP_R_REFPREBUF refbuf;
	unsigned char ocmode;
	unsigned char vmode;
	T_FDP_R_IMGSIZE insize;
	unsigned char clkmode;
	unsigned long vcnt;
} T_FDP_R_OPEN;

typedef struct {
	unsigned char    ratio_type;
	short            h_iniphase;
	short            h_endphase;
	unsigned short   h_ratio;
	short            v_iniphase;
	short            v_endphase;
	unsigned short   v_ratio;
} T_FDP_R_RATIO;

typedef struct {
	unsigned char    dummy;
} T_FDP_R_IMGSET;

typedef struct {
	unsigned short   width;
	unsigned short   height;
	unsigned char    chroma_format;
	unsigned char    progressive_sequence;
	unsigned char    progressive_frame;
	unsigned char    picture_structure;
	unsigned char    repeat_first_field;
	unsigned char    top_field_first;
} T_FDP_R_PICPAR;

typedef struct {
	unsigned long    picid;
	unsigned char    pic_par_flg;
	T_FDP_R_PICPAR    pic_par;
	unsigned char      in_buf1_flg;
	T_FDP_R_IMGBUF    in_buf1;
	unsigned char      in_buf2_flg;
	T_FDP_R_IMGBUF    in_buf2;
} T_FDP_R_PIC;

typedef struct {
	unsigned char    seq_mode;
	unsigned char    scale_mode;
	unsigned char    filter_mode;
	unsigned char    telecine_mode;
	unsigned short   in_width;
	unsigned short   in_height;
	unsigned short   imgleft;
	unsigned short   imgtop;
	unsigned short   imgwidth;
	unsigned short   imgheight;
	unsigned short   out_width;
	unsigned short   out_height;
	unsigned char    ratio_flg;
	T_FDP_R_RATIO   ratio;
} T_FDP_R_SEQ;

typedef struct {
	unsigned char      buf_refwr_flg;
	T_FDP_R_IMGBUF    buf_refwr;
	unsigned char      buf_refrd0_flg;
	T_FDP_R_IMGBUF    buf_refrd0;
	unsigned char      buf_refrd1_flg;
	T_FDP_R_IMGBUF    buf_refrd1;
	unsigned char      buf_refrd2_flg;
	T_FDP_R_IMGBUF    buf_refrd2;
	unsigned char      buf_iirwr_flg;
	T_FDP_R_IMGBUF    buf_iirwr;
	unsigned char      buf_iirrd_flg;
	T_FDP_R_IMGBUF    buf_iirrd;
} T_FDP_R_REFBUF;

typedef struct {
	unsigned char    seq_par_flg;
	T_FDP_R_SEQ     seq_par;
	unsigned char    imgset_par_flg;
	T_FDP_R_IMGSET  imgset_par;
	unsigned char    in_pic_flg;
	T_FDP_R_PIC     in_pic;
	unsigned char    last_start;
	unsigned char    cf;
	unsigned char    f_decodeseq;
	unsigned char    out_buf_flg;
	T_FDP_R_IMGBUF  out_buf;
	unsigned char    out_format;
	unsigned char    ref_buf_flg;
	T_FDP_R_REFBUF  ref_buf;
} T_FDP_R_FPROC;


typedef struct {
	unsigned char    vcnt_flg;
	unsigned short   vcnt;
	unsigned char    fdpgo;
	unsigned char    telecine_flg;
	unsigned char    next_pattern;
	unsigned char    fproc_par_flg;
	T_FDP_R_FPROC     fproc_par;
} T_FDP_R_START;

typedef struct {
	unsigned char    insize_flg;
	T_FDP_IMGSIZE   insize;
	T_FDP_IMGSIZE   refsize;
	unsigned char    outsize_flg;
	T_FDP_IMGSIZE   outsize;
	unsigned char    refwr_num;
	unsigned char    refwr_y_en;
	unsigned char    refwr_c_en;
	unsigned char    refrd0_en;
	unsigned char    refrd1_en;
	unsigned char    refrd2_en;
} T_FDP_R_CB1;

typedef struct {
	unsigned int  sensor_info[18];
} T_FDP_R_SENSOR_INFO;

typedef struct {
	char name[30];
	unsigned long reg_value;
} T_FDP_R_REG_STATUS;
#endif
