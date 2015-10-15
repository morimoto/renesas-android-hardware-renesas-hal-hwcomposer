/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef __FDPM_DRV_H__
#define __FDPM_DRV_H__

enum {
	FDP_NOGO = 0,
	FDP_GO,
};

enum {
	FDP_OUT_NOREQ = 0,
	FDP_OUT_REQ,
};

enum {
	FDP_OUT_DISABLE = 0,
	FDP_OUT_ENABLE,
};

enum {
	ST_FDP_INI = 0,
	ST_FDP_RDY,
	ST_FDP_RUN,
	ST_FDP_BSY,
};

enum {
	FDP_YUV420 = 0,
	FDP_YUV420_YV12,
	FDP_YUV420_NV21,
	FDP_YUV422_NV16,
	FDP_YUV422_YUY2,
	FDP_YUV422_UYVY,
	FDP_RGB_332,
	FDP_RGBA_4444,
	FDP_RGBA_5551,
	FDP_RGB_565,
	FDP_RGB_888,
	FDP_RGBA_8888,
};

enum {
	FDP_SEQ_PROG = 0,
	FDP_SEQ_INTER,
	FDP_SEQ_INTERH,
	FDP_SEQ_INTER_2D,
	FDP_SEQ_INTERH_2D,
};

enum {
	FDP_TC_OFF = 0,
	FDP_TC_ON,
};

enum {
	FDPM_IDLE = 0,
	FDPM_RDY,
	FDPM_SHARE_BUSY,
	FDPM_OCCUPY_BUSY,
	FDPM_FULL_BUSY,
};

enum {
	FDP_SEQ_UNLOCK = 0,
	FDP_SEQ_LOCK,
};

enum {
	FDP_IN_DISABLE = 0,
	FDP_IN_ENABLE,
};

enum {
	FDP_OCMODE_OCCUPY = 0,
	FDP_OCMODE_COMMON,
};

enum {
	FDP_VMODE_NORMAL = 0,
	FDP_VMODE_VBEST,
	FDP_VMODE_VBEST_FDP0,
	FDP_VMODE_VBEST_FDP1,
	FDP_VMODE_VBEST_FDP2,
};

enum {
	FDP_CLKMODE_0 = 0,
	FDP_CLKMODE_1,
	FDP_CLKMODE_3 = 3,
};


enum {
	E_FDP_END = 0,
	E_FDP_DELAYED,
};

typedef void       (*FP)(void);

typedef struct {
	unsigned char    intlvl;
	void             (*pm_req)(long p1, long p2);
} T_FDP_INIT;

typedef struct {
	void             *addr;
	void             *addr_c0;
	void             *addr_c1;
	unsigned short   stride;
	unsigned short   stride_c;
	unsigned short   height;
	unsigned short   height_c;
}  T_FDP_IMGBUF;

typedef struct {
	T_FDP_IMGBUF    *buf_ref0;
	T_FDP_IMGBUF    *buf_ref1;
	T_FDP_IMGBUF    *buf_ref2;
	T_FDP_IMGBUF    *buf_refprg;
	void             *stlmsk_adr;
	T_FDP_IMGBUF    *buf_iir;
} T_FDP_REFPREBUF;

typedef struct {
	unsigned short   width;
	unsigned short   height;
} T_FDP_IMGSIZE;

typedef struct {
	T_FDP_IMGBUF    *buf_in;
	T_FDP_IMGSIZE   *insize;
	T_FDP_IMGSIZE   *refsize;
	T_FDP_IMGSIZE   *iirsize;
	T_FDP_IMGSIZE   *outsize;
	unsigned char    refwr_num;
	unsigned char    refrd0_num;
	unsigned char    refrd1_num;
	unsigned char    refrd2_num;
	unsigned char    refwr_y_en;
	unsigned char    refwr_c_en;
	unsigned char    refrd0_en;
	unsigned char    refrd1_en;
	unsigned char    refrd2_en;
	unsigned char    refiir_en;
	void             *userdata1;
} T_FDP_CB1;

typedef struct {
	int              ercd;
	void             *userdata2;
} T_FDP_CB2;

typedef struct {
	unsigned char    ref_mode;
	unsigned char    refbuf_mode;
	T_FDP_REFPREBUF *refbuf;
	unsigned char    ocmode;
	unsigned char    vmode;
	T_FDP_IMGSIZE   *insize;
	unsigned char    clkmode;
	unsigned long    vcnt;
} T_FDP_OPEN;

typedef struct {
	unsigned char    ratio_type;
	short            h_iniphase;
	short            h_endphase;
	unsigned short   h_ratio;
	short            v_iniphase;
	short            v_endphase;
	unsigned short   v_ratio;
} T_FDP_RATIO;

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
	T_FDP_RATIO     *ratio;
} T_FDP_SEQ;

typedef struct {
	unsigned char    dummy;
} T_FDP_IMGSET;

typedef struct {
	unsigned short   width;
	unsigned short   height;
	unsigned char    chroma_format;
	unsigned char    progressive_sequence;
	unsigned char    progressive_frame;
	unsigned char    picture_structure;
	unsigned char    repeat_first_field;
	unsigned char    top_field_first;
} T_FDP_PICPAR;

typedef struct {
	unsigned long    picid;
	T_FDP_PICPAR    *pic_par;
	T_FDP_IMGBUF    *in_buf1;
	T_FDP_IMGBUF    *in_buf2;
} T_FDP_PIC;

typedef struct {
	T_FDP_IMGBUF    *buf_refwr;
	T_FDP_IMGBUF    *buf_refrd0;
	T_FDP_IMGBUF    *buf_refrd1;
	T_FDP_IMGBUF    *buf_refrd2;
	T_FDP_IMGBUF    *buf_iirwr;
	T_FDP_IMGBUF    *buf_iirrd;
} T_FDP_REFBUF;

typedef struct {
	T_FDP_SEQ       *seq_par;
	T_FDP_IMGSET    *imgset_par;
	T_FDP_PIC       *in_pic;
	unsigned char    last_start;
	unsigned char    cf;
	unsigned char    f_decodeseq;
	T_FDP_IMGBUF    *out_buf;
	unsigned char    out_format;
	T_FDP_REFBUF    *ref_buf;
} T_FDP_FPROC;

typedef struct {
	unsigned long    *vcnt;
	unsigned char    fdpgo;
	T_FDP_FPROC     *fproc_par;
} T_FDP_START;

typedef struct {
	unsigned char    status;
	unsigned long    delay;
	unsigned long    vcycle;
	unsigned long    vintcnt;
	unsigned char    seq_lock;
	unsigned char    in_enable;
	unsigned long    in_picid;
	unsigned char    in_left;
	unsigned char    out_enable;
	unsigned long    out_picid;
	unsigned char    out_left;
	unsigned char    out_req;
} T_FDP_STATUS;

typedef struct {
	char name[30];
	unsigned long reg_value;
} T_FDP_REG_STATUS;

typedef struct {
	unsigned char status;
	unsigned long delay;
	unsigned long vcycle;
	unsigned long vintcnt;
} T_FDP_HW_STATUS;

#endif
