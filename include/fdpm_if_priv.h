/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef __FDPM_IF_PRIV_H_
#define __FDPM_IF_PRIV_H_

#define DEVFILE "/dev/fdpm"

#define	FUNCNAME	"fdpm_api"

#ifdef	DEBUG
#define DPRINT(fmt, args...)	\
	printf(FUNCNAME ":%s() " fmt, __func__, ##args)
#else	/* DEBUG */
#define DPRINT(fmt, args...)
#endif	/* DEBUG */
#define APRINT(fmt, args...)	\
	fprintf(stderr, FUNCNAME ": " fmt, ##args)
#define ERRPRINT(fmt, args...)	\
	fprintf(stderr, FUNCNAME ":%s: " fmt, strerror(errno), ##args)

struct fdpm_handle {
	int       fd;
	pthread_t thread;
	pthread_t thread_to;
	pthread_t thread_meas;
	unsigned char idata_use;
	struct fdpm_if_data *idata;
	int idata_id;
};

struct fdpm_timer_table {
	unsigned char use;
	sigset_t sigmask;
	timer_t timer_id;
	struct itimerspec itval;
	struct sigaction sa;
	struct sigevent  se;
};

struct fdpm_timerarg {
	struct fdpm_if_data *idata;
	int    icb_no;
};

struct fdpm_if_data {
	unsigned long open_id;
	unsigned long start_id;
	struct fdpm_handle *handle;
	void (*callback)();
	void (*callback2)(T_FDP_CB2 *);
	void (*callback3)(T_FDP_CB2 *);
	void (*callback4)();
	void *userdata2;
	void *userdata3;
	void *userdata4;
	struct fdpm_timer_table timer_table[2];
	int timer_ch;
	int vintmode;
	int telecine_mode;
	unsigned char open_first_flag;
	unsigned int close_flag;
	unsigned int seq_count;
	unsigned long to_vcnt;
	unsigned char cb2_timer_flg;
	unsigned char cb2_set_flg;
	unsigned char delay_flg;
	unsigned char nogo_flg;
	sem_t cb2_comp;
	sem_t cb4_comp;
	sem_t meas_comp;
	sem_t start_comp;
	sem_t cb_thread_start_comp;
	sem_t to_thread_start_comp;
	sem_t cb2_test;
	pthread_mutex_t cb2_timer_flg_lock;
	pthread_mutex_t cb2_set_flg_lock;
	pthread_mutex_t nogo_flg_lock;
	unsigned long delay;
	unsigned long vintcnt;
	struct film_detect_private fd_data;
	struct fdpm_timerarg *fdpm_timer_arg[2];
	T_FDP_R_SENSOR_INFO sinfo;
	T_FDP_R_REG_STATUS fdp_reg_status[60];
};

struct fdpm_api_ldata {
	unsigned long *handle;
	pthread_mutex_t multi_stream_lock;
	unsigned char idata_use;
	struct fdpm_if_data *idata;
	int idata_count;
	int idata_id;
	unsigned char occupy_vbest_flg;
};

extern struct fdpm_api_ldata fdpm_api_data;
#endif
