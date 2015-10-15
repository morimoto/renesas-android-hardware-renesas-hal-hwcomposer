/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */
#ifndef	__MMNGR_USER_PUBLIC_H__
#define __MMNGR_USER_PUBLIC_H__

typedef int MMNGR_ID;

int mmngr_alloc_in_user(MMNGR_ID *pid,
			unsigned long size,
			unsigned long *pphy_addr,
			unsigned long *phard_addr,
			unsigned long *puser_virt_addr,
			unsigned long flag);
int mmngr_free_in_user(MMNGR_ID id);
#define mmngr_debug_map_va(a, b, c, d) mmngr_share_in_user(a, b, c, d)
#define mmngr_debug_unmap_va(a) mmngr_release_in_user(a)
int mmngr_share_in_user(MMNGR_ID *pid,
			unsigned long size,
			unsigned long hard_addr,
			unsigned long *puser_virt_addr);
int mmngr_release_in_user(MMNGR_ID id);

#define R_MM_OK			0
#define R_MM_FATAL		-1
#define R_MM_SEQE		-2
#define R_MM_PARE		-3
#define R_MM_NOMEM		-4

#define MMNGR_VA_SUPPORT	MM_KERNELHEAP
#define MMNGR_PA_SUPPORT	MM_CARVEOUT
#define MMNGR_PA_SUPPORT_SSP	MM_CARVEOUT_SSP
#define MMNGR_PA_SUPPORT_MV	MM_CARVEOUT_MV
#define MM_KERNELHEAP		0
#define MM_CARVEOUT		1
#define MM_CARVEOUT_SSP		3
#define MM_CARVEOUT_MV		4

#endif	/* __MMNGR_USER_PUBLIC_H__ */
