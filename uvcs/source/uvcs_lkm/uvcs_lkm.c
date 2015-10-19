/*************************************************************************/ /*
 UVCS Common

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


/******************************************************************************/
/*                    INCLUDE FILES                                           */
/******************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include "uvcs_types.h"
#include "uvcs_cmn.h"
#include "uvcs_ioctl.h"
#include "uvcs_lkm_internal.h"

/******************************************************************************/
/*                    MODULE PARAMETERS                                       */
/******************************************************************************/
ulong timeout    = UVCS_TIMEOUT_DEFAULT;
ulong uvcs_debug;

module_param(timeout, ulong, 0);
module_param(uvcs_debug, ulong, 0);	/* debug mode for UVCS-CMN library */
/******************************************************************************/
/*                    MODULE DESCRIPTIONS                                     */
/******************************************************************************/
MODULE_AUTHOR("Renesas Electronics");
MODULE_LICENSE("Dual MIT/GPL");

/******************************************************************************/
/*                    VARIABLES                                               */
/******************************************************************************/
static struct uvcs_drv_info *drv_info;


/* open (chrdev) */
static int uvcs_lkm_open(struct inode *inode, struct file *filp)
{
	struct uvcs_drv_info   *local = drv_info;
	struct uvcs_hdl_info   *hdl;
	UVCS_RESULT             uvcs_ret;
	UVCS_CMN_OPEN_PARAM_T   oparam;
	ulong                   i;
	int                     result;

	/* init hardware */
	down(&local->sem);
	if (!local->counter) {
		if (uvcs_io_init(local)) {
			dev_dbg(&local->pdev->dev, "open, rtn EFAULT");
			result = -EFAULT;
			goto err_exit_0;
		}
	}

	/* allocate memory for this handle */
	hdl = kzalloc(sizeof(struct uvcs_hdl_info), GFP_KERNEL);
	if (!hdl) {
		dev_dbg(&local->pdev->dev, "open rtn ENOMEM(1)");
		result = -ENOMEM;
		goto err_exit_1;
	}

	hdl->uvcs_hdl_work = kzalloc(local->uvcs_hdl_work_req_size, GFP_KERNEL);
	if (!hdl->uvcs_hdl_work) {
		dev_dbg(&local->pdev->dev, "open rtn ENOMEM(2)");
		result = -ENOMEM;
		goto err_exit_2;
	}

	/* init handle */
	init_waitqueue_head(&hdl->waitq);
	sema_init(&hdl->sem, 1);	/* semaphore for this handle */

	/* open uvcs-cmn library */
	oparam.struct_size     = sizeof(UVCS_CMN_OPEN_PARAM_T);
	oparam.hdl_udptr       = hdl;
	oparam.hdl_work_0_virt = hdl->uvcs_hdl_work;
	oparam.hdl_work_0_size = local->uvcs_hdl_work_req_size;
	oparam.preempt_mode    = UVCS_FALSE;
	oparam.preempt_hwid    = 0uL;

	uvcs_ret = uvcs_cmn_open(local->uvcs_info, &oparam, &hdl->uvcs_hdl);
	switch (uvcs_ret) {
	/* interrupted */
	case UVCS_RTN_CONTINUE:
		result = -ERESTARTSYS;
		goto err_exit_3;
	case UVCS_RTN_OK:
		local->counter++;
		filp->private_data = hdl;
		hdl->id    = local->hdl_serial_num++;
		hdl->local = local;
		for (i = 0uL; i < UVCS_CMN_PROC_REQ_MAX; i++)
			hdl->req_data[i].state = UVCS_REQ_NONE;
		break;
	default:
		dev_dbg(&local->pdev->dev, "open, rtn EFAULT");
		result = -EFAULT;
		goto err_exit_3;
	}

	up(&local->sem);
	return 0;

err_exit_3:
	kfree(hdl->uvcs_hdl_work);

err_exit_2:
	kfree(hdl);

err_exit_1:
	if (!local->counter)
		uvcs_io_cleanup(local);

err_exit_0:
	up(&local->sem);

	return result;
}


/* release (chrdev) */
static int uvcs_lkm_release(struct inode *inode, struct file *filp)
{
	struct uvcs_hdl_info *hdl = filp->private_data;
	struct uvcs_drv_info *local;
	UVCS_RESULT uvcs_ret;
	ulong wait_cnt = 0uL;

	hdl = filp->private_data;
	if (hdl) {
		local = hdl->local;
		do {
			uvcs_ret = uvcs_cmn_close(local->uvcs_info, hdl->uvcs_hdl, UVCS_FALSE);
			switch (uvcs_ret) {
			/* interrupted */
			case UVCS_RTN_CONTINUE:
				return -ERESTARTSYS;
			/* this handle is working */
			case UVCS_RTN_BUSY:
				if (wait_cnt >= UVCS_CLOSE_WAIT_MAX) {
					uvcs_cmn_close(local->uvcs_info, hdl->uvcs_hdl, UVCS_TRUE);
					uvcs_ret = UVCS_RTN_OK;
				} else {
					msleep(UVCS_CLOSE_WAIT_TIME);
					wait_cnt++;
				}
				break;
			case UVCS_RTN_OK:
				break;
			default:
				dev_dbg(&local->pdev->dev, "close, rtn EFAULT");
				return -EFAULT;
			}
		} while (uvcs_ret == UVCS_RTN_BUSY);

		down(&local->sem);
		filp->private_data = NULL;
		kfree(hdl->uvcs_hdl_work);
		kfree(hdl);
		if (local->counter > 1uL) {
			local->counter--;
		} else {
			local->counter = 0;
			uvcs_io_cleanup(local);
		}
		up(&local->sem);
	}

	return 0;
}


/* read (chrdev) */
static ssize_t uvcs_lkm_read(struct file *filp, char __user *buff, size_t count, loff_t *offset)
{
	struct uvcs_hdl_info *hdl = filp->private_data;
	struct platform_device *pdev;
	int              wait_result;
	int              i;

	if ((hdl == NULL) || (hdl->local == NULL) || (hdl->local->pdev == NULL)) {
		pr_debug("read, rtn EFAULT");
		return -EFAULT;
	}
	pdev = hdl->local->pdev;

	if (count != sizeof(UVCS_CMN_HW_PROC_T)) {
		dev_dbg(&pdev->dev, "read, rtn EINVAL");
		return -EINVAL;
	}

	/* into exclusive section */
	if (down_interruptible(&hdl->sem))
		return -ERESTARTSYS;

	/* waits until a result data becomes receivable */
	while (((hdl->req_data[0].state == UVCS_REQ_END) || (hdl->req_data[1].state == UVCS_REQ_END) || (hdl->req_data[2].state == UVCS_REQ_END)) == 0) {
		up(&hdl->sem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		wait_result = wait_event_interruptible_timeout(
							hdl->waitq,
							((hdl->req_data[0].state == UVCS_REQ_END) || (hdl->req_data[1].state == UVCS_REQ_END) || (hdl->req_data[2].state == UVCS_REQ_END)),
							timeout
							);
		if (wait_result == 0)
			return -ETIMEDOUT;
		if (wait_result < 0)
			return -ERESTARTSYS;
		if (down_interruptible(&hdl->sem))
			return -ERESTARTSYS;
	}

	/* search finished module number */
	for (i = 0; i < UVCS_CMN_PROC_REQ_MAX; i++) {
		if (hdl->req_data[i].state == UVCS_REQ_END) {
			/* copy result information from kernel memory to userland */
			if (copy_to_user(buff, &hdl->req_data[i].data, count)) {
				/* failed to copy */
				up(&hdl->sem);
				dev_dbg(&pdev->dev, "read, rtn EFAULT");
				return -EFAULT;
			}
			hdl->req_data[i].state = UVCS_REQ_NONE;
			up(&hdl->sem);
			return count;
		}
	}

	up(&hdl->sem);
	return 0;
}


/* write (chrdev) */
static ssize_t uvcs_lkm_write(struct file *filp, const char __user *buff, size_t count, loff_t *offset)
{
	struct uvcs_hdl_info *hdl = filp->private_data;
	struct platform_device *pdev;
	ulong temp[2];
	ssize_t result;
	struct timespec ts;
	UVCS_RESULT uvcs_ret;

	if ((hdl == NULL) || (hdl->local == NULL) || (hdl->local->pdev == NULL)) {
		pr_debug("write, rtn EFAULT(1)");
		return -EFAULT;
	}
	pdev = hdl->local->pdev;

	if (count != sizeof(UVCS_CMN_HW_PROC_T)) {
		dev_dbg(&pdev->dev, "write, rtn EINVAL(1)");
		return -EINVAL;
	}

	/* into exclusive section */
	if (down_interruptible(&hdl->sem))
		return -ERESTARTSYS;

	/* check requested data */
	if (copy_from_user(&temp, buff, 8)) {
		dev_dbg(&pdev->dev, "write, rtn EFAULT(2)");
		result = -EFAULT;
	} else if ((temp[0] < sizeof(UVCS_CMN_HW_PROC_T)) || (temp[1] >= UVCS_CMN_PROC_REQ_MAX)) {
		dev_dbg(&pdev->dev, "write, rtn EINVAL(2)");
		result = -EINVAL;
	} else if (hdl->req_data[temp[1]].state != UVCS_REQ_NONE) {
		/* state error */
		dev_dbg(&pdev->dev, "write, rtn EBUSY(1)");
		result = -EBUSY;
	} else if (copy_from_user(&hdl->req_data[temp[1]].data, buff, count)) {
		/* copy requested data from user-land */
		dev_dbg(&pdev->dev, "write, rtn EFAULT(3)");
		result = -EFAULT;
	} else {
		getrawmonotonic(&ts);
		hdl->req_data[temp[1]].state = UVCS_REQ_USED;
		hdl->req_data[temp[1]].time = ts.tv_nsec;

		/* request to UVCS-CMN */
		uvcs_ret = uvcs_cmn_request(hdl->local->uvcs_info, hdl->uvcs_hdl, &hdl->req_data[temp[1]].data);
		switch (uvcs_ret) {
		/* interrupted */
		case UVCS_RTN_CONTINUE:
			hdl->req_data[temp[1]].state = UVCS_REQ_NONE;
			result = -ERESTARTSYS;
			break;
		case UVCS_RTN_OK:
			result = count;
			break;
		/* unacceptable request */
		case UVCS_RTN_BUSY:
			dev_dbg(&pdev->dev, "write, rtn EBUSY(2)");
			hdl->req_data[temp[1]].state = UVCS_REQ_NONE;
			result = -EBUSY;
			break;
		default:
			dev_dbg(&pdev->dev, "write, rtn EFAULT(4)");
			hdl->req_data[temp[1]].state = UVCS_REQ_NONE;
			result = -EFAULT;
			break;
		}
	}

	up(&hdl->sem);
	return result;
}

/* ioctl (chrdev) */
static long uvcs_lkm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct uvcs_hdl_info *hdl = filp->private_data;
	struct platform_device *pdev;
	long result = 0;
	UVCS_RESULT uvcs_ret;
	ulong temp;
	int err;

	if ((hdl == NULL) || (hdl->local == NULL) || (hdl->local->pdev == NULL)) {
		pr_debug("ioctl, rtn EFAULT");
		return -EFAULT;
	}
	pdev = hdl->local->pdev;

	/* into exclusive section */
	if (down_interruptible(&hdl->sem))
		return -ERESTARTSYS;

	switch (cmd) {
	/* get the size of dump data */
	case UVCS_IOCTL_DUMP_GET_SIZE:
		err = put_user(hdl->local->uvcs_init_param.debug_log_size, (ulong __user *)arg);
		if (err) {
			/* unwritable */
			dev_dbg(&pdev->dev, "dumpsize, rtn EFAULT");
			result = -EFAULT;
		}
		break;

	case UVCS_IOCTL_DUMP_OUTPUT:
		if (hdl->local->uvcs_init_param.debug_log_buff != NULL) {
			if (copy_to_user((char __user *)arg, hdl->local->uvcs_init_param.debug_log_buff, hdl->local->uvcs_init_param.debug_log_size)) {
				dev_dbg(&pdev->dev, "dump, rtn EFAULT");
				result = -EFAULT;
			}
		}
		break;

	case UVCS_IOCTL_SET_PREEMPT_MODE:
		uvcs_ret = uvcs_cmn_set_preempt_mode(hdl->local->uvcs_info, hdl->uvcs_hdl, UVCS_TRUE, arg);
		switch (uvcs_ret) {
		case UVCS_RTN_BUSY:
			result = -EBUSY;
			dev_dbg(&pdev->dev, "set_pm, rtn EBUSY");
			break;
		case UVCS_RTN_OK:
			result = 0;
			break;
		default:
			dev_dbg(&pdev->dev, "set_pm, rtn EINVAL");
			result = -EINVAL;
			break;
		}
		break;

	case UVCS_IOCTL_CLR_PREEMPT_MODE:
		uvcs_ret = uvcs_cmn_set_preempt_mode(hdl->local->uvcs_info, hdl->uvcs_hdl, UVCS_FALSE, 0);
		if (uvcs_ret) {
			dev_dbg(&pdev->dev, "clr_pm, rtn EINVAL");
			result = -EINVAL;
		}
		break;

	case UVCS_IOCTL_GET_IP_INFO:
		err = get_user(temp, (ulong __user *)arg);
		if (err) {
			dev_dbg(&pdev->dev, "ipinf, rtn EFAULT(1)");
			result = -EFAULT;
		} else if (temp < sizeof(UVCS_CMN_IP_INFO_T)) {
			dev_dbg(&pdev->dev, "ipinf, rtn EINVAL");
			result = -EINVAL;
		} else if (copy_to_user((UVCS_CMN_IP_INFO_T __user *)arg, &hdl->local->ip_info, sizeof(UVCS_CMN_IP_INFO_T))) {
			dev_dbg(&pdev->dev, "ipinf, rtn EFAULT(2)");
			result = -EFAULT;
		}
		break;

	default:
		dev_dbg(&pdev->dev, "ioctl, rtn ENOTTY");
		result = -ENOTTY;
		break;
	}

	up(&hdl->sem);

	return result;
}

/* poll (chrdev) */
static unsigned int uvcs_lkm_poll(struct file *filp, poll_table *wait)
{
	struct uvcs_hdl_info *hdl = filp->private_data;
	unsigned int mask = 0;

	/* into exclusive section */
	down(&hdl->sem);
	poll_wait(filp, &hdl->waitq, wait);
	if ((hdl->req_data[0].state == UVCS_REQ_END) || (hdl->req_data[1].state == UVCS_REQ_END) || (hdl->req_data[2].state == UVCS_REQ_END))
		mask |= POLLIN | POLLRDNORM;

	up(&hdl->sem);

	return mask;
}


/******************************************************************************/
/*                    device probe / remove                                   */
/******************************************************************************/
const struct file_operations uvcs_lkm_fops = {
	.owner = THIS_MODULE,
	.read = uvcs_lkm_read,
	.write = uvcs_lkm_write,
	.unlocked_ioctl = uvcs_lkm_ioctl,
	.poll = uvcs_lkm_poll,
	.open = uvcs_lkm_open,
	.release = uvcs_lkm_release,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int uvcs_probe(struct platform_device *pdev)
#else
static int __devinit uvcs_probe(struct platform_device *pdev)
#endif
{
	struct uvcs_drv_info *local;
	int result = 0;
	dev_t dev = 0;
	struct device *ret_dev;
	UVCS_CMN_INIT_PARAM_T *init;

	local = kzalloc(sizeof(struct uvcs_drv_info), GFP_KERNEL);
	if (!local) {
		pr_debug("probe, rtn ENOMEM(1)");
		result = -ENOMEM;
		goto err_exit_0;
	}
	init = &local->uvcs_init_param;

	/* get workarea size for uvcs-cmn */
	uvcs_cmn_get_work_size(
			&local->uvcs_lib_work_req_size,
			&local->uvcs_hdl_work_req_size);

	/* allocate workarea */
	init->work_mem_0_size = local->uvcs_lib_work_req_size;
	init->work_mem_0_virt = kzalloc(local->uvcs_lib_work_req_size, GFP_KERNEL);
	if (!local->uvcs_init_param.work_mem_0_virt) {
		pr_debug("probe, rtn ENOMEM(2)");
		result = -ENOMEM;
		goto err_exit_1;
	}

	if (uvcs_debug) {
		/* alloc debug buffer */
		local->uvcs_init_param.debug_log_size = UVCS_DEBUG_BUFF_SIZE;
		local->uvcs_init_param.debug_log_buff = vzalloc(UVCS_DEBUG_BUFF_SIZE);
		if (!local->uvcs_init_param.debug_log_buff) {
			pr_debug("probe, rtn ENOMEM(3)");
			result = -ENOMEM;
			goto err_exit_2;
		}
	}

	sema_init(&local->sem, 1);

	result = alloc_chrdev_region(&dev, 0, DEVNUM, DRVNAME);
	if (result) {
		pr_debug("probe, alloc_chrdev_region");
		goto err_exit_3;
	}

	cdev_init(&local->cdev, &uvcs_lkm_fops);
	local->cdev.owner = THIS_MODULE;

	result = cdev_add(&local->cdev, dev, DEVNUM);
	if (result) {
		pr_debug("probe, cdev_add");
		goto err_exit_4;
	}

	local->pcls = class_create(THIS_MODULE, CLSNAME);
	if (IS_ERR(local->pcls)) {
		pr_debug("probe, class_create");
		result = PTR_ERR(local->pcls);
		goto err_exit_5;
	}

	ret_dev = device_create(local->pcls, NULL, dev, NULL, DEVNAME);
	if (IS_ERR(ret_dev)) {
		pr_debug("probe, device_create");
		result = PTR_ERR(ret_dev);
		goto err_exit_6;
	}

	/* save platform_device pointer, drvdata */
	local->pdev = pdev;
	drv_info = local;
	platform_set_drvdata(pdev, drv_info);

	return 0;

err_exit_6:
	class_destroy(local->pcls);

err_exit_5:
	cdev_del(&local->cdev);

err_exit_4:
	unregister_chrdev_region(dev, DEVNUM);

err_exit_3:
	if (local->uvcs_init_param.debug_log_buff)
		vfree(local->uvcs_init_param.debug_log_buff);

err_exit_2:
	kfree(local->uvcs_init_param.work_mem_0_virt);

err_exit_1:
	kfree(local);
	drv_info = NULL;

err_exit_0:
	return result;

}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int uvcs_remove(struct platform_device *pdev)
#else
static int __devexit uvcs_remove(struct platform_device *pdev)
#endif
{
	struct uvcs_drv_info *local = platform_get_drvdata(pdev);
	dev_t dev = local->cdev.dev;

	device_destroy(local->pcls, dev);
	class_destroy(local->pcls);
	cdev_del(&local->cdev);
	unregister_chrdev_region(dev, DEVNUM);

	if (local->uvcs_init_param.debug_log_buff)
		vfree(local->uvcs_init_param.debug_log_buff);
	kfree(local->uvcs_init_param.work_mem_0_virt);
	kfree(local);
	drv_info = NULL;
	platform_set_drvdata(pdev, NULL);

	return 0;
}


/* uvcs device release function */
static void uvcs_lkm_dev_release(struct device *dev)
{
	return;
}

/******************************************************************************/
/*                    module init / exit                                      */
/******************************************************************************/
static int uvcs_runtime_nop(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops uvcs_pm_ops = {
	.runtime_suspend = uvcs_runtime_nop ,
	.runtime_resume = uvcs_runtime_nop ,
	.suspend = uvcs_runtime_nop ,
	.resume = uvcs_runtime_nop ,
};

static struct platform_driver uvcs_driver = {
	.probe		= uvcs_probe,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	.remove		= uvcs_remove,
#else
	.remove		= __devexit_p(uvcs_remove),
#endif
	.driver		= {
		.name	= DEVNAME ,
		.owner	= THIS_MODULE ,
		.pm	= &uvcs_pm_ops ,
	},
};

/* device resources */
static struct resource uvcs_resources[] = {
	/* VLC0 */
	[0] = {
		.name	= DEVNAME "-vlc0" ,
		.start	= UVCS_REG_VLC0 ,
		.end	= UVCS_REG_VLC0 + UVCS_REG_SIZE_VLC - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	[1] = {
		.name	= DEVNAME "-vlc0" ,
		.start	= UVCS_INT_VLC0 ,
		.flags	= IORESOURCE_IRQ ,
	} ,
	/* CE0 */
	[2] = {
		.name	= DEVNAME "-ce0" ,
		.start	= UVCS_REG_CE0 ,
		.end	= UVCS_REG_CE0 + UVCS_REG_SIZE_CE - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	[3] = {
		.name	= DEVNAME "-ce0" ,
		.start	= UVCS_INT_CE0 ,
		.flags	= IORESOURCE_IRQ ,
	} ,
	/* VLC1 */
	[4] = {
		.name	= DEVNAME "-vlc1" ,
		.start	= UVCS_REG_VLC1 ,
		.end	= UVCS_REG_VLC1 + UVCS_REG_SIZE_VLC - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	[5] = {
		.name	= DEVNAME "-vlc1" ,
		.start	= UVCS_INT_VLC1 ,
		.flags	= IORESOURCE_IRQ ,
	} ,
	/* CE1 */
	[6] = {
		.name	= DEVNAME "-ce1" ,
		.start	= UVCS_REG_CE1 ,
		.end	= UVCS_REG_CE1 + UVCS_REG_SIZE_CE - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	[7] = {
		.name	= DEVNAME "-ce1" ,
		.start	= UVCS_INT_CE1 ,
		.flags	= IORESOURCE_IRQ ,
	} ,
	/* VPC0 */
	[8] = {
		.name	= DEVNAME "-vpc0" ,
		.start	= UVCS_REG_VPC0 ,
		.end	= UVCS_REG_VPC0 + UVCS_REG_SIZE_VPC - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	/* VPC1 */
	[9] = {
		.name	= DEVNAME "-vpc1" ,
		.start	= UVCS_REG_VPC1 ,
		.end	= UVCS_REG_VPC1 + UVCS_REG_SIZE_VPC - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	/* VPC0xy */
	[10] = {
		.name	= DEVNAME "-vpc0xy" ,
		.start	= UVCS_REG_VPC0XY ,
		.end	= UVCS_REG_VPC0XY + UVCS_REG_SIZE_SINGLE - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	/* VPC1xy */
	[11] = {
		.name	= DEVNAME "-vpc1xy" ,
		.start	= UVCS_REG_VPC1XY ,
		.end	= UVCS_REG_VPC1XY + UVCS_REG_SIZE_SINGLE - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
	/* PRR */
	[12] = {
		.name	= DEVNAME "-prr" ,
		.start	= UVCS_REG_PRR ,
		.end	= UVCS_REG_PRR + UVCS_REG_SIZE_SINGLE - 1 ,
		.flags	= IORESOURCE_MEM ,
	} ,
};

/* uvcs device */
static struct platform_device uvcs_device = {
	.name		= DEVNAME,
	.id		= -1,
	.resource	= uvcs_resources,
	.num_resources	= ARRAY_SIZE(uvcs_resources),
	.dev		= {
				.release = uvcs_lkm_dev_release,
			} ,
};


static int __init uvcs_module_init(void)
{
	int result = 0;

	/* add platform device */
	result = platform_device_register(&uvcs_device);
	if (result)
		goto err_exit_0;

	/* register driver for platform devices */
	result = platform_driver_register(&uvcs_driver);
	if (result)
		goto err_exit_1;

	return 0;

err_exit_1:
	platform_device_unregister(&uvcs_device);
err_exit_0:

	return result;
}

static void __exit uvcs_module_exit(void)
{
	platform_driver_unregister(&uvcs_driver);
	platform_device_unregister(&uvcs_device);
}

module_init(uvcs_module_init);
module_exit(uvcs_module_exit);
