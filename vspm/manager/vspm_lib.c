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

#include <linux/string.h>

#include "frame.h"
#include "vspm_public.h"
#include "vspm_private.h"
#include "vspm_log.h"
#include "vspm_common.h"
#include "vspm_ip_ctrl.h"


/**
 * vspm_lib_driver_initialize - Initialize the VSPM driver
 * Description: Initialize the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_lib_driver_initialize(struct platform_device *pdev)
{
	long ercd;

	ercd = vspm_ins_vsp_initialize(pdev);
	if (ercd) {
		EPRINT("%s failed to vspm_ins_vsp_initialize %ld\n",
			__func__, ercd);
		goto exit;
	}

	ercd = vspm_ins_2ddmac_initialize(pdev);
	if (ercd) {
		EPRINT("%s failed to vspm_ins_2ddmac_initialize %ld\n",
			__func__, ercd);
		goto exit;
	}

	NPRINT("%s\n", __func__);
	ercd = R_VSPM_OK;
exit:
	return ercd;
}


/**
 * vspm_lib_driver_quit - Exit the VSPM driver
 * Description: Exit the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_lib_driver_quit(void)
{
	long ercd;

	ercd = vspm_ins_vsp_quit();
	if (ercd) {
		EPRINT("%s failed to vspm_ins_vsp_quit %ld\n", __func__, ercd);
		goto exit;
	}

	ercd = vspm_ins_2ddmac_quit();
	if (ercd) {
		EPRINT("%s failed to vspm_ins_2ddmac_quit %ld\n",
			__func__, ercd);
		goto exit;
	}

	NPRINT("%s\n", __func__);
	ercd = R_VSPM_OK;
exit:
	return ercd;
}


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
	VSPM_IP_PAR *p_ip_par,
	unsigned long user_data,
	PFN_VSPM_COMPLETE_CALLBACK pfn_complete_cb)
{
	long ercd;
	struct vspm_api_param_entry entry;

	entry.handle			= handle;
	entry.p_job_id			= p_job_id;
	entry.job_priority		= job_priority;
	entry.p_ip_par			= p_ip_par;
	entry.pfn_complete_cb	= pfn_complete_cb;
	entry.user_data			= user_data;

	/* check entry parameter */
	ercd = vspm_ins_ctrl_entry_param_check(&entry);
	if (ercd) {
		EPRINT("%s failed to vspm_ins_ctrl_entry_param_check %ld\n",
			__func__, ercd);
		return ercd;
	}

	/* entry */
	ercd = fw_send_function(
		TASK_VSPM,
		FUNCTIONID_VSPM_BASE + FUNC_VSPM_ENTRY,
		sizeof(entry),
		&entry);
	if (ercd)
		return ercd;

	/* dispatch */
	(void)fw_send_event(
		TASK_VSPM, FUNCTIONID_VSPM_BASE + EVENT_VSPM_DISPATCH, 0, NULL);

	return R_VSPM_OK;
}


/**
 * vspm_lib_cancel - Cancel the job
 * @job_id: job id
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_lib_cancel(unsigned long job_id)
{
	return fw_send_function(
		TASK_VSPM,
		FUNCTIONID_VSPM_BASE + FUNC_VSPM_CANCEL,
		sizeof(job_id),
		&job_id);
}


/**
 * vspm_lib_forced_cancel - Forced cancel the job
 * @handle: handle
 * Description: Forced cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_lib_forced_cancel(unsigned long handle)
{
	return fw_send_function(
		TASK_VSPM,
		FUNCTIONID_VSPM_BASE + FUNC_VSPM_FORCED_CANCEL,
		sizeof(handle),
		&handle);
}


/**
 * vspm_lib_get_status - Get the state of the IP processing
 * @job_id: job id
 * Description: Get the state of the IP processing
 * Returns: The following status code is returned.
 * VSPM_STATUS_WAIT:     waiting
 * VSPM_STATUS_ACTIVE    running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_lib_get_status(unsigned long job_id)
{
	return fw_send_function(
		TASK_VSPM,
		FUNCTIONID_VSPM_BASE + FUNC_VSPM_GET_STATUS,
		sizeof(job_id),
		&job_id);
}


