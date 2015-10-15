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

#ifndef __FDPM_MAIN_H__
#define __FDPM_MAIN_H__

#include "fdpm_def.h"

typedef struct {
	unsigned int entry_id;/* increment value of recieve start order */
	unsigned int source_id;/* accpect number */
	struct completion *comp_for_wait;
	struct fdpm_start_req req;
	int decode_val;
	int decode_count;
	unsigned char timer_ch;
	struct list_head list;
} fdpm_queue_entry;

typedef struct {
	unsigned int entry_id;
	unsigned int source_id;
	struct completion *comp_for_wait;
	unsigned char timer_ch;
	unsigned char fdp_flag;
	unsigned char timer_flag;
	struct fdpm_cb_rsp cb_rsp;
	struct list_head list;
} fdpm_wait_entry;

typedef struct {
	struct task_struct *th;
	struct completion sync;
	struct completion start;
	struct completion start2;
	struct completion end_fdp;
	struct completion *comp;
	struct fdpm_drvdata *pdrv;
	unsigned char quit_flag;
	T_FDPD_MEM *FDP_obj;
	fdpm_wait_entry *fdpm_wentry;
	struct fdpm_cb_rsp cb_rsp;
} fdpm_post_th;

typedef struct {
	struct task_struct *th;
	struct completion sync;
	struct completion start;
	struct fdpm_drvdata *pdrv;
	T_FDPD_MEM *FDP_obj;
	fdpm_queue_entry *fdpm_ientry;
	fdpm_post_th *post_thread;
} fdpm_set_th;


typedef struct {
	struct task_struct *th;
	struct completion sync;
	struct completion start;
	struct fdpm_drvdata *pdrv;
	T_FDPD_MEM *FDP_obj;
} fdpm_wait_th;

struct fdpm_drvdata {
	struct platform_device *pdev;
	struct class *pcls;
	struct cdev cdev;
	struct task_struct *task;
	struct clk *fdpm_clk[FDPM_FDP_NUM];
	struct {
		void __iomem *mapbase;
		int irq;
		void *cb;
	} fdp;
	atomic_t counter;
	spinlock_t lock;
	struct semaphore resource_sem;
	struct semaphore init_sem;
	struct semaphore entry_sem;
	struct semaphore wait_sem;
	struct semaphore post_sem;
	struct semaphore wait_sem2;
	struct completion comp[FDPM_FDP_NUM];
	unsigned char comp2_flag[FDPM_FDP_NUM];
	unsigned int entry_id;
	T_FDP_HW_STATUS fdpm_hw_status[FDPM_FDP_NUM];
	struct fdpm_independ fdpm_independ;
	fdpm_queue_entry fdpm_ientry[FDPM_FDP_NUM];/* list structure */
	fdpm_queue_entry fdpm_ientryw[FDPM_FDP_NUM];/* list structure */
	fdpm_wait_entry fdpm_iwentry[FDPM_FDP_NUM];/* list structure */
	unsigned int temp[4];
};

struct fdpm_privdata {
	struct fdpm_drvdata *pdrv;
	T_FDPD_MEM *FDP_obj[FDPM_FDP_NUM];
	spinlock_t lock;
	fdpm_set_th set_thread[FDPM_FDP_NUM];
	fdpm_post_th post_thread[FDPM_FDP_NUM];
};

struct fdpm_private {
	long fdpm_handle;
	unsigned long open_id;
	struct completion comp;
	struct completion comp2;
	struct completion comp_for_wait;
	unsigned char comp2_flag;
	unsigned char comp2_flag_from_sub;
	unsigned char wait_func_flag;
};
#endif
