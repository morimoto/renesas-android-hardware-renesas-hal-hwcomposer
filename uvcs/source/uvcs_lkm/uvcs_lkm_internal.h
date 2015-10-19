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

#ifndef UVCS_LKM_INTERNAL_H
#define UVCS_LKM_INTERNAL_H

#define DEVNAME			"uvcs"
#define DRVNAME			DEVNAME
#define CLSNAME			DEVNAME
#define DEVNUM			1

#define UVCS_REG_VLC0		(0xFE900000uL)
#define UVCS_INT_VLC0		(258uL + 32uL)
#define UVCS_REG_CE0		(0xFE900200uL)
#define UVCS_INT_CE0		(259uL + 32uL)
#define UVCS_REG_VLC1		(0xFE910000uL)
#define UVCS_INT_VLC1		(260uL + 32uL)
#define UVCS_REG_CE1		(0xFE910200uL)
#define UVCS_INT_CE1		(261uL + 32uL)
#define UVCS_REG_VPC0		(0xFE908000uL)
#define UVCS_REG_VPC1		(0xFE918000uL)
#define UVCS_REG_VPC0XY		(0xFE960380uL)
#define UVCS_REG_VPC1XY		(0xFE960384uL)
#define UVCS_REG_PRR		(0xFF000044uL)
#define UVCS_REG_SRCR		(0xE61500a8uL)
#define UVCS_REG_SRSTCLR	(0xE6150944uL)
#define UVCS_PRR_MASK_LSI	(0x00007F00uL)
#define UVCS_PRR_MASK_VER	(0x000000F0uL)
#define UVCS_PRR_LSI_H2		(0x00004500uL)
#define UVCS_PRR_LSI_M2W	(0x00004700uL)
#define UVCS_PRR_LSI_M2N	(0x00004B00uL)
#define UVCS_PRR_LSI_V2H	(0x00004A00uL)
#define UVCS_PRR_LSI_E2		(0x00004C00uL)

#define UVCS_LSITYPE_H2_V1	(0)
#define UVCS_LSITYPE_H2_VX	(1)
#define UVCS_LSITYPE_M2W_V1	(2)
#define UVCS_LSITYPE_M2W_VX	(3)
#define UVCS_LSITYPE_M2N	(4)
#define UVCS_LSITYPE_E2		(5)
#if 0 /* TBD */
  #define UVCS_LSITYPE_V2H	(6)
#endif

#define UVCS_LSIH2_HW_NUM	(2uL)
#define UVCS_LSIM2_HW_NUM	(1uL)
#define UVCS_LSIE2_HW_NUM	(1uL)

#define UVCS_IPOPT_DEFAULT	(0x0002000AuL)
#define UVCS_IPOPT_H2V1_MASK	(0xFFFFFFFEuL)
#define UVCS_IPOPT_H2V1_FIX	(0x00000001uL)

/* register size */
#define UVCS_REG_SIZE_SINGLE	(0x4uL)
#define UVCS_REG_SIZE_VLC	(0x200uL)
#define UVCS_REG_SIZE_CE	(0x200uL)
#define UVCS_REG_SIZE_VPC	(0x90uL)

#define UVCS_REQ_NONE		(0x00000000uL)
#define UVCS_REQ_USED		(0x00000001uL)
#define UVCS_REQ_END		(0x80000000uL)
#define UVCS_TIMEOUT_DEFAULT	(HZ*10uL)
#define UVCS_DEBUG_BUFF_SIZE	(0x12C00uL)

#define UVCS_CLOSE_WAIT_MAX	(10uL)	/* timeout = n * CLOSE_WAIT_TIME */
#define UVCS_CLOSE_WAIT_TIME	(10uL)	/* msec time (sleep wait) */
#define UVCS_VPC_WAIT_MAX	(100uL)	/* timeout = n * VPC_WAIT_TIME */
#define UVCS_VPC_WAIT_TIME	(1uL)	/* usec delay time (busy-wait) */
#define UVCS_VPCREG_VPCCTL	(0x04uL)
#define UVCS_VPCREG_VPCSTS	(0x08uL)
#define UVCS_VPCREG_VPCCFG	(0x78uL)
#define UVCS_VCPREG_IRQENB	(0x10uL)
#define UVCS_BAAIDX_STRIDE	(0x59uL)

#define UVCS_VPCCFG_MODE	(0x00uL)

/******************************************************************************/
/*                    structures                                              */
/******************************************************************************/
struct uvcs_req_ctrl {
	UVCS_CMN_HW_PROC_T	 data;
	ulong			 state;
	ulong			 time;
};

struct uvcs_hdl_info {
	ulong			 id;	/* handle serial */
	struct uvcs_drv_info	*local;
	wait_queue_head_t	 waitq;
	struct semaphore	 sem;
	rwlock_t		 rwlock;
	UVCS_CMN_HANDLE		 uvcs_hdl;
	void			*uvcs_hdl_work;
	struct uvcs_req_ctrl	 req_data[UVCS_CMN_PROC_REQ_MAX];
};

struct uvcs_thr_ctrl {
	struct task_struct	*thread;
	wait_queue_head_t	 evt_wait_q;
	bool			 evt_req;
};

struct uvcs_mdl_param {
	ulong			 lsi_type;
	ulong			 hw_num;
};

struct uvcs_drv_info {
	struct platform_device	*pdev;
	struct cdev		 cdev;
	struct class		*pcls;

	ulong			 hdl_serial_num;
	struct uvcs_mdl_param	 module_param;
	struct uvcs_thr_ctrl	 thread_ctrl;
	struct uvcs_hdl_info	*hdl_lst_head;
	struct semaphore	 sem;
	ulong			 counter;
	struct semaphore	 uvcs_sem;
	UVCS_U32		 uvcs_hdl_work_req_size;
	UVCS_U32		 uvcs_lib_work_req_size;
	UVCS_CMN_INIT_PARAM_T	 uvcs_init_param;
	UVCS_CMN_LIB_INFO	 uvcs_info;
	UVCS_CMN_IP_INFO_T	 ip_info;
};

/******************************************************************************/
/*                    functions                                               */
/******************************************************************************/
int		 uvcs_io_init(struct uvcs_drv_info *local);
void		 uvcs_io_cleanup(struct uvcs_drv_info *local);
void		 uvcs_hw_processing_done(UVCS_PTR, UVCS_PTR,
			UVCS_CMN_HANDLE, UVCS_CMN_HW_PROC_T*);
UVCS_BOOL	 uvcs_semaphore_lock(UVCS_PTR);
void		 uvcs_semaphore_unlock(UVCS_PTR);
UVCS_BOOL	 uvcs_semaphore_create(UVCS_PTR);
void		 uvcs_semaphore_destroy(UVCS_PTR);
void		 uvcs_thread_event(UVCS_PTR);
UVCS_BOOL	 uvcs_thread_create(UVCS_PTR);
void		 uvcs_thread_destroy(UVCS_PTR);

#endif /* UVCS_LKM_INTERNAL_H */
