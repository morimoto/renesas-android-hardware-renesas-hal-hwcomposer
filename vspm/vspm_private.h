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

#ifndef __VSPM_PRIVATE_H__
#define __VSPM_PRIVATE_H__

#include <linux/platform_device.h>

/* Type of the IP */
enum {
	VSPM_TYPE_VSP_CH0 = 1,
	VSPM_TYPE_VSP_CH1,
	VSPM_TYPE_VSP_CH2,
	VSPM_TYPE_VSP_CH3,
	VSPM_TYPE_VSP_CH4,
	VSPM_TYPE_VSP_CH5,
	VSPM_TYPE_VSP_CH6,
	VSPM_TYPE_VSP_CH7,
	VSPM_TYPE_VSP_CH8,
	VSPM_TYPE_VSP_CH9,
	VSPM_TYPE_VSP_CH_MAX,		/* maximum VSP channel number */
	/* 12 - 21 reserved. */
	VSPM_TYPE_2DDMAC_CH0 = 22,
	VSPM_TYPE_2DDMAC_CH1,
	VSPM_TYPE_2DDMAC_CH2,
	VSPM_TYPE_2DDMAC_CH3,
	VSPM_TYPE_2DDMAC_CH4,
	VSPM_TYPE_2DDMAC_CH5,
	VSPM_TYPE_2DDMAC_CH6,
	VSPM_TYPE_2DDMAC_CH7,
	VSPM_TYPE_CH_MAX	/* maximum VSP and 2DDMAC channel number */
};

enum {
	VSPM_TYPE_RPF_CH0 = 0,
	VSPM_TYPE_RPF_CH1,
	VSPM_TYPE_RPF_CH2,
	VSPM_TYPE_RPF_CH3,
	VSPM_TYPE_RPF_CH4,
	VSPM_TYPE_RPF_MAX			/* maximum RPF channel number */
};

enum {
	VSPM_TYPE_UDS_CH0 = 0,
	VSPM_TYPE_UDS_CH1,
	VSPM_TYPE_UDS_CH2,
	VSPM_TYPE_UDS_MAX			/* maximum UDS channel number */
};

/* IP check macro */
#define IS_VSPS_CH(ch) \
	(((ch) == VSPM_TYPE_VSP_CH0) | \
	 ((ch) == VSPM_TYPE_VSP_CH1) | \
	 ((ch) == VSPM_TYPE_VSP_CH2) | \
	 ((ch) == VSPM_TYPE_VSP_CH3))

#define IS_VSPR_CH(ch) \
	(((ch) == VSPM_TYPE_VSP_CH4) | \
	 ((ch) == VSPM_TYPE_VSP_CH5) | \
	 ((ch) == VSPM_TYPE_VSP_CH6) | \
	 ((ch) == VSPM_TYPE_VSP_CH7))

#define IS_VSPD0_CH(ch) \
	((ch) == VSPM_TYPE_VSP_CH8)

#define IS_VSPD1_CH(ch) \
	((ch) == VSPM_TYPE_VSP_CH9)

#define IS_VSP_CH(ch) \
	(IS_VSPS_CH(ch)  | \
	 IS_VSPR_CH(ch)  | \
	 IS_VSPD0_CH(ch) | \
	 IS_VSPD1_CH(ch))

#define IS_2DDMAC_CH(ch) \
	(((ch) == VSPM_TYPE_2DDMAC_CH0) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH1) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH2) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH3) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH4) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH5) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH6) | \
	 ((ch) == VSPM_TYPE_2DDMAC_CH7))

/**
 * vspm_task - VSPM task entry
 * Description: VSPM task entry routine
 */
void vspm_task(void);

/**
 * vspm_lib_driver_initialize - Initialize the VSPM driver
 * Description: Initialize the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_lib_driver_initialize(struct platform_device *pdev);

/**
 * vspm_lib_driver_quit - Exit the VSPM driver
 * Description: Exit the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_lib_driver_quit(void);

/**
 * vspm_lib_entry - Entry of various IP operations
 * @handle:          handle
 * @p_job_id:        destination address of the job id
 * @job_priority:    job priority
 * @p_ip_par:        pointer to IP parameter
 * @user_data:       user data
 * @pfn_complete_cb: pointer to complete callback function
 * Description: Accepts requests to various IP and executed sequentially.
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_QUE_FULL : request queue full
 * R_VSPM_PARAERR : illegal parameter
 * R_VSPM_NG : other errors
 */
long vspm_lib_entry(
	unsigned long handle,
	unsigned long *p_job_id,
	char job_priority,
	VSPM_IP_PAR * p_ip_par,
	unsigned long user_data,
	PFN_VSPM_COMPLETE_CALLBACK pfn_complete_cb);

/**
 * vspm_lib_cancel - Cancel the job
 * @job_id: job id
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_lib_cancel(unsigned long job_id);

/**
 * vspm_lib_forced_cancel - Forced cancel the job
 * @handle: handle
 * Description: Forced cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_lib_forced_cancel(unsigned long handle);

#endif	/* __VSPM_PRIVATE_H__ */
