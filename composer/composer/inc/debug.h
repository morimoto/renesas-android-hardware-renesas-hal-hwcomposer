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
#ifndef _R_CAR_COMPOSER_DEBUG_H
#define _R_CAR_COMPOSER_DEBUG_H

#define INTERNAL_DEBUG  0
#define _TIM_DBG   0		/* record time log.                 */
#define _LOG_DBG   1		/* generate debug log.              */
#define _ERR_DBG   2		/* generate error log.              */
#define _ATR_DBG   0		/* use atrace log experimentally    */
#define DEV_NAME   "composer"

#define _EXTEND_TIMEOUT  1


#include "imgpctrl/imgpctrl_public.h"
#if INTERNAL_DEBUG
#include "linux/rcar_composer.h"
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif
#endif

#if _ATR_DBG
#include "atrace/atrace.h"

#define R_ATRACE_BEGIN(X)  rcar_composer_atracelog(X, 1, 0)
#define R_ATRACE_END(X)    rcar_composer_atracelog(X, 0, 0)
#define R_ATRACE_INT(X, Y) rcar_composer_atracelog(X, 2, Y)
#else
#define ATRACE_PID_BEGIN(P, X)          do { /* nothing to do */ } while (0)
#define ATRACE_PID_END(P, X)            do { /* nothing to do */ } while (0)
#define ATRACE_PID_INT(P, X, Y)         do { /* nothing to do */ } while (0)
#define ATRACE_PID_ASYNC_BEGIN(P, X, Y) do { /* nothing to do */ } while (0)
#define ATRACE_PID_ASYNC_END(P, X, Y)   do { /* nothing to do */ } while (0)
#define ATRACE_BEGIN(X)          ATRACE_PID_BEGIN(current->pid, X)
#define ATRACE_END(X)            ATRACE_PID_END(current->pid, X)
#define ATRACE_INT(X, Y)         ATRACE_PID_INT(current->pid, X, Y)
#define ATRACE_ASYNC_BEGIN(X, Y) ATRACE_PID_ASYNC_BEGIN(current->pid, X, Y)
#define ATRACE_ASYNC_END(X, Y)   ATRACE_PID_ASYNC_END(current->pid, X, Y)

#define R_ATRACE_BEGIN(X)        do { /* nothing to do */ } while (0)
#define R_ATRACE_END(X)          do { /* nothing to do */ } while (0)
#define R_ATRACE_INT(X, Y)       do { /* nothing to do */ } while (0)
#endif

/******************************/
/* define structure           */
/******************************/

/******************************/
/* define                     */
/******************************/

/* macros for general error message */
#define printk_lowerr(fmt, ...) \
	do { \
		if (pr_err(DEV_NAME ":E %s: " fmt, \
				__func__, ##__VA_ARGS__) <= 0) { \
			/* error in pr_err.   */ \
			/* but nothing to do. */ \
		} \
	} while (0)

/* macros for general log message */
#define printk_lowdbg(fmt, ...) \
	do { \
		if (pr_info(DEV_NAME ": %s: " fmt, \
				__func__, ##__VA_ARGS__) <= 0) { \
			/* error in pr_info.  */ \
			/* but nothing to do. */ \
		} \
	} while (0)

/* macros for normal-usecase error message */
#if _ERR_DBG >= 2
#define printk_err2(...) \
	do { \
		TRACE_LOG(FUNC_NONE); \
		if (debug > 1) { \
			/* report log if enabled */ \
			printk_lowerr(__VA_ARGS__); \
		} \
	} while (0)
#else
#define printk_err2(...) do { /* nothing to do */ } while (0)
#endif

/* macros for RT-API related error message */
#if _ERR_DBG >= 1
#define printk_err1(...) \
	do { \
		TRACE_LOG(FUNC_NONE); \
		if (debug > 0) { \
			/* report log if enabled */ \
			printk_lowerr(__VA_ARGS__); \
		} \
	} while (0)
#else
#define printk_err1(...) do { /* nothing to do */ } while (0)
#endif

/* macros for unexpected-error message */
#define printk_err(...) \
	do { \
		TRACE_LOG(FUNC_NONE); \
		printk_lowerr(__VA_ARGS__); \
	} while (0)

/* macros for normal-usecase log message */
#if _LOG_DBG >= 2
#define printk_dbg2(level, ...) \
	do { \
		if (((level)+2) <= debug) { \
			/* report log if enabled */ \
			printk_lowdbg(__VA_ARGS__); \
		} \
	} while (0)
#else
#define printk_dbg2(level, ...) do { /* nothing to do */ } while (0)
#endif

/* macros for RT-API log message */
#if _LOG_DBG >= 1
#define printk_dbg1(level, ...) \
	do { \
		if (((level)+2) <= debug) { \
			/* report log if enabled */ \
			printk_lowdbg(__VA_ARGS__); \
		} \
	} while (0)
#else
#define printk_dbg1(level, ...) do { /* nothing to do */ } while (0)
#endif

#define printk_dbg(level, ...) \
	do { \
		if (((level)+2) <= debug) { \
			/* report log if enabled */ \
			printk_lowdbg(__VA_ARGS__); \
		} \
	} while (0)

#define DBGENTER(...) printk_dbg2(2, "in  "  __VA_ARGS__)
#define DBGLEAVE(...) printk_dbg2(2, "out "  __VA_ARGS__)

#if INTERNAL_DEBUG == 0
/* do not record tracelog */
#define TRACE_ENTER(ID)      do { /* nothing to do */ } while (0)
#define TRACE_LEAVE(ID)      do { /* nothing to do */ } while (0)
#define TRACE_LOG(ID)        do { /* nothing to do */ } while (0)
#define TRACE_LOG1(ID, VAL1) do { /* nothing to do */ } while (0)

#else

#define ID_TRACE_ENTER        1
#define ID_TRACE_LEAVE        2
#define ID_TRACE_LOG          3
#define ID_TRACE_LOG1         4
#define FUNC_NONE             0x000
#define FUNC_OPEN             0x010
#define FUNC_CLOSE            0x011
#define FUNC_QUEUE            0x012
#define FUNC_CALLBACK         0x014
#define FUNC_WQ_CREATE        0x020
#define FUNC_WQ_DELETE        0x021
#define FUNC_WQ_BLEND         0x022

#define TRACE_ENTER(ID) \
	rcar_composer_tracelog_record(ID_TRACE_ENTER, __LINE__, ID, 0);
#define TRACE_LEAVE(ID) \
	rcar_composer_tracelog_record(ID_TRACE_LEAVE, __LINE__, ID, 0);
#define TRACE_LOG(ID) \
	rcar_composer_tracelog_record(ID_TRACE_LOG,   __LINE__, ID, 0);
#define TRACE_LOG1(ID, VAL1) \
	rcar_composer_tracelog_record(ID_TRACE_LOG1, __LINE__, ID, VAL1);

#endif

/******************************/
/* define external function   */
/******************************/
#if INTERNAL_DEBUG
extern void rcar_composer_tracelog_record(
	int logclass, int line, int ID, int val);

#ifdef CONFIG_DEBUG_FS
static int rcar_composer_dump_rhandle(char p[], int n,
	struct composer_rh *rh);
static void rcar_composer_debug_info_static(struct seq_file *s);
static void rcar_composer_debug_info_queue(struct seq_file *s);
#endif
#endif

#if _ATR_DBG
extern void rcar_composer_atracelog(char *func, int id, int arg);
#endif

/* graphics */
#if _LOG_DBG > 0
static void dump_screen_grap_image_blend(struct screen_grap_image_blend *arg);
static void dump_screen_grap_delete(struct screen_grap_delete *arg);
#endif
static const char *get_imgpctrlmsg_graphics(int rc);

#if _TIM_DBG
/* timerecord */
static void *timerecord_createhandle(void);
static void timerecord_deletehandle(void *);
static void timerecord_reset(void *);
static unsigned int timerecord_record(void *, int id);
static void timerecord_print(void *);
#endif

static void debugmodule_init(void);
static void debugmodule_exit(void);

#endif
