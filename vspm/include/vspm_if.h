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

#ifndef __VSPM_IF_H__
#define __VSPM_IF_H__

#define	DEVFILE	"/dev/vspm"

/*
 * struct vspm_entry_req - parameter of VSPM_IOC_CMD_ENTRY request
 * @puwJobId:          destination address of the job id
 * @bJobPriority:      job priority 1(low) - 126(high)
 * @ptIpParam:         pointer to IP parameter
 * @uwUserData:        user data
 * @pfnNotifyComplete: pointer to complete callback function
 */
struct vspm_entry_req {
	unsigned long *puwJobId;
	char bJobPriority;
	VSPM_IP_PAR *ptIpParam;
	unsigned long uwUserData;
	PFN_VSPM_COMPLETE_CALLBACK pfnNotifyComplete;
};

/*
 * struct vspm_entry_rsp - parameter of VSPM_IOC_CMD_ENTRY response
 * @uwJobId: job id
 * @rtcd:    processing result
 */
struct vspm_entry_rsp {
	unsigned long uwJobId;
	long rtcd;
};

/*
 * struct vspm_cancel_req - parameter of VSPM_IOC_CMD_CANCEL request
 * @uwJobId: job id
 */
struct vspm_cancel_req {
	unsigned long uwJobId;
};

/*
 * struct vspm_cancel_rsp - parameter of VSPM_IOC_CMD_CANCEL response
 * @rtcd:    processing result
 */
struct vspm_cancel_rsp {
	long rtcd;
};

/* type of callback */
enum {
	VSPM_CBTYPE_COMPLETE = 0,
};

/*
 * struct vspm_cb_rsp - parameter of VSPM_IOC_CMD_CB response
 * @end:     flag to indicate the end of the callback thread
 * @type:    type of callback
 * @cb_data: callback information
 */
struct vspm_cb_rsp {
	int end;
	int type;
	union {
		struct vspm_cb_complete_rsp {
			PFN_VSPM_COMPLETE_CALLBACK cb;
			unsigned long uwJobId;
			long wResult;
			unsigned long uwUserData;
		} complete;
	} cb_data;
};

/*
 * struct vspm_ioc_cmd - parameter of ioctl command
 * @req: pointer to request data
 * @rsp: destination address of response data
 */
struct vspm_ioc_cmd {
	void *req;
	void *rsp;
};

enum {
	VSPM_CMD_ENTRY = 0,
	VSPM_CMD_CALCEL,
	VSPM_CMD_CB,
	VSPM_CMD_WAIT_CB_START,
	VSPM_CMD_CB_END,
};

#define VSPM_IOC_MAGIC 'v'
#define VSPM_IOC_CMD_ENTRY \
	_IOWR(VSPM_IOC_MAGIC, VSPM_CMD_ENTRY, struct vspm_ioc_cmd)
#define VSPM_IOC_CMD_CANCEL \
	_IOWR(VSPM_IOC_MAGIC, VSPM_CMD_CALCEL, struct vspm_ioc_cmd)
#define VSPM_IOC_CMD_CB \
	_IOR(VSPM_IOC_MAGIC, VSPM_CMD_CB, struct vspm_ioc_cmd)
#define VSPM_IOC_CMD_WAIT_CB_START  \
	_IO(VSPM_IOC_MAGIC, VSPM_CMD_WAIT_CB_START)
#define VSPM_IOC_CMD_CB_END	\
	_IO(VSPM_IOC_MAGIC,	VSPM_CMD_CB_END)

#endif /* __VSPM_IF_H__ */
