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
#include <pthread.h>
#include <semaphore.h>
#include <sys/errno.h>
#include <signal.h>
#define LOG_TAG "FDPM"
/* #include <utils/Log.h> */

#include "fdpm_api.h"
#include "fdpm_if_par.h"
#include "fdpm_if_fd.h"
#include "fdpm_if_priv.h"
/* --------------------- */
void fdpm_timer_handle(union sigval sv);

/* timer create(open) */
int fdpm_timer_open(unsigned char timer_ch, int cb_no, struct fdpm_if_data *idata)
{
	int ret = 0;
	int icb_no;
	icb_no = cb_no & 0x1;

	struct fdpm_timerarg *fdpm_timer_arg;

	fdpm_timer_arg = malloc(sizeof(struct fdpm_timerarg));
	fdpm_timer_arg->idata = idata;
	fdpm_timer_arg->icb_no = icb_no;
	idata->fdpm_timer_arg[icb_no] = fdpm_timer_arg;

	idata->timer_table[icb_no].se.sigev_value.sival_ptr = fdpm_timer_arg;
	idata->timer_table[icb_no].se.sigev_notify = SIGEV_THREAD;
	idata->timer_table[icb_no].se.sigev_notify_function = fdpm_timer_handle;
	idata->timer_table[icb_no].se.sigev_notify_attributes = NULL;
	idata->timer_table[icb_no].se.sigev_signo  = SIGRTMIN + timer_ch + (cb_no%32);

	if (timer_create(CLOCK_MONOTONIC, &idata->timer_table[icb_no].se, &idata->timer_table[icb_no].timer_id) == -1) {
		printf("timer_create failed\n");
		ret =  -EFAULT;
		goto exit;
	}
	idata->timer_table[icb_no].use = 1;
exit:
	return ret;
}

/* timer set */
int fdpm_timer_set(int cb_no, int extend, struct fdpm_if_data *idata)
{
	int ret = 0;
	struct itimerspec itval2, itval_tmp;
	int icb_no;

	icb_no = cb_no & 0x1;

	timer_gettime(idata->timer_table[icb_no].timer_id, &itval_tmp);
	if ((extend == 1) ||
	    ((itval_tmp.it_value.tv_sec == 0) && (itval_tmp.it_value.tv_nsec == 0))) {
		itval2.it_value.tv_sec = idata->timer_table[icb_no].itval.it_value.tv_sec;
		itval2.it_value.tv_nsec = idata->timer_table[icb_no].itval.it_value.tv_nsec;
	} else {
		itval2.it_value.tv_sec = itval_tmp.it_value.tv_sec;
		itval2.it_value.tv_nsec = itval_tmp.it_value.tv_nsec;
	}
	itval2.it_interval.tv_sec = idata->timer_table[icb_no].itval.it_interval.tv_sec;
	itval2.it_interval.tv_nsec = idata->timer_table[icb_no].itval.it_interval.tv_nsec;
	if (timer_settime(idata->timer_table[icb_no].timer_id, 0, &itval2, NULL) == -1)
		ret = -EFAULT;
	return ret;
}

int fdpm_timer_set_start(unsigned long vcnt, int cb_no, int extend, int interval, struct fdpm_if_data *idata)
{
	unsigned int vint_nsec;
	unsigned int vint_sec;
	int ret = 0;
	int icb_no;

	icb_no = cb_no & 0x1;

	if (vcnt == 0) {
		ret = -EFAULT;
		ERRPRINT("vcnt value is illegal\n");
		goto exit;
	}

	vint_sec = vcnt/10000;
	vint_nsec = vcnt - (vint_sec * 10000);
	vint_nsec = vint_nsec * 100000;

	if (extend == 1) {
		idata->timer_table[icb_no].itval.it_value.tv_sec = vint_sec;
		idata->timer_table[icb_no].itval.it_value.tv_nsec = vint_nsec;
	} else {
		idata->timer_table[icb_no].itval.it_value.tv_sec = vint_sec;
		idata->timer_table[icb_no].itval.it_value.tv_nsec = vint_nsec;
	}
	if (interval == 1) {
		idata->timer_table[icb_no].itval.it_interval.tv_sec = vint_sec;
		idata->timer_table[icb_no].itval.it_interval.tv_nsec = vint_nsec;
	} else {
		idata->timer_table[icb_no].itval.it_interval.tv_sec = 0;
		idata->timer_table[icb_no].itval.it_interval.tv_nsec = 0;
	}
	ret = fdpm_timer_set(cb_no, extend, idata);

exit:
	return ret;
}

int fdpm_timer_get(int cb_no, struct fdpm_if_data *idata)
{
	double ot1, ot2;
	struct itimerspec     itval_tmp;
	int icb_no;

	icb_no = cb_no & 0x1;

	timer_gettime(idata->timer_table[icb_no].timer_id, &itval_tmp);
	ot1 = itval_tmp.it_interval.tv_sec + itval_tmp.it_interval.tv_nsec*(1e-9);
	ot2 = itval_tmp.it_value.tv_sec + itval_tmp.it_value.tv_nsec*(1e-9);
	idata->delay = (unsigned long)((ot1 - ot2)*10000);
	return 0;
}


/* timer stop */
int fdpm_timer_stop(int cb_no, struct fdpm_if_data *idata)
{
	int ret = 0;
	struct itimerspec itval_tmp;
	int icb_no;

	icb_no = cb_no & 0x1;

	itval_tmp.it_value.tv_sec = 0;
	itval_tmp.it_value.tv_nsec = 0;
	itval_tmp.it_interval.tv_sec = 0;
	itval_tmp.it_interval.tv_nsec = 0;

	if (timer_settime(idata->timer_table[icb_no].timer_id, 0, &itval_tmp, NULL) == -1)
		ret = -EFAULT;
	return ret;
}

/* timer delete(close) */
int fdpm_timer_close(int cb_no, struct fdpm_if_data *idata)
{
	int icb_no;
	struct fdpm_timerarg *fdpm_timer_arg;

	icb_no = cb_no & 0x1;
	fdpm_timer_arg = idata->fdpm_timer_arg[icb_no];

	if (idata->timer_table[icb_no].use == 1)
		timer_delete(idata->timer_table[icb_no].timer_id);
	idata->timer_table[icb_no].use = 0;
	free(fdpm_timer_arg);

	return 0;
}

/* timer  */
void fdpm_timer_handle(union sigval sv)
{
	struct fdpm_timerarg *fdpm_timer_arg = sv.sival_ptr;
	struct fdpm_if_data *idata = fdpm_timer_arg->idata;
	int icb_no = fdpm_timer_arg->icb_no;

	if ((icb_no & 0x1) == 0) {
		pthread_mutex_lock(&idata->cb2_timer_flg_lock);
		idata->cb2_timer_flg = 1;
		pthread_mutex_unlock(&idata->cb2_timer_flg_lock);
		sem_post(&idata->cb2_comp);/* for cb_thread function */
	} else {
		sem_post(&idata->cb4_comp);/* for timeout function */
	}
}
/*---------------------------------*/
