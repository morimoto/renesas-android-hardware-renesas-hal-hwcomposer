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

#include "fdpm_api.h"
#include "fdp/fdp_depend.h"
#include "fdpm_if_par.h"
#include "include/fdpm_def.h"
#include "include/fdpm_depend.h"
#include "include/fdpm_log.h"

int fdpm_check_decodeinfo(T_FDPD_PICPAR previous_pic_par, T_FDP_R_PICPAR *current_pic_par);
int fdpm_dec_decodeinfo(T_FDP_R_PICPAR *pic_par);
int fdpm_dec_decodeinfo2(T_FDPD_PICPAR *pic_par);
void fdpm_decode_count_update(T_FDP_SEQ_STATUS *fdp_seq_status,
			      int cclr);
void fdpm_update_picpar(struct fdpm_resource_table *resource_table,
			T_FDP_R_FPROC *fproc_par);

void fdpm_status_update(T_FDP_R_START *start_par,
			T_FDP_SEQ_STATUS *fdp_seq_status,
			T_FDP_HW_STATUS *fdp_hw_status,
			T_FDP_STATUS *fdp_status,
			fdpm_pdata *fdpm_pdata,
			struct fdp_independ *fdp_independ)
{
	int seq_mode;

	if (start_par->fproc_par.seq_par_flg == 1)
		seq_mode = start_par->fproc_par.seq_par.seq_mode;
	else
		seq_mode = fdp_independ->seq_mode;


	if ((seq_mode == FDP_SEQ_PROG) || (seq_mode == FDP_SEQ_INTER) ||
	    (seq_mode == FDP_SEQ_INTER_2D)) {
		fdp_status->seq_lock  = FDP_SEQ_UNLOCK;
		fdp_status->in_enable = 1;
		fdp_status->in_picid  = start_par->fproc_par.in_pic.picid;
		fdp_status->in_left   = fdp_seq_status->count_max - fdp_seq_status->decode_count;
		fdp_status->out_enable = FDP_OUT_ENABLE;
		fdp_status->out_picid = start_par->fproc_par.in_pic.picid;
		fdp_status->out_left  = fdp_seq_status->count_max - fdp_seq_status->decode_count;
		fdp_status->out_req   = FDP_OUT_REQ;
	} else if ((seq_mode == FDP_SEQ_INTERH) ||
		   (seq_mode == FDP_SEQ_INTERH_2D)) {
		if (fdp_seq_status->first_seq == 1) {
			fdp_status->seq_lock  = FDP_SEQ_LOCK;
			fdp_seq_status->first_seq = 0;
		} else{
			fdp_status->seq_lock = FDP_SEQ_UNLOCK;
		}
		fdp_status->in_enable = 1;
		fdp_status->in_picid  = start_par->fproc_par.in_pic.picid;
		fdp_status->in_left   = 0;
		fdp_status->out_enable = FDP_OUT_ENABLE;
		fdp_status->out_picid = start_par->fproc_par.in_pic.picid;
		fdp_status->out_left  = 0;
		if (fdp_seq_status->half_tgl == 1) {
			fdp_status->out_req = FDP_OUT_NOREQ;
			fdp_seq_status->half_tgl = 0;
		} else {
			fdp_status->out_req = FDP_OUT_REQ;
			fdp_seq_status->half_tgl = 1;
		}
	}
	fdp_status->status    = fdpm_pdata->fdpm_status;
	fdp_status->delay     = fdp_hw_status->delay;
	fdp_status->vcycle    = fdp_independ->fdp_state.vcycle;
	fdp_status->vintcnt   = fdp_hw_status->vintcnt;
}

void fdpm_fdp_cb1_update(T_FDP_R_START *start_par, T_FDP_R_CB1 *fdp_cb1, struct fdp_independ *fdp_independ, int stlmsk_flg)
{
	if (start_par->fproc_par.in_pic.pic_par_flg == 1) {
		fdp_cb1->insize_flg = 1;
		fdp_cb1->insize.width = start_par->fproc_par.in_pic.pic_par.width;
		fdp_cb1->insize.height = start_par->fproc_par.in_pic.pic_par.height;
		fdp_cb1->refsize.width = start_par->fproc_par.in_pic.pic_par.width;
		fdp_cb1->refsize.height = start_par->fproc_par.in_pic.pic_par.height;
	} else {
		fdp_cb1->insize_flg = 0;
	}
	if (fdp_independ->wr == 1) {
		fdp_cb1->outsize_flg = 1;
		fdp_cb1->outsize.width  = fdp_independ->fdp_seq_par.out_width;
		fdp_cb1->outsize.height = fdp_independ->fdp_seq_par.out_height;
	} else {
		fdp_cb1->outsize_flg = 0;
	}
	fdp_cb1->refwr_num = stlmsk_flg;
	fdp_cb1->refwr_y_en = fdp_independ->smw;
	fdp_cb1->refwr_c_en = fdp_independ->smw;
	fdp_cb1->refrd0_en = (fdp_independ->chact) & 0x1;
	fdp_cb1->refrd1_en = ((fdp_independ->chact)>>1) & 0x1;
	fdp_cb1->refrd2_en = ((fdp_independ->chact)>>2) & 0x1;
}

void fdpm_seq_control(T_FDP_R_START *start_par, struct fdpm_resource_table *resource_table)
{
	if (start_par->fproc_par_flg == 1) {
		if (start_par->fproc_par.seq_par_flg == 1) {
			resource_table->seq_status.seq_count = 0;
			resource_table->seq_status.seq_mode = start_par->fproc_par.seq_par.seq_mode;
			if (start_par->fproc_par.seq_par.seq_mode == FDP_SEQ_INTER)
				resource_table->seq_status.telecine_mode = start_par->fproc_par.seq_par.telecine_mode;
			else
				resource_table->seq_status.telecine_mode = 0;
		} else {
			if (resource_table->seq_status.seq_count == 0) {
				start_par->fproc_par.seq_par_flg = 2;
				start_par->fproc_par.seq_par.seq_mode = resource_table->seq_status.seq_mode;
				resource_table->seq_status.seq_count++;
			} else {
				start_par->fproc_par.seq_par_flg = 0;
				start_par->fproc_par.seq_par.seq_mode = resource_table->seq_status.seq_mode;
			}
		}
	}
}

void fdpm_stlmsk_control(T_FDP_R_START *start_par, struct fdp_independ *fdp_independ, struct fdpm_resource_table *target_table)
{
	int seq_mode;

	if (start_par->fproc_par.seq_par_flg == 1)
		seq_mode = start_par->fproc_par.seq_par.seq_mode;
	else
		seq_mode = target_table->seq_status.seq_mode;


	if ((seq_mode == FDP_SEQ_PROG) || (seq_mode == FDP_SEQ_INTER_2D) || (seq_mode == FDP_SEQ_INTERH_2D)) {
		fdp_independ->stlmsk_adr = 0;
		target_table->stlmsk_table.stlmsk_flg = 0;
	} else if (seq_mode == FDP_SEQ_INTER) {
		if (start_par->fproc_par.seq_par_flg == 1) {
			fdp_independ->stlmsk_adr = (unsigned long)target_table->stlmsk_table.stlmsk_adr[0];
			target_table->stlmsk_table.stlmsk_flg = 0;
		} else if (target_table->stlmsk_table.stlmsk_flg == 0) {
			fdp_independ->stlmsk_adr = (unsigned long)target_table->stlmsk_table.stlmsk_adr[1];
			target_table->stlmsk_table.stlmsk_flg = 1;
		} else {
			fdp_independ->stlmsk_adr = (unsigned long)target_table->stlmsk_table.stlmsk_adr[0];
			target_table->stlmsk_table.stlmsk_flg = 0;
		}
	} else if (seq_mode == FDP_SEQ_INTERH) {
		fdp_independ->stlmsk_adr = (unsigned long)target_table->stlmsk_table.stlmsk_adr[0];
		target_table->stlmsk_table.stlmsk_flg = 0;
	} else{
		fdp_independ->stlmsk_adr = 0;
		target_table->stlmsk_table.stlmsk_flg = 0;
	}
}

void fdpm_pd_control(T_FDP_R_START *start_par, struct fdpm_resource_table *resource_table)
{
	if (start_par->fproc_par_flg == 1) {
		if (start_par->fproc_par.seq_par_flg == 1) {
			resource_table->seq_status.decode_val = fdpm_dec_decodeinfo(&start_par->fproc_par.in_pic.pic_par);
			fdpm_decode_count_update(&resource_table->seq_status, 1);
			memcpy(&resource_table->seq_status.fdp_pic_par_pre, &resource_table->seq_status.fdp_pic_par, sizeof(T_FDPD_PICPAR));
		} else{
			if (fdpm_check_decodeinfo(resource_table->seq_status.fdp_pic_par, &start_par->fproc_par.in_pic.pic_par)) {
				resource_table->seq_status.decode_val = fdpm_dec_decodeinfo2(&resource_table->seq_status.fdp_pic_par);
				fdpm_decode_count_update(&resource_table->seq_status, 0);
				memcpy(&resource_table->seq_status.fdp_pic_par_pre, &resource_table->seq_status.fdp_pic_par, sizeof(T_FDPD_PICPAR));
			} else{
				resource_table->seq_status.decode_val = fdpm_dec_decodeinfo(&start_par->fproc_par.in_pic.pic_par);
				fdpm_decode_count_update(&resource_table->seq_status, 1);
				memcpy(&resource_table->seq_status.fdp_pic_par_pre, &resource_table->seq_status.fdp_pic_par, sizeof(T_FDPD_PICPAR));
			}
		}
	}
}

int fdpm_check_decodeinfo(T_FDPD_PICPAR previous_pic_par, T_FDP_R_PICPAR *current_pic_par)
{
	int ret0, ret1, ret2, ret3, ret4, ret;

	ret0 = (previous_pic_par.progressive_sequence ==
		current_pic_par->progressive_sequence) ? 1 : 0;
	ret1 = (previous_pic_par.progressive_frame ==
		current_pic_par->progressive_frame) ? 1 : 0;
	ret2 = (previous_pic_par.picture_structure ==
		current_pic_par->picture_structure) ? 1 : 0;
	ret3 = (previous_pic_par.repeat_first_field ==
		current_pic_par->repeat_first_field) ? 1 : 0;
	ret4 = (previous_pic_par.top_field_first ==
		current_pic_par->top_field_first) ? 1 : 0;

	ret = ret0 & ret1 & ret2 & ret3 & ret4;
	return ret;
}

int fdpm_dec_decodeinfo(T_FDP_R_PICPAR *pic_par)
{
	int progressive_sequence = pic_par->progressive_sequence;
	int progressive_frame = pic_par->progressive_frame;
	int picture_structure = pic_par->picture_structure;
	int repeat_first_field = pic_par->repeat_first_field;
	int top_field_first = pic_par->top_field_first;
	unsigned int chk_value;
	int decode_out;

	chk_value = (picture_structure<<2) | (repeat_first_field<<1) | (top_field_first);

	if (progressive_sequence == 1) {
		switch (chk_value) {
		case 0xc:
			decode_out = 1; break;
		case 0xe:
			decode_out = 2; break;
		case 0xf:
			decode_out = 3; break;
		default:
			decode_out = 0; break;
		}
	} else {
		if (progressive_frame == 0) {
			switch (picture_structure) {
			case 0x1:
				decode_out = 0x11; break;
			case 0x2:
				decode_out = 0x12; break;
			case 0x3:
				if (chk_value && 0x3 == 0) {
					decode_out = 0x13; break;
				} else if (chk_value && 0x3 == 1) {
					decode_out = 0x14; break;
				} else {
					decode_out = 0x10; break;
				}
			default:
				decode_out = 0x10; break;
			}
		} else {
			switch (chk_value) {
			case 0xc:
				decode_out = 0x15; break;
			case 0xd:
				decode_out = 0x16; break;
			case 0xe:
				decode_out = 0x17; break;
			case 0xf:
				decode_out = 0x18; break;
			default:
				decode_out = 0x10; break;
			}
		}
	}
	return decode_out;
}

int fdpm_dec_decodeinfo2(T_FDPD_PICPAR *pic_par)
{
	int progressive_sequence = pic_par->progressive_sequence;
	int progressive_frame = pic_par->progressive_frame;
	int picture_structure = pic_par->picture_structure;
	int repeat_first_field = pic_par->repeat_first_field;
	int top_field_first = pic_par->top_field_first;
	unsigned int chk_value;
	int decode_out;

	chk_value = (picture_structure<<2) | (repeat_first_field<<1) | (top_field_first);

	if (progressive_sequence == 1) {
		switch (chk_value) {
		case 0xc:
			decode_out = 1; break;
		case 0xe:
			decode_out = 2; break;
		case 0xf:
			decode_out = 3; break;
		default:
			decode_out = 0; break;
		}
	} else {
		if (progressive_frame == 0) {
			switch (picture_structure) {
			case 0x1:
				decode_out = 0x11; break;
			case 0x2:
				decode_out = 0x12; break;
			case 0x3:
				if (chk_value && 0x3 == 0) {
					decode_out = 0x13; break;
				} else if (chk_value && 0x3 == 1) {
					decode_out = 0x14; break;
				} else {
					decode_out = 0x10; break;
				}
			default:
				decode_out = 0x10; break;
			}
		} else {
			switch (chk_value) {
			case 0xc:
				decode_out = 0x15; break;
			case 0xd:
				decode_out = 0x16; break;
			case 0xe:
				decode_out = 0x17; break;
			case 0xf:
				decode_out = 0x18; break;
			default:
				decode_out = 0x10; break;
			}
		}
	}
	return decode_out;
}

void fdpm_decode_count_update(T_FDP_SEQ_STATUS *fdpm_seq_status, int cclr)
{
	int count_max;
	int decode_out = fdpm_seq_status->decode_val;
	switch (decode_out) {
	case 0x1:
		count_max = 0; break;
	case 0x2:
		count_max = 1; break;
	case 0x3:
		count_max = 2; break;
	case 0x11:
		count_max = 0; break;
	case 0x12:
		count_max = 0; break;
	case 0x13:
		count_max = 1; break;
	case 0x14:
		count_max = 1; break;
	case 0x15:
		count_max = 1; break;
	case 0x16:
		count_max = 1; break;
	case 0x17:
		count_max = 2; break;
	case 0x18:
		count_max = 2; break;
	default:
		count_max = 0; break;
	}
	fdpm_seq_status->count_max = count_max;
	if ((fdpm_seq_status->decode_count >= count_max) || (cclr == 1))
		fdpm_seq_status->decode_count = 0;
	else
		fdpm_seq_status->decode_count++;
}

void fdpm_update_picpar(struct fdpm_resource_table *resource_table, T_FDP_R_FPROC *fproc_par)
{
	if (fproc_par->in_pic_flg == 1) {
		if (fproc_par->in_pic.pic_par_flg == 1) {
			resource_table->seq_status.fdp_pic_par.width = fproc_par->in_pic.pic_par.width;
			resource_table->seq_status.fdp_pic_par.height = fproc_par->in_pic.pic_par.height;
			resource_table->seq_status.fdp_pic_par.chroma_format = fproc_par->in_pic.pic_par.chroma_format;
			resource_table->seq_status.fdp_pic_par.progressive_sequence = fproc_par->in_pic.pic_par.progressive_sequence;
			resource_table->seq_status.fdp_pic_par.progressive_frame = fproc_par->in_pic.pic_par.progressive_frame;
			resource_table->seq_status.fdp_pic_par.picture_structure = fproc_par->in_pic.pic_par.picture_structure;
			resource_table->seq_status.fdp_pic_par.repeat_first_field = fproc_par->in_pic.pic_par.repeat_first_field;
			resource_table->seq_status.fdp_pic_par.top_field_first = fproc_par->in_pic.pic_par.top_field_first;
		} else {
			memcpy(&resource_table->seq_status.fdp_pic_par, &resource_table->seq_status.fdp_pic_par_pre, sizeof(T_FDPD_PICPAR));
		}
	} else {
		memcpy(&resource_table->seq_status.fdp_pic_par, &resource_table->seq_status.fdp_pic_par_pre, sizeof(T_FDPD_PICPAR));
	}
}

void fdpm_seq_write(T_FDP_R_START *start_par,  struct fdpm_resource_table *target_table)
{
	if (start_par->fproc_par.seq_par_flg == 1) {
		target_table->seq_status.fdp_seq_par.seq_mode = start_par->fproc_par.seq_par.seq_mode;
		target_table->seq_status.fdp_seq_par.scale_mode = start_par->fproc_par.seq_par.scale_mode;
		target_table->seq_status.fdp_seq_par.filter_mode = start_par->fproc_par.seq_par.filter_mode;
		target_table->seq_status.fdp_seq_par.telecine_mode = start_par->fproc_par.seq_par.telecine_mode;
		target_table->seq_status.fdp_seq_par.in_width = start_par->fproc_par.seq_par.in_width;
		target_table->seq_status.fdp_seq_par.in_height = start_par->fproc_par.seq_par.in_height;
		target_table->seq_status.fdp_seq_par.imgleft = start_par->fproc_par.seq_par.imgleft;
		target_table->seq_status.fdp_seq_par.imgtop = start_par->fproc_par.seq_par.imgtop;
		target_table->seq_status.fdp_seq_par.imgwidth = start_par->fproc_par.seq_par.imgwidth;
		target_table->seq_status.fdp_seq_par.imgheight = start_par->fproc_par.seq_par.imgheight;
		target_table->seq_status.fdp_seq_par.out_width = start_par->fproc_par.seq_par.out_width;
		if (start_par->fproc_par.seq_par.seq_mode == FDP_SEQ_PROG)
			target_table->seq_status.fdp_seq_par.out_height = start_par->fproc_par.seq_par.in_height;
		else
			target_table->seq_status.fdp_seq_par.out_height = start_par->fproc_par.seq_par.in_height*2;
	} else {
		start_par->fproc_par.seq_par.seq_mode = target_table->seq_status.fdp_seq_par.seq_mode;
		start_par->fproc_par.seq_par.scale_mode = target_table->seq_status.fdp_seq_par.scale_mode;
		start_par->fproc_par.seq_par.filter_mode = target_table->seq_status.fdp_seq_par.filter_mode;
		start_par->fproc_par.seq_par.telecine_mode = target_table->seq_status.fdp_seq_par.telecine_mode;
		start_par->fproc_par.seq_par.in_width = target_table->seq_status.fdp_seq_par.in_width;
		start_par->fproc_par.seq_par.in_height = target_table->seq_status.fdp_seq_par.in_height;
		start_par->fproc_par.seq_par.imgleft = target_table->seq_status.fdp_seq_par.imgleft;
		start_par->fproc_par.seq_par.imgtop = target_table->seq_status.fdp_seq_par.imgtop;
		start_par->fproc_par.seq_par.imgwidth = target_table->seq_status.fdp_seq_par.imgwidth;
		start_par->fproc_par.seq_par.imgheight = target_table->seq_status.fdp_seq_par.imgheight;
		start_par->fproc_par.seq_par.out_width = target_table->seq_status.fdp_seq_par.out_width;
		start_par->fproc_par.seq_par.out_height = target_table->seq_status.fdp_seq_par.out_height;
	}
}
