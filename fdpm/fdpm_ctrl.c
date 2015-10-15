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

/* initalize fdpm queue entry */
int fdpm_request_entry_init(struct fdpm_drvdata *fdpm_pdata)
{
	int i;
	if (down_interruptible(&fdpm_pdata->entry_sem))
		return -ERESTARTSYS;

	for (i = 0; i < FDPM_FDP_NUM; i++) {
		memset(&fdpm_pdata->fdpm_ientry[i],
		       0,
		       sizeof(fdpm_queue_entry));
		INIT_LIST_HEAD(&fdpm_pdata->fdpm_ientry[i].list);
	}
	up(&fdpm_pdata->entry_sem);
	if (down_interruptible(&fdpm_pdata->wait_sem))
		return -ERESTARTSYS;
	for (i = 0; i < FDPM_FDP_NUM; i++) {
		memset(&fdpm_pdata->fdpm_ientryw[i],
		       0,
		       sizeof(fdpm_queue_entry));
		INIT_LIST_HEAD(&fdpm_pdata->fdpm_ientryw[i].list);
	}
	up(&fdpm_pdata->wait_sem);

	return 0;
}

/* entry fdpm queue */
int fdpm_request_entry(struct fdpm_resource_table *table,
		       struct fdpm_start_req *req,
		       struct fdpm_privdata *fdpm_pdata,
		       struct fdpm_private *fdpm_private_data)
{
	fdpm_queue_entry *ptr, *ptrw;
	int dev_no = table->device_no;
	unsigned long tmp_entry_id;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;

	if (down_interruptible(&fdpm_ddata->entry_sem))
		return -ERESTARTSYS;

	ptr = kmalloc(sizeof(fdpm_queue_entry), GFP_KERNEL);
	if (ptr) {
		ptr->entry_id = fdpm_ddata->entry_id;
		ptr->source_id = table->source_id;
		ptr->comp_for_wait = &fdpm_private_data->comp_for_wait;
		memcpy(&ptr->req, req, sizeof(struct fdpm_start_req));
		ptr->req.cb_rsp.cb_data.complete.wResult = table->source_id;
		ptr->req.cb_rsp.cb_data.complete.uwJobId = fdpm_ddata->entry_id;
		tmp_entry_id = fdpm_ddata->entry_id;
		fdpm_ddata->entry_id++;
		ptr->decode_val = table->seq_status.decode_val;
		ptr->decode_count = table->seq_status.decode_count;
		ptr->timer_ch = table->timer_ch;
		ptr->req.cb_rsp.telecine_mode = table->seq_status.telecine_mode;
		list_add_tail(&ptr->list,
			      &fdpm_ddata->fdpm_ientry[dev_no].list);
	} else {
		DPRINT("ERROR kmalloc for start\n");
		up(&fdpm_ddata->entry_sem);
		return -EFAULT;
	}
	if (down_interruptible(&fdpm_ddata->wait_sem)) {
		kfree(ptr);
		up(&fdpm_ddata->entry_sem);
		return -ERESTARTSYS;
	}
	ptrw = kmalloc(sizeof(fdpm_queue_entry), GFP_KERNEL);
	if (ptrw) {
		ptrw->entry_id = tmp_entry_id;
		ptrw->source_id = table->source_id;
		ptrw->comp_for_wait = &fdpm_private_data->comp_for_wait;
		memcpy(&ptrw->req, req, sizeof(struct fdpm_start_req));
		ptrw->req.cb_rsp.cb_data.complete.wResult = table->source_id;
		ptrw->req.cb_rsp.cb_data.complete.uwJobId = tmp_entry_id;
		ptrw->timer_ch = table->timer_ch;
		list_add_tail(&ptrw->list,
			      &fdpm_ddata->fdpm_ientryw[dev_no].list);
	} else {
		DPRINT("ERROR kmalloc for wait\n");
		kfree(ptr);
		up(&fdpm_ddata->wait_sem);
		up(&fdpm_ddata->entry_sem);
		return -EFAULT;
	}
	up(&fdpm_ddata->wait_sem);
	up(&fdpm_ddata->entry_sem);
	complete(&fdpm_pdata->set_thread[dev_no].start);
	return 0;
}

/* delete fdpm entry */
void fdpm_queue_del_entry(fdpm_queue_entry *entry)
{
	list_del(&entry->list);
	kfree(entry);
}

/* initalize fdpm post entry */
int fdpm_post_entry_init(struct fdpm_drvdata *fdpm_pdata)
{
	int i;
	if (down_interruptible(&fdpm_pdata->post_sem))
		return -ERESTARTSYS;

	for (i = 0; i < FDPM_FDP_NUM; i++) {
		memset(&fdpm_pdata->fdpm_iwentry[i],
		       0,
		       sizeof(fdpm_wait_entry));
		INIT_LIST_HEAD(&fdpm_pdata->fdpm_iwentry[i].list);
	}
	up(&fdpm_pdata->post_sem);
	return 0;
}

/* entry fdpm post queue */
int fdpm_post_entry(fdpm_queue_entry *req_entry,
		    fdpm_wait_entry *post_entry,
		    struct fdpm_drvdata *fdpm_pdata)
{
	fdpm_wait_entry *ptr;

	if (down_interruptible(&fdpm_pdata->post_sem))
		return -ERESTARTSYS;

	ptr = kmalloc(sizeof(fdpm_wait_entry), GFP_KERNEL);
	if (ptr) {
		memset(ptr, 0, sizeof(fdpm_wait_entry));
		ptr->entry_id = req_entry->entry_id;
		ptr->source_id = req_entry->source_id;
		ptr->comp_for_wait = req_entry->comp_for_wait;
		ptr->timer_ch = req_entry->timer_ch;
		ptr->fdp_flag = 0;
		ptr->timer_flag = 0;
		ptr->cb_rsp.end = req_entry->req.cb_rsp.end;
		ptr->cb_rsp.type = req_entry->req.cb_rsp.type;
		ptr->cb_rsp.cb_data.complete.cb =
			req_entry->req.cb_rsp.cb_data.complete.cb;
		ptr->cb_rsp.cb_data.complete.uwJobId =
			req_entry->req.cb_rsp.cb_data.complete.uwJobId;
		ptr->cb_rsp.cb_data.complete.wResult =
			req_entry->req.cb_rsp.cb_data.complete.wResult;
		ptr->cb_rsp.cb_data.complete.uwUserData =
			req_entry->req.cb_rsp.cb_data.complete.uwUserData;
		ptr->cb_rsp.telecine_mode = req_entry->req.cb_rsp.telecine_mode;
		list_add_tail(&ptr->list, &post_entry->list);
	} else {
		up(&fdpm_pdata->post_sem);
		DPRINT("ERROR kmalloc for wait\n");
		return -EFAULT;
	}
	up(&fdpm_pdata->post_sem);
	return 0;
}

/* delete fdpm post entry */
void fdpm_post_del_entry(fdpm_wait_entry *entry)
{
	list_del(&entry->list);
	kfree(entry);
}

/* check fdpm request queue */
int fdpm_request_entry_check(struct fdpm_resource_table *table,
			     unsigned int source_id,
			     struct fdpm_privdata *fdpm_pdata)
{
	int dev_no = table->device_no;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct list_head *listptr;
	fdpm_queue_entry *entry;
	int detect_flag = 0;

	if (down_interruptible(&fdpm_ddata->entry_sem))
		return -ERESTARTSYS;

	if (!list_empty(&fdpm_ddata->fdpm_ientry[dev_no].list)) {
		DPRINT("list not empty\n");
		list_for_each(listptr, &fdpm_ddata->fdpm_ientry[dev_no].list) {
			entry = (fdpm_queue_entry *)list_entry(listptr,
							       fdpm_queue_entry,
							       list);
			if (entry->source_id == source_id)
				detect_flag = 1;
		}
	}
	up(&fdpm_ddata->entry_sem);
	return detect_flag;
}

/* check fdpm request queue */
int fdpm_post_entry_check(struct fdpm_resource_table *table,
			  unsigned int source_id,
			  struct fdpm_privdata *fdpm_pdata)
{
	int dev_no = table->device_no;
	struct fdpm_drvdata *fdpm_ddata = fdpm_pdata->pdrv;
	struct list_head *listptr;
	fdpm_wait_entry *entry;
	int detect_flag = 0;

	if (down_interruptible(&fdpm_ddata->post_sem))
		return -ERESTARTSYS;

	if (!list_empty(&fdpm_ddata->fdpm_iwentry[dev_no].list)) {
		list_for_each(listptr, &fdpm_ddata->fdpm_iwentry[dev_no].list) {
			entry = (fdpm_wait_entry *)list_entry(listptr,
							      fdpm_wait_entry,
							      list);
			if (entry->source_id == source_id)
				detect_flag = 1;
		}
	}
	up(&fdpm_ddata->post_sem);
	return detect_flag;
}

/******************************************************************************
	Function:       fdpm_resource_add_list
	Description:	add resource table
	Parameter:	IN  : *base_table, *add_table
	Returns:	      0(successful), -1(kmalloc fail)
******************************************************************************/
int fdpm_resource_add_list(struct fdpm_resource_table *base_table,
			   struct fdpm_resource_table **add_table)
{
	struct fdpm_resource_table *tmp;

	tmp = kmalloc(sizeof(struct fdpm_resource_table), GFP_KERNEL);
	if (tmp == NULL)
		return -1;

	tmp->next = base_table->next;
	base_table->next = tmp;
	*add_table = tmp;
	return 0;
}

/******************************************************************************
	Function:       fdpm_resource_del_list
	Description:	delete resource table
	Parameter:	IN  : *target_table(pointer to table include target
				table pointer(next)
	Returns:	0(successful), -1(can not find target table pointer)
******************************************************************************/
int fdpm_resource_del_list(struct fdpm_resource_table *target_table)
{
	struct fdpm_resource_table *tmp;

	if (target_table->next == NULL)
		return -1;

	tmp = target_table->next;
	target_table->next = tmp->next;
	kfree(tmp);
	return 0;
}

/******************************************************************************
	Function:       fdpm_resource_search_source_list
	Description:	search resource table based on source_id
	Parameter:	IN  : sourcd_id : search source id
				mode    : 0(search target table), other(search
						previous table)
				*base_table: base table
			OUT : *target_table : searched target table pointer
	Returns:	0(successful), -1(can not find target table)
******************************************************************************/
int fdpm_resource_search_source_list(unsigned long source_id,
				     int mode,
				     struct fdpm_resource_table *base_table,
				     struct fdpm_resource_table **target_table)
{
	struct fdpm_resource_table *tmp      = NULL;
	struct fdpm_resource_table *previous = NULL;
	int hit = 0;

	tmp = base_table->next;
	previous = base_table;

	while (tmp != NULL) {
		if ((tmp->source_id == source_id) && (tmp->use > 0)) {
			if (mode == 0)
				*target_table = tmp;
			else
				*target_table = previous;
			hit = 1;
			break;
		}
		previous = tmp;
		tmp = tmp->next;
	}
	if (hit)
		return 0;
	else
		return -1;
}

/******************************************************************************
	Function:       fdpm_resource_search_dev_list
	Description:	search resource table based on device_no
	Parameter:	IN  : device_no, *base_table
			OUT : *target_table
	Returns:	0(successful), -1(can not find target table)
******************************************************************************/
int fdpm_resource_search_dev_list(int device_no,
				  struct fdpm_resource_table *base_table,
				  struct fdpm_resource_table **target_table)
{
	struct fdpm_resource_table *tmp = NULL;
	int hit = 0;

	tmp = base_table->next;

	while (tmp != NULL) {
		if ((device_no == tmp->device_no) && (tmp->use > 0) &&
		    (tmp->ocmode == FDP_OCMODE_OCCUPY) &&
		    ((tmp->vintmode == FDP_VMODE_VBEST) ||
		     (tmp->vintmode == FDP_VMODE_VBEST_FDP0) ||
		     (tmp->vintmode == FDP_VMODE_VBEST_FDP1) ||
		     (tmp->vintmode == FDP_VMODE_VBEST_FDP2)
		    )
		   ) {
			*target_table = tmp;
			hit = 1;
			break;
		}
		tmp = tmp->next;
	}
	if (hit)
		return 0;
	else
		return -1;
}

/******************************************************************************
	Function:       fdpm_resource_del_all_list
	Description:	delete all resource table
	Parameter:	IN  : *base_table
	Returns:	0
******************************************************************************/
int fdpm_resource_del_all_list(struct fdpm_resource_table *base_table)
{
	struct fdpm_resource_table *tmp = NULL;

	while (base_table->next != NULL) {
		tmp = base_table->next;
		base_table->next = tmp->next;
		kfree(tmp);
	}
	return 0;
}
