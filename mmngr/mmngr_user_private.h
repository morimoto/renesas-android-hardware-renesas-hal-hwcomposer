/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */
#ifndef __MMNGR_USER_PRIVATE_H__
#define __MMNGR_USER_PRIVATE_H__

#define DEVFILE "/dev/rgnmm"

struct MM_PARAM {
	unsigned long	size;
	unsigned long	phy_addr;
	unsigned long	hard_addr;
	unsigned long	user_virt_addr;
	unsigned long	kernel_virt_addr;
	unsigned long	flag;
};

#define MM_IOC_MAGIC 'm'
#define MM_IOC_ALLOC	_IOWR(MM_IOC_MAGIC, 0, struct MM_PARAM)
#define MM_IOC_FREE	_IOWR(MM_IOC_MAGIC, 1, struct MM_PARAM)
#define MM_IOC_SET	_IOWR(MM_IOC_MAGIC, 2, struct MM_PARAM)
#define MM_IOC_GET	_IOWR(MM_IOC_MAGIC, 3, struct MM_PARAM)
#define MM_IOC_ALLOC_CO	_IOWR(MM_IOC_MAGIC, 4, struct MM_PARAM)
#define MM_IOC_FREE_CO	_IOWR(MM_IOC_MAGIC, 5, struct MM_PARAM)
#define MM_IOC_SHARE	_IOWR(MM_IOC_MAGIC, 6, struct MM_PARAM)

static int mm_alloc_kh_in_user(MMNGR_ID *pid, unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *puser_virt_addr,
				unsigned long flag);
static int mm_free_kh_in_user(MMNGR_ID id);
static int mm_alloc_co_in_user(MMNGR_ID *pid, unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *puser_virt_addr,
				unsigned long flag);
static int mm_free_co_in_user(MMNGR_ID id);

#endif	/* __MMNGR_USER_PRIVATE_H__ */
