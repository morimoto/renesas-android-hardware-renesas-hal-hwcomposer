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
#ifndef __MMNGR_PRIVATE_H__
#define __MMNGR_PRIVATE_H__

#define DEVFILE "/dev/rgnmm"

struct MM_DRVDATA {
	struct device *mm_dev;
	struct device *mm_dev_reserve;
	unsigned long	reserve_size;
	unsigned long	reserve_phy_addr;
	unsigned long	reserve_kernel_virt_addr;
};

struct MM_PARAM {
	unsigned long	size;
	unsigned long	phy_addr;
	unsigned long	hard_addr;
	unsigned long	user_virt_addr;
	unsigned long	kernel_virt_addr;
	unsigned long	flag;
};

struct BM {
	unsigned long	top_phy_addr;
	unsigned long	order;
	unsigned long	end_bit;
	unsigned long	*bits;
};

#define MM_IOC_MAGIC 'm'
#define MM_IOC_ALLOC	_IOWR(MM_IOC_MAGIC, 0, struct MM_PARAM)
#define MM_IOC_FREE	_IOWR(MM_IOC_MAGIC, 1, struct MM_PARAM)
#define MM_IOC_SET	_IOWR(MM_IOC_MAGIC, 2, struct MM_PARAM)
#define MM_IOC_GET	_IOWR(MM_IOC_MAGIC, 3, struct MM_PARAM)
#define MM_IOC_ALLOC_CO	_IOWR(MM_IOC_MAGIC, 4, struct MM_PARAM)
#define MM_IOC_FREE_CO	_IOWR(MM_IOC_MAGIC, 5, struct MM_PARAM)
#define MM_IOC_SHARE	_IOWR(MM_IOC_MAGIC, 6, struct MM_PARAM)

#define DEVNAME		"rgnmm"
#define DRVNAME		DEVNAME
#define CLSNAME		DEVNAME
#define DEVNUM		1

static int mm_alloc_kh_in_kernel(struct device *mm_dev,
				unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *pkernel_virt_addr);
static void mm_free_kh_in_kernel(struct device *mm_dev,
				unsigned long size,
				unsigned long phy_addr,
				unsigned long hard_addr,
				unsigned long kernel_virt_addr);
static int mm_alloc_co_in_kernel(struct BM *pb,
				unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *pkernel_virt_addr);
static void mm_free_co_in_kernel(struct BM *pb,
				unsigned long size,
				unsigned long phy_addr,
				unsigned long hard_addr,
				unsigned long kernel_virt_addr);
static int mm_ioc_alloc(struct device *mm_dev,
			int __user *in,
			struct MM_PARAM *out);
static void mm_ioc_free(struct device *mm_dev, struct MM_PARAM *p);
static int mm_ioc_set(int __user *in, struct MM_PARAM *out);
static int mm_ioc_get(struct MM_PARAM *in, int __user *out);
static int alloc_bm(struct BM *pb,
		unsigned long top_phy_addr,
		unsigned long size,
		unsigned long order);
static void free_bm(struct BM *pb);
static int mm_ioc_alloc_co(struct BM *pb, int __user *in, struct MM_PARAM *out);
static int mm_ioc_alloc_co_select(int __user *in, struct MM_PARAM *out);
static void mm_ioc_free_co(struct BM *pb, struct MM_PARAM *p);
static void mm_ioc_free_co_select(struct MM_PARAM *p);
static int mm_ioc_share(int __user *in, struct MM_PARAM *out);
static int map_register(void);
static void unmap_register(void);
static void mm_set_mxi_path(unsigned long start, unsigned long end);
static void mm_enable_pmb(void);
static void mm_set_pmb_area(unsigned long start, void *impmba, void *impmbd);
static void mm_enable_vpc_utlb(void);
static void mmngr_dev_set_cma_area(struct device *dev, struct cma *cma);

#ifdef MMNGR_KOELSCH
#define MM_OMXBUF_ADDR	(0x78000000UL)
#define MM_OMXBUF_SIZE	(128 * 1024 * 1024)
#endif
#ifdef MMNGR_LAGER
#define MM_OMXBUF_ADDR	(0x70000000UL)
#define MM_OMXBUF_SIZE	(256 * 1024 * 1024)
#endif
#ifdef MMNGR_ALT
#define MM_OMXBUF_ADDR	(0x78000000UL)
#define MM_OMXBUF_SIZE	(128 * 1024 * 1024)
#endif

#ifdef MMNGR_KOELSCH
#define MM_IPCBUF_ADDR	(0x68000000)
#define MM_IPCBUF_SIZE	(128 * 1024 * 1024)
#endif
#ifdef MMNGR_LAGER
#define MM_IPCBUF_ADDR	(0x68000000)
#define MM_IPCBUF_SIZE	(128 * 1024 * 1024)
#endif
#ifdef MMNGR_ALT
#define MM_IPCBUF_ADDR	(0x68000000)
#define MM_IPCBUF_SIZE	(128 * 1024 * 1024)
#endif

#ifdef MMNGR_KOELSCH
#define MM_SSPBUF_ADDR	(0x6F000000)
#define MM_SSPBUF_SIZE	(16 * 1024 * 1024)
#endif
#ifdef MMNGR_LAGER
#define MM_SSPBUF_ADDR	(0x6F000000)
#define MM_SSPBUF_SIZE	(16 * 1024 * 1024)
#endif
#ifdef MMNGR_ALT
#define MM_SSPBUF_ADDR	(0x6F000000)
#define MM_SSPBUF_SIZE	(16 * 1024 * 1024)
#endif

#ifdef MMNGR_KOELSCH
#define MMNGR_MV_ENABLE
#define MM_MVBUF_ADDR	(0x75000000)
#define MM_MVBUF_SIZE	(40 * 1024 * 1024)
#else

#endif

#define MM_KERNEL_RESERVE_SIZE	(256 * 1024 * 1024)

#define	MM_CO_ORDER		(12)

#define MM_MXI_ADDR		(0xFE960000)
#define MM_MXI_SIZE		(4)

#define MM_IMPCTR_ADDR		(0xFE951A00)
#define MM_IMPMBA0_ADDR		(0xFE951A80)
#define MM_IMPMBA1_ADDR		(0xFE951A84)
#define MM_IMPMBD0_ADDR		(0xFE951AC0)
#define MM_IMPMBD1_ADDR		(0xFE951AC4)
#define MM_IMUCTR19_ADDR	(0xFE951C30)
#define MM_IMUCTR22_ADDR	(0xFE951C60)
#define MM_IPMMU_SIZE		(4)

#define MM_LEGACY_ADDR		(0x40000000)
#define MM_ADDR_MASK		(0xFF000000)
#define MM_IMPCTR_VAL		(0x00000001)
#define MM_IMUCTR_VAL		(0x00000081)
#define MM_IMPMBA_VAL		(0x00000100)
#define MM_IMPMBD_VAL		(0x00040180)
#define MM_PMB_SIZE_128M	(0x8000000)

#define MM_PRR_ADDR		(0xFF000044)
#define MM_PRR_SIZE		(4)

#define MM_PRR_ESMASK		(0x000000F0)
#define MM_PRR_ES2		(0x00000010)

#endif	/* __MMNGR_PRIVATE_H__ */
