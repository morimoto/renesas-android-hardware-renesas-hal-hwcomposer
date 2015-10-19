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
 * OMX Converter public file 
 *
 * \file cnv_public.h
 * \attention
 */
#ifndef CNV_PUBLIC_H
#define CNV_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
/**
* The CNV_Initialize will create the converter handle.
*
* \param[out] pHandle         the converter handle
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE  CNV_Initialize(
  CNV_HANDLE		*pHandle,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_Deinitialize will destroy the converter handle.
*
* \param[in] hProcessing      the converter handle
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE  CNV_Deinitialize(
  CNV_HANDLE	 	hProcessing,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_GetExtensionIndex will get a ExtensionIndex.
*
* \param[in] hProcessing      the converter handle
* \param[in] cParameterName   the name of extension index
* \param[out] pIndexType      extension index
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_GetExtensionIndex(
  CNV_HANDLE	 	hProcessing,
  CNV_STRING 		cParameterName,
  CNV_INDEXTYPE 	*pIndexType,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_QueryFormat will query limit of plugin.
*
* \param[in] hProcessing      the converter handle
* \param[in] nImageFormat     image format
* \param[out] pResponse       response
* \param[out] pConstraints    constraint
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_QueryFormat(
  CNV_HANDLE 		hProcessing,
  CNV_U32 			nImageFormat,
  CNV_U32 			*pResponse,
  CNV_CONSTRAINTS_T *pConstraints,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_GetParameter will get a pointer.
*
* \param[in] hProcessing                       the converter handle
* \param[in] nIndexType                        extension index
* \param[out] pProcessingParameterStructrure   get data
* \param[out] pSubErrorCode                    sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE                       success
* \retval others                               failure
*/
CNV_ERRORTYPE CNV_GetParameter(
  CNV_HANDLE		hProcessing,
  CNV_INDEXTYPE 	nIndexType,
  CNV_PTR 			pProcessingParameterStructrure,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_SetParameter will set a pointer.
*
* \param[in] hProcessing                      the converter handle
* \param[in] nIndexType                       extension index
* \param[in] pProcessingParameterStructrure   set data
* \param[out] pSubErrorCode                   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE                      success
* \retval others                              failure
*/
CNV_ERRORTYPE CNV_SetParameter (
  CNV_HANDLE		hProcessing,
  CNV_INDEXTYPE 	nIndexType,
  CNV_PTR 			pProcessingParameterStructrure,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_Open will start the converter.
*
* \param[in] hProcessing               the converter handle
* \param[in] pUserPointer              user pointer
* \param[out] pImageConversionConfig   configuration
* \param[out] pCallbackFuncs           callback function
* \param[out] pSubErrorCode            sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE               success
* \retval others                       failure
*/
CNV_ERRORTYPE CNV_Open (
  CNV_HANDLE					hProcessing,
  CNV_PTR 						pUserPointer,
  CNV_SETTING_IMAGECONVERSION_T *pImageConversionConfig,
  CNV_CALLBACK_FUNCS_T 		  	*pCallbackFuncs,
  CNV_SUBERRORTYPE 			  	*pSubErrorCode
);

/**
* The CNV_EmptyThisBuffer will input a buffer(Input).
*
* \param[in] hProcessing      the converter handle
* \param[in] pBuffer          input buffer
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_EmptyThisBuffer (
  CNV_HANDLE	 		hProcessing,
  CNV_BUFFERHEADER_T 	*pBuffer,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The CNV_FillThisBuffer will input a buffer(Output).
*
* \param[in] hProcessing      the converter handle
* \param[in] pBuffer          output buffer
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_FillThisBuffer(
  CNV_HANDLE			hProcessing,
  CNV_BUFFERHEADER_T 	*pBuffer,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The CNV_CommandFlush will request Flush process.
*
* \param[in] hProcessing      the converter handle
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_CommandFlush (
  CNV_HANDLE			hProcessing,
  CNV_SUBERRORTYPE 		*pSubErrorCode
);

/**
* The CNV_Close will stop the converter.
*
* \param[in] hProcessing      the converter handle
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_Close (
  CNV_HANDLE		hProcessing,
  CNV_SUBERRORTYPE 	*pSubErrorCode
);

/**
* The CNV_ErrorCodeToString will convert to error name.
*
* \breaf OMX Converter interface function
* \param[in] nErrorCode       error code
* \param[out] pErrorString    error name
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CNV_ErrorCodeToString (
  CNV_S32 		nErrorCode,
  CNV_STRING 	*pErrorString
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CNV_PUBLIC_h */
