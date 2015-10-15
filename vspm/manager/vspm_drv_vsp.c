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
#include <linux/errno.h>

#include "frame.h"
#include "vspm_public.h"
#include "vspm_private.h"
#include "vspm_log.h"
#include "vspm_common.h"

#include "vsp_drv_public.h"

#define INTLEVEL_VSPS		8
#define INTLEVEL_VSPR		8

/*
 * vspm_ins_vsp_ch - Get the channel number from module ID
 * @module_id:  module id
 * @ch:         channel number(output)
 * Description: Get the channel number from module ID
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_vsp_ch(unsigned short module_id, unsigned char *ch)
{
	register unsigned char vsp_ch = 0;

	if (module_id == VSPM_TYPE_VSP_CH0) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

	if (module_id == VSPM_TYPE_VSP_CH1) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

	if (module_id == VSPM_TYPE_VSP_CH2) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

	if (module_id == VSPM_TYPE_VSP_CH3) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

#ifdef USE_VSPR
	if (module_id == VSPM_TYPE_VSP_CH4) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

	if (module_id == VSPM_TYPE_VSP_CH5) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

	if (module_id == VSPM_TYPE_VSP_CH6) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}

	if (module_id == VSPM_TYPE_VSP_CH7) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}
#endif

#ifdef USE_VSPD0
	if (module_id == VSPM_TYPE_VSP_CH8) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}
#endif

#ifdef USE_VSPD1
	if (module_id == VSPM_TYPE_VSP_CH9) {
		*ch = vsp_ch;
		return R_VSPM_OK;
	} else {
		vsp_ch++;
	}
#endif

	return R_VSPM_NG;
}

/*
 * vspm_cb_vsp - VSP complete-callback function
 * @vsp_cb:  processing result
 * Description: VSP complete-callback function
 */
static void vspm_cb_vsp(T_VSP_CB *vsp_cb)
{
	long result;

	switch (vsp_cb->ercd) {
	case 0:
		result = R_VSPM_OK;
		break;
	case -ECANCELED:
		result = R_VSPM_CANCEL;
		break;
	default:
		result = R_VSPM_DRIVER_ERR;
		break;
	}

	IPRINT("%s ercd=%ld, userdata=%d\n",
		__func__, vsp_cb->ercd, (int)vsp_cb->userdata);
	vspm_inc_ctrl_on_driver_complete(
		(unsigned short)(unsigned long)vsp_cb->userdata, result);
}


/*
 * vspm_ins_vsp_initialize - Initialize the VSP driver
 * Description: Initialize the VSP driver
 * @pdev: Platform device
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_vsp_initialize(struct platform_device *pdev)
{
	int ercd;
	long sub_ercd;
	T_VSP_INIT init_param;

	init_param.intlvl[0] = INTLEVEL_VSPS;
	init_param.intlvl[1] = INTLEVEL_VSPR;

	ercd = vsp_lib_init(pdev, &init_param, &sub_ercd);
	if (ercd) {
		EPRINT("[%s] vsp_lib_init() Failed!! ercd=%d, sub_ercd=%ld\n",
			__func__, ercd, sub_ercd);
		return R_VSPM_NG;
	}
	return R_VSPM_OK;
}


/*
 * vspm_ins_vsp_execute - VSP processing execution routine
 * @module_id:  module id
 * @vsp_par:    VSP parameter
 * @rpf_order:  RPF alignment settings
 * Description: VSP processing execution routine
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_NG:        abnormal termination
 * R_VSPM_START_ERR: IP error(startup)
 */
long vspm_ins_vsp_execute(
	unsigned short module_id,
	VSPM_VSP_PAR *vsp_par,
	unsigned long rpf_order)
{
	T_VSP_START start_param;
	unsigned char ch;

	int ercd;
	long sub_ercd;

	DPRINT("called\n");

	sub_ercd = vspm_ins_vsp_ch(module_id, &ch);
	if (sub_ercd) {
		EPRINT("vspm_ins_vsp_ch() Failed!! ercd=%ld, module_id=%d\n",
			sub_ercd, module_id);
		return R_VSPM_NG;
	}

	ercd = vsp_lib_open(ch, &sub_ercd);
	if (ercd) {
		EPRINT("vsp_lib_open(%d) Failed!! ercd=%d, sub_ercd=%ld\n",
			ch, ercd, sub_ercd);
		return sub_ercd;
	}

	/* set parameter */
	start_param.rpf_num		= vsp_par->rpf_num;
	start_param.use_module	= vsp_par->use_module;
	start_param.rpf_order	= rpf_order;
	start_param.src1_par	= vsp_par->src1_par;
	start_param.src2_par	= vsp_par->src2_par;
	start_param.src3_par	= vsp_par->src3_par;
	start_param.src4_par	= vsp_par->src4_par;
	start_param.dst_par		= vsp_par->dst_par;
	start_param.ctrl_par	= vsp_par->ctrl_par;

	ercd = vsp_lib_start(ch, (void *)vspm_cb_vsp, &start_param,
		(void *)(unsigned long)module_id, &sub_ercd);
	if (ercd) {
		EPRINT("vsp_lib_start(%d) Failed!! ercd=%d, sub_ercd=%ld\n",
			ch, ercd, sub_ercd);

		/* forced close */
		(void)vsp_lib_close(ch, NULL);
		return sub_ercd;
	}

	DPRINT("done. module_id=%d\n", module_id);
	return R_VSPM_OK;
}


/*
 * vspm_ins_vsp_exec_complete - VSP processing completion routine
 * @module_id:  module id
 * Description: VSP processing completion routine
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_vsp_exec_complete(unsigned short module_id)
{
	int ercd;
	long sub_ercd;
	unsigned char ch;

	sub_ercd = vspm_ins_vsp_ch(module_id, &ch);
	if (sub_ercd) {
		EPRINT(
			"[%s] vspm_ins_vsp_ch() Failed!! ercd=%ld, module_id=%d\n",
			__func__, sub_ercd, module_id);
		/* forced close */
		(void)vsp_lib_close(ch, NULL);
		return R_VSPM_NG;
	}

	ercd = vsp_lib_close(ch, &sub_ercd);
	if (ercd) {
		EPRINT("[%s] vsp_lib_close() Failed!! ercd=%d, sub_ercd=%ld\n",
			__func__, ercd, sub_ercd);
		return R_VSPM_NG;
	}

	return R_VSPM_OK;
}


/*
 * vspm_ins_vsp_cancel - VSP processing cancellation routine
 * @module_id:  module id
 * Description: VSP processing cancellation routine
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_vsp_cancel(unsigned short module_id)
{
	int ercd;
	long sub_ercd;
	unsigned char ch;

	ercd = vspm_ins_vsp_ch(module_id, &ch);
	if (ercd) {
		EPRINT(
			"[%s] vspm_ins_vsp_ch() Failed!! ercd=%ld, module_id=%d\n",
			__func__, sub_ercd, module_id);
		/* forced close */
		(void)vsp_lib_close(ch, NULL);
		return R_VSPM_NG;
	}

	ercd = vsp_lib_abort(ch, &sub_ercd);
	if (ercd) {
		EPRINT(
			"[%s] vsp_lib_abort(%d) Failed!! ercd=%d, sub_ercd=%ld\n",
			__func__, ch, ercd, sub_ercd);
		/* forced close */
		(void)vsp_lib_close(ch, NULL);
		return R_VSPM_NG;
	}

	ercd = vsp_lib_close(ch, &sub_ercd);
	if (ercd) {
		EPRINT("[%s] vsp_lib_close() Failed!! ercd=%d, sub_ercd=%ld\n",
			__func__, ercd, sub_ercd);
		return R_VSPM_NG;
	}

	return R_VSPM_OK;
}


/*
 * vspm_ins_vsp_quit - Exit the VSP driver
 * Description: Exit the VSP driver
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long vspm_ins_vsp_quit(void)
{
	int ercd;
	long sub_ercd;

	ercd = vsp_lib_quit(&sub_ercd);
	if (ercd) {
		EPRINT("[%s] vsp_lib_quit() Failed!! ercd=%d, sub_ercd=%ld\n",
			__func__, ercd, sub_ercd);
		return R_VSPM_NG;
	}

	return R_VSPM_OK;
}
