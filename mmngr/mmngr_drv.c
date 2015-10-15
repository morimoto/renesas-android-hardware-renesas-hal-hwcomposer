/*************************************************************************/ /*
 MMNGR

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
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/bitmap.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <linux/dma-attrs.h>
#include <linux/dma-contiguous.h>

#include "mmngr_public.h"
#include "mmngr_private.h"

static spinlock_t		lock;
static struct BM		bm;
static struct BM		bm_h;
static struct BM		bm_ssp;
static struct BM		bm_mv;
static struct MM_DRVDATA	*mm_drvdata;
static void			*top;
static void			*top_impctr;
static void			*top_impmba0;
static void			*top_impmba1;
static void			*top_impmbd0;
static void			*top_impmbd1;
static void			*top_imuctr19;
static void			*top_imuctr22;
static void			*top_prr;

static int mm_alloc_kh_in_kernel(struct device *mm_dev,
				unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *pkernel_virt_addr)
{
	unsigned long	phy_addr;

	*pkernel_virt_addr = (unsigned long)dma_alloc_coherent(mm_dev, size,
							(dma_addr_t *)&phy_addr,
							GFP_KERNEL);
	if (*pkernel_virt_addr == 0)
		return -1;

	*phard_addr = phy_addr;
	*pphy_addr = phy_addr >> 12;

	return 0;
}

static void mm_free_kh_in_kernel(struct device *mm_dev,
				unsigned long size,
				unsigned long phy_addr,
				unsigned long hard_addr,
				unsigned long kernel_virt_addr)
{
	dma_free_coherent(mm_dev, size, (void *)kernel_virt_addr,
			(dma_addr_t)hard_addr);

}

static int mm_alloc_co_in_kernel(struct BM *pb,
				unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *pkernel_virt_addr)
{
	unsigned long	nbits;
	unsigned long	start_bit;
	unsigned long	phy_addr;

	nbits = (size + (1UL << pb->order) - 1) >> pb->order;

	spin_lock(&lock);
	start_bit = bitmap_find_next_zero_area(pb->bits, pb->end_bit, 0,
						nbits, 0);
	if (start_bit >= pb->end_bit) {
		printk(KERN_ERR "start(%ld), end(%ld)\n", start_bit,
			pb->end_bit);
		spin_unlock(&lock);
		*pphy_addr = 0;
		*phard_addr = 0;
		*pkernel_virt_addr = 0;
		return -ENOMEM;
	}
	bitmap_set(pb->bits, start_bit, nbits);
	spin_unlock(&lock);

	phy_addr = pb->top_phy_addr + (start_bit << pb->order);
	*pphy_addr = phy_addr >> 12;
	*phard_addr = phy_addr;
	*pkernel_virt_addr = 0;

	return 0;
}

static void mm_free_co_in_kernel(struct BM *pb,
				unsigned long size,
				unsigned long phy_addr,
				unsigned long hard_addr,
				unsigned long kernel_virt_addr)
{
	unsigned long	nbits;
	unsigned long	start_bit;

	start_bit = (hard_addr - pb->top_phy_addr) >> pb->order;
	nbits = (size + (1UL << pb->order) - 1) >> pb->order;

	spin_lock(&lock);
	bitmap_clear(pb->bits, start_bit, nbits);
	spin_unlock(&lock);
}

int mmngr_alloc_in_kernel(unsigned long size,
			unsigned long *pphy_addr,
			unsigned long *phard_addr,
			unsigned long *pkernel_virt_addr,
			unsigned long flag)
{
	int		ret = 0;
	struct device	*mm_dev;
	struct BM	*pb;

	if (size == 0)
		return -1;

	if ((pphy_addr == NULL) || (phard_addr == NULL)
	|| (pkernel_virt_addr == NULL))
		return -1;

	if ((flag != MM_KERNELHEAP) && (flag != MM_CARVEOUT)
	&& (flag != MM_CARVEOUT_H))
		return -1;

	if (flag == MM_KERNELHEAP) {
		mm_dev = mm_drvdata->mm_dev;
		ret = mm_alloc_kh_in_kernel(mm_dev, size, pphy_addr, phard_addr,
					pkernel_virt_addr);
	} else if (flag == MM_CARVEOUT) {
		pb = &bm;
		ret = mm_alloc_co_in_kernel(pb, size, pphy_addr, phard_addr,
					pkernel_virt_addr);
	} else if (flag == MM_CARVEOUT_H) {
		pb = &bm_h;
		ret = mm_alloc_co_in_kernel(pb, size, pphy_addr, phard_addr,
					pkernel_virt_addr);
	}

	return ret;
}
EXPORT_SYMBOL(mmngr_alloc_in_kernel);

void mmngr_free_in_kernel(unsigned long size,
			unsigned long phy_addr,
			unsigned long hard_addr,
			unsigned long kernel_virt_addr,
			unsigned long flag)
{
	struct device	*mm_dev;
	struct BM	*pb;

	if (size == 0)
		return;

	if (hard_addr == 0)
		return;

	if ((flag == MM_CARVEOUT) && (phy_addr == 0))
		return;

	if ((flag == MM_CARVEOUT_H) && (phy_addr == 0))
		return;

	if ((flag == MM_KERNELHEAP) && (kernel_virt_addr == 0))
		return;

	if ((flag != MM_KERNELHEAP) && (flag != MM_CARVEOUT)
	&& (flag != MM_CARVEOUT_H))
		return;

	if (flag == MM_KERNELHEAP) {
		mm_dev = mm_drvdata->mm_dev;
		mm_free_kh_in_kernel(mm_dev, size, phy_addr, hard_addr,
				kernel_virt_addr);
	} else if (flag == MM_CARVEOUT) {
		pb = &bm;
		mm_free_co_in_kernel(pb, size, phy_addr, hard_addr,
				kernel_virt_addr);
	} else if (flag == MM_CARVEOUT_H) {
		pb = &bm_h;
		mm_free_co_in_kernel(pb, size, phy_addr, hard_addr,
				kernel_virt_addr);
	}
}
EXPORT_SYMBOL(mmngr_free_in_kernel);

static int mm_ioc_alloc(struct device *mm_dev,
			int __user *in,
			struct MM_PARAM *out)
{
	int		ret = 0;
	struct MM_PARAM	tmp;

	if (copy_from_user(&tmp, (void __user *)in, sizeof(struct MM_PARAM))) {
		printk(KERN_ERR "%s EFAULT\n", __func__);
		ret = -EFAULT;
		return ret;
	}
	out->size = tmp.size;
	out->kernel_virt_addr = (unsigned long)dma_alloc_coherent(mm_dev,
						out->size,
						(dma_addr_t *)&out->phy_addr,
						GFP_KERNEL);
	if (!out->kernel_virt_addr) {
		printk(KERN_ERR "%s ENOMEM\n", __func__);
		ret = -ENOMEM;
		out->phy_addr = 0;
		out->hard_addr = 0;
		return ret;
	}
	out->hard_addr = out->phy_addr;
	out->flag = tmp.flag;

	return ret;
}

static void mm_ioc_free(struct device *mm_dev, struct MM_PARAM *p)
{
	dma_free_coherent(mm_dev, p->size, (void *)p->kernel_virt_addr,
			(dma_addr_t)p->phy_addr);
	memset(p, 0, sizeof(struct MM_PARAM));
}

static int mm_ioc_set(int __user *in, struct MM_PARAM *out)
{
	int		ret = 0;
	struct MM_PARAM	tmp;

	if (copy_from_user(&tmp, in, sizeof(struct MM_PARAM))) {
		ret = -EFAULT;
		return ret;
	}

	out->user_virt_addr = tmp.user_virt_addr;

	return ret;
}

static int mm_ioc_get(struct MM_PARAM *in, int __user *out)
{
	int	ret = 0;

	if (copy_to_user(out, in, sizeof(struct MM_PARAM))) {
		ret = -EFAULT;
		return ret;
	}

	return ret;
}

static int alloc_bm(struct BM *pb,
		unsigned long top_phy_addr,
		unsigned long size,
		unsigned long order)
{
	unsigned long	nbits;
	unsigned long	nbytes;

	printk(KERN_ERR "MMNGR: CO from 0x%lx to 0x%lx\n",
	top_phy_addr, top_phy_addr + size - 1);

	if (pb == NULL)
		return -1;

	nbits = (size + ((1UL << order) - 1)) >> order;
	nbytes = (nbits + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
	pb->bits = kzalloc(nbytes, GFP_KERNEL);
	if (pb->bits == NULL)
		return -1;
	pb->order = order;
	pb->top_phy_addr = top_phy_addr;
	pb->end_bit = nbits;

	return 0;
}

static void free_bm(struct BM *pb)
{
	kfree(pb->bits);
}

static int mm_ioc_alloc_co(struct BM *pb, int __user *in, struct MM_PARAM *out)
{
	int		ret = 0;
	unsigned long	nbits;
	unsigned long	start_bit;
	struct MM_PARAM	tmp;

	if (copy_from_user(&tmp, in, sizeof(struct MM_PARAM))) {
		printk(KERN_ERR "%s EFAULT\n", __func__);
		ret = -EFAULT;
		return ret;
	}

	out->size = tmp.size;
	nbits = (out->size + (1UL << pb->order) - 1) >> pb->order;

	spin_lock(&lock);
	start_bit = bitmap_find_next_zero_area(pb->bits, pb->end_bit, 0,
						nbits, 0);
	if (start_bit >= pb->end_bit) {
		printk(KERN_ERR "start(%ld), end(%ld)\n", start_bit,
			pb->end_bit);
		spin_unlock(&lock);
		out->phy_addr = 0;
		out->hard_addr = 0;
		return -ENOMEM;
	}
	bitmap_set(pb->bits, start_bit, nbits);
	spin_unlock(&lock);

	out->phy_addr = pb->top_phy_addr + (start_bit << pb->order);
	out->hard_addr = out->phy_addr;
	out->flag = tmp.flag;

	return 0;
}

static int mm_ioc_alloc_co_select(int __user *in, struct MM_PARAM *out)
{
	int		ret = 0;
	struct MM_PARAM	tmp;
	struct device	*mm_dev;

	mm_dev = mm_drvdata->mm_dev;

	if (copy_from_user(&tmp, in, sizeof(struct MM_PARAM))) {
		printk(KERN_ERR "%s EFAULT\n", __func__);
		ret = -EFAULT;
		return ret;
	}

	if (tmp.flag == MM_CARVEOUT)
		ret = mm_ioc_alloc_co(&bm, in, out);
#ifndef MMNGR_SSP_ENABLE
	else if (tmp.flag == MM_CARVEOUT_SSP) {
		printk(KERN_ERR "%s EINVAL\n", __func__);
		ret = -EINVAL;
	}
#else
	else if (tmp.flag == MM_CARVEOUT_SSP)
		ret = mm_ioc_alloc_co(&bm_ssp, in, out);
#endif
#ifndef MMNGR_MV_ENABLE
	else if (tmp.flag == MM_CARVEOUT_MV)
		ret = mm_ioc_alloc(mm_dev, (int __user *)in, out);
#else
	else if (tmp.flag == MM_CARVEOUT_MV)
		ret = mm_ioc_alloc_co(&bm_mv, in, out);
#endif

	return ret;
}

static void mm_ioc_free_co(struct BM *pb, struct MM_PARAM *p)
{
	unsigned long	nbits;
	unsigned long	start_bit;

	start_bit = (p->phy_addr - pb->top_phy_addr) >> pb->order;
	nbits = (p->size + (1UL << pb->order) - 1) >> pb->order;

	spin_lock(&lock);
	bitmap_clear(pb->bits, start_bit, nbits);
	spin_unlock(&lock);
	memset(p, 0, sizeof(struct MM_PARAM));
}

static void mm_ioc_free_co_select(struct MM_PARAM *p)
{
	struct device	*mm_dev;

	mm_dev = mm_drvdata->mm_dev;

	if (p->flag == MM_CARVEOUT)
		mm_ioc_free_co(&bm, p);
	else if (p->flag == MM_CARVEOUT_SSP)
		mm_ioc_free_co(&bm_ssp, p);
#ifndef MMNGR_MV_ENABLE
	else if (p->flag == MM_CARVEOUT_MV)
		mm_ioc_free(mm_dev, p);
#else
	else if (p->flag == MM_CARVEOUT_MV)
		mm_ioc_free_co(&bm_mv, p);
#endif
}

static int mm_ioc_share(int __user *in, struct MM_PARAM *out)
{
	int		ret = 0;
	struct MM_PARAM	tmp;

	if (copy_from_user(&tmp, in, sizeof(struct MM_PARAM))) {
		ret = -EFAULT;
		return ret;
	}

	out->phy_addr = tmp.phy_addr;
	out->size = tmp.size;

	return ret;
}

static int map_register(void)
{
	if (!request_mem_region(MM_MXI_ADDR, MM_MXI_SIZE, DRVNAME))
		return -1;

	top = ioremap_nocache(MM_MXI_ADDR, MM_MXI_SIZE);
	if (top == NULL) {
		release_mem_region(MM_MXI_ADDR, MM_MXI_SIZE);
		return -1;
	}

	top_prr = ioremap_nocache(MM_PRR_ADDR, MM_PRR_SIZE);
	if (top_prr == NULL)
		return -1;

	top_impctr = ioremap_nocache(MM_IMPCTR_ADDR, MM_IPMMU_SIZE);
	if (top_impctr == NULL)
		return -1;

	top_impmba0 = ioremap_nocache(MM_IMPMBA0_ADDR, MM_IPMMU_SIZE);
	if (top_impmba0 == NULL)
		return -1;

	top_impmba1 = ioremap_nocache(MM_IMPMBA1_ADDR, MM_IPMMU_SIZE);
	if (top_impmba1 == NULL)
		return -1;

	top_impmbd0 = ioremap_nocache(MM_IMPMBD0_ADDR, MM_IPMMU_SIZE);
	if (top_impmbd0 == NULL)
		return -1;

	top_impmbd1 = ioremap_nocache(MM_IMPMBD1_ADDR, MM_IPMMU_SIZE);
	if (top_impmbd1 == NULL)
		return -1;

	top_imuctr19 = ioremap_nocache(MM_IMUCTR19_ADDR, MM_IPMMU_SIZE);
	if (top_imuctr19 == NULL)
		return -1;
#ifdef MMNGR_LAGER
	top_imuctr22 = ioremap_nocache(MM_IMUCTR22_ADDR, MM_IPMMU_SIZE);
	if (top_imuctr22 == NULL)
		return -1;
#endif

	return 0;
}

static void unmap_register(void)
{
	if (top != NULL) {
		iounmap(top);
		release_mem_region(MM_MXI_ADDR, MM_MXI_SIZE);
		top = NULL;
	}

	if (top_impctr != NULL) {
		iounmap(top_impctr);
		top_impctr = NULL;
	}

	if (top_impmba0 != NULL) {
		iounmap(top_impmba0);
		top_impmba0 = NULL;
	}

	if (top_impmba1 != NULL) {
		iounmap(top_impmba1);
		top_impmba1 = NULL;
	}

	if (top_impmbd0 != NULL) {
		iounmap(top_impmbd0);
		top_impmbd0 = NULL;
	}

	if (top_impmbd1 != NULL) {
		iounmap(top_impmbd1);
		top_impmbd1 = NULL;
	}

	if (top_imuctr19 != NULL) {
		iounmap(top_imuctr19);
		top_imuctr19 = NULL;
	}
#ifdef MMNGR_LAGER
	if (top_imuctr22 != NULL) {
		iounmap(top_imuctr22);
		top_imuctr22 = NULL;
	}
#endif

	if (top_prr != NULL) {
		iounmap(top_prr);
		top_prr = NULL;
	}

	return;
}

static void mm_set_mxi_path(unsigned long start, unsigned long end)
{
	unsigned long sadd0;
	unsigned long eadd0;
	unsigned long mxsaar0;

	sadd0 = (start >> 24) << 16;
	eadd0 = (end >> 24);
	mxsaar0 = sadd0 | eadd0;
	iowrite32(mxsaar0, top);

	printk(KERN_ERR "MMNGR: MXSAAR0(0x%08x)\n", ioread32(top));

	return;
}

static void mm_enable_pmb(void)
{
	iowrite32(MM_IMPCTR_VAL | ioread32(top_impctr), top_impctr);
	return;
}

static void mm_enable_vpc_utlb(void)
{
	iowrite32(MM_IMUCTR_VAL, top_imuctr19);
#ifdef MMNGR_LAGER
	iowrite32(MM_IMUCTR_VAL, top_imuctr22);
#endif
	return;
}

static void mm_set_pmb_area(unsigned long start, void *impmba, void *impmbd)
{
	unsigned long	vpn;
	unsigned long	ppn;

	vpn = start & MM_ADDR_MASK;
	ppn = (start - MM_LEGACY_ADDR) & MM_ADDR_MASK;

	iowrite32(MM_IMPMBA_VAL | vpn, impmba);
	iowrite32(MM_IMPMBD_VAL | ppn, impmbd);

	return;
}

static void mmngr_dev_set_cma_area(struct device *dev, struct cma *cma)
{
	if (dev)
		dev->cma_area = cma;
}

static int open(struct inode *inode, struct file *file)
{
	struct MM_PARAM	*p;

	p = kzalloc(sizeof(struct MM_PARAM), GFP_KERNEL);
	if (!p)
		return -1;

	file->private_data = p;

	return 0;
}

static int close(struct inode *inode, struct file *file)
{
	struct MM_PARAM	*p = file->private_data;
	struct BM	*pb;
	struct device	*mm_dev;

	if (p) {
		if ((p->flag == MM_KERNELHEAP)
		&& (p->kernel_virt_addr != 0)) {
			printk(KERN_ERR "MMD close kernelheap\n");
			mm_dev = mm_drvdata->mm_dev;
			dma_free_coherent(mm_dev, p->size,
					(void *)p->kernel_virt_addr,
					(dma_addr_t)p->phy_addr);
		} else if ((p->flag == MM_CARVEOUT)
		&& (p->phy_addr != 0)) {
			printk(KERN_ERR "MMD close carveout\n");
			pb = &bm;
			mm_ioc_free_co(pb, p);
		} else if ((p->flag == MM_CARVEOUT_SSP)
		&& (p->phy_addr != 0)) {
#ifdef MMNGR_SSP_ENABLE
			printk(KERN_ERR "MMD close carveout SSP\n");
			pb = &bm_ssp;
			mm_ioc_free_co(pb, p);
#endif
		} else if (p->flag == MM_CARVEOUT_MV) {
#ifdef MMNGR_MV_ENABLE
			if (p->phy_addr != 0) {
				printk(KERN_ERR "MMD close carveout MV\n");
				pb = &bm_mv;
				mm_ioc_free_co(pb, p);
			}
#else
			if (p->kernel_virt_addr != 0) {
				printk(KERN_ERR "MMD close kernelheap\n");
				mm_dev = mm_drvdata->mm_dev;
				dma_free_coherent(mm_dev, p->size,
						(void *)p->kernel_virt_addr,
						(dma_addr_t)p->phy_addr);
			}
#endif
		}

		kfree(p);
		file->private_data = NULL;
	}

	return 0;
}

static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int		ercd;
	int		ret;
	struct MM_PARAM	*p = file->private_data;
	struct device	*mm_dev;

	mm_dev = mm_drvdata->mm_dev;

	switch (cmd) {
	case MM_IOC_ALLOC:
		ercd = mm_ioc_alloc(mm_dev, (int __user *)arg, p);
		if (ercd) {
			printk(KERN_ERR "MMD ALLOC ENOMEM\n");
			ret = ercd;
			goto exit;
		}
		break;
	case MM_IOC_FREE:
		mm_ioc_free(mm_dev, p);
		break;
	case MM_IOC_SET:
		ercd = mm_ioc_set((int __user *)arg, p);
		if (ercd) {
			printk(KERN_ERR "MMD SET EFAULT\n");
			ret = ercd;
			goto exit;
		}
		break;
	case MM_IOC_GET:
		ercd = mm_ioc_get(p, (int __user *)arg);
		if (ercd) {
			printk(KERN_ERR "MMD GET EFAULT\n");
			ret = ercd;
			goto exit;
		}
		break;
	case MM_IOC_ALLOC_CO:
		ercd = mm_ioc_alloc_co_select((int __user *)arg, p);
		if (ercd) {
			printk(KERN_ERR "MMD C ALLOC ENOMEM\n");
			ret = ercd;
			goto exit;
		}
		break;
	case MM_IOC_FREE_CO:
		mm_ioc_free_co_select(p);
		break;
	case MM_IOC_SHARE:
		ercd = mm_ioc_share((int __user *)arg, p);
		if (ercd) {
			printk(KERN_ERR "MMD C SHARE EFAULT\n");
			ret = ercd;
			goto exit;
		}
		break;
	default:
		printk(KERN_ERR "MMD CMD EFAULT\n");
		ret = -EFAULT;
		goto exit;
	}

	return 0;

exit:
	return ret;
}

static int mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long	start;
	unsigned long	off;
	unsigned long	len;
	struct MM_PARAM	*p = filp->private_data;

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT))
		return -EINVAL;

	off = vma->vm_pgoff << PAGE_SHIFT;
	start = p->phy_addr;
	len = PAGE_ALIGN((start & ~PAGE_MASK) + p->size);

	if ((vma->vm_end - vma->vm_start + off) > len)
		return -EINVAL;

	off += start;
	vma->vm_pgoff = off >> PAGE_SHIFT;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	vma->vm_flags |= (VM_IO | VM_DONTEXPAND | VM_DONTDUMP);
#else
	vma->vm_flags |= (VM_IO | VM_RESERVED);
#endif

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static const struct file_operations fops = {
	.owner		= THIS_MODULE,
	.open		= open,
	.release	= close,
	.unlocked_ioctl	= ioctl,
	.mmap		= mmap,
};

static struct miscdevice misc = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DEVNAME,
	.fops		= &fops,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
extern struct cma *rcar_gen2_dma_contiguous;
#endif

static int mm_init(void)
{
	int			ret = 0;
	struct MM_DRVDATA	*p = NULL;
	dma_addr_t		phy_addr;
	void			*pkernel_virt_addr;

	ret = alloc_bm(&bm, MM_OMXBUF_ADDR, MM_OMXBUF_SIZE, MM_CO_ORDER);
	if (ret) {
		printk(KERN_ERR "MMD mm_init ERROR");
		return -1;
	}

#ifdef MMNGR_IPC
	ret = alloc_bm(&bm_h, MM_IPCBUF_ADDR, MM_IPCBUF_SIZE, MM_CO_ORDER);
	if (ret) {
		printk(KERN_ERR "MMD mm_init ERROR");
		return -1;
	}
#endif
#ifdef MMNGR_SSP_ENABLE
	ret = alloc_bm(&bm_ssp, MM_SSPBUF_ADDR, MM_SSPBUF_SIZE, MM_CO_ORDER);
	if (ret) {
		printk(KERN_ERR "MMD mm_init ERROR");
		return -1;
	}
#endif
#ifdef MMNGR_MV_ENABLE
	ret = alloc_bm(&bm_mv, MM_MVBUF_ADDR, MM_MVBUF_SIZE, MM_CO_ORDER);
	if (ret) {
		printk(KERN_ERR "MMD mm_init ERROR");
		return -1;
	}
#endif

	ret = map_register();
	if (ret != 0) {
		printk(KERN_ERR "MMD map_register() NG\n");
		return -1;
	}

	mm_set_mxi_path(MM_OMXBUF_ADDR, MM_OMXBUF_ADDR + MM_OMXBUF_SIZE);

#ifdef MMNGR_KOELSCH
	if ((MM_PRR_ESMASK & ioread32(top_prr)) >= MM_PRR_ES2) {
		mm_enable_pmb();
		mm_set_pmb_area(MM_OMXBUF_ADDR, top_impmba0, top_impmbd0);
		mm_enable_vpc_utlb();
	}
#endif
#ifdef MMNGR_LAGER
	if ((MM_PRR_ESMASK & ioread32(top_prr)) >= MM_PRR_ES2) {
		mm_enable_pmb();
		mm_set_pmb_area(MM_OMXBUF_ADDR, top_impmba0, top_impmbd0);
		mm_set_pmb_area(MM_OMXBUF_ADDR + MM_PMB_SIZE_128M, top_impmba1,
				top_impmbd1);
		mm_enable_vpc_utlb();
	}
#endif
#ifdef MMNGR_ALT
	mm_enable_pmb();
	mm_set_pmb_area(MM_OMXBUF_ADDR, top_impmba0, top_impmbd0);
	mm_enable_vpc_utlb();
#endif

	p = kzalloc(sizeof(struct MM_DRVDATA), GFP_KERNEL);
	if (!p) {
		printk(KERN_ERR "MMD mm_init ERROR");
		return -1;
	}
	misc_register(&misc);
	p->mm_dev = NULL;
	p->mm_dev_reserve = misc.this_device;
	p->mm_dev_reserve->coherent_dma_mask = ~0;
	mm_drvdata = p;

	spin_lock_init(&lock);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	if (rcar_gen2_dma_contiguous == NULL) {
		printk(KERN_ERR "MMD mm_init ERROR");
		return -1;
	}
	mmngr_dev_set_cma_area(p->mm_dev_reserve, rcar_gen2_dma_contiguous);
	pkernel_virt_addr = dma_alloc_coherent(p->mm_dev_reserve,
					MM_KERNEL_RESERVE_SIZE,
					(dma_addr_t *)&phy_addr,
					GFP_KERNEL);
	mm_drvdata->reserve_size = MM_KERNEL_RESERVE_SIZE;
	mm_drvdata->reserve_kernel_virt_addr = (unsigned long)pkernel_virt_addr;
	mm_drvdata->reserve_phy_addr = (unsigned long)phy_addr;
	printk(KERN_ERR "MMD reserve area from 0x%08x to 0x%08x at virtual\n",
		(unsigned int)pkernel_virt_addr,
		(unsigned int)pkernel_virt_addr + MM_KERNEL_RESERVE_SIZE - 1);
	printk(KERN_ERR "MMD reserve area from 0x%08x to 0x%08x at physical\n",
		(unsigned int)phy_addr,
		(unsigned int)phy_addr + MM_KERNEL_RESERVE_SIZE - 1);
#endif

	return 0;
}

static void mm_exit(void)
{
	misc_deregister(&misc);

	unmap_register();

#ifdef MMNGR_MV_ENABLE
	free_bm(&bm_mv);
#endif
#ifdef MMNGR_SSP_ENABLE
	free_bm(&bm_ssp);
#endif
#ifdef MMNGR_IPC
	free_bm(&bm_h);
#endif
	free_bm(&bm);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	dma_free_coherent(mm_drvdata->mm_dev_reserve,
			mm_drvdata->reserve_size,
			(void *)mm_drvdata->reserve_kernel_virt_addr,
			(dma_addr_t)mm_drvdata->reserve_phy_addr);
#endif

	kfree(mm_drvdata);
}

module_init(mm_init);
module_exit(mm_exit);

MODULE_LICENSE("Dual MIT/GPL");
