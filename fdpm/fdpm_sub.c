/*************************************************************************/ /*
 FDPM

 Copyright (C) 2013 Renesas Electronics Corporation

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

#include <asm/io.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/signal.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>

#include "fdpm_api.h"
#include "fdp/fdp_depend.h"
#include "fdp/fdp_drv_l.h"
#include "fdpm_drv.h"
#include "fdpm_if_par.h"
#include "fdpm_if.h"
#include "include/fdpm_def.h"
#include "include/fdpm_depend.h"
#include "include/fdpm_main.h"
#include "include/fdpm_lfunc.h"
#include "include/fdpm_log.h"
#include "fdp/fdp_drv.h"
#include "mmngr_public.h"

#define CPU_VCR 0xFF000044
unsigned int cpu_vcr;


int fdpm_thread(struct fdpm_privdata *priv)
{
	int i;
	int rtn = 0;
	struct fdpm_drvdata *pdrv = priv->pdrv;

	DPRINT("called\n");

	for (i = 0; i < FDPM_FDP_NUM; i++) {
		priv->set_thread[i].pdrv = pdrv;
		priv->set_thread[i].FDP_obj = priv->FDP_obj[i];
		priv->set_thread[i].fdpm_ientry = &pdrv->fdpm_ientry[i];
		priv->set_thread[i].post_thread = &priv->post_thread[i];
		init_completion(&priv->set_thread[i].sync);
		init_completion(&priv->set_thread[i].start);
		priv->post_thread[i].pdrv = pdrv;
		priv->post_thread[i].comp = &pdrv->comp[i];
		priv->post_thread[i].FDP_obj = priv->FDP_obj[i];
		priv->post_thread[i].fdpm_wentry = &pdrv->fdpm_iwentry[i];
		priv->post_thread[i].quit_flag = 0;
		init_completion(&priv->post_thread[i].sync);
		init_completion(&priv->post_thread[i].start);
		init_completion(&priv->post_thread[i].start2);
		init_completion(&priv->post_thread[i].end_fdp);
		priv->set_thread[i].th = kthread_create(fdpm_drv_set_thread,
						(void *)(&priv->set_thread[i]),
							"set_thread");
		if (IS_ERR(priv->set_thread[i].th)) {
			APRINT("kthread_create (fdpm_drv_set_thread) fail.\n");
			return -EFAULT;
		}
		wake_up_process(priv->set_thread[i].th);
		wait_for_completion(&priv->set_thread[i].sync);
		DPRINT("set_thread running.\n");
		priv->post_thread[i].th = kthread_create(fdpm_post_thread,
						(void *)(&priv->post_thread[i]),
							 "post_thread");
		if (IS_ERR(priv->post_thread[i].th)) {
			APRINT("kthread_create (fdpm_post_thread) fail \n");
			return -EFAULT;
		}
		wake_up_process(priv->post_thread[i].th);
		wait_for_completion(&priv->post_thread[i].sync);
		DPRINT("post_thread running.\n");
	}

	DPRINT("done\n");
	return rtn;
}

int fdpm_init(struct fdpm_privdata *priv)
{
	int                    ret_val;
	struct fdpm_drvdata    *pdrv = priv->pdrv;
	struct platform_device *pdev = pdrv->pdev;
	int                    ercd;
	long                   sub_ercd;

	DPRINT("called\n");

	/* wake up device */
	pm_suspend_ignore_children(&pdev->dev, true);
	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	/* enable clock */
	pdrv->fdpm_clk[0] = clk_get(NULL, "fdp0");
	if (IS_ERR(pdrv->fdpm_clk[0])) {
		APRINT("failed to get FDP0 clock\n");
		ercd = -EFAULT;
		goto exit;
	}
	ercd = clk_prepare_enable(pdrv->fdpm_clk[0]);
	if (ercd < 0) {
		APRINT("failed to starting FDP0 clock\n");
		ercd = -EFAULT;
		goto exit_clk0_1;
	}

#if defined(M2CONFIG) || defined(H2CONFIG)
	pdrv->fdpm_clk[1] = clk_get(NULL, "fdp1");
	if (IS_ERR(pdrv->fdpm_clk[1])) {
		APRINT("failed to get FDP1 clock\n");
		ercd = -EFAULT;
		goto exit_clk1;
	}
	ercd = clk_prepare_enable(pdrv->fdpm_clk[1]);
	if (ercd < 0) {
		APRINT("failed to starting FDP1 clock\n");
		ercd = -EFAULT;
		goto exit_clk1_1;
	}
#endif

#if defined(H2CONFIG)
	pdrv->fdpm_clk[2] = clk_get(NULL, "fdp2");
	if (IS_ERR(pdrv->fdpm_clk[2])) {
		APRINT("failed to get FDP2 clock\n");
		ercd = -EFAULT;
		goto exit_clk2;
	}
	ercd = clk_prepare_enable(pdrv->fdpm_clk[2]);
	if (ercd < 0) {
		APRINT("failed to starting FDP2 clock\n");
		ercd = -EFAULT;
		goto exit_clk2_1;
	}
#endif

	ret_val = fdpm_lib_driver_initialize(pdev, priv, &sub_ercd);
	if (ret_val != 0) {
		APRINT("failed to fdpm_lib_driver_initialize %d (%ld)\n",
		       ret_val, sub_ercd);
		ercd = -EFAULT;
		goto exit;
	}

	DPRINT("done\n");
	return 0;

#if defined(H2CONFIG)
exit_clk2_1:
	clk_put(pdrv->fdpm_clk[2]);
exit_clk2:
	clk_disable_unprepare(pdrv->fdpm_clk[1]);
#endif
#if defined(M2CONFIG) || defined(H2CONFIG)
exit_clk1_1:
	clk_put(pdrv->fdpm_clk[1]);
exit_clk1:
	clk_disable_unprepare(pdrv->fdpm_clk[0]);
#endif
exit_clk0_1:
	clk_put(pdrv->fdpm_clk[0]);
exit:
	return ercd;
}

int fdpm_lib_driver_initialize(struct platform_device *pdev,
			       struct fdpm_privdata *priv,
			       long *sub_ercd)
{
	int ercd;
	int i;
	int ret;

	if ((pdev == NULL) || (priv == NULL)) {
		ret = -EINVAL;
		goto exit;
	}

	for (i = 0; i < FDPM_FDP_NUM; i++) {
		ercd = fdp_ins_allocate_memory(&priv->FDP_obj[i]);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			ret = -ENOMEM;
			goto exit;
		}

		/* initialize register */
		ercd = fdp_ins_init_reg(pdev, i, priv->FDP_obj[i]);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			ret = -EIO;
			goto exit;
		}
		ercd = drv_FDP_Init(priv->FDP_obj[i]);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			ret = -EIO;
			goto exit;
		}
		/* registory interrupt handler */
		ercd = fdp_reg_inth(pdev, i, priv->FDP_obj[i]);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			ret = -EIO;
			goto exit;
		}
	}

	for (i = 0; i < FDPM_FDP_NUM; i++) {
		ercd = drv_FDP_Open(priv->FDP_obj[i], sub_ercd);
		if (ercd) {
			if (sub_ercd != NULL)
				*sub_ercd = ercd;
			ret = -EIO;
			goto exit;
		}
	}
	fdpm_thread(priv);

	NPRINT("%s\n", __func__);
	ret = R_FDPM_OK;
exit:
	return ret;
}

long fdp_ins_allocate_memory(T_FDPD_MEM **prv)
{
	DPRINT("called\n");

	/* allocate memory */
	*prv = kzalloc(sizeof(T_FDPD_MEM), GFP_KERNEL);
	if (!*prv) {
		APRINT("[%s] allocate memory failed!!\n", __func__);
		return -ENOMEM;
	}

	/* clear memory */
	memset(*prv, 0, sizeof(T_FDPD_MEM));

	DPRINT("done\n");
	return 0;
}

int fdpm_quit(struct fdpm_privdata *priv)
{
	int ret;
	struct fdpm_drvdata    *pdrv = priv->pdrv;
	struct platform_device *pdev = pdrv->pdev;
	long                   wret;

	DPRINT("called\n");

	/* driver quit */
	wret = fdpm_lib_driver_quit(priv);
	if (wret != 0) {
		APRINT("failed to fdpm_lib_driver_quit %ld\n", wret);
		ret = -EFAULT;
		goto exit;
	}

	/* disable clock */
#if defined(H2CONFIG)
	clk_disable_unprepare(pdrv->fdpm_clk[2]);
	clk_put(pdrv->fdpm_clk[2]);
#endif
#if defined(M2CONFIG) || defined(H2CONFIG)
	clk_disable_unprepare(pdrv->fdpm_clk[1]);
	clk_put(pdrv->fdpm_clk[1]);
#endif
	clk_disable_unprepare(pdrv->fdpm_clk[0]);
	clk_put(pdrv->fdpm_clk[0]);

	/* mark device as idle */
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	DPRINT("done\n");
	return 0;
exit:
	return ret;
}

int fdpm_lib_driver_quit(struct fdpm_privdata *priv)
{
	int ercd;
	int ret;
	long sub_ercd;
	int i;
	struct fdpm_drvdata    *pdrv = priv->pdrv;
	struct platform_device *pdev = pdrv->pdev;

	if (priv == NULL) {
		ret = 0;
		goto exit;
	}

	fdpm_resource_del_all_list(&pdrv->fdpm_independ.fdpm_itable);
	for (i = 0; i < FDPM_FDP_NUM; i++) {
		complete(&priv->set_thread[i].start);
		complete(&priv->post_thread[i].end_fdp);
		kthread_stop(priv->set_thread[i].th);
		priv->post_thread[i].quit_flag = 1;
		complete(&priv->post_thread[i].start);
		complete(&priv->post_thread[i].start2);
		kthread_stop(priv->post_thread[i].th);

		fdpm_free_inth(pdev, i, priv->FDP_obj[i]);

		ercd = drv_FDP_Quit(priv->FDP_obj[i]);
		if (ercd) {
			DPRINT("drv_FDP_Quit fail %d\n", ercd);
			sub_ercd = ercd;
			ret = -EACCES;
			goto exit;
		}
		ercd = fdp_ins_quit_reg(priv->FDP_obj[i]);
		if (ercd) {
			sub_ercd = ercd;
			ret = -EIO;
			goto exit;
		}
		ercd = fdp_ins_free_memory(priv->FDP_obj[i]);
		if (ercd) {
			sub_ercd = ercd;
			ret = -EIO;
			goto exit;
		}
		priv->FDP_obj[i] = NULL;
	}
	sub_ercd = ercd;

	DPRINT("done\n");
	return 0;
exit:
	return ret;
}

int fdpm_close_func(int stream_id, struct fdpm_private *fdpm_private_data)
{
	int hit;
	int ret = 0;
	int device_no;
	int delete_flag;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_resource_table *target_table = NULL;
	struct list_head *listptr;
	fdpm_queue_entry *entry;

	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(stream_id,
					       0,
					&fdpm_ddata->fdpm_independ.fdpm_itable,
					       &target_table);
	if (hit != -1) {
		up(&fdpm_ddata->resource_sem);
		DPRINT("fdpm_close_func:remain resource table. try remove.\n");
		device_no = target_table->device_no;
		if (down_interruptible(&fdpm_ddata->wait_sem))
			return -ERESTARTSYS;
		listptr = &fdpm_ddata->fdpm_ientryw[device_no].list;
		list_for_each(listptr,
			      &fdpm_ddata->fdpm_ientryw[device_no].list) {
			entry = (fdpm_queue_entry *)list_entry(listptr,
							       fdpm_queue_entry,
							       list);
			if (entry->source_id == stream_id) {
				/* regist wait thread queue */
				wait_for_completion_interruptible(
					&fdpm_private_data->comp_for_wait);
			complete(&fdpm_pdata->post_thread[device_no].start2);
			}
		}
		delete_flag = 1;
		while (delete_flag != 0) {
			delete_flag = 0;
			list_for_each(listptr,
				&fdpm_ddata->fdpm_ientryw[device_no].list) {
				entry = (fdpm_queue_entry *)list_entry(listptr,
							fdpm_queue_entry,
								       list);
				if (entry->source_id == stream_id) {
					delete_flag = 1;
					break;
				}
			}
			if (delete_flag == 1)
				fdpm_queue_del_entry(entry);
		}
		up(&fdpm_ddata->wait_sem);

		if (fdpm_private_data->wait_func_flag == 1) {
			fdpm_private_data->comp2_flag = 1;
			fdpm_private_data->comp2_flag_from_sub = 1;
			complete(&fdpm_private_data->comp);
		wait_for_completion_interruptible(&fdpm_private_data->comp2);
			fdpm_private_data->comp2_flag = 0;
		}
		/* fdpm resource release */
		down(&fdpm_ddata->resource_sem);
		fdpm_resource_release(target_table->ocmode,
				      target_table->vintmode,
				      device_no,
				      target_table->size,
				      &fdpm_ddata->fdpm_independ.fdpm_ipdata);
		up(&fdpm_ddata->resource_sem);
		/* still mask area release */
		mmngr_free_in_kernel(target_table->stlmsk_table.stl_msk_byte,
			(unsigned long)target_table->stlmsk_table.phy_addr[0],
			(unsigned long)target_table->stlmsk_table.stlmsk_adr[0],
		       (unsigned long)target_table->stlmsk_table.stlmsk_vadr[0],
				     MM_KERNELHEAP);
		mmngr_free_in_kernel(target_table->stlmsk_table.stl_msk_byte,
			(unsigned long)target_table->stlmsk_table.phy_addr[1],
			(unsigned long)target_table->stlmsk_table.stlmsk_adr[1],
		       (unsigned long)target_table->stlmsk_table.stlmsk_vadr[1],
				     MM_KERNELHEAP);
		down(&fdpm_ddata->resource_sem);
		ret = fdpm_table_release(stream_id, &fdpm_ddata->fdpm_independ);
		up(&fdpm_ddata->resource_sem);
		if (ret)
			DPRINT("Can not find resource table\n");
	} else {
		up(&fdpm_ddata->resource_sem);
	}
	return ret;
}
