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

#include "vsp_drv_public.h"

/*
 * vspm_ins_exec_start - Start the execution of the job
 * @exec_info:      execution information of the IP
 * @module_id:      module id
 * @ip_par:         IP parameter
 * @job_info:       job information
 * @use_vsp_module:
 * @use_vsp_rpf:
 * @rpf_order:      RPF alignment settings
 * Description: Start the execution of the job
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_exec_start(
	struct vspm_exec_info *exec_info,
	unsigned short module_id,
	VSPM_IP_PAR *ip_par,
	struct vspm_job_info *job_info,
	unsigned long use_vsp_module,
	unsigned long use_vsp_rpf,
	unsigned long rpf_order)
{
	unsigned long channel_bit = (0x00000001UL << module_id);
	long ercd;

	if (exec_info->exec_ch_bits & channel_bit) {
		EPRINT("%s Already executing module_id=0x%04x\n",
			__func__, module_id);
		return R_VSPM_NG;
	}

	/* Update the execution information */
	exec_info->exec_ch_bits |= channel_bit;

	exec_info->p_exec_job_info[module_id] = job_info;
	exec_info->p_exec_ip_par[module_id] = ip_par;

	if (IS_VSP_CH(module_id)) {
		/* Update the VSP execution information */
		exec_info->vsp_module[module_id] = use_vsp_module;
		exec_info->vsp_rpf[module_id]    = use_vsp_rpf;

		/* Start the VSP process */
		ercd = vspm_ins_vsp_execute(
			module_id, ip_par->unionIpParam.ptVsp, rpf_order);
		if (ercd) {
			/* Update the VSP execution information */
			exec_info->vsp_module[module_id] = 0;
			exec_info->vsp_rpf[module_id] = 0;
			goto exit;
		}
	} else if (IS_2DDMAC_CH(module_id)) {
		/* Start the 2DDMAC process */
		ercd = vspm_ins_2ddmac_execute(
			module_id, ip_par->unionIpParam.pt2dDmac);
		if (ercd)
			goto exit;
	} else {
		EPRINT("%s Invalid module_id 0x%04x\n", __func__, module_id);
		ercd = R_VSPM_NG;
		goto exit;
	}

	return R_VSPM_OK;

exit:
	/* Update the execution information */
	exec_info->p_exec_ip_par[module_id] = NULL;
	exec_info->p_exec_job_info[module_id] = NULL;

	exec_info->exec_ch_bits &= ~channel_bit;
	return ercd;
}


/*
 * vspm_ins_exec_complete - Job completion processing
 * @exec_info: execution information of the IP
 * @module_id: module id
 * Description: Job completion processing
 * Returns: R_VSPM_OK is returned.
 */
long vspm_ins_exec_complete(
	struct vspm_exec_info *exec_info, unsigned short module_id)
{
	long ercd;

	if (IS_VSP_CH(module_id)) {
		/* VSP process complete */
		ercd = vspm_ins_vsp_exec_complete(module_id);
		if (ercd)
			return R_VSPM_NG;

		/* clear VSP information */
		exec_info->vsp_module[module_id] = 0;
		exec_info->vsp_rpf[module_id] = 0;
	} else if (IS_2DDMAC_CH(module_id)) {
		/* 2DDMAC process complete */
		ercd = vspm_ins_2ddmac_exec_complete(module_id);
		if (ercd)
			return R_VSPM_NG;

		/* clear 2DDMAC information */
		/* no process */
	} else {
		EPRINT("%s Invalid module_id 0x%04x\n", __func__, module_id);
		return R_VSPM_NG;
	}

	/* clear common information */
	exec_info->p_exec_job_info[module_id] = NULL;
	exec_info->p_exec_ip_par[module_id] = NULL;
	exec_info->exec_ch_bits &= ~(0x00000001UL << module_id);

	return R_VSPM_OK;
}


/*
 * vspm_ins_exec_get_current_job_info - Get job information of the currently
 * running job
 * @exec_info: execution information of the IP
 * @module_id: module id
 * Description: Get job information of the currently running job
 * Returns: job information of the currently running job
 */
struct vspm_job_info *vspm_ins_exec_get_current_job_info(
	struct vspm_exec_info *exec_info, unsigned short module_id)
{
	if (!(exec_info->exec_ch_bits & (0x00000001UL << module_id)))
		return NULL;

	return exec_info->p_exec_job_info[module_id];
}


/*
 * vspm_ins_exec_update_current_status - Update status of the running job
 * @exec_info:  execution information of the IP
 * @usable:     assignable bit information structure
 * Description: Update status of the currently running job
 */
void vspm_ins_exec_update_current_status(
	struct vspm_exec_info *exec_info, struct vspm_usable_info *usable)
{
	usable->ch_bits &= ~exec_info->exec_ch_bits;

	usable->vsp_module_bits[VSPM_IP_VSPS] &=
		~(exec_info->vsp_module[VSPM_TYPE_VSP_CH0] |
		  exec_info->vsp_module[VSPM_TYPE_VSP_CH1] |
		  exec_info->vsp_module[VSPM_TYPE_VSP_CH2] |
		  exec_info->vsp_module[VSPM_TYPE_VSP_CH3]);
	usable->vsp_module_bits[VSPM_IP_VSPR] &=
		~(exec_info->vsp_module[VSPM_TYPE_VSP_CH4] |
		  exec_info->vsp_module[VSPM_TYPE_VSP_CH5] |
		  exec_info->vsp_module[VSPM_TYPE_VSP_CH6] |
		  exec_info->vsp_module[VSPM_TYPE_VSP_CH7]);
	usable->vsp_module_bits[VSPM_IP_VSPD0] &=
		~(exec_info->vsp_module[VSPM_TYPE_VSP_CH8]);
	usable->vsp_module_bits[VSPM_IP_VSPD1] &=
		~(exec_info->vsp_module[VSPM_TYPE_VSP_CH9]);

	usable->vsp_rpf_bits[VSPM_IP_VSPS] &=
		~(exec_info->vsp_rpf[VSPM_TYPE_VSP_CH0] |
		  exec_info->vsp_rpf[VSPM_TYPE_VSP_CH1] |
		  exec_info->vsp_rpf[VSPM_TYPE_VSP_CH2] |
		  exec_info->vsp_rpf[VSPM_TYPE_VSP_CH3]);
	usable->vsp_rpf_bits[VSPM_IP_VSPR] &=
		~(exec_info->vsp_rpf[VSPM_TYPE_VSP_CH4] |
		  exec_info->vsp_rpf[VSPM_TYPE_VSP_CH5] |
		  exec_info->vsp_rpf[VSPM_TYPE_VSP_CH6] |
		  exec_info->vsp_rpf[VSPM_TYPE_VSP_CH7]);
	usable->vsp_rpf_bits[VSPM_IP_VSPD0] &=
		~(exec_info->vsp_rpf[VSPM_TYPE_VSP_CH8]);
	usable->vsp_rpf_bits[VSPM_IP_VSPD1] &=
		~(exec_info->vsp_rpf[VSPM_TYPE_VSP_CH9]);
}


/*
 * vspm_ins_exec_cancel - Cancel a running job
 * @exec_info: execution information of the IP
 * @module_id: module id
 * Description: Cancel a running job
 * Returns: On success R_VSPM_OK is returned. On error, an error number is
 * returned, except R_VSPM_OK.
 */
long vspm_ins_exec_cancel(
	struct vspm_exec_info *exec_info, unsigned short module_id)
{
	long ercd;

	if (!(exec_info->exec_ch_bits & (0x00000001UL << module_id))) {
		EPRINT("%s not execute module_id=0x%04x\n",
			__func__, module_id);
		return R_VSPM_SEQERR;
	}

	if (IS_VSP_CH(module_id)) {
		/* Cancel the VSP process */
		ercd = vspm_ins_vsp_cancel(module_id);
		if (ercd)
			return ercd;

		/* clear VSP information */
		exec_info->vsp_module[module_id] = 0;
		exec_info->vsp_rpf[module_id] = 0;
	} else if (IS_2DDMAC_CH(module_id)) {
		/* Cancel the 2DDMAC process */
		ercd = vspm_ins_2ddmac_cancel(module_id);
		if (ercd)
			return ercd;

		/* clear 2DDMAC information */
		/* no process */
	} else {
		EPRINT("%s Invalid module_id 0x%04x\n", __func__, module_id);
		return R_VSPM_PARAERR;
	}

	/* clear common information */
	exec_info->p_exec_job_info[module_id] = NULL;
	exec_info->p_exec_ip_par[module_id] = NULL;
	exec_info->exec_ch_bits &= ~(0x00000001UL << module_id);

	return R_VSPM_OK;
}

