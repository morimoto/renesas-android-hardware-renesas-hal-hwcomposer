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
#ifndef _R_CAR_COMPOSER_WORK_H
#define _R_CAR_COMPOSER_WORK_H

#include "linux/rcar_composer.h"

/******************************/
/* define                     */
/******************************/

/******************************/
/* define structure           */
/******************************/

/******************************/
/* define external function   */
/******************************/
/*! create workqueue with name */
static struct localworkqueue *localworkqueue_create(char *name, int priority);

/*! flush workqueue */
static void localworkqueue_flush(struct localworkqueue *wq);

/*! destroy workqueue */
static void localworkqueue_destroy(struct localworkqueue *wq);

/*! add task */
static int  localwork_queue(struct localworkqueue *wq,
	struct localwork *work);

/*! wait complete task */
static void localwork_flush(struct localwork *work);

/*! task initialize */
static void localwork_init(struct localwork *work,
	void (*func)(struct localwork *work));

#endif
