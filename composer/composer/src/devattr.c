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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include "inc/debug.h"

/* prototype */
static int devattr_enable_vsp_show(struct device *dev __maybe_unused,
	struct device_attribute *attr __maybe_unused, char *buf);

static int  devattr_enable_vsp_store(struct device *dev __maybe_unused,
	struct device_attribute *attr __maybe_unused,
	const char *buf, size_t len);

/* static */
static int enable_vsp = true;

/* module parameter can not allow to write other group,       */
/* therefore special sys fs created that allow to write other */
static DEVICE_ATTR(enable_vsp, S_IRUGO | (S_IWUSR|S_IWGRP|S_IWOTH),
	devattr_enable_vsp_show,
	devattr_enable_vsp_store);

static struct attribute *p_attr_array[] = {
	&dev_attr_enable_vsp.attr,
	NULL
};

static struct attribute_group attr_group = {
	.name = "parameters",
	.attrs = p_attr_array
};

/******************************************************/
/*! \brief device attributes for enable_vsp variable
 *  \param[in] dev    pointer to device
 *  \param[in] attr   pointer to device_attribute
 *  \param[out] buf   pointer to buffer
 *  \return num of character
 *  \details
 *  handle show method of device attributes for enable_vsp.\n
 */
static int devattr_enable_vsp_show(struct device *dev __maybe_unused,
	struct device_attribute *attr __maybe_unused, char *buf)
{
	return  sprintf(buf, "%d\n", enable_vsp);
}

/******************************************************/
/*! \brief device attributes for enable_vsp variable
 *  \param[in] dev    pointer to device
 *  \param[in] attr   pointer to device_attribute
 *  \param[in] buf   pointer to buffer
 *  \param[in] len   size of buffer
 *  \return num of character
 *  \details
 *  handle store method of device attributes for enable_vsp.\n
 */
static int  devattr_enable_vsp_store(struct device *dev __maybe_unused,
	struct device_attribute *attr __maybe_unused,
	const char *buf, size_t len)
{
	long val;

	if (kstrtol(buf, 0, &val))
		printk_err2("kstrtol error\n");
	else
		enable_vsp = val;

	return len;
}

/******************************************************/
/*! \brief initialize device attribute module
 *  \param[in] mod    pointer to module
 *  \return nothing
 *  \details
 *  initialize device attribute module. \n
 *  and register attributes of parameters/enable_vsp. \n
 *  \attention
 *  this function should call after misc_register call. \n
 */
static void devattr_init(struct module *mod)
{
	if (sysfs_merge_group(&mod->mkobj.kobj, &attr_group))
		printk_err("sysfs_merge_group error\n");
}

/******************************************************/
/*! \brief exit device attribute module
 *  \param[in] mod    pointer to module
 *  \return nothing
 *  \details
 *  unregister attributes of parameters/enable_vsp. \n
 *  and exit device attribute module.\n
 *  \attention
 *  this function should call before misc_deregister call. \n
 */
static void devattr_exit(struct module *mod)
{
	sysfs_unmerge_group(&mod->mkobj.kobj, &attr_group);
}
