/*
 * Copyright(C) 2013 Renesas Electronics Corporation. All Rights Reserved.
 * RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY
 * This program must be used solely for the purpose for which
 * it was furnished by Renesas Electronics Corporation.
 * No part of this program may be reproduced or disclosed to
 * others, in any form, without the prior written permission
 * of Renesas Electronics Corporation.
 */

/**
 * UVCS UDF(User Defined Function) Implementation
 *
 * \file
 * \attention
 */


/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "omxr_uvcs_udf.h"
#include "omxr_uvcs_udf_osal.h"
#include "omxr_module_common.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef struct tagOMXR_UVCS_UDF_STRUCT {
	OMXR_UVCS_UDF_OSAL_HANDLE pvUvcsUdfOsalHandle;
	OMX_PTR pvUserContext;
	OMXR_UVCS_UDF_CALLBACK_FUNCTYPE fCallback;
	OMX_U32 u32ThreadId;
	OMX_BOOL bTerminateRequest;
	OMX_U32 u32StructSize;
	OMX_PTR pvHwProcReadData;
} OMXR_UVCS_UDF_STRUCT;

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/
/**
 * UVCS UDF Thread Function
 *
 * This thread is listener thread that is to wait for the end of UVCS process
 *
 * \param[in]      u32ProcID
 * \param[in]      pvParam
 * \param[in]      pvContext
 * \return         OMX_ERROTTYPE
 * \attention
 */
static OMX_ERRORTYPE OmxrUvcsUdf_Thread(OMX_U32 u32ProcID, OMX_PTR pvParam, OMX_PTR pvContext);

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
OMX_ERRORTYPE OmxrUvcsUdf_Open(OMXR_UVCS_UDF_HANDLE *hUdfHandle, OMX_PTR pvUserContext, OMXR_UVCS_UDF_OPEN_PARAM *psParam)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_UVCS_UDF_STRUCT *psUvcsUdf = NULL;

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_DEBUG, "", "");

	if( (hUdfHandle == NULL) || (pvUserContext == NULL) || (psParam == NULL) ){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	if(psParam->fCallback == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psUvcsUdf = (OMXR_UVCS_UDF_STRUCT *)OmxrMalloc(sizeof(OMXR_UVCS_UDF_STRUCT));
	if(psUvcsUdf == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "failed at malloc", "");
		goto cleanup;
	}
	(void)OmxrMemset(psUvcsUdf, 0, sizeof(OMXR_UVCS_UDF_STRUCT));

	eError = OmxrUvcsUdfOsal_Open(&psUvcsUdf->pvUvcsUdfOsalHandle);
	if(eError != OMX_ErrorNone){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "OmxrUvcsUdfOsal_Open Failed(H'%08x)", (OMX_U32)eError);
		goto cleanup;
	}

	eError = OmxrCreateThread(&psUvcsUdf->u32ThreadId, (OMXR_THREAD_FUNCTION)&OmxrUvcsUdf_Thread, (OMX_PTR)psUvcsUdf, "PRIORITY.HIGH.ME.UDF.UVCS", 0);
	if(eError != OMX_ErrorNone){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		goto cleanup;
	}

	psUvcsUdf->fCallback = psParam->fCallback;
	psUvcsUdf->pvUserContext = pvUserContext;
	psUvcsUdf->pvHwProcReadData = NULL;
	psUvcsUdf->u32StructSize = 0;

	*hUdfHandle = (OMXR_UVCS_UDF_HANDLE)psUvcsUdf;

	return OMX_ErrorNone;

cleanup:
	if(psUvcsUdf->pvUvcsUdfOsalHandle != NULL){
		(void)OmxrUvcsUdfOsal_Close(psUvcsUdf->pvUvcsUdfOsalHandle);
	}
	if(psUvcsUdf != NULL){
		OmxrFree(psUvcsUdf);
	}
	return OMX_ErrorUndefined;
}

OMX_ERRORTYPE OmxrUvcsUdf_Close(OMXR_UVCS_UDF_HANDLE hUdfHandle)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_UVCS_UDF_STRUCT *psUvcsUdf;
	OMX_U32 u32Ret;

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_DEBUG, "", "");

	if(hUdfHandle == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psUvcsUdf = (OMXR_UVCS_UDF_STRUCT *)hUdfHandle;

	psUvcsUdf->bTerminateRequest = OMX_TRUE;

	(void)OmxrUvcsUdfOsal_CancelRead(hUdfHandle->pvUvcsUdfOsalHandle);

	eError = OmxrTerminateThread(psUvcsUdf->u32ThreadId);
	if (OMX_ErrorNone != eError) {
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		eError = OMX_ErrorUndefined;
	}
	else {
		eError = OmxrJoinThread(psUvcsUdf->u32ThreadId, NULL);
		if (OMX_ErrorNone != eError) {
			OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
			eError = OMX_ErrorUndefined;
		}
	}
	if(psUvcsUdf->pvUvcsUdfOsalHandle != NULL){
		u32Ret = OmxrUvcsUdfOsal_Close(psUvcsUdf->pvUvcsUdfOsalHandle);
		if(u32Ret != 0){
			OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		}
	}
	if(psUvcsUdf->pvHwProcReadData != NULL){
		OmxrFree(psUvcsUdf->pvHwProcReadData);
	}

	OmxrFree(psUvcsUdf);

	return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxrUvcsUdf_ProcRequest(OMXR_UVCS_UDF_HANDLE hUdfHandle, OMX_PTR pvRequestData)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_UVCS_UDF_STRUCT *psUvcsUdf;
	OMX_U32 u32SizeOfStruct;

	if( (hUdfHandle == NULL) || (pvRequestData == NULL) ){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psUvcsUdf = (OMXR_UVCS_UDF_STRUCT *)hUdfHandle;

	u32SizeOfStruct = *((OMX_U32 *)pvRequestData); 

	if(psUvcsUdf->pvHwProcReadData == NULL){
		psUvcsUdf->u32StructSize = u32SizeOfStruct;
		psUvcsUdf->pvHwProcReadData = OmxrMalloc(psUvcsUdf->u32StructSize);
		if(psUvcsUdf->pvHwProcReadData == NULL){
			OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
			return OMX_ErrorUndefined;
		}
	}

	eError = OmxrUvcsUdfOsal_Write(psUvcsUdf->pvUvcsUdfOsalHandle, pvRequestData, u32SizeOfStruct);
	if(eError != OMX_ErrorNone){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}

	eError = OmxrSendEvent(psUvcsUdf->u32ThreadId, 0, pvRequestData);
	if(eError != OMX_ErrorNone){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxrUvcsUdf_GetIpInfo(OMXR_UVCS_UDF_HANDLE hUdfHandle, OMX_PTR pvIpInfo)
{
	OMXR_UVCS_UDF_STRUCT *psUvcsUdf;

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_DEBUG, "", "");

	if( (hUdfHandle == NULL) || (pvIpInfo == NULL) ){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psUvcsUdf = (OMXR_UVCS_UDF_STRUCT *)hUdfHandle;

	return OmxrUvcsUdfOsal_GetIpInfo(psUvcsUdf->pvUvcsUdfOsalHandle, pvIpInfo);
}

static OMX_ERRORTYPE OmxrUvcsUdf_Thread(OMX_U32 u32ProcID, OMX_PTR pvParam, OMX_PTR pvContext)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_UVCS_UDF_STRUCT *psUvcsUdf;

	if (u32ProcID >= 0xFFFFFFF0UL) {
		if (u32ProcID == 0xFFFFFFFFUL) {
			if (NULL == pvParam) {
				OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
				eError = OMX_ErrorBadParameter;
			} else {
				OMX_U32 u32ThreadID;

				eError = OmxrGetThreadId(&u32ThreadID);
				if (OMX_ErrorNone != eError) {
					OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
				} else {
					eError = OmxrSaveThreadContext(u32ThreadID, pvParam);
					if (OMX_ErrorNone != eError) {
						OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
					}
				}
			}
		} else if (u32ProcID == 0xFFFFFFFEUL) {
			eError = OMX_ErrorUndefined;
		} else if (u32ProcID == 0xFFFFFFFDUL) {
		} else if (u32ProcID == 0xFFFFFFFCUL) {
			eError = OMX_ErrorUndefined;
		} else {
			eError = OMX_ErrorUndefined;
		}
	} else {
		/* Check parameter. */
		if ((NULL == pvContext) /*|| (NULL == pvParam)*/) {
			OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
			return OMX_ErrorBadParameter;
		}
		psUvcsUdf = (OMXR_UVCS_UDF_STRUCT *)pvContext;

		if(psUvcsUdf->pvHwProcReadData == NULL){
			return OMX_ErrorUndefined;
		}

		if(psUvcsUdf->bTerminateRequest != OMX_TRUE){
			eError = OmxrUvcsUdfOsal_Read(psUvcsUdf->pvUvcsUdfOsalHandle, psUvcsUdf->pvHwProcReadData, psUvcsUdf->u32StructSize);
		}
		if(psUvcsUdf->bTerminateRequest != OMX_TRUE){
			psUvcsUdf->fCallback((OMXR_UVCS_UDF_HANDLE)psUvcsUdf, psUvcsUdf->pvUserContext, psUvcsUdf->pvHwProcReadData, eError);
		}
	}

	return eError;
}

