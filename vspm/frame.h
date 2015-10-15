/*************************************************************************/ /*
 VSPM

 Copyright (C) 2013-2014 Renesas Electronics Corporation

 License        Dual MIT/GPLv2

 The contents of this file are subject to the MIT license as set out below.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 Alternatively, the contents of this file may be used under the terms of
 the GNU General Public License Version 2 ("GPL") in which case the provisions
 of GPL are applicable instead of those above.

 If you wish to allow use of your version of this file only under the terms of
 GPL, and not to allow others to use your version of this file under the terms
 of the MIT license, indicate your decision by deleting the provisions above
 and replace them with the notice and other provisions required by GPL as set
 out in the file called "GPL-COPYING" included in this distribution. If you do
 not delete the provisions above, a recipient may use your version of this file
 under the terms of either the MIT license or GPL.

 This License is also included in this distribution in the file called
 "MIT-COPYING".

 EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
 PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


 GPLv2:
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

#ifndef __FRAME_H__
#define __FRAME_H__

#define RESERVED(a) ((void) a)

#define TRUE		1	/* true */
#define FALSE		0	/* false */

#define FW_OK		0	/* success */
#define FW_NG		-1	/* error */

/**
 * struct fw_func_tbl - function table structure
 * @func_id: function id
 * @msg_id:  message id
 * @func:    function pointer
 */
struct fw_func_tbl {
	short func_id;
	short msg_id;
	long (*func)(void *mesp, void *para);
};

/**
 * struct fw_msg - received message structure
 * @msg_id:  message id
 * @func_id: function id
 * @size:    size of the message parameter
 * @para:    message parameter
 */
struct fw_msg {
	short msg_id;
	short func_id;
	int size;
	void *para;
};

/*
 * message id
 */
#define MSG_FUNCTION		(short)0
#define MSG_EVENT			(short)1

/*
 * basic function id
 */
#define FUNC_TASK_INIT		(short)1
#define FUNC_TASK_QUIT		(short)2

/*
 * task id
 */
#define TASK_VSPM	(2)

/*
 * function id for VSPM task(base)
 */
#define FUNCTIONID_VSPM_BASE	(TASK_VSPM*256)

/**
 * fw_initialize - the framework initialization
 * Description: The framework initialization. When you use a framework,
 * should be called only once at the beginning.
 */
void fw_initialize(void);


/**
 * fw_task_register - register a task-information
 * @tid: task id
 * Description: Register a task-information
 * Returns: On success zero is returned. On error, -1 is returned.
 */
int fw_task_register(unsigned short tid);

/**
 * fw_task_unregister - unregister a task-information
 * @tid: task id
 * Description: Unregister a task-information
 * Returns: On success zero is returned. On error, -1 is returned.
 */
int fw_task_unregister(unsigned short tid);


/**
 * fw_execute - start the processing framework
 * @tid:      task id
 * @func_tbl: pointer to function table
 * Description: Start the processing framework.
 * Returns: On success zero is returned. On error, -1 is returned.
 */
int fw_execute(unsigned short tid, struct fw_func_tbl *func_tbl);

/**
 * fw_send_event - send a event message
 * @tid:     task id
 * @func_id: function id
 * @size:    size of the message parameter
 * @para:    message parameter
 * Description: Send a event message to @tid.
 * Returns: On success FW_OK is returned. On error, FW_NG is returned.
 */
long fw_send_event(unsigned short tid, short func_id, size_t size, void *para);

/**
 * fw_send_function - send a function message
 * @tid:     task id
 * @func_id: function id
 * @size:    size of the message parameter
 * @para:    message parameter
 * Description: Send a function message to @tid.
 * Returns: On success FW_OK is returned. On error, FW_NG is returned.
 */
long fw_send_function(
	unsigned short tid, short func_id, size_t size, void *para);

#endif /* __FRAME_H__ */
