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
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include "include/fdpm_def.h"
#include "fdpm_drv.h"
#include "fdpm_if_par.h"
#include "fdp/fdp_depend.h"
#include "fdp/fdp_drv_l.h"
#include "fdp/fdp_drv_lfunc.h"
#include "fdp/fdp_drv.h"
#include "fdp/fdp_drv_hardw.h"
#include "include/fdpm_log.h"
#include "fdpm_public.h"

static irqreturn_t fdp_ins_ih(int irq, void *dev);
void disp_fdp_reg(T_FDPD_MEM *FDP_obj);
/*** Constant define *********************************************************/


/******************************************************************************
	Function:		drv_FDP_Init
	Description:	Initialize call
	Parameter:		*FDP_obj
	Returns:		0/EINVAL
******************************************************************************/
int drv_FDP_Init(T_FDPD_MEM *FDP_obj)
{
	/* Get_Chip_Ver Start */
	/*	gChip_Ver = GET_REG(PRR); */
	/*  Get_Chip_Ver End */

	/* Parameter check */
#ifdef DISP_PAR_CHECK_MODE
#ifdef USER_ASSERT_ON
	assert(!(init_par->intlvl > 15));
#else
	if (init_par->intlvl > 15) {
		if (sub_ercd)
			*sub_ercd = E_DISP_PARA_INTLVL;
		return -EINVAL;
	}
#endif
#endif

	/* The memory is allocated. */
	FDP_obj->fdp_in_pic.pic_par = &FDP_obj->fdp_independ.fdp_pic_par;
	/* initialize FDP_obj->fdp_in_pic_pre */
	FDP_obj->fdp_in_pic_pre.pic_par =
		&FDP_obj->fdp_independ.fdp_pic_par_pre;
	/* status initialize */
	FDP_obj->fdp_independ.fdp_state.status = ST_FDP_INI;

	return 0;
}

/******************************************************************************
	Function:		drv_FDP_Quit
	Description:	        FDP driver quit
	Parameter:		*FDP_obj
	Returns:		0/EACCES
******************************************************************************/
int drv_FDP_Quit(T_FDPD_MEM *FDP_obj)
{
	/* status check */
	if (FDP_obj == NULL)
		return EACCES;
	return 0;
}

/******************************************************************************
	Function:		drv_FDP_Open
	Description:	Start FDP driver
	Parameter:		*sub_ercd
	Returns:		0/EACCES/EFAULT
******************************************************************************/
int drv_FDP_Open(T_FDPD_MEM *FDP_obj, long *sub_ercd)
{
	T_FDPD_INITIPC_REG *initipc_reg;
	long ercd;
	long sub;
	T_FDP_R_OPEN *temp_open_par;

	ercd = 0;
	sub = 0;

	/* status check */
	if ((FDP_obj == NULL) ||
	    (FDP_obj->fdp_independ.fdp_state.status != ST_FDP_INI))
		return -EACCES;

	/* FDP initialize */
	initipc_reg = kmalloc(sizeof(T_FDPD_INITIPC_REG), GFP_KERNEL);
	if (initipc_reg == NULL)
		return -EFAULT;
	ercd = fdp_GetInitIpcParam(initipc_reg);
	fdp_set_RegInitIpc(FDP_obj, initipc_reg);
	kfree(initipc_reg);

	temp_open_par = kmalloc(sizeof(T_FDP_R_OPEN), GFP_KERNEL);
	if (temp_open_par == NULL)
		return -EFAULT;

	memset(temp_open_par, 0, sizeof(T_FDP_R_OPEN));
	temp_open_par->clkmode = 1;
	/* Vint mode & clock stop mode setting */
	fdp_set_VintClkStopMode(FDP_obj, temp_open_par);
	kfree(temp_open_par);

	/* initialize state */
	FDP_obj->fdp_independ.fdp_state.delay = 0;
	FDP_obj->fdp_independ.fdp_state.seq_lock = FDP_SEQ_UNLOCK;
	FDP_obj->fdp_independ.fdp_state.in_enable = FDP_IN_DISABLE;
	FDP_obj->fdp_independ.fdp_state.in_left = 0;
	FDP_obj->fdp_independ.fdp_state.out_enable = FDP_OUT_DISABLE;
	FDP_obj->fdp_independ.fdp_state.out_left = 0;
	FDP_obj->fdp_independ.fdp_state.out_req = FDP_OUT_NOREQ;
	FDP_obj->fdp_independ.fdp_state_pre.delay = 0;
	FDP_obj->fdp_independ.fdp_state_pre.seq_lock = FDP_SEQ_UNLOCK;
	FDP_obj->fdp_independ.fdp_state_pre.in_enable = FDP_IN_DISABLE;
	FDP_obj->fdp_independ.fdp_state_pre.in_left = 0;
	FDP_obj->fdp_independ.fdp_state_pre.out_enable = FDP_OUT_DISABLE;
	FDP_obj->fdp_independ.fdp_state_pre.out_left = 0;
	FDP_obj->fdp_independ.fdp_state_pre.out_req = FDP_OUT_NOREQ;
	memset(&FDP_obj->fdp_out_state_pipe, 0, sizeof(T_FDPD_OUT_STATE_PIPE));
	memset(&FDP_obj->fdp_telecine_par, 0, sizeof(T_FDPD_TELECINE_PAR));
	memset(&FDP_obj->fdp_telecine_par_pre, 0, sizeof(T_FDPD_TELECINE_PAR));

	/* interrupt flag set */
	FDP_obj->fdp_interrupt_chk_flag = 1;

	/* status set */
	/* FDP1 for best effort mode2 */
	FDP_obj->fdp_independ.fdp_state.status = ST_FDP_RDY;

	/* interrupt flag reset */
	FDP_obj->fdp_interrupt_chk_flag = 0;

	*sub_ercd = sub;

	return 0;
}

/******************************************************************************
	Function:		drv_FDP_Close
	Description:	        Stop FDP driver
	Parameter:		*FDP_obj
	Returns:		0/EACCES
******************************************************************************/
int drv_FDP_Close(T_FDPD_MEM *FDP_obj)
{
	int ercd;

	ercd = 0;

	/* Has not initialization been executed? */
	if (FDP_obj == NULL)
		return EACCES;

	/* status check */
	if ((FDP_obj == NULL) ||
	    (FDP_obj->fdp_independ.fdp_state.status != ST_FDP_RDY))
		return EACCES;

	/* status set */
	FDP_obj->fdp_independ.fdp_state.status = ST_FDP_INI;

	return 0;
}

/******************************************************************************
	Function:		drv_FDP_Start
	Description:	FDP Start
	Parameter:		*FDP_obj, *start_par
	Returns:		0/EACCES
******************************************************************************/
int drv_FDP_Start(T_FDPD_MEM *FDP_obj, T_FDP_R_START *start_par)
{
	DPRINT("drv_FDP_Start enter\n");
	/* Has not iitialization been executed? */
	if (FDP_obj == NULL)
		return EACCES;

	/* status check */
	if ((FDP_obj == NULL) ||
	    ((FDP_obj->fdp_independ.fdp_state.status != ST_FDP_RDY) &&
	     (FDP_obj->fdp_independ.fdp_state.status != ST_FDP_RUN)))
		return EACCES;

	/* FDP process start parameter setting */
	fdp_set_StartParm(FDP_obj, start_par);

	/* interrupt flag set */
	FDP_obj->fdp_interrupt_chk_flag = 1;

	/* status set */
	FDP_obj->fdp_independ.fdp_state.status = ST_FDP_BSY;

	/*  disp_fdp_reg(FDP_obj); */
	/* REGEND=1 */
	if (start_par->fproc_par.out_buf_flg == 1) {
		fdp_reg_write(0x1, P_FDP+FD1_CTL_REGEND);
		fdp_reg_write(0x00000001, P_FDP+FD1_CTL_SGCMD);
		fdp_reg_rwrite(0x0, 0xfffffffe, P_FDP+FD1_CTL_CMD);
		fdp_reg_write(0x0, P_FDP+FD1_CTL_SGCMD);
	}
	/* interupt flag reset */
	FDP_obj->fdp_interrupt_chk_flag = 0;

	return 0;
}

/******************************************************************************
	Function:		drv_FDP_Status
	Description:	FDP Status check
	Parameter:		*fdp_status
	Returns:		0/EACCES/EINVAL
******************************************************************************/
int drv_FDP_Status(T_FDPD_MEM *FDP_obj, T_FDP_STATUS *fdp_status)
{
	/* Has not initialization been executed? */
	if (FDP_obj == NULL)
		return EACCES;

	/* Parameter check */
#ifdef FDP_PAR_CHECK_MODE
#ifdef USER_ASSERT_ON
	assert(fdp_status != NULL);
#else
	if (fdp_status == NULL) {
		if (sub_ercd)
			*sub_ercd = E_FDP_PARA_STATUS;
		return EINVAL;
	}
#endif
#endif
	fdp_status->status      = FDP_obj->fdp_independ.fdp_state.status;
	fdp_status->delay	= FDP_obj->fdp_independ.fdp_state.delay;
	fdp_status->vcycle	= FDP_obj->fdp_independ.fdp_state.vcycle;
	fdp_status->vintcnt	= FDP_obj->fdp_independ.fdp_state.vintcnt;
	fdp_status->seq_lock	= FDP_obj->fdp_independ.fdp_state.seq_lock;
	fdp_status->in_enable	= FDP_obj->fdp_independ.fdp_state.in_enable;
	fdp_status->in_picid	= FDP_obj->fdp_independ.fdp_state.in_picid;
	fdp_status->in_left	= FDP_obj->fdp_independ.fdp_state.in_left;
	fdp_status->out_enable	= FDP_obj->fdp_independ.fdp_state.out_enable;
	fdp_status->out_picid	= FDP_obj->fdp_independ.fdp_state.out_picid;
	fdp_status->out_left	= FDP_obj->fdp_independ.fdp_state.out_left;
	fdp_status->out_req	= FDP_obj->fdp_independ.fdp_state.out_req;

	return 0;
}

/******************************************************************************
	Function:	drv_FDP_finish_write
	Description:	FDP finish interrupt force set
******************************************************************************/
void drv_FDP_finish_write(T_FDPD_MEM *FDP_obj)
{
	fdp_reg_write(0x1, P_FDP+FD1_CTL_IRQFSET);
}

/******************************************************************************
	Function:		FDP_int_Hdr
	Description:	FDP Interrupt handler
	Parameter:		none
	Returns:		none
******************************************************************************/
void FDP_int_Hdr(T_FDPD_MEM *FDP_obj)
{
	T_FDP_CB2	fdp_cb2;

	/* Has not initialization been executed? */
	if (FDP_obj == NULL)
		return;

	/* update state */
	fdp_int_update_state(FDP_obj);

	/* prepare fdp_cb2 */
	if (FDP_obj->fdp_independ.fdp_state.delay > 0)
		fdp_cb2.ercd = E_FDP_DELAYED;
	else
		fdp_cb2.ercd = E_FDP_END;

	fdp_cb2.userdata2 = FDP_obj->fdp_sub.userdata2;

	/* execute callback2 */
	FDP_obj->fdp_sub.fdp_cb2(FDP_obj->fdp_sub.userdata2);
}

/* ------------------------------------- */
long fdp_ins_init_reg(struct platform_device *pdev, int devno, T_FDPD_MEM *prv)
{
	struct resource *res;

	DPRINT("called\n");

	/* get an I/O memory resource for device */
	res = platform_get_resource(pdev, IORESOURCE_MEM, devno);
	if (!res) {
		APRINT("[%s] platform_get_resource() failed!!\n", __func__);
		return -ENXIO;
	}

	DPRINT("res->start:%08x\n", res->start);
	prv->mmio_res = request_mem_region(res->start, 0x2400, res->name);
	if (prv->mmio_res == NULL) {
		APRINT("[%s] request_mem_region failed!!\n", __func__);
		return -ENXIO;
	}
	prv->start_reg_adr = res->start;
	/* remap I/O memory */
	prv->mapbase = ioremap_nocache(res->start, resource_size(res));
	if (!prv->mapbase) {
		APRINT("[%s] ioremap_nocache() failed!!\n", __func__);
		return -ENXIO;
	}
	DPRINT("mapbase:%08x\n", (unsigned int)prv->mapbase);
	DPRINT("done\n");
	return 0;
}

long fdp_ins_quit_reg(T_FDPD_MEM *prv)
{
	DPRINT("called\n");
	if (prv->mapbase) {
		iounmap(prv->mapbase);
		prv->mapbase = 0;
	}
	if (prv->mmio_res != NULL)
		release_mem_region(prv->start_reg_adr, 0x2400);

	DPRINT("done\n");
	return 0;
}

long fdp_ins_free_memory(T_FDPD_MEM *prv)
{
	DPRINT("called\n");

	/* clear memory */
	memset(prv, 0, sizeof(T_FDPD_MEM));

	/* release memory */
	kfree(prv);

	DPRINT("done\n");
	return 0;
}

long fdp_reg_inth(struct platform_device *pdev, int devno, T_FDPD_MEM *prv)
{
	int ercd;

	prv->irq = platform_get_irq(pdev, devno);
	if (prv->irq < 0) {
		APRINT("[%s] platform_ge_irq failed!! ercd=%d\n",
		       __func__, prv->irq);
		return E_FDP_DEF_INH;
	}

	/* registory interrupt handler */
	ercd = request_irq(prv->irq, fdp_ins_ih, IRQF_SHARED, DEVNAME, prv);
	if (ercd) {
		APRINT("[%s] request_irq failed!! ercd = %d irq=%d\n",
		       __func__, ercd, prv->irq);
		return E_FDP_DEF_INH;
	}

	DPRINT("done\n");
	return 0;
}

long fdpm_free_inth(struct platform_device *pdev, int devno, T_FDPD_MEM *prv)
{
	prv->irq = platform_get_irq(pdev, devno);
	if (prv->irq < 0) {
		APRINT("[%s] platform_ge_irq failed!! ercd=%d\n",
		       __func__, prv->irq);
		return E_FDP_DEF_INH;
	}

	/* registory interrupt handler */
	free_irq(prv->irq, prv);

	DPRINT("done\n");
	return 0;
}

static irqreturn_t fdp_ins_ih(int irq, void *dev)
{
	T_FDPD_MEM *FDP_obj = (T_FDPD_MEM *)dev;

	unsigned long	intstatus;

	DPRINT("FDP INT:\n");

	/* interrupt status read */
	intstatus = fdp_reg_read(P_FDP+FD1_CTL_IRQSTA);

	/* FDP end status */
	if (intstatus & FEND_STATUS_BIT) {
		fdp_reg_rwrite(0x00000000, ~FEND_STATUS_BIT,
			       P_FDP+FD1_CTL_IRQSTA);
		FDP_int_Hdr(FDP_obj);
	}
	return IRQ_HANDLED;
}

void disp_fdp_reg(T_FDPD_MEM *FDP_obj)
{
	unsigned long reg_data;
	int i = 0;
	reg_data = fdp_reg_read(P_FDP+FD1_CTL_CMD);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_CTL_CMD");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_CTL_SGCMD);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_CTL_SGCMD");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_CTL_OPMODE);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_CTL_OPMODE");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_CTL_CHACT);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_CTL_CHACT");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_CTL_VPERIOD);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_CTL_VPERIOD");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_CTL_VCYCLE_STAT);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_VCYCLE_STAT");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF_SIZE);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF_SIZE");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF_FORMAT);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF_FORMAT");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF_PSTRIDE);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF_PSTRIDE");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF0_ADDR_Y);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF0_ADDR_Y");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF0_ADDR_C0);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF0_ADDR_C0");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF0_ADDR_C1);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF0_ADDR_C1");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF1_ADDR_Y);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF1_ADDR_Y");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF1_ADDR_C0);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF1_ADDR_C0");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF1_ADDR_C1);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF1_ADDR_C1");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF2_ADDR_Y);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF2_ADDR_Y");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF2_ADDR_C0);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF2_ADDR_C0");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF2_ADDR_C1);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF2_ADDR_C1");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_WPF_FORMAT);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_WPF_FORMAT");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_WPF_PSTRIDE);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_WPF_PSTRIDE");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_WPF_ADDR_Y);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_WPF_ADDR_Y");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_WPF_ADDR_C0);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_WPF_ADDR_C0");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_WPF_ADDR_C1);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_WPF_ADDR_C1");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_RPF_SMSK_ADDR);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_RPF_SMSK_ADDR");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;

	reg_data = fdp_reg_read(P_FDP+FD1_IPC_MODE);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_MODE");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_SMSK_THRESH);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_SMSK_THRESH");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_COMB_DET);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_COMB_DET");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_MOTDEC);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_MOTDEC");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_DLI_BLEND);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_DLI_BLEND");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_DLI_HGAIN);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_DLI_HGAIN");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_DLI_SPRS);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_DLI_SPRS");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_DLI_ANGLE);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_PIC_DLI_ANGLE");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_DLI_ISOPIX0);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_DLI_ISOPIX0");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_DLI_ISOPIX1);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_DLI_ISOPIX1");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;

	reg_data = fdp_reg_read(P_FDP+FD1_CTL_IRQSTA);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_CTL_IRQSTA");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;

	fdp_reg_write(0x0, P_FDP+FD1_CTL_IRQSTA);
	reg_data = fdp_reg_read(P_FDP+FD1_IPC_LMEM);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_IPC_LMEM");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	i++;

	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_0);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_0");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[0] = reg_data;
	i++;

	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_1);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_1");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[1] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_2);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_2");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[2] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_3);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_3");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[3] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_4);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_4");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[4] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_5);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_5");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[5] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_6);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_6");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[6] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_7);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_7");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[7] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_8);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_8");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[8] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_9);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_9");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[9] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_10);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_10");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[10] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_11);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_11");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[11] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_12);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_12");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[12] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_13);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_13");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[13] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_14);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_14");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[14] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_15);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_15");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[15] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_16);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_16");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[16] = reg_data;
	i++;
	reg_data = fdp_reg_read(P_FDP+FD1_SNSOR_17);
	strcpy(FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
	       "FD1_SNSOR_17");
	FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].reg_value = reg_data;
	FDP_obj->fdp_independ.fdp_state.sinfo[17] = reg_data;
	i++;
}
