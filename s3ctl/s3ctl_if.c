/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include "s3ctl_user_public.h"
#include "s3ctl_user_private.h"

int s3ctl_open(S3CTL_ID *pid)
{
	int	fd;

	if (pid == NULL)
		return R_S3_PARE;

	fd = open(DEVFILE, O_RDWR);
	if (fd == -1) {
		perror("S3I open");
		return R_S3_FATAL;
	}

	*((S3CTL_ID *)pid) = fd;

	return R_S3_OK;
}

int s3ctl_close(S3CTL_ID id)
{
	int	ret;

	ret = close(id);
	if (ret) {
		if (errno == EBADF) {
			perror("S3I close");
			return R_S3_SEQE;
		} else {
			perror("S3I close");
			return R_S3_FATAL;
		}
	}

	return R_S3_OK;
}

int s3ctl_set_param(S3CTL_ID id, struct S3_PARAM *param)
{
	int	ret;

	if (param == NULL)
		return R_S3_PARE;

	ret = ioctl(id, S3_IOC_SET_PARAM, param);
	if (ret) {
		if ((errno == EPERM) || (errno == EBADF)) {
			perror("S3I SET");
			return R_S3_SEQE;
		} else if (errno == EINVAL) {
			perror("S3I SET");
			return R_S3_PARE;
		} else {
			perror("S3I SET");
			return R_S3_FATAL;
		}
	}

	return R_S3_OK;
}

int s3ctl_clear_param(S3CTL_ID id)
{
	int	ret;

	ret = ioctl(id, S3_IOC_CLEAR_PARAM, NULL);
	if (ret) {
		if ((errno == EPERM) || (errno == EBADF)) {
			perror("S3I CLEAR");
			return R_S3_SEQE;
		} else {
			perror("S3I CLEAR");
			return R_S3_FATAL;
		}
	}

	return R_S3_OK;
}

int s3ctl_lock(S3CTL_ID id)
{
	int	ret;

	ret = ioctl(id, S3_IOC_LOCK, NULL);
	if (ret) {
		if ((errno == EPERM) || (errno == EBADF)) {
			perror("S3I LOCK");
			return R_S3_SEQE;
		} else if (errno == EBUSY) {
			perror("S3I LOCK");
			return R_S3_BUSY;
		} else {
			perror("S3I LOCK");
			return R_S3_FATAL;
		}
	}

	return R_S3_OK;
}

int s3ctl_unlock(S3CTL_ID id)
{
	int	ret;

	ret = ioctl(id, S3_IOC_UNLOCK, NULL);
	if (ret) {
		if ((errno == EPERM) || (errno == EBADF)) {
			perror("S3I UNLOCK");
			return R_S3_SEQE;
		} else {
			perror("S3I UNLOCK");
			return R_S3_FATAL;
		}
	}

	return R_S3_OK;
}

int s3ctl_get_param(S3CTL_ID id, struct S3_PARAM *param)
{
	int	ret;

	if (param == NULL)
		return R_S3_PARE;

	ret = ioctl(id, S3_IOC_GET_PARAM, param);
	if (ret) {
		if (errno == EBADF) {
			perror("S3I GET");
			return R_S3_SEQE;
		} else {
			perror("S3I GET");
			return R_S3_FATAL;
		}
	}

	return R_S3_OK;
}
