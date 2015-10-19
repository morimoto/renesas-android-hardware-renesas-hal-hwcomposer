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
 * OMX Converter Interface function
 *
 * \file cnvp_fdp_interface.c
 * \attention 
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include "cnv_type.h"
#include "cnv_plugin_cmn.h"

#include "cnvp_cmn.h"
#include "cnvp_fdp_interface.h"
#include "cnvp_fdp_core.h"

#include "cnv_osdep.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
/* FDP PLUGIN dependence info */
#define FDPCP_PLUGIN_ID 					(0x00000004) 

/* Query format */	
#define FDPCP_IMAGE_CROP_POSX_BOUNDARY	 	(0)
#define FDPCP_IMAGE_CROP_POSY_BOUNDARY	 	(0)
#define FDPCP_IMAGE_CROP_WIDTH_BOUNDARY	 	(0)
#define FDPCP_IMAGE_CROP_HEIGHT_BOUNDARY 	(0)
#define FDPCP_BUFFER_WIDTH_MULTIPLES	 	(2)
#define FDPCP_BUFFER_HEIGHT_MULTIPLES	 	(2)
#define FDPCP_BUFFER_TOP_ADDRESS_BOUNDARY 	(32)
#define FDPCP_MAX_BUFFER_NUM				(64)

#if defined(CNV_STATIC_LINK)
#define GET_PLUGIN_FUNCTION      Fdpcp_CNVP_GetPlugInFunc
#else
#define GET_PLUGIN_FUNCTION      CNVP_GetPlugInFunc
#endif

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/* Extension Type*/
typedef enum {
	OMXR_MC_EXTENSION_NONE					= (CNV_S32)(CNVP_EXTENSION_INDEX_BASE + 0x0000C000L),
	OMXR_MC_IndexParamVideoConverterType,
	OMXR_MC_IndexParamVideoDeinterlaceMode,
	OMXR_MC_IndexParamVideoSubModuleTimeout
}FDPCP_EXTETIONTYPE;

typedef struct{
	CNV_STRING  		cIndexName;
	FDPCP_EXTETIONTYPE	eExtensionIndex;
}FDPCP_EXTENSIONINDEX_T;

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/**
* The FDPCP_GetExtensionParameter will get a setting parameter of fdp plugin .
*
* \param[in]  pParamList     list of parameter
* \param[in]  cIndexName     name of extension index
* \param[OUT] pParam         parameter
* \return CNV_BOOL
* \retval CNV_TRUE           success
* \retval CNV_FALSE          failure
*/
static CNV_BOOL FDPCP_GetExtensionParameter(CNV_PTR pParamList, CNV_STRING cIndexName,CNV_PTR *pParam);
/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/
static FDPCP_EXTENSIONINDEX_T sExtensionIndex[] = {
	{"OMX.RENESAS.INDEX.PARAM.VIDEO.CONVERTER.CONVERTERTYPE", OMXR_MC_IndexParamVideoConverterType},
	{"OMX.RENESAS.INDEX.PARAM.VIDEO.CONVERTER.DEINITERLACEMODE", OMXR_MC_IndexParamVideoDeinterlaceMode},
	{"OMX.RENESAS.INDEX.PARAM.VIDEO.CONVERTER.TIMEOUT", OMXR_MC_IndexParamVideoSubModuleTimeout},
};

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
CNV_ERRORTYPE GET_PLUGIN_FUNCTION(
	CNVP_PLUGIN_IF_FUNCS_T	*psPlugInFunc,
	CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	/* Set SubError code */
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	if(NULL == psPlugInFunc){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM );
		return CNV_ERROR_INVALID_PARAMETER;
	}

	/* Set Function */
	psPlugInFunc->PlugIn_Open				= &FDPCP_Open;
	psPlugInFunc->PlugIn_Close				= &FDPCP_Close;
	psPlugInFunc->PlugIn_CreateHandle		= &FDPCP_CreateHandle;
	psPlugInFunc->PlugIn_DeleteHandle		= &FDPCP_DeleteHandle;
	psPlugInFunc->PlugIn_CheckParameter		= &FDPCP_CheckParameter;
	psPlugInFunc->PlugIn_GetParameter		= &FDPCP_GetParameter;
	psPlugInFunc->PlugIn_GetExtensionIndex	= &FDPCP_GetExtensionIndex;
	psPlugInFunc->PlugIn_QueryFormat		= &FDPCP_QueryFormat;
	psPlugInFunc->PlugIn_EmptyThisBuffer	= &FDPCP_EmptyThisBuffer;
	psPlugInFunc->PlugIn_FillThisBuffer		= &FDPCP_FillThisBuffer;
	psPlugInFunc->PlugIn_CommandFlush		= &FDPCP_CommandFlush;
	psPlugInFunc->PlugIn_PlugInSelect		= &FDPCP_PlugInSelect;
	
	return CNV_ERROR_NONE;
}


CNV_ERRORTYPE FDPCP_CreateHandle(
	CNV_HANDLE			*hPlugInHandle,
	CNV_U32				nConvertMode,
	CNV_PTR				pUserPointer,
	CNV_SUBERRORTYPE 	*pSubErrorCode)
{
	CNVP_CONTEXT_T 			*pCnvpContext;
	
	CNV_ERRORTYPE eResult = CNV_ERROR_INVALID_PARAMETER;
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	/* Parameter check */
	if( NULL == hPlugInHandle ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_HANDLE );
		return CNV_ERROR_INVALID_PARAMETER;
	}

	/* Create Plugin Context */
	pCnvpContext = (CNVP_CONTEXT_T *)Cnvdep_Malloc(sizeof(CNVP_CONTEXT_T));
	if( NULL == pCnvpContext){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM_CREATE_HANDLE );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	(void)Cnvdep_Memset(pCnvpContext,0,sizeof(CNVP_CONTEXT_T));
	
	/* Set UserPointer */
	pCnvpContext->pUserPointer =  pUserPointer;
	
	/* Create CnvpCmnHandle */	
	eResult = CnvpCmn_CreateHandle(&pCnvpContext->pCnvpCmnHandle);
	if( CNV_ERROR_NONE != eResult ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM_CREATE_HANDLE );
		return eResult;
	}
	
	/* Set Function Parameter */
	eResult = CnvpCore_GetPluginCoreFunction(&pCnvpContext->sPluginFunc);
	if ( CNV_ERROR_NONE != eResult ){
		Cnvdep_Free((CNV_PTR)pCnvpContext);
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	/* Set Mode Parameter */
	pCnvpContext->nConvertMode = nConvertMode;
	if( (pCnvpContext->nConvertMode != CNV_DEINT_NONE) && (pCnvpContext->nConvertMode != CNV_DEINT_2D_HALFRATE) && (pCnvpContext->nConvertMode != CNV_DEINT_2D_FULLRATE) &&
		(pCnvpContext->nConvertMode != CNV_DEINT_3D_HALFRATE) && (pCnvpContext->nConvertMode != CNV_DEINT_3D_FULLRATE) )
	{
		CNV_LOGGER(CNV_LOG_DEBUG,",FDP Select : CONVERT AUTO MODE",CNVP_DEFAULT_CONVERT_MODE);
		pCnvpContext->nConvertMode = CNVP_DEFAULT_CONVERT_MODE;
	}
	
	/* Create handle plugin */
	eResult = pCnvpContext->sPluginFunc.CNVP_CreateHandle(&pCnvpContext->pCnvpDepHandle, nConvertMode, pSubErrorCode);
	
	CNV_LOGGER(CNV_LOG_DEBUG,"pCnvpCmnHandle = %p",(CNV_U32)pCnvpContext->pCnvpCmnHandle);
	CNV_LOGGER(CNV_LOG_DEBUG,"pCnvpDepHandle = %p",(CNV_U32)pCnvpContext->pCnvpDepHandle);
	
	if( CNV_ERROR_NONE == eResult){ 
		*hPlugInHandle = (CNV_PTR)pCnvpContext;
	}
	return eResult;		
}


CNV_ERRORTYPE FDPCP_DeleteHandle(
	CNV_HANDLE		hPlugInHandle
){
	CNVP_CONTEXT_T 			*pCnvpContext;
	
	/* Parameter check */
	if( NULL == hPlugInHandle ){
		return CNV_ERROR_INVALID_PARAMETER;
	}	
	
	pCnvpContext = (CNVP_CONTEXT_T *)hPlugInHandle;
	
	/* CmnHandle Free */
	Cnvdep_Free((CNV_PTR)pCnvpContext->pCnvpCmnHandle);
	
	/* DepHandle Free */
	pCnvpContext->sPluginFunc.CNVP_DeleteHandle((CNV_PTR)pCnvpContext->pCnvpDepHandle);
	
	/* pCnvpContext Free */
	Cnvdep_Free((CNV_PTR)pCnvpContext);
	
	return CNV_ERROR_NONE;
}


CNV_ERRORTYPE FDPCP_Open(
	CNV_HANDLE			 	 		hPlugInHandle,
	CNV_PTR							pParamList,
	CNV_SETTING_IMAGECONVERSION_T 	*pImageConversionConfig,
	CNV_CALLBACK_FUNCS_T 		 	*psConverterCallbackFunc,
	CNVP_EVENT_FUNCS_T	 	 		*psEventFunc,
	CNV_SUBERRORTYPE 	 	 		*pSubErrorCode)
{
	CNVP_CONTEXT_T 			*pCnvpContext;
	
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	/* Parameter check */
	if( NULL == hPlugInHandle ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_HANDLE );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	if( NULL == pImageConversionConfig ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_NINPUTIMAGE_COLORFORMAT );
		return CNV_ERROR_INVALID_PARAMETER;
	}	
	
	if( NULL == psConverterCallbackFunc ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_CALLBACKFUNCS );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	if( NULL == psEventFunc ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERRPR_SUBMODULE_PARAM_CALLBACKFUNCS );
		return CNV_ERROR_INVALID_PARAMETER;
	}

	pCnvpContext = (CNVP_CONTEXT_T*)hPlugInHandle;
	
	pCnvpContext->sCallbackFunc.CNV_EmptyBufferDone = psConverterCallbackFunc->CNV_EmptyBufferDone;
	pCnvpContext->sCallbackFunc.CNV_FillBufferDone  = psConverterCallbackFunc->CNV_FillBufferDone;
	pCnvpContext->sCallbackFunc.CNV_EventDone	    = psConverterCallbackFunc->CNV_EventDone;
	
	/* Set local Event Parameter */
	pCnvpContext->sPluginlocalEvent.hCnvHandle = psEventFunc->hCnvHandle;
	pCnvpContext->sPluginlocalEvent.PlugIn_CallBackEvent = psEventFunc->PlugIn_CallBackEvent;	
	
	/* plugin param */
	pCnvpContext->sPluginRule.bBottomFirst			= pImageConversionConfig->bBottomFirst;
	pCnvpContext->sPluginRule.bInterlacedSequence 	= pImageConversionConfig->bInterlacedSequence; 			
	pCnvpContext->sPluginRule.nInputFormat 			= pImageConversionConfig->nInputImage_ColorFormat;
	pCnvpContext->sPluginRule.nOutputFormat 		= pImageConversionConfig->nOutputImage_ColorFormat;
	pCnvpContext->sPluginRule.bCropping				= pImageConversionConfig->bCropping;
	pCnvpContext->sPluginRule.bScaling				= pImageConversionConfig->bScaling;
	pCnvpContext->sPluginRule.bTileMode				= pImageConversionConfig->bTileMode;
	
	/* S3ctrl param */
	if( CNV_TRUE == pImageConversionConfig->bTileMode ){
		pCnvpContext->sS3Param.nPhyAddr					= pImageConversionConfig->sTileParam.nPhyAddr;
		pCnvpContext->sS3Param.nStride					= pImageConversionConfig->sTileParam.nStride;
		pCnvpContext->sS3Param.nArea					= pImageConversionConfig->sTileParam.nArea;
		pCnvpContext->sS3Param.pReserve					= pImageConversionConfig->sTileParam.pReserve;
	}
	
	/* Extension Information */
	pCnvpContext->sExtensionInfo.pParameterList             = pParamList;
	pCnvpContext->sExtensionInfo.CNVP_GetExtensionParameter = &FDPCP_GetExtensionParameter;
	
	eResult = CnvpCmn_CreateCnvertThread( (CNV_PTR)pCnvpContext , pSubErrorCode);
	
	return eResult;	
}


CNV_ERRORTYPE FDPCP_Close (
  CNV_HANDLE 			hPlugInHandle,
  CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	CNV_ERRORTYPE	eResult = CNV_ERROR_INVALID_PARAMETER;
    
	if ( NULL == hPlugInHandle ) {
		CnvpCmn_SetSubErrorCode(pSubErrorCode,CNV_ERROR_PARAM_HANDLE);
        return CNV_ERROR_INVALID_PARAMETER;
    }
	
	/* Send Event */
	eResult = CnvpCmn_SendEvent( (CNV_PTR)hPlugInHandle, CNV_EVENT_CLOSE, NULL, pSubErrorCode);	
	if( CNV_ERROR_NONE != eResult){
		return CNV_ERROR_INVALID_PARAMETER;	
	}
	
	eResult = CnvpCmn_ThreadJoin( (CNV_PTR)hPlugInHandle ,  pSubErrorCode);
	
	/* plugin close */
	
	return eResult;	
}


CNV_ERRORTYPE FDPCP_EmptyThisBuffer (
  CNV_HANDLE 			hPlugInHandle,
  CNV_BUFFERHEADER_T 	*pBuffer,
  CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	CNV_ERRORTYPE		eResult = CNV_ERROR_NONE;
	CNVP_CONTEXT_T 		*pCnvpContext;

    if ( NULL == hPlugInHandle ) {
		CnvpCmn_SetSubErrorCode(pSubErrorCode,CNV_ERROR_PARAM_HANDLE);
        return CNV_ERROR_INVALID_PARAMETER;
    }
	
    if ( NULL == pBuffer ) {
		CnvpCmn_SetSubErrorCode(pSubErrorCode,CNV_ERROR_PARAM_NINPUTIMAGE_COLORFORMAT);
        return CNV_ERROR_INVALID_PARAMETER;
    }	
		
	pCnvpContext = (CNVP_CONTEXT_T*)hPlugInHandle;

	/* Send Message (Buffer Data) */
	eResult = CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_EMPTYBUFFER, (CNV_PTR)pBuffer, pSubErrorCode);
	if ( CNV_ERROR_NONE == eResult ){
		pCnvpContext->nCountofInput++;
	}

	return eResult;
}


CNV_ERRORTYPE FDPCP_FillThisBuffer(
  CNV_HANDLE 			hPlugInHandle,
  CNV_BUFFERHEADER_T 	*pBuffer,
  CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	CNV_ERRORTYPE		eResult = CNV_ERROR_NONE;
	CNVP_CONTEXT_T 		*pCnvpContext;
	
    if ( NULL == hPlugInHandle ) {
		CnvpCmn_SetSubErrorCode(pSubErrorCode,CNV_ERROR_PARAM_HANDLE);
        return CNV_ERROR_INVALID_PARAMETER;
    }
	
    if ( NULL == pBuffer ) {
		CnvpCmn_SetSubErrorCode(pSubErrorCode,CNV_ERROR_PARAM_BUFFER);
        return CNV_ERROR_INVALID_PARAMETER;
    }	
		
	pCnvpContext = (CNVP_CONTEXT_T*)hPlugInHandle;
	
	/* Send Message (Buffer Data) */
	eResult = CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_FILLBUFFER, (CNV_PTR)pBuffer, pSubErrorCode);
	if ( CNV_ERROR_NONE == eResult ){
		pCnvpContext->nCountofOutput++;
	}

	return eResult;
}


CNV_ERRORTYPE FDPCP_CommandFlush (
  CNV_HANDLE 			hPlugInHandle,
  CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	CNV_ERRORTYPE	eResult = CNV_ERROR_INVALID_PARAMETER;

    if ( NULL == hPlugInHandle ) {
		CnvpCmn_SetSubErrorCode(pSubErrorCode,CNV_ERROR_PARAM_HANDLE);
        return CNV_ERROR_INVALID_PARAMETER;
    }

	/* Send Message (Buffer Data) */
	eResult = CnvpCmn_SendEvent( (CNV_PTR)hPlugInHandle, CNV_EVENT_FLUSHING, NULL, pSubErrorCode);
	
	return eResult;
}


CNV_ERRORTYPE FDPCP_GetExtensionIndex(
  CNV_STRING 			cParameterName,
  CNV_INDEXTYPE 		*pIndexType,
  CNV_PTR				*pDefaultParam,
  CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	CNV_U32 uCnt         = 0;
	CNV_U32 nMaxIndexNum = 0;
	CNV_ERRORTYPE eResult = CNV_ERROR_INVALID_PARAMETER;
	
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	/* Parameter check */
	if( NULL == cParameterName ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_PARAMETERNAME );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	nMaxIndexNum = sizeof(sExtensionIndex) / sizeof(FDPCP_EXTENSIONINDEX_T);
	/* Set ExtensionIndex */
	for( uCnt = 0; uCnt < nMaxIndexNum; uCnt++)
	{
		/* Set ExtensionIndex */
		if( strcmp( sExtensionIndex[uCnt].cIndexName , cParameterName ) == 0){
			*pIndexType = sExtensionIndex[uCnt].eExtensionIndex;

			eResult = CNV_ERROR_NONE;
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
			CNV_LOGGER_STRING(CNV_LOG_DEBUG,"ExtensionName = %s",cParameterName);
			break;
		} else {
			eResult = CNV_ERROR_INVALID_PARAMETER;
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_PARAMETERNAME );
		}
	}

	return eResult;
}


CNV_ERRORTYPE FDPCP_QueryFormat(
  CNV_U32 				nImageFormat,
  CNV_U32 				*pResponse,
  CNV_CONSTRAINTS_T 	*pConstraints,
  CNV_SUBERRORTYPE 		*pSubErrorCode)
{
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	/* Parameter check */
	if( NULL == pResponse ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_RESPONSE );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	/* Parameter check */
	if( NULL == pConstraints ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_CONSTRAINTS );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	/* Check support format */

	switch( nImageFormat )
	{
	case( CNVP_COLOR_FormatYUV420Planar ):
		*pResponse = CNV_QUERY_IMAGE_SUPPOERTED_INPUT_OUTPUT;
		break;
	case( CNVP_COLOR_FormatYUV420PackedPlanar ):
		*pResponse = CNV_QUERY_IMAGE_SUPPOERTED_INPUT_OUTPUT;		
		break;
	case( CNVP_COLOR_FormatYUV420SemiPlanar ):
		*pResponse = CNV_QUERY_IMAGE_SUPPOERTED_INPUT_OUTPUT;		
		break;
	default:
		*pResponse = CNV_QUERY_IMAGE_NOT_SUPPORTED;
		break;
	}
	
	/* Set Query Format */
	pConstraints->nImageCropPosXBoundary			= FDPCP_IMAGE_CROP_POSX_BOUNDARY;
	pConstraints->nImageCropPosYBoundary			= FDPCP_IMAGE_CROP_POSY_BOUNDARY;
	pConstraints->nImageCropWidthBoundary			= FDPCP_IMAGE_CROP_WIDTH_BOUNDARY;
	pConstraints->nImageCropHeightBoundary			= FDPCP_IMAGE_CROP_HEIGHT_BOUNDARY;
	pConstraints->nBufferWidthMultiplesOf			= FDPCP_BUFFER_WIDTH_MULTIPLES;
	pConstraints->nBufferHeightMultiplesOf			= FDPCP_BUFFER_HEIGHT_MULTIPLES;
	pConstraints->nBufferTopAddressBoundary			= FDPCP_BUFFER_TOP_ADDRESS_BOUNDARY;
	pConstraints->nMaxNumberOfBuffersPerProcessing 	= FDPCP_MAX_BUFFER_NUM;
	
	return CNV_ERROR_NONE;	
}


CNV_ERRORTYPE FDPCP_GetParameter(
	CNV_INDEXTYPE 		nIndexType,
	CNV_PTR 			pProcessingParameterStructrure,
	CNV_PTR				pParameter,
	CNV_SUBERRORTYPE 	*pSubErrorCode)
{
	CNV_U32 				  		uCnt = 0;
	CNV_U32							nMaxIndexNum = 0;
	CNV_ERRORTYPE 					eResult = CNV_ERROR_INVALID_PARAMETER;
	OMXR_MC_VIDEO_CONVERTER_T 		*psIdParam;
	OMXR_MC_VIDEO_DEINTERLACEMODE_T *psModeParam;
	OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T *psTimeoutParam;
	
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	/* check parameter */
	if( NULL == pProcessingParameterStructrure ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_PROCESSINGPARAMETERSTRUCTURE );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	nMaxIndexNum = sizeof(sExtensionIndex) / sizeof(FDPCP_EXTENSIONINDEX_T);
	for( uCnt = 0; uCnt < nMaxIndexNum; uCnt++)
	{
		if( (CNV_U32)sExtensionIndex[uCnt].eExtensionIndex == nIndexType ){
			switch( nIndexType )
			{
			case( OMXR_MC_IndexParamVideoConverterType ):
				psIdParam = (OMXR_MC_VIDEO_CONVERTER_T *)pProcessingParameterStructrure;
				psIdParam->nPluginID =  (CNV_U32)CNV_PLUGIN_ID_FDP;
				CNV_LOGGER(CNV_LOG_DEBUG,"OMXR_MC_IndexParamVideoConverterType ",0);
				break;
			case( OMXR_MC_IndexParamVideoDeinterlaceMode ):
				psModeParam 				 = (OMXR_MC_VIDEO_DEINTERLACEMODE_T *)pProcessingParameterStructrure;
				psModeParam->nDeinterlace	 =  (CNV_U32)CNV_DEINT_3D_HALFRATE;
				CNV_LOGGER(CNV_LOG_DEBUG,"OMXR_MC_IndexParamVideoDeinterlaceMode = %d", (CNV_U32)pParameter);
				break;
			case( OMXR_MC_IndexParamVideoSubModuleTimeout ):
				psTimeoutParam 				 = (OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T *)pProcessingParameterStructrure;
				psTimeoutParam->nWaitTime	     =  (CNV_U32)CNVP_THREAD_WAIT_TIME;
				psTimeoutParam->nTimeout	     =  (CNV_U32)CNVP_THREAD_TIMEOUT;
				CNV_LOGGER(CNV_LOG_DEBUG,"OMXR_MC_IndexParamVideoSubModuleTimeout = %d", (CNV_U32)pParameter);
				break;
			default:
				break;
			}
			eResult = CNV_ERROR_NONE;
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
			break;
		}
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM );
	}
	return eResult;
}


CNV_ERRORTYPE FDPCP_CheckParameter(
	CNV_INDEXTYPE 		nIndexType,
	CNV_PTR 			pProcessingParameterStructrure,
	CNV_PTR				pParameter,
	CNV_SUBERRORTYPE	*pSubErrorCode)
{
	CNV_U32 				  		uCnt = 0;
	CNV_U32							nMaxIndexNum = 0;
	CNV_ERRORTYPE 					eResult = CNV_ERROR_INVALID_PARAMETER;
	
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
	
	/* check parameter */
	if( NULL == pProcessingParameterStructrure ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_PROCESSINGPARAMETERSTRUCTURE );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	nMaxIndexNum = sizeof(sExtensionIndex) / sizeof(FDPCP_EXTENSIONINDEX_T);
	for( uCnt = 0; uCnt < nMaxIndexNum; uCnt++)
	{
		if( (CNV_U32)sExtensionIndex[uCnt].eExtensionIndex == nIndexType ){
			switch( nIndexType )
			{
			case( OMXR_MC_IndexParamVideoConverterType ):
				eResult = CNV_ERROR_NONE;
				CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
				break;
			case( OMXR_MC_IndexParamVideoDeinterlaceMode ):
				eResult = CNV_ERROR_NONE;
				CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
				break;
			case( OMXR_MC_IndexParamVideoSubModuleTimeout ):
				eResult = CNV_ERROR_NONE;
				CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );
				break;
			default:
				break;
			}
			break;
		}
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM );
	}
	return eResult;	
}


CNV_ERRORTYPE FDPCP_PlugInSelect(
	CNV_SETTING_IMAGECONVERSION_T	*pParam,
	CNV_EXTENSIONINDEX_T			*pExtensionData,
	CNV_U32							nMaxExtensionIndexNum,
	CNV_U32							nPluginNum,
	CNV_BOOL						*bResult,
	CNV_U32							*nMode,
	CNV_SUBERRORTYPE 				*pSubErrorCode)
{
	CNV_BOOL		bExtensionMode	= CNV_FALSE;
	CNV_BOOL		bPluginId		= CNV_FALSE;
	CNV_ERRORTYPE 	eResult 		= CNV_ERROR_INVALID_PARAMETER;
	CNV_PLUGIN_DICISION_RULE_T      sDicisionRule;
	OMXR_MC_VIDEO_CONVERTER_T 		*psIdParam;
	OMXR_MC_VIDEO_DEINTERLACEMODE_T *psModeParam;
	
	CNV_EXTENSIONINDEX_T *pHead;
	CNV_EXTENSIONINDEX_T *pNextHead;
	
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE );

	/* check parameter */
	if( NULL == pParam ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_NINPUTIMAGE_COLORFORMAT );	
		return CNV_ERROR_INVALID_PARAMETER;
	}

	if( NULL == bResult ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM );	
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	if( NULL == nMode ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM );	
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	pHead = (CNV_EXTENSIONINDEX_T*)pExtensionData;
	
	while(pHead != NULL){
		pNextHead = pHead->pNextParam;
		switch( pHead->nIndexType )
		{
		case( OMXR_MC_IndexParamVideoConverterType ):
			psIdParam  =  (OMXR_MC_VIDEO_CONVERTER_T *)pHead->pParameter;
			sDicisionRule.nPlugInId = psIdParam->nPluginID;
			if( sDicisionRule.nPlugInId == CNV_PLUGIN_ID_FDP ){
				*bResult = CNV_TRUE;
				eResult  = CNV_ERROR_NONE;
			}
			bPluginId = CNV_TRUE;
			CNV_LOGGER(CNV_LOG_DEBUG,"OMXR_MC_IndexParamVideoConverterType = %d",sDicisionRule.nPlugInId);
			break;
		case( OMXR_MC_IndexParamVideoDeinterlaceMode ):
			psModeParam = (OMXR_MC_VIDEO_DEINTERLACEMODE_T *)pHead->pParameter;
			sDicisionRule.nDeinterlace = psModeParam->nDeinterlace;
			CNV_LOGGER(CNV_LOG_DEBUG,"OMXR_MC_IndexParamVideoDeinterlaceMode = %d",psModeParam->nDeinterlace);
			bExtensionMode = CNV_TRUE;
			break;
		default:
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM_EXTENSIONINDEX );
			break;
		}
		pHead = pNextHead;
	}
	/* check parameter */
	/* plugIn dependence */
	if( CNV_FALSE == bPluginId ){
		sDicisionRule.bInterlacedSequence 		= pParam->bInterlacedSequence;
		sDicisionRule.bCropping					= pParam->bCropping;
		sDicisionRule.bScaling					= pParam->bScaling;
			
		sDicisionRule.nInputFormat  			= pParam->nInputImage_ColorFormat;
		sDicisionRule.nOutputFormat				= pParam->nOutputImage_ColorFormat;
		sDicisionRule.bBottomFirst  			= pParam->bBottomFirst;

		if( pParam->nInputImage_ColorFormat != pParam->nOutputImage_ColorFormat ){
			sDicisionRule.bImageFormatConversion	= CNV_TRUE;
		} else {
			sDicisionRule.bImageFormatConversion	= CNV_FALSE;
		}

		eResult = FdpcpCore_PluginSelected(&sDicisionRule,bResult,pSubErrorCode);
		if( CNV_ERROR_NONE != eResult ){
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_UNSELECT_PLUGIN );
		}
	} else {
		if( (CNV_FALSE == pParam->bInterlacedSequence) && (CNV_TRUE ==pParam->bBottomFirst)){
			*bResult = CNV_FALSE;
			eResult  = CNV_ERROR_INVALID_PARAMETER;
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_UNSELECT_PLUGIN );
		}
	}
	
	/* 1 plugin mode */
	if((1 == nPluginNum )){
		if( (CNV_TRUE == bPluginId ) && (CNV_ERROR_INVALID_PARAMETER != eResult)){
			*bResult = CNV_TRUE;
			eResult  = CNV_ERROR_NONE;
		}
		
		if(CNV_FALSE == bPluginId ){
			*bResult = CNV_TRUE;
			eResult  = CNV_ERROR_NONE;
		}
	}
	
	if((CNV_ERROR_NONE == eResult) && (CNV_TRUE != bExtensionMode)){
		sDicisionRule.nDeinterlace = CNVP_DEFAULT_CONVERT_MODE;
	}
	
	if((CNV_FALSE == sDicisionRule.bInterlacedSequence) && (CNV_TRUE == bExtensionMode)){
		if( CNV_DEINT_NONE !=  sDicisionRule.nDeinterlace){ 
			sDicisionRule.nDeinterlace = CNV_DEINT_NONE;
			CNV_LOGGER(CNV_LOG_DEBUG,"DeInterlase NONE",0);
		}
	}
	*nMode = sDicisionRule.nDeinterlace;
	CNV_LOGGER(CNV_LOG_DEBUG,"Result = %d", (CNV_U32)eResult);
	return eResult;
}


static CNV_BOOL FDPCP_GetExtensionParameter(CNV_PTR pParamList, CNV_STRING cIndexName,CNV_PTR *pParam)
{
	CNV_U32		   nExtensionIndex;
	CNV_BOOL       bGetFlag = CNV_FALSE;
	CNV_ERRORTYPE  eResult  = CNV_ERROR_NONE;
	CNV_EXTENSIONINDEX_T *pHead;
	CNV_EXTENSIONINDEX_T *pNextHead;
	
	pHead = (CNV_EXTENSIONINDEX_T*)pParamList;
	
	eResult = FDPCP_GetExtensionIndex(cIndexName,&nExtensionIndex,NULL,NULL);
	if( CNV_ERROR_NONE == eResult ){
		while(pHead != NULL){
			pNextHead = (CNV_EXTENSIONINDEX_T*)pHead->pNextParam;
			if(nExtensionIndex == pHead->nIndexType){
				*pParam = pHead->pParameter;
				bGetFlag = CNV_TRUE;
				break;
			}
			pHead = pNextHead;
		}
	}
	return bGetFlag;
}
