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
 * OMX Converter common file of interface and plugin
 *
 * \file cnv_plugin_cmn.h
 * \attention
 */
 
#ifndef CNV_PLUGIN_CMN_H
#define CNV_PLUGIN_CMN_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "cnv_type.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
* \struct CNVP_CALLBACK_FUNCS_T
*/
typedef struct {
	CNV_HANDLE	hCnvHandle;
	CNV_VOID (*PlugIn_CallBackEvent)( /*< Fatal or FlushDone Callback */
		CNV_HANDLE				hCnvHandle,
		CNV_CALLBACK_EVENTTYPE	eEventId
	);
} CNVP_EVENT_FUNCS_T;

/**
* \struct CNV_EXTENSIONINDEX_T
*/
typedef struct {
	CNV_STRING		cParameterName;
	CNV_INDEXTYPE	nIndexType;
	CNV_PTR			pParameter;
	CNV_PTR			pNextParam;
} CNV_EXTENSIONINDEX_T;

/**
* \struct CNVP_PLUGIN_IF_FUNCTION_T
*/
typedef struct {
	CNV_ERRORTYPE (*PlugIn_CreateHandle)(
		CNV_HANDLE			*hPlugInHandle,
		CNV_U32				nConvertMode,
		CNV_PTR				pUserPointer,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_DeleteHandle)(
		CNV_HANDLE			hPlugInHandle
	);
	CNV_ERRORTYPE (*PlugIn_Open)(
		CNV_HANDLE			 	 		hPlugInHandle,
		CNV_PTR							pParamList,
		CNV_SETTING_IMAGECONVERSION_T 	*pImageConversionConfig,
		CNV_CALLBACK_FUNCS_T 			*psConverterCallbackFunc,
		CNVP_EVENT_FUNCS_T	 	 		*psEventFunc,
		CNV_SUBERRORTYPE 	 	 		*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_Close) (
		CNV_HANDLE 			hPlugInHandle,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_EmptyThisBuffer)(
		CNV_HANDLE 			hPlugInHandle,
		CNV_BUFFERHEADER_T 	*pBuffer,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_FillThisBuffer)(
		CNV_HANDLE 			hPlugInHandle,
		CNV_BUFFERHEADER_T 	*pBuffer,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_CommandFlush)(
		CNV_HANDLE 			hPlugInHandle,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_GetExtensionIndex)(
		CNV_STRING 			cParameterName,
		CNV_INDEXTYPE 		*pIndexType,
		CNV_PTR				*pDefaultParam,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_QueryFormat)(
		CNV_U32 			nImageFormat,
		CNV_U32 			*pResponse,
		CNV_CONSTRAINTS_T 	*pConstraints,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_GetParameter)(
		CNV_INDEXTYPE 		nIndexType,
		CNV_PTR 			pProcessingParameterStructrure,
		CNV_PTR				pParameter,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_CheckParameter)(
		CNV_INDEXTYPE 		nIndexType,
		CNV_PTR 			pProcessingParameterStructrure,
		CNV_PTR				pParameter,
		CNV_SUBERRORTYPE 	*pSubErrorCode
	);
	CNV_ERRORTYPE (*PlugIn_PlugInSelect)(
		CNV_SETTING_IMAGECONVERSION_T	*pParam,
		CNV_EXTENSIONINDEX_T			*pExtensionData,
		CNV_U32							nMaxExtensionIndexNum,
		CNV_U32							nPluginNum,
		CNV_BOOL						*bResult,
		CNV_U32							*nMode,
		CNV_SUBERRORTYPE 				*pSubErrorCode
	);
}CNVP_PLUGIN_IF_FUNCS_T;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
