/*************************************************************************/ /*
 UVCS Common

 Copyright (C) 2013 Renesas Electronics Corporation

 License        Proprietary

 This program must be used solely for the purpose for which
 it was furnished by Renesas Electronics Corporation.
 No part of this program may be reproduced or disclosed to
 others, in any form, without the prior written permission
 of Renesas Electronics Corporation.
*/ /*************************************************************************/

#ifndef UVCS_CMN_H
#define UVCS_CMN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*****************************************************************************/
/*                      INCLUDE FILES                                        */
/*****************************************************************************/
#include "uvcs_types.h"

/*****************************************************************************/
/*                      MACROS/DEFINES                                       */
/*****************************************************************************/
#define UVCS_CMN_MAX_HW_NUM		(2uL)
#define UVCS_CMN_HW_MODULE_NUM	(2uL)
#define UVCS_CMN_PROC_REQ_MAX	(3uL)
#define UVCS_CMN_HWP_CMD_NOEL	(128uL)

#define UVCS_CMN_MDL_VLC		(0uL)
#define UVCS_CMN_MDL_CE			(1uL)
#define UVCS_CMN_MDL_FCV		(2uL)

#define UVCS_CMN_HWPIDX_REQ_TMG	(0uL)	/* timing */
#define UVCS_CMN_HWPIDX_REQ_DAT	(1uL)	/* data */
#define UVCS_CMN_HWPIDX_REQ_BAA	(32uL)	/* BAA */

#define UVCS_CMN_HWPIDX_RES_STA	(0uL)	/* status NML or ERR */
#define UVCS_CMN_HWPIDX_RES_CVL	(1uL)	/* vlc execute counter */
#define UVCS_CMN_HWPIDX_RES_CCE	(2uL)	/* ce execute counter */
#define UVCS_CMN_HWPIDX_RES_CFC	(3uL)	/* fcv execute counter */
#define UVCS_CMN_HWPIDX_RES_EST	(4uL)	/* execute start time */
#define UVCS_CMN_HWPIDX_RES_EET	(5uL)	/* execute end time */
#define UVCS_CMN_HWPIDX_RES_DAT	(6uL)	/* data */
#define UVCS_CMN_HWP_NOEL_BASE	(UVCS_CMN_HWPIDX_RES_EET)

/*****************************************************************************/
/*               TYPE DEFINITION                                             */
/*****************************************************************************/

typedef UVCS_U32 *	 UVCS_CMN_LIB_INFO;

typedef UVCS_U32 *	 UVCS_CMN_HANDLE;

typedef struct UVCS_CMN_OPEN_PARAM {
	UVCS_U32		 struct_size;
	UVCS_PTR		 hdl_udptr;
	UVCS_U32		*hdl_work_0_virt;
	UVCS_U32		 hdl_work_0_size;
	UVCS_BOOL		 preempt_mode;
	UVCS_U32		 preempt_hwid;
} UVCS_CMN_OPEN_PARAM_T;

typedef struct UVCS_CMN_HW_PROC {
	UVCS_U32		 struct_size;
	UVCS_U32		 module_id;
	UVCS_U32		 req_serial;
	UVCS_U32		 cmd_param_noel;
	UVCS_U32		 cmd_param[UVCS_CMN_HWP_CMD_NOEL];
} UVCS_CMN_HW_PROC_T;

typedef struct UVCS_CMN_IP_INFO {
	UVCS_U32		 struct_size;
	UVCS_U32		 ip_version;
	UVCS_U32		 ip_option;
} UVCS_CMN_IP_INFO_T;

typedef void (*UVCS_CMN_CB_REG_READ)(
	UVCS_PTR		  udptr,
	volatile UVCS_U32	 *reg_addr,
	UVCS_U32		 *dst_addr,
	UVCS_U32		  num_reg
);

typedef void (*UVCS_CMN_CB_REG_WRITE)(
	UVCS_PTR		  udptr,
	volatile UVCS_U32	 *reg_addr,
	UVCS_U32		 *src_addr,
	UVCS_U32		  num_reg
);

typedef void (*UVCS_CMN_CB_HW_START)(
	UVCS_PTR		  udptr,
	UVCS_U32		  hw_ip_id,
	UVCS_U32		  hw_module_id,
	UVCS_U32		 *baa
);

typedef void (*UVCS_CMN_CB_HW_STOP)(
	UVCS_PTR		  udptr,
	UVCS_U32		  hw_ip_id,
	UVCS_U32		  hw_module_id
);

typedef void (*UVCS_CMN_CB_PROC_DONE)(
	UVCS_PTR		  udptr,
	UVCS_PTR		  hdl_udptr,
	UVCS_CMN_HANDLE		  handle,
	UVCS_CMN_HW_PROC_T	* res_info
);

typedef UVCS_BOOL (*UVCS_CMN_CB_SEM_LOCK)(
	UVCS_PTR			 udptr
);

typedef void (*UVCS_CMN_CB_SEM_UNLOCK)(
	UVCS_PTR			 udptr
);

typedef UVCS_BOOL (*UVCS_CMN_CB_SEM_CREATE)(
	UVCS_PTR			 udptr
);

typedef void (*UVCS_CMN_CB_SEM_DESTROY)(
	UVCS_PTR			 udptr
);

typedef void (*UVCS_CMN_CB_THREAD_EVENT)(
	UVCS_PTR			 udptr
);

typedef UVCS_BOOL (*UVCS_CMN_CB_THREAD_CREATE)(
	UVCS_PTR			 udptr
);

typedef void (*UVCS_CMN_CB_THREAD_DESTROY)(
	UVCS_PTR			 udptr
);

/* for uvcs_cmn_initialize().
 * Please see the specification (section 3.2.1) for more detail.
 */
typedef struct UVCS_CMN_INIT_PARAM {
	UVCS_U32	 struct_size;
	UVCS_U32	 hw_num;
	UVCS_PTR	 udptr;

	/* work memory */
	UVCS_U32	*work_mem_0_virt;
	UVCS_U32	 work_mem_0_size;

	/* ip information */
	UVCS_U32	 ip_base_addr[UVCS_CMN_MAX_HW_NUM][UVCS_CMN_HW_MODULE_NUM];
	UVCS_U32	 ip_option;

	/* system callback */
	UVCS_CMN_CB_REG_READ		 cb_reg_read;
	UVCS_CMN_CB_REG_WRITE		 cb_reg_write;
	UVCS_CMN_CB_HW_START		 cb_hw_start;
	UVCS_CMN_CB_HW_STOP			 cb_hw_stop;
	UVCS_CMN_CB_PROC_DONE		 cb_proc_done;
	UVCS_CMN_CB_SEM_LOCK		 cb_sem_lock;
	UVCS_CMN_CB_SEM_UNLOCK		 cb_sem_unlock;
	UVCS_CMN_CB_SEM_CREATE		 cb_sem_create;
	UVCS_CMN_CB_SEM_DESTROY		 cb_sem_destroy;
	UVCS_CMN_CB_THREAD_EVENT	 cb_thr_event;
	UVCS_CMN_CB_THREAD_CREATE	 cb_thr_create;
	UVCS_CMN_CB_THREAD_DESTROY	 cb_thr_destroy;

	/* debug information */
	UVCS_U32	*debug_log_buff;
	UVCS_U32	 debug_log_size;

} UVCS_CMN_INIT_PARAM_T;


/*****************************************************************************/
/*               EXTERNAL FUNCTIONS                                          */
/*****************************************************************************/
extern UVCS_RESULT uvcs_cmn_interrupt(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_U32			 base_addr,
			UVCS_U32			 cur_time
			);
extern UVCS_RESULT uvcs_cmn_initialize(
			UVCS_CMN_INIT_PARAM_T	*init_param,
			UVCS_CMN_LIB_INFO		*lib_info
			);
extern UVCS_RESULT uvcs_cmn_deinitialize(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_BOOL			 forced
			);
extern UVCS_RESULT uvcs_cmn_open(
			UVCS_CMN_LIB_INFO		 lib_info,
			UVCS_CMN_OPEN_PARAM_T	*open_param,
			UVCS_CMN_HANDLE			*handle
			);
extern UVCS_RESULT uvcs_cmn_close(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_CMN_HANDLE		 handle,
			UVCS_BOOL			 forced
			);
extern UVCS_RESULT uvcs_cmn_request(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_CMN_HANDLE		 handle,
			UVCS_CMN_HW_PROC_T	*req_info
			);
extern UVCS_RESULT uvcs_cmn_execute(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_U32			 cur_time
			);
extern UVCS_RESULT uvcs_cmn_set_preempt_mode(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_CMN_HANDLE		 handle,
			UVCS_BOOL			 preempt_mode,
			UVCS_U32			 preempt_hwid
			);
extern UVCS_RESULT uvcs_cmn_get_work_size(
			UVCS_U32			*work_mem_0_size,
			UVCS_U32			*hdl_work_0_size
			);
extern UVCS_RESULT uvcs_cmn_get_ip_info(
			UVCS_CMN_LIB_INFO	 lib_info,
			UVCS_CMN_IP_INFO_T	*ip_info
			);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UVCS_CMN_H */

