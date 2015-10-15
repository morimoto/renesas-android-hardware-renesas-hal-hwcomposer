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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/signal.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/pm_runtime.h>

#include "fdpm_api.h"
#include "fdpm_if_par.h"
#include "fdp/fdp_depend.h"
#include "fdp/fdp_drv_l.h"
#include "fdpm_drv.h"
#include "fdpm_if.h"
#include "include/fdpm_def.h"
#include "include/fdpm_depend.h"
#include "include/fdpm_main.h"
#include "include/fdpm_lfunc.h"
#include "include/fdpm_log.h"

#define gic_spi(n) ((n)+32)

static int fdpm_open(struct inode *inode, struct file *filp);
static long drv_FDPM_release(unsigned long handle);
static int fdpm_close(struct inode *inode, struct file *filp);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int fdpm_remove(struct platform_device *pdev);
static int fdpm_probe(struct platform_device *pdev);
#else
static int __devexit fdpm_remove(struct platform_device *pdev);
static int __devinit fdpm_probe(struct platform_device *pdev);
#endif

/* banner */
#ifdef H2CONFIG
static char banner[] __initdata = KERN_INFO DRVNAME
	" (" __DATE__ " " __TIME__ ") "
	"driver for H2 installed";
#endif
#ifdef M2CONFIG
static char banner[] __initdata = KERN_INFO DRVNAME
	" (" __DATE__ " " __TIME__ ") "
	"driver for M2 installed";
#endif
#ifdef E2CONFIG
static char banner[] __initdata = KERN_INFO DRVNAME
	" (" __DATE__ " " __TIME__ ") "
	"driver for E2 installed";
#endif

/* major device number */
static int major;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number");

struct fdpm_drvdata *p_fdpm_drvdata;

static long fdpm_handle;
static unsigned long g_open_id;
/* platform device */
static struct resource fdpm_resources[] = {
	[0] = {
	.name  = RESNAME "-fdp0",
	.start = 0xFE940000,
	.end   = 0xFE942400 - 1,
	.flags = IORESOURCE_MEM,
	},
	[1] = {
	.name  = RESNAME "-fdp0",
	.start = gic_spi(262),
	.flags = IORESOURCE_IRQ,
	},
	[2] = {
	.name  = RESNAME "-fdp1",
	.start = 0xFE944000,
	.end   = 0xFE946400 - 1,
	.flags = IORESOURCE_MEM,
	},
	[3] = {
	.name  = RESNAME "-fpd1",
	.start = gic_spi(263),
	.flags = IORESOURCE_IRQ,
	},
	[4] = {
	.name  = RESNAME "-fdp2",
	.start = 0xFE948000,
	.end   = 0XFE94A400 - 1,
	.flags = IORESOURCE_MEM,
	},
	[5] = {
	.name  = RESNAME "-fdp2",
	.start = gic_spi(264),
	.flags = IORESOURCE_IRQ,
	},
};

static const struct file_operations fdpm_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = fdpm_ioctl,
	.open           = fdpm_open,
	.release        = fdpm_close,
};

static void fdpm_dev_release(struct device *dev)
{
	DPRINT("do not do anything\n");
	return;
}

static int fdpm_runtime_nop(struct device *dev)
{
	/* Runtime PM callback shared between ->runtime_suspend()
	* and ->runtime_resume(). Simply returns success.
	*
	* This driver re-initializes all registers after
	* pm_runtime_get_sync() anyway so there is no need
	* to save and restore registers here.
	*/
	return 0;
}

static const struct dev_pm_ops fdpm_pm_ops = {
	.runtime_suspend = fdpm_runtime_nop,
	.runtime_resume = fdpm_runtime_nop,
	.suspend = fdpm_runtime_nop,
	.resume = fdpm_runtime_nop,
};

/* fdpm driver platform-data */
static struct platform_driver fdpm_driver = {
	.driver = {
	.name = DRVNAME,
	.owner = THIS_MODULE,
	.pm = &fdpm_pm_ops,
	},
	.probe = fdpm_probe,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	.remove = fdpm_remove,
#else
	.remove = __devexit_p(fdpm_remove),
#endif
};

static struct platform_device fdpm_device = {
	.name          = DEVNAME,
	.id            = -1,
	.resource      = fdpm_resources,
	.num_resources = ARRAY_SIZE(fdpm_resources),
	.dev           = {
	.release       = fdpm_dev_release,
	},
};


long FDPM_lib_DriverInitialize(unsigned long *handle)
{
	int ret;
	int i;
	struct fdpm_privdata *priv = 0;
	struct fdpm_drvdata  *pdrv = p_fdpm_drvdata;

	down(&pdrv->init_sem);
	DPRINT("called");

	g_open_id = 0;
	/* allocate the private data area of the fdpm device file */
	priv = kzalloc(sizeof(struct fdpm_privdata), GFP_KERNEL);
	if (!priv) {
		APRINT("could not allocate the private data area of the ");
		APRINT("fdpm device file\n");
		ret = -ENOMEM;
		goto mem_err;
	}
	spin_lock_init(&priv->lock);
	for (i = 0; i < FDPM_FDP_NUM; i++) {
		init_completion(&pdrv->comp[i]);
		pdrv->comp2_flag[i] = 0;
		INIT_LIST_HEAD(&pdrv->fdpm_ientryw[i].list);
	}
	fdpm_request_entry_init(pdrv);
	fdpm_post_entry_init(pdrv);
	priv->pdrv = pdrv;

	if (atomic_add_return(1, &pdrv->counter) == 1) {
		DPRINT("first time open\n");
		/* first time open */
		ret = fdpm_init(priv);
		if (ret) {
			APRINT("failed to fdpm_init %d\n", ret);
			goto init_err;
		}
	} else {
		DPRINT("already opened\n");
	}

	*handle = (unsigned long)priv;

	DPRINT("done\n");
	up(&pdrv->init_sem);
	return R_FDPM_OK;

init_err:
	atomic_dec(&pdrv->counter);
mem_err:
	kfree(priv);
	up(&pdrv->init_sem);
	return R_FDPM_NG;
}
EXPORT_SYMBOL(FDPM_lib_DriverInitialize);

static int fdpm_open(struct inode *inode, struct file *filp)
{
	struct fdpm_drvdata *pdrv = p_fdpm_drvdata;
	struct fdpm_private *fdpm_private_data = NULL;
	DPRINT("called\n");

	down(&pdrv->init_sem);
	fdpm_private_data = kzalloc(sizeof(struct fdpm_private), GFP_KERNEL);
	if (fdpm_private_data == NULL)
		return -1;

	fdpm_private_data->fdpm_handle = fdpm_handle;
	fdpm_private_data->open_id = g_open_id;
	init_completion(&fdpm_private_data->comp);
	init_completion(&fdpm_private_data->comp2);
	init_completion(&fdpm_private_data->comp_for_wait);
	fdpm_private_data->comp2_flag = 0;
	fdpm_private_data->comp2_flag_from_sub = 0;
	g_open_id++;
	filp->private_data = (void *)fdpm_private_data;
	up(&pdrv->init_sem);

	DPRINT("done\n");
	return 0;
}

long drv_FDPM_release(unsigned long handle)
{
	int ret;
	struct fdpm_privdata *priv = (struct fdpm_privdata *)handle;
	struct fdpm_drvdata  *pdrv = priv->pdrv;

	if (atomic_dec_and_test(&pdrv->counter)) {
		ret = fdpm_quit(priv);
		if (ret) {
			APRINT("failed to fdpm_quit %d\n", ret);
			goto exit;
		}
	} else {
		DPRINT("still open\n");
	}
	kfree(priv);
	return R_FDPM_OK;
exit:
	return R_FDPM_NG;
}

static int fdpm_close(struct inode *inode, struct file *filp)
{
	int ret;
	struct fdpm_private *fdpm_private_data = filp->private_data;
	DPRINT("called\n");
	ret = fdpm_close_func(fdpm_private_data->open_id, fdpm_private_data);
	DPRINT("fdpm_close open_id:%ld ret:%d\n",
	       fdpm_private_data->open_id,
	       ret);
	kfree(fdpm_private_data);
	DPRINT("done\n");
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int fdpm_remove(struct platform_device *pdev)
#else
static int __devexit fdpm_remove(struct platform_device *pdev)
#endif
{
	struct fdpm_drvdata *pdrv = platform_get_drvdata(pdev);
	dev_t               dev   = pdrv->cdev.dev;

	DPRINT("called(id=%d)\n", pdev->id);

	/* remove a device that was created sith device_create() */
	device_destroy(pdrv->pcls, dev);
	/* destroys a struct class structure */
	class_destroy(pdrv->pcls);
	/* remove a cdev from the system */
	cdev_del(&pdrv->cdev);
	unregister_chrdev_region(dev, DEVNUM);
	kfree(pdrv);
	platform_set_drvdata(pdev, NULL);

	DPRINT("done(id=%d)\n", pdev->id);
	return 0;
}

#ifdef DEBUG
#define PROCNAME_DEBUG "DEBUG"

long debug_flg;
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_debug_dntry;

static int fdpm_proc_read_debug(char *page,
				char **start,
				off_t off,
				int count,
				int *eof,
				void *data)
{
	int len;

	DPRINT("called");

	if (off > 0)
		len = 0;
	else
		len = sprintf(page, "%ld\n", debug_flg);

	DPRINT("done\n");
	return len;
}

static int fdpm_proc_write_debug(struct file *file,
				 const char *buffer,
				 unsigned long count,
				 void *data)
{
	char buf[16];
	unsigned long len = count;

	DPRINT("called\n");
	if (len >= sizeof(buf))
		len = sizeof(buf) - 1;


	if (copy_from_user(buf, buffer, len)) {
		len = -EFAULT;
		goto exit;
	}
	buf[len] = '\0';

	if (kstrtol(buf, 10, &debug_flg)) {
		len = -EINVAL;
		goto exit;
	}
	DPRINT("done\n");
exit:
	return len;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int fdpm_probe(struct platform_device *pdev)
#else
static int __devinit fdpm_probe(struct platform_device *pdev)
#endif
{
	dev_t               dev   = 0;
	struct fdpm_drvdata *pdrv = 0;
	struct device       *ret_dev;
	int                 ret   = 0;

	DPRINT("called(id=%d)\n", pdev->id);

	/* allocate fdpm driver data area */
	pdrv = kzalloc(sizeof(struct fdpm_drvdata), GFP_KERNEL);
	if (!pdrv) {
		APRINT("could not allocate fdpm driver data area (id=%d)\n",
		       pdev->id);
		ret = -ENOMEM;
		goto exit;
	}

	/* register a range of char device numbers */
	if (major) {
		dev = MKDEV(major, 0);
		ret = register_chrdev_region(dev, DEVNUM, DRVNAME);
		if (ret) {
			APRINT("could not get major %d (dev=%x)\n", ret, dev);
			dev = 0;
			goto exit;
		}
	} else {
		ret = alloc_chrdev_region(&dev, 0, DEVNUM, DRVNAME);
		if (ret) {
			APRINT("could not allocate major %d\n", ret);
			goto exit;
		}
		major = MAJOR(dev);
	}
	DPRINT("major:%d\n", major);

	/* initialize a cdev structure */
	cdev_init(&pdrv->cdev, &fdpm_fops);
	pdrv->cdev.owner = THIS_MODULE;

	/* add a char device to the system */
	ret = cdev_add(&pdrv->cdev, dev, DEVNUM);
	if (ret) {
		APRINT("could not add a char device %d (dev=%x)\n", ret, dev);
		goto exit;
	}

	/* create a struct class structure */
	pdrv->pcls = class_create(THIS_MODULE, CLSNAME);
	if (IS_ERR(pdrv->pcls)) {
		APRINT("could not create a class\n");
		ret = PTR_ERR(pdrv->pcls);
		pdrv->pcls = 0;
		goto exit;
	}

	/* creates a device and registers it with sysfs */
	ret_dev = device_create(pdrv->pcls, NULL, dev, NULL, DEVNAME);
	if (IS_ERR(ret_dev)) {
		APRINT("could not create a device\n");
		ret = PTR_ERR(ret_dev);
		goto exit;
	}

	/* initialize open counter */
	atomic_set(&pdrv->counter, 0);

	/* save platform_device_pointer */
	pdrv->pdev = pdev;

	sema_init(&pdrv->resource_sem, 1); /* unlock */
	sema_init(&pdrv->init_sem, 1);/* unlock */
	sema_init(&pdrv->entry_sem, 1);/* unlock */
	sema_init(&pdrv->wait_sem, 1); /* unlock */
	sema_init(&pdrv->post_sem, 1);/* unlock */
	sema_init(&pdrv->wait_sem2, 1);/* unlock */

	/* save drvdata */
	p_fdpm_drvdata = pdrv;
	platform_set_drvdata(pdev, pdrv);

	DPRINT("done(id=%d)\n", pdev->id);
	return 0;

exit:
	if (pdrv) {
		if (pdrv->pcls)
			class_destroy(pdrv->pcls);
		if (pdrv->cdev.dev)
			cdev_del(&pdrv->cdev);
		if (dev)
			unregister_chrdev_region(dev, DEVNUM);
		kfree(pdrv);
	}
	return ret;
}

static int __init fdpm_module_init(void)
{
	int ret = 0;
	long wret = 0;

	DPRINT("called\n");

	ret = platform_device_register(&fdpm_device);
	if (ret) {
		APRINT("could not add a platform-level device %d\n", ret);
		goto exit;
	}

	ret = platform_driver_register(&fdpm_driver);
	if (ret) {
		APRINT("could not register a driver for ");
		APRINT("platform-level devices %d\n", ret);
		platform_device_unregister(&fdpm_device);
		goto exit;
	}

#ifdef DEBUG
	/* make a directory /proc/driver/fdpm */
	proc_dir = proc_mkdir(PROCNAME, NULL);
	if (!proc_dir)
		APRINT("could not make a proc dir\n");

	/* make a proc entry /proc/driver/vspm/DEBUG */
	proc_debug_entry = create_proc_entry(PROCNAME_DEBUG, 0666, proc_dir);
	if (!proc_debug_entry)
		APRINT("could not make a proc entry\n");
	proc_debug_entry->read_proc = fdpm_proc_read_debug;
	proc_debug_entry->write_proc = fdpm_proc_write_debug;
#endif

	wret = FDPM_lib_DriverInitialize((unsigned long *)&fdpm_handle);
	if (wret != R_FDPM_OK) {
		APRINT("failed to FDPM_lib_DriverInitialize %d\n", (int)wret);
		ret = -EFAULT;
		goto exit;
	}
	printk(banner);
	printk("\n");

	DPRINT("done\n");
	return 0;
exit:
	return ret;
}

static void __exit fdpm_module_exit(void)
{
	long wret;
	DPRINT("called\n");

	wret = drv_FDPM_release(fdpm_handle);
	if (wret != R_FDPM_OK)
		APRINT("failed to drv_FDPM_release %d\n", (int)wret);

	platform_driver_unregister(&fdpm_driver);
	platform_device_unregister(&fdpm_device);
#ifdef DEBUG
	if (proc_debug_entry)
		remove_proc_entry(PROCNAME_DEBUG, proc_dir);
	if (proc_dir)
		remove_proc_entry(PROCNAME, NULL);
#endif
	DPRINT("done\n");
	return;
}
module_init(fdpm_module_init);
module_exit(fdpm_module_exit);

MODULE_AUTHOR("Renesas Electronics Corporation");
MODULE_LICENSE("Dual MIT/GPL");
