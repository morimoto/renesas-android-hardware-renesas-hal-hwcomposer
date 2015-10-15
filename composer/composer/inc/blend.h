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
#ifndef _R_CAR_COMPOSER_BLEND_H
#define _R_CAR_COMPOSER_BLEND_H

#include <linux/module.h>
#include <linux/wait.h>
#include <linux/semaphore.h>

#include "linux/rcar_composer.h"
#include "imgpctrl/imgpctrl_public.h"


/******************************/
/* define for status          */
/******************************/
/*! status of blend_registerinfo for error. */
#define RTAPI_NOTIFY_RESULT_ERROR     3
/*! status of blend_registerinfo for no problem. */
#define RTAPI_NOTIFY_RESULT_NORMAL    1
/*! status of blend_registerinfo for not ready. */
#define RTAPI_NOTIFY_RESULT_UNDEFINED 0

/******************************/
/* define structure           */
/******************************/
struct blend_registerinfo {
	struct screen_grap_image_blend *blend;
	void              (*callback)(int rc, void *arg);
	void              *arg;
	wait_queue_head_t wait;
	int               status;
};

struct blend_info {
	struct blend_registerinfo info[
		CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT];
};

/******************************/
/* define external function   */
/******************************/

/*! create handle */
static void *blend_create(void);
/*! delete handle */
static void blend_delete(void *graphic_handle);
/*! request blend */
static int  blend_request(void *graphic_handle,
	struct screen_grap_image_blend *blend,
	void *user_data, void (*callback)(int rc, void *arg));

/*! configure blend parameters */
static int  blend_config(struct composer_rh *rh, struct cmp_postdata *post);

#endif
