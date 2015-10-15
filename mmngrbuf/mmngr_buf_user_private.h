/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2014 All rights reserved.
 */
#ifndef __MMNGR_BUF_USER_PRIVATE_H__
#define __MMNGR_BUF_USER_PRIVATE_H__

#define DEVFILE "/dev/rgnmmbuf"

struct MM_BUF_PARAM {
	unsigned long	size;
	unsigned long	hard_addr;
	int		buf;
};

#define MM_IOC_MAGIC 'd'
#define MM_IOC_EXPORT_START	_IOWR(MM_IOC_MAGIC, 0, struct MM_BUF_PARAM)
#define MM_IOC_EXPORT_END	_IOWR(MM_IOC_MAGIC, 1, struct MM_BUF_PARAM)
#define MM_IOC_IMPORT_START	_IOWR(MM_IOC_MAGIC, 2, struct MM_BUF_PARAM)
#define MM_IOC_IMPORT_END	_IOWR(MM_IOC_MAGIC, 3, struct MM_BUF_PARAM)

#endif	/* __MMNGR_BUF_USER_PRIVATE_H__ */
