/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */
#ifndef __S3CTL_USER_PRIVATE_H__
#define __S3CTL_USER_PRIVATE_H__

#define DEVFILE			"/dev/s3ctl"

#define S3_IOC_MAGIC 's'
#define S3_IOC_SET_PARAM	_IOWR(S3_IOC_MAGIC, 0, struct S3_PARAM)
#define S3_IOC_CLEAR_PARAM	_IOWR(S3_IOC_MAGIC, 1, struct S3_PARAM)
#define S3_IOC_LOCK		_IOWR(S3_IOC_MAGIC, 2, struct S3_PARAM)
#define S3_IOC_UNLOCK		_IOWR(S3_IOC_MAGIC, 3, struct S3_PARAM)
#define S3_IOC_GET_PARAM	_IOWR(S3_IOC_MAGIC, 4, struct S3_PARAM)

#endif	/* __S3CTL_USER_PRIVATE_H__ */
