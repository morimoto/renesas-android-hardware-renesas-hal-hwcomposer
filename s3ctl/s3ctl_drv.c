/*************************************************************************/ /*
 S3CTL

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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include "s3ctl_private.h"

static int		rg_st[S3_MAX];
static unsigned long	top;
static unsigned long	top_prr;
static struct S3_XYTL	*xytl;
static spinlock_t	lock;

static inline void S3_WRITE(unsigned long *reg, u32 value)
{
	writel(value, reg);
}

static inline u32 S3_READ(unsigned long *reg)
{
	return readl(reg);
}

static int map_register(void)
{
	if (!request_mem_region(S3_ADDR, S3_SIZE, DRVNAME))
		return -1;

	top = (unsigned long)ioremap_nocache(S3_ADDR, S3_SIZE);
	if (!top) {
		release_mem_region(S3_ADDR, S3_SIZE);
		return -1;
	}

	xytl = (struct S3_XYTL *)(top + S3_OFFSET);

	top_prr = (unsigned long)ioremap_nocache(S3_PRR_ADDR, S3_PRR_SIZE);
	if (!top_prr)
		return -1;

	return 0;
}

static void unmap_register(void)
{
	if (top != 0) {
		iounmap((void *)top);
		release_mem_region(S3_ADDR, S3_SIZE);
		top = 0;
		xytl = NULL;
	}

	if (top_prr != 0) {
		iounmap((void *)top_prr);
		top_prr = 0;
	}

	return;
}

static void set_xymodeconf(u32 value)
{
	S3_WRITE(&(xytl->mode), value);
	printk(KERN_ERR "S3D: XYMODECONF(0x%08x)\n",
		ioread32((void *)&(xytl->mode)));

	return;
}

static void enable_s3(int id, struct S3_PARAM *p)
{
	unsigned long	a = 0;
	unsigned long	b = 0;

	a |= S3_TL_A_EN;
	a |= p->phy_addr;
	b |= (p->area << S3_TL_B_SIZE_SHIFT);
	b |= (p->stride >> 2);

	S3_WRITE(&(xytl->a[id]), 0);
	S3_WRITE(&(xytl->a[id]), a);
	S3_WRITE(&(xytl->b[id]), b);

	return;
}

static void disable_s3(int id)
{
	S3_WRITE(&(xytl->a[id]), 0);
	S3_WRITE(&(xytl->b[id]), 0);

	return;
}

static int alloc_id(int *id)
{
	int	i;

	spin_lock(&lock);
	for (i = 0; i < S3_MAX; i++) {
		if (rg_st[i] == S3_FREE) {
			rg_st[i] = S3_ALLOC;
			break;
		}
	}
	spin_unlock(&lock);
	if (i == S3_MAX)
		return -1;
	else
		*id = i;

	return 0;
}

static void free_id(int *id)
{
	spin_lock(&lock);
	rg_st[*id] = S3_FREE;
	spin_unlock(&lock);

	*id = -1;

	return;
}

static int check_param(struct S3_PARAM *p)
{
	unsigned long	stride = 128;

	if (p->area > 10)
		goto exit;

	if ((p->stride < 128) || (p->stride > 4096)
	|| ((p->stride % stride) != 0))
		goto exit;

	if ((p->phy_addr == 0)
	|| ((p->phy_addr & (~S3_TL_A_ADDR_MASK)) != 0)
	|| ((p->phy_addr % (p->stride * 32 / 4096)) != 0))
		goto exit;

	return 0;
exit:
	return -1;
}

static int open(struct inode *inode, struct file *file)
{
	struct S3_PRIVATE	*p;

	p = kzalloc(sizeof(struct S3_PRIVATE), GFP_KERNEL);
	p->id = -1;
	p->st = S3_ST_OPEN;
	file->private_data = p;

	return 0;
}

static int close(struct inode *inode, struct file *file)
{
	struct S3_PRIVATE	*p = file->private_data;

	if (p) {
		if (p->id != -1) {
			disable_s3(p->id);
			free_id(&p->id);
			printk(KERN_ERR "S3D close\n");
		}
		kfree(p);
		file->private_data = NULL;
	}

	return 0;
}

static long ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	struct S3_PRIVATE	*p = file->private_data;
	int			ret;

	switch (cmd) {
	case S3_IOC_SET_PARAM:
		if (p->st != S3_ST_OPEN) {
			printk(KERN_ERR "S3D SET EPERM\n");
			ret = -EPERM;
			goto exit;
		}
		if (copy_from_user(&p->param, (int __user *)arg,
				sizeof(struct S3_PARAM))) {
			printk(KERN_ERR "S3D SET EFAULT\n");
			ret = -EFAULT;
			goto exit;
		}
		ret = check_param(&p->param);
		if (ret != 0) {
			printk(KERN_ERR "S3D SET EINVAL\n");
			ret = -EINVAL;
			memset(&p->param, 0, sizeof(struct S3_PARAM));
			goto exit;
		}
		p->st = S3_ST_SET;
		break;
	case S3_IOC_CLEAR_PARAM:
		if (p->st != S3_ST_SET) {
			printk(KERN_ERR "S3D CLEAR EPERM\n");
			ret = -EPERM;
			goto exit;
		}
		memset(&p->param, 0, sizeof(struct S3_PARAM));
		p->st = S3_ST_OPEN;
		break;
	case S3_IOC_LOCK:
		if (p->st != S3_ST_SET) {
			printk(KERN_ERR "S3D LOCK EPERM\n");
			ret = -EPERM;
			goto exit;
		}
		ret = alloc_id(&p->id);
		if (ret != 0) {
			printk(KERN_ERR "S3D LOCK EBUSY\n");
			ret = -EBUSY;
			goto exit;
		}
		enable_s3(p->id, &p->param);
		p->st = S3_ST_LOCK;
		break;
	case S3_IOC_UNLOCK:
		if (p->st != S3_ST_LOCK) {
			printk(KERN_ERR "S3D UNLOCK EPERM\n");
			ret = -EPERM;
			goto exit;
		}
		disable_s3(p->id);
		free_id(&p->id);
		p->st = S3_ST_SET;
		break;
	case S3_IOC_GET_PARAM:
		if (copy_to_user((int __user *)arg, &p->param,
				sizeof(struct S3_PARAM))) {
			printk(KERN_ERR "S3D GET_PARAM EFAULT\n");
			ret = -EFAULT;
			goto exit;
		}
		break;
	default:
		printk(KERN_ERR "S3D CMD EFAULT\n");
		ret = -EFAULT;
		goto exit;
	}

	return 0;

exit:
	return ret;
}

static const struct file_operations fops = {
	.owner		= THIS_MODULE,
	.open		= open,
	.release	= close,
	.unlocked_ioctl	= ioctl,
};

static struct miscdevice misc = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRVNAME,
	.fops		= &fops,
};


static int s3ctrl_init(void)
{
	int ret;
	unsigned int product;
	unsigned int es;

	ret = map_register();
	if (ret != 0) {
		printk(KERN_ERR "S3D map_register() NG\n");
		return -1;
	}

	product = S3_PRR_PRODUCTMASK & ioread32((void *)top_prr);
	es = S3_PRR_ESMASK & ioread32((void *)top_prr);
	if (product == S3_PRR_H2) {
		if (es == S3_PRR_ES2)
			set_xymodeconf(S3_XYMODE_VAL);
		else if (es >= S3_PRR_ES3)
			set_xymodeconf(S3_XYMODE_VAL_NEW);
	} else if (product == S3_PRR_M2W) {
		if (es == S3_PRR_ES2)
			set_xymodeconf(S3_XYMODE_VAL);
		else if (es >= S3_PRR_ES3)
			set_xymodeconf(S3_XYMODE_VAL_NEW);
	} else if (product == S3_PRR_M2N)
			set_xymodeconf(S3_XYMODE_VAL_NEW);
	else if (product == S3_PRR_E2) {
		if (es == S3_PRR_ES1)
			set_xymodeconf(S3_XYMODE_VAL);
		else
			set_xymodeconf(S3_XYMODE_VAL_NEW);
	} else
		set_xymodeconf(S3_XYMODE_VAL_NEW);

	misc_register(&misc);

	spin_lock_init(&lock);

	return 0;
}

static void s3ctrl_exit(void)
{
	misc_deregister(&misc);

	unmap_register();
}

module_init(s3ctrl_init);
module_exit(s3ctrl_exit);

MODULE_LICENSE("Dual MIT/GPL");
