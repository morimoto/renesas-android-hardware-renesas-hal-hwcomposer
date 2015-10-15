/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "fdpm_api.h"
#include "fdpm_if_par.h"
#include "fdpm_if_fd.h"
#include "fdpm_if_priv.h"
#include "fdpm_if.h"

void fdpm_mk_imgbuf_par(T_FDP_R_IMGBUF *out_par, T_FDP_IMGBUF *in_par)
{
	out_par->addr     = (unsigned long)in_par->addr;
	out_par->addr_c0  = (unsigned long)in_par->addr_c0;
	out_par->addr_c1  = (unsigned long)in_par->addr_c1;
	out_par->stride   = in_par->stride;
	out_par->stride_c = in_par->stride_c;
	out_par->height   = in_par->height;
	out_par->height_c = in_par->height_c;
}

void fdpm_mk_refprebuf_par(T_FDP_R_REFPREBUF *out_par, T_FDP_REFPREBUF *in_par)
{
	if (in_par->buf_ref0 != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_ref0, in_par->buf_ref0);
		out_par->buf_ref0_flg = 1;
	} else {
		out_par->buf_ref0_flg = 0;
		memset(&out_par->buf_ref0, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_ref1 != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_ref1, in_par->buf_ref1);
		out_par->buf_ref1_flg = 1;
	} else {
		out_par->buf_ref1_flg = 0;
		memset(&out_par->buf_ref1, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_ref2 != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_ref2, in_par->buf_ref2);
		out_par->buf_ref2_flg = 1;
	} else {
		out_par->buf_ref2_flg = 0;
		memset(&out_par->buf_ref2, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_refprg != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_refprg, in_par->buf_refprg);
		out_par->buf_refprg_flg = 1;
	} else {
		out_par->buf_refprg_flg = 0;
		memset(&out_par->buf_refprg, 0, sizeof(T_FDP_R_IMGBUF));
	}
	out_par->stlmsk_adr = (unsigned long)in_par->stlmsk_adr;
	if (in_par->buf_iir != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_iir, in_par->buf_iir);
		out_par->buf_iir_flg = 1;
	} else {
		out_par->buf_iir_flg = 0;
		memset(&out_par->buf_iir, 0, sizeof(T_FDP_R_IMGBUF));
	}
}

void fdpm_mk_open_par(T_FDP_R_OPEN *out_par, T_FDP_OPEN *in_par)
{
	out_par->ref_mode = in_par->ref_mode;
	out_par->refbuf_mode = in_par->refbuf_mode;
	if (in_par->refbuf != NULL) {
		fdpm_mk_refprebuf_par(&out_par->refbuf, in_par->refbuf);
		out_par->refbuf_flg = 1;
	} else {
		out_par->refbuf_flg = 0;
		memset(&out_par->refbuf, 0, sizeof(T_FDP_R_REFPREBUF));
	}
	out_par->ocmode = in_par->ocmode;
	out_par->vmode = in_par->vmode;
	out_par->insize.width = in_par->insize->width;
	out_par->insize.height = in_par->insize->height;
	out_par->clkmode = in_par->clkmode;
	out_par->vcnt = in_par->vcnt;
}

void fdpm_mk_ratio_par(T_FDP_R_RATIO *out_par, T_FDP_RATIO *in_par)
{
	out_par->ratio_type = in_par->ratio_type;
	out_par->h_iniphase = in_par->h_iniphase;
	out_par->h_endphase = in_par->h_endphase;
	out_par->h_ratio    = in_par->h_ratio;
	out_par->v_iniphase = in_par->v_iniphase;
	out_par->v_endphase = in_par->v_endphase;
	out_par->v_ratio    = in_par->v_ratio;
}

void fdpm_mk_picpar_par(T_FDP_R_PICPAR *out_par, T_FDP_PICPAR *in_par)
{
	out_par->width                = in_par->width;
	out_par->height               = in_par->height;
	out_par->chroma_format        = in_par->chroma_format;
	out_par->progressive_sequence = in_par->progressive_sequence;
	out_par->progressive_frame    = in_par->progressive_frame;
	out_par->picture_structure    = in_par->picture_structure;
	out_par->repeat_first_field   = in_par->repeat_first_field;
	out_par->top_field_first      = in_par->top_field_first;
}

void fdpm_mk_seq_par(T_FDP_R_SEQ *out_par, T_FDP_SEQ *in_par)
{
	out_par->seq_mode            = in_par->seq_mode;
	out_par->scale_mode          = in_par->scale_mode;
	out_par->filter_mode         = in_par->filter_mode;
	out_par->telecine_mode       = in_par->telecine_mode;
	out_par->in_width            = in_par->in_width;
	out_par->in_height           = in_par->in_height;
	out_par->imgleft             = in_par->imgleft;
	out_par->imgtop              = in_par->imgtop;
	out_par->imgwidth            = in_par->imgwidth;
	out_par->imgheight           = in_par->imgheight;
	out_par->out_width           = in_par->out_width;
	out_par->out_height          = in_par->out_height;
	out_par->ratio_flg = 0;
	memset(&out_par->ratio, 0, sizeof(T_FDP_R_RATIO));
}

void fdpm_mk_pic_par(T_FDP_R_PIC *out_par, T_FDP_PIC *in_par)
{
	out_par->picid = in_par->picid;
	if (in_par->pic_par != NULL) {
		fdpm_mk_picpar_par(&out_par->pic_par, in_par->pic_par);
		out_par->pic_par_flg = 1;
	} else {
		out_par->pic_par_flg = 0;
		memset(&out_par->pic_par, 0, sizeof(T_FDP_R_PICPAR));
	}
	out_par->in_buf1_flg = 0;
	memset(&out_par->in_buf1, 0, sizeof(T_FDP_R_IMGBUF));
	out_par->in_buf2_flg = 0;
	memset(&out_par->in_buf2, 0, sizeof(T_FDP_R_IMGBUF));
}

void fdpm_mk_refbuf_par(T_FDP_R_REFBUF *out_par, T_FDP_REFBUF *in_par)
{
	if (in_par->buf_refwr != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_refwr, in_par->buf_refwr);
		out_par->buf_refwr_flg = 1;
	} else {
		out_par->buf_refwr_flg = 0;
		memset(&out_par->buf_refwr, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_refrd0 != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_refrd0, in_par->buf_refrd0);
		out_par->buf_refrd0_flg = 1;
	} else {
		out_par->buf_refrd0_flg = 0;
		memset(&out_par->buf_refrd0, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_refrd1 != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_refrd1, in_par->buf_refrd1);
		out_par->buf_refrd1_flg = 1;
	} else {
		out_par->buf_refrd1_flg = 0;
		memset(&out_par->buf_refrd1, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_refrd2 != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_refrd2, in_par->buf_refrd2);
		out_par->buf_refrd2_flg = 1;
	} else {
		out_par->buf_refrd2_flg = 0;
		memset(&out_par->buf_refrd2, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_iirwr != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_iirwr, in_par->buf_iirwr);
		out_par->buf_iirwr_flg = 1;
	} else {
		out_par->buf_iirwr_flg = 0;
		memset(&out_par->buf_iirwr, 0, sizeof(T_FDP_R_IMGBUF));
	}
	if (in_par->buf_iirrd != NULL) {
		fdpm_mk_imgbuf_par(&out_par->buf_iirrd, in_par->buf_iirrd);
		out_par->buf_iirrd_flg = 1;
	} else {
		out_par->buf_iirrd_flg = 0;
		memset(&out_par->buf_iirrd, 0, sizeof(T_FDP_R_IMGBUF));
	}
}

void fdpm_mk_fproc_par(T_FDP_R_FPROC *out_par, T_FDP_FPROC *in_par)
{
	if (in_par->seq_par != NULL) {
		fdpm_mk_seq_par(&out_par->seq_par, in_par->seq_par);
		out_par->seq_par_flg = 1;
	} else {
		out_par->seq_par_flg = 0;
		memset(&out_par->seq_par, 0, sizeof(T_FDP_R_SEQ));
	}
	if (in_par->imgset_par != NULL) {
		out_par->imgset_par.dummy = in_par->imgset_par->dummy;
		out_par->imgset_par_flg = 1;
	} else {
		out_par->imgset_par_flg = 0;
		out_par->imgset_par.dummy = 0;
	}
	if (in_par->in_pic != NULL) {
		fdpm_mk_pic_par(&out_par->in_pic, in_par->in_pic);
		out_par->in_pic_flg = 1;
	} else {
		out_par->in_pic_flg = 0;
		memset(&out_par->in_pic, 0, sizeof(T_FDP_R_PIC));
	}
	out_par->last_start  = in_par->last_start;
	out_par->cf          = in_par->cf;
	out_par->f_decodeseq = in_par->f_decodeseq;
	if (in_par->out_buf != NULL) {
		fdpm_mk_imgbuf_par(&out_par->out_buf, in_par->out_buf);
		out_par->out_buf_flg = 1;
	} else {
		out_par->out_buf_flg = 0;
		memset(&out_par->out_buf, 0, sizeof(T_FDP_R_IMGBUF));
	}
	out_par->out_format  = in_par->out_format;
	if (in_par->ref_buf != NULL) {
		fdpm_mk_refbuf_par(&out_par->ref_buf, in_par->ref_buf);
		out_par->ref_buf_flg = 1;
	} else {
		out_par->ref_buf_flg = 0;
		memset(&out_par->ref_buf, 0, sizeof(T_FDP_R_REFBUF));
	}
}

void fdpm_mk_start_par(T_FDP_R_START *out_par, struct film_detect_private *fd_data, T_FDP_START *in_par)
{
	if (in_par->vcnt != NULL) {
		out_par->vcnt = *in_par->vcnt;
		out_par->vcnt_flg = 1;
	} else {
		out_par->vcnt_flg = 0;
		out_par->vcnt = 0;
	}
	out_par->fdpgo = in_par->fdpgo;
	out_par->telecine_flg = fd_data->state;
	out_par->next_pattern = fd_data->next_pattern;
	if (in_par->fproc_par != NULL) {
		fdpm_mk_fproc_par(&out_par->fproc_par, in_par->fproc_par);
		out_par->fproc_par_flg = 1;
	} else {
		out_par->fproc_par_flg = 0;
		memset(&out_par->fproc_par, 0, sizeof(T_FDP_R_FPROC));
	}
}

void fdpm_mk_fdp_cb1(T_FDP_CB1 *userdata1, T_FDP_R_CB1 *fdp_cb1, void *userdata)
{
	userdata1->buf_in = (T_FDP_IMGBUF *)NULL;
	if (fdp_cb1->insize_flg == 1) {
		userdata1->insize = (T_FDP_IMGSIZE *)(&(fdp_cb1->insize));
		userdata1->refsize = (T_FDP_IMGSIZE *)(&(fdp_cb1->refsize));
	} else {
		userdata1->insize = NULL;
		userdata1->refsize = NULL;
	}
	userdata1->iirsize = NULL;
	if (fdp_cb1->outsize_flg == 1)
		userdata1->outsize = (T_FDP_IMGSIZE *)(&(fdp_cb1->outsize));
	else
		userdata1->outsize = NULL;
	userdata1->refwr_num = fdp_cb1->refwr_num;
	userdata1->refrd0_num = 0;
	userdata1->refrd1_num = 0;
	userdata1->refrd2_num = 0;
	userdata1->refwr_y_en = fdp_cb1->refwr_y_en;
	userdata1->refwr_c_en = fdp_cb1->refwr_c_en;
	userdata1->refrd0_en = fdp_cb1->refrd0_en;
	userdata1->refrd1_en = fdp_cb1->refrd1_en;
	userdata1->refrd2_en = fdp_cb1->refrd2_en;
	userdata1->refiir_en = 0;
	userdata1->userdata1 = userdata;
}

int fdpm_check_open_par(T_FDP_OPEN *open_par, int *result_sub_ercd)
{
	int ret = 0;

	if (open_par == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_OPENPAR;
		goto exit;
	}
	if (open_par->refbuf != NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_REFBUF;
		goto exit;
	}
	if ((open_par->ocmode != FDP_OCMODE_OCCUPY) && (open_par->ocmode != FDP_OCMODE_COMMON)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_OCMODE;
		goto exit;
	}
	if ((open_par->vmode != FDP_VMODE_NORMAL) && (open_par->vmode != FDP_VMODE_VBEST) && (open_par->vmode != FDP_VMODE_VBEST_FDP0) && (open_par->vmode != FDP_VMODE_VBEST_FDP1) && (open_par->vmode != FDP_VMODE_VBEST_FDP2)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_VMODE;
		goto exit;
	}
	if (open_par->insize == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_INSIZE;
		goto exit;
	}
	if ((open_par->insize->width < 80) || (open_par->insize->width > 1920) || (open_par->insize->width%2)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_INSIZE;
		goto exit;
	}
	if ((open_par->insize->height < 40) || (open_par->insize->height > 1920)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_INSIZE;
		goto exit;
	}
	if ((open_par->vcnt < 160) || (open_par->vcnt > 700)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_VCNT;
		goto exit;
	}
exit:
	return ret;
}

int fdpm_check_open_func(void *callback2, void *callback3, void *callback4, T_FDP_OPEN *open_par, int *result_sub_ercd)
{
	int ret = 0;

	if (fdpm_check_open_par(open_par, result_sub_ercd) == -1) {
		ret = -1;
		goto exit;
	}
	if (((open_par->vmode == FDP_VMODE_NORMAL) && (callback2 == NULL)) ||
	((open_par->vmode != FDP_VMODE_NORMAL) && (callback2 != NULL))) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_CB2;
		goto exit;
	}
	if ((open_par->vmode != FDP_VMODE_NORMAL) && (callback3 != NULL)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_CB3;
		goto exit;
	}
exit:
	return ret;
}

int fdpm_check_seq_par(T_FDP_SEQ *seq_par, int *result_sub_ercd)
{
	int ret = 0;
	if (seq_par != NULL) {
		if ((seq_par->seq_mode != FDP_SEQ_PROG) && (seq_par->seq_mode != FDP_SEQ_INTER) &&
		    (seq_par->seq_mode != FDP_SEQ_INTERH) && (seq_par->seq_mode != FDP_SEQ_INTER_2D) &&
		    (seq_par->seq_mode != FDP_SEQ_INTERH_2D)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_SEQMODE;
			goto exit;
		}
		if ((seq_par->telecine_mode != FDP_TC_OFF) && (seq_par->telecine_mode != FDP_TC_ON)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_TELECINEMODE;
			goto exit;
		}
		if ((seq_par->in_width < 80) || (seq_par->in_width > 1920)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_INWIDTH;
			goto exit;
		}
		if (seq_par->seq_mode == FDP_SEQ_PROG) {
			if ((seq_par->in_height < 80) || (seq_par->in_height > 1920) || (seq_par->in_height%2)) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_INHEIGHT;
				goto exit;
			}
		} else {
			if ((seq_par->in_height < 40) || (seq_par->in_height > 960)) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_INHEIGHT;
				goto exit;
			}
		}
	}
exit:
	return ret;
}

int fdpm_check_pic_par(T_FDP_FPROC *fproc_par, int *result_sub_ercd)
{
	int ret = 0;
	T_FDP_PICPAR *pic_par = fproc_par->in_pic->pic_par;

	if (pic_par == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_PICPAR;
		goto exit;
	}
	if ((pic_par->chroma_format != FDP_YUV420) && (pic_par->chroma_format != FDP_YUV420_YV12) &&
	    (pic_par->chroma_format != FDP_YUV420_NV21) && (pic_par->chroma_format != FDP_YUV422_NV16) &&
	    (pic_par->chroma_format != FDP_YUV422_YUY2)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_CHROMA;
		goto exit;
	}
	if (fproc_par->f_decodeseq == 1) {
		if ((pic_par->progressive_sequence != 0) && (pic_par->progressive_sequence != 1)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_PROGSEQ;
			goto exit;
		}
		if (fproc_par->seq_par != NULL) {
			if ((fproc_par->seq_par->seq_mode == FDP_SEQ_PROG) && (pic_par->progressive_sequence != 1)) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_PROGSEQ;
				goto exit;
			}
			if ((fproc_par->seq_par->seq_mode != FDP_SEQ_PROG) && (pic_par->progressive_sequence != 0)) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_PROGSEQ;
				goto exit;
			}
		}
		if ((pic_par->picture_structure != 0) && (pic_par->picture_structure != 1) &&
		    (pic_par->picture_structure != 2) && (pic_par->picture_structure != 3)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_PICSTRUCT;
			goto exit;
		}
		if ((pic_par->repeat_first_field != 0) && (pic_par->repeat_first_field != 1)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_REPEATTOP;
			goto exit;
		}
		if ((pic_par->top_field_first != 0) && (pic_par->top_field_first != 1)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_REPEATTOP;
			goto exit;
		}
		if (pic_par->progressive_sequence == 1) {
			if (pic_par->picture_structure != 3) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_REPEATTOP;
				goto exit;
			}
			if ((pic_par->picture_structure == 3) && (pic_par->repeat_first_field == 0) && (pic_par->top_field_first == 1)) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_REPEATTOP;
				goto exit;
			}
		}
		if (pic_par->progressive_sequence == 0) {
			if (pic_par->picture_structure == 0) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_REPEATTOP;
				goto exit;
			}
			if ((pic_par->progressive_frame == 1) && (pic_par->picture_structure != 3)) {
				ret = -1;
				*result_sub_ercd = E_FDP_PARA_REPEATTOP;
				goto exit;
			}
		}
	}
exit:
	return ret;
}

int fdpm_check_in_pic(T_FDP_FPROC *fproc_par, int *result_sub_ercd)
{
	int ret = 0;
	T_FDP_PIC *in_pic = fproc_par->in_pic;
	if (in_pic == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_INPIC;
		goto exit;
	}
	if (fdpm_check_pic_par(fproc_par, result_sub_ercd) == -1) {
		ret = -1;
		goto exit;
	}
exit:
	return ret;
}

int fdpm_check_refbuf(T_FDP_FPROC *fproc_par, int *result_sub_ercd)
{
	int ret = 0;
	T_FDP_REFBUF *ref_buf = fproc_par->ref_buf;

	if (ref_buf == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_REFBUF;
		goto exit;
	}
	if (ref_buf->buf_refrd1 == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_BUFREFRD1;
		goto exit;
	}
exit:
	return ret;
}

int fdpm_check_fproc_par(T_FDP_FPROC *fproc_par, int *result_sub_ercd)
{
	int ret = 0;
	if (fproc_par != NULL) {
		if (fdpm_check_seq_par(fproc_par->seq_par, result_sub_ercd) == -1) {
			ret = -1;
			goto exit;
		}
		if (fdpm_check_in_pic(fproc_par, result_sub_ercd) == -1) {
			ret = -1;
			goto exit;
		}
		if ((fproc_par->last_start != 0) && (fproc_par->last_start != 1)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_LASTSTART;
			goto exit;
		}
		if ((fproc_par->cf != 0) && (fproc_par->cf != 1)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_CF;
			goto exit;
		}
		if ((fproc_par->f_decodeseq != 0) && (fproc_par->f_decodeseq != 1)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_FDECODE;
			goto exit;
		}
		if ((fproc_par->out_format != FDP_YUV420) && (fproc_par->out_format != FDP_YUV420_YV12) &&
		    (fproc_par->out_format != FDP_YUV420_NV21) && (fproc_par->out_format != FDP_YUV422_NV16) &&
		    (fproc_par->out_format != FDP_YUV422_YUY2) && (fproc_par->out_format != FDP_YUV422_UYVY) &&
		    (fproc_par->out_format != FDP_RGB_332) && (fproc_par->out_format != FDP_RGBA_4444) &&
		    (fproc_par->out_format != FDP_RGBA_5551) && (fproc_par->out_format != FDP_RGB_565) &&
		    (fproc_par->out_format != FDP_RGB_888) && (fproc_par->out_format != FDP_RGBA_8888)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_OUTFORMAT;
			goto exit;
		}
		if (fdpm_check_refbuf(fproc_par, result_sub_ercd) == -1) {
			ret = -1;
			goto exit;
		}
	} else {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_FPROCPAR;
		goto exit;
	}
exit:
	return ret;
}

int fdpm_check_start_par(T_FDP_START *start_par, int *result_sub_ercd)
{
	int ret = 0;

	if (start_par == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_STARTPAR;
		goto exit;
	}
	if (start_par->vcnt != NULL) {
		if ((*(start_par->vcnt) < 160) || (*(start_par->vcnt) > 700)) {
			ret = -1;
			*result_sub_ercd = E_FDP_PARA_VCNT;
			goto exit;
		}
	}
	if ((start_par->fdpgo != FDP_NOGO) && (start_par->fdpgo != FDP_GO)) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_FDPGO;
		goto exit;
	}
	if (start_par->fdpgo == FDP_GO) {
		if (fdpm_check_fproc_par(start_par->fproc_par, result_sub_ercd) == -1) {
			ret = -1;
			goto exit;
		}
	}
exit:
	return ret;
}

int fdpm_check_start_func(void *callback1, void *callback2, T_FDP_START *start_par, int *result_sub_ercd)
{
	int ret = 0;

	if (callback1 == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_CB1;
		goto exit;
	}
	if (callback2 == NULL) {
		ret = -1;
		*result_sub_ercd = E_FDP_PARA_CB2;
		goto exit;
	}
	if (fdpm_check_start_par(start_par, result_sub_ercd) == -1) {
		ret = -1;
		goto exit;
	}
exit:
	return ret;
}
