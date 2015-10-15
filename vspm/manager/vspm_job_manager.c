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

/*
 * vspm_ins_job_entry - Job registration
 * @job_manager:     job information management
 * @entry:           entry information structure
 * Description: Job registration
 * Returns: On success, pointer to job information table is returned.
 * On error, NULL is returned.
 */
struct vspm_job_info *vspm_ins_job_entry(
	struct vspm_job_manager *job_manager,
	struct vspm_api_param_entry *entry)
{
	struct vspm_job_info *job_info;

	unsigned long count;
	unsigned long index;

	/* looking for space */
	for (count = 0; count < VSPM_MAX_ELEMENTS; count++) {
		index = (job_manager->write_index + count) % VSPM_MAX_ELEMENTS;
		if (job_manager->job_info[index].status
				== VSPM_JOB_STATUS_EMPTY)
			break;
	}

	if (count >= VSPM_MAX_ELEMENTS) {
		EPRINT("%s queue full\n", __func__);
		return NULL;
	}

	/* Get a job information table */
	job_info = &job_manager->job_info[index];

	/* Update a registration count */
	job_manager->entry_count++;
	job_manager->write_index =
		(job_manager->write_index + 1) % VSPM_MAX_ELEMENTS;

	/* Set job information */
	job_info->status = VSPM_JOB_STATUS_ENTRY;
	job_info->job_id = (job_manager->entry_count << 8) | (index + 1);
	job_info->ch_num = 0;
	job_info->result = R_VSPM_OK;
	job_info->entry	 = *entry;
	job_info->next_job_info = NULL;

	DPRINT("%s job_id=0x%08lx index=%ld\n",
		__func__, job_info->job_id, index);
	return job_info;
}


/*
 * vspm_ins_job_find_job_info - Seach a job information
 * @job_manager: job information management
 * @job_id:      job id
 * Description: Seach a job information
 * Returns: On success, pointer to job information table is returned.
 * On error, NULL is returned.
 */
struct vspm_job_info *vspm_ins_job_find_job_info(
	struct vspm_job_manager *job_manager, unsigned long job_id)
{
	struct vspm_job_info *job_info;
	unsigned long index = VSPM_JOB_GET_ARRAY_INDEX(job_id);

	if (index >= VSPM_MAX_ELEMENTS) {
		EPRINT("%s Invalid job_id 0x%08lx\n", __func__, job_id);
		return NULL;
	}

	job_info = &job_manager->job_info[index];

	if (job_info->status == VSPM_JOB_STATUS_EMPTY) {
		EPRINT("%s Invalid status %ld job_id=0x%08lx\n",
			__func__, job_info->status, job_id);
		return NULL;
	}

	if (job_info->job_id != job_id) {
		EPRINT("%s Invalid job_id 0x%08lx job_id=0x%08lx\n",
			__func__, job_info->job_id, job_id);
		return NULL;
	}

	DPRINT("%s job_id=0x%08lx index=%ld\n",
		__func__, job_info->job_id, index);
	return job_info;
}


/*
 * vspm_ins_job_get_status - Get status information of the job
 * @job_info: job information
 * Description: Get status information of the job
 * Returns: status information of the job
 */
unsigned long vspm_ins_job_get_status(struct vspm_job_info *job_info)
{
	return job_info->status;
}


/*
 * vspm_ins_job_cancel - Cancel the job
 * @job_info: job information
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_job_cancel(struct vspm_job_info *job_info)
{
	/* check status */
	if (job_info->status != VSPM_JOB_STATUS_ENTRY) {
		EPRINT("%s Illigal status %ld\n", __func__, job_info->status);
		return R_VSPM_NG;
	}

	/* call callback funnction */
	job_info->entry.pfn_complete_cb(
		job_info->job_id, R_VSPM_CANCEL, job_info->entry.user_data);

	/* update status */
	job_info->status = VSPM_JOB_STATUS_EMPTY;

	DPRINT("%s job_id=0x%08lx\n", __func__, job_info->job_id);
	return R_VSPM_OK;
}


/*
 * vspm_ins_job_execute_start - Set to executing the state of the job
 * @job_info:             job information
 * @exec_ch:              channel number
 * Description: Set to executing the state of the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_job_execute_start(
	struct vspm_job_info *job_info, unsigned long exec_ch)
{
	if (job_info->status != VSPM_JOB_STATUS_ENTRY) {
		EPRINT("%s Illigal status %ld\n", __func__, job_info->status);
		return R_VSPM_NG;
	}

	/* set channel bits */
	job_info->ch_num = exec_ch;

	/* update status */
	job_info->status = VSPM_JOB_STATUS_EXECUTING;

	IPRINT("%s job_id=0x%08lx\n", __func__, job_info->job_id);
	return R_VSPM_OK;
}


/*
 * vspm_ins_job_execute_complete - Job completion processing
 * @job_info:         job information
 * @result:           processing result
 * @comp_ch:          complete channel number
 * Description: Job completion processing
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_job_execute_complete(
	struct vspm_job_info *job_info, long result, unsigned long comp_ch)
{
	if (job_info->status != VSPM_JOB_STATUS_EXECUTING) {
		EPRINT("%s Illigal status %ld\n", __func__, job_info->status);
		return R_VSPM_NG;
	}

	if (job_info->ch_num == comp_ch) {
		/* call callback function */
		job_info->entry.pfn_complete_cb(
			job_info->job_id, result, job_info->entry.user_data);

		/* update status */
		job_info->status = VSPM_JOB_STATUS_EMPTY;
	}

	NPRINT("%s job_id=0x%08lx\n", __func__, job_info->job_id);
	return R_VSPM_OK;
}


/*
 * vspm_ins_job_remove - Remove the job
 * @job_info: job information
 * Description: Remove the job
 */
void vspm_ins_job_remove(struct vspm_job_info *job_info)
{
	job_info->status = VSPM_JOB_STATUS_EMPTY;
}



/*
 * vspm_ins_job_get_job_id - Get job ID of the job information
 * @job_info: job information
 * Description: Get job ID of the job information
 * Returns: Job ID of the job information
 */
unsigned long vspm_ins_job_get_job_id(struct vspm_job_info *job_info)
{
	return job_info->job_id;
}

/*
 * vspm_ins_job_get_param - Get IP parameter of the job information
 * @job_info: job information
 * Description: Get IP parameter of the job information
 * Returns: IP parameter of the job information
 */
VSPM_IP_PAR *vspm_ins_job_get_param(struct vspm_job_info *job_info)
{
	return job_info->entry.p_ip_par;
}
