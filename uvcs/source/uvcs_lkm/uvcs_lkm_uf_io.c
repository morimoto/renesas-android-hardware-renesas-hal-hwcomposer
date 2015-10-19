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
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include "uvcs_types.h"
#include "uvcs_cmn.h"
#include "uvcs_lkm_internal.h"

#ifdef PSEUDO_DEV
#error
#endif
/******************************************************************************/
/*                    VARIABLES                                               */
/******************************************************************************/
static struct tasklet_struct tl_vlc[UVCS_CMN_MAX_HW_NUM];
static struct tasklet_struct tl_ce[UVCS_CMN_MAX_HW_NUM];
static ulong reg_vlc[UVCS_CMN_MAX_HW_NUM];
static ulong reg_ce[UVCS_CMN_MAX_HW_NUM];
static ulong reg_vpc[UVCS_CMN_MAX_HW_NUM];
static ulong reg_vpcxy[UVCS_CMN_MAX_HW_NUM];
static struct clk *clk_vcp[UVCS_CMN_MAX_HW_NUM];
static struct clk *clk_vpc[UVCS_CMN_MAX_HW_NUM];

ulong ip_option = UVCS_IPOPT_DEFAULT;
module_param(ip_option, ulong, 0);
/******************************************************************************/
/*                    FORWARD DECLARATIONS                                    */
/******************************************************************************/
static irqreturn_t uvcs_isr_0(int irq, void *dev);
static irqreturn_t uvcs_isr_1(int irq, void *dev);
static irqreturn_t uvcs_isr_2(int irq, void *dev);
static irqreturn_t uvcs_isr_3(int irq, void *dev);
static void uvcs_tasklet_0(ulong value);
static void uvcs_tasklet_1(ulong value);
static void uvcs_tasklet_2(ulong value);
static void uvcs_tasklet_3(ulong value);
static void uvcs_vpc_setup(struct uvcs_drv_info *, UVCS_U32, UVCS_U32 *);
static void uvcs_vpc_init(struct uvcs_drv_info *, UVCS_U32);

/************************************************************//**
 *
 * Function Name
 *      uvcs_isr
 *
 * Return
 *      @retval IRQ_HANDLED
 *      @retval IRQ_NONE
 *
 ****************************************************************/
static irqreturn_t uvcs_isr_0(int irq, void *dev)
{
	if (dev == &reg_vlc[0]) {
		iowrite32(0uL, (void *)(reg_vlc[0] + UVCS_VCPREG_IRQENB));
		tasklet_schedule(&tl_vlc[0]);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static irqreturn_t uvcs_isr_1(int irq, void *dev)
{
	if (dev == &reg_ce[0]) {
		iowrite32(0uL, (void *)(reg_ce[0] + UVCS_VCPREG_IRQENB));
		tasklet_schedule(&tl_ce[0]);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static irqreturn_t uvcs_isr_2(int irq, void *dev)
{
	if (dev == &reg_vlc[1]) {
		iowrite32(0uL, (void *)(reg_vlc[1] + UVCS_VCPREG_IRQENB));
		tasklet_schedule(&tl_vlc[1]);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static irqreturn_t uvcs_isr_3(int irq, void *dev)
{
	if (dev == &reg_ce[1]) {
		iowrite32(0uL, (void *)(reg_ce[1] + UVCS_VCPREG_IRQENB));
		tasklet_schedule(&tl_ce[1]);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_tasklet
 *
 * Return
 *      @return none
 *
 ****************************************************************/
static void uvcs_tasklet_0(ulong value)
{
	struct uvcs_drv_info *local = (struct uvcs_drv_info *)value;
	struct timespec ts;

	if (local != NULL) {
		getrawmonotonic(&ts);
		uvcs_cmn_interrupt(local->uvcs_info, reg_vlc[0], ts.tv_nsec);
	}
}

static void uvcs_tasklet_1(ulong value)
{
	struct uvcs_drv_info *local = (struct uvcs_drv_info *)value;
	struct timespec ts;

	if (local != NULL) {
		getrawmonotonic(&ts);
		uvcs_cmn_interrupt(local->uvcs_info, reg_ce[0], ts.tv_nsec);
	}
}

static void uvcs_tasklet_2(ulong value)
{
	struct uvcs_drv_info *local = (struct uvcs_drv_info *)value;
	struct timespec ts;

	if (local != NULL) {
		getrawmonotonic(&ts);
		uvcs_cmn_interrupt(local->uvcs_info, reg_vlc[1], ts.tv_nsec);
	}
}

static void uvcs_tasklet_3(ulong value)
{
	struct uvcs_drv_info *local = (struct uvcs_drv_info *)value;
	struct timespec ts;

	if (local != NULL) {
		getrawmonotonic(&ts);
		uvcs_cmn_interrupt(local->uvcs_info, reg_ce[1], ts.tv_nsec);
	}
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_register_read (-> cb_reg_read)
 *
 * Return
 *      @return none
 *
 ****************************************************************/
static void uvcs_register_read(
		UVCS_PTR udptr,
		volatile UVCS_U32 *reg_addr,
		UVCS_U32 *dst_addr,
		UVCS_U32 num_reg)
{
	while (num_reg > 0) {
		*dst_addr++ = ioread32(reg_addr++);
		num_reg--;
	}
	rmb();
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_register_write (-> cb_reg_write)
 *
 * Return
 *      @return none
 *
 ****************************************************************/
static void uvcs_register_write(
		UVCS_PTR udptr,
		volatile UVCS_U32 *reg_addr,
		UVCS_U32 *src_addr,
		UVCS_U32 num_reg)
{
	while (num_reg > 0) {
		iowrite32(*src_addr++, reg_addr++);
		num_reg--;
	}
	wmb();
}



/* cb_hw_xxxxx */
void uvcs_hw_start(
		UVCS_PTR  udptr,
		UVCS_U32  hw_ip_id,
		UVCS_U32  hw_module_id,
		UVCS_U32 *baa)
{
	struct uvcs_drv_info *local = (struct uvcs_drv_info *)udptr;

	/* power management code or hw-cache setting code is implemented here */
	if ((local != NULL) && (baa != NULL)) {
		if ((hw_module_id == 1uL) && (hw_ip_id < UVCS_CMN_MAX_HW_NUM))
			uvcs_vpc_setup(local, hw_ip_id, baa);
	}
}

void uvcs_hw_stop(
		UVCS_PTR  udptr,
		UVCS_U32  hw_ip_id,
		UVCS_U32  hw_module_id)
{
	/* power management code or hw-cache setting code is implemented here */
}

static void uvcs_vpc_wait_end(void *vpcsts)
{
	ulong reg;
	ulong wait_cnt = 0uL;

	reg = ioread32(vpcsts);
	rmb();

	while (((reg & 0x1uL) == 0uL) && (wait_cnt < UVCS_VPC_WAIT_MAX)) {
		udelay(UVCS_VPC_WAIT_TIME);
		wait_cnt++;
		reg = ioread32(vpcsts);
		rmb();
	}
}

static void uvcs_vpc_clear(void *vpcctl)
{
	ulong reg;
	ulong wait_cnt = 0uL;

	reg = ioread32(vpcctl);
	rmb();

	reg |= 0x2uL;
	iowrite32(reg, vpcctl);
	wmb();

	reg = ioread32(vpcctl);
	rmb();

	while (((reg & 0x2uL) != 0uL) && (wait_cnt < UVCS_VPC_WAIT_MAX)) {
		udelay(UVCS_VPC_WAIT_TIME);
		wait_cnt++;
		reg = ioread32(vpcctl);
		rmb();
	}
}

static void uvcs_vpc_setup(
					struct uvcs_drv_info *local,
					UVCS_U32 hw_ip_id,
					UVCS_U32 *baa)
{
	void *vpcctl = (void *)(reg_vpc[hw_ip_id] + UVCS_VPCREG_VPCCTL);
	void *vpcsts = (void *)(reg_vpc[hw_ip_id] + UVCS_VPCREG_VPCSTS);
	ulong regdata;

	uvcs_vpc_wait_end(vpcsts);

	/* 30A.3.1-1 */
	regdata = ioread32(vpcctl);
	rmb();
	regdata &= ~0xCuL;

	if (baa[UVCS_BAAIDX_STRIDE] > 1024)
		regdata |= 0x1000000CuL;
	else if (baa[UVCS_BAAIDX_STRIDE] > 512)
		regdata |= 0x10000008uL;
	else if (baa[UVCS_BAAIDX_STRIDE] > 256)
		regdata |= 0x10000004uL;
	else
		regdata |= 0x10000000uL;

	iowrite32(regdata, vpcctl);
	wmb();

	/* 30A.3.1-2 (init) */

	/* 30A.3.1-3 */
	uvcs_vpc_wait_end(vpcsts);
	uvcs_vpc_clear(vpcctl);

	/* 30A.3.1-4 */
	uvcs_vpc_wait_end(vpcsts);
	regdata = ioread32(vpcctl);
	regdata |= 1uL;
	rmb();
	iowrite32(regdata, vpcctl);
	wmb();
}

static void uvcs_vpc_init(struct uvcs_drv_info *local, UVCS_U32 hw_ip_id)
{
	if (reg_vpc[hw_ip_id] != 0uL) {
		void *vpcctl = (void *)(reg_vpc[hw_ip_id] + UVCS_VPCREG_VPCCTL);
		void *vpccfg = (void *)(reg_vpc[hw_ip_id] + UVCS_VPCREG_VPCCFG);
		void *vpcsts = (void *)(reg_vpc[hw_ip_id] + UVCS_VPCREG_VPCSTS);
		UVCS_U32 regdata;

		uvcs_vpc_wait_end(vpcsts);
		regdata  = ioread32(vpcctl);
		rmb();

		regdata |= 0x100CuL;
		iowrite32(regdata, vpcctl);
		wmb();

		uvcs_vpc_clear(vpcctl);

		/* 30A.3.1-2 */
		switch (local->module_param.lsi_type) {
		case UVCS_LSITYPE_H2_VX:
			regdata = 0x0002030AuL | UVCS_VPCCFG_MODE;
			iowrite32(regdata, vpccfg);
			wmb();

			if ((regdata & 0x00000001) != 1uL)
				regdata = 0x0uL;
			else
				regdata = 0x3uL;

			iowrite32(regdata, (void *)reg_vpcxy[hw_ip_id]);
			break;

		case UVCS_LSITYPE_M2W_VX:
		case UVCS_LSITYPE_M2N:
		case UVCS_LSITYPE_E2:
			regdata = 0x0002030AuL | UVCS_VPCCFG_MODE;
			iowrite32(regdata, vpccfg);
			break;

		case UVCS_LSITYPE_M2W_V1:
			regdata = 0x0002030BuL;
			iowrite32(regdata, vpccfg);
			break;

		case UVCS_LSITYPE_H2_V1:
			iowrite32(0, vpccfg);
			break;

		default: /* same as M2W_Vx, including V2H */
			regdata = 0x0002030AuL | UVCS_VPCCFG_MODE;
			iowrite32(regdata, vpccfg);
			break;
		}
		wmb();
	}
}

static int uvcs_remap_reg(struct uvcs_drv_info *local)
{
	ulong *regptr;
	ulong  regdat;

	if (local == NULL)
		return -EFAULT;

	/* check hw type */
	regptr = ioremap_nocache(UVCS_REG_PRR, UVCS_REG_SIZE_SINGLE);
	regdat = ioread32(regptr);
	iounmap(regptr);

	/* common */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	clk_vpc[0] = devm_clk_get(&local->pdev->dev, "vpc0");
	clk_vpc[1] = NULL;
	clk_vcp[0] = devm_clk_get(&local->pdev->dev, "vcp0");
	clk_vcp[1] = NULL;
	clk_prepare_enable(clk_vpc[0]);
	clk_prepare_enable(clk_vcp[0]);
#else
	clk_vpc[0] = clk_get(NULL, "vpc0");
	clk_vpc[1] = NULL;
	clk_vcp[0] = clk_get(NULL, "vcp0");
	clk_vcp[1] = NULL;
	clk_enable(clk_vpc[0]);
	clk_enable(clk_vcp[0]);
#endif
	reg_vlc[0] = (ulong)ioremap_nocache(UVCS_REG_VLC0, UVCS_REG_SIZE_VLC);
	reg_vlc[1] = 0;
	reg_ce[0]  = (ulong)ioremap_nocache(UVCS_REG_CE0,  UVCS_REG_SIZE_CE);
	reg_ce[1]  = 0;
	reg_vpc[0] = (ulong)ioremap_nocache(UVCS_REG_VPC0, UVCS_REG_SIZE_VPC);
	reg_vpc[1] = 0;
	reg_vpcxy[0] = 0;
	reg_vpcxy[1] = 0;

	switch (regdat & UVCS_PRR_MASK_LSI) {
	case UVCS_PRR_LSI_H2:
		/* clock setting */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		clk_vpc[1] = devm_clk_get(&local->pdev->dev, "vpc1");
		clk_vcp[1] = devm_clk_get(&local->pdev->dev, "vcp1");
		clk_prepare_enable(clk_vpc[1]);
		clk_prepare_enable(clk_vcp[1]);
#else
		clk_vpc[1] = clk_get(NULL, "vpc1");
		clk_vcp[1] = clk_get(NULL, "vcp1");
		clk_enable(clk_vpc[1]);
		clk_enable(clk_vcp[1]);
#endif
		/* register mapping */
		local->module_param.hw_num	= UVCS_LSIH2_HW_NUM;
		reg_vlc[1] = (ulong)ioremap_nocache(
					UVCS_REG_VLC1, UVCS_REG_SIZE_VLC);
		reg_ce[1]  = (ulong)ioremap_nocache(
					UVCS_REG_CE1,  UVCS_REG_SIZE_CE);
		reg_vpc[1] = (ulong)ioremap_nocache(
					UVCS_REG_VPC1, UVCS_REG_SIZE_VPC);
		if ((regdat & UVCS_PRR_MASK_VER) == 0uL) {
			local->module_param.lsi_type = UVCS_LSITYPE_H2_V1;
		} else {
			local->module_param.lsi_type = UVCS_LSITYPE_H2_VX;
			reg_vpcxy[0] = (ulong)ioremap_nocache(
					UVCS_REG_VPC0XY, UVCS_REG_SIZE_SINGLE);
			reg_vpcxy[1] = (ulong)ioremap_nocache(
					UVCS_REG_VPC1XY, UVCS_REG_SIZE_SINGLE);
		}
		break;

	case UVCS_PRR_LSI_M2W:
		/* register mapping */
		local->module_param.hw_num = UVCS_LSIM2_HW_NUM;
		if ((regdat & UVCS_PRR_MASK_VER) == 0uL)
			local->module_param.lsi_type	= UVCS_LSITYPE_M2W_V1;
		else
			local->module_param.lsi_type	= UVCS_LSITYPE_M2W_VX;
		break;

	case UVCS_PRR_LSI_M2N:
		/* register mapping */
		local->module_param.hw_num	= UVCS_LSIM2_HW_NUM;
		local->module_param.lsi_type	= UVCS_LSITYPE_M2N;
		break;

	case UVCS_PRR_LSI_E2:
		/* register mapping */
		local->module_param.hw_num	= UVCS_LSIE2_HW_NUM;
		local->module_param.lsi_type	= UVCS_LSITYPE_E2;
		break;

	default: /* same as M2W Vx, including V2H */
		/* clock setting */
		/* register mapping */
		local->module_param.hw_num	= UVCS_LSIM2_HW_NUM;
		local->module_param.lsi_type	= UVCS_LSITYPE_M2W_VX;
		break;
	}

	return 0;
}

static void uvcs_unmap_reg(struct uvcs_drv_info *local)
{
	ulong i;

	/* unmap register for unit0 */
	for (i = 0; i < local->module_param.hw_num; i++) {
		/* unmap registers */
		iounmap((void *)reg_vlc[i]);
		iounmap((void *)reg_ce[i]);
		if (reg_vpc[i])
			iounmap((void *)reg_vpc[i]);
		if (reg_vpcxy[i])
			iounmap((void *)reg_vpcxy[i]);
		/* stop clock */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		clk_disable_unprepare(clk_vcp[i]);
		devm_clk_put(&local->pdev->dev, clk_vcp[i]);
#else
		clk_disable(clk_vcp[i]);
		clk_put(clk_vcp[i]);
#endif
		/* clear */
		reg_vlc[i] = 0;
		reg_ce[i] = 0;
		reg_vpc[i] = 0;
		reg_vpcxy[i] = 0;
		clk_vcp[i] = NULL;
		clk_vpc[i] = NULL;
	}
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_io_init
 *
 * Return
 *      @retval 0          normal end
 *      @retval other      error
 *
 ****************************************************************/
int uvcs_io_init(struct uvcs_drv_info *local)
{
	ulong i;
	UVCS_RESULT uvcs_ret;
	UVCS_CMN_INIT_PARAM_T *iparam;
	int ret;

	if (!local)
		goto err_exit_0;
	if (uvcs_remap_reg(local))
		goto err_exit_0;
	iparam = &local->uvcs_init_param;

#ifdef ENABLE_VIDEO_HW_RESET
	{
		void *srcr1 = ioremap_nocache(
				UVCS_REG_SRCR, UVCS_REG_SIZE_SINGLE);
		void *srstclr1 = ioremap_nocache(
				UVCS_REG_SRSTCLR, UVCS_REG_SIZE_SINGLE);

		/* reset vcp */
		iowrite32(0x3uL, srcr1);
		wmb();
		iowrite32(0x3uL, srstclr1);
		wmb();
		/* reset vpc */
		iowrite32(0xcuL, srcr1);
		wmb();
		iowrite32(0xcuL, srstclr1);
		wmb();

		iounmap(srstclr1);
		iounmap(srcr1);
	}
#endif /* ENABLE_VIDEO_HW_RESET */

	for (i = 0uL; i < local->module_param.hw_num; i++) {
		iparam->ip_base_addr[i][0] = reg_vlc[i];
		iparam->ip_base_addr[i][1] = reg_ce[i];
		uvcs_vpc_init(local, i);
	}

	/* init uvcs-cmn */
	iparam->struct_size    = sizeof(UVCS_CMN_INIT_PARAM_T);
	iparam->udptr          = local;
	iparam->hw_num         = local->module_param.hw_num;
	iparam->cb_hw_start    = &uvcs_hw_start;
	iparam->cb_hw_stop     = &uvcs_hw_stop;
	iparam->cb_proc_done   = &uvcs_hw_processing_done;
	iparam->cb_sem_lock    = &uvcs_semaphore_lock;
	iparam->cb_sem_unlock  = &uvcs_semaphore_unlock;
	iparam->cb_sem_create  = &uvcs_semaphore_create;
	iparam->cb_sem_destroy = &uvcs_semaphore_destroy;
	iparam->cb_thr_event   = &uvcs_thread_event;
	iparam->cb_thr_create  = &uvcs_thread_create;
	iparam->cb_thr_destroy = &uvcs_thread_destroy;
	iparam->cb_reg_read    = &uvcs_register_read;
	iparam->cb_reg_write   = &uvcs_register_write;
	iparam->ip_option      = ip_option;

	uvcs_ret = uvcs_cmn_initialize(iparam, &local->uvcs_info);
	if (uvcs_ret != UVCS_RTN_OK) {
		dev_dbg(&local->pdev->dev, "ioinit, init %d", uvcs_ret);
		goto err_exit_1;
	}

	local->ip_info.struct_size = sizeof(UVCS_CMN_IP_INFO_T);
	uvcs_ret = uvcs_cmn_get_ip_info(local->uvcs_info, &local->ip_info);
	if (uvcs_ret != UVCS_RTN_OK) {
		dev_dbg(&local->pdev->dev, "ioinit, ipinf %d", uvcs_ret);
		goto err_exit_2;
	}

	if (local->module_param.lsi_type == UVCS_LSITYPE_H2_V1) {
		local->ip_info.ip_option &= UVCS_IPOPT_H2V1_MASK;
		local->ip_info.ip_option |= UVCS_IPOPT_H2V1_FIX;
	}

	/* install top half */
	tasklet_init(&tl_vlc[0], uvcs_tasklet_0, (ulong)local);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	ret = devm_request_irq(&local->pdev->dev, UVCS_INT_VLC0,
			&uvcs_isr_0, IRQF_SHARED, "vcp3_0", &reg_vlc[0]);
#else
	ret = request_irq(UVCS_INT_VLC0,
			&uvcs_isr_0, IRQF_SHARED, "vcp3_0", &reg_vlc[0]);
#endif
	if (ret) {
		dev_dbg(&local->pdev->dev, "ioinit, request_irq vlc0");
		goto err_exit_2;
	}

	tasklet_init(&tl_ce[0], uvcs_tasklet_1, (ulong)local);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	ret = devm_request_irq(&local->pdev->dev, UVCS_INT_CE0,
			&uvcs_isr_1, IRQF_SHARED, "vcp3_1", &reg_ce[0]);
#else
	ret = request_irq(UVCS_INT_CE0,
			&uvcs_isr_1, IRQF_SHARED, "vcp3_1", &reg_ce[0]);
#endif
	if (ret) {
		dev_dbg(&local->pdev->dev, "ioinit, request_irq ce0");
		goto err_exit_3;
	}

	if (local->module_param.hw_num == 2) {
		tasklet_init(&tl_vlc[1], uvcs_tasklet_2, (ulong)local);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		ret = devm_request_irq(&local->pdev->dev, UVCS_INT_VLC1,
			&uvcs_isr_2, IRQF_SHARED, "vcp3_2", &reg_vlc[1]);
#else
		ret = request_irq(UVCS_INT_VLC1,
			&uvcs_isr_2, IRQF_SHARED, "vcp3_2", &reg_vlc[1]);
#endif
		if (ret) {
			dev_dbg(&local->pdev->dev, "ioinit, request_irq vlc1");
			goto err_exit_4;
		}

		tasklet_init(&tl_ce[1], uvcs_tasklet_3, (ulong)local);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		ret = devm_request_irq(&local->pdev->dev, UVCS_INT_CE1,
			&uvcs_isr_3, IRQF_SHARED, "vcp3_3", &reg_ce[1]);
#else
		ret = request_irq(UVCS_INT_CE1,
			&uvcs_isr_3, IRQF_SHARED, "vcp3_3", &reg_ce[1]);
#endif
		if (ret) {
			dev_dbg(&local->pdev->dev, "ioinit, request_irq ce1");
			goto err_exit_5;
		}
	}

	return 0;

err_exit_5:
	tasklet_kill(&tl_ce[1]);
	tasklet_kill(&tl_vlc[1]);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	devm_free_irq(&local->pdev->dev, UVCS_INT_VLC1, &reg_vlc[1]);
#else
	free_irq(UVCS_INT_VLC1, &reg_vlc[1]);
#endif

err_exit_4:
	tasklet_kill(&tl_ce[0]);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	devm_free_irq(&local->pdev->dev, UVCS_INT_CE0, &reg_ce[0]);
#else
	free_irq(UVCS_INT_CE0,  &reg_ce[0]);
#endif

err_exit_3:
	tasklet_kill(&tl_vlc[0]);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	devm_free_irq(&local->pdev->dev, UVCS_INT_VLC0, &reg_vlc[0]);
#else
	free_irq(UVCS_INT_VLC0, &reg_vlc[0]);
#endif

err_exit_2:
	uvcs_cmn_deinitialize(local->uvcs_info, UVCS_TRUE);

err_exit_1:
	uvcs_unmap_reg(local);

err_exit_0:

	return -EFAULT;
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_io_cleanup
 *
 * Return
 *      @return none
 *
 ****************************************************************/
void uvcs_io_cleanup(struct uvcs_drv_info *local)
{
	if (local) {
		/* uninstall interrupt */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		devm_free_irq(&local->pdev->dev,
				UVCS_INT_VLC0, &reg_vlc[0]);
		devm_free_irq(&local->pdev->dev,
				UVCS_INT_CE0, &reg_ce[0]);
#else
		free_irq(UVCS_INT_VLC0, &reg_vlc[0]);
		free_irq(UVCS_INT_CE0, &reg_ce[0]);
#endif
		tasklet_kill(&tl_vlc[0]);
		tasklet_kill(&tl_ce[0]);

		if (local->uvcs_init_param.hw_num == 2) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
			devm_free_irq(&local->pdev->dev,
					UVCS_INT_VLC1, &reg_vlc[1]);
			devm_free_irq(&local->pdev->dev,
					UVCS_INT_CE1, &reg_ce[1]);
#else
			free_irq(UVCS_INT_VLC1, &reg_vlc[1]);
			free_irq(UVCS_INT_CE1, &reg_ce[1]);
#endif
			tasklet_kill(&tl_vlc[1]);
			tasklet_kill(&tl_ce[1]);
		}

		uvcs_cmn_deinitialize(local->uvcs_info, UVCS_TRUE);
		uvcs_unmap_reg(local);
	}
}


