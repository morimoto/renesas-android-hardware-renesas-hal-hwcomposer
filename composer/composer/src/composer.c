/*
 * Function        : Composer driver
 *
 * Copyright (C) 2011-2014 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/rwlock.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/file.h>
#include <linux/string.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#include "imgpctrl/imgpctrl_public.h"

#include "linux/rcar_composer.h"

#include "inc/remote.h"
#include "inc/debug.h"
#include "inc/work.h"
#include "inc/blend.h"
#include "inc/swfence.h"

#include "inc/ionmem.h"
#include "inc/request.h"

#ifdef CONFIG_ION
#include <ion/ion.h>
#include <linux/dma-buf.h>
#include <linux/scatterlist.h>
#endif

static int debug;    /* default debug level */

/******************************************************/
/* define prototype                                   */
/******************************************************/
static long core_ioctl(struct file *filep,
		unsigned int cmd, unsigned long arg);
static int core_open(struct inode *inode, struct file *filep);
static int core_release(struct inode *inode, struct file *filep);

static void free_buffer_handle(struct composer_rh  *rh);

/* module interface */
static int composer_queue(
	void *data,
	int   data_size,
	void  (*callback)(void *user_data, int result),
	void   *user_data,
	struct composer_fh *fh);

/* callback */
static void process_composer_queue_callback(struct composer_rh *rh);

/* complete */
static void complete_work_blend(struct composer_rh *rh);

/* task processed in workqueue */
static int work_create_handle(unsigned long *args);
static int work_delete_handle(unsigned long *args);

static void work_blend(struct localwork *work);
static void work_schedule(struct localwork *work);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pm_early_suspend(struct early_suspend *h);
static void pm_late_resume(struct early_suspend *h);
#endif

static struct composer_fh *allocate_device(void);
static void  free_device(struct composer_fh *fh);
static void ioc_post_callback(void *user_data, int result);
static int  ioc_post(struct composer_fh *fh, struct cmp_postdata *data);
static int iocgs_register(struct composer_fh *fh, struct cmp_registerinfo *arg);
static void rcar_composer_allsyncready(struct composer_rh *rh, int id);
static void work_syncdevice(struct composer_rh *rh);
static dma_addr_t register_buffer_handle(struct composer_rh  *rh,
	struct ionmem     *ionmem, int fd, int cached, int index);
static int handle_queue_data_type3(
	struct composer_rh   *rh, struct cmp_postdata  *post,
	struct composer_fh   *fh __maybe_unused);
static int __init rcar_composer_init(void);
static void __exit rcar_composer_release(void);


/******************************************************/
/* define local define                                */
/******************************************************/

#if INTERNAL_DEBUG
#define INTERNAL_LOG_MSG_SIZE 16384
#endif

#define MAX_OPEN      32
#define MAX_KERNELREQ 4

#define DEV_NAME      "composer"

/* define for core_ioctl function. */
#define CORE_IOCTL_MAX_ARG_LENGTH     300

/* define for time-out */
#if _EXTEND_TIMEOUT
#define WAITCOMP_WAITTIME                 (20 * 1000)  /* msec */
#define WORK_DISPDRAW_SWFENCE_WAITTIME    (10 * 1000)  /* msec */
#define QUEUE_WAITTIME                    (10 * 1000)  /* msec */
#else
#define WAITCOMP_WAITTIME                  300  /* msec */
#define WORK_DISPDRAW_SWFENCE_WAITTIME     200  /* msec */
#define QUEUE_WAITTIME                     100  /* msec */
#endif

#if _TIM_DBG
/* define slot ID for timerecord */
#define TIMID_QUEUE          0
#define TIMID_SWSYNC1        1
#define TIMID_BLEND_S        2
#define TIMID_BLEND_E        3
#define TIMID_CALLBACK       4
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#define NUM_OF_COMPOSER_PROHIBITE_AT_START    0
#define NUM_OF_COMPOSER_PROHIBITE_AT_RESUME   0
#endif

/******************************************************/
/* define local variables                             */
/******************************************************/
static const struct file_operations composer_fops = {
	.owner		= THIS_MODULE,
	.write		= NULL,
	.unlocked_ioctl	= &core_ioctl,
	.open		= &core_open,
	.release	= &core_release,
};

static struct miscdevice composer_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "composer",
	.fops = &composer_fops,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static  struct early_suspend    early_suspend = {
	.level		= EARLY_SUSPEND_LEVEL_STOP_DRAWING,
	.suspend	= &pm_early_suspend,
	.resume		= &pm_late_resume,
};
static int composer_prohibited_count = NUM_OF_COMPOSER_PROHIBITE_AT_START;
#endif /* CONFIG_HAS_EARLYSUSPEND */

static DEFINE_SPINLOCK(irqlock);
static DEFINE_SEMAPHORE(sem);
static int                    num_open;

/* workqueue to execute RT-API Graphics API, */
static struct localworkqueue  *workqueue[
	CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT];

/* workqueue to schedule request. */
static struct localworkqueue  *workqueue_schedule;

/* task for blending */
static void                   *graphic_handle[
	CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT];

#if CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT > 1
/* counter to select blend unit. */
static unsigned char          blend_unit_sel;
#endif

/* queue data */
static DECLARE_WAIT_QUEUE_HEAD(kernel_waitqueue_comp);
static DEFINE_SEMAPHORE(kernel_queue_sem);

#ifdef CONFIG_HAS_EARLYSUSPEND
static int                    in_early_suspend;
#endif /* CONFIG_HAS_EARLYSUSPEND */

#if INTERNAL_DEBUG
static char        *internal_log_msg;
static int         internal_log_msgsize;
#endif

/******************************************************/
/* include other modules                              */
/******************************************************/
#include "src/debug.c"
#include "src/swfence.c"
#include "src/work.c"
#include "src/blend.c"
#include "src/remote.c"
#include "src/ionmem.c"
#include "src/request.c"
#if CONFIG_MISC_R_CAR_COMPOSER_DEVATTR
#include "src/devattr.c"
#endif


/******************************************************/
/* local functions                                    */
/******************************************************/
/*! \brief allocate private memory for device
 *  \return result of processing.
 *  \retval 0 error
 *  \retval other pointer to structure composer_fh
 *  \details
 *  Allocate private memory of structure composer_fh.\n
 *  And initialize variables to use private data for file handle.
 */
static struct composer_fh *allocate_device(void)
{
	struct composer_fh *fh;
	int    i;

	DBGENTER("\n");

	fh = kzalloc(sizeof(*fh), GFP_KERNEL);
	if (NULL == fh)
		goto err_exit;

	/* semaphore */
	sema_init(&fh->fh_sem, 1);

	/* initialize list head */
	INIT_LIST_HEAD(&fh->fhlist_top);

	fh->ioctl_args = kmalloc(CORE_IOCTL_MAX_ARG_LENGTH * 4, GFP_KERNEL);
	if (NULL == fh->ioctl_args) {
		kfree(fh);
		fh = NULL;
		goto err_exit;
	}

	/* set sequence number.*/
#if INTERNAL_DEBUG || _ATR_DBG
	fh->dev_number = num_open;
	fh->seq_number = 0;
#endif

	fh->ionmem = initialize_ionmem();
	if (NULL == fh->ionmem) {
		kfree(fh->ioctl_args);
		kfree(fh);
		fh = NULL;
		goto err_exit;
	}
	fh->swfence = fence_get_handle();
	if (NULL == fh->swfence) {
		release_ionmem(fh->ionmem);
		kfree(fh->ioctl_args);
		kfree(fh);
		fh = NULL;
		goto err_exit;
	}

	/* add request handle */
	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	for (i = 0; i < MAX_KERNELREQ; i++) {
		/* append request handle */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL == rhandle_create(fh)) {
			/* error report */
			printk_err2("can not create request handle.\n");
			break;
		}
	}

	up(&kernel_queue_sem);

err_exit:
	DBGLEAVE("%p\n", fh);

	return fh;
}

/*! \brief free private memory for device
 *  \param[in] fh pointer to structure composer_fh
 *  \return none
 *  \details
 *  Free resources used in file handle.
 */
static void  free_device(struct composer_fh *fh)
{
	DBGENTER("fh:%p\n", fh);

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	/* free request handle */
	rhandle_free(fh);

	up(&kernel_queue_sem);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != fh->ionmem) {
		printk_dbg2(2, "release_ionmem.\n");
		release_ionmem(fh->ionmem);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != fh->swfence) {
		printk_dbg2(2, "fence_put_handle.\n");
		fence_put_handle(fh->swfence);
	}

	kfree(fh->ioctl_args);
	kfree(fh);

	DBGLEAVE("\n");
}

/*! \brief callback of post for ioctl
 *  \param[in] user_data  user data
 *  \param[in] result     reserved to 1
 *  \return none
 *  \details
 *  Callback if complete composition requested at composer_queue.
 */
static void ioc_post_callback(void *user_data, int result)
{
	printk_dbg2(3, "callback user_data:%p result:%d\n", user_data, result);

	if (0 != result) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}
	if (NULL != user_data) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}
}

/*! \brief post for ioctl
 *  \param[in] fh pointer to structure composer_fh.
 *  \param[in] data pointer to structure cmp_postdata.
 *  \return result of processing.
 *  \retval fence   normal. fence is grater than or equal to zero.
 *  \retval -EINVAL error
 *  \details
 *  Request to start composition.
 *  Information of composition is passed by structure cmp_postdata.\n
 *  If composition starts successfully, this function creates a fence.
 *  \msc
 *    application, composer;
 *    |||;
 *    --- [label="initialize"];
 *    application=>composer
 *        [label="open", URL="\ref ::core_open"];
 *    application=>composer
 *        [label="query/register info", URL="\ref ::iocgs_register"];
 *    --- [label="composition"];
 *    application=>composer
 *        [label="post", URL="\ref ::ioc_post"];
 *    application=>application
 *        [label="wait fence signaled"];
 *    composer box composer
 *        [label="do composition"];
 *    composer->application
 *        [label="fence signaled"];
 *    composer box composer
 *        [label="complete composition", URL="\ref ::ioc_post_callback"];
 *    application box application
 *        [label="close fence"];
 *    --- [label="finalize"];
 *    application=>composer
 *        [label="close", URL="\ref ::core_release"];
 *  \endmsc
 */
static int  ioc_post(struct composer_fh *fh, struct cmp_postdata *data)
{
	int rc;

	DBGENTER("fh:%p data:%p\n", fh, data);
	printk_dbg2(3, "arg num_buffer:%d operation_type:%d\n",
		data->num_buffer, data->operation_type);

	if (data->num_buffer > CMP_DATA_NUM_GRAP_LAYER) {
		/* num of buffer too large */
		rc = -EINVAL;
		goto err;
	}

	/* increment sequence number.*/
#if INTERNAL_DEBUG || _ATR_DBG
	fh->seq_number++;
#endif

	rc = composer_queue(data, sizeof(*data),
		&ioc_post_callback, NULL, fh);

	if (rc == CMP_OK) {
		/* no error */
		rc = fence_get_syncfd(fh->swfence);
	} else {
		/* set error code */
		rc = -EINVAL;
	}
err:
	DBGLEAVE("%d\n", rc);
	return rc;
}

/*! \brief query and register for ioctl
 *  \param[in] fh pointer to structure composer_fh.
 *  \param[in,out] arg pointer to structure cmp_registerinfo.
 *  \return result of processing.
 *  \retval 0   always return this value.
 *  \details
 *  Set information of max images that composer supports.
 *  Registration of information is reserved for future use.
 */
static int iocgs_register(struct composer_fh *fh, struct cmp_registerinfo *arg)
{
	int rc;
	DBGENTER("fh:%p arg:%p\n", fh, arg);

	if (NULL != fh) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}

	/* set info */
	arg->max_size = RT_GRAPHICS_MAX_IMAGE_LENGTH;
	arg->max_area = RT_GRAPHICS_MAX_IMAGE_AREA;
	arg->max_rotbuf = CONFIG_MISC_R_CAR_COMPOSER_MAX_ROTBUFFER;

	/* register information */

	rc = 0;

	DBGLEAVE("\n");
	return rc;
}


#if FEATURE_USE_ASYNC_WAIT
/*! \brief notice fence signaled
 *  \param[in] rh  pointer to structure composer_rh.
 *  \param[in] id  reserved to 0.
 *  \return none
 *  \details
 *  Schedule to start schedule task when all fences become signaled.
 */
static void rcar_composer_allsyncready(struct composer_rh *rh, int id)
{
	DBGENTER("rh:%p id:%d\n", rh, id);
	if (id == 0) {
		printk_dbg2(3, "schedule to blend\n");

		/* schedule */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false == localwork_queue(workqueue_schedule,
			&rh->rh_wqtask_schedule)) {
			/* fatal error */
			printk_err("drop blend schedule.\n");
		}
#if _TIM_DBG
		timerecord_record(rh->timerecord, TIMID_SWSYNC1);
#endif
	}
	DBGLEAVE("\n");
}
#endif

/*! \brief task for schedule
 *  \param[in] work  pointer to structure localwork.
 *  \return none
 *  \details
 *  Schedule to start blend task.\n
 *  If async wait feature is not used,
 *  then wait a fence in the schedule task.
 */
static void work_schedule(struct localwork *work)
{
	struct composer_rh *rh;

	ATRACE_BEGIN("schedule");

	rh = container_of(work, struct composer_rh, rh_wqtask_schedule);

	DBGENTER("work:%p\n", work);

	/* blend request add to control. */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false != rh->data.valid) {
		/* handle sync_wait */
#if FEATURE_USE_ASYNC_WAIT == 0
		printk_dbg2(3, "wait sync fence.\n");
		fence_wait(rh, WORK_DISPDRAW_SWFENCE_WAITTIME, 0);
#if _TIM_DBG
		timerecord_record(rh->timerecord, TIMID_SWSYNC1);
#endif
#endif

		/* queue blend tasks */
		localwork_flush(&rh->rh_wqtask);
#if CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT > 1
		rh->use_blendunit = blend_unit_sel;

		blend_unit_sel++;
		if (blend_unit_sel >= CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT)
			blend_unit_sel = 0;
#else
		rh->use_blendunit = 0;
#endif
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false == localwork_queue(workqueue[rh->use_blendunit],
			&rh->rh_wqtask)) {
			/* fatal error */
			printk_err("drop blend request.\n");
		}
	}

	ATRACE_END("schedule");
	DBGLEAVE("\n");
	return;
}

/*! \brief delete handle for blend
 *  \param[in] args  Array of integer.\n
 *                   Element 0 is used to specify the blend unit.
 *  \return result of processing.
 *  \retval 0 always return this value.
 *  \details
 *  If handle is created then delete the handle.
 */
static int work_delete_handle(unsigned long *args)
{
	int blendunit = (int)args[0];

	TRACE_ENTER(FUNC_WQ_DELETE);
	DBGENTER("blendunit:%d\n", blendunit);

	ATRACE_BEGIN("del_handle");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != graphic_handle[blendunit]) {
		/* ignore rtapi-error because of an unrecoverable. */
		blend_delete(graphic_handle[blendunit]);

		graphic_handle[blendunit] = NULL;
	}

	ATRACE_END("del_handle");
	TRACE_LEAVE(FUNC_WQ_DELETE);
	DBGLEAVE("\n");
	return 0;
}

/*! \brief create handle for blend
 *  \param[in] args  Array of integer.\n
 *                   Element 0 is used to specify the blend unit.
 *  \return result of processing.
 *  \retval 0 always return this value.
 *  \details
 *  If handle is created then delete the handle at first,
 *  and create a new handle.
 */
static int work_create_handle(unsigned long *args)
{
	int blendunit = (int)args[0];

	TRACE_ENTER(FUNC_WQ_CREATE);
	DBGENTER("blendunit:%d\n", blendunit);

	ATRACE_BEGIN("create_handle");

	/* from static analysis tool message Msg(4:3344).      */
	/* Conditional expression require a boolean expression */
	if (NULL != graphic_handle[blendunit]) {
		/* error report and free handle to re-create graphic handle */
		printk_err("graphic_handle is not NULL\n");
		blend_delete(graphic_handle[blendunit]);
		graphic_handle[blendunit] = NULL;
	}

	/* create graphic handle */
	graphic_handle[blendunit] = blend_create();

	ATRACE_END("create_handle");
	TRACE_LEAVE(FUNC_WQ_CREATE);
	DBGLEAVE("\n");
	return 0;
}

/*! \brief process after blend
 *  \param[in] rh  pointer to structure composer_rh.
 *  \return none
 *  \details
 *  Free all buffers where the composer using during composition.\n
 *  Increment timeline to a fence become signaled.
 *  And issue callback.
 */
static void complete_work_blend(struct composer_rh *rh)
{
	DBGENTER("rh:%p\n", rh);

	ATRACE_BEGIN("blend comp");

	free_buffer_handle(rh);

	printk_dbg2(3, "generate signal for blend\n");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rh->swfence) {
		/* static analysis tool message (4:3200): */
		/* all function with return value         */
		/* require confirm it is no-error         */
		if (fence_signal(rh->swfence) != CMP_OK) {
			/* this condition never become true.                */
			printk_err2("error in fence_signal\n");
		}
		fence_put_handle(rh->swfence);
		rh->swfence = NULL;
	}

	/* process callback */
	process_composer_queue_callback(rh);

	ATRACE_END("blend comp");
	DBGLEAVE("\n");
	return;
}

/*! \brief sync buffer to device.
 *  \param[in] rh  pointer to structure composer_rh.
 *  \return none
 *  \details
 *  If sync to device is necessary, then call dma_sync_sg_for_device().\n
 *  This implementation is same as ion_sync_for_device()
 *  in driver/gpu/ion/ion.c
 */
static void work_syncdevice(struct composer_rh *rh)
{
	int i;
	struct dma_buf    *dmabuf;
	struct ion_handle *handle;
	struct ion_client *client;
	struct sg_table   *sg_table;
	int    cache_flag;

	DBGENTER("rh:%p\n", rh);
	ATRACE_BEGIN("blend cache");

	client = rh->ionmem->client;

	for (i = 0; i < rh->num_buffer; i++) {
		dmabuf = rh->iondmabuf[i];
		handle = rh->ionhandle[i];
		cache_flag = rh->ioncached[i];

		printk_dbg2(3, "%d: dmabuf:%p handle:%p\n", i, dmabuf, handle);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if ((NULL == dmabuf) || (NULL == handle))
			continue;

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 == cache_flag)
			continue;

		sg_table = ion_sg_table(client, handle);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != IS_ERR_OR_NULL(sg_table)) {
			/* ignore dma buf operation. */
			printk_err("ion_sg_table fail\n");
		} else {
			dma_sync_sg_for_device(NULL, sg_table->sgl,
				(int)sg_table->nents, DMA_BIDIRECTIONAL);
		}
	}
	ATRACE_END("blend cache");

	DBGLEAVE("\n");
}



/*! \brief task for blend
 *  \param[in] work  pointer to structure localwork.
 *  \return none
 *  \details
 *  Processing the cache operations, create or delete a graphic handle,
 *  and request blending.
 */
static void work_blend(struct localwork *work)
{
	struct composer_rh *rh;
	int  rc;
	unsigned long blendunit;
#if _ATR_DBG
	char   taskname[64];
#endif

	TRACE_ENTER(FUNC_WQ_BLEND);
	DBGENTER("work:%p\n", work);

	rh = container_of(work, struct composer_rh, rh_wqtask);

#if _ATR_DBG
	/* static analysis tool message (4:3200):           */
	/* in order to make it clear that did not check     */
	/* the return value of snprintf, cast to void       */
#if _LOG_DBG > 1
	(void) snprintf(taskname, sizeof(taskname), "blend-%u-%x",
		rh->dev_number, rh->seq_number);
#else
	(void) snprintf(taskname, sizeof(taskname), "blend-%u",
		rh->dev_number);
#endif
#endif

	ATRACE_BEGIN(taskname);

#if _TIM_DBG
	timerecord_record(rh->timerecord, TIMID_BLEND_S);
#endif

	/* sync device */
	work_syncdevice(rh);

	blendunit = rh->use_blendunit;

	/* confirm create graphic handle */
	if (graphic_handle[blendunit] == NULL) {
		int need_create = true;

#ifdef CONFIG_HAS_EARLYSUSPEND
		printk_dbg2(2,
			"graphic_handle NULL num_open:%d in_early_suspend:%d\n",
			num_open, in_early_suspend);
#else
		printk_dbg2(2, "graphic_handle NULL num_open:%d\n", num_open);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
		if (false != in_early_suspend) {
			/* cancel create handle */
			need_create = false;
		}
#endif
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != need_create) {
			/* try to create handle */

			/* static analysis tool message (4:3200): */
			/* all function with return value         */
			/* require confirm it is no-error         */
			if (work_create_handle(&blendunit) != 0) {
				/* this condition never become true.          */
				printk_err2("can not create handle.\n");
			}
		}
	}

	printk_dbg2(2, "data.valid:%d\n", rh->data.valid);

#if _LOG_DBG > 1
	/* detect un-expected condition. */
	WARN_ON(rh->data.valid == false || rh->active == false);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	if (false != in_early_suspend) {
		printk_err1(
"composition was skipped because already in early suspend state.\n");
		rc = CMP_OK;
		goto finish;
	}
#endif
	if (graphic_handle[blendunit] == NULL) {
		printk_err1("handle not created.\n");
		rc = CMP_NG;
		goto finish;
	}

	{
		/* run blending */
		rc = blend_request(graphic_handle[blendunit], &rh->data.blend,
			rh, NULL);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (CMP_OK != rc) {
			/* delete graphic handle */

			/* from static analysis tool message (4:3344). */
			/* Conditional expression require a boolean    */
			/* expression                                  */
			if (0 != work_delete_handle(&blendunit)) {
				/* this condition never become true.      */

				/* static analysis tool message (4:3200): */
				/* all function with return value         */
				/* require confirm it is no-error         */
				printk_err2("can not delete handle.\n");
			}
		}
	}

finish:

	printk_dbg1(2, "results rc:%d\n", rc);
	if (rc != CMP_OK) {
		const char *msg1 = "";

		/* error report */
#ifdef CONFIG_HAS_EARLYSUSPEND
		if (false != in_early_suspend) {
			/* set message */
			msg1 = "[in suspend]";
		}
#endif
		printk_err("blend result is error %s\n", msg1);
	}

#if _TIM_DBG
	timerecord_record(rh->timerecord, TIMID_BLEND_E);
#endif

	{
		/* complete blend task. */
		complete_work_blend(rh);
	}

	ATRACE_END(taskname);
	TRACE_LEAVE(FUNC_WQ_BLEND);
	DBGLEAVE("\n");
	return;
}

/*! \brief free buffer
 *  \param[in] rh  pointer to structure composer_rh.
 *  \return none
 *  \details
 *  Free ion buffers used for composition.
 */
static void free_buffer_handle(struct composer_rh  *rh)
{
	int i;

	DBGENTER("rh:%p\n", rh);

	for (i = 0; i < rh->num_buffer; i++) {
		struct ion_handle *handle = rh->ionhandle[i];
		struct dma_buf    *dmabuf = rh->iondmabuf[i];

		rh->ionhandle[i] = NULL;
		rh->iondmabuf[i] = NULL;

		printk_dbg2(3, "%d: handle:%p dmabuf:%p\n",
			i, handle, dmabuf);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != dmabuf) {
			/* free dmabuf */
			dma_buf_put(dmabuf);
		}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != handle) {
			/* free memory */
			ion_free(rh->ionmem->client, handle);
		}
	}
	DBGLEAVE("\n");
}

/*! \brief regist buffer
 *  \param[in] rh     pointer to structure composer_rh.
 *  \param[in] ionmem pointer to structure ionmem.
 *  \param[in] fd     ion buffer
 *  \param[in] cached cache flag for ion buffer.
 *  \param[in] index  arrays to register.
 *  \return physical address
 *  \details
 *  register buffer to request handle for composition.
 */
static dma_addr_t register_buffer_handle(struct composer_rh  *rh,
	struct ionmem     *ionmem,
	int               fd,
	int               cached,
	int               index)
{
	dma_addr_t phys_addr;

	DBGENTER("rh:%p fd:%d index:%d\n", rh, fd, index);

	/* set default */
	phys_addr = 0;

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if ((NULL != rh->ionhandle[index]) || (NULL != rh->iondmabuf[index])) {
		/* report error */
		printk_err("ionhandle already registered.\n");
	} else {
		struct ion_handle *handle;
		struct dma_buf    *dmabuf;
		struct ion_client *client = ionmem->client;

		handle = ion_import_dma_buf(client, fd);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != IS_ERR_OR_NULL(handle)) {
			printk_err("ion_import_dma_buf fail\n");
			handle = NULL;
			dmabuf = NULL;
		} else {
			dmabuf = dma_buf_get(fd);
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			if (false != IS_ERR_OR_NULL(dmabuf)) {
				printk_err("dma_buf_get fail\n");
				ion_free(client, handle);
				handle = NULL;
				dmabuf = NULL;
			}
		}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != handle) {
			/* register */
			rh->ionhandle[index] = handle;
			rh->iondmabuf[index] = dmabuf;
			rh->ioncached[index] = cached;

			phys_addr = getphys_ionhandle(ionmem, handle);
		}
	}

	DBGLEAVE("0x%llx\n", (uint64_t)phys_addr);
	return phys_addr;
}

#if _LOG_DBG >= 2
/*! \brief log arguments for handle_queue_data_type3.
 *  \param[in] post pointer to structure cmp_postdata.
 *  \return none
 *  \details
 *  log arguments of postdata.
 *  this function is available if _LOG_DBG is grater equal 2.
 */
static void log_handle_queue_data_type3(struct cmp_postdata  *post)
{
	int i;
	unsigned int info[CMP_DATA_NUM_GRAP_LAYER];

	printk_dbg2(3, "num_buffer:%d\n", post->num_buffer);
	printk_dbg2(3, "operation_type:%d\n", post->operation_type);

	for (i = 0; i < post->num_buffer; i++)
		info[i] = post->phys_address[i];
	for (; i < CMP_DATA_NUM_GRAP_LAYER; i++)
		info[i] = 0;

	printk_dbg2(3, "phys: 0:0x%x 1:0x%x 2:0x%x 3:0x%x 4:0x%x\n",
		info[0], info[1], info[2], info[3], info[4]);

	for (i = 0; i < post->num_buffer; i++)
		info[i] = post->buffer_fd[i];
	for (; i < CMP_DATA_NUM_GRAP_LAYER; i++)
		info[i] = 0;

	printk_dbg2(3, "buffer_fd: 0:%d 1:%d 2:%d 3:%d 4:%d\n",
		info[0], info[1], info[2], info[3], info[4]);

	printk_dbg2(3, "acquire_fd: %d\n",
		post->acquire_fd);
}
#endif

/*! \brief handle queue of structure cmp_postdata.
 *  \param[in,out] rh  pointer to structure composer_rh.
 *  \param[in] post pointer to structure cmp_postdata.
 *  \param[in] fh   pointer to structure composer_fh.
 *  \return result of processing.
 *  \retval CMP_OK  normal
 *  \retval CMP_NG  error found
 *  \details
 *  Get buffer information from argument post,
 *  and create blend parameter.
 */
static int handle_queue_data_type3(
	struct composer_rh   *rh,
	struct cmp_postdata  *post,
	struct composer_fh   *fh __maybe_unused)
{
	int i;
#if _LOG_DBG >= 2
	log_handle_queue_data_type3(post);
#endif

	/**********************************
	 confirm graphic buffer
	**********************************/
	if ((post->num_buffer < 0) ||
		((unsigned int)(post->num_buffer) >
		ARRAY_SIZE(post->phys_address))) {
		printk_err2("num_buffer invalid.\n");
		goto err_exit;
	}

	/**********************************
	 resolve buffer address.
	**********************************/
	/* static analysis tool message (4:3200):           */
	/* in order to make it clear that did not check     */
	/* the return value of memset, cast to void         */
	(void) memset(&rh->buffer_address, 0, sizeof(rh->buffer_address));

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == fh->ionmem->client) {
		printk_err("ion not registered.");
		goto err_exit;
	}

	for (i = 0; i < post->num_buffer; i++) {
		int fd;
		unsigned int phys_addr;

		/* default address */
		rh->buffer_address[i] = 0;

		fd = post->buffer_fd[i];
		if (fd < 0) {
			/* pass from argument */
			phys_addr = post->phys_address[i];
		} else {
			int cached = post->buffer_cached[i];
			/* pass from ion */
			phys_addr = register_buffer_handle(
				rh, fh->ionmem, fd, cached, i);

			if (phys_addr == 0) {
				printk_err2("phys_addr is 0.");
				break;
			}
			printk_dbg2(3, "rtAddress[%d] = 0x%x\n",
				i, phys_addr);

			post->phys_address[i] = phys_addr;
		}

		rh->buffer_address[i] = phys_addr;
		printk_dbg2(2, "index:%d phys-address:0x%x\n",
			i, phys_addr);
	}
	if (i != post->num_buffer) {
		printk_err2("can not resolve address.\n");
		goto err_exit;
	}

	/**********************************
	 set num buffer
	**********************************/

	rh->num_buffer = post->num_buffer;

	/**********************************
	 setup parameters.
	**********************************/

	if (blend_config(rh, post) == CMP_NG) {
		printk_err2("error in blend config.");
		goto err_exit;
	}

	rh->refcount = 0;

	/* from static analysis tool message(4:3344).          */
	/* Conditional expression require a boolean expression */
	if (false != rh->data.valid)
		rh->refcount++;

	if (rh->refcount == 0) {
		printk_err2("usable blend config not found.");
		goto err_exit;
	}

	/* configure fence_sync */

	/* static analysis tool message (4:3200): */
	/* all function with return value         */
	/* require confirm it is no-error         */
	if (fence_config(rh, post) != CMP_OK) {
		/* this condition never become true.          */
		printk_dbg2(3,
		"fence_config error. continue operation without fence.\n");
	}

	/* increment reference */
	rh->ionmem = fh->ionmem;
	incref_ionclient(rh->ionmem);

	return CMP_OK;

err_exit:
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != fh->ionmem) {
		size_t j;
		for (j = 0; j < ARRAY_SIZE(rh->iondmabuf); j++) {
			/* from static analysis tool message(4:3344).  */
			/* Conditional expression require a boolean    */
			/* expression                                  */
			if (NULL != rh->iondmabuf[j]) {
				dma_buf_put(rh->iondmabuf[j]);
				rh->iondmabuf[j] = NULL;
			}
		}
		for (j = 0; j < ARRAY_SIZE(rh->ionhandle); j++) {
			/* from static analysis tool message(4:3344).  */
			/* Conditional expression require a boolean    */
			/* expression                                  */
			if (NULL != rh->ionhandle[j]) {
				ion_free(fh->ionmem->client, rh->ionhandle[j]);
				rh->ionhandle[j] = NULL;
			}
		}
	}

	return CMP_NG;
}

/*! \brief handle queue of structure cmp_postdata.
 *  \param[in] data  pointer to data to post.
 *  \param[in] data_size size of data.
 *  \param[in] callback when complete composition.
 *  \param[in] user_data argument of callback
 *  \param[in] fh    pointer to structure composer_fh.
 *  \return result of processing.
 *  \retval CMP_OK  normal
 *  \retval CMP_NG  error found
 *  \details
 *  Search unused structure of kernel_queue,
 *  Initialize it by handle_queue_data_type3().\n
 *  And wait fence signals asynchronously by fence_async_wait().
 *  Composition is started after all fences signaled.
 */
static int composer_queue(
	void *data,
	int   data_size,
	void  (*callback)(void *user_data, int result),
	void   *user_data,
	struct composer_fh  *fh)
{
	int rc = -1;
	TRACE_ENTER(FUNC_QUEUE);
	DBGENTER("data:%p data_size:%d callback:%p user_data:%p\n",
		data, data_size, callback, user_data);

	R_ATRACE_BEGIN("queue");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == fh) {
		printk_err2("fh invalid.\n");
		goto err_exit;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	if (false != in_early_suspend) {
		printk_dbg2(2, "now early-suspending.\n");
		/* nothing to do */
		goto err_exit;
	}
	if (composer_prohibited_count > 0) {
		printk_dbg2(2, "now prohibited composer.\n");
		composer_prohibited_count--;
		goto err_exit;
	}
#endif

	/* from static analysis tool message (4:3:3197).            */
	/* verbose initialization of rh is causes warning.          */
	/* if remode this initialization, it may be causes          */
	/* reffere in error handling.                               */
	/* to prevent this problem, minimize available range of rh. */
	{
		struct composer_rh  *rh;

		printk_dbg2(3, "down\n");
		down(&kernel_queue_sem);

		rh = rhandle_getunusedhandle(fh);

		up(&kernel_queue_sem);

		if (rh == NULL) {
			printk_err2("no space left to request blending.");
			goto err_exit;
		}

#if _TIM_DBG
		timerecord_reset(rh->timerecord);
		timerecord_record(rh->timerecord, TIMID_QUEUE);
#endif

		if ((unsigned int)data_size == sizeof(struct cmp_postdata)) {
			/* handle queue using cmp_postdata*/
			struct cmp_postdata *_data;
			_data = (struct cmp_postdata *)data;

			/* from static analysis tool message (4:3344).  */
			/* Conditional expression require a boolean     */
			/* expression                                   */
			if (CMP_OK != handle_queue_data_type3(rh, _data, fh)) {
				printk_err1("queue data invalid.\n");
				goto err_exit2;
			}
		} else {
			printk_err("size of queue not match.\n");
			goto err_exit2;
		}

		/* set fence object */
		rh->swfence = fence_dup_handle(fh->swfence);

		/* increase timeline signaled. */
		if (NULL != rh->swfence) {
			/* static analysis tool message (4:3200): */
			/* all function with return value         */
			/* require confirm it is no-error         */
			if (fence_inc_timeline(rh->swfence) != CMP_OK) {
				/* this condition never become true.          */
				printk_err2("fence_inc_timeline error\n");
			}
		}

		/* set sequence number */
#if INTERNAL_DEBUG || _ATR_DBG
		rh->seq_number = fh->seq_number;
#endif

		rh->user_callback  = callback;
		rh->user_data      = user_data;

		/**********************************
		 schedule to run.
		**********************************/
		/* queue tasks */
		localwork_flush(&rh->rh_wqtask_schedule);
#if FEATURE_USE_ASYNC_WAIT

		/* static analysis tool message (4:3200): */
		/* all function with return value         */
		/* require confirm it is no-error         */
		if (fence_async_wait(rh) != CMP_OK) {
			/* this condition never become true.          */
			printk_err2("fence_async_wait error\n");
		}
#else
		if (false == localwork_queue(workqueue_schedule,
			&rh->rh_wqtask_schedule)) {
			/* fatal error */
			printk_err("drop blend schedule.\n");
		}
#endif

		rc = 0;

err_exit2:
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != rc) {
			/**********************************
			 some error found
			**********************************/
			rh->refcount = 0;
			rhandle_putusedhandle(rh);
		}
	}
err_exit:

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != rc) {
		/* error detected. */
		TRACE_ENTER(FUNC_CALLBACK);
		callback(user_data, 1);
		TRACE_LEAVE(FUNC_CALLBACK);

		printk_err2("request failed.\n");

		/* wake-up waiting thread */
		wake_up_all(&kernel_waitqueue_comp);
	}

	R_ATRACE_END("queue");

	TRACE_LEAVE(FUNC_QUEUE);
	DBGLEAVE("%d\n", rc);
	return rc;
}

/*! \brief handle callback
 *  \param[in] rh  pointer to structure composer_rh
 *  \return none
 *  \details
 *  Release all used resources, and issue callback.
 */
static void process_composer_queue_callback(struct composer_rh *rh)
{
	void   (*user_callback)(void *user_data, int result);
	void   *user_data;

	TRACE_ENTER(FUNC_CALLBACK);
	DBGENTER("rh:%p\n", rh);

	user_callback = NULL;
	user_data     = NULL;

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	rh->refcount--;
	printk_dbg2(3, "refcount:%d\n", rh->refcount);
	if (rh->refcount <= 0) {
		user_callback = rh->user_callback;
		user_data     = rh->user_data;
		rh->user_callback = NULL;

		/* expire all sync object here. */

		/* static analysis tool message (4:3200): */
		/* all function with return value         */
		/* require confirm it is no-error         */
		if (fence_expire(rh) != CMP_OK) {
			/* this condition never become true.          */
			printk_err2("fence_expire error.");
		}

		/* decrement reference */
		decref_ionclient(rh->ionmem);
		rh->ionmem = NULL;

#if _TIM_DBG
		timerecord_record(rh->timerecord, TIMID_CALLBACK);
		timerecord_print(rh->timerecord);
#endif

		rhandle_putusedhandle(rh);

		/* wake-up waiting thread */
		wake_up_all(&kernel_waitqueue_comp);
	}
	up(&kernel_queue_sem);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != user_callback) {
		TRACE_LOG(FUNC_CALLBACK);
		user_callback(user_data, 1);
		/* process callback */
	}
	TRACE_LEAVE(FUNC_CALLBACK);
	DBGLEAVE("\n");
}

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
/*! \brief static information of composer
 *  \param[in] s  pointer to structure seq_file
 *  \return none
 *  \details
 *  Create debug log of static variables.
 */
static void rcar_composer_debug_info_static(struct seq_file *s)
{
	size_t i;

	/* log of static variable */
	seq_puts(s,   "[semaphore]\n");
	seq_printf(s, "  sem:%d\n", sem.count);
	seq_printf(s, "  kernel_queue_sem:%d\n",
		kernel_queue_sem.count);
	seq_puts(s,   "[static]\n");
	seq_printf(s, "  num_open:%d\n", num_open);
	seq_printf(s, "  debug:%d\n", debug);
#ifdef CONFIG_HAS_EARLYSUSPEND
	seq_printf(s, "  in_early_suspend:%d\n", in_early_suspend);
#endif

	for (i = 0; i < ARRAY_SIZE(graphic_handle); i++) {
		/* graphic_handle log */
		seq_printf(s, "  graphic_handle[%d]:%p\n", i,
			graphic_handle[i]);
	}
}

#define DBGMSG_WORKQUEUE(ARG) { if (ARG)                         \
		seq_printf(s, "  %s run:%d priority:%d\n", #ARG, \
			false == list_empty(&ARG->top), ARG->priority); }

#define DBGMSG_WORKTASK(NAME, ARG) {                              \
		seq_printf(s, "  %s queue:%d status:%d\n", #NAME, \
			false == list_empty(&ARG.link), ARG.status); }

/*! \brief queue information of composer
 *  \param[in] s  pointer to structure seq_file
 *  \return none
 *  \details
 *  Create debug log of composition queue.
 */
static void rcar_composer_debug_info_queue(struct seq_file *s)
{
	int i;
	struct composer_rh *rh;
	struct composer_rh *next_rh;

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	/* queue info */
	i = 0;
	list_for_each_entry_safe(rh, next_rh, &rhlist_top, rh_link) {
		seq_printf(s, "[queue-%d]\n", i);

		*internal_log_msg = 0;
		rcar_composer_dump_rhandle(
			internal_log_msg, internal_log_msgsize, rh);

		seq_puts(s, internal_log_msg);
		seq_puts(s, "\n");
		rcar_composer_debug_info_fencewait_handle(s, rh);
		rcar_composer_debug_info_fence_handle(s, rh->swfence);
		i++;
	}

	/* workqueue */

	seq_puts(s, "[workqueue]\n");

	DBGMSG_WORKQUEUE(workqueue[0]);
#if CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT > 1
	DBGMSG_WORKQUEUE(workqueue[1]);
	/* all log is not printed, because currently it is not necessary. */
#endif
	DBGMSG_WORKQUEUE(workqueue_schedule);
	seq_puts(s, "[worktask]\n");

	i = 0;
	list_for_each_entry_safe(rh, next_rh, &rhlist_top, rh_link) {
		seq_printf(s, "  [queue-%d]\n", i);

		DBGMSG_WORKTASK(rh_wqtask,
			rh->rh_wqtask);
		DBGMSG_WORKTASK(rh_wqtask_schedule,
			rh->rh_wqtask_schedule);
		i++;
	}

	up(&kernel_queue_sem);
}
#undef DBGMSG_WORKQUEUE
#undef DBGMSG_WORKTASK

#endif
#endif

/******************************************************/
/* file operation entry function                      */
/******************************************************/

/*! \brief ioctl operation
 *  \param[in] filep  pointer to structure file
 *  \param[in] cmd    command
 *  \param[in] arg    argument of command.
 *  \return result of processing.
 *  \retval 0         normal
 *  \retval -EINVAL   error
 *  \details
 *  Dispatch ioctl that composer supported.
 */
static long core_ioctl(struct file *filep,
		unsigned int cmd, unsigned long arg)
{
	int rc  = -EINVAL;
	unsigned int dir = _IOC_DIR(cmd);
	unsigned int sz  = _IOC_SIZE(cmd);
	struct composer_fh *fh;
	unsigned int    parg_size;
	void   *parg;

	DBGENTER("filep:%p cmd:0x%x arg:0x%lx\n", filep, cmd, arg);

	fh = (struct composer_fh *)filep->private_data;
	parg = fh->ioctl_args;
	parg_size = CORE_IOCTL_MAX_ARG_LENGTH * 4;

/*********************/
/* Prologue of IOCTL */
/*********************/
	if ((sz != 0) && ((dir & (_IOC_WRITE|_IOC_READ)) != 0)) {
		if (sz >= parg_size) {
			printk_err2("ioctl argument size too large\n");
			goto err_exit;
		}
	}

	if ((sz != 0) && ((dir & _IOC_WRITE) != 0)) {
		printk_dbg2(3, "copy_from_user\n");

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != copy_from_user(parg, (void __user *)arg, sz)) {
			printk_err2("fail in copy_from_user\n");
			goto err_exit;
		}
	}

	printk_dbg2(3, "down\n");
	down(&fh->fh_sem);

	switch (cmd) {
	case CMP_IOC_POST:
		rc = ioc_post(fh, parg);
		break;
	case CMP_IOCGS_REGISTER:
		rc = iocgs_register(fh, parg);
		break;
	default:
		printk_err2("invalid cmd 0x%x\n", cmd);
		break;
	}

	up(&fh->fh_sem);

/*********************/
/* Epilogue of IOCTL */
/*********************/
	if ((rc == 0) && (sz != 0) && ((dir & _IOC_READ) != 0)) {
		printk_dbg2(3, "copy_to_user\n");

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != copy_to_user((void __user *)arg, parg, sz)) {
			printk_err2("fail in copy_to_user\n");
			rc = -EINVAL;
		}
	}
err_exit:

	DBGLEAVE("%d\n", rc);
	return rc;
}

/*! \brief open operation
 *  \param[in] inode  pointer to structure inode
 *  \param[in] filep  pointer to structure file
 *  \return result of processing.
 *  \retval 0         normal
 *  \retval -ENODEV   error count of open exceed limits.
 *  \retval -ENOMEM   error can not allocate memory.
 *  \details
 *  Allocate private memory and open device.
 */
static int core_open(struct inode *inode, struct file *filep)
{
	int rc = 0;
	struct composer_fh *private_fh;

	TRACE_ENTER(FUNC_OPEN);
	DBGENTER("inode:%p filep:%p\n", inode, filep);

	if (NULL != inode) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}

	printk_dbg2(3, "down\n");
	down(&sem);
	if (num_open >= MAX_OPEN) {
		/* set return code */
		printk_err("reach the upper limit of open\n");
		rc = -ENODEV;
	} else {
		/* increment open count */
		num_open++;
	}
	up(&sem);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != rc) {
		/* return by error */
		goto err_exit;
	}
	printk_dbg2(3, "current num of opens:%d\n", num_open);

	/* allocate per-filehandle data */
	private_fh = allocate_device();
	if (private_fh == NULL) {

		printk_dbg2(3, "down\n");
		down(&sem);

		num_open--;

		up(&sem);

		rc = -ENOMEM;
	} else {
		filep->private_data = private_fh;
	}
err_exit:
	TRACE_LEAVE(FUNC_OPEN);
	DBGLEAVE("%d\n", rc);
	return rc;
}

/*! \brief close operation
 *  \param[in] inode  pointer to structure inode
 *  \param[in] filep  pointer to structure file
 *  \return result of processing.
 *  \retval 0         normal
 *  \details
 *  Free private memory and close device.\n
 */
static int core_release(struct inode *inode, struct file *filep)
{
	struct composer_fh *fh;

	TRACE_ENTER(FUNC_CLOSE);
	DBGENTER("inode:%p filep:%p\n", inode, filep);

	if (NULL != inode) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}

	fh = (struct composer_fh *)filep->private_data;

	free_device(fh);

	printk_dbg2(3, "down\n");
	down(&sem);

	num_open--;

	up(&sem);
	printk_dbg2(3, "current num of opens:%d\n", num_open);

	TRACE_LEAVE(FUNC_CLOSE);
	DBGLEAVE("%d\n", 0);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
/*! \brief suspend operation
 *  \param[in] h  pointer to structure early_suspend
 *  \return none
 *  \details
 *  Update composer states.\n
 *  Wait completion of on going composition,
 *  and close a handle for blend.
 */
static void pm_early_suspend(struct early_suspend *h)
{
	DBGENTER("h:%p\n", h);
	in_early_suspend = true;
	composer_prohibited_count = NUM_OF_COMPOSER_PROHIBITE_AT_RESUME;

	{
		int rc;
		size_t i;
		unsigned long blendunit;

		/***************************************
		* release RTAPI resource,
		***************************************/
		printk_dbg2(3, "down\n");
		down(&sem);

		for (i = 0; i < ARRAY_SIZE(workqueue); i++) {
			blendunit = i;
			rc = indirect_call(workqueue[i],
				work_delete_handle, 1, &blendunit);

			if (rc) {
				/* error */
				printk_err(
					"failed to release graphic handle\n");
			}
		}

		up(&sem);
	}

	/* confirm complete of request queue. */
	if (CMP_OK != rhandle_checkcomplete(WAITCOMP_WAITTIME)) {
		/* only report error, may be fence problem. */
		printk_err("time out, continue.\n");
	}

#if CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT > 1
	printk_dbg2(3, "suspend state:%d graphic_handle:%p %p\n",
		in_early_suspend, graphic_handle[0], graphic_handle[1]);
#else
	printk_dbg2(3, "suspend state:%d graphic_handle:%p\n",
		in_early_suspend, graphic_handle[0]);
#endif
	/* nothing to do */
	DBGLEAVE("\n");
	return;
}

/*! \brief resume operation
 *  \param[in] h  pointer to structure early_suspend
 *  \return none
 *  \details
 *  Update composer states.
 */
static void pm_late_resume(struct early_suspend *h)
{
	DBGENTER("h:%p\n", h);
	down(&sem);

	in_early_suspend = false;

	/* graphic_handle is created at next draw. */

	up(&sem);
	printk_dbg2(3, "suspend state:%d\n", in_early_suspend);
	DBGLEAVE("\n");
	return;
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

/******************************************************/
/* module initialize function                         */
/******************************************************/
/*! \brief init driver
 *  \return result or processing
 *  \retval 0  normal
 *  \retval -ENOMEM error, can not alloc memory.
 *  \details
 *  Initialize composer driver.
 */
static int __init rcar_composer_init(void)
{
	int ret;
	size_t i;

	DBGENTER("\n");

	/*****************************/
	/* initialize driver static  */
	/*****************************/
	spin_lock_init(&irqlock);
	sema_init(&sem, 1);
	num_open = 0;
	for (i = 0; i < ARRAY_SIZE(graphic_handle); i++)
		graphic_handle[i] = NULL;

#if FEATURE_USE_ASYNC_WAIT
	/* register fence listener */
	fence_async_wait_register_listener(&rcar_composer_allsyncready);
#endif

	sema_init(&kernel_queue_sem, 1);
	init_waitqueue_head(&kernel_waitqueue_comp);

	/* initialize function call */
	indirect_call_init();

#if INTERNAL_DEBUG
	internal_log_msg = kmalloc(INTERNAL_LOG_MSG_SIZE, GFP_KERNEL);
	if (internal_log_msg) {
		/* record available memory size */
		internal_log_msgsize = INTERNAL_LOG_MSG_SIZE;
	}
#endif

	/* Linux standard workqueue can not be used,
	   because RT-API requires a single thread where PID never changed. */

	/* create workqueue */
	for (i = 0; i < ARRAY_SIZE(workqueue); i++) {
		char taskname[64];

		/* static analysis tool message (4:3200):           */
		/* in order to make it clear that did not check     */
		/* the return value of snprintf, cast to void       */
		(void) snprintf(taskname, sizeof(taskname), "rcar_cmp%d", i);

		workqueue[i] = localworkqueue_create(taskname, 0);
		if (workqueue[i] == NULL) {
			printk_err("fail to workqueue_create\n");
			ret = -ENOMEM;
			goto err_exit;
		}
	}

	/* create workqueue */
	workqueue_schedule = localworkqueue_create("rcar_cmpsc", 2);
	if (workqueue_schedule == NULL) {
		printk_err("fail to workqueue_create");
		ret = -ENOMEM;
		goto err_exit;
	}

	/* register device */
	ret = misc_register(&composer_device);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != ret) {
		printk_err("fail to misc_register (MISC_DYNAMIC_MINOR)\n");
		goto err_exit;
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	in_early_suspend = false;
	register_early_suspend(&early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

#if CONFIG_MISC_R_CAR_COMPOSER_DEVATTR
	devattr_init(composer_fops.owner);
#endif

	rhandlemodule_init();

	/* initialize debug module */
	debugmodule_init();

	DBGLEAVE("%d\n", 0);
	return 0;

err_exit:
	for (i = 0; i < ARRAY_SIZE(workqueue); i++) {
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != workqueue[i]) {
			localworkqueue_destroy(workqueue[i]);
			workqueue[i] = NULL;
		}
	}
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != workqueue_schedule) {
		localworkqueue_destroy(workqueue_schedule);
		workqueue_schedule = NULL;
	}

	DBGLEAVE("%d\n", ret);
	return ret;
}

/*! \brief exit driver
 *  \return result or processing
 *  \retval none
 *  \details
 *  Free all resources used in composer driver.
 */
static void __exit rcar_composer_release(void)
{
	size_t i;

	DBGENTER("\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	if (num_open > 0)
		printk_err("there is 'not close device'.\n");

	/* wait all request completed. */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (CMP_OK != rhandle_checkcomplete(WAITCOMP_WAITTIME)) {
		/* fatal error, following operation not guranteed. */
		printk_err("forcely unload.\n");
	}

	/* unregist device */
#if CONFIG_MISC_R_CAR_COMPOSER_DEVATTR
	devattr_exit(composer_fops.owner);
#endif

	/* static analysis tool message (4:3200): */
	/* all function with return value         */
	/* require confirm it is no-error         */
	if (0 != misc_deregister(&composer_device)) {
		/* this condition never become true.          */
		printk_err2("fail to misc_deregister\n");
	}

	/* release all resources */

	/* wait complete flush in workqueue. */
	for (i = 0; i < ARRAY_SIZE(workqueue); i++) {
		/* flush workqueue */
		localworkqueue_flush(workqueue[i]);
	}
	localworkqueue_flush(workqueue_schedule);

	/* delete graphics handle */
	for (i = 0; i < ARRAY_SIZE(workqueue); i++) {
		int           rc;
		unsigned long blendunit = i;

		rc = indirect_call(workqueue[i],
			&work_delete_handle, 1, &blendunit);

		/* from static analysis tool message(4:3344).          */
		/* Conditional expression require a boolean expression */
		if (0 != rc) {
			/* report error */
			printk_err2("can not delete handle.\n");
		}
	}

	rhandlemodule_exit();

	/* destroy workqueue */
	for (i = 0; i < ARRAY_SIZE(workqueue); i++) {
		localworkqueue_destroy(workqueue[i]);
		workqueue[i] = NULL;
	}
	localworkqueue_destroy(workqueue_schedule);
	workqueue_schedule = NULL;

#if INTERNAL_DEBUG >= 1
	if (internal_log_msg) {
		/* free memory */
		kfree(internal_log_msg);
		internal_log_msgsize = 0;
	}
#endif

	/* initialize debug module */
	debugmodule_exit();

	DBGLEAVE("\n");
	return;
}

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "RCar Composer debug level");
MODULE_LICENSE("GPL");
module_init(rcar_composer_init);
module_exit(rcar_composer_release);
