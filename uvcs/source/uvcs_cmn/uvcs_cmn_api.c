/*************************************************************************/ /*
 UVCS Common

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

#ifdef __RENESAS__
#pragma section UVCSCMN
#endif /* __RENESAS__ */

/*****************************************************************************/
/*                    INCLUDE FILES                                          */
/*****************************************************************************/
#include "uvcs_types.h"
#include "mcvx_types.h"
#include "mcvx_api.h"
#include "uvcs_cmn.h"		/* external header file */
#include "uvcs_cmn_internal.h"	/* internal common header file */
#include "uvcs_cmn_dump.h"

/*****************************************************************************/
/*                    MACROS/DEFINES                                         */
/*****************************************************************************/

#if (UVCS_DEBUG != 0)
#define UVCS_C_DUMP_ID			UVCS_C_API_C	/**< for debug */
#endif

/*****************************************************************************/
/*                    FORWARD DECLARATIONS                                   */
/*****************************************************************************/
UVCS_STATIC void uvcs_c_vlc_event(
			void		*udp,
			MCVX_U32	 vlc_event,
			void		*unused
			);
UVCS_STATIC void uvcs_c_ce_event(
			void		*udp,
			MCVX_U32	 ce_event,
			void		*unused
			);
UVCS_STATIC void uvcs_c_fcv_event(
			void		*udp,
			MCVX_U32	 fcv_event,
			void		*unused
			);
UVCS_STATIC struct UVCS_C_HANDLE *uvcs_c_search_hdl(
			struct UVCS_C_INR_INFO *inr,
			UVCS_U32 mdl_id
			);
UVCS_STATIC void uvcs_c_hw_execute(
			struct UVCS_C_INR_INFO *inr,
			UVCS_U32 tgt_hw,
			UVCS_U32 tgt_mdl,
			struct UVCS_C_HANDLE *tgt_hdl
			);
UVCS_STATIC void uvcs_c_hw_finish(
			struct UVCS_C_INR_INFO *inr,
			UVCS_U32 tgt_hw,
			UVCS_U32 tgt_mdl
			);
UVCS_STATIC struct UVCS_C_INR_INFO *uvcs_c_mng_inr_param(
			UVCS_BOOL mng_mode,
			struct UVCS_C_INR_INFO *new_ptr
			);
UVCS_STATIC void uvcs_c_reg_read(
			volatile MCVX_REG *reg_addr,
			MCVX_U32 *dst_addr,
			MCVX_U32  num_reg
			);
UVCS_STATIC void uvcs_c_reg_write(
			volatile MCVX_REG *reg_addr,
			MCVX_U32 *src_addr,
			MCVX_U32  num_reg
			);
UVCS_STATIC void uvcs_c_memclr4(
			UVCS_U32 *dst_mem,
			UVCS_U32  num_bytes
			);

/*****************************************************************************/
/*                    EXTERNAL FUNCTIONS                                     */
/*****************************************************************************/

/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_interrupt
 *
 * Return
 *      \return none
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_interrupt(
	UVCS_CMN_LIB_INFO	lib_info, /**< [in] library information */
	UVCS_U32	base_addr, /**< [in] interrupted module address */
	UVCS_U32	cur_time /**< [in] current time (for debug) */
	)
{
	UVCS_RESULT result = UVCS_RTN_PARAMETER_ERROR;
	UVCS_U32 i;
	/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
	struct UVCS_C_INR_INFO *inr = (struct UVCS_C_INR_INFO *)lib_info;
	struct UVCS_CMN_INIT_PARAM *init;
	struct UVCS_C_HWU_INFO *hwused;

	/** (1) parameter check */
	/* PRQA S 0306 3 *//* unavoidable in the casts of the address. */
	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info)
	&& (inr->init_param.cb_thr_event != NULL)) {
		/** (2) search hardware number from base_addr */
		init = &inr->init_param;
		for (i = 0uL; i < init->hw_num; i++) {

			/** (3) execute interrupt handler in MCVX (VLC) */
			if (init->ip_base_addr[i][UVCS_CMN_MDL_VLC] == base_addr) {
				mcvx_vlc_interrupt_handler(&inr->driver_info[i]);
				/* VLC */
				hwused = &inr->hw_used[i][UVCS_CMN_MDL_VLC];
				if (hwused->hwp_state == UVCS_C_HWP_END) {
					hwused->cur_time  = cur_time;
					hwused->hwp_state = UVCS_C_HWP_FINISH;
				}
				result = UVCS_RTN_OK;
			}
			/** (3) execute interrupt handler in MCVX (CE and FCV) */
			if (init->ip_base_addr[i][UVCS_CMN_MDL_CE] == base_addr) {
				mcvx_ce_interrupt_handler(&inr->driver_info[i]);
				/* CE */
				hwused = &inr->hw_used[i][UVCS_CMN_MDL_CE];
				if (hwused->hwp_state == UVCS_C_HWP_END) {
					hwused->cur_time  = cur_time;
					hwused->hwp_state = UVCS_C_HWP_FINISH;
				}
				/* FCV */
				hwused = &inr->hw_used[i][UVCS_CMN_MDL_FCV];
				if (hwused->hwp_state == UVCS_C_HWP_END) {
					hwused->cur_time  = cur_time;
					hwused->hwp_state = UVCS_C_HWP_FINISH;
				}
				result = UVCS_RTN_OK;
			}

			if (result == UVCS_RTN_OK) {
				break;
			}
		}

		/** (final) uvcs_cmn_execute() should be called from thread */
		inr->init_param.cb_thr_event(inr->init_param.udptr);
	}

	return result;
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_initialize
 *
 * Return
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_SYSTEM_ERROR
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_initialize(
	UVCS_CMN_INIT_PARAM_T	*init_param, /**< [in] init parameter *//* PRQA S 3673 1 *//* do not use const. */
	UVCS_CMN_LIB_INFO	*lib_info /**< [out] library information */
	)
{
	struct UVCS_C_INR_INFO	*inr;
	UVCS_RESULT		 result;
	UVCS_U32		 i;
	MCVX_IP_CONFIG_T	 ip_config;

	/**
	 * \note QAC STMCC is over 10, but most items are parameter checks.
	 * This function is excluded.
	 */

	/** (1) parameter checking */
	if ((init_param == NULL)
	|| (lib_info == NULL)
	|| (init_param->struct_size < sizeof(struct UVCS_CMN_INIT_PARAM))) {
		result = UVCS_RTN_PARAMETER_ERROR;

	} else if ((init_param->hw_num > UVCS_C_NOEL_HW)
	|| (init_param->hw_num == 0uL)
	|| (init_param->work_mem_0_virt == NULL)
	|| (init_param->work_mem_0_size < sizeof(struct UVCS_C_INR_INFO))) {
		result = UVCS_RTN_PARAMETER_ERROR;

	} else if ((init_param->cb_reg_read == NULL)
	|| (init_param->cb_reg_write == NULL)
	|| (init_param->cb_proc_done == NULL)
	|| (init_param->cb_sem_create == NULL)
	|| (init_param->cb_sem_destroy == NULL)
	|| (init_param->cb_sem_lock == NULL)
	|| (init_param->cb_sem_unlock == NULL)
	|| (init_param->cb_thr_create == NULL)
	|| (init_param->cb_thr_destroy == NULL)
	|| (init_param->cb_thr_event == NULL)) {
		result = UVCS_RTN_PARAMETER_ERROR;

	/** (2) request to create semaphore */
	} else if (init_param->cb_sem_create(init_param->udptr) == UVCS_FALSE) {
		result = UVCS_RTN_SYSTEM_ERROR;
	/** (3) request to create thread */
	} else if (init_param->cb_thr_create(init_param->udptr) == UVCS_FALSE) {
		init_param->cb_sem_destroy(init_param->udptr);
		result = UVCS_RTN_SYSTEM_ERROR;
	} else {
		result = UVCS_RTN_OK;

		uvcs_c_memclr4(init_param->work_mem_0_virt, init_param->work_mem_0_size);
		inr = (struct UVCS_C_INR_INFO *)init_param->work_mem_0_virt; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
		inr->init_param = *init_param;

		/* initialise callback function for register access */
		(void)uvcs_c_mng_inr_param(UVCS_TRUE, inr);

		for (i = 0uL; i < init_param->hw_num; i++) {
			if ((init_param->ip_base_addr[i][UVCS_CMN_MDL_VLC] == 0uL) ||
				(init_param->ip_base_addr[i][UVCS_CMN_MDL_CE] == 0uL)) {
				result = UVCS_RTN_PARAMETER_ERROR;
			} else {
				ip_config.ip_id = i;
				ip_config.ip_vlc_base_addr = init_param->ip_base_addr[i][UVCS_CMN_MDL_VLC];
				ip_config.ip_ce_base_addr = init_param->ip_base_addr[i][UVCS_CMN_MDL_CE];
				ip_config.udf_reg_read = &uvcs_c_reg_read;
				ip_config.udf_reg_write = &uvcs_c_reg_write;
				if (mcvx_ip_init(&ip_config,
					&inr->driver_info[i]) != MCVX_NML_END) {
					result = UVCS_RTN_SYSTEM_ERROR;
				}
			}

			if (result != UVCS_RTN_OK) {
				break;
			}
		}

		if (result != UVCS_RTN_OK) {
			/* clear */
			(void)uvcs_c_mng_inr_param(UVCS_TRUE, NULL);
			init_param->cb_thr_destroy(init_param->udptr);
			init_param->cb_sem_destroy(init_param->udptr);
			uvcs_c_memclr4(init_param->work_mem_0_virt, sizeof(struct UVCS_C_INR_INFO));
		} else {
			(void)mcvx_get_ip_version(&inr->driver_info[0], &inr->ip_version);

			UVCS_CMN_DUMP_INIT(inr);
			inr->address = (UVCS_U32)inr;	/* PRQA S 0306 *//* unavoidable in the casts of the address. */
			*lib_info = (UVCS_CMN_LIB_INFO)inr; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
		}
	}

	return result;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_deinitialize
 *
 * Return
 *      \retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_CONTINUE
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_deinitialize(
	UVCS_CMN_LIB_INFO	 lib_info, /**< [in] library information */
	UVCS_BOOL		 forced
	)
{
	UVCS_RESULT		 result = UVCS_RTN_NOT_INITIALISE;
	struct UVCS_C_INR_INFO	*inr = (struct UVCS_C_INR_INFO *)lib_info; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */

	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info) /* PRQA S 0306 *//* unavoidable in the casts of the address. */
	&& (inr->init_param.cb_sem_lock != NULL)
	&& (inr->init_param.cb_sem_unlock != NULL)
	&& (inr->init_param.cb_sem_destroy != NULL)
	&& (inr->init_param.cb_thr_destroy != NULL)) {
		result = UVCS_RTN_OK;

		if (forced == UVCS_FALSE) {
			if (inr->init_param.cb_sem_lock(inr->init_param.udptr) == UVCS_FALSE) {
				result = UVCS_RTN_CONTINUE;
			}
		}

		UVCS_CMN_DUMP(inr, UVCS_CAPI_DEIN, UVCS_CP3((UVCS_U32)result), 0);

		if (result == UVCS_RTN_OK) {
			inr->address = 0uL;

			/* destroy resources */
			if (forced == UVCS_FALSE) {
				inr->init_param.cb_sem_unlock(inr->init_param.udptr);
			}
			inr->init_param.cb_thr_destroy(inr->init_param.udptr);
			inr->init_param.cb_sem_destroy(inr->init_param.udptr);
			(void)uvcs_c_mng_inr_param(UVCS_TRUE, NULL);
			/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
			uvcs_c_memclr4((UVCS_U32 *)inr, sizeof(struct UVCS_C_INR_INFO));
		}
	}

	return result;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_open
 *
 * Return
 *      \retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_CONTINUE
 *      \retval UVCS_RTN_BUSY
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_open(
	UVCS_CMN_LIB_INFO	 lib_info,	/**< [in] library information */
	UVCS_CMN_OPEN_PARAM_T	*open_param,	/**< [in] configuration for handle *//* PRQA S 3673 *//* do not use const. */
	UVCS_CMN_HANDLE		*handle	/**< [out] virtual device handle */
)
{
	struct UVCS_C_INR_INFO	 *inr    = (struct UVCS_C_INR_INFO *)lib_info; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
	UVCS_RESULT		  result = UVCS_RTN_NOT_INITIALISE;

	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info) /* PRQA S 0306 *//* unavoidable in the casts of the address. */
	&& (inr->init_param.cb_sem_lock != NULL)
	&& (inr->init_param.cb_sem_unlock != NULL)) {
		if ((handle == NULL)
		|| (open_param == NULL)
		|| (open_param->hdl_work_0_virt == NULL)
		|| (open_param->hdl_work_0_size < sizeof(struct UVCS_C_HANDLE))) {
			result = UVCS_RTN_PARAMETER_ERROR;

		} else if ((open_param->preempt_mode == UVCS_TRUE)
		&& (open_param->preempt_hwid >= inr->init_param.hw_num)) {
			result = UVCS_RTN_PARAMETER_ERROR;

		} else if (inr->init_param.cb_sem_lock(inr->init_param.udptr) == UVCS_FALSE) {
			result = UVCS_RTN_CONTINUE;

		} else if ((open_param->preempt_mode == UVCS_TRUE)
		&& ((inr->init_param.hw_num < 2uL) || (inr->preempt_hdl != NULL))) {
			inr->init_param.cb_sem_unlock(inr->init_param.udptr);
			result = UVCS_RTN_BUSY;

		} else {
			/* PRQA S 0310 2 *//* unavoidable in the casts of the structure. */
			struct UVCS_C_HANDLE *curr_head = inr->hdl_lst_head;
			struct UVCS_C_HANDLE *new_hdl = (struct UVCS_C_HANDLE *)open_param->hdl_work_0_virt;

			uvcs_c_memclr4(open_param->hdl_work_0_virt, sizeof(struct UVCS_C_HANDLE));
			*handle = (UVCS_CMN_HANDLE)open_param->hdl_work_0_virt;

			if (curr_head == NULL) { /* first context */
				inr->hdl_lst_head = new_hdl;
				inr->hdl_lst_tail = new_hdl;
				new_hdl->next = NULL;
				new_hdl->prev = NULL;
			} else {
				inr->hdl_lst_head = new_hdl;
				curr_head->prev = new_hdl;
				new_hdl->next = curr_head;
				new_hdl->prev = NULL;
			}

			if (open_param->preempt_mode == UVCS_TRUE) {
				inr->preempt_hdl = new_hdl;
				inr->preempt_hw  = open_param->preempt_hwid;
			}
			new_hdl->address = open_param->hdl_work_0_virt;
			new_hdl->open_info = *open_param;
			result = UVCS_RTN_OK;

			inr->init_param.cb_sem_unlock(inr->init_param.udptr);
		}

		UVCS_CMN_DUMP(inr, UVCS_CAPI_OPEN, UVCS_CP3((UVCS_U32)result), 0);
	}

	return result;
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_close
 *
 * Return
 *      \retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_INVALID_HANDLE
 *      \retval UVCS_RTN_CONTINUE
 *      \retval UVCS_RTN_BUSY
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_close(
	UVCS_CMN_LIB_INFO	 lib_info,	/**< [in] library information */
	UVCS_CMN_HANDLE		 handle,	/**< [in] virtual device handle */
	UVCS_BOOL		 forced
)
{
	UVCS_RESULT		 result = UVCS_RTN_NOT_INITIALISE;
	struct UVCS_C_INR_INFO	*inr = (struct UVCS_C_INR_INFO *)lib_info; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
	struct UVCS_C_HANDLE	*hdl = (struct UVCS_C_HANDLE *)handle;	  /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
	UVCS_U32		 ipid;
	UVCS_U32		 mdlid;

	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info) /* PRQA S 0306 *//* unavoidable in the casts of the address. */
	&& (inr->init_param.cb_sem_lock != NULL)
	&& (inr->init_param.cb_sem_unlock != NULL)) {

		if ((hdl == NULL)
		|| (hdl->address != handle)) {
			result = UVCS_RTN_INVALID_HANDLE;

		} else if (inr->init_param.cb_sem_lock(inr->init_param.udptr) == UVCS_FALSE) {
			result = UVCS_RTN_CONTINUE;

		} else {
			/* semaphore locked */
			if (forced == UVCS_TRUE) {
				hdl->module_info[UVCS_CMN_MDL_VLC].req_info = NULL;
				hdl->module_info[UVCS_CMN_MDL_CE].req_info = NULL;
				hdl->module_info[UVCS_CMN_MDL_FCV].req_info = NULL;

				/* remove from hw_used */
				for (ipid = 0uL; ipid < UVCS_C_NOEL_HW; ipid++) {
					for (mdlid = 0uL; mdlid < UVCS_C_NOEL_REQ; mdlid++) {
						if ((inr->hw_used[ipid][mdlid].hwp_state == UVCS_C_HWP_RUN)
						&& (inr->hw_used[ipid][mdlid].used_hdl == hdl)) {
							inr->hw_used[ipid][mdlid].hwp_state = UVCS_C_HWP_NONE;
							inr->hw_used[ipid][mdlid].used_hdl  = NULL;
						}
					}
				}
			}

			/* release non-executed requests */
			for (mdlid = 0uL; mdlid < UVCS_C_NOEL_REQ; mdlid++) {
				if ((hdl->module_info[mdlid].req_info != NULL) &&
					(hdl->module_info[mdlid].req_exec == UVCS_FALSE)) {
					hdl->module_info[mdlid].req_info = NULL;
				}
			}

			if ((hdl->module_info[UVCS_CMN_MDL_VLC].req_info != NULL)
			|| (hdl->module_info[UVCS_CMN_MDL_CE].req_info != NULL)
			|| (hdl->module_info[UVCS_CMN_MDL_FCV].req_info != NULL)) {
				result = UVCS_RTN_BUSY;

			} else {
				if (inr->preempt_hdl == hdl) {
					inr->preempt_hdl = NULL;
					inr->preempt_hw  = 0;
				}

				/* remove from list */
				if (hdl->next == NULL) {
					inr->hdl_lst_tail = hdl->prev;
				} else {
					hdl->next->prev = hdl->prev;
				}
				if (hdl->prev == NULL) {
					inr->hdl_lst_head = hdl->next;
				} else {
					hdl->prev->next = hdl->next;
				}

				/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
				uvcs_c_memclr4((UVCS_U32 *)hdl, sizeof(struct UVCS_C_HANDLE));
				result = UVCS_RTN_OK;
			}

			inr->init_param.cb_sem_unlock(inr->init_param.udptr);
		}

		UVCS_CMN_DUMP(inr, UVCS_CAPI_CLOS, UVCS_CP3((UVCS_U32)result), 0);
	}

	return result;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_request
 *
 * Return
 *      \retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_INVALID_HANDLE
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_CONTINUE
 *      \retval UVCS_RTN_BUSY
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_request(
	UVCS_CMN_LIB_INFO		 lib_info, /**< [in] library information */
	UVCS_CMN_HANDLE			 handle, /**< [in] virtual device handle */
	UVCS_CMN_HW_PROC_T		*req_info /**< [in] request information for hardware processing */
	)
{
	UVCS_RESULT result = UVCS_RTN_NOT_INITIALISE;
	struct UVCS_C_INR_INFO *inr = (struct UVCS_C_INR_INFO *)lib_info; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
	struct UVCS_C_HANDLE *hdl = (struct UVCS_C_HANDLE *)handle;	  /* PRQA S 0310 *//* unavoidable in the casts of the structure. */

	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info) /* PRQA S 0306 *//* unavoidable in the casts of the address. */
	&& (inr->init_param.cb_sem_lock != NULL)
	&& (inr->init_param.cb_sem_unlock != NULL)
	&& (inr->init_param.cb_thr_event != NULL)) {

		if ((hdl == NULL)
		|| (hdl->address != handle)) {
			result = UVCS_RTN_INVALID_HANDLE;
			UVCS_CMN_DUMP(inr, UVCS_CAPI_REQ0, UVCS_CP3((UVCS_U32)result), 0);

		} else if ((req_info == NULL)
		|| (req_info->struct_size < sizeof(UVCS_CMN_HW_PROC_T))
		|| (req_info->module_id >= UVCS_C_NOEL_REQ)) {
			result = UVCS_RTN_PARAMETER_ERROR;
			UVCS_CMN_DUMP(inr, UVCS_CAPI_REQ0, UVCS_CP3((UVCS_U32)result), 0);

		} else if (inr->init_param.cb_sem_lock(inr->init_param.udptr) == UVCS_FALSE) {
			result = UVCS_RTN_CONTINUE;
			UVCS_CMN_DUMP(inr, UVCS_CAPI_REQ0, UVCS_CP3((UVCS_U32)result), 0);

		} else {
			/* sempahore locked */
			struct UVCS_C_MODULE *mdl = &hdl->module_info[req_info->module_id];

			if (mdl->req_info != NULL) {
				result = UVCS_RTN_BUSY;
				UVCS_CMN_DUMP(inr, UVCS_CAPI_REQ0, UVCS_CP3((UVCS_U32)result), 0);

			} else {
				UVCS_CMN_DUMP(inr, UVCS_CAPI_REQ1, UVCS_CP3(req_info->module_id), req_info->req_serial);

				/* store request info */
				mdl->req_exec    = UVCS_FALSE;
				mdl->req_info    = req_info;

				/* request */
				inr->init_param.cb_thr_event(inr->init_param.udptr);

				result = UVCS_RTN_OK;
			}

			inr->init_param.cb_sem_unlock(inr->init_param.udptr);
		}
	}

	return result;
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_execute
 *
 * Return
 *      \retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_CONTINUE
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_execute(
	UVCS_CMN_LIB_INFO	 lib_info, /**< [in] library information */
	UVCS_U32		 cur_time /**< [in] current time for debug */
	)
{
	UVCS_RESULT	 result = UVCS_RTN_NOT_INITIALISE;
	/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
	struct UVCS_C_INR_INFO	*inr = (struct UVCS_C_INR_INFO *)lib_info;
	UVCS_U32	 h_idx;
	UVCS_U32	 m_idx;

	/** (1) initialize check */
	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info) /* PRQA S 0306 *//* unavoidable in the casts of the address. */
	&& (inr->init_param.cb_sem_lock != NULL)
	&& (inr->init_param.cb_sem_unlock != NULL)) {

		/** (2) semaphore lock */
		if (inr->init_param.cb_sem_lock(inr->init_param.udptr) == UVCS_FALSE) {
			result = UVCS_RTN_CONTINUE;

		} else {
			/** (3) change hw_state and output log */
			for (h_idx = 0uL; h_idx < inr->init_param.hw_num; h_idx++) {
				if (inr->hw_used[h_idx][UVCS_CMN_MDL_VLC].hwp_state == UVCS_C_HWP_FINISH) {
					/* change state */
					uvcs_c_hw_finish(inr, h_idx, UVCS_CMN_MDL_VLC);
				}
				if (inr->hw_used[h_idx][UVCS_CMN_MDL_CE].hwp_state == UVCS_C_HWP_FINISH) {
					/* change state */
					uvcs_c_hw_finish(inr, h_idx, UVCS_CMN_MDL_CE);
				}
				if (inr->hw_used[h_idx][UVCS_CMN_MDL_FCV].hwp_state == UVCS_C_HWP_FINISH) {
					/* change state */
					uvcs_c_hw_finish(inr, h_idx, UVCS_CMN_MDL_FCV);
				}
			}

			/** (4) search executable handle and execute it */
			for (h_idx = 0uL; h_idx < inr->init_param.hw_num; h_idx++) {
				if ((inr->preempt_hdl != NULL)
				&& (inr->preempt_hw == h_idx)) {
					for (m_idx = 0uL; m_idx < UVCS_C_NOEL_REQ; m_idx++) {
						if ((inr->hw_used[h_idx][m_idx].hwp_state == UVCS_C_HWP_NONE) /* idle */
						&& (inr->preempt_hdl->module_info[m_idx].req_info != NULL)) { /* request exist */
							/* The checking of req_exec is not needed here. Because the value of hwp_state and req_exec
							 * are always changed on finish timing.
							 */
							inr->hw_used[h_idx][m_idx].cur_time = cur_time;
							uvcs_c_hw_execute(inr, h_idx, m_idx, inr->preempt_hdl);
						}
					}
				} else {
#if (UVCS_C_FCV_UNIFIED == UVCS_C_YES)
					if (inr->hw_used[h_idx][UVCS_CMN_MDL_VLC].hwp_state == UVCS_C_HWP_NONE) {
						struct UVCS_C_HANDLE *candidate = uvcs_c_search_hdl(inr, UVCS_CMN_MDL_VLC);
						if (candidate != NULL) {
							inr->hw_used[h_idx][UVCS_CMN_MDL_VLC].cur_time = cur_time;
							uvcs_c_hw_execute(inr, h_idx, UVCS_CMN_MDL_VLC, candidate);
						}
					}
					if ((inr->hw_used[h_idx][UVCS_CMN_MDL_FCV].hwp_state == UVCS_C_HWP_NONE)
					&& (inr->hw_used[h_idx][UVCS_CMN_MDL_CE].hwp_state == UVCS_C_HWP_NONE)) {
						struct UVCS_C_HANDLE *candidate = uvcs_c_search_hdl(inr, UVCS_CMN_MDL_FCV);
						if (candidate != NULL) {
							inr->hw_used[h_idx][UVCS_CMN_MDL_FCV].cur_time = cur_time;
							uvcs_c_hw_execute(inr, h_idx, UVCS_CMN_MDL_FCV, candidate);
						}
					}
					if ((inr->hw_used[h_idx][UVCS_CMN_MDL_FCV].hwp_state == UVCS_C_HWP_NONE)
					&& (inr->hw_used[h_idx][UVCS_CMN_MDL_CE].hwp_state == UVCS_C_HWP_NONE)) {
						struct UVCS_C_HANDLE *candidate = uvcs_c_search_hdl(inr, UVCS_CMN_MDL_CE);
						if (candidate != NULL) {
							inr->hw_used[h_idx][UVCS_CMN_MDL_CE].cur_time = cur_time;
							uvcs_c_hw_execute(inr, h_idx, UVCS_CMN_MDL_CE, candidate);
						}
					}
#else
					for (m_idx = 0uL; m_idx < UVCS_C_NOEL_REQ; m_idx++) {
						if (inr->hw_used[h_idx][m_idx].hwp_state == UVCS_C_HWP_NONE) {
							struct UVCS_C_HANDLE *candidate = uvcs_c_search_hdl(inr, m_idx);
							if (candidate != NULL) {
								inr->hw_used[h_idx][m_idx].cur_time = cur_time;
								uvcs_c_hw_execute(inr, h_idx, m_idx, candidate);
							}
						}
					}
#endif
				}
			}

			/** (4) semaphore unlock */
			inr->init_param.cb_sem_unlock(inr->init_param.udptr);
			result = UVCS_RTN_OK;
		}
	}

	return result;
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_set_preempt_mode
 *
 * Return
 *      \retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_CONTINUE
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_set_preempt_mode(
	UVCS_CMN_LIB_INFO	 lib_info,
	UVCS_CMN_HANDLE		 handle,	/**< [in] virtual device handle */
	UVCS_BOOL		 preempt_mode,
	UVCS_U32		 preempt_hwid
	)
{
	UVCS_RESULT		 result = UVCS_RTN_NOT_INITIALISE;
	struct UVCS_C_INR_INFO	*inr = (struct UVCS_C_INR_INFO *)lib_info; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
	struct UVCS_C_HANDLE	*hdl = (struct UVCS_C_HANDLE *)handle; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */

	/** (1) parameter check */
	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info) /* PRQA S 0306 *//* unavoidable in the casts of the address. */
	&& (inr->init_param.cb_sem_lock != NULL)
	&& (inr->init_param.cb_sem_unlock != NULL)) {

		if ((hdl == NULL)
		|| (hdl->address != handle)) {
			result = UVCS_RTN_INVALID_HANDLE;
		}
		/** (2) semaphore lock */
		else if (inr->init_param.cb_sem_lock(inr->init_param.udptr) == UVCS_FALSE) {
			result = UVCS_RTN_CONTINUE;
		} else {
			if (preempt_mode == UVCS_TRUE) {
				if (inr->preempt_hdl != NULL) {
					result = UVCS_RTN_BUSY;
				} else if (preempt_hwid >= inr->init_param.hw_num) {
					result = UVCS_RTN_PARAMETER_ERROR;
				} else {
					hdl->open_info.preempt_mode = preempt_mode;
					hdl->open_info.preempt_hwid = preempt_hwid;
					inr->preempt_hdl = hdl;
					inr->preempt_hw  = preempt_hwid;
					result = UVCS_RTN_OK;
				}
			} else {
				hdl->open_info.preempt_mode = UVCS_FALSE;
				hdl->open_info.preempt_hwid = 0;
				inr->preempt_hdl = NULL;
				inr->preempt_hw  = 0;
				result = UVCS_RTN_OK;
			}

			inr->init_param.cb_sem_unlock(inr->init_param.udptr);
		}

		UVCS_CMN_DUMP(inr, UVCS_CAPI_PEMP, UVCS_CP3((UVCS_U32)result), 0);
	}

	return result;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_get_work_size
 *
 * Return
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_get_work_size(
	UVCS_U32	*work_mem_0_size,
	UVCS_U32	*hdl_work_0_size
	)
{
	UVCS_RESULT	 result;

	/** (1) parameter check */
	if ((work_mem_0_size == NULL)
	|| (hdl_work_0_size == NULL)) {
		result = UVCS_RTN_PARAMETER_ERROR;
	} else {
		*work_mem_0_size = sizeof(struct UVCS_C_INR_INFO);
		*hdl_work_0_size = sizeof(struct UVCS_C_HANDLE);
		result = UVCS_RTN_OK;
	}

	return result;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_cmn_get_ip_info
 *
 * Return
 *		\retval UVCS_RTN_NOT_INITIALISE
 *      \retval UVCS_RTN_PARAMETER_ERROR
 *      \retval UVCS_RTN_OK
 *
 ****************************************************************/
UVCS_RESULT uvcs_cmn_get_ip_info(
	UVCS_CMN_LIB_INFO		 lib_info,
	UVCS_CMN_IP_INFO_T		*ip_info
)
{
	UVCS_RESULT result = UVCS_RTN_NOT_INITIALISE;
	struct UVCS_C_INR_INFO *inr = (struct UVCS_C_INR_INFO *)lib_info; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */

	/** (1) parameter check */
	if ((inr != NULL)
	&& (inr->address == (UVCS_U32)lib_info)) { /* PRQA S 0306 *//* unavoidable in the casts of the address. */
		if ((ip_info == NULL)
		|| (ip_info->struct_size < sizeof(struct UVCS_CMN_IP_INFO))) {
			result = UVCS_RTN_PARAMETER_ERROR;
		} else {
			ip_info->struct_size = sizeof(struct UVCS_CMN_IP_INFO);
			ip_info->ip_version  = inr->ip_version;
			ip_info->ip_option   = inr->init_param.ip_option;
			result = UVCS_RTN_OK;
		}
	}

	return result;
}

/******************************************************************************/
/*                    INTERNAL FUNCTIONS                                      */
/******************************************************************************/

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_mng_inr_param
 *
 * Return
 *      \return pointer of local parameter
 *
 * Description
 *     \brief Set and Get a pointer of local parameter.
 *
 ****************************************************************/
UVCS_STATIC struct UVCS_C_INR_INFO *uvcs_c_mng_inr_param(
	UVCS_BOOL		 mng_mode,	/**< [in] TRUE: change to new pointer */
	struct UVCS_C_INR_INFO	*new_ptr	/**< [in] new pointer value */
)
{
	static struct UVCS_C_INR_INFO *inr_param;

	if (mng_mode == UVCS_TRUE) {
		inr_param = new_ptr;
	}

	return inr_param;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_reg_read
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Read data from register. This function is callback function for driver.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_reg_read(
	volatile MCVX_REG	* reg_addr,	/**< [in] pointer of target register */
	MCVX_U32		*dst_addr,	/**< [out] register value */
	MCVX_U32		 num_reg	/**< [in] number of register to read */
)
{
	struct UVCS_C_INR_INFO *inr = uvcs_c_mng_inr_param(UVCS_FALSE, NULL);

	if ((inr != NULL)
	&& (inr->init_param.cb_reg_read != NULL)) {
		(*inr->init_param.cb_reg_read)(inr->init_param.udptr, reg_addr, dst_addr, num_reg);
	}
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_reg_write
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Write data to register. This function is callback function for driver.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_reg_write(
	volatile MCVX_REG	*reg_addr,	/**< [in] pointer of target register */
	MCVX_U32		*src_addr,	/**< [in] source data */
	MCVX_U32		 num_reg	/**< [in] number of register to write */
)
{
	struct UVCS_C_INR_INFO *inr = uvcs_c_mng_inr_param(UVCS_FALSE, NULL);

	if ((inr != NULL)
	&& (inr->init_param.cb_reg_write != NULL)) {
		(*inr->init_param.cb_reg_write)(inr->init_param.udptr, reg_addr, src_addr, num_reg);
	}
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_search_hdl
 *
 * Return
 *      \return pointer of handle inforamtion
 *
 * Description
 *     \brief Search a handle who has a lowest timing value.
 *
 ****************************************************************/
UVCS_STATIC struct UVCS_C_HANDLE *uvcs_c_search_hdl(
	struct UVCS_C_INR_INFO	 *inr,		/**< [in] local information *//* PRQA S 3673 *//* do not use const. */
	UVCS_U32		  mdl_id	/**< [in] module identifier */
)
{
	struct UVCS_C_HANDLE	*cur_hdl;
	struct UVCS_C_HANDLE	*candidate = NULL;
	UVCS_U32		 min_tmg = 0xFFFFFFFFuL;
	UVCS_U32		 merge_tmg = 0xFFFFFFFFuL;

	if ((inr != NULL)
	&& (mdl_id < UVCS_C_NOEL_REQ)) {
		cur_hdl = inr->hdl_lst_head;

		while (cur_hdl != NULL) {
			if ((cur_hdl->open_info.preempt_mode == UVCS_FALSE)
			&& (cur_hdl->module_info[mdl_id].req_info != NULL) /* request exists */
			&& (cur_hdl->module_info[mdl_id].req_exec == UVCS_FALSE)) { /* not executed */
				if (cur_hdl->module_info[mdl_id].tmg_ctrl < min_tmg) {
					min_tmg = cur_hdl->module_info[mdl_id].tmg_ctrl;
					candidate = cur_hdl;
					merge_tmg = cur_hdl->module_info[mdl_id].req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_TMG];
				} else if (cur_hdl->module_info[mdl_id].tmg_ctrl == min_tmg) {
					if (cur_hdl->module_info[mdl_id].req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_TMG] < merge_tmg) {
						candidate = cur_hdl;
						merge_tmg = cur_hdl->module_info[mdl_id].req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_TMG];
					}
				} else {
					;
				}
			}
			cur_hdl = cur_hdl->next;
		}

		if (candidate != NULL) {
			cur_hdl = inr->hdl_lst_head;

			while (cur_hdl != NULL) {
				if ((cur_hdl->open_info.preempt_mode == UVCS_FALSE) /* normal handle */
				&& (cur_hdl->module_info[mdl_id].req_info != NULL) /* request exists */
				&& (cur_hdl->module_info[mdl_id].req_exec == UVCS_FALSE)) { /* not executed */
					cur_hdl->module_info[mdl_id].tmg_ctrl -= min_tmg; /* normallize */
				}
				cur_hdl = cur_hdl->next;
			}
			candidate->module_info[mdl_id].tmg_ctrl = merge_tmg;
		}
	}

	return candidate;
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_hw_execute
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Start hardware processing.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_hw_execute(
	struct UVCS_C_INR_INFO		*inr,		/**< [in] local parameters */
	UVCS_U32			 tgt_hw,	/**< [in] target hardware identifier */
	UVCS_U32			 tgt_mdl,	/**< [in] target module identifier */
	struct UVCS_C_HANDLE		*tgt_hdl	/**< [in] target handle */
	)
{
	if ((inr != NULL)
	&& (tgt_mdl < UVCS_C_NOEL_REQ)
	&& (tgt_hdl != NULL)
	&& (tgt_hw < inr->init_param.hw_num)) {
		struct UVCS_C_MODULE	*mdl_info = &tgt_hdl->module_info[tgt_mdl];
		struct UVCS_C_HWU_INFO	*use_info = &inr->hw_used[tgt_hw][tgt_mdl];
		MCVX_IP_INFO_T		*ip_info  = &inr->driver_info[tgt_hw];
		MCVX_VLC_BAA_T		*vlc_baa;
		MCVX_CE_BAA_T		*ce_baa;

		use_info->used_hdl    = tgt_hdl;
		use_info->local       = inr;
		mdl_info->req_exec = UVCS_TRUE;
		mdl_info->exe_cnt++; /* for debug */

		if (inr->init_param.cb_hw_start != NULL) {
			inr->init_param.cb_hw_start(inr->init_param.udptr,
					tgt_hw,
					tgt_mdl,
					&mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_BAA]);
		}

		switch (tgt_mdl) {
		case UVCS_CMN_MDL_VLC:
			/* log */
			/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
			vlc_baa	= (MCVX_VLC_BAA_T *)&mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_BAA];
			UVCS_CMN_DUMP(inr, UVCS_CAPI_RUNV, tgt_hw,                  0);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAV0, vlc_baa->irp_p_addr,     vlc_baa->list_item_p_addr);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAV1, vlc_baa->imc_buff_addr,  vlc_baa->ims_buff_addr[0]);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAV2, vlc_baa->str_es_addr[0], vlc_baa->str_es_size[0]);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAV3, vlc_baa->str_es_addr[1], vlc_baa->str_es_size[1]);

			/* start hardware */
			(void)mcvx_vlc_start(ip_info,
					use_info,
					(MCVX_UDF_VLC_EVENT_T)&uvcs_c_vlc_event,	/* callback ptr */
					/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
					(MCVX_VLC_EXE_T *)&mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_DAT]);
			break;

		case UVCS_CMN_MDL_CE:
			/* log */
			/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
			ce_baa = (MCVX_CE_BAA_T *)&mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_BAA];
			UVCS_CMN_DUMP(inr, UVCS_CAPI_RUNC, tgt_hw,                0);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAC0, ce_baa->irp_p_addr,    ce_baa->img_ref_num);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAC1, ce_baa->imc_buff_addr, ce_baa->ims_buff_addr[0]);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAC2, ce_baa->stride_dec,    ce_baa->stride_ref);
			UVCS_CMN_DUMP(inr, UVCS_CAPI_BAC3, ce_baa->stride_enc_y,  ce_baa->stride_enc_c);

			/* start hardware */
			(void)mcvx_ce_start(ip_info,
					use_info,
					(MCVX_UDF_CE_EVENT_T)&uvcs_c_ce_event,	/* callback ptr */
					/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
					(MCVX_CE_EXE_T *)&mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_DAT]);
			break;

		default:
			/* log */
			UVCS_CMN_DUMP(inr, UVCS_CAPI_RUNF, tgt_hw, 0);

			/* start hardware */
			(void)mcvx_fcv_start(ip_info,
					use_info,
					(MCVX_UDF_FCV_EVENT_T)&uvcs_c_fcv_event,	/* callback ptr */
					/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
					(MCVX_FCV_EXE_T *)&mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_REQ_DAT]);
			break;
		}

		mdl_info->req_info->cmd_param[UVCS_CMN_HWPIDX_RES_EST] = use_info->cur_time; /* for debug */
	}
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_c_hw_finish
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Stop hardware processing.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_hw_finish(
	struct UVCS_C_INR_INFO	*inr,		/**< [in] local parameter */
	UVCS_U32		 tgt_hw,	/**< [in] target hardware identifier */
	UVCS_U32		 tgt_mdl	/**< [in] target module identifier */
	)
{
	if ((inr != NULL)
	&& (tgt_hw < inr->init_param.hw_num)
	&& (tgt_mdl < UVCS_C_NOEL_REQ)
	&& (inr->init_param.cb_proc_done != NULL)
	&& (inr->hw_used[tgt_hw][tgt_mdl].used_hdl != NULL)) {
		struct UVCS_C_HANDLE	*cur_hdl  = inr->hw_used[tgt_hw][tgt_mdl].used_hdl;
		UVCS_CMN_HW_PROC_T	*res	  = cur_hdl->module_info[tgt_mdl].req_info;

		if (res != NULL) {
			switch (tgt_mdl) {
			case UVCS_CMN_MDL_VLC:
				/* log */
				UVCS_CMN_DUMP(inr, UVCS_CAPI_ENDV, tgt_hw, 0);
				UVCS_CMN_DUMP(inr, UVCS_CAPI_ENDT, res->cmd_param[UVCS_CMN_HWPIDX_RES_EST], inr->hw_used[tgt_hw][tgt_mdl].cur_time);

				/* stop hardware */
				/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
				(void)mcvx_vlc_stop(&inr->driver_info[tgt_hw], (MCVX_VLC_RES_T *)&res->cmd_param[UVCS_CMN_HWPIDX_RES_DAT]);
				/* write result */
				res->module_id      = UVCS_CMN_MDL_VLC; /* by way of safety */
				res->cmd_param_noel = ((sizeof(MCVX_VLC_RES_T) + 3uL) >> 2uL) + UVCS_CMN_HWP_NOEL_BASE;
				/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
				res->cmd_param[UVCS_CMN_HWPIDX_RES_STA] = ((MCVX_VLC_RES_T *)&res->cmd_param[UVCS_CMN_HWPIDX_RES_DAT])->error_level;
				break;

			case UVCS_CMN_MDL_CE:
				/* log */
				UVCS_CMN_DUMP(inr, UVCS_CAPI_ENDC, tgt_hw, 0);
				UVCS_CMN_DUMP(inr, UVCS_CAPI_ENDT, res->cmd_param[UVCS_CMN_HWPIDX_RES_EST], inr->hw_used[tgt_hw][tgt_mdl].cur_time);

				/* stop hardware */
				/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
				(void)mcvx_ce_stop(&inr->driver_info[tgt_hw], (MCVX_CE_RES_T *)&res->cmd_param[UVCS_CMN_HWPIDX_RES_DAT]);
				/* write result */
				res->module_id      = UVCS_CMN_MDL_CE; /* by way of safety */
				res->cmd_param_noel = ((sizeof(MCVX_CE_RES_T) + 3uL) >> 2uL) + UVCS_CMN_HWP_NOEL_BASE;
				/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
				res->cmd_param[UVCS_CMN_HWPIDX_RES_STA] = ((MCVX_CE_RES_T *)&res->cmd_param[UVCS_CMN_HWPIDX_RES_DAT])->error_code;
				break;

			default:
				/* log */
				UVCS_CMN_DUMP(inr, UVCS_CAPI_ENDF, tgt_hw, 0);
				UVCS_CMN_DUMP(inr, UVCS_CAPI_ENDT, res->cmd_param[UVCS_CMN_HWPIDX_RES_EST], inr->hw_used[tgt_hw][tgt_mdl].cur_time);

				/* stop hardware */
				/* PRQA S 0310 1 *//* unavoidable in the casts of the structure. */
				(void)mcvx_fcv_stop(&inr->driver_info[tgt_hw], (MCVX_FCV_RES_T *)&res->cmd_param[UVCS_CMN_HWPIDX_RES_DAT]);
				/* write result */
				res->module_id      = UVCS_CMN_MDL_FCV; /* by way of safety */
				res->cmd_param_noel = ((sizeof(MCVX_CE_RES_T) + 3uL) >> 2uL) + UVCS_CMN_HWP_NOEL_BASE;
				res->cmd_param[UVCS_CMN_HWPIDX_RES_STA] = 0;
				break;
			}

			if (inr->init_param.cb_hw_stop != NULL) {
				inr->init_param.cb_hw_stop(inr->init_param.udptr, tgt_hw, tgt_mdl);
			}

			/* write remain result data */
			res->struct_size = sizeof(UVCS_CMN_HW_PROC_T);
			res->cmd_param[UVCS_CMN_HWPIDX_RES_CVL] = cur_hdl->module_info[UVCS_CMN_MDL_VLC].exe_cnt;
			res->cmd_param[UVCS_CMN_HWPIDX_RES_CCE] = cur_hdl->module_info[UVCS_CMN_MDL_CE].exe_cnt;
			res->cmd_param[UVCS_CMN_HWPIDX_RES_CFC] = cur_hdl->module_info[UVCS_CMN_MDL_FCV].exe_cnt;
			res->cmd_param[UVCS_CMN_HWPIDX_RES_EET] = inr->hw_used[tgt_hw][tgt_mdl].cur_time;

			inr->init_param.cb_proc_done(
				inr->init_param.udptr,
				cur_hdl->open_info.hdl_udptr,
				(UVCS_CMN_HANDLE)cur_hdl, /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
				res);
			cur_hdl->module_info[tgt_mdl].req_info = NULL;
			cur_hdl->module_info[tgt_mdl].req_exec = UVCS_FALSE;
			inr->hw_used[tgt_hw][tgt_mdl].used_hdl = NULL;
			inr->hw_used[tgt_hw][tgt_mdl].hwp_state = UVCS_C_HWP_NONE;
		}
	}
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_vlc_event
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Callback function for driver.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_vlc_event(
	void		*udp,		/**< [in] library information */
	MCVX_U32	 vlc_event,	/**< [in] event identifier */
	void		*unused		/**< [in] event parameter (unused) *//* PRQA S 3673 *//* do not use const. */
	)
{
	struct UVCS_C_HWU_INFO *use_info = (struct UVCS_C_HWU_INFO *)udp;

	if (use_info != NULL) {
		switch (vlc_event) {
		case MCVX_EVENT_VLC_START:
			use_info->hwp_state = UVCS_C_HWP_RUN;
			UVCS_CMN_ASSERT(use_info->local, unused == NULL);
			break;

		case MCVX_EVENT_REQ_VLC_STOP:
			use_info->hwp_state  = UVCS_C_HWP_END;
			UVCS_CMN_ASSERT(use_info->local, unused != NULL);
			break;

		default:
			break;
		}
	}
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_ce_event
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Callback function for driver.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_ce_event(
	void		*udp,		/**< [in] library information */
	MCVX_U32	 ce_event,	/**< [in] event identifier */
	void		*unused		/**< [in] event parameter (unused) *//* PRQA S 3673 *//* do not use const. */
	)
{
	struct UVCS_C_HWU_INFO *use_info = (struct UVCS_C_HWU_INFO *)udp;

	if (use_info != NULL) {
		switch (ce_event) {
		case MCVX_EVENT_CE_START:
			use_info->hwp_state = UVCS_C_HWP_RUN;
			UVCS_CMN_ASSERT(use_info->local, unused == NULL);
			break;

		case MCVX_EVENT_REQ_CE_STOP:
			use_info->hwp_state  = UVCS_C_HWP_END;
			UVCS_CMN_ASSERT(use_info->local, unused != NULL);
			break;

		default:
			break;
		}
	}
}

/************************************************************//**
 *
 * Function Name
 *      uvcs_c_fcv_event
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Callback function for driver.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_fcv_event(
	void		*udp,		 /**< [in] library information */
	MCVX_U32	 fcv_event,	 /**< [in] event identifier */
	void		*unused		 /**< [in] event parameter (unused) *//* PRQA S 3673 *//* do not use const. */
	)
{
	struct UVCS_C_HWU_INFO *use_info = (struct UVCS_C_HWU_INFO *)udp;

	if (use_info != NULL) {
		switch (fcv_event) {
		case MCVX_EVENT_FCV_START:
			use_info->hwp_state = UVCS_C_HWP_RUN;
			UVCS_CMN_ASSERT(use_info->local, unused == NULL);
			break;

		case MCVX_EVENT_REQ_FCV_STOP:
			use_info->hwp_state  = UVCS_C_HWP_END;
			UVCS_CMN_ASSERT(use_info->local, unused != NULL);
			break;

		default:
			break;
		}
	}
}


/************************************************************//**
 *
 * Function Name
 *      uvcs_c_memclr4
 *
 * Return
 *      \return none
 *
 * Description
 *     \brief Clear target memory area.
 *
 ****************************************************************/
UVCS_STATIC void uvcs_c_memclr4(
	UVCS_U32	*dst_mem, /**< [io] destination address */
	UVCS_U32	 num_bytes /**< [in] length of memory area */
	)
{
	UVCS_U32	 cnt;
	UVCS_U32	*dst_ptr = (UVCS_U32 *)dst_mem;

	if (dst_mem != NULL) {
		num_bytes >>= 2uL;

		for (cnt = 0uL; cnt < num_bytes; cnt++) {
			/* PRQA S 0491 2 */
			/* substitute of the pointer arithmetic. */
			dst_ptr[cnt] = 0uL;
		}
	}
}
