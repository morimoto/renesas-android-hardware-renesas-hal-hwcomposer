/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */
#ifndef	__S3CTL_USER_PUBLIC_H__
#define __S3CTL_USER_PUBLIC_H__

#define R_S3_OK		0
#define R_S3_FATAL	-1
#define R_S3_SEQE	-2
#define R_S3_PARE	-3
#define R_S3_BUSY	-4

#define S3_STRIDE_128	128
#define S3_STRIDE_256	256
#define S3_STRIDE_512	512
#define S3_STRIDE_1K	1024
#define S3_STRIDE_2K	2048
#define S3_STRIDE_4K	4096

#define S3_AREA_256	0
#define S3_AREA_512	1
#define S3_AREA_1K	2
#define S3_AREA_2K	3
#define S3_AREA_4K	4
#define S3_AREA_8K	5
#define S3_AREA_16K	6
#define S3_AREA_32K	7
#define S3_AREA_64K	8
#define S3_AREA_128K	9
#define S3_AREA_256K	10

struct S3_PARAM {
	unsigned long	phy_addr;
	unsigned long	stride;
	unsigned long	area;
};

typedef	int	S3CTL_ID;

int s3ctl_open(S3CTL_ID *pid);
int s3ctl_close(S3CTL_ID id);
int s3ctl_set_param(S3CTL_ID id, struct S3_PARAM *param);
int s3ctl_clear_param(S3CTL_ID id);
int s3ctl_lock(S3CTL_ID id);
int s3ctl_unlock(S3CTL_ID id);
int s3ctl_get_param(S3CTL_ID id, struct S3_PARAM *param);

#endif	/* __S3CTL_USER_PUBLIC_H__ */
