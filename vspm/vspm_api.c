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
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "vspm_public.h"
#include "vspm_if.h"

#ifdef DEBUG
#define DPRINT(fmt, args...)		\
	printf(FUNCNAME ":%s() " fmt, __func__, ##args)
#else	/* DEBUG */
#define DPRINT(fmt, args...)
#endif	/* DEBUG */
#define APRINT(fmt, args...)		\
	fprintf(stderr, FUNCNAME ": " fmt, ##args)
#define ERRPRINT(fmt, args...)		\
	fprintf(stderr, FUNCNAME ":%s: " fmt, strerror(errno), ##args)

#define FUNCNAME		"vspm_api"
#define PRIO_OFFSET		10

struct vspm_handle {
	int fd;
	pthread_t thread;
};

/**
 * cb_thread - callback thread routine
 * @hdl: destination address of the handle
 * Description: 
 * Returns: 
 */
static int cb_thread(struct vspm_handle *hdl)
{
	struct vspm_cb_rsp rsp;
	struct vspm_ioc_cmd cmd;

	DPRINT("called\n");

#ifdef DEBUG
	{
		int policy;
		struct sched_param sched_param;

		pthread_getschedparam(pthread_self(), &policy, &sched_param);

		DPRINT("thread=%d\n", pthread_self());
		DPRINT("policy=%d\n", policy);
		DPRINT("sched_param.sched_priority=%d\n", sched_param.sched_priority);
	}
#endif
	cmd.req = NULL;
	cmd.rsp = (void *)&rsp;
	while (1) {
		if (ioctl(hdl->fd, VSPM_IOC_CMD_CB, &cmd)) {
			ERRPRINT("ioctl(VSPM_IOC_CMD_CB)\n");
			continue;
		}

		/* check end code */
		if (rsp.end < 0) {
			DPRINT("break\n");
			break;
		}

		/* execut callback function */
		if (rsp.type == VSPM_CBTYPE_COMPLETE) {
			(*rsp.cb_data.complete.cb)(
				rsp.cb_data.complete.uwJobId,
				rsp.cb_data.complete.wResult,
				rsp.cb_data.complete.uwUserData);
		}
	}

	DPRINT("done\n");
	return 0;
}

/**
 * VSPM_lib_DriverInitialize - Initialize the VSPM driver
 * @handle: destination address of the handle
 * Description: Initialize the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long VSPM_lib_DriverInitialize(unsigned long *handle)
{
	struct vspm_handle *hdl = 0;
	int policy;
	struct sched_param sched_param;
	pthread_attr_t thread_attr;

	int ercd;

	DPRINT("called\n");

	/* check parameter */
	if (handle == NULL) {
		goto err_exit1;
	}

	/* allocate memory */
	hdl = malloc(sizeof(*hdl));
	if (!hdl) {
		APRINT("failed to malloc()\n");
		goto err_exit1;
	}

	/* open VSP magnager */
	hdl->fd = open(DEVFILE, O_RDWR);
	if (hdl->fd == -1) {
		ERRPRINT("failed to open()\n");
		goto err_exit2;
	}

	/* init attribution of thread */
	ercd = pthread_attr_init(&thread_attr);
	if (ercd) {
		APRINT("failed to pthread_attr_init() %d\n", ercd);
		goto err_exit3;
	}

	ercd = pthread_getschedparam(pthread_self(), &policy, &sched_param);
	if (ercd) {
		APRINT("failed to pthread_getschedparam() %d\n", ercd);
		goto err_exit3;
	}

	DPRINT("thread=%d\n", pthread_self());
	DPRINT("policy=%d\n", policy);
	DPRINT("sched_param.sched_priority=%d\n", sched_param.sched_priority);

	ercd = pthread_attr_setschedpolicy(&thread_attr, policy);
	if (ercd) {
		APRINT("failed to pthread_attr_setschedpolicy() %d\n", ercd);
		goto err_exit3;
	}

	if ((policy == SCHED_FIFO) || (policy == SCHED_RR)) {
		sched_param.sched_priority += PRIO_OFFSET;
		ercd = pthread_attr_setschedparam(&thread_attr, &sched_param);
		if (ercd) {
			APRINT("failed to pthread_attr_setschedparam() %d\n", ercd);
			goto err_exit3;
		}
	}

	/* create callback thread */
	ercd = pthread_create(
		&hdl->thread, &thread_attr, (void *)cb_thread, (void *)hdl);
	if (ercd) {
		APRINT("failed to pthread_create() %d\n", ercd);
		goto err_exit3;
	}

	/* start callback */
	if (ioctl(hdl->fd, VSPM_IOC_CMD_WAIT_CB_START, NULL)) {
		ERRPRINT("failed to ioctl(VSPM_IOC_CMD_WAIT_CB_START)\n");
		goto err_exit4;
	}

	*handle = (unsigned long)hdl;

	DPRINT("done\n");

	return R_VSPM_OK;

err_exit4:
	(void)pthread_join(hdl->thread, NULL);

err_exit3:
	(void)close(hdl->fd);

err_exit2:
	free(hdl);

err_exit1:
	return R_VSPM_NG;
}

/**
 * VSPM_lib_DriverQuit - Exit the VSPM driver
 * @handle: handle
 * Description: Exit the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long VSPM_lib_DriverQuit(unsigned long handle)
{
	struct vspm_handle *hdl = (struct vspm_handle *)handle;

	int ercd;

	DPRINT("called\n");

	/* check parameter */
	if (hdl == NULL) {
		return R_VSPM_NG;
	}

	if (ioctl(hdl->fd, VSPM_IOC_CMD_CB_END, 0)) {
		ERRPRINT("failed to ioctl(VSPM_IOC_CMD_CB_END)\n");
		return R_VSPM_NG;
	}

	ercd = pthread_join(hdl->thread, NULL);
	if (ercd) {
		APRINT("failed to pthread_join() %d\n", ercd);
		return R_VSPM_NG;
	}

	if (close(hdl->fd)) {
		ERRPRINT("failed to close()\n");
		return R_VSPM_NG;
	}

	free(hdl);

	DPRINT("done\n");
	return R_VSPM_OK;
}

/**
 * VSPM_lib_Entry - Entry of various IP operations
 * @handle:            handle
 * @puwJobId:          destination address of the job id
 * @bJobPriority:      job priority
 * @ptIpParam:         pointer to IP parameter
 * @uwUserData:        user data
 * @pfnNotifyComplete: pointer to complete callback function
 * Description: Accepts requests to various IP and executed sequentially.
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_QUE_FULL : request queue full
 * R_VSPM_PARAERR : illegal parameter
 * R_VSPM_NG : other errors
 */
long VSPM_lib_Entry(
	unsigned long handle,
	unsigned long *puwJobId,
	char bJobPriority,
	VSPM_IP_PAR *ptIpParam,
	unsigned long uwUserData,
	PFN_VSPM_COMPLETE_CALLBACK pfnNotifyComplete)
{
	struct vspm_handle *hdl = (struct vspm_handle *)handle;
	struct vspm_ioc_cmd cmd;
	struct vspm_entry_req req;
	struct vspm_entry_rsp rsp;

	DPRINT("called\n");

	/* check parameter */
	if (hdl == NULL) {
		return R_VSPM_NG;
	}

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));
	req.puwJobId = puwJobId;
	req.bJobPriority = bJobPriority;
	req.ptIpParam = (VSPM_IP_PAR *)ptIpParam;
	req.uwUserData = uwUserData;
	req.pfnNotifyComplete = (PFN_VSPM_COMPLETE_CALLBACK)pfnNotifyComplete;

	cmd.req = &req;
	cmd.rsp = &rsp;
	if (ioctl(hdl->fd, VSPM_IOC_CMD_ENTRY, &cmd)) {
		ERRPRINT("failed to ioctl(VSPM_IOC_CMD_ENTRY)\n");
		return R_VSPM_NG;
	}

	if (rsp.rtcd) {
		DPRINT("failed to VSPM_IOC_CMD_ENTRY(%d)\n", rsp.rtcd);
		return rsp.rtcd;
	}

	if (puwJobId)	*puwJobId = rsp.uwJobId;

	DPRINT("done\n");
	return R_VSPM_OK;
}

/**
 * VSPM_lib_Cancel - Cancel the job
 * @handle:  handle
 * @uwJobId: job id
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long VSPM_lib_Cancel(unsigned long handle, unsigned long uwJobId)
{
	struct vspm_handle *hdl = (struct vspm_handle *)handle;
	struct vspm_ioc_cmd cmd;
	struct vspm_cancel_req req;
	struct vspm_cancel_rsp rsp;

	DPRINT("called\n");

	/* check parameter */
	if (hdl == NULL) {
		return R_VSPM_NG;
	}

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));
	req.uwJobId = uwJobId;

	cmd.req = &req;
	cmd.rsp = &rsp;
	if (ioctl(hdl->fd, VSPM_IOC_CMD_CANCEL, &cmd)) {
		ERRPRINT("failed to ioctl(VSPM_IOC_CMD_CANCEL)\n");
		return R_VSPM_NG;
	}

	if (rsp.rtcd) {
		DPRINT("failed to VSPM_IOC_CMD_CANCEL(%d)\n", rsp.rtcd);
		return rsp.rtcd;
	}

	DPRINT("done\n");
	return R_VSPM_OK;
}

/**
 * VSPM_lib_PVioUdsGetRatio - Calculating UDS ratio
 * @ushInSize:  input size
 * @ushOutSize: output size
 * @ushRatio:	pointer of UDS ratio
 * Description: Calculate UDS ratio(AMD=0) and check error.
 * Returns: On success R_VSPM_OK is returned. On error R_VSPM_NG
 * is returned.
 */
short VSPM_lib_PVioUdsGetRatio(
	unsigned short ushInSize,
	unsigned short ushOutSize,
	unsigned short *ushRatio)
{
	unsigned long ratio;

	/* check parameter */
	if (ushRatio == NULL) {
		return R_VSPM_NG;
	}

	if ((ushInSize < 1) | (ushOutSize < 1)) {
		return R_VSPM_NG;
	}

	/* AMD=0 */
	if (ushInSize < ushOutSize) {
		ushInSize--;
		ushOutSize--;
	}

	/* calculate ratio */
	ratio = ((unsigned long)ushInSize) * 0x1000UL;
	ratio /= (unsigned long)ushOutSize;

	/* check error */
	if ((ratio < 0x100) || (ratio > 0xFFFF)) {
		return R_VSPM_NG;
	}

	*ushRatio = (unsigned short)ratio;

	return R_VSPM_OK;
}

