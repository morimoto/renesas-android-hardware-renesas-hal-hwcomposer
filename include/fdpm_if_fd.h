/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef __FDPM_IF_FD_H_
#define __FDPM_IF_FD_H_

#define FDPM_FD_HIST_LENGTH 60
#define FDPM_FD_FILM_MATCH_COUNT_32 10
#define FDPM_FD_FILM_MATCH_COUNT_22 (FDPM_FD_HIST_LENGTH - 3)
#define FDPM_FD_FILM_UNMATCH_COUNT_32 10
#define FDPM_FD_FILM_UNMATCH_COUNT_22 (FDPM_FD_HIST_LENGTH - 3)
#define FDPM_FD_MATCH32_LENGTH 5
#define FDPM_FD_MATCH22_LENGTH 2

/* #define FDPM_FD_FORCE_DET */

enum {
	FDPM_FD_IDLE = 0,
	FDPM_FD_VIDEO,
	FDPM_FD_FILM,
};

struct hist {
	unsigned int p[3];
	unsigned int n[3];
	unsigned int pn[3];
	unsigned int xx[3];
	unsigned int sf[3];
	unsigned int df[3];
};

struct rep_bin {
	unsigned char bin;
	unsigned char rff;
};

struct fd_pat {
	struct rep_bin previous_data[5];
};

struct match_hist {
	unsigned char match32[FDPM_FD_HIST_LENGTH];
	unsigned char match22[FDPM_FD_HIST_LENGTH];
};

struct film_detect_private {
	struct hist hist_data;
	unsigned char win[3];
	unsigned char fld[3];
	struct rep_bin cf_bin;
	unsigned char state;
	struct fd_pat fd_pattern;
	struct match_hist match_data;
	unsigned char detect_pat; /* 1: 3:2 2: 2:2 */
	unsigned char match_pattern;
	unsigned char match_count;
	unsigned char next_pattern; /* 3: previous 4:next */
};

int drv_FDPM_Status_fd(struct film_detect_private *fd_data, int *sub_ercd);
#endif
