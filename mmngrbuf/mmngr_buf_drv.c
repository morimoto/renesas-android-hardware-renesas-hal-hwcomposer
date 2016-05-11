/*************************************************************************/ /*
 MMNGR

 Copyright (C) 2014 Renesas Electronics Corporation

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
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/miscdevice.h>
#include <linux/dma-buf.h>
#include <linux/version.h>

#include "mmngr_buf_private.h"

static int open(struct inode *inode, struct file *file)
{
	struct MM_BUF_PRIVATE *priv;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	file->private_data = priv;

	return 0;
}

static int close(struct inode *inode, struct file *file)
{
	struct MM_BUF_PRIVATE *priv;

	priv = file->private_data;

	if (priv->sgt) {
		printk(KERN_ERR "unmap@close\n");
		dma_buf_unmap_attachment(priv->attach,
			priv->sgt, DMA_BIDIRECTIONAL);
	}

	if (priv->attach) {
		printk(KERN_ERR "detach@close\n");
		dma_buf_detach(priv->dma_buf, priv->attach);
	}

	kfree(priv);

	return 0;
}

static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int		ercd;
	int		ret;
	struct MM_BUF_PRIVATE *priv;

	priv = file->private_data;

	switch (cmd) {
	case MM_IOC_EXPORT_START:
		ercd = mm_ioc_export_start((int __user *)arg, priv);
		if (ercd) {
			printk(KERN_ERR "MMD EXPORT START\n");
			ret = ercd;
			goto exit;
		}
		break;
	case MM_IOC_EXPORT_END:
		mm_ioc_export_end((int __user *)arg, priv);
		break;
	case MM_IOC_IMPORT_START:
		ercd = mm_ioc_import_start((int __user *)arg, priv);
		if (ercd) {
			printk(KERN_ERR "MMD IMPORT START\n");
			ret = ercd;
			goto exit;
		}
		break;
	case MM_IOC_IMPORT_END:
		mm_ioc_import_end(priv);
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

static const struct file_operations fops = {
	.owner		= THIS_MODULE,
	.open		= open,
	.release	= close,
	.unlocked_ioctl	= ioctl,
};

static struct miscdevice misc = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DEVNAME,
	.fops		= &fops,
};

static int dmabuf_attach(struct dma_buf *buf, struct device *dev,
	struct dma_buf_attachment *attach)
{
	return 0;
}

static void dmabuf_detach(struct dma_buf *buf,
	struct dma_buf_attachment *attach)
{

}

static struct sg_table *dmabuf_map_dma_buf(struct dma_buf_attachment *attach,
	enum dma_data_direction dir)
{
	struct MM_BUF_PRIVATE *priv = attach->dmabuf->priv;
	struct sg_table *sgt;

	sgt = kmalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	if (sg_alloc_table(sgt, 1, GFP_KERNEL)) {
		kfree(sgt);
		return NULL;
	}

	sg_dma_address(sgt->sgl) = priv->hard_addr;
	sg_dma_len(sgt->sgl) = priv->size;

	return sgt;
}

static void dmabuf_unmap_dma_buf(struct dma_buf_attachment *attach,
	struct sg_table *sgt, enum dma_data_direction dir)
{
	sg_free_table(sgt);
	kfree(sgt);
}

static void dmabuf_release(struct dma_buf *buf)
{

}

static int dmabuf_begin_cpu_access(struct dma_buf *buf, size_t size,
	size_t length, enum dma_data_direction direction)
{
	return 0;
}

static void dmabuf_end_cpu_access(struct dma_buf *buf, size_t size,
	size_t length, enum dma_data_direction direction)
{

}

static void *dmabuf_kmap_atomic(struct dma_buf *buf, unsigned long page)
{
	return NULL;
}

static void dmabuf_kunmap_atomic(struct dma_buf *buf, unsigned long page,
	void *vaddr)
{

}

static void *dmabuf_kmap(struct dma_buf *buf, unsigned long page)
{
	return NULL;
}

static void dmabuf_kunmap(struct dma_buf *buf, unsigned long page, void *vaddr)
{

}

static int dmabuf_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	pgprot_t prot = vm_get_page_prot(vma->vm_flags); 
	struct MM_BUF_PRIVATE *priv = buf->priv;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	vma->vm_flags |= (VM_IO | VM_DONTEXPAND | VM_DONTDUMP);
#else
	vma->vm_flags |= (VM_IO | VM_RESERVED);
#endif
	vma->vm_private_data = priv; 
	vma->vm_page_prot = pgprot_writecombine(prot); 

	return remap_pfn_range(vma, vma->vm_start, priv->hard_addr >> PAGE_SHIFT, 
		vma->vm_end - vma->vm_start, vma->vm_page_prot); 

	return 0;
}

static void *dmabuf_vmap(struct dma_buf *buf)
{
	return NULL;
}

static void dmabuf_vunmap(struct dma_buf *buf, void *vaddr)
{

}

static const struct dma_buf_ops dmabuf_ops = {
	.attach = dmabuf_attach,
	.detach = dmabuf_detach,
	.map_dma_buf = dmabuf_map_dma_buf,
	.unmap_dma_buf = dmabuf_unmap_dma_buf,
	.release = dmabuf_release,
	.begin_cpu_access = dmabuf_begin_cpu_access,
	.end_cpu_access = dmabuf_end_cpu_access,
	.kmap_atomic = dmabuf_kmap_atomic,
	.kunmap_atomic = dmabuf_kunmap_atomic,
	.kmap = dmabuf_kmap,
	.kunmap = dmabuf_kunmap,
	.mmap = dmabuf_mmap,
	.vmap = dmabuf_vmap,
	.vunmap = dmabuf_vunmap,
};

static int mm_ioc_export_start(int __user *arg, struct MM_BUF_PRIVATE *priv)
{
	struct MM_BUF_PARAM	tmp;

	if (copy_from_user(&tmp, arg, sizeof(struct MM_BUF_PARAM)))
		goto exit;

	priv->dma_buf = dma_buf_export(priv, &dmabuf_ops, tmp.size, O_RDWR);
	if (IS_ERR(priv->dma_buf))
		goto exit;

	tmp.buf = dma_buf_fd(priv->dma_buf, 0);
	if (tmp.buf < 0)
		goto exit;

	priv->size = tmp.size;
	priv->hard_addr = tmp.hard_addr;
	priv->buf = tmp.buf;

	if (copy_to_user(arg, &tmp, sizeof(struct MM_BUF_PARAM)))
		goto exit;

	return 0;
exit:
	return -1;
}

static int mm_ioc_export_end(int __user *arg, struct MM_BUF_PRIVATE *priv)
{
	struct MM_BUF_PARAM	tmp;

	tmp.buf = priv->buf;

	if (copy_to_user(arg, &tmp, sizeof(struct MM_BUF_PARAM)))
		goto exit;

	return 0;
exit:
	return -1;
}

static int mm_ioc_import_start(int __user *arg, struct MM_BUF_PRIVATE *priv)
{
	struct MM_BUF_PARAM	tmp;

	if (copy_from_user(&tmp, arg, sizeof(struct MM_BUF_PARAM)))
		goto exit;

	priv->dma_buf = dma_buf_get(tmp.buf);
	if (IS_ERR(priv->dma_buf))
		goto exit;

	priv->attach = dma_buf_attach(priv->dma_buf, misc.this_device);
	if (IS_ERR(priv->attach))
		goto exit;

	priv->sgt = dma_buf_map_attachment(priv->attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(priv->sgt))
		goto err_detach;

	tmp.hard_addr = sg_dma_address(priv->sgt->sgl);
	tmp.size = sg_dma_len(priv->sgt->sgl);
	priv->hard_addr = tmp.hard_addr;
	priv->size = tmp.size;

	if (copy_to_user(arg, &tmp, sizeof(struct MM_BUF_PARAM)))
		goto err_unmap;

	return 0;
err_unmap:
	dma_buf_unmap_attachment(priv->attach, priv->sgt, DMA_BIDIRECTIONAL);
	priv->sgt = NULL;
err_detach:
	dma_buf_detach(priv->dma_buf, priv->attach);
	priv->attach = NULL;
exit:
	return -1;
}

static int mm_ioc_import_end(struct MM_BUF_PRIVATE *priv)
{
	dma_buf_unmap_attachment(priv->attach, priv->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(priv->dma_buf, priv->attach);
	dma_buf_put(priv->dma_buf);
	priv->sgt = NULL;
	priv->attach = NULL;

	return 0;
}

static int mm_init(void)
{
	misc_register(&misc);

	return 0;
}

static void mm_exit(void)
{
	misc_deregister(&misc);
}

module_init(mm_init);
module_exit(mm_exit);

MODULE_LICENSE("Dual MIT/GPL");
