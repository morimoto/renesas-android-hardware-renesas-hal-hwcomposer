/*
 * Function        : Composer driver
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
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
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/kthread.h>

#include "inc/debug.h"
#include "inc/work.h"

/******************************************************/
/* define prototype                                   */
/******************************************************/
static inline int localworkqueue_thread(void *arg);


/******************************************************/
/* define local define                                */
/******************************************************/
/*! use FIFO schedule if specify 1 */
#define USE_FIFO_SCHDULE    1

#if USE_FIFO_SCHDULE
/*! range is 1(min) to MAX_PRIO-4(max). */
#define THREAD_PRIORITY  (1)
#else
#define THREAD_NICE      (-15)
#endif

/******************************************************/
/* define local variables                             */
/******************************************************/


/******************************************************/
/* local functions                                    */
/******************************************************/

/*! \brief initialize worktask.
 *  \param[in,out] work a task of pointer to initialize
 *  \param[in] func      function of entry of task.
 *  \return none
 *  \details
 *  initialize structure to use localwork function.\n
 *  basic idea of this function is same as work_init.
 */
static void localwork_init(
	struct localwork *work, void (*func)(struct localwork *work))
{
	INIT_LIST_HEAD(&work->link);
	work->func = func;
	work->status = false;
	work->wq = NULL;
}


/*! \brief flush workqueue.
 *  \param[in] wq      pointer to workqueue
 *  \return none
 *  \details
 *  wait all task processing in specified workqueue.
 *  basic idea of this function is same as flush_workqueue.
 */
static void localworkqueue_flush(struct localworkqueue *wq)
{
	if (wq == NULL) {
		/* report error */
		printk_err("invalid argument.\n");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	} else if (false == list_empty(&wq->top)) {
		/* wait all task complete */
		printk_dbg2(3, "wait localworkqueue complete\n");

		wait_event(
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			wq->finish, false != list_empty(&wq->top));
	} else {
		/* from static analysis tool message (4:2004).         */
		/* else block require when use if-elseif statement.    */

		/* task not scheduled, so nothing to do */
	}
}

/*! \brief destroy workqueue.
 *  \param[in] wq      pointer to workqueue
 *  \return none
 *  \details
 *  to destroy workqueue request stop thread
 *  and all pending task canceled.
 *  basic idea of this function is same as destroy_workqueue.
 */
static void localworkqueue_destroy(struct localworkqueue *wq)
{
	unsigned long flags;

	if (wq == NULL) {
		/* report error */
		printk_err("invalid argument.\n");
	} else {
		/* request task stop */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != wq->task) {
			/* from static analysis tool message (4:3344). */
			/* Conditional expression require a boolean    */
			/* expression                                  */
			if (0 != kthread_stop(wq->task)) {
				/* It is meaningless to this check.   */
				/* nothing to do                      */

				/* static analysis tool message (4:3200): */
				/* all function with return value         */
				/* require confirm it is no-error         */
			}
		}

		/* wakeup pending thread */
		printk_dbg2(3, "spinlock\n");
		spin_lock_irqsave(&wq->lock, flags);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		while (false == list_empty(&wq->top)) {
			struct list_head *list;
			struct localwork *work = NULL;

			printk_dbg2(3, "localwork not empty\n");

			list_for_each(list, &wq->top)
			{
				work = list_entry(list,
					struct localwork, link);
				break;
			}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			if (NULL != work) {
				printk_dbg2(3, "localwork pending: %p\n", work);
				work->status = true;
				list_del_init(&work->link);
			}
		}
		spin_unlock_irqrestore(&wq->lock, flags);

		wake_up_interruptible_all(&wq->wait);

		kfree(wq);
	}
}


/*! \brief thread entry of workqueue.
 *  \param[in] arg      pointer to workqueue
 *  \return always return 0
 *  \retval 0.
 *  \details
 *  Main thread to handle work queued in workqueue.
 *  At first change scheduling method to FIFO.
 *  And process task in FIFO order.
 *  \msc
 *    self, task;
 *    |||;
 *    --- [label="initialize"];
 *    self box self
 *        [label="workqueue create", URL="\ref ::localworkqueue_create"];
 *    self box self
 *        [label="work initialize", URL="\ref ::localwork_init"];
 *    --- [label="task handling"];
 *    self box self [label="queue", URL="\ref ::localwork_queue"];
 *    self->task [label="start task"];
 *    self box self [label="wait complete", URL="\ref ::localwork_flush"];
 *    task box task [label="process task"];
 *    self->self [label="wait task complete"];
 *    task->self [label="end task"];
 *    --- [label="finalize"];
 *    self box self
 *          [label="workqueue flush", URL="\ref ::localworkqueue_flush"];
 *    self box self
 *          [label="workqueue destroy", URL="\ref ::localworkqueue_destroy"];
 *  \endmsc
 */
static inline int localworkqueue_thread(void *arg)
{
	struct localworkqueue *wq = (struct localworkqueue *)arg;
	unsigned long flags;

#if USE_FIFO_SCHDULE
	struct sched_param param = {.sched_priority = THREAD_PRIORITY};
	param.sched_priority += wq->priority;
	if (sched_setscheduler(current, SCHED_FIFO, &param) < 0) {
		/* ignore error, continue opeartaion */
		printk_err2("failed to sched_setscheduler\n");
	}
#else
	set_user_nice(current, THREAD_NICE - wq->priority);
#endif

	DBGENTER("\n");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	while (0 == kthread_should_stop()) {
		struct localwork *work;
		void   (*func)(struct localwork *work);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (wait_event_interruptible(wq->wait,
			(false == list_empty(&wq->top)) ||
			(0 != kthread_should_stop())) < 0) {
			/* interrupt unexpected. report error. */
			printk_err2("failed in wait_event_interruptible\n");
		}

		/* ignore all signal */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != signal_pending(current)) {
			/* this condition never become true */
			flush_signals(current);
		}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != kthread_should_stop()) {
			/* terminate loop */
			break;
		}

		printk_dbg2(3, "spinlock\n");
		spin_lock_irqsave(&wq->lock, flags);
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		while (false == list_empty(&wq->top)) {
			work = list_first_entry(&wq->top,
				struct localwork, link);

			printk_dbg2(3, "work:%p\n", work);

			func = work->func;
			spin_unlock_irqrestore(&wq->lock, flags);

			(*func)(work);

			spin_lock_irqsave(&wq->lock, flags);
			work->status = true;
			list_del_init(&work->link);
			wake_up_all(&wq->finish);
		}
		spin_unlock_irqrestore(&wq->lock, flags);
	}

	DBGLEAVE("\n");
	return 0;
}


/*! \brief create workqueue.
 *  \param[in] name       pointer to task name
 *  \param[in] priority   priority of workqueue. range 0 to 3
 *  \return pointer of workqueue
 *  \details
 *  create workqueue with priority.\n
 *  this function has following advantage against standard workqueue.\n
 *  1. support FIFO schedule workqueue with priority.\n
 *  2. always same pid used to process task.\n
 *  basic idea of this function is same as create_workqueue.
 */
static struct localworkqueue *localworkqueue_create(char *name,
	int priority)
{
	struct localworkqueue *wq;

	wq = kzalloc(sizeof(*wq), GFP_KERNEL);
	if (wq == NULL) {
		/* report error */
		printk_err("can not create localwork.\n");
	} else {
		INIT_LIST_HEAD(&wq->top);
		spin_lock_init(&wq->lock);
		init_waitqueue_head(&wq->wait);
		init_waitqueue_head(&wq->finish);
		wq->priority = priority;

		wq->task = kthread_run(&localworkqueue_thread,
				     wq,
				     name);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != IS_ERR(wq->task)) {
			printk_err("could not create kernel thread\n");
			kfree(wq);
			wq = NULL;
		}
	}
	return wq;
}


/*! \brief queue task.
 *  \param[in] wq   pointer to workqueue.
 *  \param[in] work pointer to task.
 *  \return process result
 *  \retval true   success to queue.
 *  \retval false  task already queued, or error found.
 *  \details
 *  Add task to workqueue to start task.\n
 *  basic idea of this function is same as queue_work.
 */
static int localwork_queue(
	struct localworkqueue *wq, struct localwork *work)
{
	unsigned long flags;
	int rc;
	DBGENTER("wq:%p work:%p\n", wq, work);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if ((NULL != wq) && (NULL != work)) {
		rc = true;

		spin_lock_irqsave(&wq->lock, flags);
		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != list_empty(&work->link)) {
			list_add_tail(&work->link, &wq->top);
			work->wq = wq;
			work->status = false;
		} else {
			printk_err2("work %p already queued.\n", work);
			rc = false;
		}
		spin_unlock_irqrestore(&wq->lock, flags);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != rc)
			wake_up_interruptible(&wq->wait);
	} else {
		/* set error code */
		printk_err("invalid argument.\n");
		rc = false;
	}
	DBGLEAVE("%d\n", rc);
	return rc;
}

/*! \brief wait complete task.
 *  \param[in] work pointer to task.
 *  \return none
 *  \details
 *  wait task complete.\n
 *  basic idea of this function is same as work_flush.
 */
static void localwork_flush(struct localwork *work)
{
	unsigned long flags;
#if _LOG_DBG > 1
	int rc = 0;
#endif
	DBGENTER("work:%p\n", work);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if ((NULL != work) && (NULL != work->wq)) {
		struct localworkqueue *wq = work->wq;
		int wait = false;
		spin_lock_irqsave(&wq->lock, flags);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != work->status) {
			/* wait is not necessary. */
			printk_dbg2(3, "work %p finished.\n", work);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		} else if (false != list_empty(&work->link)) {
			/* report error */
			printk_dbg2(3, "work %p not queued\n", work);
#if _LOG_DBG > 1
			rc = -EINVAL;
#endif
		} else if (current == wq->task) {
			/* report error */
			printk_err2("can not wait in same workqueue\n");
#if _LOG_DBG > 1
			rc = -EINVAL;
#endif
		} else
			wait = true;
		spin_unlock_irqrestore(&wq->lock, flags);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (false != wait) {
			printk_dbg2(3, "wait complete of work %p\n", work);
			wait_event(
				wq->finish, work->status != false);
		}
	} else if (NULL == work) {
		/* set error code */
		printk_err("invalid argument.\n");
#if _LOG_DBG > 1
		rc = -EINVAL;
#endif
	} else {
		/* from static analysis tool message (4:2004).         */
		/* else block require when use if-elseif statement.    */

		/* task not scheduled, so nothing to do */
	}
	DBGLEAVE("%d\n", rc);
	return;
}

