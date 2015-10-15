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
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/spinlock.h>

#include "inc/debug.h"
#include "inc/swfence.h"

/******************************************************/
/* define prototype                                   */
/******************************************************/

#if FEATURE_USE_ASYNC_WAIT
static void callback_fencewait(struct sync_fence *fence,
	struct sync_fence_waiter *waiter);
#endif
static void fence_release(struct kref *kref);

/******************************************************/
/* define local define                                */
/******************************************************/

/******************************************************/
/* define local variables                             */
/******************************************************/
#if FEATURE_USE_ASYNC_WAIT
static void (*callback_fencewait_notify)(struct composer_rh *rh, int id);
#endif

/******************************************************/
/* local functions                                    */
/******************************************************/

#if FEATURE_USE_ASYNC_WAIT
/*! \brief callback entry for async wait.
 *  \param[in] fence  pointer to fence object
 *  \param[in] waiter  pointer to fence waiter object
 *  \return none
 *  \details
 *  When a fence object becomes signal state,
 *  callback this function from framework.\n
 *  If a fences signaled, notice this event by callback.
 */
static void callback_fencewait(struct sync_fence *fence,
	struct sync_fence_waiter *waiter)
{
	struct composer_rh *rh;

	DBGENTER("fence:%p waiter:%p\n", fence, waiter);

	rh = container_of(waiter, struct composer_rh, wait.waiter);

	if ((NULL != fence) && (5 <= debug)) {
		/* debug code of sync driver. */
#if _LOG_DBG >= 2
		struct list_head *pos;

		/* confirm all sync pt signaled.                  */
		/* if othere condition found, report information. */

		/* access fence object may be not safe. */
		list_for_each(pos, &fence->pt_list_head) {
			struct sync_pt *pt =
				container_of(pos, struct sync_pt, pt_list);
			const char *pt_name;

			if (NULL != pt->parent->name) {
				/* Get the Timeline name that belongs sync_pt */
				pt_name = &pt->parent->name[0];
			} else {
				/* no-name availabel */
				pt_name = "";
			}

			if (pt->status < 0) {
				printk_dbg2(3,
"sync_pt %s signaled by error\n", pt_name);
			} else if (pt->status > 0) {
				printk_dbg2(3,
"sync_pt %s signaled by normal\n", pt_name);
			} else {
				/* sync driver has problem, should fix-bug. */
				printk_err(
"fence become signal, but sync_pt %s active.\n", pt_name);
			}
		}
#endif
	}

	/* notify sync object signaled. */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != callback_fencewait_notify) {
		/* issue callback */
		callback_fencewait_notify(rh, 0);
	}

	DBGLEAVE("\n");
}
#endif

/*! \brief get fence from postdata.
 *  \param[in] rh    pointer to composer_rh
 *  \param[in] post  pointer to cmp_postdata
 *  \return result of processing.
 *  \retval CMP_OK  success
 *  \retval CMP_NG  error, there is some problem.
 *  \details
 *  Get fences from post->acquire_fd.\n
 *  Handles of async wait are partially initialized.
 *  \msc
 *    sync, waiter, composer;
 *    |||;
 *    --- [label="set fence"];
 *    composer box composer
 *        [label="get fence", URL="\ref ::fence_config"];
 *    composer box composer
 *        [label="async wait", URL="\ref ::fence_async_wait"];
 *    composer -> waiter
 *        [label="sync_fence_wait_async"];
 *    --- [label="waiting singal"];
 *    |||;
 *    sync -> waiter
 *        [label="fence become signal"];
 *    |||;
 *   waiter -> composer
 *        [label="callback if a fence become signal"];
 *    composer box composer
 *          [label="put fence", URL="\ref ::fence_expire"];
 *  \endmsc
 *  \attention
 *  If waiter is used, following operation does not work correctly.
 */
static int fence_config(struct composer_rh *rh, struct cmp_postdata *post)
{
	int rc = CMP_OK;
	int fd;

	DBGENTER("rh:%p post:%p\n", rh, post);

	/* clear fence. */
#if FEATURE_USE_ASYNC_WAIT
	sync_fence_waiter_init(&rh->wait.waiter,
		&callback_fencewait);
#endif
	rh->wait.fence = NULL;

	/* set information */
	fd = post->acquire_fd;

	if (fd >= 0) {
		struct sync_fence *fence;

		fence = sync_fence_fdget(fd);

		if (fence == NULL) {
			/* nothing to do */
			printk_err1("ignore invalid acquire_fd.\n");
		} else {
			/* record fence */
			rh->wait.fence = fence;
		}
	}

	printk_dbg2(3, "fence:%p\n", rh->wait.fence);

	DBGLEAVE("%d\n", rc);
	return rc;
}

/*! \brief free fence
 *  \param[in] rh        a pointer to composer_rh structure.
 *  \return result of processing
 *  \retval CMP_OK  success
 *  \details
 *  All fences acquired in fence_config are closed.
 *  And cancel all fence waiters to reuse safely.
 */
static int fence_expire(struct composer_rh *rh)
{
	struct sync_fence *fence;
#if FEATURE_USE_ASYNC_WAIT
	struct sync_fence_waiter *waiter;
#endif

	DBGENTER("rh:%p\n", rh);

	/* all sync object released without wait. */
	printk_dbg2(3, "sync handle released.\n");

	fence = rh->wait.fence;
#if FEATURE_USE_ASYNC_WAIT
	waiter = &rh->wait.waiter;
#endif
	rh->wait.fence = NULL;

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != fence) {
#if FEATURE_USE_ASYNC_WAIT
		if (sync_fence_cancel_async(fence, waiter) == 0) {
			printk_err("cancel wait fence.");

			/* notify sync object signaled. */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			if (NULL != callback_fencewait_notify) {
				/* issue callback */
				callback_fencewait_notify(rh, 0);
			}
		} else {
			/* report log */
			printk_dbg2(3, "sync_fence_cancel_async fail.\n");
		}
#endif
		/* report log */
		printk_dbg2(3, "sync_fence_put.\n");
		sync_fence_put(fence);
	}

	return CMP_OK;
}

#if FEATURE_USE_ASYNC_WAIT
/*! \brief register notify function
 *  \param[in] cb        function pointer.
 *  \return nothing
 *  \details
 *  register notify function when fence become signal\n
 */
static void fence_async_wait_register_listener(
	void (*cb)(struct composer_rh *rh, int id))
{
	DBGENTER("cb:%p\n", cb);

	/* static analysis tool message (8:0554):                       */
	/* Because the function call has been a warning of              */
	/* undefined function call directly between functional modules, */
	/* changed to an indirect call.                                 */
	/* As a result, code efficiency is lowered.                     */
	callback_fencewait_notify = cb;

	DBGLEAVE("\n");
}

/*! \brief wait fence asynchronously
 *  \param[in] rh        a pointer to composer_rh structure.
 *  \return result of processing
 *  \retval CMP_OK  success. always return this value.
 *  \details
 *  Set up waiter to wait fences asynchronously.\n
 *  If async wait is not needed, execute callback in this function.
 */
static int fence_async_wait(struct composer_rh *rh)
{
	struct sync_fence *fence;
	struct sync_fence_waiter *waiter;

	DBGENTER("rh:%p\n", rh);

	fence = rh->wait.fence;
	waiter = &rh->wait.waiter;

	if (fence == NULL) {
		/* not found valid sync object. */
		goto finish;
	}

	/* async wait */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != sync_fence_wait_async(fence, waiter)) {
		/* already signaled or error */
		fence = NULL;
	}

finish:
	if (fence == NULL) {
		/* notify sync object signaled. */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != callback_fencewait_notify) {
			/* issue callback */
			callback_fencewait_notify(rh, 0);
		}
	}

	DBGLEAVE("\n");
	return CMP_OK;
}
#else
static int fence_wait(struct composer_rh *rh, int timeout, int usage)
{
	int rc = CMP_OK;
	struct sync_fence *fence;

	DBGENTER("rh:%p timeout:%d usage:%d\n", rh, timeout, usage);

	/* wait sync_fence signaled. */
	fence = rh->wait.fence;
	if (fence) {
		if (sync_fence_wait(fence, timeout) < 0) {
			/* report error */
			printk_err("sync_wait error.");
		}
	}

	/* close sync_fence execute at fence_expire */

	DBGLEAVE("%d\n", rc);
	return rc;
}
#endif

/*! \brief create handle for support fence
 *  \return handle of async wait
 *  \retval NULL   can not alloc memory.
 *  \retval others handle to support fence
 *  \details
 *  Create a handle to support fence.\n
 *  This handle is used to notice blend complete timing.
 *  \msc
 *    composer, fence;
 *    |||;
 *    --- [label="initialize"];
 *    composer box composer
 *        [label="create handle", URL="\ref ::fence_get_handle"];
 *    --- [label="create fence"];
 *    composer box composer
 *        [label="increment time be signaled", URL="\ref ::fence_inc_timeline"];
 *    composer box composer
 *        [label="create sync fence", URL="\ref ::fence_get_syncfd"];
 *    composer->fence
 *        [label="create fence."];
 *    --- [label="fence signaled"];
 *    composer box composer
 *        [label="fence signal", URL="\ref ::fence_signal"];
 *    composer->fence
 *        [label="fence become signaled."];
 *    --- [label="finalize"];
 *    composer box composer
 *        [label="close handle", URL="\ref ::fence_put_handle"];
 *  \endmsc
 */
static struct sw_fence_handle *fence_get_handle(void)
{
	struct sw_fence_handle *handle;

	DBGENTER("\n");

	handle = kzalloc(sizeof(struct sw_fence_handle), GFP_KERNEL);
	if (handle == NULL) {
		/* no memory */
		printk_err2("memory allocation failed.\n");
	} else {
		kref_init(&handle->kref);
		handle->timeline = sw_sync_timeline_create("composer_sync");
		handle->timeline_count = 0; /* count of increment */
		handle->timeline_inc   = 0; /* count of signal    */
	}

	DBGLEAVE("%p\n", handle);
	return handle;
}

/*! \brief release resource for support fence
 *  \param[in] kref pointer to handle.
 *  \return none
 *  \details
 *  Destroy timeline and release resource that used to support fence.
 */
static void fence_release(struct kref *kref)
{
	struct sw_fence_handle *handle;

	DBGENTER("kref:%p\n", kref);

	handle = container_of(kref, struct sw_fence_handle, kref);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != handle->timeline) {
		sync_timeline_destroy(&handle->timeline->obj);
		handle->timeline = NULL;
	}
	kfree(handle);

	DBGLEAVE("\n");
	return;
}

/*! \brief duplicate handle for support fence
 *  \param[in] handle pointer to handle.
 *  \return duplicate handle.
 *  \details
 *  Increment reference to use duplicate handle.
 */
static struct sw_fence_handle *fence_dup_handle(struct sw_fence_handle *handle)
{
	DBGENTER("handle:%p\n", handle);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != handle) {
		kref_get(&handle->kref);

		printk_dbg2(3, "kref:%d\n",
			atomic_read(&handle->kref.refcount));
	}

	DBGLEAVE("%p\n", handle);

	return handle;
}

/*! \brief close handle for support fence
 *  \param[in] handle pointer to a handle.
 *  \return none
 *  \details
 *  Decrement reference to close handle.
 */
static void  fence_put_handle(struct sw_fence_handle *handle)
{
	DBGENTER("handle:%p\n", handle);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != handle) {
		printk_dbg2(3, "kref:%d\n",
			atomic_read(&handle->kref.refcount));

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != kref_put(&handle->kref, &fence_release)) {
			/* generate log */
			printk_dbg2(3, "sw_fence_handle removed\n");
		}
	}

	DBGLEAVE("\n");
}

/*! \brief create fence
 *  \param[in] handle pointer to handle.
 *  \return file descriptor to detect composition complete.
 *  \retval -1        error.
 *  \retval "0 or positive" success. return value is file descriptor.
 *  \details
 *  create fence to be signaled that the timeline of
 *  sw_sync become handle->timeline_count.
 */
static int fence_get_syncfd(struct sw_fence_handle *handle)
{
	int sync_fd = -1;
	struct sw_sync_timeline  *timeline;
	unsigned int             timeline_count;

	DBGENTER("handle:%p\n", handle);

	if (handle == NULL) {
		printk_dbg1(3, "handle is NULL.\n");
		goto err;
	}
	timeline       = handle->timeline;
	timeline_count = handle->timeline_count;

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != timeline) {
		struct sync_pt *pt = NULL;
		int rc = -EINVAL;
		int fd;

		do {
			struct sync_fence *fence;

			fd = get_unused_fd();
			if (fd < 0) {
				printk_dbg2(3, "get_unused_fd failed\n");
				break;
			}

			pt = sw_sync_pt_create(timeline, timeline_count);
			if (pt == NULL) {
				printk_dbg2(3, "sw_sync_pt_create failed\n");
				rc = -ENOMEM;
				break;
			}

			fence = sync_fence_create("composer_fence", pt);
			if (fence == NULL) {
				printk_dbg2(3, "sync_fence_create failed\n");
				rc = -ENOMEM;
				break;
			}

			sync_fence_install(fence, fd);

			rc = 0;
			sync_fd = fd;
		} while (0);

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (0 != rc) {
			/* free_resource */
			printk_dbg1(3, "iocg_sw_sync failed\n");

			if (NULL != pt) {
				/* free sw_sync_pt_create */
				sync_pt_free(pt);
			}
			if (fd >= 0) {
				/* free get_unused_fd */
				put_unused_fd((unsigned int)fd);
			}
		}
	}
err:

	DBGLEAVE("%d\n", sync_fd);
	return sync_fd;
}

/*! \brief increment target time
 *  \param[in] handle pointer to handle.
 *  \return result of processing.
 *  \retval CMP_OK success.
 *  \details
 *  increment handle->timeline_count.\n
 *  this member used in ::fence_get_syncfd
 */
static int fence_inc_timeline(struct sw_fence_handle *handle)
{
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != handle) {
		struct sw_sync_timeline  *timeline = handle->timeline;
		unsigned long flags;

		spin_lock_irqsave(&timeline->obj.child_list_lock, flags);
		handle->timeline_count++;
		spin_unlock_irqrestore(&timeline->obj.child_list_lock, flags);
	}

	return CMP_OK;
}

/*! \brief increment timeline
 *  \param[in] handle pointer to handle.
 *  \return result of processing.
 *  \retval CMP_OK success.
 *  \details
 *  Increment timeline and fence will be signaled.\n
 *  Member of handle->timeline_inc is only used for debug.
 */
static int fence_signal(struct sw_fence_handle *handle)
{
	struct sw_sync_timeline  *timeline;
	unsigned int timeline_count;
	unsigned int timeline_inc;

	DBGENTER("handle:%p\n", handle);

	if (NULL == handle) {
		printk_dbg1(3, "handle is NULL.\n");
		goto err;
	}

	timeline       = handle->timeline;
	timeline_count = handle->timeline_count;
	timeline_inc   = handle->timeline_inc;

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != timeline) {
		sw_sync_timeline_inc(timeline, 1);

		handle->timeline_inc++;
		timeline_inc   = handle->timeline_inc;
	}

	printk_dbg1(3, "sw_sync pt_value:%d, value:%d\n",
		timeline_count, timeline_inc);
err:

	DBGLEAVE("\n");
	return CMP_OK;
}

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
/*! \brief create debug message of structure fencewait_handle.
 *  \param[in] s  pointer to structure seq_file.
 *  \param[in] rh pointer to structure composer_rh.
 *  \return none
 *  \details
 *  Generate information of structure fencewait_handle.\n
 *  And generate fence sync information that composer waiting.
 *  This function is available if INTERNAL_DEBUG is 1.
 *  \attention
 *  There is race-condition between closing fence and generating logs.
 *  At worst case, kernel panic occurs. But information of fence is
 *  useful to debug problem of fence.
 */
static void rcar_composer_debug_info_fencewait_handle(struct seq_file *s,
	struct composer_rh *rh)
{
	struct list_head *pos;
	struct sync_fence *fence;
	unsigned long flags;

	fence = rh->wait.fence;

	if (NULL != fence) {
		seq_puts(s,   "  fencewaitinfo\n");
		seq_printf(s, "  status:%d\n", fence->status);

/* access fence object may be not safe. */
		list_for_each(pos, &fence->pt_list_head) {
			struct sync_pt *pt =
				container_of(pos, struct sync_pt, pt_list);
			int pt_status;
			struct timeval pt_tv;
			char *pt_name;

/* get fence information */
			spin_lock_irqsave(&fence->waiter_list_lock, flags);
			pt_status = pt->status;
			if (NULL != pt->parent->name)
				pt_name = &pt->parent->name[0];
			else
				pt_name = "";
			if (0 != pt_status) {
				/* get time */
				pt_tv = ktime_to_timeval(pt->timestamp);
			}
			spin_unlock_irqrestore(&fence->waiter_list_lock, flags);

/* print info */
			seq_printf(s, " %s_pt %d ", pt_name, pt_status);

			if (0 != pt_status) {
				seq_printf(s, "@%ld.%06ld",
					pt_tv.tv_sec, pt_tv.tv_usec);
			}

			if ((NULL != pt->parent->ops->timeline_value_str) &&
				(NULL != pt->parent->ops->pt_value_str)) {
				char value[64];
				pt->parent->ops->pt_value_str(pt, value,
					sizeof(value));
				seq_printf(s, ": %s", value);

				pt->parent->ops->timeline_value_str(
					pt->parent, value, sizeof(value));
				seq_printf(s, " / %s", value);
			} else if ((NULL != pt->parent->ops->print_pt) &&
				(NULL != pt->parent->ops->print_obj)) {
				seq_puts(s, ": ");
				pt->parent->ops->print_pt(s, pt);
				seq_puts(s, "/ ");
				pt->parent->ops->print_obj(s, pt->parent);
			}

			seq_puts(s, "\n");
		}
	}
}

/*! \brief create debug message of structure sw_fence_handle.
 *  \param[in] s  pointer to structure seq_file.
 *  \param[in] handle pointer to structure sw_fence_handle.
 *  \return none
 *  \details
 *  generate information of structure sw_fence_handle.\n
 *  this function is available if INTERNAL_DEBUG is 1.
 *  \attention
 *  There is race-condition between composition complete and generating logs.
 *  at worst case, kernel panic occurs. But information of timeline is
 *  useful to debug problem of fence.
 */
static void rcar_composer_debug_info_fence_handle(struct seq_file *s,
	struct sw_fence_handle *handle)
{
	seq_printf(s, "  sw_fence_handle:%p\n", handle);
	if (NULL != handle) {
		seq_printf(s, "    timeline_count:%d\n",
			handle->timeline_count);
		seq_printf(s, "    timeline_inc:%d\n",
			handle->timeline_inc);
	}
}

#endif
#endif

/******************************************************/
/* global functions                                   */
/******************************************************/

