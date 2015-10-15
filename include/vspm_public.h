/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013-2014 All rights reserved.
 */

#ifndef __VSPM_PUBLIC_H__
#define __VSPM_PUBLIC_H__

#include "tddmac_drv.h"
#include "vsp_drv.h"

/* VSPM driver APIs return codes */
#define R_VSPM_OK			(0)
#define	R_VSPM_NG			(-1)	/* abnormal termination */
#define	R_VSPM_PARAERR		(-2)	/* illegal parameter */
#define	R_VSPM_SEQERR		(-3)	/* sequence error */
#define R_VSPM_QUE_FULL		(-4)	/* request queue full */
#define R_VSPM_CANCEL		(-5)	/* processing was canceled */
#define R_VSPM_DRIVER_ERR	(-10)	/* IP error(in driver) */
#define R_VSPM_HARDWARE_ERR	(-11)	/* not used */
#define R_VSPM_START_ERR	(-12)	/* not used */

/* Type of the IP */
#define VSPM_TYPE_VSP_AUTO		0x0600
#define VSPM_TYPE_2DDMAC_AUTO	0x0400

#define VSPM_TYPE_VSP_VSPS		0x1000
#define VSPM_TYPE_VSP_VSPR		0x2000
#define VSPM_TYPE_VSP_VSPD0		0x4000
#define VSPM_TYPE_VSP_VSPD1		0x8000

/* Job priority */
#define VSPM_PRI_MAX		((char)126)
#define VSPM_PRI_MIN		((char)  1)

/* State of the entry */
#define VSPM_STATUS_WAIT		1
#define VSPM_STATUS_ACTIVE		2
#define VSPM_STATUS_NO_ENTRY	3

/* typedef vsp parameter */
typedef T_VSP_START		VSPM_VSP_PAR;

/**
 * typedef PFN_VSPM_COMPLETE_CALLBACK - complete callback function pointer
 *
 */
typedef void (*PFN_VSPM_COMPLETE_CALLBACK)(
	unsigned long uwJobId, long wResult, unsigned long uwUserData);

/**
 * struct t_vspm_2ddmac_par - parameter to 2DDMAC processing
 * @ptTdDmacMode:    request mode setting table pointer
 * @ptTdDmacRequest: DMA transfer setting table pointer
 */
typedef struct t_vspm_2ddmac_par {
	T_TDDMAC_MODE *ptTdDmacMode;
	T_TDDMAC_REQUEST *ptTdDmacRequest;
} VSPM_2DDMAC_PAR;

/**
 * struct t_vspm_ip_par - parameter to VSPM_lib_Entry()
 * @uhType:       type of IP
 * @unionIpParam: parameters to the IP
 */
typedef struct t_vspm_ip_par {
	unsigned short uhType;
	union {
		VSPM_VSP_PAR *ptVsp;
		VSPM_2DDMAC_PAR *pt2dDmac;
	} unionIpParam;
} VSPM_IP_PAR;


/*
 * VSPM driver APIs
 */

/**
 * VSPM_lib_DriverInitialize - Initialize the VSPM driver
 * @handle: destination address of the handle
 * Description: Initialize the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long VSPM_lib_DriverInitialize(unsigned long *handle);

/**
 * VSPM_lib_DriverQuit - Exit the VSPM driver
 * @handle: handle
 * Description: Exit the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long VSPM_lib_DriverQuit(unsigned long handle);

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
	PFN_VSPM_COMPLETE_CALLBACK pfnNotifyComplete);

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
long VSPM_lib_Cancel(unsigned long handle, unsigned long uwJobId);

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
	unsigned short *ushRatio);

#endif	/* __VSPM_PUBLIC_H__ */
