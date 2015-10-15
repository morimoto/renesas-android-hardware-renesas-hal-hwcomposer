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

struct vspm_ctrl_info g_vspm_ctrl_info;

/*
 * vspm_ins_ctrl_initialize - Initialize the VSPM driver control information
 * Description: Initialize the VSPM driver control information
 * Returns: On success R_VSPM_OK is returned. On error, an error number is
 * returned, except R_VSPM_OK.
 */
long vspm_ins_ctrl_initialize(void)
{
	struct vspm_usable_info *usable = &g_vspm_ctrl_info.usable_info;
	long ercd;

	/* clear the VSPM driver control information table */
	memset(&g_vspm_ctrl_info, 0, sizeof(g_vspm_ctrl_info));

	/* initialize the queue information table */
	ercd = vspm_inc_sort_queue_initialize(&g_vspm_ctrl_info.queue_info);
	if (R_VSPM_OK != ercd) {
		EPRINT("failed to vspm_inc_sort_queue_initialize %ld\n", ercd);
		return ercd;
	}

	/* assaigned usable channel */
	usable->ch_bits =
		VSPM_IPCTRL_VSPS_WPF_CH0|
		VSPM_IPCTRL_VSPS_WPF_CH1|
		VSPM_IPCTRL_VSPS_WPF_CH2|
		VSPM_IPCTRL_VSPS_WPF_CH3|
#ifdef USE_VSPR
		VSPM_IPCTRL_VSPR_WPF_CH0|
		VSPM_IPCTRL_VSPR_WPF_CH1|
		VSPM_IPCTRL_VSPR_WPF_CH2|
		VSPM_IPCTRL_VSPR_WPF_CH3|
#endif
#ifdef USE_VSPD0
		VSPM_IPCTRL_VSPD0_WPF_CH0|
#endif
#ifdef USE_VSPD1
		VSPM_IPCTRL_VSPD1_WPF_CH0|
#endif
		VSPM_IPCTRL_2DDMAC_CH0|
		VSPM_IPCTRL_2DDMAC_CH1|
		VSPM_IPCTRL_2DDMAC_CH2|
		VSPM_IPCTRL_2DDMAC_CH3|
		VSPM_IPCTRL_2DDMAC_CH4|
		VSPM_IPCTRL_2DDMAC_CH5|
		VSPM_IPCTRL_2DDMAC_CH6|
		VSPM_IPCTRL_2DDMAC_CH7;

	usable->vsp_module_bits[VSPM_IP_VSPS] =
		VSP_SRU_USE|
		VSP_UDS_USE|
#ifdef USE_VSPS_UDS
		VSP_UDS1_USE|
		VSP_UDS2_USE|
#endif
		VSP_LUT_USE|
		VSP_CLU_USE|
		VSP_HST_USE|
		VSP_HSI_USE|
		VSP_BRU_USE|
		VSP_HGO_USE|
		VSP_HGT_USE;

	usable->vsp_rpf_bits[VSPM_IP_VSPS] =
		VSPM_IPCTRL_VSP_RPF_CH0|
		VSPM_IPCTRL_VSP_RPF_CH1|
		VSPM_IPCTRL_VSP_RPF_CH2|
		VSPM_IPCTRL_VSP_RPF_CH3|
		VSPM_IPCTRL_VSP_RPF_CH4;

	usable->vsp_rpf_clut_bits[VSPM_IP_VSPS] =
		VSPM_IPCTRL_VSP_RPF_CH1|
		VSPM_IPCTRL_VSP_RPF_CH2;

	usable->vsp_uds_bits[VSPM_IP_VSPS] =
		VSPM_IPCTRL_VSP_UDS_CH0;
#ifdef USE_VSPS_UDS
	usable->vsp_uds_bits[VSPM_IP_VSPS] |=
		VSPM_IPCTRL_VSP_UDS_CH1|
		VSPM_IPCTRL_VSP_UDS_CH2;
#endif

#ifdef USE_VSPR
	usable->vsp_module_bits[VSPM_IP_VSPR] =
		VSP_SRU_USE|
		VSP_UDS_USE|
		VSP_HST_USE|
		VSP_HSI_USE|
		VSP_BRU_USE;

	usable->vsp_rpf_bits[VSPM_IP_VSPR] =
		VSPM_IPCTRL_VSP_RPF_CH0|
		VSPM_IPCTRL_VSP_RPF_CH1|
		VSPM_IPCTRL_VSP_RPF_CH2|
		VSPM_IPCTRL_VSP_RPF_CH3|
		VSPM_IPCTRL_VSP_RPF_CH4;

	usable->vsp_rpf_clut_bits[VSPM_IP_VSPR] =
		VSPM_IPCTRL_VSP_RPF_CH2;

	usable->vsp_uds_bits[VSPM_IP_VSPR] =
		VSPM_IPCTRL_VSP_UDS_CH0;
#endif

#ifdef USE_VSPD0
	usable->vsp_module_bits[VSPM_IP_VSPD0] =
		VSP_UDS_USE|
		VSP_LUT_USE|
		VSP_HST_USE|
		VSP_HSI_USE|
		VSP_BRU_USE|
		VSP_HGO_USE;

	usable->vsp_rpf_bits[VSPM_IP_VSPD0] =
		VSPM_IPCTRL_VSP_RPF_CH0|
		VSPM_IPCTRL_VSP_RPF_CH1|
		VSPM_IPCTRL_VSP_RPF_CH2|
		VSPM_IPCTRL_VSP_RPF_CH3;

	usable->vsp_rpf_clut_bits[VSPM_IP_VSPD0] =
		VSPM_IPCTRL_VSP_RPF_CH2;

	usable->vsp_uds_bits[VSPM_IP_VSPD0] =
		VSPM_IPCTRL_VSP_UDS_CH0;
#endif

#ifdef USE_VSPD1
	usable->vsp_module_bits[VSPM_IP_VSPD1] =
		VSP_UDS_USE|
		VSP_LUT_USE|
		VSP_HST_USE|
		VSP_HSI_USE|
		VSP_BRU_USE|
		VSP_HGO_USE;

	usable->vsp_rpf_bits[VSPM_IP_VSPD1] =
		VSPM_IPCTRL_VSP_RPF_CH0|
		VSPM_IPCTRL_VSP_RPF_CH1|
		VSPM_IPCTRL_VSP_RPF_CH2|
		VSPM_IPCTRL_VSP_RPF_CH3;

	usable->vsp_rpf_clut_bits[VSPM_IP_VSPD1] =
		VSPM_IPCTRL_VSP_RPF_CH2;

	usable->vsp_uds_bits[VSPM_IP_VSPD1] =
		VSPM_IPCTRL_VSP_UDS_CH0;
#endif

	IPRINT("usable channel = %08lx\n", usable->ch_bits);

	return R_VSPM_OK;
}


/*
 * vspm_ins_ctrl_quit - Exit the VSPM driver
 * Description: Exit the VSPM driver
 * Returns: R_VSPM_OK is returned.
 */
long vspm_ins_ctrl_quit(void)
{
	return R_VSPM_OK;
}


/*
 * vspm_ins_ctrl_entry - Job registration
 * @entry: parameter of job entry
 * Description: Job registration
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_QUE_FULL : request queue full
 * R_VSPM_PARAERR : illegal parameter
 * R_VSPM_NG : other errors
 */
long vspm_ins_ctrl_entry(struct vspm_api_param_entry *entry)
{
	struct vspm_job_info *job_info;

	long ercd;

	/* Register in the job management table */
	job_info = vspm_ins_job_entry(&g_vspm_ctrl_info.job_manager, entry);
	if (job_info == NULL) {
		EPRINT("failed to vspm_ins_job_entry\n");
		return R_VSPM_QUE_FULL;
	}

	/* Add a job information to the queue */
	ercd = vspm_inc_sort_queue_entry(
		&g_vspm_ctrl_info.queue_info, job_info);
	if (R_VSPM_OK != ercd) {
		/* Remove a job */
		vspm_ins_job_remove(job_info);
		EPRINT("failed to vspm_inc_sort_queue_entry %ld\n", ercd);
		return R_VSPM_NG;
	}

	/* Save the job id */
	*entry->p_job_id = vspm_ins_job_get_job_id(job_info);

	NPRINT("%s job_id=0x%08lX\n", __func__, *entry->p_job_id);
	return R_VSPM_OK;
}


/*
 * vspm_ins_ctrl_on_complete - IP operations completion
 * @module_id: module id
 * @result:    processing result
 * Description: IP operations completion
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_NG:      abnormal termination
 * R_VSPM_PARAERR: illegal parameter
 * R_VSPM_SEQERR:  sequence error
 */
long vspm_ins_ctrl_on_complete(unsigned short module_id, long result)
{
	struct vspm_job_info *job_info;
	unsigned long channel_bit;

	long ercd;

	IPRINT("---- %s (%d, %ld)\n", __func__, module_id, result);

	channel_bit = (unsigned long)(0x00000001UL << module_id);

	/* Check the parameter */
	if (!(g_vspm_ctrl_info.usable_info.ch_bits & channel_bit)) {
		EPRINT("%s Invalid module_id\n", __func__);
		return R_VSPM_PARAERR;
	}

	/* Get job information of the job that is running */
	job_info = vspm_ins_exec_get_current_job_info(
		&g_vspm_ctrl_info.exec_info, module_id);
	if (job_info == NULL) {
		IPRINT("%s already completed!!\n", __func__);
		ercd = R_VSPM_OK;
		goto dispatch;
	}

	if (vspm_ins_job_get_status(job_info) != VSPM_JOB_STATUS_EXECUTING) {
		IPRINT("%s already completed!!\n", __func__);
		ercd = R_VSPM_OK;
		goto dispatch;
	}

	/* Notification that the IP processing is complete */
	ercd = vspm_ins_exec_complete(&g_vspm_ctrl_info.exec_info, module_id);
	if (ercd) {
		EPRINT("failed to vspm_ins_exec_complete %ld\n", ercd);
		goto dispatch;
	}

	/* Inform the completion of the job to the job management */
	ercd = vspm_ins_job_execute_complete(job_info, result, module_id);
	if (ercd) {
		EPRINT("failed to vspm_ins_job_execute_complete %ld\n", ercd);
		goto dispatch;
	}

dispatch:
	/* One job is completed, execute the next job */
	if (vspm_inc_sort_queue_get_count(&g_vspm_ctrl_info.queue_info) > 0)
		vspm_ins_ctrl_dispatch();

	return ercd;
}


/*
 * vspm_ins_ctrl_get_status - Get the state of the IP processing
 * @job_id: job id
 * Description: Get the state of the IP processing
 * Returns: The following status code is returned.
 * VSPM_STATUS_WAIT:     waiting
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_ins_ctrl_get_status(unsigned long job_id)
{
	struct vspm_job_info *job_info;
	unsigned long status;

	long rtncd = VSPM_STATUS_NO_ENTRY;

	/* Search a job information */
	job_info = vspm_ins_job_find_job_info(
		&g_vspm_ctrl_info.job_manager, job_id);
	if (job_info != NULL) {
		/* Get a job status */
		status = vspm_ins_job_get_status(job_info);
		if (status == VSPM_JOB_STATUS_ENTRY)
			rtncd = VSPM_STATUS_WAIT;
		else if (status == VSPM_JOB_STATUS_EXECUTING)
			rtncd = VSPM_STATUS_ACTIVE;
	}

	return rtncd;
}


/*
 * vspm_ins_ctrl_cancel - Cancel the job
 * @job_id: job id
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long vspm_ins_ctrl_cancel(unsigned long job_id)
{
	struct vspm_job_info *job_info;
	unsigned long status;

	long rtncd = VSPM_STATUS_NO_ENTRY;

	/* Search a job information */
	job_info = vspm_ins_job_find_job_info(
		&g_vspm_ctrl_info.job_manager, job_id);
	if (job_info != NULL) {
		status = vspm_ins_job_get_status(job_info);
		if (status == VSPM_JOB_STATUS_EXECUTING) {
			rtncd = VSPM_STATUS_ACTIVE;
		} else if (status == VSPM_JOB_STATUS_ENTRY) {
			unsigned short index;

			/* Search a job information from queue */
			rtncd = vspm_inc_sort_queue_find_item(
				&g_vspm_ctrl_info.queue_info, job_info, &index);
			if (rtncd == R_VSPM_OK) {
				/* Remove a job information from queue */
				(void)vspm_inc_sort_queue_remove(
					&g_vspm_ctrl_info.queue_info, index);
			}

			/* Cancel the job */
			(void)vspm_ins_job_cancel(job_info);

			rtncd = R_VSPM_OK;
		}
	}

	NPRINT("%s job_id=0x%08lx ercd=%ld\n", __func__, job_id, rtncd);
	return rtncd;
}


/*
 * vspm_ins_ctrl_forced_cancel - Forced cancel the job
 * @handle: handle
 * Description: Forced cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_ctrl_forced_cancel(unsigned long handle)
{
	struct vspm_job_info *job_info;

	long ercd;
	int i;

	job_info = &g_vspm_ctrl_info.job_manager.job_info[0];
	for (i = 0; i < VSPM_MAX_ELEMENTS; i++) {
		if ((job_info->entry.handle == handle) &&
				(job_info->status != VSPM_JOB_STATUS_EMPTY)) {

			if (job_info->status == VSPM_JOB_STATUS_ENTRY) {
				unsigned short index;

				/* Search a job information from queue */
				ercd = vspm_inc_sort_queue_find_item(
					&g_vspm_ctrl_info.queue_info,
					job_info,
					&index);
				if (ercd == R_VSPM_OK) {
					/* Remove a job info from queue */
					(void)vspm_inc_sort_queue_remove(
						&g_vspm_ctrl_info.queue_info,
						index);
				}

				/* Cancel the job */
				(void)vspm_ins_job_cancel(job_info);
			} else if (job_info->status ==
					VSPM_JOB_STATUS_EXECUTING) {
				/* Cancel of executing IP */
				ercd = vspm_ins_exec_cancel(
					&g_vspm_ctrl_info.exec_info,
					job_info->ch_num);
				if (ercd) {
					EPRINT(
						"failed to vspm_ins_exec_cancel %ld\n",
						ercd);
					return ercd;
				}

				/* Calcel the executing job */
				(void)vspm_ins_job_execute_complete(
					job_info,
					R_VSPM_CANCEL,
					job_info->ch_num);
			}
		}

		job_info++;
	}

	NPRINT("%s handle=0x%08lx\n", __func__, handle);
	return R_VSPM_OK;
}

/*
 * vspm_ins_ctrl_entry_param_check - Check the entry parameter
 * @entry: pointer to entry API parameter
 * Description: Check the entry parameter
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_PARAERR: parameter error
 */
long vspm_ins_ctrl_entry_param_check(struct vspm_api_param_entry *entry)
{
	struct vspm_usable_info *usable = &g_vspm_ctrl_info.usable_info;
	VSPM_IP_PAR *p_ip_par = entry->p_ip_par;

	unsigned short ch;

	/* Check the parameter */
	if (entry->p_job_id == NULL) {
		EPRINT("%s p_job_id is NULL\n", __func__);
		return R_VSPM_PARAERR;
	}
	if ((entry->job_priority < VSPM_PRI_MIN) ||
		(VSPM_PRI_MAX < entry->job_priority)) {
		EPRINT("%s Invalid job_priority %d\n",
			__func__, entry->job_priority);
		return R_VSPM_PARAERR;
	}
	if (entry->pfn_complete_cb == NULL) {
		EPRINT("%s pfn_complete_cb is NULL\n", __func__);
		return R_VSPM_PARAERR;
	}
	if (entry->p_ip_par == NULL) {
		EPRINT("%s p_ip_par is NULL\n", __func__);
		return R_VSPM_PARAERR;
	}
	if (p_ip_par->uhType == VSPM_TYPE_2DDMAC_AUTO) {
		if (p_ip_par->unionIpParam.pt2dDmac == NULL) {
			EPRINT("%s unionIpParam.pt2dDmac is NULL\n", __func__);
			return R_VSPM_PARAERR;
		}
	} else {
		if ((p_ip_par->uhType == VSPM_TYPE_VSP_AUTO) ||
			(p_ip_par->uhType &
				(VSPM_TYPE_VSP_VSPS |
				 VSPM_TYPE_VSP_VSPR |
				 VSPM_TYPE_VSP_VSPD0 |
				 VSPM_TYPE_VSP_VSPD1))) {
			if (p_ip_par->unionIpParam.ptVsp == NULL) {
				EPRINT("%s unionIpParam.ptVsp is NULL\n",
					__func__);
				return R_VSPM_PARAERR;
			}
		} else {
			EPRINT("%s Illegal uhType 0x%04x\n",
				__func__, p_ip_par->uhType);
			return R_VSPM_PARAERR;
		}
	}

	/* Pre assign channel */
	ch = vspm_ins_ctrl_assign_channel(p_ip_par, usable);
	if (ch == VSPM_TYPE_CH_MAX) {
		EPRINT("%s can not auto assign uhType=%d\n",
			__func__, p_ip_par->uhType);
		return R_VSPM_PARAERR;
	}

	return R_VSPM_OK;
}


/*
 * vspm_ins_ctrl_dispatch - Execute the scheduling and processing
 * Description: Execute the scheduling and processing
 */
void vspm_ins_ctrl_dispatch(void)
{
	/* queue count */
	unsigned short queue_count;

	/* available channel bits */
	struct vspm_usable_info usable;

	long ercd;

	/* Get the number of entry */
	queue_count = vspm_inc_sort_queue_get_count(
		&g_vspm_ctrl_info.queue_info);
	DPRINT("vspm_ins_ctrl_dispatch: queue_count=%d\n", queue_count);

	while (queue_count) {
		struct vspm_job_info *job_info;
		VSPM_IP_PAR *p_ip_par;
		unsigned short assign_ch;

		unsigned long use_module_bit;
		unsigned long use_rpf_bit;
		unsigned long rpf_order = VSP_RPF_NO_USE;

		/* Set the channel available */
		usable = g_vspm_ctrl_info.usable_info;

		/* Update the execution status of the current module */
		vspm_ins_exec_update_current_status(
			&g_vspm_ctrl_info.exec_info, &usable);

		/* Get a 1st job information from queue */
		ercd = vspm_inc_sort_queue_refer(
			&g_vspm_ctrl_info.queue_info, 0, &job_info);
		if (ercd) {
			/* not found 1st Job */
			return;
		}

		/* Get IP parameter */
		p_ip_par = vspm_ins_job_get_param(job_info);

		/* assign channel */
		assign_ch = vspm_ins_ctrl_assign_channel(p_ip_par, &usable);
		if (assign_ch == VSPM_TYPE_CH_MAX) {
			/* not assigned */
			return;
		}

		if (p_ip_par->uhType != VSPM_TYPE_2DDMAC_AUTO) {
			/* re-connect UDS module */
			ercd = vspm_ins_ctrl_reconnect_uds_module(
				assign_ch,
				p_ip_par->unionIpParam.ptVsp,
				&usable);
			if (ercd != R_VSPM_OK) {
				/* can not reconnecting UDS module */
				return;
			}

			use_module_bit =
				p_ip_par->unionIpParam.ptVsp->use_module;

			/* assign RPF channel */
			ercd = vspm_ins_ctrl_assign_vsp_rpf(
				assign_ch,
				p_ip_par->unionIpParam.ptVsp,
				&usable,
				&use_rpf_bit,
				&rpf_order);
			if (ercd == VSP_ASSIGN_NG) {
				/* RPF channel is not assigned */
				return;
			}
		} else {	/* uhType == VSPM_TYPE_2DDMAC_AUTO */
			use_module_bit = 0;			/* not used */
			use_rpf_bit = 0;			/* not used */
			rpf_order = VSP_RPF_NO_USE;	/* not used */
		}

		/* Remove a job information from queue */
		(void)vspm_inc_sort_queue_remove(
			&g_vspm_ctrl_info.queue_info, 0);

		/* Get the number of entry */
		queue_count = vspm_inc_sort_queue_get_count(
			&g_vspm_ctrl_info.queue_info);

		/* Inform the start of the job to the job management */
		(void)vspm_ins_job_execute_start(job_info, assign_ch);

		/* Start the process */
		ercd = vspm_ins_exec_start(
			&g_vspm_ctrl_info.exec_info,
			assign_ch,
			p_ip_par,
			job_info,
			use_module_bit,
			use_rpf_bit,
			rpf_order);
		if (ercd) {
			EPRINT("failed to vspm_ins_exec_start");
			EPRINT("ercd=%ld, assign_ch=%d\n", ercd, assign_ch);

			/* Info the comp of the job to the job management */
			(void)vspm_ins_job_execute_complete(
				job_info, ercd, assign_ch);
		}
	}
}


/*
 * vspm_ins_ctrl_assign_channel - Assign the channel
 * @ip_par:            IP parameter structure
 * @usable:            assignable bits information structure
 * Description:        Assign the channel
 * Returns:            assigned channel is returned.
 *  VSPM_TYPE_CH_MAX is not assinged.
 */
unsigned short vspm_ins_ctrl_assign_channel(
	VSPM_IP_PAR *ip_par, struct vspm_usable_info *usable)
{
	unsigned short ch = VSPM_TYPE_CH_MAX;

	if (ip_par->uhType != VSPM_TYPE_2DDMAC_AUTO) {
		unsigned long assignable;

		assignable = vspm_ins_ctrl_assign_vsp_module(
			ip_par->unionIpParam.ptVsp, usable);

		/* priority VSPR > VSPD0 > VSPD1 > VSPS (When use AUTO) */
		/* select VSPS channel */
		if ((ip_par->uhType == VSPM_TYPE_VSP_AUTO) ||
			(ip_par->uhType & VSPM_TYPE_VSP_VSPS)) {
			/* priority CH3 > CH2 > CH1 > CH0 */
			if (assignable & VSPM_IPCTRL_VSPS_WPF_CH3)
				ch = VSPM_TYPE_VSP_CH3;
			else if (assignable & VSPM_IPCTRL_VSPS_WPF_CH2)
				ch = VSPM_TYPE_VSP_CH2;
			else if (assignable & VSPM_IPCTRL_VSPS_WPF_CH1)
				ch = VSPM_TYPE_VSP_CH1;
			else if (assignable & VSPM_IPCTRL_VSPS_WPF_CH0)
				ch = VSPM_TYPE_VSP_CH0;
		}

		/* select VSPD1 channel */
		if ((ip_par->uhType == VSPM_TYPE_VSP_AUTO) ||
			(ip_par->uhType & VSPM_TYPE_VSP_VSPD1)) {
			/* priority CH0 */
			if (assignable & VSPM_IPCTRL_VSPD1_WPF_CH0)
				ch = VSPM_TYPE_VSP_CH9;
		}

		/* select VSPD0 channel */
		if ((ip_par->uhType == VSPM_TYPE_VSP_AUTO) ||
			(ip_par->uhType & VSPM_TYPE_VSP_VSPD0)) {
			/* check assignment channel */
			/* priority CH0 */
			if (assignable & VSPM_IPCTRL_VSPD0_WPF_CH0)
				ch = VSPM_TYPE_VSP_CH8;
		}

		/* select VSPR channel */
		if ((ip_par->uhType == VSPM_TYPE_VSP_AUTO) ||
			(ip_par->uhType & VSPM_TYPE_VSP_VSPR)) {
			/* check assignment channel */

			/* priority CH3 > CH2 > CH1 > CH0 */
			if (assignable & VSPM_IPCTRL_VSPR_WPF_CH3)
				ch = VSPM_TYPE_VSP_CH7;
			else if (assignable & VSPM_IPCTRL_VSPR_WPF_CH2)
				ch = VSPM_TYPE_VSP_CH6;
			else if (assignable & VSPM_IPCTRL_VSPR_WPF_CH1)
				ch = VSPM_TYPE_VSP_CH5;
			else if (assignable & VSPM_IPCTRL_VSPR_WPF_CH0)
				ch = VSPM_TYPE_VSP_CH4;
		}
	} else {
		/* select 2DDMAC channel */
		/* priority CH7 > CH3 > CH6 > CH2 > CH5 > CH1 > CH4 > CH0 */
		if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH7)
			ch = VSPM_TYPE_2DDMAC_CH7;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH3)
			ch = VSPM_TYPE_2DDMAC_CH3;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH6)
			ch = VSPM_TYPE_2DDMAC_CH6;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH2)
			ch = VSPM_TYPE_2DDMAC_CH2;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH5)
			ch = VSPM_TYPE_2DDMAC_CH5;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH1)
			ch = VSPM_TYPE_2DDMAC_CH1;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH4)
			ch = VSPM_TYPE_2DDMAC_CH4;
		else if (usable->ch_bits & VSPM_IPCTRL_2DDMAC_CH0)
			ch = VSPM_TYPE_2DDMAC_CH0;
	}

	if (ch == VSPM_TYPE_CH_MAX) {
		IPRINT("%s channel is not assigned.\n", __func__);
		IPRINT(" type=%d, assignable=0x%08lx\n",
			ip_par->uhType, usable->ch_bits);
	}

	return ch;
}


/*
 * vspm_ins_ctrl_count_bits - Count bits
 * @bits: bits
 * Description: Count bits
 * Returns: bit count is returned.
 */
static unsigned char vspm_ins_ctrl_count_bits(unsigned long bits)
{
	unsigned char cnt = 0;

	bits &= 0xffff;
	while (bits) {
		bits &= (bits - 1);
		cnt++;
	}

	return cnt;
}


/*
 * vspm_ins_ctrl_assign_vsp_rpf - Assign the RPF module
 * @ch:                 VSP channel number
 * @vsp_par:            VSP parameter
 * @usable:             Assignable bit information structure
 * @p_vsp_rpf:          assigned RPF module
 * @p_rpf_order:        RPF alignment settings(output)
 * Description: Assign the RPF module
 * Returns: The following assignment result code is returned.
 * VSP_ASSIGN_OK : OK
 * VSP_ASSIGN_NG : NG
 */
long vspm_ins_ctrl_assign_vsp_rpf(
	unsigned short ch,
	VSPM_VSP_PAR *vsp_par,
	struct vspm_usable_info *usable,
	unsigned long *p_vsp_rpf,
	unsigned long *p_rpf_order)
{
	unsigned long assign_bits = 0;
	unsigned long bit;

	unsigned long order = 0;

	unsigned long available_not_clut_bits;
	unsigned long available_clut_bits;

	unsigned long shift_tbl[4] = {0, 8, 16, 24};
	T_VSP_IN * vsp_in[4];

	unsigned char num = 0;
	long ercd;

	/* set the pointer to the input image */
	vsp_in[0] = vsp_par->src1_par;
	vsp_in[1] = vsp_par->src2_par;
	vsp_in[2] = vsp_par->src3_par;
	vsp_in[3] = vsp_par->src4_par;

	/* get VSP IP number */
	ercd = vspm_inc_get_vsp_num(&num, ch);
	if (ercd != R_VSPM_OK)
		return VSP_ASSIGN_NG;

	available_not_clut_bits =
		usable->vsp_rpf_bits[num] & ~usable->vsp_rpf_clut_bits[num];
	available_clut_bits =
		usable->vsp_rpf_bits[num] &  usable->vsp_rpf_clut_bits[num];

	for (num = 0; num < vsp_par->rpf_num; num++) {
		unsigned char use_rpf_flag = 1;
		unsigned char use_rpf_clut_flag = 1;

		if (num >= 4) {
			/* rpf_num = 4 or over is not supported. Stop assign. */
			break;
		}

		if (vsp_in[num] != NULL) {
			if ((vsp_in[num]->format == VSP_IN_RGB_CLUT_DATA) ||
				(vsp_in[num]->format == VSP_IN_YUV_CLUT_DATA)) {
				/* If using CLUT of RPF, */
				/* RPF unsupported CLUT can not be used. */
				use_rpf_flag = 0;
			}
		}

		/* searching bit of RPF unsupported CLUT. */
		if (use_rpf_flag) {
			bit = available_not_clut_bits &
				~(available_not_clut_bits-1);
			if (bit != 0) {
				assign_bits |= bit;
				available_not_clut_bits &= ~bit;

				order |= ((unsigned long)
					vspm_ins_ctrl_count_bits(bit-1))
						<< shift_tbl[num];

				use_rpf_clut_flag = 0;
			}
		}

		/* searching bit of RPF supported CLUT */
		if (use_rpf_clut_flag) {
			bit = available_clut_bits & ~(available_clut_bits-1);
			if (bit != 0) {
				assign_bits |= bit;
				available_clut_bits &= ~bit;

				order |= ((unsigned long)
					vspm_ins_ctrl_count_bits(bit-1))
						<< shift_tbl[num];
			} else {
				IPRINT("%s RPF channel is not assigned!!\n",
					__func__);
				return VSP_ASSIGN_NG;
			}
		}
	}

	*p_vsp_rpf = assign_bits;
	*p_rpf_order = order;

	DPRINT("assign_bits=%04x, rpf_order=%08x\n",
		(unsigned int)assign_bits, (unsigned int)order);
	return VSP_ASSIGN_OK;
}


/*
 * vspm_ins_ctrl_get_rpf_clut_count - Get using RPF(CLUT) count
 * @vsp_par: VSP parameter structure
 * Description: Get using RPF(CLUT) count
 * Returns: using RPF(CLUT) count is returned.
 */
static unsigned char vspm_ins_ctrl_get_rpf_clut_count(VSPM_VSP_PAR *vsp_par)
{
	T_VSP_IN * vsp_in[4];
	unsigned char cnt = 0;

	int i;

	/* set the pointer to the input image */
	vsp_in[0] = vsp_par->src1_par;
	vsp_in[1] = vsp_par->src2_par;
	vsp_in[2] = vsp_par->src3_par;
	vsp_in[3] = vsp_par->src4_par;

	for (i = 0; i < 4; i++) {
		if (vsp_in[i] != NULL) {
			if ((vsp_in[i]->format == VSP_IN_RGB_CLUT_DATA) ||
				(vsp_in[i]->format == VSP_IN_YUV_CLUT_DATA)) {
				cnt++;
			}
		}
	}

	return cnt;
}


/*
 * vspm_ins_ctrl_get_uds_count - Get UDS channel count
 * @module_bits:        module bits
 * Description: Get UDS channel count
 * Returns: UDS channel count is returned.
 */
static unsigned char vspm_ins_ctrl_get_uds_count(unsigned long module_bits)
{
	unsigned char cnt = 0;

	if (module_bits & VSP_UDS_USE)
		cnt++;

	if (module_bits & VSP_UDS1_USE)
		cnt++;

	if (module_bits & VSP_UDS2_USE)
		cnt++;

	return cnt;
}


/*
 * vspm_ins_ctrl_assign_vsp_module - Assign VSP module
 * @vsp_par:           VSP parameter structure
 * @usable:            assignable bits information structure
 * Description: Assign VSP module
 * Returns: Assignable channel of VSP is returned.
 */
unsigned long vspm_ins_ctrl_assign_vsp_module(
	VSPM_VSP_PAR *vsp_par, struct vspm_usable_info *usable)
{
	const unsigned long enable_ch_bits[VSPM_IP_MAX] = {
		/* VSPS */
		VSPM_IPCTRL_VSPS_WPF_CH0|
		VSPM_IPCTRL_VSPS_WPF_CH1|
		VSPM_IPCTRL_VSPS_WPF_CH2|
		VSPM_IPCTRL_VSPS_WPF_CH3,
		/* VSPR */
		VSPM_IPCTRL_VSPR_WPF_CH0|
		VSPM_IPCTRL_VSPR_WPF_CH1|
		VSPM_IPCTRL_VSPR_WPF_CH2|
		VSPM_IPCTRL_VSPR_WPF_CH3,
		/* VSPD0 */
		VSPM_IPCTRL_VSPD0_WPF_CH0,
		/* VSPD1 */
		VSPM_IPCTRL_VSPD1_WPF_CH0
	};
	unsigned long ch_bits = 0;	/* disable */

	unsigned long use_module_bits;

	unsigned char usable_cnt;
	unsigned char use_cnt;
	unsigned char assign_flag;

	int i;

	for (i = 0; i < VSPM_IP_MAX; i++) {
		assign_flag = 1;

		/* check RPF module */
		usable_cnt =
			vspm_ins_ctrl_count_bits(usable->vsp_rpf_bits[i]);
		if (usable_cnt < vsp_par->rpf_num)
			assign_flag = 0;

		/* check RPF(CLUT) module */
		usable_cnt = vspm_ins_ctrl_count_bits(
			usable->vsp_rpf_bits[i] & usable->vsp_rpf_clut_bits[i]);
		use_cnt = vspm_ins_ctrl_get_rpf_clut_count(vsp_par);
		if (usable_cnt < use_cnt)
			assign_flag = 0;

		use_module_bits = vsp_par->use_module;

		/* check UDS module */
		usable_cnt =
			vspm_ins_ctrl_get_uds_count(usable->vsp_module_bits[i]);
		use_cnt = vspm_ins_ctrl_get_uds_count(use_module_bits);
		if (usable_cnt < use_cnt)
			assign_flag = 0;

		/* check other module */
		use_module_bits &= ~(VSP_UDS_USE|VSP_UDS1_USE|VSP_UDS2_USE);
		if ((usable->vsp_module_bits[i] & use_module_bits) !=
				use_module_bits)
			assign_flag = 0;

		if (assign_flag) {
			/* set assignable bits */
			ch_bits |= usable->ch_bits & enable_ch_bits[i];
		}
	}

	return ch_bits;
}


/*
 * vspm_ins_ctrl_swap_connect_param - Swapping connect parameter
 * @vsp_par:           VSP parameter pointer
 * @old_bit:           searching bit
 * @new_bit:           changing bit
 * Description: Swapping VSP parameter
 * Returns: none
 */
static void vspm_ins_ctrl_swap_connect_param(
	VSPM_VSP_PAR *vsp_par, unsigned long old_bit, unsigned long new_bit)
{
	/* T_VSP_IN */
	if (vsp_par->src1_par) {
		if (vsp_par->src1_par->connect == old_bit)
			vsp_par->src1_par->connect = new_bit;
		else if (vsp_par->src1_par->connect == new_bit)
			vsp_par->src1_par->connect = old_bit;
	}

	if (vsp_par->src2_par) {
		if (vsp_par->src2_par->connect == old_bit)
			vsp_par->src2_par->connect = new_bit;
		else if (vsp_par->src2_par->connect == new_bit)
			vsp_par->src2_par->connect = old_bit;
	}

	if (vsp_par->src3_par) {
		if (vsp_par->src3_par->connect == old_bit)
			vsp_par->src3_par->connect = new_bit;
		else if (vsp_par->src3_par->connect == new_bit)
			vsp_par->src3_par->connect = old_bit;
	}

	if (vsp_par->src4_par) {
		if (vsp_par->src4_par->connect == old_bit)
			vsp_par->src4_par->connect = new_bit;
		else if (vsp_par->src4_par->connect == new_bit)
			vsp_par->src4_par->connect = old_bit;
	}

	if (vsp_par->ctrl_par) {
		/* T_VSP_SRU */
		if (vsp_par->ctrl_par->sru) {
			if (vsp_par->ctrl_par->sru->connect == old_bit)
				vsp_par->ctrl_par->sru->connect = new_bit;
			else if (vsp_par->ctrl_par->sru->connect == new_bit)
				vsp_par->ctrl_par->sru->connect = old_bit;
		}

		/* T_VSP_LUT */
		if (vsp_par->ctrl_par->lut) {
			if (vsp_par->ctrl_par->lut->connect == old_bit)
				vsp_par->ctrl_par->lut->connect = new_bit;
			else if (vsp_par->ctrl_par->lut->connect == new_bit)
				vsp_par->ctrl_par->lut->connect = old_bit;
		}

		/* T_VSP_CLU */
		if (vsp_par->ctrl_par->clu) {
			if (vsp_par->ctrl_par->clu->connect == old_bit)
				vsp_par->ctrl_par->clu->connect = new_bit;
			else if (vsp_par->ctrl_par->clu->connect == new_bit)
				vsp_par->ctrl_par->clu->connect = old_bit;
		}

		/* T_VSP_HST */
		if (vsp_par->ctrl_par->hst) {
			if (vsp_par->ctrl_par->hst->connect == old_bit)
				vsp_par->ctrl_par->hst->connect = new_bit;
			else if (vsp_par->ctrl_par->hst->connect == new_bit)
				vsp_par->ctrl_par->hst->connect = old_bit;
		}

		/* T_VSP_HSI */
		if (vsp_par->ctrl_par->hsi) {
			if (vsp_par->ctrl_par->hsi->connect == old_bit)
				vsp_par->ctrl_par->hsi->connect = new_bit;
			else if (vsp_par->ctrl_par->hsi->connect == new_bit)
				vsp_par->ctrl_par->hsi->connect = old_bit;
		}

		/* T_VSP_BRU */
		if (vsp_par->ctrl_par->bru) {
			if (vsp_par->ctrl_par->bru->connect == old_bit)
				vsp_par->ctrl_par->bru->connect = new_bit;
			else if (vsp_par->ctrl_par->bru->connect == new_bit)
				vsp_par->ctrl_par->bru->connect = old_bit;
		}
	}
}


/*
 * vspm_ins_ctrl_swap_sampling_param - Swapping sampling parameter
 * @vsp_par:           VSP parameter pointer
 * @old_bit:           searching sampling
 * @new_bit:           changing sampling
 * Description: Swapping VSP parameter
 * Returns: none
 */
static void vspm_ins_ctrl_swap_sampling_param(
	VSPM_VSP_PAR *vsp_par, unsigned long old_smppt, unsigned long new_smppt)
{
	if (vsp_par->ctrl_par) {
		/* T_VSP_HGO */
		if (vsp_par->ctrl_par->hgo) {
			if (vsp_par->ctrl_par->hgo->sampling == old_smppt) {
				vsp_par->ctrl_par->hgo->sampling = new_smppt;
			} else if (vsp_par->ctrl_par->hgo->sampling ==
					 new_smppt) {
				vsp_par->ctrl_par->hgo->sampling = old_smppt;
			}
		}

		/* T_VSP_HGT */
		if (vsp_par->ctrl_par->hgt) {
			if (vsp_par->ctrl_par->hgt->sampling == old_smppt)
				vsp_par->ctrl_par->hgt->sampling = new_smppt;
			else if (vsp_par->ctrl_par->hgt->sampling == new_smppt)
				vsp_par->ctrl_par->hgt->sampling = old_smppt;
		}
	}
}

/*
 * vspm_ins_ctrl_reconnect_uds_module - Re-connect UDS module
 * @ch:                    channel number
 * @vsp_par:               VSP parameter pointer
 * @usable_module_bits:    usable module bits
 * Description: Reconnecting UDS module
 * Returns: The following reconnecting result code is returned.
 * R_VSPM_OK
 * R_VSPM_NG
 */
long vspm_ins_ctrl_reconnect_uds_module(
	unsigned short ch,
	VSPM_VSP_PAR *vsp_par,
	struct vspm_usable_info *usable)
{
	const unsigned long uds_bit[3] = {
		VSP_UDS_USE,
		VSP_UDS1_USE,
		VSP_UDS2_USE,
	};
	const unsigned long uds_smppt[3] = {
		VSP_SMPPT_UDS,
		VSP_SMPPT_UDS1,
		VSP_SMPPT_UDS2,
	};
	T_VSP_UDS * *uds_par[3];

	unsigned long target_bits;
	unsigned long empty_bits;

	unsigned char i, j;
	long ercd;

	/* set UDS parameter */
	uds_par[0] = &vsp_par->ctrl_par->uds;
	uds_par[1] = &vsp_par->ctrl_par->uds1;
	uds_par[2] = &vsp_par->ctrl_par->uds2;

	/* get VSP IP number */
	ercd = vspm_inc_get_vsp_num(&i, ch);
	if (ercd != R_VSPM_OK)
		return R_VSPM_NG;

	target_bits = ~(usable->vsp_module_bits[i]) & (vsp_par->use_module);
	empty_bits = (usable->vsp_module_bits[i]) & ~(vsp_par->use_module);

	for (i = 0; i < 3; i++) {
		if (target_bits & uds_bit[i]) {
			for (j = 0; j < 3; j++) {
				if (empty_bits & uds_bit[j])
					break;
			}

			if (j == 3) {
				/* no empty */
				return R_VSPM_NG;
			}

			/* swap connect parameter */
			vspm_ins_ctrl_swap_connect_param(
				vsp_par, uds_bit[i], uds_bit[j]);

			/* swap sampling parameter */
			vspm_ins_ctrl_swap_sampling_param(
				vsp_par, uds_smppt[i], uds_smppt[j]);

			/* swap UDS parameter */
			{
				T_VSP_UDS *tmp_par;

				tmp_par = *uds_par[j];
				*uds_par[j] = *uds_par[i];
				*uds_par[i] = tmp_par;

				vsp_par->use_module &= ~uds_bit[i];
				vsp_par->use_module |= uds_bit[j];
			}

			/* clear bit */
			empty_bits &= ~uds_bit[j];

			DPRINT("reconnected UDS bit (0x%04x)->(0x%04x)\n",
				(unsigned int)uds_bit[i],
				(unsigned int)uds_bit[j]);
		}
	}

	return R_VSPM_OK;
}

/*
 * vspm_inc_ctrl_on_driver_complete - complete callback function
 * @module_id:  module id
 * @result:     processing result
 * Description: complete callback function
 */
void vspm_inc_ctrl_on_driver_complete(unsigned short module_id, long result)
{
	struct vspm_api_param_on_complete api_param;

	api_param.module_id = module_id;
	api_param.result = result;

	(void)fw_send_event(
		TASK_VSPM,
		FUNCTIONID_VSPM_BASE + EVENT_VSPM_DRIVER_ON_COMPLETE,
		sizeof(api_param),
		&api_param);
}

/*
 * vspm_inc_get_vsp_num - Get VSP IP number
 * @num:    IP number(enum VSPM_IP_VSP)
 * @ch:     channel
 * Description: Get IP number from channel
 * Returns: The following reconnecting result code is returned.
 * R_VSPM_OK
 * R_VSPM_NG
 */
long vspm_inc_get_vsp_num(unsigned char *num, unsigned short ch)
{
	long ercd = R_VSPM_OK;

	if (IS_VSPS_CH(ch)) {
		/* VSPS */
		*num = VSPM_IP_VSPS;
	} else if (IS_VSPR_CH(ch)) {
		/* VSPR */
		*num = VSPM_IP_VSPR;
	} else if (IS_VSPD0_CH(ch)) {
		/* VSPD0 */
		*num = VSPM_IP_VSPD0;
	} else if (IS_VSPD1_CH(ch)) {
		/* VSPD1 */
		*num = VSPM_IP_VSPD1;
	} else {
		ercd = R_VSPM_NG;
	}

	return ercd;
}

