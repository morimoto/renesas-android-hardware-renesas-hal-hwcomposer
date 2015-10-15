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

#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/pm_runtime.h>

#include "vspm_public.h"
#include "vspm_private.h"
#include "vspm_main.h"
#include "vspm_log.h"

#define gic_spi(n)		((n)+32)

/* banner */
static char banner[] __initdata =
	 KERN_INFO DRVNAME " (" __DATE__ " " __TIME__ ")" " driver installed";

/* major device number */
static int major;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number");

struct vspm_drvdata *p_vspm_drvdata;

/**
 * VSPM_lib_DriverInitialize - Initialize the VSPM driver
 * @handle: destination address of the handle
 * Description: Initialize the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long VSPM_lib_DriverInitialize(unsigned long *handle)
{
	int ercd;
	struct vspm_privdata *priv = 0;
	struct vspm_drvdata *pdrv = p_vspm_drvdata;

	down(&pdrv->init_sem);
	DPRINT("called\n");

	/* check parameter */
	if (handle == NULL)
		goto err_exit1;

	/* allocate the private data area of the vspm device file */
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		APRINT("could not allocate the private data area of the ");
		APRINT("vspm device file\n");
		goto err_exit1;
	}

	spin_lock_init(&priv->lock);

	init_completion(&priv->comp);
	init_completion(&priv->cb_start_comp);

	INIT_LIST_HEAD(&priv->req_head.list);
	INIT_LIST_HEAD(&priv->rsp_head.list);

	priv->pdrv = pdrv;

	if (atomic_add_return(1, &pdrv->counter) == 1) {
		DPRINT("first time open\n");
		/* first time open */
		ercd = vspm_init(priv);
		if (ercd) {
			APRINT("failed to vspm_init %d\n", ercd);
			goto err_exit2;
		}
	} else {
		DPRINT("already opened\n");
	}

	*handle = (unsigned long)priv;

	DPRINT("done handle=%08x\n", (unsigned int)*handle);
	up(&pdrv->init_sem);
	return R_VSPM_OK;

err_exit2:
	atomic_dec(&pdrv->counter);
	kfree(priv);

err_exit1:
	up(&pdrv->init_sem);
	return R_VSPM_NG;
}
EXPORT_SYMBOL(VSPM_lib_DriverInitialize);

/*
 * vspm_open - Open the VSPM driver
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int vspm_open(struct inode *inode, struct file *filp)
{
	long ercd;
	long handle;

	DPRINT("called\n");

	ercd = VSPM_lib_DriverInitialize(&handle);
	if (ercd != R_VSPM_OK) {
		APRINT("failed to VSPM_lib_DriverInitialize %d\n", (int)ercd);
		return -EFAULT;
	}

	filp->private_data = (void *)handle;

	DPRINT("done\n");
	return 0;
}

/**
 * VSPM_lib_DriverQuit - Exit the VSPM driver
 * @handle: handle
 * Description: Exit the driver of each IP (VSPS, 2DDAMC).
 * Returns: On success R_VSPM_OK is returned. On error, R_VSPM_NG is returned.
 */
long VSPM_lib_DriverQuit(unsigned long handle)
{
	int ercd;
	struct vspm_privdata *priv = (struct vspm_privdata *)handle;
	struct vspm_drvdata *pdrv = p_vspm_drvdata;
	int cnt;

	down(&pdrv->init_sem);
	DPRINT("called\n");

	/* check parameter */
	if (priv == NULL)
		goto err_exit1;

	if (priv->pdrv != pdrv)
		goto err_exit1;

	cnt = atomic_sub_return(1, &pdrv->counter);
	if (cnt == 0) {
		DPRINT("last close\n");
		ercd = vspm_quit(priv);
		if (ercd) {
			APRINT("failed to vspm_quit %d\n", ercd);
			goto err_exit2;
		}
	} else if (cnt > 0) {
		DPRINT("still open\n");
		ercd = vspm_cancel(priv);
		if (ercd) {
			APRINT("failed to vspm_cancel %d\n", ercd);
			goto err_exit2;
		}
	} else {
		APRINT("already closed\n");
		goto err_exit2;
	}

	up(&pdrv->init_sem);
	priv->pdrv = NULL;
	kfree(priv);

	DPRINT("done\n");
	return R_VSPM_OK;

err_exit2:
	atomic_inc(&pdrv->counter);

err_exit1:
	up(&pdrv->init_sem);
	return R_VSPM_NG;
}
EXPORT_SYMBOL(VSPM_lib_DriverQuit);

/*
 * vspm_release - Release the VSPM driver
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int vspm_release(struct inode *inode, struct file *filp)
{
	long ercd;
	long handle = (long)filp->private_data;

	DPRINT("called\n");

	ercd = VSPM_lib_DriverQuit(handle);
	if (ercd != R_VSPM_OK) {
		APRINT("failed to VSPM_lib_DriverQuit %d\n", (int)ercd);
		return -EFAULT;
	}

	filp->private_data = NULL;

	DPRINT("done\n");
	return 0;
}


static const struct file_operations vspm_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= vspm_ioctl,
	.open			= vspm_open,
	.release		= vspm_release,
};


/*
 * vspm_probe - Probe the VSPM driver
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int vspm_probe(struct platform_device *pdev)
#else
static int __devinit vspm_probe(struct platform_device *pdev)
#endif
{
	dev_t dev = 0;
	struct vspm_drvdata *pdrv = 0;
	struct device *ret_dev;
	int ercd = 0;

	DPRINT("called(id=%d)\n", pdev->id);

	/* allocate vspm driver data area */
	pdrv = kzalloc(sizeof(*pdrv), GFP_KERNEL);
	if (!pdrv) {
		APRINT("could not allocate vspm driver data area (id=%d)\n",
			pdev->id);
		ercd = -ENOMEM;
		goto exit;
	}

	/* register a range of char device numbers */
	if (major) {
		dev = MKDEV(major, 0);
		ercd = register_chrdev_region(dev, DEVNUM, DRVNAME);
		if (ercd) {
			APRINT("could not get major %d (dev=%x)\n", ercd, dev);
			dev = 0;
			goto exit;
		}
	} else {
		ercd = alloc_chrdev_region(&dev, 0, DEVNUM, DRVNAME);
		if (ercd) {
			APRINT("could not allocate major %d\n", ercd);
			goto exit;
		}
		major = MAJOR(dev);
	}
	DPRINT("major:%d\n", major);

	/* initialize a cdev structure */
	cdev_init(&pdrv->cdev, &vspm_fops);
	pdrv->cdev.owner = THIS_MODULE;

	/* add a char device to the system */
	ercd = cdev_add(&pdrv->cdev, dev, DEVNUM);
	if (ercd) {
		APRINT("could not add a char device %d (dev=%x)\n", ercd, dev);
		goto exit;
	}

	/* create a struct class structure */
	pdrv->pcls = class_create(THIS_MODULE, CLSNAME);
	if (IS_ERR(pdrv->pcls)) {
		APRINT("could not create a class\n");
		ercd = PTR_ERR(pdrv->pcls);
		pdrv->pcls = 0;
		goto exit;
	}
	/*class->devnode = devnode;*/

	/* creates a device and registers it with sysfs */
	ret_dev = device_create(pdrv->pcls, NULL, dev, NULL, DEVNAME);
	if (IS_ERR(ret_dev)) {
		APRINT("could not create a device\n");
		ercd = PTR_ERR(ret_dev);
		goto exit;
	}

	/* initialize open counter */
	atomic_set(&pdrv->counter, 0);

	/* save platform_device pointer */
	pdrv->pdev = pdev;

	sema_init(&pdrv->init_sem, 1);/* unlock */

	/* save drvdata */
	p_vspm_drvdata = pdrv;
	platform_set_drvdata(pdev, pdrv);

	DPRINT("done(id=%d)\n", pdev->id);
	return 0;

exit:
	if (pdrv) {
		if (pdrv->cdev.dev)
			cdev_del(&pdrv->cdev);
		if (pdrv->pcls)
			class_destroy(pdrv->pcls);
		if (dev)
			unregister_chrdev_region(dev, DEVNUM);
		kfree(pdrv);
	}
	return ercd;
}

/*
 * vspm_remove - Remove the VSPM driver
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int vspm_remove(struct platform_device *pdev)
#else
static int __devexit vspm_remove(struct platform_device *pdev)
#endif
{
	struct vspm_drvdata	*pdrv = platform_get_drvdata(pdev);
	dev_t dev = pdrv->cdev.dev;

	DPRINT("called(id=%d)\n", pdev->id);

	/* emoves a device that was created with device_create() */
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

static int vspm_runtime_nop(struct device *dev)
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

static const struct dev_pm_ops vspm_pm_ops = {
	.runtime_suspend = vspm_runtime_nop,
	.runtime_resume = vspm_runtime_nop,
	.suspend = vspm_runtime_nop,
	.resume = vspm_runtime_nop,
};

/* vspm driver platform-data */
static struct platform_driver vspm_driver = {
	.driver		= {
		.name		= DRVNAME,
		.owner		= THIS_MODULE,
		.pm			= &vspm_pm_ops,
	},
	.probe		= vspm_probe,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	.remove		= vspm_remove,
#else
	.remove		= __devexit_p(vspm_remove),
#endif
};

/* vspm device resources */
static struct resource vspm_resources[] = {
	/* 2D-DMAC */
	[0] = {
		.name	= RESNAME "-tddmac",
		.start	= 0xFEA00000,
		.end	= 0xFEA00200 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= RESNAME "-tddmac",
		.start	= gic_spi(285),
		.flags	= IORESOURCE_IRQ,
	},
	/* VSPR */
	[2] = {
		.name	= RESNAME "-vspr",
		.start	= 0xFE920000,
		.end	= 0xFE927404 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.name	= RESNAME "-vspr",
		.start	= gic_spi(266),
		.flags	= IORESOURCE_IRQ,
	},
	/* VSPS */
	[4] = {
		.name	= RESNAME "-vsps",
		.start	= 0xFE928000,
		.end	= 0xFE92F404 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[5] = {
		.name	= RESNAME "-vsps",
		.start	= gic_spi(267),
		.flags	= IORESOURCE_IRQ,
	},
	/* VSPD0 */
	[6] = {
		.name	= RESNAME "-vspd0",
		.start	= 0xFE930000,
		.end	= 0xFE937404 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[7] = {
		.name	= RESNAME "-vspd0",
		.start	= gic_spi(246),
		.flags	= IORESOURCE_IRQ,
	},
	/* VSPD1 */
	[8] = {
		.name	= RESNAME "-vspd1",
		.start	= 0xFE938000,
		.end	= 0xFE93F404 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[9] = {
		.name	= RESNAME "-vspd1",
		.start	= gic_spi(247),
		.flags	= IORESOURCE_IRQ,
	},
};

/* vspm device release function */
static void vspm_dev_release(struct device *dev)
{
	DPRINT("do not do anything\n");
	return;
}

/* vspm device */
static struct platform_device vspm_device = {
	.name			= DEVNAME,
	.id				= -1,
	.resource		= vspm_resources,
	.num_resources	= ARRAY_SIZE(vspm_resources),
	.dev			= {
						.release = vspm_dev_release,
					},
};

#ifdef DEBUG
#define PROCNAME_DEBUG	"debug"
long debug_flg;
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_debug_entry;

static int vspm_proc_read_debug(
	char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	DPRINT("called\n");

	if (off > 0)
		len = 0;
	else
		len = sprintf(page, "%ld\n", debug_flg);

	DPRINT("done\n");
	return len;
}

static int vspm_proc_write_debug(
	struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[16];
	unsigned long len = count;

	DPRINT("called\n");

	if (len >= sizeof(buf))
		len = sizeof(buf) - 1;

	if (copy_from_user(buf, buffer, len))
		return -EFAULT;
	buf[len] = '\0';

	if (strict_strtol(buf, 10, &debug_flg))
		return -EINVAL;

	DPRINT("done\n");
	return len;
}
#endif	/* DEBUG */

/*
 * vspm_module_init - Initialize the VSPM driver module
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int __init vspm_module_init(void)
{
	int ercd = 0;

	DPRINT("called\n");

	/* add a platform-level device */
	ercd = platform_device_register(&vspm_device);
	if (ercd) {
		APRINT("could not add a platform-level device %d\n", ercd);
		goto err_exit1;
	}

	/* register a driver for platform-level devices */
	ercd = platform_driver_register(&vspm_driver);
	if (ercd) {
		APRINT("could not register a driver for ");
		APRINT("platform-level devices %d\n", ercd);
		goto err_exit2;
	}

#ifdef DEBUG
	/* make a directory /proc/driver/vspm */
	proc_dir = proc_mkdir(PROCNAME, NULL);
	if (!proc_dir) {
		APRINT("could not make a proc dir\n");
		goto err_exit3;
	}

	/* make a proc entry /proc/driver/vspm/debug */
	proc_debug_entry = create_proc_entry(PROCNAME_DEBUG, 0666, proc_dir);
	if (!proc_debug_entry) {
		APRINT("could not make a proc entry\n");
		goto err_exit4;
	}
	proc_debug_entry->read_proc = vspm_proc_read_debug;
	proc_debug_entry->write_proc = vspm_proc_write_debug;
#endif	/* DEBUG */

	printk(banner);
	printk("\n");

	DPRINT("done\n");
	return 0;

#ifdef DEBUG
err_exit4:
	remove_proc_entry(PROCNAME, NULL);
err_exit3:
	platform_driver_unregister(&vspm_driver);
#endif	/* DEBUG */
err_exit2:
	platform_device_unregister(&vspm_device);
err_exit1:
	return ercd;
}

/*
 * vspm_module_exit - Exit the VSPM driver module
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static void __exit vspm_module_exit(void)
{
	DPRINT("called\n");

	platform_driver_unregister(&vspm_driver);
	platform_device_unregister(&vspm_device);

#ifdef DEBUG
	if (proc_debug_entry)
		remove_proc_entry(PROCNAME_DEBUG, proc_dir);
	if (proc_dir)
		remove_proc_entry(PROCNAME, NULL);
#endif	/* DEBUG */

	DPRINT("done\n");
}

module_init(vspm_module_init);
module_exit(vspm_module_exit);

MODULE_AUTHOR("Renesas Electronics Corporation");
MODULE_LICENSE("Dual MIT/GPL");

