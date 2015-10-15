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

#ifndef VSPM_COMMON_H
#define VSPM_COMMON_H

#include <linux/platform_device.h>

/*
 * Function id of VSPM task
 */
enum VSPM_FUNCTION_ID {
	FUNC_VSPM_ENTRY = FUNC_TASK_QUIT + 1,
	EVENT_VSPM_DRIVER_ON_COMPLETE,
	FUNC_VSPM_GET_STATUS,
	FUNC_VSPM_CANCEL,
	FUNC_VSPM_FORCED_CANCEL,
	EVENT_VSPM_DISPATCH,
	EVENT_VSPM_MAX
};

/*
 *   The return value of the vspm_ins_ctrl_assign_vsp_rpf()
 */
#define VSP_ASSIGN_OK		0
#define VSP_ASSIGN_NG		-1

/*
 * State of the job
 */
#define VSPM_JOB_STATUS_EMPTY		0
#define VSPM_JOB_STATUS_ENTRY		1
#define VSPM_JOB_STATUS_EXECUTING	2

/*
 * IP number
 */
enum VSPM_IP_VSP {
	VSPM_IP_VSPS = 0,
	VSPM_IP_VSPR,
	VSPM_IP_VSPD0,
	VSPM_IP_VSPD1,
	VSPM_IP_MAX
};

/*
 * Other definitions
 */
#define VSPM_MAX_ELEMENTS		32	/* maximum number of jobs */

#define VSP_RPF_NO_USE			0xFFFFFFFF	/* not use the RPF */

/*
 * Other macros
 */
#define VSPM_JOB_GET_ARRAY_INDEX(job_id)	(0xFF & ((job_id) - 1))

/*
 * struct vspm_api_param_entry - parameter of FUNC_VSPM_ENTRY message
 * @handle:          handle
 * @p_job_id:        destination address of the job id
 * @job_priority:    job priority 1(low) - 126(high)
 * @p_ip_par:        pointer to IP parameter
 * @pfn_complete_cb: pointer to complete callback function
 * @user_data:       user data
 *
 */
struct vspm_api_param_entry {
	unsigned long handle;
	unsigned long *p_job_id;
	char job_priority;
	VSPM_IP_PAR *p_ip_par;
	PFN_VSPM_COMPLETE_CALLBACK pfn_complete_cb;
	unsigned long user_data;
};

/*
 * struct vspm_job_info - job information
 * @status:          state of job
 * @job_id:          job id
 * @ch_num:          channel number
 * @vsp_rpf_bits:    VSP-RPF assignment information
 * @vsp_module_bits: VSP conversion process assignment information
 * @result:          processing results
 * @entry:           entry parameter structure
 * @changed_uds:
 *
 */
struct vspm_job_info {
	unsigned long status;
	unsigned long job_id;
	unsigned long ch_num;
	long result;
	struct vspm_api_param_entry entry;
	void *next_job_info;
};


/*
 * struct vspm_job_manager - job information management table
 * @entry_count: total number of data entry
 * @write_index: write position
 * @job_info:    job information
 *
 */
struct vspm_job_manager {
	unsigned long entry_count;
	unsigned long write_index;
	struct vspm_job_info job_info[VSPM_MAX_ELEMENTS];
};

/*
 * struct vspm_queue_info - queue information table
 * @data_count:     number of data entry
 * @first_job_info: first job information structure.
 *
 */
struct vspm_queue_info {
	unsigned short data_count;
	void *first_job_info;
};

/*
 * struct vspm_exec_info - information about the execution of the IP
 * @exec_ch_bits:             execution status management of the module
 * @exec_vsp_rpf_bits:        usage management of RPF channel bits
 * @exec_vsp_module_bits:     usage management of VSP module bits
 * @vsp_module:               VSP module assignment information
 * @vsp_rpf:                  RPF channel assignment information
 * @p_exec_job_info:          job information
 * @p_exec_ip_par:            pointer to IP parameter
 *
 */
struct vspm_exec_info {
	unsigned long exec_ch_bits;
	unsigned long vsp_module[VSPM_TYPE_VSP_CH_MAX];
	unsigned long vsp_rpf[VSPM_TYPE_VSP_CH_MAX];
	struct vspm_job_info *p_exec_job_info[VSPM_TYPE_CH_MAX];
	VSPM_IP_PAR *p_exec_ip_par[VSPM_TYPE_CH_MAX];
};

/*
 * struct vspm_usable_info - VSPM driver usable bits information
 * @ch_bits:           VSP manager channel bits
 * @vsp_module_bits:   VSP module bits
 * @vsp_rpf_bits:      RPF channel bits
 * @vsp_rpf_clut_bits: RPF(CLUT) channel bits
 * @vsp_uds_bits:      UDS module channel bits
 *
 */
struct vspm_usable_info {
	unsigned long ch_bits;
	unsigned long vsp_module_bits[VSPM_IP_MAX];
	unsigned long vsp_rpf_bits[VSPM_IP_MAX];
	unsigned long vsp_rpf_clut_bits[VSPM_IP_MAX];
	unsigned long vsp_uds_bits[VSPM_IP_MAX];
};

/*
 * struct vspm_ctrl_info - VSPM driver control information
 * @job_manager: management of job information
 * @queue_info:  queue information
 * @exec_info:   execution information of the IP
 *
 */
struct vspm_ctrl_info {
	struct vspm_job_manager job_manager;
	struct vspm_queue_info queue_info;
	struct vspm_exec_info exec_info;
	struct vspm_usable_info usable_info;
};

/*
 * struct vspm_api_param_on_complete - parameter of
 *                                     EVENT_VSPM_DRIVER_ON_COMPLETE message
 * @module_id: module id
 * @result:    processing results
 *
 */
struct vspm_api_param_on_complete {
	unsigned short module_id;
	long result;
};


/* control functions */
long vspm_ins_ctrl_initialize(void);
long vspm_ins_ctrl_quit(void);
long vspm_ins_ctrl_entry(struct vspm_api_param_entry *entry);
long vspm_ins_ctrl_on_complete(unsigned short module_id, long result);
long vspm_ins_ctrl_get_status(unsigned long job_id);
long vspm_ins_ctrl_cancel(unsigned long job_id);
long vspm_ins_ctrl_forced_cancel(unsigned long handle);
long vspm_ins_ctrl_entry_param_check(struct vspm_api_param_entry *entry);
unsigned short vspm_ins_ctrl_assign_channel(
	VSPM_IP_PAR *ip_par, struct vspm_usable_info *usable);
long vspm_ins_ctrl_assign_vsp_rpf(
	unsigned short ch,
	VSPM_VSP_PAR *vsp_par,
	struct vspm_usable_info *usable,
	unsigned long *p_vsp_rpf,
	unsigned long *p_rpf_order);
unsigned long vspm_ins_ctrl_assign_vsp_module(
	VSPM_VSP_PAR *vsp_par, struct vspm_usable_info *usable);
long vspm_ins_ctrl_reconnect_uds_module(
	unsigned short ch,
	VSPM_VSP_PAR *vsp_par,
	struct vspm_usable_info *usable);
void vspm_ins_ctrl_dispatch(void);
void vspm_inc_ctrl_on_driver_complete(unsigned short module_id, long result);
long vspm_inc_get_vsp_num(unsigned char *num, unsigned short ch);


/* job manager functions */
struct vspm_job_info *vspm_ins_job_entry(
	struct vspm_job_manager *job_manager,
	struct vspm_api_param_entry *entry);
struct vspm_job_info *vspm_ins_job_find_job_info(
	struct vspm_job_manager *job_manager, unsigned long job_id);
unsigned long vspm_ins_job_get_status(struct vspm_job_info *job_info);
long vspm_ins_job_cancel(struct vspm_job_info *job_info);
long vspm_ins_job_execute_start(
	struct vspm_job_info *job_info, unsigned long exec_ch);
long vspm_ins_job_execute_complete(
	struct vspm_job_info *job_info, long result, unsigned long comp_ch);
void vspm_ins_job_remove(struct vspm_job_info *job_info);
unsigned long vspm_ins_job_get_job_id(struct vspm_job_info *job_info);
VSPM_IP_PAR *vspm_ins_job_get_param(struct vspm_job_info *job_info);


/* executing manager functions */
long vspm_ins_exec_start(
	struct vspm_exec_info *exec_info,
	unsigned short module_id,
	VSPM_IP_PAR *ip_par,
	struct vspm_job_info *job_info,
	unsigned long use_vsp_module,
	unsigned long use_vsp_rpf,
	unsigned long rpf_order);
long vspm_ins_exec_complete(
	struct vspm_exec_info *exec_info, unsigned short module_id);
struct vspm_job_info *vspm_ins_exec_get_current_job_info(
	struct vspm_exec_info *exec_info, unsigned short module_id);
void vspm_ins_exec_update_current_status(
	struct vspm_exec_info *exec_info, struct vspm_usable_info *usable);
long vspm_ins_exec_cancel(
	struct vspm_exec_info *exec_info, unsigned short module_id);

/* sort queue functions */
long vspm_inc_sort_queue_initialize(struct vspm_queue_info *queue_info);
long vspm_inc_sort_queue_entry(
	struct vspm_queue_info *queue_info, struct vspm_job_info *job_info);
long vspm_inc_sort_queue_refer(
	struct vspm_queue_info *queue_info,
	unsigned short index,
	struct vspm_job_info **p_job_info);
long vspm_inc_sort_queue_remove(
	struct vspm_queue_info *queue_info, unsigned short index);
unsigned short vspm_inc_sort_queue_get_count(
	struct vspm_queue_info *queue_info);
long vspm_inc_sort_queue_find_item(
	struct vspm_queue_info *queue_info,
	struct vspm_job_info *job_info,
	unsigned short *p_index);


/* vsp control functions */
long vspm_ins_vsp_ch(unsigned short module_id, unsigned char *ch);
long vspm_ins_vsp_initialize(struct platform_device *pdev);
long vspm_ins_vsp_execute(
	unsigned short module_id,
	VSPM_VSP_PAR *vsp_par,
	unsigned long rpf_order);
long vspm_ins_vsp_exec_complete(unsigned short module_id);
long vspm_ins_vsp_cancel(unsigned short module_id);
long vspm_ins_vsp_quit(void);


/* 2ddmac control functions */
long vspm_ins_2ddmac_ch(unsigned short module_id, unsigned char *ch);
long vspm_ins_2ddmac_initialize(struct platform_device *pdev);
long vspm_ins_2ddmac_execute(
	unsigned short module_id, VSPM_2DDMAC_PAR *tddmac_par);
long vspm_ins_2ddmac_exec_complete(unsigned short module_id);
long vspm_ins_2ddmac_cancel(unsigned short module_id);
long vspm_ins_2ddmac_quit(void);

#endif /* VSPM_COMMON_H */
