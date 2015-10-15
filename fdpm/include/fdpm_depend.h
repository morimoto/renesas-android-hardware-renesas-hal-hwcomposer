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

#ifndef __FDPM_DEPEND_H__
#define __FDPM_DEPEND_H__

typedef struct {
	unsigned char    half_tgl;
	unsigned char    first_seq;
	unsigned int     seq_count;
	unsigned char    seq_mode;
	int              decode_val;
	int              decode_count;
	int              count_max;
	unsigned char    telecine_mode;
	T_FDPD_PICPAR    fdp_pic_par;
	T_FDPD_PICPAR    fdp_pic_par_pre;
	T_FDPD_SEQ       fdp_seq_par;
} T_FDP_SEQ_STATUS;

typedef struct {
	unsigned char *stlmsk_adr[2];
	unsigned char *stlmsk_vadr[2];
	int stl_msk_byte;
	unsigned char *phy_addr[2];
	int stlmsk_flg;
} fdpm_stlmsk_table;

typedef struct {
	unsigned char use;
	unsigned char use_count;
	unsigned char mode;
	unsigned long sumsize;
} fdpm_hw_resource;

typedef struct {
	unsigned char fdp_resource_count;
	unsigned char fdp_share_vint_count;
	unsigned char fdp_share_be_count;
	unsigned int fdpm_status;
	fdpm_hw_resource fdpm_hw[FDPM_FDP_NUM];
} fdpm_pdata;

struct fdpm_resource_table {
	unsigned int use;
	unsigned long source_id;
	unsigned int device_no;
	unsigned char ocmode;
	unsigned char vintmode;
	unsigned char timer_ch;
	unsigned long size;
	T_FDP_STATUS status;
	T_FDP_SEQ_STATUS seq_status;
	fdpm_stlmsk_table stlmsk_table;
	struct fdpm_resource_table *next;
};

struct fdpm_independ {
	struct fdpm_resource_table    fdpm_itable;
	fdpm_pdata             fdpm_ipdata;
};

#endif
