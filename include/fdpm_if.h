/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef __FDPM_IF_H__
#define __FDPM_IF_H__

/**
	* typedef PFN_FDPM_COMPLETE_CALLBACK - complete callback function pointer
	*
	*/
typedef void (*PFN_FDPM_COMPLETE_CALLBACK)(unsigned long uwUserData);


struct fdpm_cb_rsp {
	int end;
	int type;
	union {
		struct {
			PFN_FDPM_COMPLETE_CALLBACK cb;
			unsigned long uwJobId;
			long wResult;
			unsigned long uwUserData;
		} complete;
	} cb_data;
	unsigned char close_flag;
	unsigned char telecine_mode;
	T_FDP_R_SENSOR_INFO sinfo;
	T_FDP_R_REG_STATUS fdp_reg_status[60];
};

struct fdpm_ioc_cmd {
	void *req;
	void *rsp;
};

struct fdpm_open_req {
	unsigned long open_id;
	T_FDP_R_OPEN open_par;
};

struct fdpm_open_rsp {
	unsigned long open_id;
	int timer_ch;
	long rtcd;
};

struct fdpm_wait_req {
	unsigned long wait_id;
	int           vintmode;
	int           close_flag;
};

struct fdpm_close_req {
	unsigned long close_id;
	unsigned char f_release;
};

struct fdpm_close_rsp {
	unsigned long close_id;
	long rtcd;
};

struct fdpm_cancel_req {
	unsigned long cancel_fid;
	int cancel_id;
};

struct fdpm_cancel_rsp {
	unsigned long cancel_fid;
	long rtcd;
};

struct fdpm_start_req {
	unsigned long start_id;
	struct fdpm_cb_rsp cb_rsp;
	T_FDP_R_START start_par;
};

struct fdpm_start_rsp {
	unsigned long start_fid;
	T_FDP_R_CB1 fdp_cb1;
};

struct fdpm_status_req {
	unsigned long status_id;
};

struct fdpm_status_rsp {
	unsigned long status_id;
	T_FDP_STATUS status;
};

struct fdpm_status_reg_rsp {
	unsigned long status_id;
	T_FDP_REG_STATUS status[60];
};

enum {
	FDPM_CBTYPE_COMPLETE = 0,
};

enum {
	FDPM_CMD_OPEN = 0,
	FDPM_CMD_START,
	FDPM_CMD_CANCEL,
	FDPM_CMD_CLOSE,
	FDPM_CMD_STATUS,
	FDPM_CMD_WAIT,
	FDPM_CMD_WAIT_END,
	FDPM_CMD_REG_STATUS,
};

#define FDPM_IOC_MAGIC 'f'
#define FDPM_IOCTL_OPEN   _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_OPEN,   struct fdpm_ioc_cmd)
#define FDPM_IOCTL_START  _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_START,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_CANCEL _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_CANCEL, struct fdpm_ioc_cmd)
#define FDPM_IOCTL_CLOSE  _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_CLOSE,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_STATUS  _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_STATUS,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_WAIT   _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_WAIT,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_WAIT_END   _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_WAIT_END,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_REG_STATUS _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_REG_STATUS,  struct fdpm_ioc_cmd)

#endif
