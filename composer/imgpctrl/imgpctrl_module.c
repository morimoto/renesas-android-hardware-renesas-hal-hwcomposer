/*************************************************************************/ /*
  imgpctrl_module.c
   imgpctrl kernel module function file.

 Copyright (C) 2013 Renesas Electronics Corporation

 License        GPLv2

 If you wish to use this file under the terms of GPL, following terms are
 effective.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ /*************************************************************************/

#include <linux/module.h>

#include "imgpctrl_private.h"
#include "imgpctrl_common.h"

static int imgpctrl_initialize(void);
static int imgpctrl_release(void);

/**********************************************************************/
/* initialize/release function                                        */
/**********************************************************************/
static int imgpctrl_initialize(void)
{
	int ret = 0;

	return ret;
}

static int imgpctrl_release(void)
{
	int ret = 0;

	return ret;
}

/**********************************************************************/
/* module initialize and exit                                         */
/**********************************************************************/
static int __init imgpctrl_module_init(void)
{
	int ret;

	(void)pr_alert("%s", "imgpctrl driver loaded\n");
	ret = imgpctrl_initialize();

	return ret;
}

static void __exit imgpctrl_module_exit(void)
{
	int ret;

	(void)pr_alert("%s", "imgpctrl driver unloaded\n");
	ret = imgpctrl_release();
	(void)pr_alert("return value :%d\n", ret);

	return;
}

module_init(imgpctrl_module_init);
module_exit(imgpctrl_module_exit);
MODULE_LICENSE("GPL");
