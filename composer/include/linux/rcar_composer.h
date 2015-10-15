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
#ifndef _R_CAR_COMPOSER_H
#define _R_CAR_COMPOSER_H

#include "imgpctrl/imgpctrl_public.h"

/* common kernel and user */

/*******************/
/* define constant */
/*******************/
#define CMP_DATA_NUM_GRAP_LAYER    (1+4)

/******************************/
/* define structure for ioctl */
/******************************/
struct cmp_registerinfo {
	int max_size;
	int max_area;
	int max_rotbuf;
};

struct cmp_postdata {
	int           num_buffer;

	/* graphic buffer parameter */
	unsigned int  phys_address[5];
	int           buffer_fd[5];
	int           buffer_cached[5];

	/* hwcomposer parameter */
	int           acquire_fd;

	int           operation_type;

	struct screen_grap_layer  buffer[5];

	unsigned int       bgcolor;
};



/************************/
/* define cmd for ioctl */
/************************/

#define IOC_R_CAR_COMP_MAGIC 'S'

#define CMP_IOC_POST \
	_IOW(IOC_R_CAR_COMP_MAGIC, 0x21, struct cmp_postdata)
#define CMP_IOCGS_REGISTER \
	_IOWR(IOC_R_CAR_COMP_MAGIC, 0x25, struct cmp_registerinfo)



/*******************/
/* define constant */
/*******************/

#define CMP_OK	0
#define CMP_NG	-1

#if defined(CONFIG_MISC_R_CAR_COMPOSER) || \
	defined(CONFIG_MISC_R_CAR_COMPOSER_MODULE)

#ifdef __KERNEL__
/* please not define __KERNEL__ when build application. */

#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <ion/ion.h>
#include <../android/sync.h>
#include <linux/dma-buf.h>
#include <linux/kref.h>

/********************/
/* define constant  */

#define COMPOSER_NUM_INPUT_GRAP_LAYER 4

/* for internal use for kernel */

struct localworkqueue {
	struct   list_head  top;
	spinlock_t          lock;
	wait_queue_head_t   wait;
	wait_queue_head_t   finish;
	struct task_struct  *task;
	int                 priority;
};

struct localwork;

struct localwork {
	struct list_head  link;
	void              (*func)(struct localwork *work);
	int               status;
	struct localworkqueue *wq;
};

#if defined(CONFIG_ION)
struct ionmem;
#endif

struct sw_fence_handle;

/* file handle */
struct composer_fh {
	struct semaphore         fh_sem;
	void                     *ioctl_args;

	unsigned short           dev_number;
	unsigned short           seq_number;

	struct list_head         fhlist_top;

#if defined(CONFIG_ION)
	struct ionmem            *ionmem;
#endif
	struct sw_fence_handle   *swfence;
};



/* compose parameter for blend */
struct cmp_data_compose_blend {
	int                            valid;
	struct screen_grap_image_blend blend;
	struct screen_grap_layer       layer[COMPOSER_NUM_INPUT_GRAP_LAYER];
};

/* request queue handle */
struct composer_rh {
	struct localwork             rh_wqtask;

	struct localwork             rh_wqtask_schedule;

	struct cmp_data_compose_blend data;

	unsigned short               seq_number;
	unsigned char                dev_number;
	unsigned char                use_blendunit;

	int                          num_buffer;
	unsigned long                buffer_address[CMP_DATA_NUM_GRAP_LAYER];
	struct {
		struct sync_fence_waiter waiter;
		struct sync_fence        *fence;
	} wait;
	struct sw_fence_handle       *swfence;

#if defined(CONFIG_ION)
	struct ionmem                *ionmem;
	struct ion_handle            *ionhandle[CMP_DATA_NUM_GRAP_LAYER];
	struct dma_buf               *iondmabuf[CMP_DATA_NUM_GRAP_LAYER];
	int                          ioncached[CMP_DATA_NUM_GRAP_LAYER];
#endif

	int                          active;
	void                        (*user_callback)(
		void *user_data, int result);
	void                         *user_data;
	int                          refcount;

	void                         *timerecord;

	struct kref                  ref;
	struct list_head             fh_link;
	struct list_head             rh_link;
};

#endif
/* end __KERNEL__ */
#endif
/* end  defined(CONFIG_MISC_R_CAR_COMPOSER) || \
	defined(CONFIG_MISC_R_CAR_COMPOSER_MODULE) */
#endif
/* end _R_CAR_COMPOSER_H */

