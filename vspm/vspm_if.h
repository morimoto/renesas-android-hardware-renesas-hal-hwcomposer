/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

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
enum{
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

enum{
	VSPM_CMD_ENTRY = 0,
	VSPM_CMD_CALCEL,
	VSPM_CMD_CB,
	VSPM_CMD_WAIT_CB_START,
	VSPM_CMD_CB_END,
};

#define VSPM_IOC_MAGIC 'v'
#define VSPM_IOC_CMD_ENTRY			_IOWR(VSPM_IOC_MAGIC,	\
									VSPM_CMD_ENTRY,			\
									struct vspm_ioc_cmd)
#define VSPM_IOC_CMD_CANCEL			_IOWR(VSPM_IOC_MAGIC,	\
									VSPM_CMD_CALCEL,		\
									struct vspm_ioc_cmd)
#define VSPM_IOC_CMD_CB				_IOR(VSPM_IOC_MAGIC,	\
									VSPM_CMD_CB,			\
									struct vspm_ioc_cmd)
#define VSPM_IOC_CMD_WAIT_CB_START	_IO(VSPM_IOC_MAGIC,		\
									VSPM_CMD_WAIT_CB_START)
#define VSPM_IOC_CMD_CB_END			_IO(VSPM_IOC_MAGIC,		\
									VSPM_CMD_CB_END)

#endif /* __VSPM_IF_H__ */
