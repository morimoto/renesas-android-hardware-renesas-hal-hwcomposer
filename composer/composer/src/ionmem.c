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

#include "linux/rcar_composer.h"
#include "inc/debug.h"
#include "inc/ionmem.h"
#include <linux/spinlock.h>

/******************************************************/
/* define prototype                                   */
/******************************************************/

/******************************************************/
/* define local define                                */
/******************************************************/
#ifndef CONFIG_ION_RCAR
#error CONFIG_ION_RCAR necessary
#endif

/******************************************************/
/* define local variables                             */
/******************************************************/

/******************************************************/
/* local functions                                    */
/******************************************************/
/*! \brief initalize ion memory
 *  \return handle to use ion memory.
 *  \retval NULL    error
 *  \retval others  pointer to struct ionmem;
 *  \details
 *  create ion_client to use ion memory. and return handle.\n
 *  to free this handle should call release_ionmem.
 *  \msc
 *    composer, ionmem;
 *    |||;
 *    --- [label="initialize"];
 *    composer=>ionmem
 *        [label="init", URL="\ref ::initialize_ionmem"];
 *    --- [label="use ion_client"];
 *    composer=>ionmem
 *        [label="incement reference", URL="\ref ::incref_ionclient"];
 *    composer box composer
 *        [label="use ion_client"];
 *    composer=>ionmem
 *        [label="decrement reference", URL="\ref ::decref_ionclient"];
 *    --- [label="finalize"];
 *    composer=>ionmem
 *        [label="free", URL="\ref ::release_ionmem"];
 *  \endmsc
 *  \attention
 *  this function should call before call getphys_ionhandle().
 */
static struct ionmem *initialize_ionmem(void)
{
	struct ionmem *mem = NULL;
	struct ion_client *client;

	DBGENTER("\n");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false != IS_ERR_OR_NULL(g_psIonDev)) {
		printk_err("ion_device invalid\n");
		goto err_exit;
	}

	mem = kzalloc(sizeof(*mem), GFP_KERNEL);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == mem) {
		printk_dbg2(2, "kzalloc fail\n");
		goto err_exit;
	}
	printk_dbg2(3, "kmalloc(%d)= %p\n", sizeof(*mem), mem);

	spin_lock_init(&mem->lock);

	client = ion_client_create(g_psIonDev, DEV_NAME);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (false != IS_ERR_OR_NULL(client)) {
		printk_err("ion_client_create fail\n");
		kfree(mem);
		mem = NULL;
		goto err_exit;
	}

	spin_lock(&mem->lock);
	mem->client = client;
	mem->private_client.count = 1;
	spin_unlock(&mem->lock);

err_exit:
	DBGLEAVE("%p:\n", mem);
	return mem;
}

/*! \brief release ion memory
 *  \param[in] mem  pointer to struct ionmem
 *  \return none
 *  \details
 *  to release handle internally call decref_ionclient.\n
 */
static void release_ionmem(struct ionmem *mem)
{
	DBGENTER("mem:%p\n", mem);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != mem)
		decref_ionclient(mem);
	else
		printk_err("invalid sequence\n");

	DBGLEAVE("\n");
}

/* operation for client */
/*! \brief increment reference of ion memory
 *  \param[in] mem  pointer to struct ionmem
 *  \return none
 *  \details
 *  increment reference of objects.
 *  this function should call when copy of pointer created.
 */
static void incref_ionclient(struct ionmem *mem)
{
	DBGENTER("mem:%p\n", mem);

	spin_lock(&mem->lock);
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != mem->client)
		mem->private_client.count++;
	spin_unlock(&mem->lock);

	DBGLEAVE("\n");
}

/*! \brief decrement reference of ion memory
 *  \param[in] mem  pointer to struct ionmem
 *  \return none
 *  \details
 *  decrement reference of objects.
 *  this function should call when copy of pointer is not necessary.
 *  if count of reference become zeros then release resources.
 */
static void decref_ionclient(struct ionmem *mem)
{
	int v;
	struct ion_client *client  = NULL;
	struct ionmem     *freemem = NULL;

	DBGENTER("mem:%p\n", mem);

	spin_lock(&mem->lock);
	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != mem->client) {
		mem->private_client.count--;
		v = mem->private_client.count;

		if (v == 0) {
			/* unregister */
			client = mem->client;
			mem->client = NULL;

			freemem     = mem;
		}

		printk_dbg2(3, "count %d\n", v);
	}
	spin_unlock(&mem->lock);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != client)
		ion_client_destroy(client);

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != freemem) {
		printk_dbg2(3, "kfree %p\n", freemem);
		kfree(freemem);
	}

	DBGLEAVE("\n");
}

/* operation for handle */
/*! \brief get physical address of ion memory
 *  \param[in] mem  pointer to struct ionmem
 *  \param[in] handle  pointer to struct ion_handle
 *  \return address of physical memory.
 *  \retval 0       error
 *  \retval others  physical address.
 *  \details
 *  get physical address from ion_phys function.
 *  this driver only use address information.
 */
static unsigned long getphys_ionhandle(struct ionmem *mem,
	struct ion_handle *handle)
{
	struct ion_client *client;
	unsigned int phys_addr = 0;
	ion_phys_addr_t addr;
	size_t          len __maybe_unused;
#if _LOG_DBG > 1
	struct sg_table *sg_table;
#endif

	DBGENTER("mem:%p handle:%p\n", mem, handle);

	client = mem->client;

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == client) {
		printk_dbg2(2, "not registered\n");
		goto err_exit;
	}

#if _LOG_DBG > 1
	sg_table = ion_sg_table(client, handle);

	if (false != IS_ERR_OR_NULL(sg_table)) {
		/* not found valid sg_table */
		printk_dbg2(3, "ion_sg_table fail.\n");
		sg_table = NULL;
	} else {
		printk_dbg2(3, "sg_table:%p nents:%d top_phys:0x%llx\n",
			sg_table, sg_table->nents,
			(uint64_t)sg_phys(sg_table->sgl));
	}
#endif

	if (ion_phys(client, handle, &addr, &len) == 0) {
		/* found physical address */
		phys_addr = (unsigned int)addr;

		printk_dbg2(3, "ion_phys result: 0x%x\n", phys_addr);
	}
#if _LOG_DBG > 1
	else if (NULL != sg_table) {
		if (sg_table->nents != 1) {
			/* not single table */
			printk_dbg2(3, "sg_table is construct %d parts.\n",
				sg_table->nents);
		} else {
			/* found contiguous memory. */
			phys_addr = sg_phys(sg_table->sgl);

			printk_dbg2(3, "sg_phys result: 0x%x\n", phys_addr);
		}
	}
#endif

err_exit:
	DBGLEAVE("%x\n", phys_addr);
	return phys_addr;
}

