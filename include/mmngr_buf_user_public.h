/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2014 All rights reserved.
 */
#ifndef	__MMNGR_BUF_USER_PUBLIC_H__
#define __MMNGR_BUF_USER_PUBLIC_H__

int mmngr_export_start_in_user(int *pid,
			unsigned long size,
			unsigned long hard_addr,
			int *pbuf);
int mmngr_export_end_in_user(int id);
int mmngr_import_start_in_user(int *pid,
			unsigned long *psize,
			unsigned long *phard_addr,
			int buf);
int mmngr_import_end_in_user(int id);

#define R_MM_OK			0
#define R_MM_FATAL		-1
#define R_MM_SEQE		-2
#define R_MM_PARE		-3
#define R_MM_NOMEM		-4

#endif	/* __MMNGR_BUF_USER_PUBLIC_H__ */
