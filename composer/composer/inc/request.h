/*
 * Function        : Composer driver
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
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
#ifndef _R_CAR_COMPOSER_REQUEST_H
#define _R_CAR_COMPOSER_REQUEST_H

/******************************/
/* define                     */
/******************************/

/******************************/
/* define structure           */
/******************************/

/******************************/
/* define external function   */
/******************************/
/*! add reference of composer_rh structure */
static void rhandle_incref(struct composer_rh *rh);

/*! sub reference of composer_rh structure */
static void rhandle_decref(struct composer_rh *rh);

/*! create request handle and register composer_fh structure */
static struct composer_rh *rhandle_create(struct composer_fh *fh);

/*! free request handle registered in composer_fh structure */
static void rhandle_free(struct composer_fh *fh);

/*! lookup un-used request handle registered in composer_fh structure */
static struct composer_rh *rhandle_getunusedhandle(struct composer_fh *fh);

/*! free request handle used for operation */
static void rhandle_putusedhandle(struct composer_rh *rh);

/*! confirm all request handle complete. */
static int rhandle_checkcomplete(unsigned int timeout);

/*! initialize request handle module */
static void rhandlemodule_init(void);

/*! exit request handle module */
static void rhandlemodule_exit(void);

#endif
