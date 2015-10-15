/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef __FDPM_API_H__
#define __FDPM_API_H__

#include "fdpm_public.h"
#include "fdpm_drv.h"

/* FDP Manager open */
int drv_FDPM_Open(void *callback2, void *callback3, void *callback4, T_FDP_OPEN *open_par, FP user_function, void *userdata2, void *userdata3, void *userdata4, int *sub_ercd);
/* FDP Manager close */
int drv_FDPM_Close(FP user_function, int *sub_ercd, unsigned char f_release);
/* FDP Manager Entry Start */
int drv_FDPM_Start(void *callback1, void *callback2, T_FDP_START *start_par, void *userdata1, void *userdata2, int *sub_ercd);
/* FDP Manager Entry Cancel */
int drv_FDPM_Cancel(int id, int *sub_ercd);
/* FDP Manager Get Status */
int drv_FDPM_Status(T_FDP_STATUS *fdp_status, int *sub_ercd);

int drv_FDPM_Status_reg(T_FDP_REG_STATUS *fdp_status, int *sub_ercd);
#endif
