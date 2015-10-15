/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#define LOG_TAG "FDPM"
/* #include <utils/Log.h> */

#include "fdpm_api.h"
#include "fdpm_if_par.h"
#include "fdpm_if_fd.h"

/* --------------------- */
unsigned char fdpm_winner_bin(int area, struct hist *cf_hist)
{
	int winner;
	unsigned int comp[4];
	unsigned char bin[4] = {0, 1, 2, 3};
	unsigned int min;
	unsigned char min_bin;
	int i, j, mini;
	unsigned int total_value = 0;

	comp[0] = cf_hist->xx[area];
	comp[1] = cf_hist->p[area];
	comp[2] = cf_hist->n[area];
	comp[3] = cf_hist->pn[area];

	for (i = 0; i < 4; i++)
		total_value = total_value + comp[i];

	/* small order sort */
	for (i = 0; i < 4; i++) {
		min = comp[i];
		min_bin = bin[i];
		mini = i;
		for (j = i+1; j < 4; j++) {
			if (comp[j] < min) {
				min = comp[j];
				min_bin = bin[j];
				mini = j;
			}
		}
		comp[mini] = comp[i];
		bin[mini] = bin[i];
		comp[i] = min;
		bin[i] = min_bin;
	}

	winner = bin[3];

	return winner;
}

int check_pattern(struct fd_pat *fd_pattern, struct rep_bin *target_pattern, int length)
{
	int result = 1;
	int i;

	for (i = 0; i < length; i++) {
		if ((fd_pattern->previous_data[i].bin != target_pattern[i].bin) ||
		    (fd_pattern->previous_data[i].rff != target_pattern[i].rff)) {
			result = 0;
			break;
		}
	}
	return result;
}

int fdpm_obtain_rep_bin(T_FDP_R_SENSOR_INFO *sinfo, struct film_detect_private *fd_data)
{
	unsigned char *win = fd_data->win;
	unsigned char *fld = fd_data->fld;
	struct hist *cf_hist = &fd_data->hist_data;
	struct rep_bin *cf_bin = &fd_data->cf_bin;
	int i;

	cf_hist->p[0] = sinfo->sensor_info[0];
	cf_hist->p[1] = sinfo->sensor_info[1];
	cf_hist->p[2] = sinfo->sensor_info[2];
	cf_hist->n[0] = sinfo->sensor_info[3];
	cf_hist->n[1] = sinfo->sensor_info[4];
	cf_hist->n[2] = sinfo->sensor_info[5];
	cf_hist->pn[0] = sinfo->sensor_info[6];
	cf_hist->pn[1] = sinfo->sensor_info[7];
	cf_hist->pn[2] = sinfo->sensor_info[8];
	cf_hist->xx[0] = sinfo->sensor_info[9];
	cf_hist->xx[1] = sinfo->sensor_info[10];
	cf_hist->xx[2] = sinfo->sensor_info[11];
	cf_hist->sf[0] = sinfo->sensor_info[12];
	cf_hist->sf[1] = sinfo->sensor_info[13];
	cf_hist->sf[2] = sinfo->sensor_info[14];
	cf_hist->df[0] = sinfo->sensor_info[15];
	cf_hist->df[1] = sinfo->sensor_info[16];
	cf_hist->df[2] = sinfo->sensor_info[17];

	for (i = 0; i < 3; i++) {
		win[i] = fdpm_winner_bin(i, cf_hist);
		if (cf_hist->sf[i] > cf_hist->df[i])
			fld[i] = 1;
		else
			fld[i] = 0;
	}

	if ((win[0] == win[1]) && (win[0] == win[2]))
		cf_bin->bin = win[0];
	else
		cf_bin->bin = 0; /* decide xx */
	if ((fld[0] == 1) && (fld[0] == fld[1]) && (fld[0] == fld[2]))
		cf_bin->rff = 1;
	else
		cf_bin->rff = 0;
	return 0;
}

int fdpm_film_det(struct film_detect_private *fd_data)
{
	struct rep_bin pattern_32[FDPM_FD_MATCH32_LENGTH][FDPM_FD_MATCH32_LENGTH];
	struct rep_bin pattern_22[FDPM_FD_MATCH22_LENGTH][FDPM_FD_MATCH22_LENGTH];
	struct fd_pat *fd_pattern = &fd_data->fd_pattern;
	struct match_hist *match_data = &fd_data->match_data;
	int i;
	unsigned char check_result32;
	unsigned char check_result22;
	unsigned char match_pattern32;
	unsigned char match_pattern22;
	unsigned char match_count32;
	unsigned char match_count22;

	pattern_32[0][0].bin = 2; pattern_32[0][0].rff = 0;
	pattern_32[0][1].bin = 3; pattern_32[0][1].rff = 1;
	pattern_32[0][2].bin = 1; pattern_32[0][2].rff = 0;
	pattern_32[0][3].bin = 2; pattern_32[0][3].rff = 0;
	pattern_32[0][4].bin = 1; pattern_32[0][4].rff = 0;

	pattern_32[1][0].bin = 3; pattern_32[1][0].rff = 1;
	pattern_32[1][1].bin = 1; pattern_32[1][1].rff = 0;
	pattern_32[1][2].bin = 2; pattern_32[1][2].rff = 0;
	pattern_32[1][3].bin = 1; pattern_32[1][3].rff = 0;
	pattern_32[1][4].bin = 2; pattern_32[1][4].rff = 0;

	pattern_32[2][0].bin = 1; pattern_32[2][0].rff = 0;
	pattern_32[2][1].bin = 2; pattern_32[2][1].rff = 0;
	pattern_32[2][2].bin = 1; pattern_32[2][2].rff = 0;
	pattern_32[2][3].bin = 2; pattern_32[2][3].rff = 0;
	pattern_32[2][4].bin = 3; pattern_32[2][4].rff = 1;

	pattern_32[3][0].bin = 2; pattern_32[3][0].rff = 0;
	pattern_32[3][1].bin = 1; pattern_32[3][1].rff = 0;
	pattern_32[3][2].bin = 2; pattern_32[3][2].rff = 0;
	pattern_32[3][3].bin = 3; pattern_32[3][3].rff = 1;
	pattern_32[3][4].bin = 1; pattern_32[3][4].rff = 0;

	pattern_32[4][0].bin = 1; pattern_32[4][0].rff = 0;
	pattern_32[4][1].bin = 2; pattern_32[4][1].rff = 0;
	pattern_32[4][2].bin = 3; pattern_32[4][2].rff = 1;
	pattern_32[4][3].bin = 1; pattern_32[4][3].rff = 0;
	pattern_32[4][4].bin = 2; pattern_32[4][4].rff = 0;

	pattern_22[0][0].bin = 2; pattern_22[0][0].rff = 0;
	pattern_22[0][1].bin = 1; pattern_22[0][1].rff = 0;

	pattern_22[1][0].bin = 1; pattern_22[1][0].rff = 0;
	pattern_22[1][1].bin = 2; pattern_22[1][1].rff = 0;

	for (i = 1; i < FDPM_FD_MATCH32_LENGTH; i++) {
		fd_pattern->previous_data[i-1].bin = fd_pattern->previous_data[i].bin;
		fd_pattern->previous_data[i-1].rff = fd_pattern->previous_data[i].rff;
	}
	fd_pattern->previous_data[4].bin = fd_data->cf_bin.bin;
	fd_pattern->previous_data[4].rff = fd_data->cf_bin.rff;

	check_result32 = 0;
	match_pattern32 = 0;
	for (i = 0; i < FDPM_FD_MATCH32_LENGTH; i++) {
		if (check_pattern(fd_pattern, pattern_32[i], FDPM_FD_MATCH32_LENGTH) == 1) {
			check_result32 = 1;
			match_pattern32 = i;
			break;
		}
	}
	check_result22 = 0;
	match_pattern22 = 0;
	for (i = FDPM_FD_MATCH32_LENGTH - FDPM_FD_MATCH22_LENGTH; i < FDPM_FD_MATCH32_LENGTH; i++) {
		if (check_pattern(fd_pattern, pattern_22[i-3], FDPM_FD_MATCH22_LENGTH) == 1) {
			check_result22 = 1;
			match_pattern22 = i - (FDPM_FD_MATCH32_LENGTH - FDPM_FD_MATCH22_LENGTH);
			break;
		}
	}
	match_count32 = 0;
	match_count22 = 0;
	for (i = 1; i < FDPM_FD_HIST_LENGTH; i++) {
		if (match_data->match32[i] == 1)
			match_count32++;
		if (match_data->match22[i] == 1)
			match_count22++;
		match_data->match32[i-1] = match_data->match32[i];
		match_data->match22[i-1] = match_data->match22[i];
	}
	if (check_result32 == 1)
		match_count32++;
	if (check_result22 == 1)
		match_count22++;
	match_data->match32[FDPM_FD_HIST_LENGTH-1] = check_result32;
	match_data->match22[FDPM_FD_HIST_LENGTH-1] = check_result22;

	switch (fd_data->state) {
	case FDPM_FD_VIDEO:
		if (match_count32 > FDPM_FD_FILM_MATCH_COUNT_32) {
			fd_data->state = FDPM_FD_FILM;
			fd_data->detect_pat = 1;
			fd_data->match_pattern = match_pattern32;
			fd_data->match_count = 0;
		} else if (match_count22 > FDPM_FD_FILM_MATCH_COUNT_22) {
			fd_data->state = FDPM_FD_FILM;
			fd_data->detect_pat = 2;
			fd_data->match_pattern = match_pattern22;
			fd_data->match_count = 0;
		}
		break;
	case FDPM_FD_FILM:
		if (fd_data->detect_pat == 1) {
			if (match_count32 < FDPM_FD_FILM_UNMATCH_COUNT_32) {
#ifdef FDPM_FD_FORCE_DET
				fd_data->state = FDPM_FD_FILM;
#else
				fd_data->state = FDPM_FD_VIDEO;
				fd_data->detect_pat = 0;
				fd_data->match_pattern = 0;
				fd_data->match_count = 0;
#endif
			}
		} else if (fd_data->detect_pat == 2) {
			if (match_count22 < FDPM_FD_FILM_UNMATCH_COUNT_22) {
#ifdef FDPM_FD_FORCE_DET
				fd_data->state = FDPM_FD_FILM;
#else
				fd_data->state = FDPM_FD_VIDEO;
				fd_data->detect_pat = 0;
				fd_data->match_pattern = 0;
				fd_data->match_count = 0;
#endif
			}
		}
		break;
	default:
		fd_data->state = FDPM_FD_VIDEO;
		fd_data->detect_pat = 0;
		fd_data->match_pattern = 0;
		fd_data->match_count = 0;
		break;
	}

	if (fd_data->state == FDPM_FD_FILM) {
		if (fd_data->detect_pat == 1) { /* 3:2 sequence */
			switch (pattern_32[fd_data->match_pattern][fd_data->match_count].bin) {
			case 1: /* p */
				fd_data->next_pattern = 3;
				if (fd_data->match_count == FDPM_FD_MATCH32_LENGTH - 1)
					fd_data->match_count = 0;
				else
					fd_data->match_count++;
				break;
			case 2: /* n */
				fd_data->next_pattern = 4;
				if (fd_data->match_count == FDPM_FD_MATCH32_LENGTH - 1)
					fd_data->match_count = 0;
				else
					fd_data->match_count++;
				break;
			case 3: /* pn */
				fd_data->next_pattern = 3;
				if (fd_data->match_count == FDPM_FD_MATCH32_LENGTH - 1)
					fd_data->match_count = 0;
				else
					fd_data->match_count++;
				break;
			default:
				break;
			}
		} else if (fd_data->detect_pat == 2) { /* 2:2 sequence */
			switch (pattern_22[fd_data->match_pattern][fd_data->match_count].bin) {
			case 1: /* p */
				fd_data->next_pattern = 4;
				if (fd_data->match_count == FDPM_FD_MATCH22_LENGTH - 1)
					fd_data->match_count = 0;
				else
					fd_data->match_count++;
				break;
			case 2: /* n */
				fd_data->next_pattern = 3;
				if (fd_data->match_count == FDPM_FD_MATCH22_LENGTH - 1)
					fd_data->match_count = 0;
				else
					fd_data->match_count++;
				break;
			default:
				break;
			}
		}
	} else {
		fd_data->next_pattern = 0;
		fd_data->match_count = 0;
	}
	return 0;
}

int fdpm_if_fd_func(T_FDP_R_SENSOR_INFO *sinfo, struct film_detect_private *fd_data)
{
	fdpm_obtain_rep_bin(sinfo, fd_data);
	fdpm_film_det(fd_data);
	return 0;
}
/*---------------------------------*/
