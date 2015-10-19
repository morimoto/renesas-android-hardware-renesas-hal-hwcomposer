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
 * OMX Converter fdp plugin file 
 *
 * \file cnvp_fdp_interface.h
 * \attention
 */
#ifndef CNVP_FDP_INTERFACE_H
#define CNVP_FDP_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

/**
* The CNVP_GetPlugInFunc will get fdp plugin interface functions.
*
* \param[out] psPlugInFunc     fdp plugin interface functions
* \param[out] pSubErrorCode    sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNVP_GetPlugInFunc(
	CNVP_PLUGIN_IF_FUNCS_T	*psPlugInFunc,
	CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_CreateHandle will create the fdp plugin interface handle.
*
* \param[out] hPlugInHandle    fdp plugin interface functions
* \param[in]  nConvertMode     converter mode
* \param[in]  pUserPointer     user Pointer
* \param[out] pSubErrorCode    sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_CreateHandle(
	CNV_HANDLE			*hPlugInHandle,
	CNV_U32				nConvertMode,
	CNV_PTR				pUserPointer,
	CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The FDPCP_CreateHandle will destroy the fdp plugin interface handle.
*
* \param[in] hPlugInHandle    fdp plugin interface functions
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_DeleteHandle(
	CNV_HANDLE		hPlugInHandle
);

/**
* The FDPCP_Open will start the converter thread.
*
* \param[in] hPlugInHandle            fdp plugin interface functions
* \param[in] pParamList               user setting parameter
* \param[in] pImageConversionConfig   configuration
* \param[in] psConverterCallbackFunc  converter Callback functions
* \param[in] psEventFunc              local Callback functions
* \param[out] pSubErrorCode           sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_Open(
	CNV_HANDLE			 	 		hPlugInHandle,
	CNV_PTR							pParamList,
	CNV_SETTING_IMAGECONVERSION_T 	*pImageConversionConfig,
	CNV_CALLBACK_FUNCS_T 			*psConverterCallbackFunc,
	CNVP_EVENT_FUNCS_T	 	 		*psEventFunc,
	CNV_SUBERRORTYPE 	 	 		*pSubErrorCode
);

/**
* The FDPCP_Open will stop the converter thread.
*
* \param[in] hPlugInHandle            fdp plugin interface functions
* \param[out] pSubErrorCode           sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_Close (
  CNV_HANDLE 			hPlugInHandle,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_EmptyThisBuffer will send the input buffer to the converter thread.
*
* \param[in] hPlugInHandle            fdp plugin interface functions
* \param[in] pBuffer                  input buffer( input )
* \param[out] pSubErrorCode           sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_EmptyThisBuffer (
  CNV_HANDLE 			hPlugInHandle,
  CNV_BUFFERHEADER_T 	*pBuffer,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_FillThisBuffer will send the input buffer to the converter thread.
*
* \param[in] hPlugInHandle            fdp plugin interface functions
* \param[in] pBuffer                  input buffer( output )
* \param[out] pSubErrorCode           sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_FillThisBuffer(
  CNV_HANDLE 			hPlugInHandle,
  CNV_BUFFERHEADER_T 	*pBuffer,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_CommandFlush will send a FlushCommand to the converter thread.
*
* \param[in]  hPlugInHandle           fdp plugin interface functions
* \param[out] pSubErrorCode           sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_CommandFlush (
  CNV_HANDLE 			hPlugInHandle,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_GetExtensionIndex will get a ExtensionIndex of converter .
*
* \param[in] cParameterName      the name of extension index
* \param[out] pIndexType         extension index
* \param[out] pDefaultParam      default parameter
* \param[out] pSubErrorCode      sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_GetExtensionIndex(
  CNV_STRING 			cParameterName,
  CNV_INDEXTYPE 		*pIndexType,
  CNV_PTR				*pDefaultParam,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_GetExtensionIndex will get a format information of fdp plugin .
*
* \param[in]  nImageFormat         image format 
* \param[out] pResponse            response
* \param[out] pConstraints         constraints
* \param[out] pSubErrorCode        sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_QueryFormat(
  CNV_U32 				nImageFormat,
  CNV_U32 				*pResponse,
  CNV_CONSTRAINTS_T 	*pConstraints,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_GetParameter will get a setting parameter of fdp plugin .
*
* \param[in] nIndexType                       extension index
* \param[out] pProcessingParameterStructrure  extension parameter
* \param[in] pParameter                       parameter
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_GetParameter(
  CNV_INDEXTYPE 		nIndexType,
  CNV_PTR 				pProcessingParameterStructrure,
  CNV_PTR				pParameter,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_CheckParameter will check a setting parameter of fdp plugin .
*
* \param[in] nIndexType                       extension index
* \param[out] pProcessingParameterStructrure  extension parameter
* \param[in] pParameter                       parameter
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_CheckParameter(
  CNV_INDEXTYPE 		nIndexType,
  CNV_PTR 				pProcessingParameterStructrure,
  CNV_PTR				pParameter,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The FDPCP_PlugInSelect will decide a fdp plugin.
*
* \param[in] pParam                 the pointer of setting data
* \param[in] pExtensionData         extension data
* \param[in] nMaxExtensionIndexNum  the number of Extension index
* \param[in] nPluginNum             the number of plugin
* \param[out] bResult               CNV_FALSE: No select, CNV_TRUE: Select
* \param[out] nMode                 deinterlace mode
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE FDPCP_PlugInSelect(
	CNV_SETTING_IMAGECONVERSION_T	*pParam,
	CNV_EXTENSIONINDEX_T			*pExtensionData,
	CNV_U32							nMaxExtensionIndexNum,
	CNV_U32							nPluginNum,
	CNV_BOOL						*bResult,
	CNV_U32							*nMode,
	CNV_SUBERRORTYPE 				*pSubErrorCode
);

#ifdef __cplusplus
}
#endif

#endif
