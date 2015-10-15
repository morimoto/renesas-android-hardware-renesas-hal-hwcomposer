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
#include <linux/ioctl.h>
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
#include "mmngr_public.h"
#include "fdpm_public.h"

int fdpm_ioctl_open_func(unsigned long arg,
			 struct fdpm_private *fdpm_private_data)
{
	int ret = 0;
	int dev_no;
	int first_get;
	int stl_msk_byte;
	int i = 0;
	unsigned long phy_addr;
	unsigned long hard_addr;
	unsigned long stlmsk_vaddr;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	int ercd;

	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_open_req req;
	struct fdpm_open_rsp rsp;
	struct fdpm_resource_table *entry_table = NULL;

	DPRINT("called\n");

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));
	if (copy_from_user(&ioc_cmd, (void __user *)arg, sizeof(ioc_cmd))) {
		DPRINT("copy_from_user arg fail\n");
		ret = -EFAULT;
		goto exit;
	}
	if (copy_from_user(&req, (void __user *)ioc_cmd.req, sizeof(req))) {
		DPRINT("copy_from_user ioc_cmd.req fail\n");
		ret = -EFAULT;
		goto exit;
	}

	/* fdpm resource get */
	DPRINT("vmode:%d\n", req.open_par.vmode);
	/* FDP H/W resource get: return value=0 success, dev_no = device no */
	down(&fdpm_ddata->resource_sem);
	if (fdpm_resource_get(&req.open_par,
			      &dev_no,
			      &first_get,
			      &fdpm_ddata->fdpm_independ.fdpm_ipdata)) {
		ret = -EFAULT;
		goto exit;
	}
	/* FDP H/W resource get successfull */
	/* FDP resource table update */
	if (fdpm_table_entry(&req.open_par,
			     dev_no,
			     &fdpm_private_data->open_id,
			     &fdpm_ddata->fdpm_independ) == -1) {
		ret = -EFAULT;
		goto exit;
	}
	if (fdpm_resource_search_source_list(fdpm_private_data->open_id,
					     0,
				     &fdpm_ddata->fdpm_independ.fdpm_itable,
					     &entry_table) == -1) {
		ret = -EFAULT;
		goto exit;
	}

	/* memory allocate for stil mask */
	stl_msk_byte = 2 *
		((req.open_par.insize.width + 7)>>3) *
		req.open_par.insize.height;
	DPRINT("still mask area = %d bytes\n", stl_msk_byte);
	for (i = 0; i < 2; i++) {
		ercd = mmngr_alloc_in_kernel(stl_msk_byte,
					     &phy_addr,
					     &hard_addr,
					     &stlmsk_vaddr,
					     MM_KERNELHEAP);
		if (ercd != 0) {
			DPRINT("still mask table %d fail\n", i);
			ret = -EFAULT;
			goto exit;
		}
		entry_table->stlmsk_table.stlmsk_adr[i] =
			(unsigned char *)hard_addr;
		entry_table->stlmsk_table.stlmsk_vadr[i] =
			(unsigned char *)stlmsk_vaddr;
		entry_table->stlmsk_table.stl_msk_byte =
			stl_msk_byte;
		entry_table->stlmsk_table.phy_addr[i] =
			(unsigned char *)phy_addr;
	}
	up(&fdpm_ddata->resource_sem);
	/* response source_id */
	rsp.open_id = entry_table->source_id;
	rsp.timer_ch = entry_table->timer_ch;
	if (copy_to_user((void __user *)ioc_cmd.rsp, &rsp, sizeof(rsp))) {
		DPRINT("copy_to_user fail\n");
		ret = -EFAULT;
		goto exit;
	}
	DPRINT("done\n");
DPRINT("FDPM open_id:%ld vmode:%d ocmode:%d dev_no:%d size:%ld(%d x %d)\n",
	       fdpm_private_data->open_id,
	       req.open_par.vmode,
	       req.open_par.ocmode,
	       dev_no,
	       entry_table->size,
	       req.open_par.insize.width,
	       req.open_par.insize.height);
	return 0;
exit:
	up(&fdpm_ddata->resource_sem);
	return ret;
}

int fdpm_ioctl_close_func(unsigned long arg,
			  struct fdpm_private *fdpm_private_data)
{
	int ret;
	int hit;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_close_req req;
	struct fdpm_close_rsp rsp;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_resource_table *target_table = NULL;

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));
	if (copy_from_user(&ioc_cmd,
			   (void __user *)arg,
			   sizeof(ioc_cmd))) {
		ret = -EFAULT;
		goto exit;
	}
	if (copy_from_user(&req,
			   (void __user *)ioc_cmd.req,
			   sizeof(struct fdpm_close_req))) {
		ret = -EFAULT;
		goto exit;
	}
	/* search sourcd id in resouce table, if hit, release resouce.
	   if not hit, return error */
	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(fdpm_private_data->open_id,
					       0,
				       &fdpm_ddata->fdpm_independ.fdpm_itable,
					       &target_table);
	up(&fdpm_ddata->resource_sem);
	if (hit == -1) {
		ret = -EFAULT;
		goto exit;
	}

	ret = 1;
	down(&fdpm_ddata->resource_sem);
	ret = fdpm_request_entry_check(target_table,
				       fdpm_private_data->open_id,
				       fdpm_pdata);
	up(&fdpm_ddata->resource_sem);
	if ((req.f_release != 1) && (ret == 1)) {/* detect entry queue */
		DPRINTA("detect entry queue\n");
		ret = -EACCES;
		goto exit;
	}
	ret = 1;
	down(&fdpm_ddata->resource_sem);
	ret = fdpm_post_entry_check(target_table,
				    fdpm_private_data->open_id,
				    fdpm_pdata);
	up(&fdpm_ddata->resource_sem);
	if ((req.f_release != 1) && (ret == 1)) {/* detect post queue */
		DPRINTA("detect post queue\n");
		ret = -EACCES;
		goto exit;
	}
	/* ioctl wait func exit */
	wait_for_completion_interruptible(&fdpm_private_data->comp2);
	fdpm_private_data->comp2_flag = 1;
	complete(&fdpm_private_data->comp);
	wait_for_completion_interruptible(&fdpm_private_data->comp2);
	fdpm_private_data->comp2_flag = 0;
	/* fdpm resource release */
	down(&fdpm_ddata->resource_sem);
	fdpm_resource_release(target_table->ocmode,
			      target_table->vintmode,
			      target_table->device_no,
			      target_table->size,
			      &fdpm_ddata->fdpm_independ.fdpm_ipdata);
	up(&fdpm_ddata->resource_sem);

	/* still mask area release */
	mmngr_free_in_kernel(target_table->stlmsk_table.stl_msk_byte,
		     (unsigned long)target_table->stlmsk_table.phy_addr[0],
		     (unsigned long)target_table->stlmsk_table.stlmsk_adr[0],
		     (unsigned long)target_table->stlmsk_table.stlmsk_vadr[0],
			     MM_KERNELHEAP);
	mmngr_free_in_kernel(target_table->stlmsk_table.stl_msk_byte,
		     (unsigned long)target_table->stlmsk_table.phy_addr[1],
		     (unsigned long)target_table->stlmsk_table.stlmsk_adr[1],
		     (unsigned long)target_table->stlmsk_table.stlmsk_vadr[1],
			     MM_KERNELHEAP);
	down(&fdpm_ddata->resource_sem);
	ret = fdpm_table_release(fdpm_private_data->open_id,
				 &fdpm_ddata->fdpm_independ);
	DPRINT("table_release %ld ret:%d\n", fdpm_private_data->open_id, ret);
	up(&fdpm_ddata->resource_sem);
	if (ret)
		DPRINT("Can not find release table\n");
exit:
	return ret;
}

int fdpm_ioctl_start_func(unsigned long arg,
			  struct fdpm_private *fdpm_private_data)
{
	int ret = 0;
	int hit;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_start_req *req;
	struct fdpm_start_rsp *rsp;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_resource_table *target_table = NULL;

	DPRINT("called\n");

	req = kmalloc(sizeof(struct fdpm_start_req), GFP_KERNEL);
	if (req == NULL)
		return -EFAULT;

	rsp = kmalloc(sizeof(struct fdpm_start_rsp), GFP_KERNEL);
	if (rsp == NULL) {
		kfree(req);
		return -EFAULT;
	}

	memset(rsp, 0, sizeof(struct fdpm_start_rsp));

	if (copy_from_user(&ioc_cmd,
			   (void __user *)arg,
			   sizeof(ioc_cmd))) {
		ret = -EFAULT;
		goto exit;
	}
	if (copy_from_user(req,
			   (void __user *)ioc_cmd.req,
			   sizeof(struct fdpm_start_req))) {
		ret = -EFAULT;
		goto exit;
	}

	/* get table index from start_id */
	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(fdpm_private_data->open_id,
					       0,
					       &fdpm_ddata->
					       fdpm_independ.fdpm_itable,
					       &target_table);
	if (hit == -1) {
		DPRINTA("Start:Cannot find source_id source_id:%ld\n",
			fdpm_private_data->open_id);
		ret = -EFAULT;
		up(&fdpm_ddata->resource_sem);
		goto exit;
	}
	if (req->start_par.fproc_par.seq_par_flg == 1)
		DPRINT("input size width:%d height:%d\n",
		       req->start_par.fproc_par.seq_par.in_width,
		       req->start_par.fproc_par.seq_par.in_height);

	/* overwrite for sequence control */
	fdpm_seq_control(&req->start_par, target_table);
	fdpm_pd_control(&req->start_par, target_table);
	fdpm_update_picpar(target_table, &req->start_par.fproc_par);

	/* register start request queue */
	ret = fdpm_request_entry(target_table,
				 req,
				 fdpm_pdata,
				 fdpm_private_data);
	if (ret == -ERESTARTSYS)
		goto exit;

	/* status update */
	fdpm_status_update(&req->start_par,
			   &target_table->seq_status,
			   &fdpm_ddata->fdpm_hw_status[target_table->device_no],
			   &target_table->status,
			   &fdpm_ddata->fdpm_independ.fdpm_ipdata,
			   &fdpm_pdata->FDP_obj[target_table->device_no]->
			   fdp_independ);
	fdpm_fdp_cb1_update(&req->start_par,
			    &rsp->fdp_cb1,
			    &fdpm_pdata->FDP_obj[target_table->device_no]->
			    fdp_independ,
			    target_table->stlmsk_table.stlmsk_flg);

	/* response entry_id */
	rsp->start_fid = fdpm_ddata->entry_id;

	up(&fdpm_ddata->resource_sem);
	if (copy_to_user((void __user *)ioc_cmd.rsp,
			 rsp,
			 sizeof(struct fdpm_start_rsp))) {
		ret = -EFAULT;
		goto exit;
	}
exit:
	kfree(req);
	kfree(rsp);
	return ret;
}

int fdpm_ioctl_cancel_func(unsigned long arg,
			   struct fdpm_private *fdpm_private_data)
{
	int ret;
	int hit;
	int device_ino;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_cancel_req req;
	struct fdpm_cancel_rsp rsp;
	struct list_head *listptr;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	fdpm_queue_entry *entryptr;
	struct fdpm_resource_table *target_table = NULL;

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));
	if (copy_from_user(&ioc_cmd,
			   (void __user *)arg,
			   sizeof(ioc_cmd))) {
		ret = -EFAULT;
		goto exit;
	}
	if (copy_from_user(&req,
			   (void __user *)ioc_cmd.req,
			   sizeof(req))) {
		ret = -EFAULT;
		goto exit;
	}
	/* get table index based on cancel_id */
	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(fdpm_private_data->open_id,
					       0,
				       &fdpm_ddata->fdpm_independ.fdpm_itable,
					       &target_table);
	up(&fdpm_ddata->resource_sem);
	if (hit == -1) {
		ret = 0;
		rsp.rtcd = E_FDP_CANCEL_NOID;
		goto exit2;
	}
	/* search start request queue */
	device_ino = target_table->device_no;
	hit = -1;
	down(&fdpm_ddata->entry_sem);
	list_for_each(listptr,
		      &fdpm_ddata->fdpm_ientry[device_ino].list) {
		entryptr = (fdpm_queue_entry *)list_entry(listptr,
							  fdpm_queue_entry,
							  list);
		if (req.cancel_fid == entryptr->entry_id) {
			/* if detect request queue, delete entry */
			fdpm_queue_del_entry(entryptr);
			hit = 1;
			break;
		}
	}
	list_for_each(listptr, &fdpm_ddata->fdpm_ientryw[device_ino].list) {
		entryptr = (fdpm_queue_entry *)list_entry(listptr,
							  fdpm_queue_entry,
							  list);
		if (req.cancel_fid == entryptr->entry_id) {
			/* if decetect wait queue, delete entry */
			fdpm_queue_del_entry(entryptr);
			hit = 1;
			break;
		}
	}
	up(&fdpm_ddata->entry_sem);
	/* if can not detect request queue, notify rtcd */
	if (hit == -1)
		rsp.rtcd = E_FDP_CANCEL_NOID;

exit2:
	if (copy_to_user((void __user *)ioc_cmd.rsp,
			 &rsp,
			 sizeof(rsp))) {
		ret = -EFAULT;
		goto exit;
	}
	return 0;
exit:
	return ret;
}

int fdpm_ioctl_status_func(unsigned long arg,
			   struct fdpm_private *fdpm_private_data)
{
	int rtn;
	int hit;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_status_req req;
	struct fdpm_status_rsp rsp;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_resource_table *target_table = NULL;

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	if (copy_from_user(&ioc_cmd,
			   (void __user *)arg,
			   sizeof(ioc_cmd))) {
		rtn = -EFAULT;
		goto exit;
	}
	if (copy_from_user(&req,
			   (void __user *)ioc_cmd.req,
			   sizeof(req))) {
		rtn = -EFAULT;
		goto exit;
	}
	/* get table index besed on status_id */
	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(fdpm_private_data->open_id,
					       0,
				       &fdpm_ddata->fdpm_independ.fdpm_itable,
					       &target_table);
	up(&fdpm_ddata->resource_sem);
	if (hit == -1) {
		rtn = -EFAULT;
		goto exit;
	}
	rsp.status_id = req.status_id;
	down(&fdpm_ddata->resource_sem);
	target_table->status.vcycle =
		fdpm_pdata->FDP_obj[target_table->device_no]->
		fdp_independ.fdp_state.vcycle;
	memcpy(&rsp.status,
	       &target_table->status,
	       sizeof(T_FDP_STATUS));
	up(&fdpm_ddata->resource_sem);
	/* return status information */
	if (copy_to_user((void __user *)ioc_cmd.rsp,
			 &rsp,
			 sizeof(rsp))) {
		rtn = -EFAULT;
		goto exit;
	}
	return 0;
exit:
	return rtn;
}

int fdpm_ioctl_wait_func(unsigned long arg,
			 struct fdpm_private *fdpm_private_data)
{
	int rtn;
	int hit;
	int device_no;
	int empty_flg;
	int entry_flg = 0;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_wait_req req;
	struct fdpm_cb_rsp   *rsp;
	struct list_head *listptr;
	fdpm_queue_entry *entry;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_resource_table *target_table = NULL;
	int result = 0;

	fdpm_private_data->wait_func_flag = 1;

	rtn = 0;
	empty_flg = 0;
	listptr = 0;
	entry = 0;

	rsp = kmalloc(sizeof(struct fdpm_cb_rsp), GFP_KERNEL);
	if (rsp == NULL)
		return -EFAULT;

	memset(&req, 0, sizeof(req));
	memset(rsp, 0, sizeof(struct fdpm_cb_rsp));


	if (copy_from_user(&ioc_cmd,
			   (void __user *)arg,
			   sizeof(ioc_cmd))) {
		DPRINT("copy_from_user arg failed\n");
		rtn = -EFAULT;
		goto exit;
	}

	if (copy_from_user(&req,
			   ioc_cmd.req,
			   sizeof(struct fdpm_wait_req))) {
		DPRINT("copy_from_user ioc_cmd.req");
		rtn = -EFAULT;
		goto exit;
	}
	rsp->close_flag = req.close_flag;
	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(fdpm_private_data->open_id,
					       0,
				       &fdpm_ddata->fdpm_independ.fdpm_itable,
					       &target_table);
	up(&fdpm_ddata->resource_sem);
	if (hit == -1) {
		DPRINTA("Wait:Cannot find source_id source_id:%ld\n",
			fdpm_private_data->open_id);
		rtn = -EFAULT;
		goto exit;
	}
	device_no = target_table->device_no;
	if ((req.vintmode == FDP_VMODE_VBEST) ||
	    (req.vintmode == FDP_VMODE_VBEST_FDP0) ||
	    (req.vintmode == FDP_VMODE_VBEST_FDP1) ||
	    (req.vintmode == FDP_VMODE_VBEST_FDP2)) {
		if (down_interruptible(&fdpm_ddata->wait_sem)) {
			kfree(rsp);
			return -ERESTARTSYS;
		}
		listptr = &fdpm_ddata->fdpm_ientryw[device_no].list;
		if (list_empty(&fdpm_ddata->fdpm_ientryw[device_no].list)) {
			rsp->end = -1;
			up(&fdpm_ddata->wait_sem);
			if (req.close_flag == 1) {
				complete(&fdpm_private_data->comp2);
				do {
					result = wait_for_completion_interruptible(
					&fdpm_private_data->comp);
				} while (result);
			}
		} else {
			list_for_each(listptr,
			      &fdpm_ddata->fdpm_ientryw[device_no].list) {
				entry = (fdpm_queue_entry *)list_entry(listptr,
							       fdpm_queue_entry,
								       list);
				if (entry->source_id == req.wait_id) {
					entry_flg = 1;
					break;
				}
			}
			up(&fdpm_ddata->wait_sem);
			if (entry_flg == 0) {
				rsp->end = -1;
				if (req.close_flag == 1) {
					complete(&fdpm_private_data->comp2);
					do {
						result = wait_for_completion_interruptible(
						&fdpm_private_data->comp);
					} while (result);
				}
			} else {
				/* regist wait thread queue */
				do {
					result = wait_for_completion_interruptible(
					&fdpm_private_data->comp_for_wait);
				} while (result);
				memcpy(rsp,
			       &fdpm_pdata->post_thread[device_no].cb_rsp,
				       sizeof(struct fdpm_cb_rsp));
				complete(
				&fdpm_pdata->post_thread[device_no].start2);
				if (down_interruptible(&fdpm_ddata->wait_sem)) {
					kfree(rsp);
					return -ERESTARTSYS;
				}
				if (!list_empty(
				&fdpm_ddata->fdpm_ientryw[device_no].list))
					fdpm_queue_del_entry(entry);
				else
					rsp->end = 1;
				up(&fdpm_ddata->wait_sem);
			}
		}
		if (fdpm_private_data->comp2_flag == 1)
			complete(&fdpm_private_data->comp2);
	} else {
		if (down_interruptible(&fdpm_ddata->wait_sem)) {
			kfree(rsp);
			return -ERESTARTSYS;
		}
		list_for_each(listptr,
			      &fdpm_ddata->fdpm_ientryw[device_no].list) {
			entry = (fdpm_queue_entry *)list_entry(listptr,
							       fdpm_queue_entry,
							       list);
			if (entry->source_id == req.wait_id) {
				entry_flg = 1;
				break;
			}
		}
		up(&fdpm_ddata->wait_sem);
		if (entry_flg == 0) {
			rsp->end = -1;
		} else {
			/* regist wait thread queue */
			do {
				result = wait_for_completion_interruptible(
				&fdpm_private_data->comp_for_wait);
			} while (result);
			memcpy(rsp,
			       &fdpm_pdata->post_thread[device_no].cb_rsp,
			       sizeof(struct fdpm_cb_rsp));
			complete(&fdpm_pdata->post_thread[device_no].start2);
			if (entry->source_id == fdpm_private_data->open_id) {
				if (down_interruptible(&fdpm_ddata->wait_sem)) {
					kfree(rsp);
					return -ERESTARTSYS;
				}
				fdpm_queue_del_entry(entry);
				up(&fdpm_ddata->wait_sem);
				rsp->end = 1;
			} else {
				rsp->end = 1;
			}
		}
		if (req.close_flag == 1) {
			complete(&fdpm_private_data->comp2);
			do {
				result = wait_for_completion_interruptible(
				&fdpm_private_data->comp);
			} while (result);
			if (fdpm_private_data->comp2_flag == 1)
				complete(&fdpm_private_data->comp2);
		}
	}
	fdpm_private_data->wait_func_flag = 0;
	if (fdpm_private_data->comp2_flag_from_sub == 0) {
		if (copy_to_user((void __user *)ioc_cmd.rsp,
				 rsp,
				 sizeof(struct fdpm_cb_rsp))) {
			DPRINT("copy_to_user ioc_cmd.rsp fail\n");
			rtn = -EFAULT;
			goto exit;
		}
	}
	kfree(rsp);
	return 0;
exit:
	kfree(rsp);
	return rtn;
}

int fdpm_ioctl_reg_status_func(unsigned long arg,
			       struct fdpm_private *fdpm_private_data)
{
	int rtn;
	int hit;
	struct fdpm_ioc_cmd ioc_cmd;
	struct fdpm_status_req req;
	struct fdpm_status_reg_rsp *rsp;
	struct fdpm_privdata *fdpm_pdata =
		(struct fdpm_privdata *)fdpm_private_data->fdpm_handle;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct fdpm_resource_table *target_table = NULL;

	rsp = kmalloc(sizeof(struct fdpm_status_reg_rsp), GFP_KERNEL);
	if (rsp == NULL)
		return -EFAULT;

	memset(&req, 0, sizeof(req));
	memset(rsp, 0, sizeof(struct fdpm_status_reg_rsp));

	if (copy_from_user(&ioc_cmd,
			   (void __user *)arg,
			   sizeof(ioc_cmd))) {
		rtn = -EFAULT;
		goto exit;
	}
	if (copy_from_user(&req,
			   (void __user *)ioc_cmd.req,
			   sizeof(req))) {
		rtn = -EFAULT;
		goto exit;
	}
	/* get table index besed on status_id */
	down(&fdpm_ddata->resource_sem);
	hit = fdpm_resource_search_source_list(fdpm_private_data->open_id,
					       0,
				       &fdpm_ddata->fdpm_independ.fdpm_itable,
					       &target_table);
	up(&fdpm_ddata->resource_sem);
	if (hit == -1) {
		rtn = -EFAULT;
		goto exit;
	}
	rsp->status_id = req.status_id;
	down(&fdpm_ddata->resource_sem);
	up(&fdpm_ddata->resource_sem);
	/* return status information */
	if (copy_to_user((void __user *)ioc_cmd.rsp,
			 rsp,
			 sizeof(struct fdpm_status_reg_rsp))) {
		rtn = -EFAULT;
		goto exit;
	}
	kfree(rsp);
	return 0;
exit:
	kfree(rsp);
	return rtn;
}


long fdpm_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct fdpm_private *fdpm_pdata = fp->private_data;

	switch (cmd) {
	case FDPM_IOCTL_OPEN:/* open */
		ret = fdpm_ioctl_open_func(arg, fdpm_pdata);
		break;
	case FDPM_IOCTL_CLOSE:/* close */
		ret = fdpm_ioctl_close_func(arg, fdpm_pdata);
		break;
	case FDPM_IOCTL_START:/* start */
		ret = fdpm_ioctl_start_func(arg, fdpm_pdata);
		break;
	case FDPM_IOCTL_CANCEL:/* cancel */
		ret = fdpm_ioctl_cancel_func(arg, fdpm_pdata);
		break;
	case FDPM_IOCTL_STATUS:/* status */
		ret = fdpm_ioctl_status_func(arg, fdpm_pdata);
		break;
	case FDPM_IOCTL_WAIT:/* wait */
		ret = fdpm_ioctl_wait_func(arg, fdpm_pdata);
		break;
	case FDPM_IOCTL_REG_STATUS:
		ret = fdpm_ioctl_reg_status_func(arg, fdpm_pdata);
		break;
	default:
		ret = -ENOTTY;
		break;
	}
	DPRINT("done\n");
	return ret;
}
