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
#include <linux/spinlock.h>

#include "linux/rcar_composer.h"
#include "inc/debug.h"

#if _TIM_DBG
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#endif

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#if INTERNAL_DEBUG
#include <linux/slab.h>
#include <linux/seq_file.h>
#endif

#if _ATR_DBG
#include "atrace/atrace.h"
#endif /* _ATR_DBG */

/******************************************************/
/* define prototype                                   */
/******************************************************/
#if _LOG_DBG > 0
static int raw_dump_screen_grap_image_param(
	char p[], int n, struct screen_grap_image_param *arg, char *name);
static int raw_dump_screen_grap_layer(
	char p[], int n, struct screen_grap_layer *arg, char *name);
static int raw_dump_screen_grap_image_blend(char p[], int n,
	struct screen_grap_image_blend *arg);
#endif

#if _ATR_DBG
static void atrace_log_thread(struct kthread_work *work);
static int atrace_logtask_thread(void *arg);
static int atrace_logtask_init(void);
#endif

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
static int internal_debug_static_show(struct seq_file *s, void *unused);
static int internal_debug_static_open(struct inode *inode, struct file *file);
static int internal_debug_queue_show(struct seq_file *s, void *unused);
static int internal_debug_queue_open(struct inode *inode, struct file *file);
static int internal_debug_trace_show(struct seq_file *s, void *unused);
static int internal_debug_trace_open(struct inode *inode, struct file *file);
static int internal_debug_init(struct dentry *entry);
#endif
#endif

/******************************************************/
/* define local define                                */
/******************************************************/
#if INTERNAL_DEBUG
/*! max entry for trace log. */
#define TRACELOG_SIZE         128
#endif

/* define for tracelog_record function. */
/*! shift count to extract log class. */
#define TRACELOG_RECORD_VALUE0_SHIFT_TO_LOGCLASS  24

/*! macro to append debug message. */
#define DBGMSG_APPEND_NEXT \
do { \
	if (c < 0) {            \
		c = 0; /* ignore */ \
	} else if (c < n) {     \
		p_index += c;       \
		n   -= c;           \
	} else {                \
		printk_err2("size not enough\n"); \
	} \
} while (0)

/*! macro to append debug message. */
#define DBGMSG_APPEND(...) \
do { \
	c = snprintf(&p[p_index], (size_t)n, __VA_ARGS__); \
	DBGMSG_APPEND_NEXT;                                \
} while (0)

/******************************************************/
/* define local variables                             */
/******************************************************/
#if INTERNAL_DEBUG
/*! flag of already initialize of tracelog. */
static int         init_flag;

/*! trace log buffer */
static int         log_tracebuf[TRACELOG_SIZE][3];
/*! exclusive control for trace log */
static spinlock_t  log_irqlock;
/*! next store location of trace log */
static int         log_tracebuf_wp;
#endif

#if _TIM_DBG
struct record_time_info {
	int use_slot;     /*!< mask of valid information in time array. */
	ktime_t time[16]; /*!< ktime_t information. */
};

#ifdef CONFIG_DEBUG_FS
/*! max record of time information. */
#define MAX_TIMERECORD_DATA   8
/*! wait queue for time information. */
static DECLARE_WAIT_QUEUE_HEAD(timerecord_waitdata);
/*! read point for time information. */
static int                     timerecord_rp;
/*! write point for time information. */
static int                     timerecord_wp;
/*! num of records for time information. */
static int                     timerecord_count;
/*! data for time information. */
static struct record_time_info timerecord_data[MAX_TIMERECORD_DATA];
/*! num of sysfs opened for time information. */
static int                     timerecord_opencount;
/*! exclusive control for time information. */
static spinlock_t              timerecord_lock;
#endif
#endif

#if _ATR_DBG
/*! thread of recotd atrace */
static struct task_struct     *atrace_task;
/*! task of recotd atrace */
static DEFINE_KTHREAD_WORKER(atrace_worker);
#endif

#ifdef CONFIG_DEBUG_FS
#if _TIM_DBG | INTERNAL_DEBUG
/*! debug-fs entry */
struct dentry *debug_dentry;
#endif
#endif

#if _TIM_DBG
static int timerecord_createmessage(struct record_time_info *handle,
	char p[], int n);
#ifdef CONFIG_DEBUG_FS
static ssize_t timerecord_read(struct file *filp, char __user *buf,
		size_t sz, loff_t *off);
static int timerecord_open(struct inode *inode, struct file *filep);
static int timerecord_release(struct inode *inode, struct file *filep);
static int timerecord_debugfs_init(struct dentry *entry);
#endif
#endif

/******************************************************/
/* define global variables                            */
/******************************************************/

/******************************************************/
/* local functions                                    */
/******************************************************/
#if INTERNAL_DEBUG
/********************
 initialize
********************/
/******************************************************/
/*! \brief initialize tracelog
 *  \return none
 *  \details
 *  initialize global variables uses for tracelog.\n
 *  this function is available if INTERNAL_DEBUG is 1.
 *  \attention
 *  this function should call before call rcar_composer_tracelog_record().
 */
static void rcar_composer_tracelog_init(void)
{
	/* reset debug level */
	log_tracebuf_wp = 0;
	spin_lock_init(&log_irqlock);

	init_flag = 1;
}

/********************
 record
********************/
/******************************************************/
/*! \brief record trace
 *  \param[in] logclass  type of log.
 *  \param[in] line  line number on source code.
 *  \param[in] ID  ID for tracelog.
 *  \param[in] val  reserved.
 *  \return none
 *  \details
 *  record trace log with logclass line ID and value.\n
 *  this function is available if INTERNAL_DEBUG is 1.\n
 *  syntax of trace log.\n
 *  [0] (logclass << 24) | line\n
 *  [1] ID\n
 *  [2] val\n
 *  four type of loclass is defined.\n
 *  ID_TRACE_ENTER\p   functino start\n
 *  ID_TRACE_LEAVE   function leave\n
 *  ID_TRACE_LOG     log no argument\n
 *  ID_TRACE_LOG1    log 1 argument\n
 *  \attention
 *  do not allow to call this function directly. use next macros\n
 *  TRACE_ENTER, TRACE_LEAVE, TRACE_LOG, TRACE_LOG1.
 */
void rcar_composer_tracelog_record(int logclass, int line, int ID, int val)
{
	unsigned long flags;

	/* check initialized */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 == init_flag)
		return;

	/* record information */
	spin_lock_irqsave(&log_irqlock, flags);
	log_tracebuf[log_tracebuf_wp][0] =
		(logclass<<TRACELOG_RECORD_VALUE0_SHIFT_TO_LOGCLASS) |
		(line);
	log_tracebuf[log_tracebuf_wp][1] = ID;
	log_tracebuf[log_tracebuf_wp][2] = val;
	log_tracebuf_wp = (log_tracebuf_wp+1) & (TRACELOG_SIZE-1);
	spin_unlock_irqrestore(&log_irqlock, flags);
}
#endif

/*******************************
 RT-API debug log for graphics
*******************************/

#if _LOG_DBG > 0
/* maximum message length is 170 character */
/******************************************************/
/*! \brief dump structure screen_grap_image_param
 *  \param[in] p  pointer to message buffer.
 *  \param[in] n  size of message buffer.
 *  \param[in] arg  pointer to structure screen_grap_image_param.
 *  \param[in] name name of property
 *  \return size of message.
 *  \details
 *  dump debug log of structure screen_grap_image_param.\n
 *  this function is available if _LOG_DBG is grater equal 1.
 */
static int raw_dump_screen_grap_image_param(
	char p[], int n, struct screen_grap_image_param *arg, char *name)
{
	int c;
/* from static analysis tool message (4:0488),        */
/* Pointer arithmetic of p+p_index is causes warning, */
/* so declare of p changed from pointer to array      */
	int  p_index      = 0;

	DBGMSG_APPEND("[%6s]", name);
	DBGMSG_APPEND(" width:%4d height:%4d stride:%4d stride_c:%4d ",
		arg->width, arg->height, arg->stride, arg->stride_c);
	DBGMSG_APPEND("format:%2d yuv_format:%1d yuv_range:%1d ",
		arg->format, arg->yuv_format, arg->yuv_range);
	DBGMSG_APPEND("address:%p address_c0:%p address_c1:%p",
		arg->address, arg->address_c0, arg->address_c1);
	return p_index;
}

/* maximum message length is 350 character */
/******************************************************/
/*! \brief dump structure screen_grap_layer
 *  \param[in] p  pointer to message buffer.
 *  \param[in] n  size of message buffer.
 *  \param[in] arg  pointer to structure screen_grap_layer.
 *  \param[in] name name of property
 *  \return size of message.
 *  \details
 *  dump debug log of structure screen_grap_layer.\n
 *  this function is available if _LOG_DBG is grater equal 1.
 */
static int raw_dump_screen_grap_layer(
	char p[], int n, struct screen_grap_layer *arg, char *name)
{
	int c;
/* from static analysis tool message (4:0488),        */
/* Pointer arithmetic of p+p_index is causes warning, */
/* so declare of p changed from pointer to array      */
	int  p_index      = 0;

	if (arg == NULL) {
		/* layer not opened */
		return 0;
	}

	/* maximum message length is 210 character */
	c = raw_dump_screen_grap_image_param(&p[p_index], n,
		&arg->image, name);
	DBGMSG_APPEND_NEXT;

	/* maximum message length is 140 character */
	DBGMSG_APPEND(" rect(x:%4d y:%4d width:%4d height:%4d) ",
		arg->rect.x, arg->rect.y, arg->rect.width, arg->rect.height);
	DBGMSG_APPEND("alpha:%3d rotate:%1d mirror:%1d key_color:0x%8lx ",
		arg->alpha, arg->rotate, arg->mirror, arg->key_color);
	DBGMSG_APPEND("premultiplied:%1d alpha_coef:%1d",
		arg->premultiplied, arg->alpha_coef);
	return p_index;
}

/* maximum message length is 370 character */
/******************************************************/
/*! \brief dump structure screen_grap_image_blend
 *  \param[in] p  pointer to message buffer.
 *  \param[in] n  size of message buffer.
 *  \param[in] arg  pointer to structure screen_grap_image_blend.
 *  \return size of message.
 *  \details
 *  dump debug log of structure screen_grap_image_blend.\n
 *  this function is available if _LOG_DBG is grater equal 1.
 */
static int raw_dump_screen_grap_image_blend(char p[], int n,
	struct screen_grap_image_blend *arg)
{
	int c;
/* from static analysis tool message (4:0488),        */
/* Pointer arithmetic of p+p_index is causes warning, */
/* so declare of p changed from pointer to array      */
	int  p_index      = 0;

	/* maximum message length is 160 character */
	DBGMSG_APPEND("handle:%p input_layer:(%p %p %p %p) ",
		arg->handle, arg->input_layer[0], arg->input_layer[1],
		arg->input_layer[2], arg->input_layer[3]);
	DBGMSG_APPEND("background_color:0x%08lx user_data:0x%08lx\n",
		arg->background_color, arg->user_data);

	/* maximum message length is 210 character */
	DBGMSG_APPEND("    ");
	c = raw_dump_screen_grap_image_param(&p[p_index], n,
		&arg->output_image, "output");
	DBGMSG_APPEND_NEXT;
	return p_index;
}

/******************************************************/
/*! \brief print structure screen_grap_image_blend
 *  \param[in] arg  pointer to structure screen_grap_image_blend.
 *  \return none.
 *  \details
 *  print information of structure screen_grap_image_blend.\n
 *  this function is available if _LOG_DBG is grater equal 1.
 */
static void dump_screen_grap_image_blend(struct screen_grap_image_blend *arg)
{
	static char msg[370];

	/* static analysis tool message (4:3200): */
	/* all function with return value         */
	/* require confirm it is no-error         */
	if (raw_dump_screen_grap_image_blend(&msg[0], sizeof(msg), arg) >
		(int)sizeof(msg)) {
		/* this condition never become true.          */
		printk_err2("size of msg buffer not enough\n");
	}
	printk_lowdbg("    %s\n", &msg[0]);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != raw_dump_screen_grap_layer(&msg[0], sizeof(msg),
		arg->input_layer[0], "layer0")) {
		/* report log */
		printk_lowdbg("    %s\n", &msg[0]);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != raw_dump_screen_grap_layer(&msg[0], sizeof(msg),
		arg->input_layer[1], "layer1")) {
		/* report log */
		printk_lowdbg("    %s\n", &msg[0]);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != raw_dump_screen_grap_layer(&msg[0], sizeof(msg),
		arg->input_layer[2], "layer2")) {
		/* report log */
		printk_lowdbg("    %s\n", &msg[0]);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != raw_dump_screen_grap_layer(&msg[0], sizeof(msg),
		arg->input_layer[3], "layer3")) {
		/* report log */
		printk_lowdbg("    %s\n", &msg[0]);
	}
}

/******************************************************/
/*! \brief print structure screen_grap_delete
 *  \param[in] arg  pointer to structure screen_grap_delete.
 *  \return none.
 *  \details
 *  print information of structure screen_grap_delete.\n
 *  this function is available if _LOG_DBG is grater equal 1.
 */
static void dump_screen_grap_delete(struct screen_grap_delete *arg)
{
	printk_lowdbg("  handle:%p\n", arg->handle);
}
#endif

/******************************************************/
/*! \brief return string of result of screen_graphics_blend
 *  \param[in] rc  return value of screen_graphics_blend
 *  \return readable string correspond to argument rc.
 *  \details
 *  return string of result of screen_graphics_blend.\n
 *  if argument is unknown then return string "unknown imgpctrl error".
 */
static const char *get_imgpctrlmsg_graphics(int rc)
{
	const char *msg = "unknown imgpctrl error";
	switch (rc) {
	case SMAP_LIB_GRAPHICS_OK:
		msg = "SMAP_LIB_GRAPHICS_OK";
		break;
	case SMAP_LIB_GRAPHICS_NG:
		msg = "SMAP_LIB_GRAPHICS_NG";
		break;
	case SMAP_LIB_GRAPHICS_PARAERR:
		msg = "SMAP_LIB_GRAPHICS_PARAERR";
		break;
	case SMAP_LIB_GRAPHICS_SEQERR:
		msg = "SMAP_LIB_GRAPHICS_SEQERR";
		break;
	case SMAP_LIB_GRAPHICS_MEMERR:
		msg = "SMAP_LIB_GRAPHICS_MEMERR";
		break;
	default:
		/* nothing to do */
		break;
	}
	return msg;
}

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
/******************************************************/
/*! \brief create debug message of structure composer_rh.
 *  \param[in] p  pointer to message buffer.
 *  \param[in] n  size of message buffer.
 *  \param[in] rh  pointer to structure composer_rh.
 *  \return size of message.
 *  \details
 *  generate debug log of structure composer_rh.\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int rcar_composer_dump_rhandle(char p[], int n,
	struct composer_rh *rh)
{
	int           i, c;
/* from static analysis tool message (4:0488),        */
/* Pointer arithmetic of p+p_index is causes warning, */
/* so declare of p changed from pointer to array      */
	int  p_index      = 0;

	if ((0 == rh->rh_wqtask.status) &&
		(0 == rh->active)) {
		DBGMSG_APPEND("  not used\n");
		goto finish;
	}

	/* variable */
	DBGMSG_APPEND("  active:%d\n", rh->active);
	DBGMSG_APPEND("  ref_count:%d\n", rh->refcount);
	DBGMSG_APPEND("  kfref:%d\n", (int)atomic_read(&rh->ref.refcount));
	DBGMSG_APPEND("  user_data:%p user_callback:%p\n",
		rh->user_data, rh->user_callback);
	DBGMSG_APPEND("  num_buffer:%d\n", rh->num_buffer);
	for (i = 0; i < CMP_DATA_NUM_GRAP_LAYER; i++) {
		DBGMSG_APPEND("  buf%d buffer_address:0x%lx\n",
			i, rh->buffer_address[i]);
	}
	DBGMSG_APPEND("  wait.fence:%p", rh->wait.fence);
	DBGMSG_APPEND("  swfence:%p\n", rh->swfence);
	DBGMSG_APPEND("  sequence_id:%u, dev_id:%u\n",
		rh->seq_number, rh->dev_number);
	DBGMSG_APPEND("  use_blendunit:%u\n", rh->use_blendunit);

	/* blend parameter */
	DBGMSG_APPEND("  blend_data\n");
	{
		struct cmp_data_compose_blend *data;

		data = &rh->data;
		DBGMSG_APPEND("    valid:%d\n", data->valid);

		/* delete incorrect function call under _LOG_DBG==0. */
#if _LOG_DBG > 0
		DBGMSG_APPEND("    ");
		c = raw_dump_screen_grap_image_blend(&p[p_index],
			n, &data->blend);
		DBGMSG_APPEND_NEXT;

		DBGMSG_APPEND("\n    ");
		c = raw_dump_screen_grap_layer(&p[p_index], n,
			&data->layer[0], "layer0");
		DBGMSG_APPEND_NEXT;

		DBGMSG_APPEND("\n    ");
		c = raw_dump_screen_grap_layer(&p[p_index], n,
			&data->layer[1], "layer1");
		DBGMSG_APPEND_NEXT;

		DBGMSG_APPEND("\n    ");
		c = raw_dump_screen_grap_layer(&p[p_index], n,
			&data->layer[2], "layer2");
		DBGMSG_APPEND_NEXT;

		DBGMSG_APPEND("\n    ");
		c = raw_dump_screen_grap_layer(&p[p_index], n,
			&data->layer[3], "layer3");
		DBGMSG_APPEND_NEXT;

		DBGMSG_APPEND("\n");
#endif
	}

finish:;
	return p_index;
}
#endif
#endif


#if _TIM_DBG
/******************************************************/
/*! \brief create handle to recordtime
 *  \return handle of recordtime
 *  \retval 0 error
 *  \retval others handle of recordtime
 *  \details
 *  allocate memory and initialize structure record_time_info.\n
 *  this function is available if _TIM_DBG is 1.
 */
static void *timerecord_createhandle(void)
{
	struct record_time_info *handle;

	handle = kzalloc(sizeof(struct record_time_info), GFP_KERNEL);

	return handle;
}

/******************************************************/
/*! \brief delete handle to recordtime
 *  \param [in] arg handle for timerecord
 *  \return none
 *  \details
 *  free resources allocated in timerecord_createhandle function.
 *  this function is available if _TIM_DBG is 1.
 */
static void timerecord_deletehandle(void *arg)
{
	kfree(arg);
	return;
}

/******************************************************/
/*! \brief clear all record
 *  \param [in] arg handle for timerecord
 *  \return none
 *  \details
 *  clear all record information.\n
 *  this function is available if _TIM_DBG is 1.
 */
static void timerecord_reset(void *arg)
{
	if (NULL != arg) {
		struct record_time_info *handle;

		handle = (struct record_time_info *)arg;
		handle->use_slot = 0;
	}
}

/******************************************************/
/*! \brief record time
 *  \param[in] arg handle for timerecord
 *  \param[in] id  slot number used
 *  \return time in nano-sec
 *  \details
 *  record time and store time.\n
 *  this function is available if _TIM_DBG is 1.
 */
static unsigned int timerecord_record(void *arg, int id)
{
	unsigned int rc = 0;

	if (id < 0 || id >= 16) {
		printk_err("slot number %d invalid\n", id);
		return 0;
	}

	if (NULL != arg) {
		struct record_time_info *handle;

		handle = (struct record_time_info *)arg;

#ifdef CONFIG_DEBUG_FS
		spin_lock(&timerecord_lock);
#endif
		if (0 != (handle->use_slot & (1<<id))) {
			/* error report */
			printk_err("slot number %d overwrite\n", id);
		}
		handle->use_slot |= (1<<id);
#ifdef CONFIG_DEBUG_FS
		spin_unlock(&timerecord_lock);
#endif

		handle->time[id] = ktime_get();

		rc = ktime_to_ns(handle->time[id]);
	}
	return rc;
}

/******************************************************/
/*! \brief create message of timerecord handle
 *  \param[in] handle  pointer to structure record_time_info.
 *  \param[in] p  pointer to message buffer.
 *  \param[in] n  size of message buffer.
 *  \return size of message.
 *  \details
 *  create message of timerecord handle.\n
 *  this function is available if _TIM_DBG is 1.
 */
static int timerecord_createmessage(struct record_time_info *handle,
	char p[], int n)
{
	int i;
	int c;
	unsigned long long start_time;
/* from static analysis tool message (4:0488),        */
/* Pointer arithmetic of p+p_index is causes warning, */
/* so declare of p changed from pointer to array      */
	int  p_index      = 0;

	for (i = 0; i < 16; i++) {
		if (0 != (handle->use_slot & (1<<i))) {
			/* get time */
			start_time = ktime_to_ns(handle->time[i]);
			break;
		}
	}

	if (i == 16) {
		DBGMSG_APPEND("no record\n");
		return p_index;
	}

	DBGMSG_APPEND("%d:%lld ", i, start_time);

	i++;

	for (; i < 16; i++) {
		if (0 != (handle->use_slot & (1<<i))) {
			DBGMSG_APPEND("%d:%lld ", i,
				ktime_to_ns(handle->time[i]) - start_time);
			if (c >= n) {
				printk_err("no space\n");
				break;
			}
		}
	}
	DBGMSG_APPEND("\n");

	return p_index;
}

/******************************************************/
/*! \brief timerecord print
 *  \param[in] arg  pointer to recordtime handle
 *  \return none
 *  \details
 *  if sysfs is opened, then add information to FIFO queue.
 *  if FIFO is not enough then print error and discard result.\n
 *  this function is available if _TIM_DBG is 1.
 */
static void timerecord_print(void *arg)
{
	struct record_time_info *handle = (struct record_time_info *)arg;
	if (NULL != arg) {
#ifdef CONFIG_DEBUG_FS
		if (0 == timerecord_opencount) {
			/* nothing to do */
			/* reader of timerecord is not opened. */
		} else if (timerecord_count >= MAX_TIMERECORD_DATA) {
			/* error report */
			printk_err("no space to recordtime\n");
		} else {
			/* record information */
			timerecord_data[timerecord_wp] = *handle;
			timerecord_wp = (timerecord_wp + 1) &
				(MAX_TIMERECORD_DATA - 1);
			timerecord_count++;
			wake_up_interruptible(&timerecord_waitdata);
		}
#else
		char msg[512];
		timerecord_createmessage(handle, &msg[0], sizeof(msg)-1);
		printk_lowdbg("%s\n" , &msg[0]);
#endif
	}
}

#ifdef CONFIG_DEBUG_FS
/******************************************************/
/*! \brief read for timerecord
 *  \param[in] filp  pointer to structure file
 *  \param[in] buf   pointer to user buffer
 *  \param[in] sz    size of user buffer
 *  \param[in] off   offset information.
 *  \return size of message
 *  \details
 *  wait timerecord information queued in FIFO, and
 *  create message by timerecord_createmessage().\n
 *  this function is available if _TIM_DBG is 1.
 */
static ssize_t timerecord_read(struct file *filp __maybe_unused,
		char __user *buf,
		size_t sz __maybe_unused, loff_t *off __maybe_unused)
{
	char msg[512];
	int c = 0;

	if (timerecord_count == 0) {
		if (wait_event_interruptible(timerecord_waitdata,
			(timerecord_rp != timerecord_wp) ||
			(timerecord_count == MAX_TIMERECORD_DATA)) < 0) {
			/* interrupt unexpected. report error. */
			printk_err2("failed in wait_event_interruptible\n");
		}
		if (0 != signal_pending(current)) {
			/* flush pending signal */
			flush_signals(current);
		}
	}

	if (0 != timerecord_count) {
		struct record_time_info *handle;

		handle = &timerecord_data[timerecord_rp];
		timerecord_rp = (timerecord_rp + 1) &
			(MAX_TIMERECORD_DATA - 1);

		c = timerecord_createmessage(handle, &msg[0], sizeof(msg)-1);

		timerecord_count--;

		if (0 != copy_to_user(buf, &msg[0], c)) {
			/* error in copy to user space. */
			c = 0;
		}
	}
	return c;
}

/******************************************************/
/*! \brief open for timerecord
 *  \param[in] inode  pointer to structure inode
 *  \param[in] filep  pointer to structure file
 *  \return result of processing
 *  \details
 *  increment open count.
 *  this function is available if _TIM_DBG is 1.
 */
static int timerecord_open(struct inode *inode __maybe_unused,
	struct file *filep __maybe_unused)
{
	/* nothing to do */
	timerecord_opencount++;
	return 0;
}

/******************************************************/
/*! \brief close for timerecord
 *  \param[in] inode  pointer to structure inode
 *  \param[in] filep  pointer to structure file
 *  \return result of processing
 *  \details
 *  decrement open count.
 *  this function is available if _TIM_DBG is 1.
 */
static int timerecord_release(struct inode *inode __maybe_unused,
	struct file *filep __maybe_unused)
{
	/* nothing to do */
	timerecord_opencount--;
	return 0;
}

static const struct file_operations timerecord_debugfs_fops = {
	.open           = timerecord_open,
	.read           = timerecord_read,
	.release        = timerecord_release,
};

/******************************************************/
/*! \brief initialize to use debugfs
 *  \param[in] entry  parent of debugfs
 *  \return result of processing
 *  \retval 0.  always return 0.
 *  \details
 *  create debugfs that name "processtime".\n
 *  this function is available if _TIM_DBG is 1.
 */
static int timerecord_debugfs_init(struct dentry *entry)
{
	spin_lock_init(&timerecord_lock);
	debugfs_create_file("processtime", S_IRUGO,
		entry, NULL, &timerecord_debugfs_fops);
	return 0;
}
#endif
#endif

#if _ATR_DBG
struct atrace_work {
	struct kthread_work work; /*!< task for atrace log */
	char   name[32];          /*!< name of atrace log */
	int    arg0;              /*!< id   of atrace log */
	int    arg1;              /*!< arg  of atrace log */
};

/******************************************************/
/*! \brief task for atrace
 *  \param[in] work  pointer to ktherad_work
 *  \return none
 *  \details
 *  indirectly generate atrace log.\n
 *  log is created from atrace_work structure.\n
 *  this function is available if _ATR_DBG is 1.
 *  \sa ::rcar_composer_atracelog
 */
static void atrace_log_thread(struct kthread_work *work)
{
	struct atrace_work *log;

	log = container_of(work, struct atrace_work, work);
	switch (log->arg0) {
	case 0:
		ATRACE_END(log->name);
		break;
	case 1:
		ATRACE_BEGIN(log->name);
		break;
	case 2:
		ATRACE_INT(log->name, log->arg1);
		break;
	default:
		printk_err("arg0 not support\n");
		break;
	}
}

/******************************************************/
/*! \brief generate atrace log
 *  \param[in] func  name of traces.
 *  \param[in] id    type of traces.
 *  \param[in] arg   arg  of traces.
 *  \return none
 *  \details
 *  when id is 0. generate atrace log of end function.\n
 *  when id is 1. generate atrace log of start function.\n
 *  when id is 2. generate atrace log of integer of values.\n
 *  \attention
 *  do not allow to call this function directly. use next macros\n
 *  R_ATRACE_BEGIN, R_ATRACE_END, R_ATRACE_INT.
 */
void rcar_composer_atracelog(char *func, int id, int arg)
{
	struct atrace_work work;

	if (NULL == atrace_task) {
		/* nothing to do */
		printk_err2("initialize incomplete.");
	} else {
		init_kthread_work(&work.work, atrace_log_thread);

		/* static analysis tool message (4:3200):           */
		/* in order to make it clear that did not check     */
		/* the return value of snprintf, cast to void       */
		(void) snprintf(work.name, 32, "%.30s", func);
		work.arg0 = id;
		work.arg1 = arg;

		queue_kthread_work(&atrace_worker, &work.work);
		flush_kthread_work(&work.work);
	}
}
EXPORT_SYMBOL(rcar_composer_atracelog);

/******************************************************/
/*! \brief thread for atrace
 *  \return result of processing.
 *  \retval 0  normal result.
 *  \details
 *  set schduler as FIFO.\n
 *  and call kthread_worker_fn to use standard kthread_work.\n
 *  this function is available if _ATR_DBG is 1.
 */
static int atrace_logtask_thread(void *arg)
{
	struct sched_param param = {.sched_priority = 1 };
	if (sched_setscheduler(current, SCHED_FIFO, &param) < 0) {
		/* ignore error, continue opeartaion */
		printk_err("failed to sched_setscheduler\n");
	}

	while (0 == kthread_should_stop()) {
		kthread_worker_fn(arg);

		/* ignore all signal */
		if (0 != signal_pending(current))
			flush_signals(current);
	}
	return 0;
}

/******************************************************/
/*! \brief create task for atrace
 *  \return result of processing.
 *  \retval 0  normal result.
 *  \details
 *  create thread.\n
 *  this function is available if _ATR_DBG is 1.
 */
static int atrace_logtask_init(void)
{
	atrace_task = kthread_run(atrace_logtask_thread,
		&atrace_worker, "rcar_cmptr");
	if (false != IS_ERR(atrace_task)) {
		printk_err("can not create dbg_thread task.\n");
		atrace_task = NULL;
	}
	return 0;
}
#endif

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
/******************************************************/
/*! \brief show debugfs for static.
 *  \param[in] s  pointer to structure seq_file
 *  \param[in] unused   pointer to void
 *  \return result of processing
 *  \retval 0 always return 0.
 *  \details
 *  debug log is created in rcar_composer_debug_info_static().\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_static_show(struct seq_file *s,
	void *unused __maybe_unused)
{
	rcar_composer_debug_info_static(s);
	return 0;
}

/******************************************************/
/*! \brief open debugfs for static.
 *  \param[in] inode  pointer to structure inode
 *  \param[in] file   pointer to structure file
 *  \return result of processing
 *  \details
 *  return result of single_open.\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_static_open(struct inode *inode, struct file *file)
{
	return single_open(file, internal_debug_static_show, inode->i_private);
}

/******************************************************/
/*! \brief show debugfs for queue.
 *  \param[in] s  pointer to structure seq_file
 *  \param[in] unused  pointer to void
 *  \return result of processing
 *  \retval 0 always return 0.
 *  \details
 *  debug log is created in rcar_composer_debug_info_queue().\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_queue_show(struct seq_file *s,
	void *unused __maybe_unused)
{
	rcar_composer_debug_info_queue(s);
	return 0;
}

/******************************************************/
/*! \brief open debugfs for queue.
 *  \param[in] inode  pointer to structure inode
 *  \param[in] file   pointer to structure file
 *  \return result of processing
 *  \details
 *  return result of single_open.\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_queue_open(struct inode *inode, struct file *file)
{
	return single_open(file, internal_debug_queue_show, inode->i_private);
}

/******************************************************/
/*! \brief show debugfs for trace.
 *  \param[in] s  pointer to structure seq_file
 *  \param[in] unused   pointer to void
 *  \return result of processing
 *  \retval 0 always return 0.
 *  \details
 *  generate trace result.\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_trace_show(struct seq_file *s,
	void *unused __maybe_unused)
{
	int i, rp;
	unsigned long flags;
	int    (*tracelog)[TRACELOG_SIZE][3];

	/* check initialized */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 == init_flag)
		return 0;

	tracelog = (int (*)[TRACELOG_SIZE][3]) kmalloc(sizeof(log_tracebuf),
		GFP_KERNEL);
	if (NULL == tracelog) {
		printk_err("error at kmalloc\n");
		return 0;
	}

	/* create copy of traces */
	spin_lock_irqsave(&log_irqlock, flags);

	memcpy(tracelog, &log_tracebuf[0][0], sizeof(*tracelog));
	rp = log_tracebuf_wp & (TRACELOG_SIZE-1);

	spin_unlock_irqrestore(&log_irqlock, flags);

	/* create tracelog message */
	for (i = 0; i < TRACELOG_SIZE; i++) {
		int logclass = (*tracelog)[rp][0]>>
			TRACELOG_RECORD_VALUE0_SHIFT_TO_LOGCLASS;
		int logline  = (*tracelog)[rp][0] & 0xffffff;
		switch (logclass) {
		case ID_TRACE_ENTER:
			seq_printf(s, "[0x%03x:ent:%d]",
				(*tracelog)[rp][1], logline);
			break;
		case ID_TRACE_LEAVE:
			seq_printf(s, "[0x%03x:lev:%d]",
				(*tracelog)[rp][1], logline);
			break;
		case ID_TRACE_LOG:
			seq_printf(s, "[0x%03x:%d]",
				(*tracelog)[rp][1], logline);
			break;
		case ID_TRACE_LOG1:
			seq_printf(s, "[0x%03x:%d:%d]",
				(*tracelog)[rp][1], logline,
				(*tracelog)[rp][2]);
			break;
		default:
			printk_err2("logclass not defined.");
			/* no log message */
			break;
		}
		rp = (rp + 1) & (TRACELOG_SIZE-1);
	}
	kfree(tracelog);

	return 0;
}

/******************************************************/
/*! \brief open debugfs for trace.
 *  \param[in] inode  pointer to structure inode
 *  \param[in] file   pointer to structure file
 *  \return result of processing
 *  \details
 *  return result of single_open.
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_trace_open(struct inode *inode, struct file *file)
{
	return single_open(file, internal_debug_trace_show, inode->i_private);
}

static const struct file_operations internal_debug_static_debugfs_fops = {
	.open           = internal_debug_static_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static const struct file_operations internal_debug_queue_debugfs_fops = {
	.open           = internal_debug_queue_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static const struct file_operations internal_debug_trace_debugfs_fops = {
	.open           = internal_debug_trace_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

/******************************************************/
/*! \brief initialize to use debugfs
 *  \param[in] entry  parent of debugfs
 *  \return result of processing
 *  \retval 0.  always return 0.
 *  \details
 *  create debugfs that name "static", "queeu", "trace".\n
 *  this function is available if INTERNAL_DEBUG is 1.
 */
static int internal_debug_init(struct dentry *entry)
{
	debugfs_create_file("static", S_IRUGO, entry, NULL,
		&internal_debug_static_debugfs_fops);
	debugfs_create_file("queue", S_IRUGO, entry, NULL,
		&internal_debug_queue_debugfs_fops);
	debugfs_create_file("trace", S_IRUGO, entry, NULL,
		&internal_debug_trace_debugfs_fops);
	return 0;
}
#endif
#endif

/******************************************************/
/*! \brief initialize debug module
 *  \return none
 *  \details
 *  execute initialization process of debug module.
 */
static void debugmodule_init(void)
{
#if INTERNAL_DEBUG
	rcar_composer_tracelog_init();
#endif

#ifdef CONFIG_DEBUG_FS
#if _TIM_DBG | INTERNAL_DEBUG
	debug_dentry = debugfs_create_dir("composer", NULL);
#endif
#if _TIM_DBG
	timerecord_debugfs_init(debug_dentry);
#endif
#if INTERNAL_DEBUG
	internal_debug_init(debug_dentry);
#endif
#endif

#if _ATR_DBG
	atrace_logtask_init();
#endif
}

/******************************************************/
/*! \brief exit debug module
 *  \return none
 *  \details
 *  release resources acquired in debugmodule_init().
 */
static void debugmodule_exit(void)
{
#ifdef CONFIG_DEBUG_FS
#if _TIM_DBG | INTERNAL_DEBUG
	debugfs_remove_recursive(debug_dentry);
#endif
#endif
#if _ATR_DBG
	if (NULL != atrace_task) {
		if (0 != kthread_stop(atrace_task)) {
			/* It is meaningless to this check.   */
			/* nothing to do                      */

			/* static analysis tool message (4:3200): */
			/* all function with return value         */
			/* require confirm it is no-error         */
		}
	}
#endif
}

