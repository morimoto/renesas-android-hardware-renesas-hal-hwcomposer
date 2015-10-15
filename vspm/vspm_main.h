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

#ifndef __VSPM_MAIN_H__
#define __VSPM_MAIN_H__

#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/clk.h>

#include "vspm_public.h"
#include "vspm_private.h"
#include "vspm_if.h"

#define CLSNAME				"vspm"
#define DEVNAME				"vspm"
#define THREADNAME			"vspm"
#define PROCNAME			"driver/vspm"
#define DEVNUM				1
#define DRVNAME				DEVNAME
#define RESNAME				DEVNAME

enum {
	RESOURCE_TDDMAC,
	RESOURCE_VSPR,
	RESOURCE_VSPS,
	RESOURCE_VSPD0,
	RESOURCE_VSPD1
};

/* vspm driver data structure */
struct vspm_drvdata {
	struct platform_device *pdev;
	struct class *pcls;
	struct cdev cdev;
	struct clk *vsps_clk;
#ifdef USE_VSPR
	struct clk *vspr_clk;
#endif
#ifdef USE_VSPD0
	struct clk *vspd0_clk;
#endif
#ifdef USE_VSPD1
	struct clk *vspd1_clk;
#endif
	struct clk *tddmac_clk;
	struct task_struct *task;
	struct {
		void __iomem *mapbase;
		int irq;
		void *cb;
	} tddmac;
	struct {
		void __iomem *mapbase;
		int irq;
		void *cb;
	} vsp;
	atomic_t counter;
	struct semaphore init_sem;
};

/* ip parameter structure */
struct vspm_entry_ip_par {
	union {
		struct vspm_entry_vsp {
			/* parameter to VSP processing */
			VSPM_VSP_PAR par;
			/* input image settings */
			struct vspm_entry_vsp_in {
				T_VSP_IN in;
				T_VSP_OSDLUT osdlut;
				T_VSP_ALPHA alpha;
				T_VSP_CLRCNV clrcnv;
			} in[4];
			/* output image settings */
			struct vspm_entry_vsp_out {
				T_VSP_OUT out;
			} out;
			/* conversion processing settings */
			struct vspm_entry_vsp_ctrl {
				T_VSP_CTRL ctrl;
				struct vspm_entry_vsp_bru {
					T_VSP_BRU bru;
					T_VSP_BLEND_VIRTUAL blend_virtual;
					T_VSP_BLEND_CONTROL blend_control_a;
					T_VSP_BLEND_CONTROL blend_control_b;
					T_VSP_BLEND_CONTROL blend_control_c;
					T_VSP_BLEND_CONTROL blend_control_d;
					T_VSP_BLEND_ROP blend_rop;
				} bru;
				T_VSP_SRU sru;
				T_VSP_UDS uds;
				T_VSP_UDS uds1;
				T_VSP_UDS uds2;
				T_VSP_LUT lut;
				T_VSP_CLU clu;
				T_VSP_HST hst;
				T_VSP_HSI hsi;
				struct vspm_entry_vsp_hgo {
					T_VSP_HGO hgo;	 /* HGO parameter */
					void *user_addr; /* user area address */
				} hgo;
				struct vspm_entry_vsp_hgt {
					T_VSP_HGT hgt;	 /* HGT parameter */
					void *user_addr; /* user area address */
				} hgt;
			} ctrl;
		} vsp;
		struct vspm_entry_tddmac {
			/* parameter to 2DDMAC processing */
			VSPM_2DDMAC_PAR par;
			/* request mode settings */
			struct vspm_entry_tddmac_mode {
				T_TDDMAC_MODE mode;
				T_TDDMAC_EXTEND extend;
			} mode;
			/* DMA transfer settings */
			T_TDDMAC_REQUEST request;
		} tddmac;
	} ip_param;
};

/* request data structure */
struct vspm_entry_req_data {
	struct list_head list;
	struct vspm_entry_req entry;
	struct vspm_entry_ip_par *ip_par;
	struct vspm_privdata *priv;
	pid_t pid;
};

/* response data structure */
struct vspm_cb_rsp_data {
	struct list_head list;
	struct vspm_cb_rsp rsp;
	struct {
		void *user_addr;
		void *knel_addr;
	} vsp_hgo;
	struct vspm_cb_vsp_hgt_rsp {
		void *user_addr;
		void *knel_addr;
	} vsp_hgt;
};

/* vspm device file private data structure */
struct vspm_privdata {
	struct vspm_drvdata *pdrv;
	spinlock_t lock;
	struct completion comp;
	struct completion cb_start_comp;
	struct task_struct *cb_thread;
	struct vspm_entry_req_data req_head;
	struct vspm_cb_rsp_data rsp_head;
};

extern struct vspm_drvdata *p_vspm_drvdata;

/* subroutines */
long vspm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int vspm_init(struct vspm_privdata *priv);
int vspm_quit(struct vspm_privdata *priv);
int vspm_cancel(struct vspm_privdata *priv);

#endif /* __VSPM_MAIN_H__ */
