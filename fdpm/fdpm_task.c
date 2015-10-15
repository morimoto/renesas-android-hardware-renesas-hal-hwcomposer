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
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/signal.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/fs.h>

#include "fdpm_api.h"
#include "fdp/fdp_depend.h"
#include "fdp/fdp_drv_l.h"
#include "fdpm_drv.h"
#include "fdpm_if_par.h"
#include "fdpm_if.h"
#include "include/fdpm_def.h"
#include "include/fdpm_depend.h"
#include "include/fdpm_main.h"
#include "include/fdpm_lfunc.h"
#include "include/fdpm_log.h"
#include "fdp/fdp_drv.h"

int fdpm_drv_set_thread(void *thread)
{
	int                     ret = 0;
	int                     hit = 0;
	T_FDP_R_START          *start_par;
	struct list_head        *listptr = 0;
	fdpm_queue_entry        *entry;
	fdpm_set_th             *priv = (fdpm_set_th *)thread;
	T_FDPD_MEM               *FDP_obj = priv->FDP_obj;
	struct fdpm_drvdata     *pdrv = priv->pdrv;
	fdpm_post_th            *post_thread = priv->post_thread;
	fdpm_queue_entry        *fdpm_tmp_entry;
	fdpm_wait_entry         *fdpm_wait_post_entry =
		post_thread->fdpm_wentry;
	struct fdpm_resource_table *target_table = NULL;
	int result = 0;

	start_par = kmalloc(sizeof(T_FDP_R_START), GFP_KERNEL);
	if (start_par == NULL)
		return -EFAULT;

	fdpm_tmp_entry = kmalloc(sizeof(fdpm_queue_entry), GFP_KERNEL);
	if (fdpm_tmp_entry == NULL) {
		kfree(start_par);
		return -EFAULT;
	}

	/* thread start notify */
	complete(&priv->sync);

	for (;;) {
		/* waiting entry queue */
		do {
			result = wait_for_completion_interruptible(
			&priv->start);
		} while (result);
		if (kthread_should_stop())
			break;
		DPRINT("set_thread wait release\n");
		if (down_interruptible(&pdrv->entry_sem)) {
			kfree(start_par);
			kfree(fdpm_tmp_entry);
			return -ERESTARTSYS;
		}
		listptr = &priv->fdpm_ientry->list;
		if (!list_empty(listptr)) {
			entry = (fdpm_queue_entry *)list_first_entry(listptr,
							fdpm_queue_entry,
								     list);
			memcpy(fdpm_tmp_entry,
			       entry,
			       sizeof(fdpm_queue_entry));
			memcpy(start_par,
			       &fdpm_tmp_entry->req.start_par,
			       sizeof(T_FDP_R_START));
			/* delete entry queue */
			fdpm_queue_del_entry(entry);
			up(&pdrv->entry_sem);
			/* register post queue */
			ret = fdpm_post_entry(fdpm_tmp_entry,
					      fdpm_wait_post_entry,
					      pdrv);
			if ((ret == -ERESTARTSYS) || (ret == -EFAULT)) {
				kfree(start_par);
				kfree(fdpm_tmp_entry);
				return ret;
			}
		} else {
			up(&pdrv->entry_sem);
		}

		hit = fdpm_resource_search_source_list(fdpm_tmp_entry->
						       source_id,
						       0,
						       &pdrv->
						fdpm_independ.fdpm_itable,
						       &target_table);
		if (hit == -1) {
			kfree(start_par);
			kfree(fdpm_tmp_entry);
			return -EFAULT;
		}
		/* overwrite for sequence control */
		FDP_obj->fdp_independ.decode_val = fdpm_tmp_entry->decode_val;
		FDP_obj->fdp_independ.decode_count =
			fdpm_tmp_entry->decode_count;
		fdpm_stlmsk_control(start_par,
				    &FDP_obj->fdp_independ,
				    target_table);
		fdpm_seq_write(start_par, target_table);
		/* internal callback set */
		FDP_obj->fdp_sub.userdata2 = (void *)post_thread;
		FDP_obj->fdp_sub.fdp_cb2 = (void (*)(void *))icb2;
		/* start */
		if (kthread_should_stop())
			break;
		ret = drv_FDP_Start(FDP_obj, start_par);
		if (ret != 0)
			DPRINT("drv_FDP_Start fail.%d\n", ret);
		if (start_par->fproc_par.out_buf_flg == 0)
			drv_FDP_finish_write(FDP_obj);
		do {
			result = wait_for_completion_interruptible(
			&post_thread->end_fdp);
		} while (result);
		if (kthread_should_stop())
			break;
	}
	kfree(start_par);
	kfree(fdpm_tmp_entry);
	return ret;
}

int fdpm_post_thread(void *thread)
{
	int ret = 0;
	int i;
	fdpm_post_th            *priv = (fdpm_post_th *)thread;
	struct fdpm_drvdata     *pdrv = priv->pdrv;
	T_FDPD_MEM              *FDP_obj = priv->FDP_obj;
	struct list_head        *listptr = 0;
	fdpm_wait_entry         *entry;
	struct completion       *comp = NULL;
	struct fdpm_resource_table *tmp_table = NULL;
	unsigned int            source_id = 0;
	int           search_result = 0;
	int result = 0;

	/* thread start notify */
	complete(&priv->sync);

	for (;;) {
		/* wait finish interrupt notify */
		DPRINT("post thread wait %x\n", (unsigned int)&priv->start);
		do {
			result = wait_for_completion_interruptible(
			&priv->start);
		} while (result);
		DPRINT("post thread wait release\n");
		if (priv->quit_flag == 1) {
			while (!kthread_should_stop()) {
				;
				;
			}
			break;
		}
		if (down_interruptible(&pdrv->post_sem))
			return -ERESTARTSYS;
		DPRINT("post_sem enter\n");

		listptr = &priv->fdpm_wentry->list;
		if (!list_empty(listptr)) {
			entry = (fdpm_wait_entry *)list_first_entry(listptr,
								fdpm_wait_entry,
								    list);
			source_id = entry->source_id;
			comp = entry->comp_for_wait;

			for (i = 0; i < 18; i++)
				entry->cb_rsp.sinfo.sensor_info[i] =
					FDP_obj->
					fdp_independ.fdp_state.sinfo[i];
			for (i = 0; i < 60; i++) {
				memcpy(&entry->cb_rsp.fdp_reg_status[i].name,
			&FDP_obj->fdp_independ.fdp_state.fdp_reg_status[i].name,
				       sizeof(char)*30);
				entry->cb_rsp.fdp_reg_status[i].reg_value =
			FDP_obj->
			fdp_independ.fdp_state.fdp_reg_status[i].reg_value;
			}
			/* delete post queue */
			memcpy(&priv->cb_rsp,
			       &entry->cb_rsp,
			       sizeof(struct fdpm_cb_rsp));
			pdrv->temp[0] = (unsigned int)listptr;
			pdrv->temp[1] = (unsigned int)listptr->next;
			pdrv->temp[2] = (unsigned int)&entry->list;

			fdpm_post_del_entry(entry);
			pdrv->temp[3] = (unsigned int)listptr->next;
		}
		/* notify ioctl wait */
		up(&pdrv->post_sem);
		if (down_interruptible(&pdrv->resource_sem))
			return -ERESTARTSYS;

		search_result = fdpm_resource_search_source_list(source_id,
								 0,
					&pdrv->fdpm_independ.fdpm_itable,
								 &tmp_table);
		up(&pdrv->resource_sem);
		if (search_result != -1) {
			if (comp != NULL) {
				complete(comp);
				do {
					result = wait_for_completion_interruptible(
					&priv->start2);
				} while (result);
			}
		}
		if (priv->quit_flag == 1) {
			while (!kthread_should_stop()) {
				;
				;
			}
			break;
		}
	}
	return ret;
}

int icb2(void *thread)
{
	fdpm_post_th *priv = (fdpm_post_th *)thread;
	complete(&priv->start);
	complete(&priv->end_fdp);
	return 0;
}
