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
#ifndef _R_CAR_COMPOSER_IONMEM_H
#define _R_CAR_COMPOSER_IONMEM_H

#include <ion/ion.h>
#include <linux/file.h>
#include <linux/dma-buf.h>
#include <linux/spinlock.h>

/******************************/
/* define                     */
/******************************/

/******************************/
/* define structure           */
/******************************/
struct ionmem {
	spinlock_t        lock;    /*!< exclusive control. */

	struct ion_client *client; /*!< pointer to ion_client. */

	struct {
		int          count;/*!< reference count. */
	} private_client;
};


/******************************/
/* define external function   */
/******************************/
#ifdef CONFIG_ION_RCAR
/*! defined in drivers/gpu/ion/rcar/rcar_ion.c */
extern struct ion_device *g_psIonDev;
#endif

/*! initialize */
static struct ionmem *initialize_ionmem(void);

/*! release */
static void release_ionmem(struct ionmem *mem);

/* operation for client */
/*! increment reference */
static void incref_ionclient(struct ionmem *mem);

/*! decrement reference */
static void decref_ionclient(struct ionmem *mem);

/* operation for handle */
/*! get physical address */
static unsigned long getphys_ionhandle(struct ionmem *mem,
	struct ion_handle *handle);

#endif
