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

#include <linux/platform_device.h>
#include "vspm_main.h"
#include "vspm_log.h"
#include "vsp_drv.h"
#include "vsp_drv_public.h"
#include "vsp_drv_local.h"

static VSP_PRIVATE_DATA *g_vsp_obj[VSP_IP_MAX] = {NULL};

/*
 * vsp_lib_init - Initialize VSP driver routine
 * @pdev:		platform device
 * @param:		initialize parameter
 * @sub_ercd:	detail error code
 *
 */
int vsp_lib_init(
	struct platform_device *pdev, T_VSP_INIT *param, long *sub_ercd)
{
	VSP_PRIVATE_DATA **prv;

	unsigned char ch;
	unsigned char vsp_ch;

	long ercd = 0;
	int drv_ercd = 0;

	DPRINT("called (%d IPs)\n", VSP_IP_MAX);

	/* check parameter */
	if ((pdev == NULL) || (param == NULL)) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_PARA_INPAR;
		return -EINVAL;
	}

	/* check initialize parameter */
	ercd = vsp_ins_check_init_parameter(param);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}

	/* check condition */
	prv = &g_vsp_obj[0];
	for (vsp_ch = 0; vsp_ch < VSP_IP_MAX; vsp_ch++) {
		if (*prv != NULL) {
			/* quit */
			if (vsp_lib_quit(sub_ercd))
				return -EIO;
		}
		prv++;
	}

	prv = &g_vsp_obj[0];
	for (vsp_ch = 0; vsp_ch < VSP_IP_MAX; vsp_ch++) {
		/* allocate private data */
		ercd = vsp_ins_allocate_memory(prv);
		if (ercd) {
			drv_ercd = -ENOMEM;
			goto err_exit;
		}

		/* initialize register */
		ercd = vsp_ins_init_reg(pdev, *prv, vsp_ch);
		if (ercd) {
			drv_ercd = -EIO;
			goto err_exit;
		}

		/* registory interrupt handler */
		ercd = vsp_ins_reg_ih(pdev, *prv, vsp_ch);
		if (ercd) {
			drv_ercd = -EIO;
			goto err_exit;
		}

		/* update status */
		for (ch = 0; ch < VSP_WPF_MAX; ch++)
			(*prv)->ch_info[ch].status = VSP_STAT_INIT;

		prv++;
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;

err_exit:
	prv = &g_vsp_obj[0];
	for (vsp_ch = 0; vsp_ch < VSP_IP_MAX; vsp_ch++) {
		if (*prv != NULL) {
			/* unregistory interrupt handler */
			(void)vsp_ins_unreg_ih(*prv);

			/* Finalize register */
			(void)vsp_ins_quit_reg(*prv);

			/* release memory */
			(void)vsp_ins_free_memory(*prv);

			*prv = NULL;
		}
		prv++;
	}

	if (sub_ercd != NULL)
		*sub_ercd = ercd;

	return drv_ercd;
}

/*
 * vsp_lib_quit - Finalize VSP driver routine
 * @sub_ercd:	detail error code
 *
 */
int vsp_lib_quit(long *sub_ercd)
{
	VSP_PRIVATE_DATA **prv;

	signed char ch;
	unsigned char vsp_ch;

	long ercd = 0;

	DPRINT("called\n");

	prv = &g_vsp_obj[0];
	for (vsp_ch = 0; vsp_ch < VSP_IP_MAX; vsp_ch++) {
		/* check condition */
		if (*prv != NULL) {
			for (ch = 0; ch < VSP_WPF_MAX; ch++) {
				if ((*prv)->ch_info[ch].status
						== VSP_STAT_RUN) {
					/* stop VSP processing */
					ercd = vsp_ins_stop_processing(
						*prv, ch);
					if (ercd) {
						if (sub_ercd != NULL)
							*sub_ercd = ercd;
						return -EIO;
					}
				}

				/* update status */
				(*prv)->ch_info[ch].status = VSP_STAT_NOT_INIT;
			}

			/* unregistory interrupt handler */
			ercd = vsp_ins_unreg_ih(*prv);
			if (ercd) {
				if (sub_ercd != NULL)
					*sub_ercd = ercd;
				return -EIO;
			}

			/* Finalize register */
			ercd = vsp_ins_quit_reg(*prv);
			if (ercd) {
				if (sub_ercd != NULL)
					*sub_ercd = ercd;
				return -EIO;
			}

			/* release memory */
			ercd = vsp_ins_free_memory(*prv);
			if (ercd) {
				if (sub_ercd != NULL)
					*sub_ercd = ercd;
				return -EIO;
			}

			*prv = NULL;
		}

		prv++;
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;
}

/*
 * vsp_lib_open - Open VSP driver routine
 * @ch:			channel number
 * @sub_ercd:	detail error code
 *
 */
int vsp_lib_open(unsigned char ch, long *sub_ercd)
{
	VSP_PRIVATE_DATA *prv;
	struct vsp_ch_info *ch_info;
	unsigned char vsp = 0;
	unsigned char wpf = 0;

	long ercd;

	DPRINT("called ch=%d\n", ch);

	/* check parameter */
	ercd = vsp_ins_get_vsp_ip_num(&vsp, &wpf, ch);
	if (ercd != 0) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}
	prv = g_vsp_obj[vsp];

	/* check condition */
	if (prv == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_NO_INIT;
		return -EACCES;
	}
	ch_info = &prv->ch_info[wpf];

	/* check status */
	if (ch_info->status != VSP_STAT_INIT) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_INVALID_STATE;
		return -EBUSY;
	}

	/* update status */
	ch_info->status = VSP_STAT_READY;

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done vsp=%d, wpf=%d\n", vsp, wpf);
	return 0;
}

/*
 * vsp_lib_close - Close VSP driver routine
 * @ch:			channel number
 * @sub_ercd:	detail error code
 *
 */
int vsp_lib_close(unsigned char ch, long *sub_ercd)
{
	VSP_PRIVATE_DATA *prv;
	struct vsp_ch_info *ch_info;
	unsigned char vsp = 0;
	unsigned char wpf = 0;

	long ercd;

	DPRINT("called ch=%d\n", ch);

	/* check parameter */
	ercd = vsp_ins_get_vsp_ip_num(&vsp, &wpf, ch);
	if (ercd != 0) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}
	prv = g_vsp_obj[vsp];

	/* check condition */
	if (prv == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_NO_INIT;
		return -EACCES;
	}
	ch_info = &prv->ch_info[wpf];

	/* check status */
	if (ch_info->status != VSP_STAT_READY) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_INVALID_STATE;
		return -EACCES;
	}

	/* update status */
	ch_info->status = VSP_STAT_INIT;

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done vsp=%d, wpf=%d\n", vsp, wpf);
	return 0;
}

/*
 * vsp_lib_start - Start VSP driver routine
 * @ch:			channel number
 * @callback:	callback function
 * @param:		stating parameter
 * @userdata:	userdata
 * @sub_ercd:	detail error code
 *
 */
int vsp_lib_start(unsigned char ch, void *callback,
	T_VSP_START *param, void *userdata, long *sub_ercd)
{
	VSP_PRIVATE_DATA *prv;
	struct vsp_ch_info *ch_info;
	unsigned char vsp = 0;
	unsigned char wpf = 0;

	long ercd;

	DPRINT("called ch=%d\n", ch);

	/* check parameter */
	ercd = vsp_ins_get_vsp_ip_num(&vsp, &wpf, ch);
	if (ercd != 0) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}
	prv = g_vsp_obj[vsp];

	/* check condition */
	if (prv == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_NO_INIT;
		return -EACCES;
	}
	ch_info = &prv->ch_info[wpf];

	/* check parameter */
	if (callback == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_PARA_CB;
		return -EINVAL;
	}

	if (param == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_PARA_INPAR;
		return -EINVAL;
	}

	/* check status */
	if (ch_info->status != VSP_STAT_READY) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_INVALID_STATE;
		return -EACCES;
	}

	/* update status */
	ch_info->status = VSP_STAT_RUN;

	/* check start parameter */
	ercd = vsp_ins_check_start_parameter(prv, vsp, wpf, param);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;

		/* update status */
		ch_info->status = VSP_STAT_READY;

		if ((ercd == E_VSP_BUSY_RPF_OVER) ||
			(ercd == E_VSP_BUSY_MODULE_OVER)) {
			return -EBUSY;
		} else {
			return -EINVAL;
		}
	}

	/* set start parameter */
	ercd = vsp_ins_set_start_parameter(prv, wpf, param);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;

		/* update status */
		ch_info->status = VSP_STAT_READY;

		return -EIO;
	}

	/* set callback information */
	ch_info->cb_func = callback;
	ch_info->cb_userdata = userdata;

	/* set using flag */
	disable_irq(prv->irq);
	prv->use_rpf |= ch_info->reserved_rpf;
	prv->use_module |= ch_info->reserved_module;
	enable_irq(prv->irq);

	/* start */
	vsp_ins_start_processing(ch_info);

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done vsp=%d, wpf=%d\n", vsp, wpf);
	return 0;
}

/*
 * vsp_lib_abort - Stop VSP driver routine
 * @ch:			channel number
 * @sub_ercd:	detail error code
 *
 */
int vsp_lib_abort(unsigned char ch, long *sub_ercd)
{
	VSP_PRIVATE_DATA *prv;
	struct vsp_ch_info *ch_info;
	unsigned char vsp = 0;
	unsigned char wpf = 0;

	long ercd;

	DPRINT("called ch=%d\n", ch);

	/* check parameter */
	ercd = vsp_ins_get_vsp_ip_num(&vsp, &wpf, ch);
	if (ercd != 0) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}
	prv = g_vsp_obj[vsp];

	/* check condition */
	if (prv == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_VSP_NO_INIT;
		return -EACCES;
	}
	ch_info = &prv->ch_info[wpf];

	/* check status */
	if (ch_info->status == VSP_STAT_RUN) {
		/* stop VSP processing */
		ercd = vsp_ins_stop_processing(prv, wpf);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			return -EIO;
		}
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done vsp=%d, wpf=%d\n", vsp, wpf);
	return 0;
}

