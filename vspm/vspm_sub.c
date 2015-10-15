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

#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>

#include "frame.h"
#include "vspm_public.h"
#include "vspm_private.h"
#include "vspm_main.h"
#include "vspm_log.h"

/*
 * vspm_thread - VSPM thread main routine
 * @num:
 *
 */
static int vspm_thread(void *num)
{
	DPRINT("called\n");

	vspm_task();

	DPRINT("done\n");
	return 0;
}

/*
 * vspm_init - Initialize the VSPM thread
 * @priv: VSPM driver private data
 *
 */
int vspm_init(struct vspm_privdata *priv)
{
	struct vspm_drvdata *pdrv = priv->pdrv;
	struct platform_device *pdev = pdrv->pdev;

	long drv_ercd;
	int ercd;

	DPRINT("called\n");

	/* wake up device */
	pm_suspend_ignore_children(&pdev->dev, true);
	pm_runtime_enable(&pdev->dev);

	pm_runtime_get_sync(&pdev->dev);

	/* enable clock */
	pdrv->tddmac_clk = clk_get(NULL, "tddmac");
	if (IS_ERR(pdrv->tddmac_clk)) {
		APRINT("failed to get 2DDMAC clock\n");
		ercd = -EFAULT;
		goto err_exit1;
	}

	ercd = clk_prepare_enable(pdrv->tddmac_clk);
	if (ercd < 0) {
		APRINT("failed to stating 2DDMAC clock\n");
		ercd = -EFAULT;
		goto err_exit2;
	}

	pdrv->vsps_clk = clk_get(NULL, "vsps");
	if (IS_ERR(pdrv->vsps_clk)) {
		APRINT("failed to get VSPS clock\n");
		ercd = -EFAULT;
		goto err_exit3;
	}

	ercd = clk_prepare_enable(pdrv->vsps_clk);
	if (ercd < 0) {
		APRINT("failed to stating VSPS clock\n");
		ercd = -EFAULT;
		goto err_exit4;
	}

#ifdef USE_VSPR
	pdrv->vspr_clk = clk_get(NULL, "vspr");
	if (IS_ERR(pdrv->vspr_clk)) {
		APRINT("failed to get VSPR clock\n");
		ercd = -EFAULT;
		goto err_exit5;
	}

	ercd = clk_prepare_enable(pdrv->vspr_clk);
	if (ercd < 0) {
		APRINT("failed to stating VSPR clock\n");
		ercd = -EFAULT;
		goto err_exit6;
	}
#endif

#ifdef USE_VSPD0
	pdrv->vspd0_clk = clk_get(NULL, "vsp1-du0");
	if (IS_ERR(pdrv->vspd0_clk)) {
		APRINT("failed to get VSPD0 clock\n");
		ercd = -EFAULT;
		goto err_exit11;
	}

	ercd = clk_prepare_enable(pdrv->vspd0_clk);
	if (ercd < 0) {
		APRINT("failed to stating VSPD0 clock\n");
		ercd = -EFAULT;
		goto err_exit12;
	}
#endif

#ifdef USE_VSPD1
	pdrv->vspd1_clk = clk_get(NULL, "vsp1-du1");
	if (IS_ERR(pdrv->vspd1_clk)) {
		APRINT("failed to get VSPD1 clock\n");
		ercd = -EFAULT;
		goto err_exit13;
	}

	ercd = clk_prepare_enable(pdrv->vspd1_clk);
	if (ercd < 0) {
		APRINT("failed to stating VSPD1 clock\n");
		ercd = -EFAULT;
		goto err_exit14;
	}
#endif

	drv_ercd = vspm_lib_driver_initialize(pdev);
	if (drv_ercd != R_VSPM_OK) {
		APRINT("failed to vspm_lib_driver_initialize %d\n",
			(int)drv_ercd);
		ercd = -EFAULT;
		goto err_exit7;
	}

	/* Initialize the framework */
	fw_initialize();

	/* Register VSPM task to framework */
	ercd = fw_task_register(TASK_VSPM);
	if (ercd) {
		APRINT("failed to fw_task_register\n");
		ercd = -EFAULT;
		goto err_exit8;
	}

	/* create vspm thread */
	pdrv->task = kthread_run(vspm_thread, priv, THREADNAME);
	if (IS_ERR(pdrv->task)) {
		APRINT("failed to kthread_run\n");
		ercd = -EFAULT;
		goto err_exit9;
	}

	/* Send FUNC_TASK_INIT message */
	drv_ercd = fw_send_function(TASK_VSPM, FUNC_TASK_INIT, 0, NULL);
	if (drv_ercd != FW_OK) {
		APRINT("failed to fw_send_function(FUNC_TASK_INIT)\n");
		ercd = -EFAULT;
		goto err_exit10;
	}

	DPRINT("done\n");
	return 0;

err_exit10:
	/* Send FUNC_TASK_QUIT message */
	fw_send_function(TASK_VSPM, FUNC_TASK_QUIT, 0, NULL);
err_exit9:
	/* Unregister VSPM task */
	fw_task_unregister(TASK_VSPM);
err_exit8:
	vspm_lib_driver_quit();
err_exit7:
#ifdef USE_VSPD1
	clk_disable_unprepare(pdrv->vspd1_clk);
err_exit14:
	clk_put(pdrv->vspd1_clk);
err_exit13:
#endif
#ifdef USE_VSPD0
	clk_disable_unprepare(pdrv->vspd0_clk);
err_exit12:
	clk_put(pdrv->vspd0_clk);
err_exit11:
#endif
#ifdef USE_VSPR
	clk_disable_unprepare(pdrv->vspr_clk);
err_exit6:
	clk_put(pdrv->vspr_clk);
err_exit5:
#endif
	clk_disable_unprepare(pdrv->vsps_clk);
err_exit4:
	clk_put(pdrv->vsps_clk);
err_exit3:
	clk_disable_unprepare(pdrv->tddmac_clk);
err_exit2:
	clk_put(pdrv->tddmac_clk);
err_exit1:
	/* mark device as idle */
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return ercd;
}

/*
 * vspm_quit - Exit the VSPM thread
 * @priv: VSPM driver private data
 *
 */
int vspm_quit(struct vspm_privdata *priv)
{
	struct vspm_drvdata *pdrv = priv->pdrv;
	struct platform_device *pdev = pdrv->pdev;

	long drv_ercd;
	int ercd;

	DPRINT("called\n");

	/* Send FUNC_TASK_QUIT message */
	drv_ercd = fw_send_function(TASK_VSPM, FUNC_TASK_QUIT, 0, NULL);
	if (drv_ercd != FW_OK) {
		APRINT("failed to fw_send_function(FUNC_TASK_QUIT)\n");
		ercd = -EFAULT;
		goto exit;
	}

	/* Unregister VSPM task */
	(void)fw_task_unregister(TASK_VSPM);

	drv_ercd = vspm_lib_driver_quit();
	if (drv_ercd != R_VSPM_OK) {
		APRINT("failed to vspm_lib_driver_quit %d\n", (int)drv_ercd);
		ercd = -EFAULT;
		goto exit;
	}

	/* disable clock */
#ifdef USE_VSPD1
	clk_disable_unprepare(pdrv->vspd1_clk);
	clk_put(pdrv->vspd1_clk);
#endif

#ifdef USE_VSPD0
	clk_disable_unprepare(pdrv->vspd0_clk);
	clk_put(pdrv->vspd0_clk);
#endif

#ifdef USE_VSPR
	clk_disable_unprepare(pdrv->vspr_clk);
	clk_put(pdrv->vspr_clk);
#endif

	clk_disable_unprepare(pdrv->vsps_clk);
	clk_put(pdrv->vsps_clk);

	clk_disable_unprepare(pdrv->tddmac_clk);
	clk_put(pdrv->tddmac_clk);

	/* mark device as idle */
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	DPRINT("done\n");
	return 0;

exit:
	return ercd;
}


/*
 * vspm_cancel - Cancel the VSPM thread
 * @priv: VSPM driver private data
 *
 */
int vspm_cancel(struct vspm_privdata *priv)
{
	long ercd;

	DPRINT("called\n");

	ercd = vspm_lib_forced_cancel((unsigned long)priv);
	if (ercd != R_VSPM_OK) {
		APRINT("failed to vspm_lib_forced_cancel %d\n", (int)ercd);
		return -EFAULT;
	}

	DPRINT("done\n");
	return 0;
}

