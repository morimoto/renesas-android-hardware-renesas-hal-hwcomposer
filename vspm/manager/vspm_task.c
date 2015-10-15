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
#include "vspm_task_private.h"


/* function table */
static struct fw_func_tbl g_vspm_func_tbl[] = {
	[FUNC_TASK_INIT - 1] = {
		.func_id = FUNC_TASK_INIT,
		.msg_id	 = MSG_FUNCTION,
		.func	 = vspm_inm_init
	},
	[FUNC_TASK_QUIT - 1] = {
		.func_id = FUNC_TASK_QUIT,
		.msg_id	 = MSG_FUNCTION,
		.func	 = vspm_inm_quit
	},
	[FUNC_VSPM_ENTRY - 1] = {
		.func_id = FUNCTIONID_VSPM_BASE + FUNC_VSPM_ENTRY,
		.msg_id	 = MSG_FUNCTION,
		.func	 = vspm_inm_entry
	},
	[EVENT_VSPM_DRIVER_ON_COMPLETE - 1] = {
		.func_id = FUNCTIONID_VSPM_BASE + EVENT_VSPM_DRIVER_ON_COMPLETE,
		.msg_id	 = MSG_EVENT,
		.func	 = vspm_inm_driver_on_complete
	},
	[FUNC_VSPM_GET_STATUS - 1] = {
		.func_id = FUNCTIONID_VSPM_BASE + FUNC_VSPM_GET_STATUS,
		.msg_id	 = MSG_FUNCTION,
		.func	 = vspm_inm_get_status
	},
	[FUNC_VSPM_CANCEL - 1] = {
		.func_id = FUNCTIONID_VSPM_BASE + FUNC_VSPM_CANCEL,
		.msg_id	 = MSG_FUNCTION,
		.func	 = vspm_inm_cancel
	},
	[FUNC_VSPM_FORCED_CANCEL - 1] = {
		.func_id = FUNCTIONID_VSPM_BASE + FUNC_VSPM_FORCED_CANCEL,
		.msg_id	 = MSG_FUNCTION,
		.func	 = vspm_inm_forced_cancel
	},
	[EVENT_VSPM_DISPATCH - 1] = {
		.func_id = FUNCTIONID_VSPM_BASE + EVENT_VSPM_DISPATCH,
		.msg_id	 = MSG_EVENT,
		.func	 = vspm_inm_dispatch
	},
	[EVENT_VSPM_MAX - 1] = {
		.func_id = 0,
		.msg_id	 = 0,
		.func	 = NULL
	}
};

/*
 * vspm_task - VSPM task entry
 * Description: VSPM task entry
 */
void vspm_task(void)
{
	/* start the processing framework */
	fw_execute(TASK_VSPM, g_vspm_func_tbl);
}


/*
 * vspm_inm_init - Initialize the VSPM task
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Initialize the VSPM task
 * Returns: On success FW_OK is returned. On error, FW_NG is returned.
 */
long vspm_inm_init(void *mesp, void *para)
{
	long ercd;

	RESERVED(mesp);
	RESERVED(para);

	/* Initialize the VSPM driver control information */
	ercd = vspm_ins_ctrl_initialize();
	if (ercd) {
		EPRINT("%s failed to vspm_ins_ctrl_initialize %ld\n",
			__func__, ercd);
		return FW_NG;
	}

	return FW_OK;
}


/*
 * vspm_inm_quit - Exit the VSPM task
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Exit the VSPM task
 * Returns: On success FW_OK is returned. On error, FW_NG is returned.
 */
long vspm_inm_quit(void *mesp, void *para)
{
	long ercd;

	RESERVED(mesp);
	RESERVED(para);

	/* Exit the VSPM driver */
	ercd = vspm_ins_ctrl_quit();
	if (ercd) {
		EPRINT("%s failed to vspm_ins_ctrl_quit %ld\n", __func__, ercd);
		return FW_NG;
	}

	return FW_OK;
}


/*
 * vspm_inm_entry - Entry of various IP operations
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Accepts requests to various IP and executed sequentially.
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_QUE_FULL : request queue full
 * R_VSPM_PARAERR : illegal parameter
 * R_VSPM_NG : other errors
 */
long vspm_inm_entry(void *mesp, void *para)
{
	long ercd;

	RESERVED(mesp);

	ercd = vspm_ins_ctrl_entry((struct vspm_api_param_entry *)para);
	if (ercd) {
		EPRINT("%s failed to vspm_ins_ctrl_entry %ld\n",
			__func__, ercd);
	}

	return ercd;
}


/*
 * vspm_inm_driver_on_complete - IP operations completion
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: IP operations completion
 * Returns: FW_OK is returned.
 */
long vspm_inm_driver_on_complete(void *mesp, void *para)
{
	long ercd;
	struct vspm_api_param_on_complete *api_param =
		(struct vspm_api_param_on_complete *)para;

	RESERVED(mesp);

	ercd = vspm_ins_ctrl_on_complete(
		api_param->module_id, api_param->result);
	if (ercd) {
		EPRINT("%s failed to vspm_ins_ctrl_on_complete %ld\n",
			__func__, ercd);
	}
	return FW_OK;
}


/*
 * vspm_inm_get_status - Get the state of the IP processing
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Get the state of the IP processing
 * Returns: The following status code is returned.
 * VSPM_STATUS_WAIT:     waiting
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_inm_get_status(void *mesp, void *para)
{
	RESERVED(mesp);

	return vspm_ins_ctrl_get_status(*(unsigned long *)para);
}


/*
 * vspm_inm_cancel - Cancel the job
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_inm_cancel(void *mesp, void *para)
{
	RESERVED(mesp);

	return vspm_ins_ctrl_cancel(*(unsigned long *)para);
}


/*
 * vspm_inm_forced_cancel - Forced cancel the job
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Forced cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_inm_forced_cancel(void *mesp, void *para)
{
	RESERVED(mesp);

	return vspm_ins_ctrl_forced_cancel(*(unsigned long *)para);
}


/*
 * vspm_inm_dispatch - Execute the scheduling and processing
 * @mesp: pointer to the receive message header
 * @para: pointer to the receive message parameter
 * Description: Execute the scheduling and processing
 * Returns: FW_OK is returned.
 */
long vspm_inm_dispatch(void *mesp, void *para)
{
	RESERVED(mesp);
	RESERVED(para);

	vspm_ins_ctrl_dispatch();
	return FW_OK;
}

