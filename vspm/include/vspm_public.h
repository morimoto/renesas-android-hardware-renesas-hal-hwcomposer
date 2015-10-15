/*************************************************************************/ /*
 VSPM

 Copyright (C) 2013-2014 Renesas Electronics Corporation

 License        Dual MIT/GPLv2

 The contents of this file are subject to the MIT license as set out below.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 Alternatively, the contents of this file may be used under the terms of
 the GNU General Public License Version 2 ("GPL") in which case the provisions
 of GPL are applicable instead of those above.

 If you wish to allow use of your version of this file only under the terms of
 GPL, and not to allow others to use your version of this file under the terms
 of the MIT license, indicate your decision by deleting the provisions above
 and replace them with the notice and other provisions required by GPL as set
 out in the file called "GPL-COPYING" included in this distribution. If you do
 not delete the provisions above, a recipient may use your version of this file
 under the terms of either the MIT license or GPL.

 This License is also included in this distribution in the file called
 "MIT-COPYING".

 EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
 PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


 GPLv2:
 If you wish to use this file under the terms of GPL, following terms are
 effective.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ /*************************************************************************/

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
	VSPM_IP_PAR * ptIpParam,
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

#endif	/* __VSPM_PUBLIC_H__ */
