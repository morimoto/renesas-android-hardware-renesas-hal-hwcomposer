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
#include "tddmac_drv.h"
#include "tddmac_drv_public.h"
#include "tddmac_drv_local.h"

static TDDMAC_PRIVATE_DATA *g_tddmac_obj;

/*
 * tddmac_lib_init - Initialize 2DDMAC driver routine
 * @priv:		VSP manager private data
 * @param:		initialize parameter
 * @sub_ercd:	detail error code
 *
 */
int tddmac_lib_init(
	struct platform_device *pdev, T_TDDMAC_INIT *param, long *sub_ercd)
{
	unsigned char lp_ch;

	long ercd = 0;
	int drv_ercd = 0;

	DPRINT("called\n");

	/* check parameter */
	if ((pdev == NULL) || (param == NULL)) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_PARAMS;
		return -EINVAL;
	}

	/* check initialize parameter */
	ercd = tddmac_ins_check_init_param(param);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}

	/* check condition */
	if (g_tddmac_obj != NULL) {
		/* quit */
		if (tddmac_lib_quit(sub_ercd))
			return -EIO;
	}

	/* allocate private data */
	ercd = tddmac_ins_allocate_memory(&g_tddmac_obj);
	if (ercd) {
		drv_ercd = -ENOMEM;
		goto err_exit;
	}

	/* initialize register */
	ercd = tddmac_ins_init_reg(pdev, g_tddmac_obj);
	if (ercd) {
		drv_ercd = -EIO;
		goto err_exit;
	}

	/* registory interrupt handler */
	ercd = tddmac_ins_reg_ih(pdev, g_tddmac_obj);
	if (ercd) {
		drv_ercd = -EIO;
		goto err_exit;
	}

	/* update status */
	for (lp_ch = 0; lp_ch < TDDMAC_CH_MAX; lp_ch++)
		g_tddmac_obj->ch_info[lp_ch].status = TDDMAC_STAT_INIT;

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;

err_exit:
	if (g_tddmac_obj != NULL) {
		/* unregistory interupt handler */
		(void)tddmac_ins_unreg_ih(g_tddmac_obj);

		/* finalize register */
		(void)tddmac_ins_quit_reg(g_tddmac_obj);

		/* release memory */
		(void)tddmac_ins_free_memory(g_tddmac_obj);

		g_tddmac_obj = NULL;
	}

	if (sub_ercd != NULL)
		*sub_ercd = ercd;
	return drv_ercd;
}

/*
 * tddmac_lib_quit - Finalize 2DDMAC driver routine
 * @sub_ercd:	detail error code
 *
 */
int tddmac_lib_quit(long *sub_ercd)
{
	long ercd = 0;
	int i;

	DPRINT("called\n");

	/* check condition */
	if (g_tddmac_obj != NULL) {
		for (i = 0; i < TDDMAC_CH_MAX; i++) {
			if (g_tddmac_obj->ch_info[i].status
					== TDDMAC_STAT_RUN) {
				/* stop DMA transfer */
				ercd = tddmac_ins_stop_dma(
					&g_tddmac_obj->ch_info[i]);
				if (ercd) {
					if (sub_ercd != NULL)
						*sub_ercd = ercd;
					return -EIO;
				}
			}

			/* update status */
			g_tddmac_obj->ch_info[i].status = TDDMAC_STAT_NOT_INIT;
		}

		/* unregistory interrupt handler */
		ercd = tddmac_ins_unreg_ih(g_tddmac_obj);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			return -EIO;
		}

		/* Finalize register */
		ercd = tddmac_ins_quit_reg(g_tddmac_obj);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			return -EIO;
		}

		/* release memory */
		ercd = tddmac_ins_free_memory(g_tddmac_obj);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			return -EIO;
		}

		g_tddmac_obj = NULL;
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;
}

/*
 * tddmac_lib_open - Open channel routine
 * @ch:			channel number
 * @param:		mode parameter
 * @sub_ercd:	detail error code
 *
 */
int tddmac_lib_open(unsigned char ch, T_TDDMAC_MODE *param, long *sub_ercd)
{
	long ercd = 0;

	DPRINT("called\n");

	/* check condition */
	if (g_tddmac_obj == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_NO_INIT;
		return -EACCES;
	}

	/* check parameter */
	if (param == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_PARAMS;
		return -EINVAL;
	}

	if (ch >= TDDMAC_CH_MAX) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_CHANNEL;
		return -EINVAL;
	}

	/* check mode parameter */
	ercd = tddmac_ins_check_mode_param(param);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}

	/* check status */
	if (g_tddmac_obj->ch_info[ch].status != TDDMAC_STAT_INIT) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_INVALID_STATE;
		return -EBUSY;
	}

	/* update status */
	g_tddmac_obj->ch_info[ch].status = TDDMAC_STAT_READY;

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;
}

/*
 * tddmac_lib_close - Close channel routine
 * @ch:			channel number
 * @sub_ercd:	detail error code
 *
 */
int tddmac_lib_close(unsigned char ch, long *sub_ercd)
{
	struct tddmac_ch_info *ch_info;

	DPRINT("called\n");

	/* check condition */
	if (g_tddmac_obj == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_NO_INIT;
		return -EACCES;
	}

	/* check parameter */
	if (ch >= TDDMAC_CH_MAX) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_CHANNEL;
		return -EINVAL;
	}

	ch_info = &g_tddmac_obj->ch_info[ch];

	/* check status */
	if (ch_info->status == TDDMAC_STAT_RUN) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_INVALID_STATE;
		return -EBUSY;
	}

	/* close processing */
	if (ch_info->status == TDDMAC_STAT_READY) {
		/* update status */
		ch_info->status = TDDMAC_STAT_INIT;
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;
}

/*
 * tddmac_lib_start - Start DMA transfer routine
 * @ch:			channel number
 * @param:		request parameter
 * @sub_ercd:	detail error code
 *
 */
int tddmac_lib_start(unsigned char ch, T_TDDMAC_REQUEST *param, long *sub_ercd)
{
	struct tddmac_ch_info *ch_info;
	long ercd = 0;

	DPRINT("called\n");

	/* check condition */
	if (g_tddmac_obj == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_NO_INIT;
		return -EACCES;
	}

	/* check parameter */
	if (param == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_PARAMS;
		return -EINVAL;
	}

	if (ch >= TDDMAC_CH_MAX) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_CHANNEL;
		return -EINVAL;
	}

	/* check request parameter */
	ercd = tddmac_ins_check_request_param(param);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;
		return -EINVAL;
	}

	ch_info = &g_tddmac_obj->ch_info[ch];

	/* check status */
	if (ch_info->status != TDDMAC_STAT_READY) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_INVALID_STATE;
		return -EACCES;
	}

	/* update status */
	ch_info->status = TDDMAC_STAT_RUN;

	/* set request parameter */
	tddmac_ins_set_request_param(ch_info, param);

	/* start DMA transfer */
	ercd = tddmac_ins_start_dma(ch_info);
	if (ercd) {
		if (sub_ercd != NULL)
			*sub_ercd = ercd;

		/* update status */
		ch_info->status = TDDMAC_STAT_READY;
		return -EIO;
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;
}

/*
 * tddmac_lib_abort - Abort DMA transfer routine
 * @ch:			channel number
 * @sub_ercd:	detail error code
 *
 */
int tddmac_lib_abort(unsigned char ch, long *sub_ercd)
{
	struct tddmac_ch_info *ch_info;
	long ercd = 0;

	DPRINT("called\n");

	/* check condition */
	if (g_tddmac_obj == NULL) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_NO_INIT;
		return -EACCES;
	}

	/* check parameter */
	if (ch >= TDDMAC_CH_MAX) {
		if (sub_ercd != NULL)
			*sub_ercd = E_TDDMAC_PARA_CHANNEL;
		return -EINVAL;
	}

	ch_info = &g_tddmac_obj->ch_info[ch];

	/* stop processing */
	if (ch_info->status == TDDMAC_STAT_RUN) {
		/* stop DMA transfer */
		ercd = tddmac_ins_stop_dma(ch_info);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			return -EIO;
		}

		/* check status */
		if (ch_info->status == TDDMAC_STAT_RUN) {
			/* callback function */
			tddmac_ins_cb_function(ch_info, ch, -ECANCELED);
		}
	}

	if (sub_ercd != NULL)
		*sub_ercd = 0;

	DPRINT("done\n");
	return 0;
}
