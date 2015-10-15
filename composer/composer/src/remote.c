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
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

#include "inc/remote.h"
#include "inc/debug.h"

/******************************************************/
/* define prototype                                   */
/******************************************************/
static void work_indirectcall(struct localwork *work);

/******************************************************/
/* define local define                                */
/******************************************************/
/* this module uses follow process */
/* - destroy blend handle          */

/*! max allowable remote task */
#define MAX_REMOTE_TASK  8

/******************************************************/
/* define local variables                             */
/******************************************************/
/*! struct composer_indirectcall */
static struct composer_indirectcall _request_indirect_call[MAX_REMOTE_TASK];
/*! top of unused list of _request_indirect_call array. */
static LIST_HEAD(_free_entry_indirect_call);
/*! exclusive control to handle _free_entry_indirect_call. */
static spinlock_t _lock_indirect_call;

/******************************************************/
/* local functions                                    */
/******************************************************/

/*! \brief initialize function of indirect call.
 *  \return none
 *  \details
 *  initialize function of indirectly call functions.\n
 *  the function is callback from specified workqueue.\n
 *  this process is not use dynamically memory allocations.
 *  \attention
 *  this function should call before call indirect_call function.
 *  num of available resources is limited by MAX_REMOTE_TASK.
 */
static void indirect_call_init(void)
{
	int i;
	struct composer_indirectcall *rr;
	DBGENTER("\n");

	spin_lock_init(&_lock_indirect_call);
	INIT_LIST_HEAD(&_free_entry_indirect_call);

	for (i = 0; i < MAX_REMOTE_TASK; i++) {
		rr = &_request_indirect_call[i];
		localwork_init(&rr->wqtask, &work_indirectcall);
		INIT_LIST_HEAD(&rr->list);

		list_add_tail(&rr->list,
			&_free_entry_indirect_call);
	}
	DBGLEAVE("\n");
}

/*! \brief task to implement indirect call.
 *  \param[in] work  pointer to struct localwork.
 *  \return none
 *  \details
 *  call function specified by struct composer_indirectcall.
 *  and store result of function call.
 */
static void work_indirectcall(struct localwork *work)
{
	struct composer_indirectcall *rr;

	DBGENTER("work:%p\n", work);

	rr = container_of(work, struct composer_indirectcall, wqtask);

	printk_dbg2(3, "rr->function:%p rr->args:0x%lx,0x%lx\n",
		rr->remote, rr->args[0], rr->args[1]);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != rr->remote) {
		/* do remote function call */
		rr->args[0] = (unsigned long)rr->remote(&rr->args[0]);
	}
	DBGLEAVE("\n");
}

/*! \brief indirect call.
 *  \param[in] queue  pointer to struct localworkqueue.
 *  \param[in] func  function pointer to call indirectly.
 *  \param[in] num_arg  number of argument. valid range is from 0 to 4.
 *  \param[in] args  array of argument.
 *  \return result of function call.
 *  \retval -1     no resource available.
 *  \retval others result of function. normally 0.
 *  \details
 *  get unused resource of struct composer_indirectcall
 *  and construct task and schedule to run task within workqueue.\n
 *  wait complete task and return the result of function call.
 *  \msc
 *    composer, remote, workqueue;
 *    |||;
 *    --- [label="initialize"];
 *    composer=>remote
 *        [label="init", URL="\ref ::indirect_call_init"];
 *    --- [label="indirect call"];
 *    composer=>remote
 *        [label="call", URL="\ref ::indirect_call"];
 *    remote->workqueue
 *        [label="queue task", URL="\ref ::localwork_queue"];
 *    workqueue box workqueue
 *        [label="call specified function"];
 *    remote->workqueue
 *        [label="wait complete task", URL="\ref ::localwork_flush"];
 *    composer<=remote
 *        [label="result of call"];
 *  \endmsc
 */
static int indirect_call(struct localworkqueue *queue,
	int (*func)(unsigned long *args),
	int num_arg,
	unsigned long *args)
{
	struct composer_indirectcall *rr = NULL;
	int rc = -1;
	int i;

	DBGENTER("queue:%p func:%p num:%d args:%p\n",
		queue, func, num_arg, args);

	if (num_arg > 4) {
		printk_err("invalid argument\n");
		goto err;
	}

	/* get free entry */
	spin_lock(&_lock_indirect_call);
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false == list_empty(&_free_entry_indirect_call)) {
		rr = list_first_entry(&_free_entry_indirect_call,
			struct composer_indirectcall, list);
		list_del_init(&rr->list);
	}
	spin_unlock(&_lock_indirect_call);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == rr) {
		printk_err("no available entry\n");
		goto err;
	}

	rr->remote = func;
	for (i = 0; i < num_arg; i++)
		rr->args[i] = args[i];

	rc = localwork_queue(queue, &rr->wqtask);
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false != rc) {
		/* wait work compete. */
		localwork_flush(&rr->wqtask);

		rc = (int)rr->args[0];
	} else {
		printk_err("failed to call function\n");
		rc = -1;
	}

	/* add free entry */
	spin_lock(&_lock_indirect_call);
	list_add_tail(&rr->list,
		&_free_entry_indirect_call);
	spin_unlock(&_lock_indirect_call);
err:
	DBGLEAVE("rc:%d\n", rc);
	return rc;
}
