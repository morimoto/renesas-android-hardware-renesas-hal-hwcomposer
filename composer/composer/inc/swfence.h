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
#ifndef _R_CAR_COMPOSER_SWFENCE_H
#define _R_CAR_COMPOSER_SWFENCE_H

#include "linux/rcar_composer.h"

#include <linux/kref.h>
#include <../android/sync.h>
#include <../android/sw_sync.h>

/*! async wait interface used */
#define FEATURE_USE_ASYNC_WAIT   1


/******************************/
/* define                     */
/******************************/

/******************************/
/* define structure           */
/******************************/
struct sw_fence_handle {
	struct kref              kref;           /*!< kernel reference */
	struct sw_sync_timeline  *timeline;      /*!< timeline structure  */
	unsigned int             timeline_count; /*!< time to create fence   */
	unsigned int             timeline_inc;   /*!< current time for debug */
};

/******************************/
/* define external function   */
/******************************/

/* make ready to use sync object */
static int fence_config(struct composer_rh *rh, struct cmp_postdata *post);

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
static void rcar_composer_debug_info_fencewait_handle(struct seq_file *s,
	struct composer_rh *rh);
#endif
#endif


#if FEATURE_USE_ASYNC_WAIT
/* register fence wait listener */
static void fence_async_wait_register_listener(
	void (*cb)(struct composer_rh *rh, int id));
/* start wait to signal of sync object */
static int fence_async_wait(struct composer_rh *rh);
#else
/* wait signal of sync object */
static int fence_wait(struct composer_rh *rh, int timeout, int usage);
#endif

/* release sync object */
static int fence_expire(struct composer_rh *rh);



/* interface of handle sw_fence.  */
static struct sw_fence_handle *fence_get_handle(void);

static void   fence_put_handle(struct sw_fence_handle *handle);

static struct sw_fence_handle *fence_dup_handle(struct sw_fence_handle *handle);

#if INTERNAL_DEBUG
#ifdef CONFIG_DEBUG_FS
static void rcar_composer_debug_info_fence_handle(struct seq_file *s,
	struct sw_fence_handle *handle);
#endif
#endif

/* get sync fd */
static int fence_get_syncfd(struct sw_fence_handle *handle);

/* increment timeline */
static int fence_inc_timeline(struct sw_fence_handle *handle);

/* assert signal */
static int fence_signal(struct sw_fence_handle *handle);

#endif
