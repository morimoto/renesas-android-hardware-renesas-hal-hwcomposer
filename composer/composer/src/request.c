/*
 * Function        : Composer driver
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#include "linux/rcar_composer.h"
#include "inc/work.h"
#include "inc/debug.h"
#include "inc/request.h"


/******************************************************/
/* define prototype                                   */
/******************************************************/
static inline int _rhandle_free_task(void *unused);
static void _rhandle_delete(struct kref *kref);
static int _rhandle_check_allcomplete(void);


/******************************************************/
/* define local variables                             */
/******************************************************/
static DEFINE_SPINLOCK(rhlock);
static LIST_HEAD(rhlist_top);

static struct task_struct *rhfree_task;
static DECLARE_WAIT_QUEUE_HEAD(rhfree_wait);
static LIST_HEAD(rhfree_top);

/*! \brief task to free request handle
 *  \param[in] unused   pointer to void
 *  \return processing result.
 *  \retval 0  always return this value.
 *  \details
 *  this function free resources used for request handle.
 */
static inline int _rhandle_free_task(void *unused)
{
	unsigned long flags;

	DBGENTER("unused:%p\n", unused);

	if (NULL != unused) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}

	/* do not change default priority. */

	while (true) {
		/* confirm terminate condition */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if ((0 != kthread_should_stop()) &&
			(false != list_empty(&rhfree_top))) {
			/* terminate loop */
			break;
		}

		/* wait request to free request handle */
		if (wait_event_interruptible(rhfree_wait,
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			(false == list_empty(&rhfree_top)) ||
			(0 != kthread_should_stop())) < 0) {
			/* interrupt unexpected. report error. */
			printk_err2("failed in wait_event_interruptible\n");
		}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false == list_empty(&rhfree_top)) {
			struct composer_rh *rh;

#if _ATR_DBG
			ATRACE_BEGIN("free");
#endif
			/* free request handle */

			rh = list_first_entry(&rhfree_top,
				struct composer_rh, rh_link);

			/* unlink list. */
			spin_lock_irqsave(&rhlock, flags);
			list_del_init(&rh->rh_link);
			spin_unlock_irqrestore(&rhlock, flags);

			/* to wait task not running, do flush. */
			localwork_flush(&rh->rh_wqtask);
			localwork_flush(&rh->rh_wqtask_schedule);

#if _TIM_DBG
			timerecord_deletehandle(rh->timerecord);
#endif

			kfree(rh);

#if _ATR_DBG
			ATRACE_END("free");
#endif
		}
	}

	DBGLEAVE("\n");
	return 0;
}

/*! \brief delete request handle
 *  \param[in] kref  pointer to kref structure
 *  \return none
 *  \details
 *  this function is called when reference become zero.
 */
static void _rhandle_delete(struct kref *kref)
{
	struct composer_rh *rh;
	unsigned long flags;
	int err_flag = 0;

	DBGENTER("kref:%p\n", kref);

	rh = container_of(kref, struct composer_rh, ref);

	printk_dbg2(3, "rh:%p free\n", rh);

	/* unlink list. */
	spin_lock_irqsave(&rhlock, flags);
	list_del_init(&rh->rh_link);
	spin_unlock_irqrestore(&rhlock, flags);

	/* confirm valid sequence. */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false == list_empty(&rh->fh_link)) {
		err_flag = 1;
		printk_dbg2(3, "fh_link active\n");
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rh->ionmem) {
		err_flag = 1;
		printk_dbg2(3, "ionmem active\n");
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rh->swfence) {
		err_flag = 1;
		printk_dbg2(3, "swfence active\n");
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rh->wait.fence) {
		err_flag = 1;
		printk_dbg2(3, "fence active\n");
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if ((false != rh->active) || (0 != rh->refcount)) {
		err_flag = 1;
		printk_dbg2(3, "active state\n");
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != err_flag) {
		/* report error */
		printk_err("invalid implement found.\n");
	}


	spin_lock_irqsave(&rhlock, flags);
	/* unlink list. */
	list_del_init(&rh->fh_link);
	/* schedule to free resource */
	list_add(&rh->rh_link, &rhfree_top);
	spin_unlock_irqrestore(&rhlock, flags);

	/* wake-up */
	wake_up(&rhfree_wait);

	DBGLEAVE("");
}

/*! \brief increment reference of request handle
 *  \param[in] rh  pointer to composer_rh structure
 *  \return none
 *  \details
 *  increment reference.
 *  this function is called from rhandle_getunusedhandle.
 */
static void rhandle_incref(struct composer_rh *rh)
{
	DBGENTER("rh:%p\n", rh);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 == kref_get_unless_zero(&rh->ref)) {
		/* requested handle already free. */
		printk_err("invalid sequence\n");
	}

	printk_dbg2(3, "rh:%p after incref count:%d\n",
		rh, atomic_read(&rh->ref.refcount));

	DBGLEAVE("");
}

/*! \brief decrement reference of request handle
 *  \param[in] rh  pointer to composer_rh structure
 *  \return none
 *  \details
 *  decrement reference.
 *  if reference become zero, function _rhandle_delete is called.
 *  this function is called from rhandle_putusedhandle, rhandle_free.
 */
static void rhandle_decref(struct composer_rh *rh)
{
	DBGENTER("rh:%p\n", rh);

	printk_dbg2(3, "rh:%p before decref count:%d\n",
		rh, atomic_read(&rh->ref.refcount));

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != kref_put(&rh->ref, &_rhandle_delete)) {
		/* generate log */
		printk_dbg2(3, "request handle removed\n");
	}

	DBGLEAVE("");
}

/*! \brief register request handle
 *  \param[in] fh  pointer to composer_fh structure
 *  \return pointer to composer_rh structure
 *  \retval 0     not enough memory
 *  \retval other composer_rh structure
 *  \details
 *  create request handle and initialized.
 *  and this handle register fh->fhlist_top and rhlist_top.
 *  fhlist_top is used to free handle whenc close is executed.
 *  rhlist_top is used to confirm all request finished.
 */
static struct composer_rh *rhandle_create(struct composer_fh *fh)
{
	struct composer_rh *rh;
	unsigned long flags;

	DBGENTER("fh:%p\n", fh);
	rh = kzalloc(sizeof(*rh), GFP_KERNEL);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rh) {
		printk_dbg2(3, "rh:%p create\n", rh);

		/* initialize task */
		localwork_init(&rh->rh_wqtask, &work_blend);
		localwork_init(&rh->rh_wqtask_schedule, &work_schedule);

#if INTERNAL_DEBUG || _ATR_DBG
		rh->dev_number = fh->dev_number;
#endif

#if _TIM_DBG
		rh->timerecord = timerecord_createhandle();
#endif

		/* initialize references and link. */
		kref_init(&rh->ref);
		INIT_LIST_HEAD(&rh->fh_link);
		INIT_LIST_HEAD(&rh->rh_link);

		/* add list */
		spin_lock_irqsave(&rhlock, flags);
		list_add(&rh->rh_link, &rhlist_top);
		list_add(&rh->fh_link, &fh->fhlist_top);
		spin_unlock_irqrestore(&rhlock, flags);
	} else {
		/* report error */
		printk_err("kzalloc failed\n");
	}

	DBGLEAVE("%p\n", rh);
	return rh;
}

/*! \brief free request handle
 *  \param[in] fh  pointer to composer_fh structure
 *  \return none
 *  \details
 *  free all handle related composer_fh structure.
 *  \sa ::rhandle_create
 */
static void rhandle_free(struct composer_fh *fh)
{
	struct composer_rh *rh;
	unsigned long flags;

	DBGENTER("fh:%p\n", fh);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	while (false == list_empty(&fh->fhlist_top)) {
		rh = list_first_entry(&fh->fhlist_top,
			struct composer_rh, fh_link);

		/* delete list */
		spin_lock_irqsave(&rhlock, flags);
		list_del_init(&rh->fh_link);
		spin_unlock_irqrestore(&rhlock, flags);

		rhandle_decref(rh);
	}
	DBGLEAVE("\n");
}

/*! \brief get unused request handle
 *  \param[in] fh  pointer to composer_fh structure
 *  \return pointer to composer_rh structure
 *  \retval 0     no free handle found.
 *  \retval other composer_rh structure
 *  \details
 *  return unused request handle.
 *  returned handle mark as active, untill rhandle_putusedhandle executed.
 */
static struct composer_rh *rhandle_getunusedhandle(struct composer_fh *fh)
{
	struct composer_rh *ret;
	struct composer_rh *rh;
	unsigned long flags;

	DBGENTER("fh:%p\n", fh);

	ret = NULL;
	/* search unused handle */
	spin_lock_irqsave(&rhlock, flags);
	list_for_each_entry(rh, &fh->fhlist_top, fh_link) {
		if (rh->active == false) {
			rh->active = true;
			ret = rh;
			break;
		}
	}
	spin_unlock_irqrestore(&rhlock, flags);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != ret) {
		/* increment reference */
		rhandle_incref(rh);
	}
	DBGLEAVE("%p\n", ret);
	return ret;
}

/*! \brief return used request handle
 *  \param[in] rh  pointer to composer_rh structure
 *  \return none
 *  \details
 *  used request handle return to unused.
 *  handle mark as not active.
 */
static void rhandle_putusedhandle(struct composer_rh *rh)
{
	DBGENTER("rh:%p\n", rh);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false != rh->active) {
		unsigned long flags;

		spin_lock_irqsave(&rhlock, flags);
		rh->active = false;
		spin_unlock_irqrestore(&rhlock, flags);

		rhandle_decref(rh);
	} else {
		/* report error */
		printk_err2("invalid sequence\n");
	}
	DBGLEAVE("\n");
}

/*! \brief confirm complete
 *  \return result of processing.
 *  \retval true   all composition finished.
 *  \retval false  there is pending request.
 *  \details
 *  Check all request from rhlist_top list and
 *  confirm all composition is finished.
 */
static int _rhandle_check_allcomplete(void)
{
	unsigned long flags;
	struct composer_rh *rh;
	int    busy = false;

	/* search unused handle */
	spin_lock_irqsave(&rhlock, flags);
	list_for_each_entry(rh, &rhlist_top, rh_link) {
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != rh->active) {
			busy = true;
			break;
		}
	}
	spin_unlock_irqrestore(&rhlock, flags);

	return (busy == false);
}


/*! \brief wait complete
 *  \param[in] timeout  specify wait time in mill-second
 *  \return result of processing.
 *  \retval CMP_OK   normal
 *  \retval CMP_NG   error
 *  \details
 *  Wait completion of all composition during
 *  timeout milliseconds.
 *  If timeout detected, return error.
 */
static int rhandle_checkcomplete(unsigned int timeout)
{
	int ret;

	DBGENTER("timeout:%d\n", timeout);
	ret = wait_event_timeout(kernel_waitqueue_comp,
		_rhandle_check_allcomplete(),
		(long)msecs_to_jiffies(timeout));
	if (ret == 0) {
		/* timeout */
		printk_err2("there is active request.\n");
		ret = CMP_NG;
	} else {
		/* wait success before timeout */
		ret = CMP_OK;
	}
	DBGLEAVE("\n");
	return ret;
}

/******************************************************/
/*! \brief initialize request handle module
 *  \return none
 *  \details
 *  execute initialization process of request handle module.
 */
static void rhandlemodule_init(void)
{
	DBGENTER("\n");

	rhfree_task = kthread_run(&_rhandle_free_task, NULL, "rcar_cmpfr");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false != IS_ERR(rhfree_task)) {
		/* this is fatal, it is caused resource leak and freeze. */
		printk_err("could not create kernel thread\n");

		rhfree_task = NULL;
	}

	DBGLEAVE("\n");
	return;
}

/******************************************************/
/*! \brief exit request handle module
 *  \return none
 *  \details
 *  release resources acquired in rhandlemodule_init().
 */
static void rhandlemodule_exit(void)
{
	DBGENTER("\n");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rhfree_task) {
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != kthread_stop(rhfree_task)) {
			/* It is meaningless to this check.   */
			/* nothing to do                      */

			/* static analysis tool message (4:3200): */
			/* all function with return value         */
			/* require confirm it is no-error         */
		}
	}

	DBGLEAVE("\n");
}
