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

#include "atrace/atrace.h"

noinline void tracing_mark_write(int begin, int pid, const char *name, int val)
{
	if (begin == 1)
		trace_printk("B|%d|%s\n", pid, name);
	else if (begin == 2)
		trace_printk("C|%d|%s|%d\n", pid, name, val);
	else if (begin == 3)
		trace_printk("S|%d|%s|%d\n", pid, name, val);
	else if (begin == 4)
		trace_printk("F|%d|%s|%d\n", pid, name, val);
	else
		trace_printk("E\n");
}

