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
#ifndef _R_CAR_COMPOSER_REMOTE_H
#define _R_CAR_COMPOSER_REMOTE_H

#include "inc/work.h"
#include <linux/list.h>

/******************************/
/* define structure           */
/******************************/

/* request remote function handle. */
struct composer_indirectcall {
	struct localwork               wqtask; /*!< task for indirect call */
	int    (*remote)(unsigned long *args); /*!< function call. */
	unsigned long                  args[4];/*!< argument of function. */
	struct list_head               list;   /*!< next unused list. */
};

/******************************/
/* define                     */
/******************************/

/******************************/
/* define external function   */
/******************************/
/*! initialize indirect call function */
static void indirect_call_init(void);

/*! indirect call function. */
static int indirect_call(struct localworkqueue *queue,
	int (*func)(unsigned long *args),
	int num_arg,
	unsigned long *args);

#endif
