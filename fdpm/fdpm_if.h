/*************************************************************************/ /*
 FDPM

 Copyright (C) 2013 Renesas Electronics Corporation

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

#ifndef __FDPM_IF_H__
#define __FDPM_IF_H__

/**
	* typedef PFN_FDPM_COMPLETE_CALLBACK - complete callback function pointer
	*
	*/

typedef void (*PFN_FDPM_COMPLETE_CALLBACK)(unsigned long uwUserData);


struct fdpm_cb_rsp {
	int end;
	int type;
	union {
		struct {
			PFN_FDPM_COMPLETE_CALLBACK cb;
			unsigned long uwJobId;
			long wResult;
			unsigned long uwUserData;
		} complete;
	} cb_data;
	unsigned char close_flag;
	unsigned char telecine_mode;
	T_FDP_R_SENSOR_INFO sinfo;
	T_FDP_R_REG_STATUS fdp_reg_status[60];
};

struct fdpm_ioc_cmd {
	void *req;
	void *rsp;
};

struct fdpm_open_req {
	unsigned long open_id;
	T_FDP_R_OPEN open_par;
};

struct fdpm_open_rsp {
	unsigned long open_id;
	int timer_ch;
	long rtcd;
};

struct fdpm_wait_req {
	unsigned long wait_id;
	int           vintmode;
	int           close_flag;
};

struct fdpm_close_req {
	unsigned long close_id;
	unsigned char f_release;
};

struct fdpm_close_rsp {
	unsigned long close_id;
	long rtcd;
};

struct fdpm_cancel_req {
	unsigned long cancel_fid;
	int cancel_id;
};

struct fdpm_cancel_rsp {
	unsigned long cancel_fid;
	long rtcd;
};

struct fdpm_start_req {
	unsigned long start_id;
	struct fdpm_cb_rsp cb_rsp;
	T_FDP_R_START start_par;
};

struct fdpm_start_rsp {
	unsigned long start_fid;
	T_FDP_R_CB1 fdp_cb1;
};

struct fdpm_status_req {
	unsigned long status_id;
};

struct fdpm_status_rsp {
	unsigned long status_id;
	T_FDP_STATUS status;
};

struct fdpm_status_reg_rsp {
	unsigned long status_id;
	T_FDP_REG_STATUS status[60];
};

enum {
	FDPM_CBTYPE_COMPLETE = 0,
};

enum {
	FDPM_CMD_OPEN = 0,
	FDPM_CMD_START,
	FDPM_CMD_CANCEL,
	FDPM_CMD_CLOSE,
	FDPM_CMD_STATUS,
	FDPM_CMD_WAIT,
	FDPM_CMD_WAIT_END,
	FDPM_CMD_REG_STATUS,
};

#define FDPM_IOC_MAGIC 'f'
#define FDPM_IOCTL_OPEN   _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_OPEN,   struct fdpm_ioc_cmd)
#define FDPM_IOCTL_START  _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_START,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_CANCEL _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_CANCEL, struct fdpm_ioc_cmd)
#define FDPM_IOCTL_CLOSE  _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_CLOSE,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_STATUS  _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_STATUS,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_WAIT   _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_WAIT,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_WAIT_END   _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_WAIT_END,  struct fdpm_ioc_cmd)
#define FDPM_IOCTL_REG_STATUS _IOWR(FDPM_IOC_MAGIC, FDPM_CMD_REG_STATUS,  struct fdpm_ioc_cmd)

#endif
