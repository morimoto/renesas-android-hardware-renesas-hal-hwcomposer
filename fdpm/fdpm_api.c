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
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define LOG_TAG "FDPM"
/* #include <utils/Log.h> */


#include "fdpm_api.h"
#include "fdpm_if_par.h"
#include "fdpm_if_fd.h"
#include "fdpm_if_priv.h"
#include "fdpm_if.h"


void fdpm_mk_open_par(T_FDP_R_OPEN *out_par, T_FDP_OPEN *in_par);
void fdpm_mk_start_par(T_FDP_R_START *out_par, struct film_detect_private *fd_data, T_FDP_START *in_par);
void fdpm_mk_fdp_cb1(T_FDP_CB1 *userdata1, T_FDP_R_CB1 *fdp_cb1, void *userdata);
int fdpm_check_open_func(void *callback2, void *callback3, void *callback4, T_FDP_OPEN *open_par, int *result_sub_ercd);
int fdpm_check_start_func(void *callback1, void *callback2, T_FDP_START *start_par, int *result_sub_ercd);

int fdpm_if_fd_func(T_FDP_R_SENSOR_INFO *sinfo, struct film_detect_private *fd_data);

int fdpm_timer_open(unsigned char timer_ch, int cb_no, struct fdpm_if_data *idata);
int fdpm_timer_set(int cb_no, int extend, struct fdpm_if_data *idata);
int fdpm_timer_set_start(unsigned long vcnt, int cb_no, int extend, int interval, struct fdpm_if_data *idata);
int fdpm_timer_get(int cb_no, struct fdpm_if_data *idata);
int fdpm_timer_stop(int cb_no, struct fdpm_if_data *idata);
int fdpm_timer_close(int cb_no, struct fdpm_if_data *idata);

typedef void (*FUNCPTR)(T_FDP_CB1 *);
typedef void (*FUNCPTR2)(T_FDP_CB2 *);

struct fdpm_api_ldata fdpm_api_data;

/*---------------------------------*/
void *cb_thread(void *arg)
{
	struct fdpm_wait_req  req;
	struct fdpm_cb_rsp	rsp;
	struct fdpm_ioc_cmd	cmd;
	struct fdpm_if_data *idata = (struct fdpm_if_data *)arg;
	int			ret = 0;
	unsigned char         start_set_flg = 0;
	unsigned char         nogo_set_flg = 0;
	unsigned char         reread_flg = 0;
	FUNCPTR2 func_ptr;
	T_FDP_CB2 fdp_cb2;
	int cb_no;

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));
	req.wait_id = idata->open_id;
	req.vintmode = idata->vintmode;
	cmd.req = (void *)&req;
	cmd.rsp = (void *)&rsp;
	cb_no = (idata->open_id)*2 + 1;
	sem_post(&idata->cb_thread_start_comp);
	while (!ret) {
		if ((idata->vintmode == FDP_VMODE_VBEST) || (idata->vintmode == FDP_VMODE_VBEST_FDP0) ||
		    (idata->vintmode == FDP_VMODE_VBEST_FDP1) || (idata->vintmode == FDP_VMODE_VBEST_FDP2)) {
			sem_wait(&idata->start_comp);
		}
		if (idata->close_flag == 1)
			req.close_flag = 1;

		ret = ioctl(idata->handle->fd, FDPM_IOCTL_WAIT, &cmd);
		if (rsp.close_flag == 1)
			break;
		if (ret) {
			ERRPRINT("cb_thread:failed to ioctl() %d\n", ret);
		} else {
			if (idata->vintmode == FDP_VMODE_NORMAL) {
				if (rsp.end < 0) {
					sem_wait(&idata->cb2_comp);
					pthread_mutex_lock(&idata->cb2_timer_flg_lock);
					idata->cb2_timer_flg = 0;
					pthread_mutex_unlock(&idata->cb2_timer_flg_lock);
					pthread_mutex_lock(&idata->cb2_set_flg_lock);
					if (idata->cb2_set_flg == 0) {
						start_set_flg = 0;
					} else {
						start_set_flg = 1;
						idata->cb2_set_flg = 0;
					}
					pthread_mutex_unlock(&idata->cb2_set_flg_lock);
					if (start_set_flg == 0) {
						if (idata->open_first_flag == 1) {
							func_ptr = (FUNCPTR2)idata->callback2;
							fdp_cb2.userdata2 = idata->userdata2;
							idata->open_first_flag = 0;
						} else {
							func_ptr = (FUNCPTR2)idata->callback3;
							fdp_cb2.userdata2 = idata->userdata3;
						}
						fdp_cb2.ercd = E_FDP_END;
						if (func_ptr != NULL)
							(func_ptr)(&fdp_cb2);
					} else {
						pthread_mutex_lock(&idata->nogo_flg_lock);
						nogo_set_flg = idata->nogo_flg;
						pthread_mutex_unlock(&idata->nogo_flg_lock);
						if (nogo_set_flg == 1) {
							func_ptr = (FUNCPTR2)idata->callback2;
							fdp_cb2.userdata2 = idata->userdata2;
							fdp_cb2.ercd = E_FDP_END;
							idata->vintcnt++;
							if (func_ptr != NULL)
								(func_ptr)(&fdp_cb2);
							start_set_flg = 0;
						} else {
							start_set_flg = 0;
							reread_flg = 1;
							memset(&rsp, 0, sizeof(rsp));
							continue;
						}
					}
				} else {
					pthread_mutex_lock(&idata->cb2_timer_flg_lock);
					if (idata->cb2_timer_flg == 1) {
						idata->delay_flg = 1;
					} else {
						idata->delay_flg = 0;
						idata->delay = 0;
					}
					idata->cb2_timer_flg = 0;
					pthread_mutex_unlock(&idata->cb2_timer_flg_lock);

					if (reread_flg == 0) {
						sem_wait(&idata->cb2_comp);
						pthread_mutex_lock(&idata->cb2_timer_flg_lock);
						idata->cb2_timer_flg = 0;
						pthread_mutex_unlock(&idata->cb2_timer_flg_lock);
					} else {
						reread_flg = 0;
					}
					if (idata->delay_flg == 1)
						fdpm_timer_get(0, idata);

					idata->vintcnt++;
					memcpy(&idata->sinfo, &rsp.sinfo, sizeof(T_FDP_R_SENSOR_INFO));
					memcpy(&idata->fdp_reg_status, &rsp.fdp_reg_status, sizeof(T_FDP_R_REG_STATUS)*60);
					if (rsp.telecine_mode == 1)
						/* film detection function */
						fdpm_if_fd_func(&rsp.sinfo, &idata->fd_data);

					switch (rsp.type) {
					case FDPM_CBTYPE_COMPLETE:
						func_ptr = (FUNCPTR2)rsp.cb_data.complete.cb;
						fdp_cb2.userdata2 = (void *)rsp.cb_data.complete.uwUserData;
						if (idata->delay > 0)
							fdp_cb2.ercd =  E_FDP_DELAYED;
						else
							fdp_cb2.ercd =  E_FDP_END;
						if (func_ptr != NULL)
							(func_ptr)(&fdp_cb2);
						break;
					default:
						ERRPRINT("ERROR\n");
						break;
					}
				}
			} else {/* best effort mode */
				if (rsp.end < 0) {
					continue;
				} else {
					memcpy(&idata->sinfo, &rsp.sinfo, sizeof(T_FDP_R_SENSOR_INFO));
					memcpy(&idata->fdp_reg_status, &rsp.fdp_reg_status, sizeof(T_FDP_R_REG_STATUS)*60);
					if (idata->callback4 != NULL) {
						if (fdpm_timer_stop(cb_no, idata) != 0)
							ERRPRINT("error:fdpm_timer_stop\n");
					}
					if (rsp.telecine_mode == 1) {
						/* film detection function */
						fdpm_if_fd_func(&rsp.sinfo, &idata->fd_data);
					}
					switch (rsp.type) {
					case FDPM_CBTYPE_COMPLETE:
						func_ptr = (FUNCPTR2)rsp.cb_data.complete.cb;
						fdp_cb2.userdata2 = (void *)rsp.cb_data.complete.uwUserData;
						fdp_cb2.ercd =  E_FDP_END;
						if (func_ptr != NULL)
							(func_ptr)(&fdp_cb2);
						break;
					default:
						break;
					}
				}
			}
		}
		memset(&rsp, 0, sizeof(rsp));
	}
	return 0;
}

void *to_thread(void *arg)
{
	FUNCPTR2 func_ptr;
	T_FDP_CB2 fdp_cb2;
	struct fdpm_if_data *idata = (struct fdpm_if_data *)arg;
	int ret = 0;

	sem_post(&idata->to_thread_start_comp);
	while (!ret) {
		sem_wait(&idata->cb4_comp);
		if (idata->close_flag == 1)
			break;

		func_ptr = (FUNCPTR2)idata->callback4;
		fdp_cb2.userdata2 = idata->userdata4;
		fdp_cb2.ercd = E_FDP_DELAYED;
		if (func_ptr != NULL)
			(func_ptr)(&fdp_cb2);
	}
	return 0;
}

int drv_FDPM_Open(void *callback2, void *callback3, void *callback4, T_FDP_OPEN *open_par, FP user_function, void *userdata2, void *userdata3, void *userdata4, int *sub_ercd)
{
	struct fdpm_handle *hdl = 0;
	long rtcd = 0;
	struct fdpm_if_data *idata = 0;
	struct fdpm_ioc_cmd    cmd;
	struct fdpm_open_req   req;
	struct fdpm_open_rsp   rsp;
	int ret = 0;
	int cb_no = 0;
	int occupy_vbest_flg = 0;

#ifdef FDP_PAR_CHECK_MODE
	int result_sub_ercd;

	if (fdpm_check_open_func(callback2, callback3, callback4, open_par, &result_sub_ercd) == -1) {
		if (sub_ercd != NULL)
			*sub_ercd = result_sub_ercd;
		rtcd = -EINVAL;
		goto exit;
	}
#endif
	/* make ioctl_data */
	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	fdpm_mk_open_par(&req.open_par, open_par);

	occupy_vbest_flg = (((open_par->vmode == FDP_VMODE_VBEST) || (open_par->vmode == FDP_VMODE_VBEST_FDP0) || (open_par->vmode == FDP_VMODE_VBEST_FDP1) || (open_par->vmode == FDP_VMODE_VBEST_FDP2)) && (open_par->ocmode == FDP_OCMODE_OCCUPY)) ? 1 : 0;

	hdl = malloc(sizeof(struct fdpm_handle));
	if (!hdl) {
		APRINT("failed to malloc()\n");
		rtcd = -ENOMEM;
		goto exit;
	}
	hdl->fd = open(DEVFILE, O_RDWR);
	if (hdl->fd == -1) {
		APRINT("Failed to open()\n");
		rtcd = -EACCES;
		goto exit;
	}
	fdpm_api_data.handle = (unsigned long *)hdl;
	if (occupy_vbest_flg) {
		if (sub_ercd != NULL) {
			*sub_ercd = (int)hdl;
		} else {
			APRINT("failed to store file descriptor\n");
			rtcd = -EACCES;
			goto exit;
		}
	}
	idata = malloc(sizeof(struct fdpm_if_data));
	if (!idata) {
		APRINT("failed to malloc()\n");
		rtcd = -ENOMEM;
		goto exit;
	}
	memset(idata, 0, sizeof(struct fdpm_if_data));
	hdl->idata_use = 1;
	hdl->idata = (struct fdpm_if_data *)idata;
	fdpm_api_data.idata_use = 1;
	fdpm_api_data.idata = (struct fdpm_if_data *)idata;
	fdpm_api_data.occupy_vbest_flg = occupy_vbest_flg;

	idata->handle = hdl;
	idata->callback2 = (void (*)())callback2;
	idata->callback3 = (void (*)())callback3;
	idata->callback4 = (void (*)())callback4;
	idata->userdata2 = userdata2;
	idata->userdata3 = userdata3;
	idata->userdata4 = userdata4;
	idata->close_flag = 0;

	cmd.req = &req;
	cmd.rsp = &rsp;

	ret = ioctl(hdl->fd, FDPM_IOCTL_OPEN, &cmd);
	if (ret) {
		ERRPRINT("OPEN:failed to ioctl() %d\n", ret);
		if (occupy_vbest_flg == 0)
			*sub_ercd = ret;
		rtcd = -EINVAL;
		goto exit;
	} else {
		fdpm_api_data.idata_id = rsp.open_id;
		hdl->idata_id = rsp.open_id;
		idata->open_id = rsp.open_id;
		idata->timer_ch = rsp.timer_ch;
		idata->vintmode = open_par->vmode;
		idata->telecine_mode = 0;
		idata->to_vcnt  = TO_VCNT;
		idata->delay = 0;
		idata->vintcnt = 0;

		sem_init(&idata->start_comp, 0, 0);
		sem_init(&idata->cb_thread_start_comp, 0, 0);
		sem_init(&idata->to_thread_start_comp, 0, 0);

		pthread_mutex_init(&idata->nogo_flg_lock, NULL);

		/* vint mode : timer cleate */
		if (open_par->vmode == FDP_VMODE_NORMAL) {
			idata->open_first_flag = 1;
			sem_init(&idata->meas_comp, 0, 0);
			cb_no = (idata->open_id*2) + 0;
			if (callback2 != NULL) {
				idata->cb2_timer_flg = 0;
				idata->cb2_set_flg = 0;
				pthread_mutex_init(&idata->cb2_timer_flg_lock, NULL);
				pthread_mutex_init(&idata->cb2_set_flg_lock, NULL);
				ret = fdpm_timer_open(rsp.timer_ch, cb_no, idata);
				if (ret) {
					ERRPRINT("failed to fdpm_timer_open\n");
					rtcd = R_FDPM_NG;
					goto exit;
				}
			}
			ret = fdpm_timer_set_start(open_par->vcnt, cb_no, 0, 1, idata);
			if (ret) {
				ERRPRINT("failed to fdpm_timer_ser_start(0)\n");
				rtcd = R_FDPM_NG;
				goto exit;
			}
		} else {
			idata->open_first_flag = 0;
		}
		cb_no = (idata->open_id)*2 + 1;
		if (callback4 != NULL) {
			ret = fdpm_timer_open(rsp.timer_ch, cb_no, idata);
			if (ret) {
				ERRPRINT("failed to fdpm_timer_open\n");
				rtcd = R_FDPM_NG;
				goto exit;
			}
		}
		if (callback4 != NULL) {
			sem_init(&idata->cb4_comp, 0, 0);
			ret = pthread_create(&hdl->thread_to, NULL, to_thread, idata);
			if (ret) {
				APRINT("callback4 pthread_create fail.\n");
				rtcd = R_FDPM_NG;
				goto exit;
			}
			/* wait run to_thread */
			sem_wait(&idata->to_thread_start_comp);
			ret = fdpm_timer_set_start(idata->to_vcnt, cb_no, 1, 0, idata);
			if (ret) {
				ERRPRINT("failed to fdpm_timer_set_start(1)\n");
				rtcd = R_FDPM_NG;
				goto exit;
			}
		}
		/* callback2 wait thread create */
		/* FDPMIF_wait_callback_thread for vint */
		sem_init(&idata->cb2_comp, 0, 0);
		ret = pthread_create(&hdl->thread, NULL, cb_thread, idata);
		if (ret) {
			APRINT("callback2 pthread_create fail.\n");
			rtcd = R_FDPM_NG;
			goto exit_cb;
		}
		/* wait run cb_thread */
		sem_wait(&idata->cb_thread_start_comp);
		sigemptyset(&idata->timer_table[0].sigmask);
		sigaddset(&idata->timer_table[0].sigmask, SIGRTMIN + rsp.timer_ch);
		if (pthread_sigmask(SIG_BLOCK, &idata->timer_table[0].sigmask, NULL) != 0)
			perror("pthread_sigmask error");
		if (user_function != NULL)
			user_function();
		return 0;
	}
exit_cb:
	cb_no = (idata->open_id)*2 + 1;
	if (idata->callback4 != NULL) {
		idata->close_flag = 1;
		fdpm_timer_close(cb_no, idata);
		sem_post(&idata->cb4_comp);
		ret = pthread_join(hdl->thread_to, NULL);
		if (ret)
			APRINT("callback4 pthread_join() fail.\n");
		sem_destroy(&idata->cb4_comp);
		sem_destroy(&idata->to_thread_start_comp);
	}
exit:
	if (hdl) {
		free(hdl);
		fdpm_api_data.handle = NULL;
	}
	if (idata) {
		free(idata);
		fdpm_api_data.idata = NULL;
		fdpm_api_data.idata_use = 0;
	}
	return rtcd;
}

/* drv_FDPM_Close */
int drv_FDPM_Close(FP user_function, int *sub_ercd, unsigned char f_release)
{
	long rtcd = 0;
	int ret;
	struct fdpm_ioc_cmd cmd;
	struct fdpm_close_req req;
	struct fdpm_close_rsp rsp;
	struct fdpm_handle  *hdl = NULL;
	struct fdpm_if_data *idata;
	int cb_no = 0;
	int occupy_vbest_flg;

	occupy_vbest_flg = (fdpm_api_data.occupy_vbest_flg == 1) ? 1 : 0;

	if (occupy_vbest_flg != 1) {
		hdl = (struct fdpm_handle *)fdpm_api_data.handle;
		idata = (struct fdpm_if_data *)fdpm_api_data.idata;
	} else {
		if (sub_ercd != NULL) {
			if ((*sub_ercd) == 0) {
				hdl = NULL;
			} else {
				hdl = (struct fdpm_handle *)(*sub_ercd);
				idata = hdl->idata;
			}
		} else {
			rtcd = R_FDPM_NG;
			goto exit;
		}
	}

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	if (hdl == NULL) {
		rtcd = -EACCES;
		goto exit;
	}

	/* make ioctl_data */
	req.close_id = idata->open_id;
	req.f_release = f_release;

	cmd.req = &req;
	cmd.rsp = &rsp;

	idata->close_flag = 1;

	if ((idata->vintmode == FDP_VMODE_VBEST) || (idata->vintmode == FDP_VMODE_VBEST_FDP0) ||
	    (idata->vintmode == FDP_VMODE_VBEST_FDP1) || (idata->vintmode == FDP_VMODE_VBEST_FDP2)) {
		sem_post(&idata->start_comp);
	}
	ret = ioctl(hdl->fd, FDPM_IOCTL_CLOSE, &cmd);
	if (ret) {
		ERRPRINT("CLOSE:failed to ioctl() %d\n", ret);
			rtcd = R_FDPM_NG;
		goto exit;
	} else {
		/* callback2 wait thread destroy */
		/* FDPMIF_wait_callback_thread for vint */
		ret = pthread_join(hdl->thread, NULL);
		if (ret) {
			APRINT("callback2 pthread_join() fail.\n");
			rtcd = R_FDPM_NG;
			goto exit;
		}
		cb_no = (idata->open_id)*2 + 0;
		if (idata->vintmode == FDP_VMODE_NORMAL)
			fdpm_timer_close(cb_no, idata);

		pthread_mutex_destroy(&idata->cb2_timer_flg_lock);
		pthread_mutex_destroy(&idata->cb2_set_flg_lock);
		pthread_mutex_destroy(&idata->nogo_flg_lock);
		sem_destroy(&idata->cb2_comp);
		sem_destroy(&idata->cb_thread_start_comp);
		sem_destroy(&idata->start_comp);
		/* callback4 wait thread destroy */
		/* FDPMIF_wait_callback_thread for timeout */
		cb_no = (idata->open_id)*2 + 1;
		if (idata->callback4 != NULL) {
			fdpm_timer_close(cb_no, idata);
			sem_post(&idata->cb4_comp);
			ret = pthread_join(hdl->thread_to, NULL);
			if (ret) {
				APRINT("callback4 pthread_join() fail.\n");
				rtcd = R_FDPM_NG;
				goto exit;
			}
			sem_destroy(&idata->cb4_comp);
			sem_destroy(&idata->to_thread_start_comp);
		}
		/* user function */
		if (user_function != NULL)
			user_function();
	}

	ret = close(hdl->fd);
	if (ret) {
		APRINT("failed to close()\n");
		rtcd = -EACCES;
		goto exit;
	}
	if (idata) {
		free(idata);
		fdpm_api_data.idata = NULL;
		fdpm_api_data.idata_use = 0;
		hdl->idata = NULL;
		hdl->idata_use = 0;
	}
	free(hdl);
	fdpm_api_data.handle = NULL;
	if (occupy_vbest_flg == 0) {
		if (sub_ercd != NULL)
			*sub_ercd = 0;
	} else {
		*sub_ercd = (int)NULL;
	}
exit:
	return rtcd;
}

/* drv_FDPM_Start */
int drv_FDPM_Start(void *callback1, void *callback2, T_FDP_START *start_par, void *userdata1, void *userdata2, int *sub_ercd)
{
	long rtcd = 0;
	int ret;
	struct fdpm_ioc_cmd cmd;
	struct fdpm_start_req req;
	struct fdpm_start_rsp rsp;
	struct fdpm_if_data *idata;
	struct fdpm_handle *hdl = NULL;
	FUNCPTR func_ptr;
	T_FDP_CB1 iuserdata;
	void *userdata;
	int cb_no = 0;
	int occupy_vbest_flg;

	occupy_vbest_flg = (fdpm_api_data.occupy_vbest_flg == 1) ? 1 : 0;

	if (occupy_vbest_flg != 1) {
		hdl = (struct fdpm_handle *)fdpm_api_data.handle;
		idata = (struct fdpm_if_data *)fdpm_api_data.idata;
	} else {
		if (sub_ercd != NULL) {
			if ((*sub_ercd) == 0) {
				hdl = NULL;
			} else {
				hdl = (struct fdpm_handle *)(*sub_ercd);
				idata = hdl->idata;
			}
		} else {
			rtcd = R_FDPM_NG;
			goto exit;
		}
	}

#ifdef FDP_PAR_CHECK_MODE
	int result_sub_ercd;

	if (fdpm_check_start_func(callback1, callback2, start_par, &result_sub_ercd) == -1) {
		if (occupy_vbest_flg == 0) {
			if (sub_ercd != NULL)
				*sub_ercd = result_sub_ercd;
		}
		rtcd = -EINVAL;
		goto exit;
	}
#endif

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	if (hdl == NULL) {
		rtcd = -EACCES;
		goto exit;
	}

	iuserdata.userdata1 = userdata1;
	/* make ioctl_data */
	fdpm_mk_start_par(&req.start_par, &idata->fd_data, start_par);
#ifdef FDP_FD_VINT_FON
	if (idata->vintmode == FDP_VMODE_NORMAL)
		req.start_par.fproc_par.seq_par.telecine_mode = FDP_TC_ON;
#endif
#ifdef FDP_FD_VINT_FOFF
	if (idata->vintmode == FDP_VMODE_NORMAL)
		req.start_par.fproc_par.seq_par.telecine_mode = FDP_TC_OFF;
#endif
#ifdef FDP_FD_VBEST_FON
	if (idata->vintmode != FDP_VMODE_NORMAL)
		req.start_par.fproc_par.seq_par.telecine_mode = FDP_TC_ON;
#endif
#ifdef FDP_FD_VBEST_FOFF
	if (idata->vintmode != FDP_VMODE_NORMAL)
		req.start_par.fproc_par.seq_par.telecine_mode = FDP_TC_OFF;
#endif
	req.start_id = idata->open_id;
	req.cb_rsp.end = 0;
	req.cb_rsp.type = 0;
	req.cb_rsp.cb_data.complete.cb = (void (*)())callback2;
	req.cb_rsp.cb_data.complete.uwJobId = 0;
	req.cb_rsp.cb_data.complete.wResult = 0;
	req.cb_rsp.cb_data.complete.uwUserData = (unsigned long)userdata2;

	cmd.req = &req;
	cmd.rsp = &rsp;
	if (start_par->fdpgo == 1) {
		ret = ioctl(hdl->fd, FDPM_IOCTL_START, &cmd);
	} else {
		idata->callback2 = (void (*)())callback2;
		idata->userdata2 = userdata2;
		ret = 0;
	}
	if (ret) {
		ERRPRINT("failed to ioctl()\n");
		if (occupy_vbest_flg == 0) {
			if (sub_ercd != NULL)
				*sub_ercd = ret;
		}
		rtcd = R_FDPM_NG;
		goto exit;
	} else {
		if ((idata->vintmode == FDP_VMODE_VBEST) || (idata->vintmode == FDP_VMODE_VBEST_FDP0) ||
		    (idata->vintmode == FDP_VMODE_VBEST_FDP1) || (idata->vintmode == FDP_VMODE_VBEST_FDP2)) {
			sem_post(&idata->start_comp);
		} else {
			cb_no = (idata->open_id)*2 + 0;
			if (callback2 != NULL) {
				pthread_mutex_lock(&idata->cb2_set_flg_lock);
				idata->cb2_set_flg = 1;
				if (start_par->fdpgo == 0)
					idata->nogo_flg = 1;
				else
					idata->nogo_flg = 0;
				pthread_mutex_unlock(&idata->cb2_set_flg_lock);
				if (req.start_par.vcnt_flg == 1) {
					ret = fdpm_timer_set_start(req.start_par.vcnt, cb_no, 0, 1, idata);
					if (ret) {
						ERRPRINT("failed to fdpm_timer_set_start(0)\n");
						rtcd = E_FDP_TIMER_CB;
						goto exit;
					}
				}
			}
		}
		idata->start_id = rsp.start_fid;
		cb_no = (idata->open_id)*2 + 1;
		if (idata->callback4 != NULL) {
			ret = fdpm_timer_set_start(idata->to_vcnt, cb_no, 1, 0, idata);
			if (ret) {
				ERRPRINT("failed to fdpm_timer_set_start(1)\n");
				rtcd = E_FDP_TIMER_TO;
				goto exit;
			}
		}
		/* callback1 */
		userdata = (void *)(iuserdata.userdata1);
		fdpm_mk_fdp_cb1(&iuserdata, &rsp.fdp_cb1, userdata);
		func_ptr = (FUNCPTR)callback1;
		if (func_ptr != NULL)
			(func_ptr)(&iuserdata);
		return 0;
	}
exit:
	return rtcd;
}

/* drv_FDPM_Cancel */
int drv_FDPM_Cancel(int id, int *sub_ercd)
{
	long rtcd;
	int ret;
	struct fdpm_ioc_cmd cmd;
	struct fdpm_cancel_req req;
	struct fdpm_cancel_rsp rsp;
	struct fdpm_handle *hdl = NULL;
	struct fdpm_if_data *idata;
	int occupy_vbest_flg;

	occupy_vbest_flg = (fdpm_api_data.occupy_vbest_flg == 1) ? 1 : 0;

	if (occupy_vbest_flg != 1) {
		hdl = (struct fdpm_handle *)fdpm_api_data.handle;
		idata = (struct fdpm_if_data *)fdpm_api_data.idata;
	} else {
		if (sub_ercd != NULL) {
			if ((*sub_ercd) == 0) {
				hdl = NULL;
			} else {
				hdl = (struct fdpm_handle *)(*sub_ercd);
				idata = hdl->idata;
			}
		} else {
			rtcd = R_FDPM_NG;
			goto exit;
		}
	}

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	if (hdl == NULL) {
		rtcd = -EACCES;
		goto exit;
	}
	/* make ioctl_data */
	req.cancel_id = id;
	req.cancel_fid = idata->open_id;

	cmd.req = &req;
	cmd.rsp = &rsp;

	ret = ioctl(hdl->fd, FDPM_IOCTL_CANCEL, &cmd);
	if (ret) {
		ERRPRINT("failed to ioctl()\n");
		rtcd = -EINVAL;
		goto exit;
	} else {
		if (rsp.rtcd != 0) {
			if (occupy_vbest_flg == 0) {
				if (sub_ercd != NULL)
					*sub_ercd = rsp.rtcd;
			}
			rtcd = -EINVAL;
		} else {
			rtcd = 0;
		}
	}
exit:
	return rtcd;
}

/* drv_FDPM_Status */
int drv_FDPM_Status(T_FDP_STATUS *fdp_status, int *sub_ercd)
{
	long rtcd;
	int ret;
	struct fdpm_ioc_cmd cmd;
	struct fdpm_status_req req;
	struct fdpm_status_rsp rsp;
	struct fdpm_handle *hdl = NULL;
	struct fdpm_if_data *idata;
	int occupy_vbest_flg;

	occupy_vbest_flg = (fdpm_api_data.occupy_vbest_flg == 1) ? 1 : 0;

#ifdef FDP_PAR_CHECK_MODE
	if (fdp_status == NULL) {
		if (occupy_vbest_flg != 0) {
			if (sub_ercd != NULL)
				*sub_ercd = E_FDP_PARA_STATUS;
		}
		rtcd = -EINVAL;
		goto exit;
	}
#endif

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	cmd.req = &req;
	cmd.rsp = &rsp;

	if (occupy_vbest_flg != 1) {
		hdl = (struct fdpm_handle *)fdpm_api_data.handle;
		idata = (struct fdpm_if_data *)fdpm_api_data.idata;
	} else {
		if (sub_ercd != NULL) {
			if ((*sub_ercd) == 0) {
				hdl = NULL;
			} else {
				hdl = (struct fdpm_handle *)(*sub_ercd);
				idata = hdl->idata;
			}
		} else {
			rtcd = R_FDPM_NG;
			goto exit;
		}
	}

	if (hdl == NULL) {
		memcpy(fdp_status , &rsp.status, sizeof(T_FDP_STATUS));
		return 0;
	}

	req.status_id = idata->open_id;
	ret = ioctl(hdl->fd, FDPM_IOCTL_STATUS, &cmd);
	if (ret) {
		ERRPRINT("failed to ioctl()\n");
		if (occupy_vbest_flg == 0) {
			if (sub_ercd != NULL)
				*sub_ercd = ret;
		}
		rtcd = -EACCES;
		goto exit;
	} else {
		memcpy(fdp_status , &rsp.status, sizeof(T_FDP_STATUS));
		fdp_status->delay   = idata->delay;
		fdp_status->vintcnt = idata->vintcnt;
		return 0;
	}
exit:
	return rtcd;
}

int drv_FDPM_Status_reg(T_FDP_REG_STATUS *fdp_reg_status, int *sub_ercd)
{
	long rtcd = 0;
	struct fdpm_handle *hdl;
	struct fdpm_if_data *idata;
	int occupy_vbest_flg;

	occupy_vbest_flg = (fdpm_api_data.occupy_vbest_flg == 1) ? 1 : 0;

	if (occupy_vbest_flg != 1) {
		hdl = (struct fdpm_handle *)fdpm_api_data.handle;
		idata = (struct fdpm_if_data *)fdpm_api_data.idata;
	} else {
		if (sub_ercd != NULL) {
			if ((*sub_ercd) == 0) {
				hdl = NULL;
			} else {
				hdl = (struct fdpm_handle *)(*sub_ercd);
				idata = hdl->idata;
			}
		} else {
			rtcd = R_FDPM_NG;
			goto exit;
		}
	}

	if (hdl == NULL) {
		rtcd = -EACCES;
		goto exit;
	}

	memcpy(fdp_reg_status, &idata->fdp_reg_status, sizeof(T_FDP_REG_STATUS)*60);
exit:
	return rtcd;
}

int drv_FDPM_Status_fd(struct film_detect_private *fd_data, int *sub_ercd)
{
	int rtcd = 0;
	struct fdpm_handle *hdl;
	struct fdpm_if_data *idata;
	int occupy_vbest_flg;

	occupy_vbest_flg = (fdpm_api_data.occupy_vbest_flg == 1) ? 1 : 0;

	if (occupy_vbest_flg != 1) {
		hdl = (struct fdpm_handle *)fdpm_api_data.handle;
		idata = (struct fdpm_if_data *)fdpm_api_data.idata;
	} else {
		if (sub_ercd != NULL) {
			if ((*sub_ercd) == 0) {
				hdl = NULL;
			} else {
				hdl = (struct fdpm_handle *)(*sub_ercd);
				idata = hdl->idata;
			}
		} else {
			rtcd = R_FDPM_NG;
			goto exit;
		}
	}

	if (hdl == NULL) {
		rtcd = -EACCES;
		goto exit;
	}

	memcpy(fd_data, &idata->fd_data, sizeof(struct film_detect_private));
exit:
	return rtcd;
}
