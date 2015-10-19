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
 *  OMX Converter plugin main function
 *
 * \file cnvp_fdp_core.c
 * \attention 
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "fdpm_api.h"
#include "s3ctl_user_public.h"

#include "cnv_type.h"
#include "cnvp_cmn.h"
#include "cnvp_fdp_core.h"
#include "cnv_osdep.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define CNV_S3_STRIDE_SIZE	    (0x00000080)
#define CNV_S3_AREA_SIZE        (0x00000100)
#define CNV_S3_AREA_BYTE		(0x400)

#define CNV_S3_MAX_AREA_COUNT   (10)
#define CNV_S3_MAX_STRIDE_COUNT (7)
/* Release Index */
#define CNVPT_2D_FULLRATE_RELASE_INDEX (2)
#define CNVPT_3D_FULLRATE_RELASE_INDEX (2)
#define CNVPT_3D_HALFRATE_RELASE_INDEX (3)

/* Input case */
#define CNVP_BUFFER_DATA (0) 
#define CNVP_EOS_NULL    (1)
#define CNVP_EOS_DATA    (2)

#define CNVP_FDP_QUEUE   (2)
/* image size */
#define CNVP_FDP_MAX_IMAGE_WHDTH  (1920)
#define CNVP_FDP_MAX_IMAGE_HEIGHT (1920)
#define CNVP_FDP_MIN_IMAGE_WHDTH  (80)
#define CNVP_FDP_MIN_IMAGE_HEIGHT (80)

#define CNVP_FDP_OPEN_WIDTH  (1920)
#define CNVP_FDP_OPEN_HEIGHT (960)

#define ENABLE_S3_MODULE 1
#define ENABLE_FDP_MODULE 1

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/**
* The FdpcpCore_Callback1 will be called the fdpm.
*
* \param[in] pFdp_cb1       fdpm parameter
* \return none
*/
static CNV_VOID FdpcpCore_Callback1(T_FDP_CB1 *pFdp_cb1);

/**
* The FdpcpCore_Callback2 will be called the fdpm.
*
* \param[in] pFdp_cb2       fdpm parameter
* \return none
*/
static CNV_VOID FdpcpCore_Callback2(T_FDP_CB2 *pFdp_cb2);

/**
* The FdpcpCore_Callback4 will be called the fdpm.
*
* \param[in] pFdp_cb2       fdpm parameter
* \return none
*/
static CNV_VOID FdpcpCore_Callback4(T_FDP_CB2 *pFdp_cb2);

/**
* The FdpcpCore_CheckBufferFlag will check a intput buffer and a output buffer.
*
* \param[in] pEmptyBuffer   input buffer
* \param[in] pFillBuffer    output buffer
* \return CNV_BOOL
* \retval CNV_TRUE          the fdp plugin can convert input buffer.
* \retval CNV_FALSE         the fdp plugin can not convert input buffer.
*/
static CNV_BOOL FdpcpCore_CheckBufferFlag(CNV_BUFFERHEADER_T *pEmptyBuffer, CNV_BUFFERHEADER_T *pFillBuffer);

/**
* The FdpcpCore_SetUserData will set a user data to common context.
*
* \param[in] pCnvpContext           common context
* \param[in] pCurrentEmptyBuffer    intput buffer
* \param[in] pPastEmptyBuffer       intput buffer
* \return none
*/
static CNV_VOID FdpcpCore_SetUserData( CNVP_CONTEXT_T* pCnvpContext, CNV_BUFFERHEADER_T *pCurrentEmptyBuffer, CNV_BUFFERHEADER_T *pPastEmptyBuffer);

/**
* The FdpcpCore_CreatInputData will create the input data format of fdpm .
*
* \param[in]  pFdpHandle           plugin handle
* \param[in]  pFirstEmptyBuffer    input buffer 1
* \param[in]  pSecondEmptyBuffer   input buffer 2
* \param[in]  bBottomFirst         CNV_FALSE: top first, CNV_TRUE: bottom first
* \param[in]  nInputFormat         format
* \param[out] pFutureBuffer        input buffer 1
* \param[out] pCurrentBuffer       input buffer 2
* \param[out] pPastBuffer          input buffer 3
* \return none
*/

static CNV_VOID FdpcpCore_CreatInputData(
	FDP_HANDLE_T  		*pFdpHandle,
	CNV_BUFFERHEADER_T 	*pFirstEmptyBuffer,
	CNV_BUFFERHEADER_T 	*pSecondEmptyBuffer,
	CNV_BOOL			bBottomFirst,
	CNV_U32				nInputFormat,
	T_FDP_IMGBUF 		*pFutureBuffer,
	T_FDP_IMGBUF 		*pCurrentBuffer,
	T_FDP_IMGBUF 		*pPastBuffer);
	
/**
* The FdpcpCore_SetReleaseFlag will set a release flag.
*
* \param[in] pFdpHandle   plugin handle
* \return none
*/
static CNV_VOID FdpcpCore_SetReleaseFlag(FDP_HANDLE_T *pFdpHandle);

/**
* The FdpcpCore_GetS3CtrlStride will calculate the s3ctl stride.
*
* \param[in] nStride      stride
* \return                 stride( This stride can set s3ctl_set_param) 
*/
static CNV_U32 FdpcpCore_GetS3CtrlStride(CNV_U32 nStride);

/**
* The FdpcpCore_GetS3CtrlArea will calculate the s3ctl area.
*
* \param[in] nStride      area size
* \return                 area( This area can set s3ctl_set_param) 
*/
static CNV_U32 FdpcpCore_GetS3CtrlArea(CNV_U32 nArea);

/**
* The FdpcpCore_CheckChangeStride will check a input stride.
*
* \param[in] pFdpHandle   plugin handle
* \param[in] nStride      stride
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
static CNV_ERRORTYPE FdpcpCore_CheckChangeStride(FDP_HANDLE_T *pFdpHandle, CNV_U32 nStride);

/**
* The FdpcpCore_Locks3ctl will be locked s3ctl.
*
* \param[in] pFdpHandle   plugin handle
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
static CNV_ERRORTYPE FdpcpCore_Locks3ctl(FDP_HANDLE_T *pFdpHandle);

/**
* The FdpcpCore_CheckLastStart will check EOS.
*
* \param[in] pFdpHandle       plugin handle
* \param[in] pCurrentBuffer   input buffer
* \param[in] pFuturebuffer    input buffer
* \return CNV_BOOL
* \retval CNV_TRUE            EOS
* \retval CNV_FALSE           EOF
*/
static CNV_BOOL FdpcpCore_CheckLastStart(T_FDP_IMGBUF *pPastBuffer,T_FDP_IMGBUF *pCurrentBuffer,T_FDP_IMGBUF *pFuturebuffer);

/**
* The FdpcpCore_CheckLastStart will set input picture error flags.
*
* \param[in] pFdpHandle       plugin handle
* \param[in] pCurrentBuffer   input buffer
* \param[in] pFuturebuffer    input buffer
* \param[in] pFillBuffer      output buffer
* \return none
*/
static CNV_VOID FdpcpCore_SetPictrueErrorFlag(FDP_HANDLE_T *pFdpHandle, CNV_BUFFERHEADER_T *pCurrentBuffer,CNV_BUFFERHEADER_T *pPastBuffer,CNV_BUFFERHEADER_T *pFillBuffer);

/**
* The FdpcpCore_SetTimeStamp will set time stamp.
*
* \param[in] pFdpHandle       plugin handle
* \param[in] pCurrentBuffer   input buffer
* \param[in] pFuturebuffer    input buffer
* \param[in] pFillBuffer      output buffer
* \return none
*/
static CNV_VOID FdpcpCore_SetTimeStamp(FDP_HANDLE_T *pFdpHandle, CNV_BUFFERHEADER_T *pCurrentBuffer,CNV_BUFFERHEADER_T *pPastBuffer, CNV_BUFFERHEADER_T *pFillBuffer);

/**
* The FdpcpCore_CheckBufferInfo will check buffers information.
*
* \param[in] pFdpHandle       plugin handle
* \param[in] pEmptyBuffer     input buffer
* \param[in] pFillBuffer      output buffer
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
static CNV_ERRORTYPE FdpcpCore_CheckBufferInfo(CNVP_CONTEXT_T *pCnvpContext, CNV_BUFFERHEADER_T *pEmptyBuffer,  CNV_BUFFERHEADER_T *pFillBuffer);

/**
* The FdpcpCore_SetInputData will set input data.
*
* \param[out] pConvertBuffer   T_FDP_IMGBUF
* \param[in]  pBuffer          input buffer
* \param[in]  bInputMode       fdpm mode
* \param[in]  bBottomFirst     top or bottom first
* \param[in]  nInputFormat     input color format
* \return none
*/
static CNV_VOID FdpcpCore_SetInputData(T_FDP_IMGBUF *pConvertBuffer, CNV_BUFFERHEADER_T *pBuffer, CNV_BOOL bInputMode, CNV_BOOL bBottomFirst, CNV_U32 nInputFormat);

/**
* The FdpcpCore_SetFieldcf will set cf.
*
* \param[in] pFdpHandle       plugin handle
* \param[in] bBottomFirst     top or bottom first
* \param[in] bInputMode       fdpm mode
* \param[in] bInputType       input sequence
* \return none
*/
static CNV_VOID FdpcpCore_SetFieldcf(FDP_HANDLE_T *pFdpHandle, CNV_BOOL bBottomFirst, CNV_BOOL bInputType);

/**
* The FdpcpCore_CheckErrorFlag will check error flags.
*
* \param[in] pBuffer          input buffer
* \return CNV_BOOL
* \retval CNV_TRUE            include error flags
* \retval CNV_FALSE           error none
*/
static CNV_BOOL FdpcpCore_CheckErrorFlag(CNV_BUFFERHEADER_T *pBuffer);

/**
* The FdpcpCore_SetErrorFlag will set error flags.
*
* \param[in] pInputBuffer     input buffer
* \param[in] pOutputBuffer    optput buffer
* \return CNV_BOOL
* \retval CNV_TRUE            include error flags
* \retval CNV_FALSE           error none
*/
static CNV_BOOL FdpcpCore_SetErrorFlag(CNV_BUFFERHEADER_T *pInputBuffer, CNV_BUFFERHEADER_T *pOutputBuffer );

/**
* The FdpcpCore_CrateTimeStamp will create time stamp.
*
* \param[in] nTimeStamp          time stamp
* \param[in] nFrameTimeStamp     frame taime stamp
* \return time stamp
*/
static CNV_TICKS FdpcpCore_CrateTimeStamp(CNV_TICKS nTimeStamp, CNV_U32 nFrameTimeStamp );

/**
* The FdpcpCore_GetFullRateTime will get full rate time stamp.
*
* \param[in] pBuffer             input buffer
* \param[in] nFrameRateTime      frame rate time
* \return none
*/
static CNV_VOID FdpcpCore_GetFullRateTime(CNV_BUFFERHEADER_T *pBuffer, CNV_U32 *nFrameRateTime);

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
CNV_ERRORTYPE CnvpCore_GetPluginCoreFunction(
	CNVP_PLUGIN_FUNCS_T *psPlugInFunc)
{	
	if( NULL == psPlugInFunc){
		return CNV_ERROR_FATAL;
	}

	psPlugInFunc->CNVP_CreateHandle 			= &FdpcpCore_CreateHandle;
	psPlugInFunc->CNVP_DeleteHandle				= &FdpcpCore_DeleteHandle;
	psPlugInFunc->CNVP_ModuleInit	 			= &FdpcpCore_InitPluginParam;
	psPlugInFunc->CNVP_CheckConvertStart		= &FdpcpCore_CheckConvertBuffer;
	psPlugInFunc->CNVP_GetFlushBuffer			= &FdpcpCore_CheckFlushBuffer;
	psPlugInFunc->CNVP_ConverterExecuting		= &FdpcpCore_Executing;
	psPlugInFunc->CNVP_CheckReleaseBuffer		= &FdpcpCore_CheckReleaseBuffer;
	psPlugInFunc->CNVP_PluginSelected			= &FdpcpCore_PluginSelected;

	CNV_LOGGER(CNV_LOG_DEBUG,"psPlugInFunc->CNVP_CreateHandle %d", (CNV_U32)psPlugInFunc->CNVP_CreateHandle);
	
	return CNV_ERROR_NONE;
}


CNV_ERRORTYPE FdpcpCore_PluginSelected (
  CNV_PLUGIN_DICISION_RULE_T 	*psDicisionRule,
  CNV_BOOL						*pbResult,
  CNV_SUBERRORTYPE 				*peSubErrorCode)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	
	CnvpCmn_SetSubErrorCode( peSubErrorCode, CNV_ERROR_SUBMODULE_PARAM );

	if(NULL == pbResult){
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	if(NULL == psDicisionRule){
		*pbResult = CNV_FALSE;
		return CNV_ERROR_INVALID_PARAMETER;
	}

	/* Check progressive */
	if((psDicisionRule->bInterlacedSequence == CNV_FALSE) && (psDicisionRule->bBottomFirst == CNV_TRUE )){ 
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	/* check plugin rule */
	if( psDicisionRule->bCropping != CNV_FALSE ){
		return  CNV_ERROR_INVALID_PARAMETER;
	}
	
	if( psDicisionRule->bScaling != CNV_FALSE ){
		*pbResult = CNV_FALSE;
		return  CNV_ERROR_INVALID_PARAMETER;
	}

	*pbResult = CNV_TRUE;
	CnvpCmn_SetSubErrorCode( peSubErrorCode, CNV_SUBERROR_NONE );
	
	return eResult;
}


CNV_ERRORTYPE FdpcpCore_CreateHandle(
	CNV_PTR 		 *hPluginHandle, 
	CNV_U32 		 nConvertMode,
	CNV_SUBERRORTYPE *peSubErrorCode)
{
	FDP_HANDLE_T  *hHandle;
	CNV_U32		  nCnt;
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	
	hHandle = ( FDP_HANDLE_T *)Cnvdep_Malloc(sizeof(FDP_HANDLE_T));
	if( NULL == hHandle){
		CnvpCmn_SetSubErrorCode( peSubErrorCode, CNV_ERROR_SUBMODULE_PARAM_CREATE_HANDLE );
		return CNV_ERROR_INVALID_PARAMETER;
	}
	(void)Cnvdep_Memset(hHandle,0,sizeof(FDP_HANDLE_T));
	
	/* clear */
	(void)Cnvdep_Memset(&hHandle->sLocalOutputBuffer,0,sizeof(CNV_BUFFERHEADER_T));	
	
	/* Init parameter */
	for( nCnt = 0; nCnt < CNVP_FDP_QUEUE; nCnt++){
		(void)Cnvdep_Memset(&hHandle->sLocalInputBuffer[nCnt],0,sizeof(CNV_BUFFERHEADER_T));
		hHandle->sConvertEmptyBuffer[nCnt].nReleaseFlag      = CNV_BUFFER_NONE; 
		hHandle->sConvertEmptyBuffer[nCnt].pConvertBuffer    = &hHandle->sLocalInputBuffer[nCnt];
	}

	hHandle->bEosFlag     = CNV_FALSE;
	hHandle->bFirstInput  = CNV_FALSE;
	hHandle->bReleaseFlag = CNV_FALSE;
	hHandle->sConvertFillBuffer.nReleaseFlag   = CNV_BUFFER_NONE;
	hHandle->sConvertFillBuffer.pConvertBuffer = &hHandle->sLocalOutputBuffer;
	hHandle->nInputIndex = 0;
	
	*hPluginHandle = (CNV_PTR)hHandle;
	
	return eResult;
}


CNV_ERRORTYPE FdpcpCore_DeleteHandle(CNV_PTR hPluginHandle)
{
	FDP_HANDLE_T  *hHandle;
	CNV_S32		  nFdpResult;
	CNV_S32		  nS3Result;
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	
	hHandle = (FDP_HANDLE_T *)hPluginHandle;
	
	if( CNV_TRUE == hHandle->bInitFlag){
	
#ifdef ENABLE_S3_MODULE	
		/* S3 Ctrl */
		if( CNV_TRUE == hHandle->bS3mode){
			nS3Result = s3ctl_clear_param((S3CTL_ID)hHandle->nS3ctrlId);
			if( R_S3_OK != nS3Result ){
				CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_clear_param fatal %d", (CNV_U32)nS3Result);
			}
			nS3Result = s3ctl_close((S3CTL_ID)hHandle->nS3ctrlId);
			if( R_S3_OK != nS3Result ){
				CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_clear_param fatal %d", (CNV_U32)nS3Result);
			}
		}
#endif
		
#ifdef ENABLE_FDP_MODULE
		nFdpResult = drv_FDPM_Close(NULL, (CNV_S32*)&(hHandle->pFdpModuleHandle), 1);
		if(0 != nFdpResult){
			CNV_LOGGER(CNV_LOG_ERROR,"drv_FDPM_Close fatal %d", (CNV_U32)nFdpResult);
		}
#endif
	}
	Cnvdep_Free(hPluginHandle);
		
	return eResult;
}

CNV_ERRORTYPE FdpcpCore_InitPluginParam(
  CNV_PTR  pParam,
  CNV_PTR *ppBuffer
)
{
	CNV_S32		  		nS3Result;
	CNV_S32				nFdpResult = 0;
	CNV_S32				sub_ercd;
	CNVP_CONTEXT_T 		*pCnvpContext;
	FDP_HANDLE_T  		*pFdpHandle;
	struct S3_PARAM		sS3CtlParam;
	CNV_ERRORTYPE		eResult = CNV_ERROR_NONE;
	
	T_FDP_OPEN			sFdpOpen;
	T_FDP_IMGSIZE		sFdpImageSize;
	
	pCnvpContext = (CNVP_CONTEXT_T *)pParam;
	pFdpHandle   = (FDP_HANDLE_T *)pCnvpContext->pCnvpDepHandle;
	
	/* Init Param */
	pFdpHandle->nInputIndex = 0;
	pFdpHandle->bS3mode = pCnvpContext->sPluginRule.bTileMode;
	
	if( CNV_TRUE == pFdpHandle->bS3mode ){
#ifdef 	ENABLE_S3_MODULE
		/* S3 ctrl */
		(void)Cnvdep_Memset(&sS3CtlParam,0,sizeof(sS3CtlParam));
		nS3Result = s3ctl_open((S3CTL_ID*)&pFdpHandle->nS3ctrlId);
		if( R_S3_OK != nS3Result ){
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_open fatal %d", (CNV_U32)nS3Result);
			eResult = CNV_ERROR_FATAL;
		} else {
			/* Set S3ctrl param */
			sS3CtlParam.phy_addr	= (CNV_U32)pCnvpContext->sS3Param.nPhyAddr;
			sS3CtlParam.stride		= (CNV_U32)pCnvpContext->sS3Param.nStride;
			sS3CtlParam.area		= (CNV_U32)pCnvpContext->sS3Param.nArea;
			
			sS3CtlParam.stride 		= FdpcpCore_GetS3CtrlStride((CNV_U32)pCnvpContext->sS3Param.nStride);
			sS3CtlParam.area		= FdpcpCore_GetS3CtrlArea((CNV_U32)pCnvpContext->sS3Param.nArea);
			CNV_LOGGER(CNV_LOG_DEBUG,"S3Param.stride = %d", (CNV_U32)sS3CtlParam.stride);
			CNV_LOGGER(CNV_LOG_DEBUG,"S3Param.area   = %d", (CNV_U32)sS3CtlParam.area);
			nS3Result = s3ctl_set_param((S3CTL_ID)pFdpHandle->nS3ctrlId,&sS3CtlParam);
			if( R_S3_OK != nS3Result ){
				CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_set_param error = %d", (CNV_U32)nS3Result);
				eResult = CNV_ERROR_FATAL;
			} else {
				pFdpHandle->nStride = (CNV_U32)pCnvpContext->sS3Param.nStride;
			}
		}
#endif
	}
	
	/* Converter default mode(Auto) */
	if( (pCnvpContext->nConvertMode == CNVP_DEFAULT_CONVERT_MODE) && ( pCnvpContext->sPluginRule.bInterlacedSequence == CNV_TRUE )){
		CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_3D_HALFRATE MODE",CNV_DEINT_3D_HALFRATE);
		pCnvpContext->nConvertMode = CNV_DEINT_3D_HALFRATE;
	}
	
	if(pCnvpContext->nConvertMode == CNVP_DEFAULT_CONVERT_MODE){
		CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_NONE MODE",CNV_DEINT_NONE);
		pCnvpContext->nConvertMode = CNV_DEINT_NONE;
	}
	
	/* Input Info */
	(void)Cnvdep_Memset(&sFdpImageSize,0,sizeof(T_FDP_IMGSIZE));
	sFdpImageSize.width	= CNVP_FDP_OPEN_WIDTH;
	if( CNV_DEINT_NONE == pCnvpContext->nConvertMode){
		sFdpImageSize.height	= CNVP_FDP_OPEN_HEIGHT;
	} else {
		sFdpImageSize.height	= CNVP_FDP_OPEN_HEIGHT;
	}
	
	pFdpHandle->nOpenWidth  = sFdpImageSize.width;
	pFdpHandle->nOpenHeight = sFdpImageSize.height;
	
	(void)Cnvdep_Memset(&sFdpOpen,0,sizeof(T_FDP_OPEN));
	sFdpOpen.ref_mode 		= 0;
	sFdpOpen.refbuf_mode	= 0;
	sFdpOpen.refbuf			= NULL;
	sFdpOpen.ocmode			= FDP_OCMODE_OCCUPY;
	sFdpOpen.vmode			= FDP_VMODE_VBEST;
	sFdpOpen.clkmode		= FDP_CLKMODE_1;
	sFdpOpen.vcnt			= 166;
	sFdpOpen.insize			= &sFdpImageSize;
	
#ifdef ENABLE_FDP_MODULE
	pFdpHandle->sCallBackParam2.userdata2 = pCnvpContext;
	nFdpResult = drv_FDPM_Open(NULL, NULL, (CNV_PTR)&FdpcpCore_Callback4, &sFdpOpen, NULL,
		NULL, NULL, (CNV_PTR)&pFdpHandle->sCallBackParam2, &sub_ercd);
	
	if( 0 != nFdpResult ){
		/* FDPM issue */
		CNV_LOGGER(CNV_LOG_ERROR,"drv_FDPM_Open error result = %d", (CNV_U32)nFdpResult);
		CNV_LOGGER(CNV_LOG_ERROR,"drv_FDPM_Open error sub_ercd = %d", (CNV_U32)sub_ercd);
		eResult = CNV_ERROR_FATAL;
	} else {
		pFdpHandle->pFdpModuleHandle = (CNV_U32)sub_ercd;
	}
#endif
	
	pFdpHandle->bInitFlag = CNV_TRUE;
		
	return eResult;
}


CNV_BOOL FdpcpCore_CheckConvertBuffer(CNV_PTR pParam)
{
	CNV_BOOL			bConvertCheck = CNV_FALSE;
	
	CNVP_CONTEXT_T 		*pCnvpContext;
	CNVP_CMN_HANDLE_T	*pCnvpCmnHdl;
	FDP_HANDLE_T  		*pFdpHandle;
	
	CNV_BUFFERHEADER_T *pEmptyBuffer = NULL;
	CNV_BUFFERHEADER_T *pFillBuffer  = NULL;
	
	/* cast */
	pCnvpContext   	= (CNVP_CONTEXT_T*)pParam;
	pCnvpCmnHdl 	= (CNVP_CMN_HANDLE_T*)pCnvpContext->pCnvpCmnHandle;	
	pFdpHandle		= (FDP_HANDLE_T*)pCnvpContext->pCnvpDepHandle;
	
	/* Get Buffer */
	(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_FILLBUFFER, &pFdpHandle->sConvertFillBuffer);	
	pFillBuffer  = pFdpHandle->sConvertFillBuffer.pConvertBuffer;
	if(NULL != pFillBuffer->sTopArea.pTopAddress_Y_Area ){
		/* Mode dep */
		switch(pCnvpContext->nConvertMode)
		{
		case(CNV_DEINT_NONE):
			/* Check Buffer Data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_NONE",CNV_DEINT_NONE);
			/* Get Buffer */
			(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER, &pFdpHandle->sConvertEmptyBuffer[0]);
			pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
			bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
			if( CNV_TRUE == bConvertCheck ){
				pFdpHandle->nConvertMode = FDP_SEQ_PROG;
			}
			break;
		case(CNV_DEINT_2D_HALFRATE):
			/* Check Buffer Data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_2D_HALFRATE",CNV_DEINT_2D_HALFRATE);
			/* Get Buffer */
			(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER, &pFdpHandle->sConvertEmptyBuffer[0]);
			pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
			bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
			pFdpHandle->nConvertMode = FDP_SEQ_INTERH_2D;
			break;
		case(CNV_DEINT_2D_FULLRATE):
			/* Check Buffer Data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_2D_FULLRATE",CNV_DEINT_2D_FULLRATE);
			/* Get Buffer */
			(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER, &pFdpHandle->sConvertEmptyBuffer[0]);
			pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
			if( CNV_TRUE != pFdpHandle->bEosFlag ){
				bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
			}
			pFdpHandle->nConvertMode = FDP_SEQ_INTER_2D;
			break;
		case(CNV_DEINT_3D_HALFRATE):
			/* Check Buffer Data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_3D_HALFRATE",CNV_DEINT_3D_HALFRATE);
			/* Get Buffer */
			if( 1 == pFdpHandle->nInputIndex){
				pFdpHandle->nInputIndex = 0;
				/* data copy */
				(void)Cnvdep_Memcpy(pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer,
					pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer,sizeof(CNV_BUFFERHEADER_T));
				
				/* mem clear*/
				(void)Cnvdep_Memset(pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer,0,sizeof(CNV_BUFFERHEADER_T));
				/* Set buffer Flag*/
				pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag   = pFdpHandle->sConvertEmptyBuffer[1].nReleaseFlag;
				pFdpHandle->sConvertEmptyBuffer[1].nReleaseFlag   = CNV_BUFFER_NONE;
			}
			(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER, &pFdpHandle->sConvertEmptyBuffer[1]);
			if( CNV_TRUE != pFdpHandle->bEosFlag ){
				pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer;
				bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
				if( CNV_FALSE == bConvertCheck ){
					/* Data + EOS*/
					pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
					if( CNVP_EOS_FLAG == pEmptyBuffer->nFlag ){
						bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
					}
				}
			} else {
				/* NULL + EOS*/
				if(  CNVP_EOS_FLAG == pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag ){
						bConvertCheck = CNV_TRUE;
				}
			}
			pFdpHandle->nConvertMode = FDP_SEQ_INTERH;
			break;
		case(CNV_DEINT_3D_FULLRATE):
			/* Check Buffer Data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_3D_FULLRATE",CNV_DEINT_3D_FULLRATE);
			/* Get Buffer */
			if( 1 == pFdpHandle->nInputIndex){
				if( CNV_BUFFER_NONE == pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag ){
					/* data copy */
					(void)Cnvdep_Memcpy(pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer,
					pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer,sizeof(CNV_BUFFERHEADER_T));
					/* mem clear*/
					(void)Cnvdep_Memset(pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer,0,sizeof(CNV_BUFFERHEADER_T));
					/* Set buffer Flag*/
					pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag   = pFdpHandle->sConvertEmptyBuffer[1].nReleaseFlag;
					pFdpHandle->sConvertEmptyBuffer[1].nReleaseFlag   = CNV_BUFFER_NONE;
				}
			}
			(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER, &pFdpHandle->sConvertEmptyBuffer[1]);
			if( CNV_TRUE != pFdpHandle->bEosFlag ){
				pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer;
				bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
				if( CNV_FALSE == bConvertCheck ){
					/* Data + EOS*/
					pEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
					if( CNVP_EOS_FLAG == pEmptyBuffer->nFlag ){
						bConvertCheck = FdpcpCore_CheckBufferFlag(pEmptyBuffer,pFillBuffer);
					}
					CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_3D_FULLRATE (DATA + EOS) Check = %d ", (CNV_U32)bConvertCheck);
				}
			} else {
				/* NULL + EOS*/
				if(  CNVP_EOS_FLAG == pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag ){
					bConvertCheck = CNV_TRUE;
				} else {
					if( CNV_TRUE == pFdpHandle->bEosFlag ){
						bConvertCheck = CNV_TRUE;
					}
				}
				CNV_LOGGER(CNV_LOG_DEBUG,"CNV_DEINT_3D_FULLRATE (NULL + EOS) Check = %d", (CNV_U32)bConvertCheck);
			}
			pFdpHandle->nConvertMode = FDP_SEQ_INTER;
			break;
		default:
			break;
		}
	}
	CNV_LOGGER(CNV_LOG_DEBUG,"Convert Flag = %d", (CNV_U32)bConvertCheck);
	return bConvertCheck;
}


static CNV_ERRORTYPE FdpcpCore_CheckBufferInfo(
	CNVP_CONTEXT_T *pCnvpContext,
	CNV_BUFFERHEADER_T *pEmptyBuffer, 
	CNV_BUFFERHEADER_T *pFillBuffer)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	CNV_U32 nInWidth_C;
	CNV_U32 nInHeight_C;
	CNV_U32 nOutWidth_C;
	CNV_U32 nOutHeight_C;
	
	if(CNV_DEINT_NONE == pCnvpContext->nConvertMode){
		if((pEmptyBuffer->sTopAreaImage.nWidth > CNVP_FDP_MAX_IMAGE_WHDTH) || (pEmptyBuffer->sTopAreaImage.nHeight > CNVP_FDP_MAX_IMAGE_HEIGHT)){
			CNV_LOGGER(CNV_LOG_ERROR,"max image size error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((pEmptyBuffer->sTopAreaImage.nWidth < CNVP_FDP_MIN_IMAGE_WHDTH) || (pEmptyBuffer->sTopAreaImage.nHeight < CNVP_FDP_MIN_IMAGE_HEIGHT)){
			CNV_LOGGER(CNV_LOG_ERROR,"min image size error",0);
			eResult = CNV_ERROR_FATAL;
		}
	} else {
		if((pEmptyBuffer->sTopAreaImage.nWidth > CNVP_FDP_MAX_IMAGE_WHDTH) || (pEmptyBuffer->sTopAreaImage.nHeight > (CNVP_FDP_MAX_IMAGE_HEIGHT/2))){
			CNV_LOGGER(CNV_LOG_ERROR,"max image size error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((pEmptyBuffer->sTopAreaImage.nWidth < CNVP_FDP_MIN_IMAGE_WHDTH) || (pEmptyBuffer->sTopAreaImage.nHeight < (CNVP_FDP_MIN_IMAGE_HEIGHT/2))){
			CNV_LOGGER(CNV_LOG_ERROR,"min image size error",0);
			eResult = CNV_ERROR_FATAL;
		}
	}
		
	if( (CNV_YUV420SP == pCnvpContext->sPluginRule.nInputFormat)|| (CNV_YVU420SP == pCnvpContext->sPluginRule.nInputFormat)){
		/* SemiPlaner */
		nInWidth_C  = pEmptyBuffer->sTopAreaImage.nWidth;
		nInHeight_C = pEmptyBuffer->sTopAreaImage.nHeight/2;
	}else{
		/* Planer */
		nInWidth_C  = pEmptyBuffer->sTopAreaImage.nWidth/2;
		nInHeight_C = pEmptyBuffer->sTopAreaImage.nHeight/2;
	}
	
	if( (CNV_YUV420SP == pCnvpContext->sPluginRule.nOutputFormat)|| (CNV_YVU420SP == pCnvpContext->sPluginRule.nOutputFormat)){
		/* SemiPlaner */
		nOutWidth_C  = pEmptyBuffer->sTopAreaImage.nWidth;
		nOutHeight_C = pEmptyBuffer->sTopAreaImage.nHeight/2;
	}else{
		/* Planer */
		nOutWidth_C  = pEmptyBuffer->sTopAreaImage.nWidth/2;
		nOutHeight_C = pEmptyBuffer->sTopAreaImage.nHeight/2;
	}
	
	if(  CNV_TRUE != pCnvpContext->sPluginRule.bInterlacedSequence ){
		/* EmptyBuffer */
		if((pEmptyBuffer->sTopAreaImage.nWidth > pEmptyBuffer->sTopArea.nStride_Y_Area) ||
			(pEmptyBuffer->sTopAreaImage.nHeight > pEmptyBuffer->sTopArea.nHeight_Y_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Empty Y_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((nInWidth_C > pEmptyBuffer->sTopArea.nStride_C_Area) ||
			(nInHeight_C > pEmptyBuffer->sTopArea.nHeight_C_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Empty C_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		/* FillBuffer */
		if((pEmptyBuffer->sTopAreaImage.nWidth > pFillBuffer->sTopArea.nStride_Y_Area) ||
			(pEmptyBuffer->sTopAreaImage.nHeight > pFillBuffer->sTopArea.nHeight_Y_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Fill Y_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((nOutWidth_C > pFillBuffer->sTopArea.nStride_C_Area) ||
			(nOutHeight_C > pFillBuffer->sTopArea.nHeight_C_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Fill C_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
	} else {
		/* EmptyBuffer */
		if((pEmptyBuffer->sTopAreaImage.nWidth > pEmptyBuffer->sTopArea.nStride_Y_Area) ||
			(pEmptyBuffer->sTopAreaImage.nHeight > pEmptyBuffer->sTopArea.nHeight_Y_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Empty top Y_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((nInWidth_C > pEmptyBuffer->sTopArea.nStride_C_Area) ||
			(nInHeight_C > pEmptyBuffer->sTopArea.nHeight_C_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Empty top C_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((pEmptyBuffer->sBottomAreaImage.nWidth > pEmptyBuffer->sBottomArea.nStride_Y_Area) ||
			(pEmptyBuffer->sBottomAreaImage.nHeight > pEmptyBuffer->sBottomArea.nHeight_Y_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Empty bottom Y_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((nInWidth_C > pEmptyBuffer->sBottomArea.nStride_C_Area) ||
			(nInHeight_C > pEmptyBuffer->sBottomArea.nHeight_C_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Empty bottom C_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		/* FillBuffer */
		if((pEmptyBuffer->sTopAreaImage.nWidth > pFillBuffer->sTopArea.nStride_Y_Area) ||
			(pEmptyBuffer->sTopAreaImage.nHeight > pFillBuffer->sTopArea.nHeight_Y_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Fill Y_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
		if((nOutWidth_C > pFillBuffer->sTopArea.nStride_C_Area) ||
			(nOutHeight_C > pFillBuffer->sTopArea.nHeight_C_Area)){
			CNV_LOGGER(CNV_LOG_ERROR,"Fill C_Area error",0);
			eResult = CNV_ERROR_FATAL;
		}
	}
	
	CNV_LOGGER(CNV_LOG_DEBUG,"Input buffer nWidth = %d",pEmptyBuffer->sTopAreaImage.nWidth);
	CNV_LOGGER(CNV_LOG_DEBUG,"Input buffer nHeigh = %d",pEmptyBuffer->sTopAreaImage.nHeight);
	
	return eResult;
}


CNV_ERRORTYPE FdpcpCore_Executing (
  CNV_PTR			 	pParam,
  CNV_BOOL				bStart,
  CNV_SUBERRORTYPE 		*peSubErrorCode)
{
	CNV_ERRORTYPE 		eResult = CNV_ERROR_NONE;
	CNV_S32				s32Result = 0L;

	CNVP_CONTEXT_T 		*pCnvpContext;
	FDP_HANDLE_T  		*pFdpHandle;

	CNV_BUFFERHEADER_T *pPastEmptyBuffer;
	CNV_BUFFERHEADER_T *pCurrentEmptyBuffer;
	CNV_BUFFERHEADER_T *pFillBuffer;
	
	/* FDP Param */
	T_FDP_START			sStartParam;
	T_FDP_FPROC			sFprocParam;
	T_FDP_SEQ			sSeqParam;
	T_FDP_IMGSET		sImageSet;
	T_FDP_PIC			sPic;
	T_FDP_PICPAR		sPicParam;
	T_FDP_REFBUF		sRefBuffer;
	T_FDP_IMGBUF		sOutPutBuffer;
	T_FDP_IMGBUF 		sFutureBuffer;
	T_FDP_IMGBUF 		sCurrentBuffer;
	T_FDP_IMGBUF 		sPastBuffer;
	
	/* cast */
	pCnvpContext   	= (CNVP_CONTEXT_T*)pParam;
	pFdpHandle		= (FDP_HANDLE_T*)pCnvpContext->pCnvpDepHandle;
	
	/* Buffer Data */
	if( (FDP_SEQ_PROG == pFdpHandle->nConvertMode) || 
		(FDP_SEQ_INTER_2D == pFdpHandle->nConvertMode) ||
		(FDP_SEQ_INTERH_2D == pFdpHandle->nConvertMode)) {
		pCurrentEmptyBuffer	 = (CNV_BUFFERHEADER_T *)pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
		pPastEmptyBuffer     = NULL;
	} else {
		pPastEmptyBuffer	 = (CNV_BUFFERHEADER_T *)pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
		pCurrentEmptyBuffer	 = (CNV_BUFFERHEADER_T *)pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer;
	}
	pFillBuffer 		 = (CNV_BUFFERHEADER_T *)pFdpHandle->sConvertFillBuffer.pConvertBuffer;
	
	CNV_LOGGER(CNV_LOG_DEBUG,"Input Buffer   Yaddr = %p", (CNV_U32)pCurrentEmptyBuffer->sTopArea.pTopAddress_Y_Area);
	CNV_LOGGER(CNV_LOG_DEBUG,"Output Buffer  Yaddr = %p", (CNV_U32)pFillBuffer->sTopArea.pTopAddress_Y_Area );
	
	/* check image size */
	if(NULL != pCurrentEmptyBuffer->sTopArea.pTopAddress_Y_Area){
		if(NULL != pFillBuffer->sTopArea.pTopAddress_Y_Area ){
			eResult = FdpcpCore_CheckBufferInfo(pCnvpContext,pCurrentEmptyBuffer,pFillBuffer);
		} else {
			eResult = CNV_ERROR_NONE;
		}
	}
	
	if( (CNV_TRUE == bStart) && (CNV_ERROR_NONE == eResult) ){
		/* T_FDP_SEQ */
		(void)Cnvdep_Memset(&sSeqParam,0,sizeof(T_FDP_SEQ));
		sSeqParam.seq_mode  		= (CNV_U8)pFdpHandle->nConvertMode;
		sSeqParam.scale_mode		= 0;
		sSeqParam.filter_mode		= 0;
		sSeqParam.telecine_mode		= 0;/* FDP_TC_OFF */
		sSeqParam.in_width			= (CNV_U16)pCurrentEmptyBuffer->sTopAreaImage.nWidth;
		if( (CNV_DEINT_NONE == pCnvpContext->nConvertMode) && ( CNV_TRUE == pCnvpContext->sPluginRule.bInterlacedSequence )){
			sSeqParam.in_height			= (CNV_U16)(pCurrentEmptyBuffer->sTopAreaImage.nHeight*2U);
		} else {
			sSeqParam.in_height			= (CNV_U16)(pCurrentEmptyBuffer->sTopAreaImage.nHeight);
		}
		sSeqParam.imgleft			= 0U;/* Ignore this member */
		sSeqParam.imgtop			= 0U;/* Ignore this member */
		sSeqParam.imgwidth			= (CNV_U16)pCurrentEmptyBuffer->sTopAreaImage.nWidth;/* Ignore this member */
		sSeqParam.imgheight			= (CNV_U16)pCurrentEmptyBuffer->sTopAreaImage.nHeight;/* Ignore this member */
		sSeqParam.out_width			= (CNV_U16)pFillBuffer->sTopAreaImage.nWidth;/* Ignore this member */
		sSeqParam.out_height		= (CNV_U16)pFillBuffer->sTopAreaImage.nHeight;/* Ignore this member */
		sSeqParam.ratio				= NULL;
		
		/* T_FDP_PIC */
		(void)Cnvdep_Memset(&sPicParam,0,sizeof(T_FDP_PICPAR));
		sPicParam.width					= (CNV_U16)pCurrentEmptyBuffer->sTopAreaImage.nWidth;
		sPicParam.height				= (CNV_U16)pCurrentEmptyBuffer->sTopAreaImage.nHeight;
		switch(pCnvpContext->sPluginRule.nInputFormat)
		{
		case(CNV_YUV420SP):
			sPicParam.chroma_format  = FDP_YUV420;
			break;
		case(CNV_YVU420SP):
			sPicParam.chroma_format  = FDP_YUV420_NV21;
			break;
		case(CNV_YUV420P):
			sPicParam.chroma_format  = FDP_YUV420_YV12;
			break;
		case(CNV_YVU420P):
			sPicParam.chroma_format  = FDP_YUV420_YV12;
			break;
		default:
			break;
		}
		
		/* Create FDP Input Data */
		(void)Cnvdep_Memset(&sFutureBuffer,0,sizeof(T_FDP_IMGBUF));
		(void)Cnvdep_Memset(&sCurrentBuffer,0,sizeof(T_FDP_IMGBUF));
		(void)Cnvdep_Memset(&sPastBuffer,0,sizeof(T_FDP_IMGBUF));
		FdpcpCore_CreatInputData(
				pFdpHandle, 
				pCurrentEmptyBuffer,
				pPastEmptyBuffer,
				pCnvpContext->sPluginRule.bBottomFirst,
				pCnvpContext->sPluginRule.nInputFormat,
				&sFutureBuffer, /* output */
				&sCurrentBuffer,/* output */
				&sPastBuffer);  /* output */
		
		/* Set Release Flag */
		FdpcpCore_SetReleaseFlag(pFdpHandle);

		/* Set userpointer */
		if( ( CNV_DEINT_3D_FULLRATE == pCnvpContext->nConvertMode ) || ( CNV_DEINT_3D_HALFRATE == pCnvpContext->nConvertMode ) ){
			FdpcpCore_SetUserData(pCnvpContext, pCurrentEmptyBuffer,pPastEmptyBuffer);
		} else {
			FdpcpCore_SetUserData(pCnvpContext, pCurrentEmptyBuffer,NULL);
		}
		
		/* T_FDP_PIC */
		(void)Cnvdep_Memset(&sPic,0,sizeof(T_FDP_PIC));
		sPic.picid   = pFdpHandle->pFdpModuleHandle;
		sPic.pic_par = &sPicParam;
		sPic.in_buf1 = NULL;
		sPic.in_buf2 = NULL;
		
		/* T_FDP_IMGBUF */
		(void)Cnvdep_Memset(&sOutPutBuffer,0,sizeof(T_FDP_IMGBUF));
		sOutPutBuffer.addr     = pFillBuffer->sTopArea.pTopAddress_Y_Area;
		if( CNV_YVU420P == pCnvpContext->sPluginRule.nOutputFormat){
			sOutPutBuffer.addr_c0  = pFillBuffer->sTopArea.pTopAddress_C_Area1;
			sOutPutBuffer.addr_c1  = pFillBuffer->sTopArea.pTopAddress_C_Area0;
		} else {
			sOutPutBuffer.addr_c0  = pFillBuffer->sTopArea.pTopAddress_C_Area0;
			sOutPutBuffer.addr_c1  = pFillBuffer->sTopArea.pTopAddress_C_Area1;
		}
		sOutPutBuffer.stride   = (CNV_U16)pFillBuffer->sTopArea.nStride_Y_Area;
		sOutPutBuffer.stride_c = (CNV_U16)pFillBuffer->sTopArea.nStride_C_Area;
		sOutPutBuffer.height   = (CNV_U16)pFillBuffer->sTopArea.nHeight_Y_Area;
		sOutPutBuffer.height_c = (CNV_U16)pFillBuffer->sTopArea.nHeight_C_Area;
		
		/* T_FDP_REFBUF */
		(void)Cnvdep_Memset(&sRefBuffer,0,sizeof(T_FDP_REFBUF));
		sRefBuffer.buf_refwr  = NULL;
		sRefBuffer.buf_iirwr  = NULL;
		sRefBuffer.buf_iirrd  = NULL;
		
		sRefBuffer.buf_refrd0 = &sFutureBuffer;
		sRefBuffer.buf_refrd1 = &sCurrentBuffer;
		sRefBuffer.buf_refrd2 = &sPastBuffer;
		
		if( (sPicParam.width != pFdpHandle->nOpenWidth) || (sPicParam.height != pFdpHandle->nOpenHeight) ){
			pFdpHandle->bFirstInput = CNV_FALSE;
			pFdpHandle->nOpenWidth = sPicParam.width;
			pFdpHandle->nOpenHeight= sPicParam.height;
		}
		
		/* T_FDP_FPROC */
		(void)Cnvdep_Memset(&sFprocParam,0,sizeof(T_FDP_FPROC));
		if( CNV_FALSE == pFdpHandle->bFirstInput){
			sFprocParam.seq_par     = &sSeqParam;
			pFdpHandle->bFirstInput = CNV_TRUE;
		} else {
			sFprocParam.seq_par     = NULL; 
		}
		(void)Cnvdep_Memset(&sImageSet,0,sizeof(T_FDP_IMGSET));
		sFprocParam.imgset_par  = &sImageSet;		
		sFprocParam.in_pic      = &sPic;			
		sFprocParam.out_buf     = &sOutPutBuffer; 	
		sFprocParam.ref_buf     = &sRefBuffer;
		sFprocParam.cf          = (CNV_U8)pFdpHandle->bField;
		sFprocParam.last_start  = (CNV_U8)FdpcpCore_CheckLastStart(&sPastBuffer,&sCurrentBuffer,&sFutureBuffer);
		sFprocParam.f_decodeseq = 0;
		
		switch(pCnvpContext->sPluginRule.nOutputFormat)
		{
		case(CNV_YUV420SP):
			sFprocParam.out_format   = FDP_YUV420;
			break;
		case(CNV_YVU420SP):
			sFprocParam.out_format   = FDP_YUV420_NV21;
			break;
		case(CNV_YUV420P):
			sFprocParam.out_format   = FDP_YUV420_YV12;
			break;
		case(CNV_YVU420P):
			sFprocParam.out_format   = FDP_YUV420_YV12;
			break;
		default:
			break;
		}

		/* T_FDP_START */
		(void)Cnvdep_Memset(&sStartParam,0,sizeof(T_FDP_START));
		sStartParam.vcnt      = NULL;
		sStartParam.fdpgo     = FDP_GO;
		sStartParam.fproc_par = &sFprocParam;
		
		if( (NULL == sCurrentBuffer.addr) && (CNVP_EOS_FLAG == pCurrentEmptyBuffer->nFlag) && (CNV_TRUE != pFdpHandle->bEosFlag) ){
			/* NULL + EOS */ /* Case 2D FULL, 3D Half/FUll */
			if((FDP_SEQ_INTERH == pFdpHandle->nConvertMode)||(FDP_SEQ_INTER == pFdpHandle->nConvertMode)){
				pFdpHandle->bEosFlag = CNV_TRUE;
			} else {
				pFdpHandle->bEosFlag = CNV_FALSE;
				pFdpHandle->bFirstInput = CNV_FALSE;
			}
			pFdpHandle->nEosMode = CNVP_EOS_NULL;
			pFillBuffer->sTopArea.pTopAddress_Y_Area  = NULL;
			pFillBuffer->sTopArea.pTopAddress_C_Area0 = NULL;
			pFillBuffer->sTopArea.pTopAddress_C_Area1 = NULL;
			pFillBuffer->nTimeStamp = pCurrentEmptyBuffer->nTimeStamp;
			pFillBuffer->nErrorFlag = pCurrentEmptyBuffer->nErrorFlag;
			eResult = CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_DONE, NULL, peSubErrorCode);
		} else {
			/* FDPM Start */
			if(CNV_TRUE == pFdpHandle->bEosFlag){
				/* Case 2D Full 3D Half/Full */
				pFdpHandle->bEosFlag = CNV_FALSE;
				if((FDP_SEQ_INTERH == pFdpHandle->nConvertMode)||(FDP_SEQ_INTER == pFdpHandle->nConvertMode)){
					pFdpHandle->bFirstInput = CNV_FALSE;
				}
				eResult = CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_DONE, NULL, peSubErrorCode);
			} else {
				if( NULL == sCurrentBuffer.addr ){
					/* Case 2D Half / Deint */
					pFillBuffer->sTopArea.pTopAddress_Y_Area  = NULL;
					pFillBuffer->sTopArea.pTopAddress_C_Area0 = NULL;
					pFillBuffer->sTopArea.pTopAddress_C_Area1 = NULL;
					eResult = CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_DONE, NULL, peSubErrorCode);
				} else {
					if( CNVP_EOS_FLAG == pCurrentEmptyBuffer->nFlag ){
						pFdpHandle->nEosMode = CNVP_EOS_DATA;
					} else {
						pFdpHandle->nEosMode = CNVP_BUFFER_DATA;
					}
					
#ifdef ENABLE_S3_MODULE
					if (CNV_TRUE == pFdpHandle->bS3mode){
						eResult = FdpcpCore_CheckChangeStride(pFdpHandle,sCurrentBuffer.stride);
						if( CNV_ERROR_NONE != eResult){
							goto end_point;
						}
						eResult = FdpcpCore_Locks3ctl(pFdpHandle);
						if( CNV_ERROR_NONE != eResult){
							goto end_point;
						}
					}
#endif
					/* Set TimeStamp */
					FdpcpCore_SetTimeStamp(pFdpHandle,pCurrentEmptyBuffer,pPastEmptyBuffer,pFillBuffer);
					
					/* Set Picture ErrorCode */
					FdpcpCore_SetPictrueErrorFlag(pFdpHandle, pCurrentEmptyBuffer,pPastEmptyBuffer,pFillBuffer);
					
					/* FDPM Start */
					pFdpHandle->sCallBackParam1.userdata1 = pCnvpContext;
					pFdpHandle->sCallBackParam2.userdata2 = pCnvpContext;
					s32Result = drv_FDPM_Start( (CNV_PTR)&FdpcpCore_Callback1, (CNV_PTR)&FdpcpCore_Callback2, &sStartParam, (CNV_PTR)&pFdpHandle->sCallBackParam1, (CNV_PTR)&pFdpHandle->sCallBackParam2, (CNV_S32*)&(pFdpHandle->pFdpModuleHandle));
					if( 0 != s32Result){
						CNV_LOGGER(CNV_LOG_ERROR,"drv_FDPM_Start Result = %d", (CNV_U32)s32Result);
						eResult = CNV_ERROR_FATAL;
					}
				}
			}
		}
	}
end_point:
	return eResult;
}


CNV_ERRORTYPE FdpcpCore_CheckReleaseBuffer(
  CNV_PTR			 	pParam,
  CNV_PTR    			*ppEmptyBuffer,
  CNV_PTR			    *ppFillBuffer)
{
	CNVP_CONTEXT_T 		*pCnvpContext;
	FDP_HANDLE_T  		*pFdpHandle;
	
	CNV_U32				nCurFlag 			= 0;
	
	CNV_BUFFERHEADER_T *pEmptyBufferDone;
	CNV_BUFFERHEADER_T *pEmptyBufferDone2;
	CNV_BUFFERHEADER_T *pFillBufferDone;
	
	/* cast */
	pCnvpContext   	= (CNVP_CONTEXT_T*)pParam;
	pFdpHandle		= (FDP_HANDLE_T*)pCnvpContext->pCnvpDepHandle;
	
	pEmptyBufferDone = (CNV_BUFFERHEADER_T*)pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer;
	pEmptyBufferDone2= (CNV_BUFFERHEADER_T*)pFdpHandle->sConvertEmptyBuffer[1].pConvertBuffer;
	pFillBufferDone  = (CNV_BUFFERHEADER_T*)pFdpHandle->sConvertFillBuffer.pConvertBuffer;

	/* Check EmptyBuffer */
	if( NULL != pEmptyBufferDone ){
		if ( CNV_BUFFER_RELEASE == pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag ){
			*ppEmptyBuffer = pEmptyBufferDone;
			/* Release Buffer */
			pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag   = CNV_BUFFER_NONE;
			if( CNVP_EOS_NULL == pFdpHandle->nEosMode ){
				if((FDP_SEQ_INTERH == pFdpHandle->nConvertMode) || (FDP_SEQ_INTER == pFdpHandle->nConvertMode)){
					nCurFlag		= pEmptyBufferDone2->nFlag;
				} else {
					nCurFlag		= pEmptyBufferDone->nFlag;
				}
			} else {
				if((FDP_SEQ_INTERH == pFdpHandle->nConvertMode)){
					nCurFlag		= pEmptyBufferDone2->nFlag;
				} else {
					nCurFlag		= pEmptyBufferDone->nFlag;
				}
			}
		} else {
			*ppEmptyBuffer = NULL;
		}
	}
	
	/* Check FillBuffer */
	if( NULL != pFillBufferDone ){
		if ( CNV_BUFFER_RELEASE == pFdpHandle->sConvertFillBuffer.nReleaseFlag ){
			pFillBufferDone->nFlag 		   = nCurFlag;
			*ppFillBuffer = pFillBufferDone;
			
			if(CNVP_EOS_FLAG != pFillBufferDone->nFlag){
				if((FDP_SEQ_INTER_2D == pFdpHandle->nConvertMode) || (FDP_SEQ_INTER == pFdpHandle->nConvertMode)){
					if(pFdpHandle->nInputIndex == 1 ){
						pCnvpContext->sOutputPicInfo.bFrameExtended		  = CNV_FALSE;
						pCnvpContext->sOutputPicInfo.nFrameExtendedNum    = 0;
					} else {
						pCnvpContext->sOutputPicInfo.bFrameExtended		  = CNV_TRUE;
						pCnvpContext->sOutputPicInfo.nFrameExtendedNum    = 1;
					}
				} else {
					pCnvpContext->sOutputPicInfo.bFrameExtended		  = CNV_FALSE;
					pCnvpContext->sOutputPicInfo.nFrameExtendedNum    = 0;
				}
			} else {
				/* EOS */
				pCnvpContext->sOutputPicInfo.bFrameExtended		  = CNV_FALSE;
				pCnvpContext->sOutputPicInfo.nFrameExtendedNum    = 0;
				pCnvpContext->sOutputPicInfo.bEmptyKeepFlag       = CNV_FALSE;
			}
			/* Release Buffer */
			pFdpHandle->sConvertFillBuffer.nReleaseFlag   = CNV_BUFFER_NONE;
		} else {
			*ppFillBuffer = NULL;
		}
	}

	return CNV_ERROR_NONE;	
}


CNV_ERRORTYPE FdpcpCore_CheckFlushBuffer(
  CNV_PTR			 	pParam,
  CNV_PTR    			*ppEmptyBuffer,
  CNV_PTR			    *ppFillBuffer)
{
	CNVP_CONTEXT_T 		*pCnvpContext;
	FDP_HANDLE_T  		*pFdpHandle;
	CNV_U32				nCnt;
	
	/* cast */
	pCnvpContext   	= (CNVP_CONTEXT_T*)pParam;
	pFdpHandle		= (FDP_HANDLE_T*)pCnvpContext->pCnvpDepHandle;
	
	*ppEmptyBuffer = NULL;
	*ppFillBuffer  = NULL;

	/* Check FillBuffer */
	if( CNV_BUFFER_NONE != pFdpHandle->sConvertFillBuffer.nReleaseFlag ){
		*ppFillBuffer = pFdpHandle->sConvertFillBuffer.pConvertBuffer;
		pFdpHandle->sConvertFillBuffer.nReleaseFlag   = CNV_BUFFER_NONE;
	}
	
	/* Check EmptyBuffer */
	for( nCnt = 0; nCnt < CNVP_FDP_QUEUE; nCnt++){
		if( CNV_BUFFER_NONE != pFdpHandle->sConvertEmptyBuffer[nCnt].nReleaseFlag ){
			*ppEmptyBuffer = pFdpHandle->sConvertEmptyBuffer[nCnt].pConvertBuffer;
			pFdpHandle->sConvertEmptyBuffer[nCnt].nReleaseFlag   = CNV_BUFFER_NONE;
			break;
		}
 	}
	
	pFdpHandle->bEosFlag = CNV_FALSE;
	pFdpHandle->nInputIndex = 0;
	pFdpHandle->bFirstInput = CNV_FALSE;
	
	return CNV_ERROR_NONE;
}

/***************************************************************************
 * local Funciton .
 ***************************************************************************/
static CNV_VOID FdpcpCore_Callback1(T_FDP_CB1 *pFdp_cb1)
{
	return;
}


static CNV_VOID  FdpcpCore_Callback2(T_FDP_CB2 *pFdp_cb2)
{
	CNVP_CONTEXT_T 		*pCnvpContext;
	FDP_HANDLE_T  		*pFdpHandle;
	T_FDP_CB2           *pCbParam;
	CNV_S32				s3Result;
	
	pCbParam 		= (T_FDP_CB2*)pFdp_cb2->userdata2;
	pCnvpContext    = (CNVP_CONTEXT_T*)pCbParam->userdata2;
	pFdpHandle		= (FDP_HANDLE_T*)pCnvpContext->pCnvpDepHandle;
	
#ifdef ENABLE_S3_MODULE
	if (CNV_TRUE == pFdpHandle->bS3mode){
		s3Result = s3ctl_unlock((S3CTL_ID)pFdpHandle->nS3ctrlId);
		if( R_S3_OK != s3Result){
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_unlock error Result = %d", (CNV_U32)s3Result);
		}
	}
#endif
	(void)CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_DONE, NULL, NULL);
	
	return;
}


static CNV_VOID  FdpcpCore_Callback4(T_FDP_CB2 *pFdp_cb2)
{
	CNVP_CONTEXT_T 		*pCnvpContext;
	T_FDP_CB2           *pCbParam;
	
	pCbParam 		= (T_FDP_CB2*)pFdp_cb2->userdata2;
	pCnvpContext    = (CNVP_CONTEXT_T*)pCbParam->userdata2;
	
	if(CNV_FALSE == pCnvpContext->bDisableTimeout){
		if( pCnvpContext->nThreadState != CNVP_STATE_IDLE ){
			CNV_LOGGER(CNV_LOG_WARN,"time out event",0);
			(void)CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_TIMEOUT, NULL, NULL);	
		}
	} else {
		CNV_LOGGER(CNV_LOG_DEBUG,"time out event ignore",0);
	}
	return;
}


static CNV_BOOL FdpcpCore_CheckBufferFlag(CNV_BUFFERHEADER_T *pEmptyBuffer, CNV_BUFFERHEADER_T *pFillBuffer)
{
	CNV_BOOL bBufferCheck = CNV_FALSE;
	
	if( (NULL != pEmptyBuffer) && (NULL != pFillBuffer)){
		if( (NULL != pEmptyBuffer->sTopArea.pTopAddress_Y_Area) &&  (NULL != pFillBuffer->sTopArea.pTopAddress_Y_Area)){
			bBufferCheck = CNV_TRUE;
		} else {
			if( (CNVP_EOS_FLAG == pEmptyBuffer->nFlag) && (NULL != pFillBuffer->sTopArea.pTopAddress_Y_Area)){
				bBufferCheck = CNV_TRUE;
			}
		}
	}
	return bBufferCheck;
}


static CNV_VOID FdpcpCore_SetInputData(
	T_FDP_IMGBUF		*pConvertBuffer,
	CNV_BUFFERHEADER_T 	*pBuffer,
    CNV_BOOL			bInputMode,
    CNV_BOOL			bBottomFirst,
	CNV_U32				nInputFormat)
{
	if( CNV_TRUE == bInputMode ){
		if( CNV_FALSE == bBottomFirst ){
			pConvertBuffer->addr     = pBuffer->sTopArea.pTopAddress_Y_Area;
			if( CNV_YVU420P == nInputFormat){
				pConvertBuffer->addr_c0  = pBuffer->sTopArea.pTopAddress_C_Area1;
				pConvertBuffer->addr_c1  = pBuffer->sTopArea.pTopAddress_C_Area0;
			} else {
				pConvertBuffer->addr_c0  = pBuffer->sTopArea.pTopAddress_C_Area0;
				pConvertBuffer->addr_c1  = pBuffer->sTopArea.pTopAddress_C_Area1;
			}
			pConvertBuffer->stride   = (CNV_U16)pBuffer->sTopArea.nStride_Y_Area;
			pConvertBuffer->stride_c = (CNV_U16)pBuffer->sTopArea.nStride_C_Area;
			pConvertBuffer->height   = (CNV_U16)pBuffer->sTopArea.nHeight_Y_Area;
			pConvertBuffer->height_c = (CNV_U16)pBuffer->sTopArea.nHeight_C_Area;
		} else {
			pConvertBuffer->addr     = pBuffer->sBottomArea.pTopAddress_Y_Area;
			if( CNV_YVU420P == nInputFormat){
				pConvertBuffer->addr_c0  = pBuffer->sBottomArea.pTopAddress_C_Area1;
				pConvertBuffer->addr_c1  = pBuffer->sBottomArea.pTopAddress_C_Area0;
			} else {
				pConvertBuffer->addr_c0  = pBuffer->sBottomArea.pTopAddress_C_Area0;
				pConvertBuffer->addr_c1  = pBuffer->sBottomArea.pTopAddress_C_Area1;
			}
			pConvertBuffer->stride   = (CNV_U16)pBuffer->sBottomArea.nStride_Y_Area;
			pConvertBuffer->stride_c = (CNV_U16)pBuffer->sBottomArea.nStride_C_Area;
			pConvertBuffer->height   = (CNV_U16)pBuffer->sBottomArea.nHeight_Y_Area;
			pConvertBuffer->height_c = (CNV_U16)pBuffer->sBottomArea.nHeight_C_Area;
		}
	} else {
		/* NULL Set*/
		pConvertBuffer->addr     = NULL;
		pConvertBuffer->addr_c0  = NULL;
		pConvertBuffer->addr_c1  = NULL;
		pConvertBuffer->stride   = 0;
		pConvertBuffer->stride_c = 0;
		pConvertBuffer->height   = 0;
		pConvertBuffer->height_c = 0;
	}
	CNV_LOGGER(CNV_LOG_DEBUG,"pConvertBuffer->addr = %x",pConvertBuffer->addr);
}

static CNV_VOID FdpcpCore_SetFieldcf(
	FDP_HANDLE_T  		*pFdpHandle,
	CNV_BOOL			bBottomFirst,
	CNV_BOOL			bInputType)
{
	if( CNV_TRUE == bInputType){
		if(CNV_TRUE == bBottomFirst){
			pFdpHandle->bField = CNV_TRUE;
		} else {
			pFdpHandle->bField = CNV_FALSE;
		}
	} else {
		if(CNV_TRUE == bBottomFirst){
			pFdpHandle->bField = CNV_FALSE;
		} else {
			pFdpHandle->bField = CNV_TRUE;
		}
	}
}

static CNV_VOID FdpcpCore_CreatInputData(
	FDP_HANDLE_T  		*pFdpHandle,
	CNV_BUFFERHEADER_T 	*pFirstEmptyBuffer,
	CNV_BUFFERHEADER_T 	*pSecondEmptyBuffer,
	CNV_BOOL			bBottomFirst,
	CNV_U32				nInputFormat,
	T_FDP_IMGBUF 		*pFutureBuffer,
	T_FDP_IMGBUF 		*pCurrentBuffer,
	T_FDP_IMGBUF 		*pPastBuffer)
{
	switch(pFdpHandle->nConvertMode)
	{
	case(FDP_SEQ_PROG):
		/* Input data */
		CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_PROG",0);
		FdpcpCore_SetInputData(pCurrentBuffer, pFirstEmptyBuffer,CNV_TRUE, bBottomFirst,nInputFormat);
		FdpcpCore_SetInputData(pPastBuffer,   pFirstEmptyBuffer,CNV_FALSE, bBottomFirst,nInputFormat);
		FdpcpCore_SetInputData(pFutureBuffer, pFirstEmptyBuffer,CNV_FALSE, bBottomFirst,nInputFormat);
		FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_TRUE);
		break;
	case(FDP_SEQ_INTERH_2D):
		/* Input data */
		CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_INTERH_2D",0);
		FdpcpCore_SetInputData(pCurrentBuffer, pFirstEmptyBuffer,CNV_TRUE, bBottomFirst,nInputFormat);
		FdpcpCore_SetInputData(pPastBuffer,   pFirstEmptyBuffer,CNV_FALSE, bBottomFirst,nInputFormat);
		FdpcpCore_SetInputData(pFutureBuffer, pFirstEmptyBuffer,CNV_FALSE, bBottomFirst,nInputFormat);
		FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_TRUE);
		break;
	case(FDP_SEQ_INTER_2D):
		if( pFdpHandle->nInputIndex == 0){
			CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_INTER_2D bBottomFirst = %d", (CNV_U32)bBottomFirst);
			FdpcpCore_SetInputData(pCurrentBuffer, pFirstEmptyBuffer,CNV_TRUE, bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pPastBuffer,   pFirstEmptyBuffer,CNV_FALSE, bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pFutureBuffer, pFirstEmptyBuffer,CNV_FALSE, bBottomFirst,nInputFormat);
			FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_TRUE);
		} else {
			CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_INTER_2D bBottomFirst = %d", (CNV_U32)(!bBottomFirst) );
			FdpcpCore_SetInputData(pCurrentBuffer, pFirstEmptyBuffer,CNV_TRUE, !bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pPastBuffer,   pFirstEmptyBuffer,CNV_FALSE, !bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pFutureBuffer, pFirstEmptyBuffer,CNV_FALSE, !bBottomFirst,nInputFormat);
			FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_FALSE);
		}
		pFdpHandle->nInputIndex++;
		break;
	case(FDP_SEQ_INTERH):
		CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_INTERH bBottomFirst = %d", (CNV_U32)bBottomFirst);
		FdpcpCore_SetInputData(pPastBuffer,    pSecondEmptyBuffer,CNV_TRUE, !bBottomFirst,nInputFormat);
		FdpcpCore_SetInputData(pCurrentBuffer, pFirstEmptyBuffer ,CNV_TRUE , bBottomFirst,nInputFormat);
		FdpcpCore_SetInputData(pFutureBuffer,  pFirstEmptyBuffer,CNV_TRUE,  !bBottomFirst,nInputFormat);
		FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_TRUE);		
		pFdpHandle->nInputIndex++;
		break;
	case(FDP_SEQ_INTER):
		if( (pFdpHandle->nInputIndex == 0)){
			CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_INTER bBottomFirst = %d", (CNV_U32)bBottomFirst);
			FdpcpCore_SetInputData(pPastBuffer,    pSecondEmptyBuffer,CNV_TRUE,  !bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pCurrentBuffer, pFirstEmptyBuffer, CNV_TRUE,  bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pFutureBuffer,  pFirstEmptyBuffer, CNV_TRUE,  !bBottomFirst,nInputFormat);
			FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_TRUE);	
			pFdpHandle->nInputIndex++;
			if( CNV_TRUE == pFdpHandle->bFirstInput ){
				 pFdpHandle->bReleaseFlag = CNV_TRUE;
			}
		} else {
			CNV_LOGGER(CNV_LOG_DEBUG,"FDP_SEQ_INTER bBottomFirst = %d", (CNV_U32)(!bBottomFirst) );
			FdpcpCore_SetInputData(pPastBuffer,    pSecondEmptyBuffer    ,CNV_TRUE,  bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pCurrentBuffer, pSecondEmptyBuffer    ,CNV_TRUE,  !bBottomFirst,nInputFormat);
			FdpcpCore_SetInputData(pFutureBuffer,  pFirstEmptyBuffer     ,CNV_TRUE,  bBottomFirst,nInputFormat);
			FdpcpCore_SetFieldcf(pFdpHandle,bBottomFirst,CNV_FALSE);
			pFdpHandle->nInputIndex = 0;
		}
		break;
	default:
		break;
	}
}


static CNV_VOID FdpcpCore_SetReleaseFlag(FDP_HANDLE_T *pFdpHandle)
{
	CNV_PTR pTopAddr     = NULL;
	CNV_PTR pBottomAddr  = NULL;

	pTopAddr			= pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->sTopArea.pTopAddress_Y_Area;
	pBottomAddr			= pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->sBottomArea.pTopAddress_Y_Area;
	
	switch(pFdpHandle->nConvertMode)
	{
	case( FDP_SEQ_PROG ):
		pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE; 
		pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;
		break;
	case( FDP_SEQ_INTERH_2D ):
		pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE; 
		pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;
		break;
	case( FDP_SEQ_INTER_2D ):
		if( CNVPT_2D_FULLRATE_RELASE_INDEX == pFdpHandle->nInputIndex){
			pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE; 
			pFdpHandle->nInputIndex = 0;
		} else {
			/* NULL + EOS */
			if( CNVP_EOS_FLAG == pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag ){
				if( (NULL == pTopAddr) && (NULL == pBottomAddr) ){
					pFdpHandle->bEosFlag = CNV_TRUE;
					pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE; 
					pFdpHandle->nInputIndex = 0;
				}
			}
		}
		pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;
		break;
	case(FDP_SEQ_INTERH):
		if( (NULL != pTopAddr) && (NULL != pBottomAddr) ){
			pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE;
			if(CNVP_EOS_FLAG != pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag){
				pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;				
			}
		} else {
			if( (CNVP_EOS_FLAG == pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag) && (NULL == pTopAddr) ){
				/* NULL + EOS */
				pFdpHandle->bEosFlag = CNV_TRUE;
				pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE;
				pFdpHandle->nInputIndex = 0;
			} else {
				pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;
			}
		}
		break;
	case(FDP_SEQ_INTER):
		if( (NULL != pTopAddr) && (NULL != pBottomAddr) ){
			if( CNV_TRUE == pFdpHandle->bReleaseFlag ){
				pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE;
				pFdpHandle->bReleaseFlag = CNV_FALSE;
			}
			
			if(CNVP_EOS_FLAG == pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag){
				pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag  = CNV_BUFFER_RELEASE;
			}	
			pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;
		} else {
			if( (CNVP_EOS_FLAG == pFdpHandle->sConvertEmptyBuffer[0].pConvertBuffer->nFlag) && (NULL == pTopAddr)){
				/* NULL + EOS */
				pFdpHandle->bEosFlag = CNV_TRUE;
				pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag = CNV_BUFFER_RELEASE;
				pFdpHandle->nInputIndex = 0;
			} else {
				if( CNV_TRUE != pFdpHandle->bEosFlag ){
					pFdpHandle->sConvertFillBuffer.nReleaseFlag     = CNV_BUFFER_RELEASE;
				}
			}
		}
		
		break;
	default:
		break;
	}
}


static CNV_VOID FdpcpCore_SetUserData( CNVP_CONTEXT_T* pCnvpContext, CNV_BUFFERHEADER_T *pCurrentEmptyBuffer, CNV_BUFFERHEADER_T *pPastEmptyBuffer)
{
	FDP_HANDLE_T  		*pFdpHandle;
	CNV_BUFFERHEADER_T *pFillBuffer;
	/* cast */
	pFdpHandle		= (FDP_HANDLE_T*)pCnvpContext->pCnvpDepHandle;	
	pFillBuffer 		 = (CNV_BUFFERHEADER_T *)pFdpHandle->sConvertFillBuffer.pConvertBuffer;
	/* Mode dep */
	switch(pCnvpContext->nConvertMode)
	{
	case(CNV_DEINT_NONE):
		/* Set Release Flag */
		pCnvpContext->sOutputPicInfo.pEmptyUserPointer = (CNV_U32)pCurrentEmptyBuffer->pUserPointer;
		pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_FALSE;
		pFillBuffer->pExtendedInfo = pCurrentEmptyBuffer->pExtendedInfo;
		break;
	case(CNV_DEINT_2D_HALFRATE):
		pCnvpContext->sOutputPicInfo.pEmptyUserPointer = (CNV_U32)pCurrentEmptyBuffer->pUserPointer;
		pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_FALSE;
		pFillBuffer->pExtendedInfo =  pCurrentEmptyBuffer->pExtendedInfo;
		break;
	case(CNV_DEINT_2D_FULLRATE):
		pCnvpContext->sOutputPicInfo.pEmptyUserPointer = (CNV_U32)pCurrentEmptyBuffer->pUserPointer;
		pFillBuffer->pExtendedInfo =  pCurrentEmptyBuffer->pExtendedInfo;
		if(pFdpHandle->nInputIndex == 1){
			pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_TRUE;
		} else {
			pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_FALSE;
		}
		break;
	case(CNV_DEINT_3D_HALFRATE):
		pCnvpContext->sOutputPicInfo.pEmptyUserPointer = (CNV_U32)pCurrentEmptyBuffer->pUserPointer;
		pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_FALSE;
		pFillBuffer->pExtendedInfo =  pCurrentEmptyBuffer->pExtendedInfo;
		break;
	case(CNV_DEINT_3D_FULLRATE):
		if(pFdpHandle->nInputIndex == 1){
			pCnvpContext->sOutputPicInfo.pEmptyUserPointer = (CNV_U32)pCurrentEmptyBuffer->pUserPointer;
			pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_TRUE;
			pFillBuffer->pExtendedInfo = pCurrentEmptyBuffer->pExtendedInfo;
		} else {
			pCnvpContext->sOutputPicInfo.pEmptyUserPointer = (CNV_U32)pPastEmptyBuffer->pUserPointer;
			pCnvpContext->sOutputPicInfo.bEmptyKeepFlag    = CNV_FALSE;
			pFillBuffer->pExtendedInfo = pPastEmptyBuffer->pExtendedInfo;
		}
		break;
	default:
		break;
	}
}


static CNV_U32 FdpcpCore_GetS3CtrlArea(CNV_U32 nArea)
{
	CNV_U32 nSize = CNV_S3_AREA_SIZE;
	CNV_U32 nResult = 0;
	CNV_U32 nDiff;
	CNV_U32 nCnt;
	
	nArea = nArea/CNV_S3_AREA_BYTE;
	
	for(nCnt = 0; nCnt < (CNV_S3_MAX_AREA_COUNT+1) ; nCnt++){
		nDiff = nArea - nSize;
		if( (CNV_S32)nDiff < 0 ){
			nResult = nCnt - 1;
			break;
		} else {
			nSize = nSize * 2;
		}
	}
	return nResult;
}


static CNV_U32 FdpcpCore_GetS3CtrlStride(CNV_U32 nStride)
{
	CNV_U32 nSize = CNV_S3_STRIDE_SIZE;
	CNV_U32 nDiff;
	CNV_U32 nCnt;
	
	for(nCnt = 0; nCnt < CNV_S3_MAX_STRIDE_COUNT ; nCnt++){
		nDiff = nSize - nStride;
		if( (CNV_S32)nDiff < 0 ){
			nSize = nSize << 1;
		} else {
			break;
		}
	}
	return nSize;
}


static CNV_ERRORTYPE FdpcpCore_CheckChangeStride(FDP_HANDLE_T *pFdpHandle, CNV_U32 nStride)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	CNV_S32 s3Result;
	struct S3_PARAM	sS3CtlParam;
	
	(void)Cnvdep_Memset(&sS3CtlParam,0,sizeof(sS3CtlParam));
	if( pFdpHandle->nStride != nStride){
		s3Result = s3ctl_get_param((S3CTL_ID)pFdpHandle->nS3ctrlId, &sS3CtlParam);
		if(R_S3_OK != s3Result){
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_get_param error = %d", (CNV_U32)s3Result);
			eResult= CNV_ERROR_FATAL;
			goto end_point;
		}
		s3Result = s3ctl_clear_param((S3CTL_ID)pFdpHandle->nS3ctrlId);
		if(R_S3_OK != s3Result){
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_clear_param error = %d", (CNV_U32)s3Result);
			eResult= CNV_ERROR_FATAL;
			goto end_point;
		}
		sS3CtlParam.stride = FdpcpCore_GetS3CtrlStride(nStride);
		s3Result = s3ctl_set_param((S3CTL_ID)pFdpHandle->nS3ctrlId,&sS3CtlParam);
		if(R_S3_OK != s3Result){
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_set_param error = %d", (CNV_U32)s3Result);
			eResult= CNV_ERROR_FATAL;
			goto end_point;
		}
		pFdpHandle->nStride;
		CNV_LOGGER(CNV_LOG_DEBUG," Sequence change : Input buffer nStride = %d", nStride);
	}
end_point:
	return eResult;
}


static CNV_ERRORTYPE FdpcpCore_Locks3ctl(FDP_HANDLE_T *pFdpHandle)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	CNV_S32 s3Result;
	CNV_U32 nCnt = 0;
	CNV_BOOL bLoopEnd = CNV_FALSE;
	
	while(nCnt != CNVP_MODULE_TIMEOUT){
		s3Result = s3ctl_lock((S3CTL_ID)pFdpHandle->nS3ctrlId);
		if(R_S3_OK == s3Result){
			bLoopEnd = CNV_TRUE;
		}else if(R_S3_BUSY == s3Result){
			nCnt++;
		}else {
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_lock error = %d", (CNV_U32)s3Result);
			eResult = CNV_ERROR_FATAL;
			bLoopEnd = CNV_TRUE;
		}
		if (bLoopEnd == CNV_TRUE){
			break;
		}
		Cnvdep_Sleep(CNVP_MODULE_WAIT_TIME);
		if(nCnt == CNVP_MODULE_TIMEOUT){
			CNV_LOGGER(CNV_LOG_ERROR,"s3ctl_lock error = %d", (CNV_U32)s3Result);
			eResult = CNV_ERROR_FATAL;
		}
	}
	return eResult;
}


static CNV_BOOL FdpcpCore_CheckLastStart(T_FDP_IMGBUF *pPastBuffer,T_FDP_IMGBUF *pCurrentBuffer,T_FDP_IMGBUF *pFuturebuffer)
{
	CNV_BOOL bResult = CNV_FALSE;
	
	if( pPastBuffer->addr_c0 != NULL ){
		if( pCurrentBuffer->addr_c0 != NULL ){
			if( pFuturebuffer->addr_c0 == NULL ){
				bResult = CNV_TRUE;
			}
		}
	}
	return bResult;
}


static CNV_BOOL FdpcpCore_CheckErrorFlag(CNV_BUFFERHEADER_T *pBuffer)
{
	CNV_BOOL bResult = CNV_FALSE;
	if((pBuffer->nErrorFlag & CNVP_MASK_FLAG) != 0){
		bResult = CNV_TRUE;
	} 
	return bResult;
}


static CNV_BOOL FdpcpCore_SetErrorFlag(CNV_BUFFERHEADER_T *pInputBuffer, CNV_BUFFERHEADER_T *pOutputBuffer )
{
	CNV_BOOL bResult = CNV_FALSE;
	
	pOutputBuffer->nErrorFlag = 0;
	if( FdpcpCore_CheckErrorFlag(pInputBuffer) == CNV_TRUE){
		pOutputBuffer->nErrorFlag = pInputBuffer->nErrorFlag;
		bResult = CNV_TRUE;
	}
	return bResult;
}


static CNV_VOID FdpcpCore_SetPictrueErrorFlag(FDP_HANDLE_T *pFdpHandle, CNV_BUFFERHEADER_T *pCurrentBuffer,CNV_BUFFERHEADER_T *pPastBuffer,CNV_BUFFERHEADER_T *pFillBuffer)
{
	switch(pFdpHandle->nConvertMode)
	{
	case( FDP_SEQ_PROG ):
		if(FdpcpCore_SetErrorFlag(pCurrentBuffer,pFillBuffer) == CNV_TRUE){
			CNV_LOGGER(CNV_LOG_WARN,"%x",pCurrentBuffer->nErrorFlag);
		}
		break;
	case( FDP_SEQ_INTERH_2D ):
		if(FdpcpCore_SetErrorFlag(pCurrentBuffer,pFillBuffer) == CNV_TRUE){
			CNV_LOGGER(CNV_LOG_WARN,"%x",pCurrentBuffer->nErrorFlag);
		}
		break;
	case( FDP_SEQ_INTER_2D ):
		if(FdpcpCore_SetErrorFlag(pCurrentBuffer,pFillBuffer) == CNV_TRUE){
			CNV_LOGGER(CNV_LOG_WARN,"%x",pCurrentBuffer->nErrorFlag);
		}
		break;
	case( FDP_SEQ_INTERH ):
		if(FdpcpCore_SetErrorFlag(pCurrentBuffer,pFillBuffer) == CNV_TRUE){
			CNV_LOGGER(CNV_LOG_WARN,"%x",pCurrentBuffer->nErrorFlag);
		} else {
			if(FdpcpCore_SetErrorFlag(pPastBuffer,pFillBuffer) == CNV_TRUE){
				CNV_LOGGER(CNV_LOG_WARN,"%x",pPastBuffer->nErrorFlag);
			} 	
		}
		break;
	case( FDP_SEQ_INTER ):
		if(FdpcpCore_SetErrorFlag(pCurrentBuffer,pFillBuffer) == CNV_TRUE){
			CNV_LOGGER(CNV_LOG_WARN,"%x",pCurrentBuffer->nErrorFlag);
		} else {
			if(FdpcpCore_SetErrorFlag(pPastBuffer,pFillBuffer) == CNV_TRUE){
				CNV_LOGGER(CNV_LOG_WARN,"%x",pPastBuffer->nErrorFlag);
			}
		}
		break;
	default:
		break;
	}
	return;
}


/* Full Rate Time Stamp Function */
static CNV_TICKS FdpcpCore_CrateTimeStamp(CNV_TICKS nTimeStamp, CNV_U32 nFrameTimeStamp ){
	CNV_TICKS nTimeResult;
#ifndef CNV_SKIP64BIT
	nTimeResult = (CNV_S64)((CNV_U64)nTimeStamp + (CNV_U64)nFrameTimeStamp);
#endif
	return nTimeResult;
}


/* Full Rate Time Stamp Function */
static CNV_VOID FdpcpCore_GetFullRateTime(
	CNV_BUFFERHEADER_T *pBuffer, 
	CNV_U32 *nFrameRateTime)
{
	CNVP_EXTEND_INFORMATION_T *pExtendInfo;
	pExtendInfo = (CNVP_EXTEND_INFORMATION_T*)pBuffer->pExtendedInfo;
	
	if(NULL != pExtendInfo){
		if(pExtendInfo->struct_size != sizeof(CNVP_EXTEND_INFORMATION_T)){
			/* Set default Time Rate(Full Rate) */
			*nFrameRateTime = CNVP_TIME_DEFAULT;
		} else {
			switch(pExtendInfo->frame_rate_code){
			case(CNVP_FRAME_RATE_CODE_FORBIDDEN):
				/* Set default Frame Rate(Full Rate) */
				*nFrameRateTime = CNVP_TIME_DEFAULT;
				break;
			case(CNVP_FRAME_RATE_CODE_23_976):
				*nFrameRateTime = CNVP_TIME_RATE_23_976;
				break;
			case(CNVP_FRAME_RATE_CODE_24):
				*nFrameRateTime = CNVP_TIME_RATE_24;
				break;
			case(CNVP_FRAME_RATE_CODE_25):
				*nFrameRateTime = CNVP_TIME_RATE_25;
				break;
			case(CNVP_FRAME_RATE_CODE_29_97):
				*nFrameRateTime = CNVP_TIME_RATE_29_97;
				break;
			case(CNVP_FRAME_RATE_CODE_30):
				*nFrameRateTime = CNVP_TIME_RATE_30;
				break;
			case(CNVP_FRAME_RATE_CODE_50):
				*nFrameRateTime = CNVP_TIME_RATE_50;
				break;
			case(CNVP_FRAME_RATE_CODE_59_94):
				*nFrameRateTime = CNVP_TIME_RATE_59_94;
				break;
			case(CNVP_FRAME_RATE_CODE_60):
				*nFrameRateTime = CNVP_TIME_RATE_60;
				break;
			case(CNVP_FRAME_RATE_CODE_EXTENDED):
				if((pExtendInfo->frame_rate_n == 0) || (pExtendInfo->frame_rate_d == 0)){
					*nFrameRateTime = CNVP_TIME_DEFAULT;
				}else{
					*nFrameRateTime = (pExtendInfo->frame_rate_d*CNVP_TIME_OFFSET)/pExtendInfo->frame_rate_n/2;
				}
				break;
			default:
				break;
			}
		}
	} else {
		*nFrameRateTime = CNVP_TIME_DEFAULT;
	}
	return;
}


static CNV_VOID FdpcpCore_SetTimeStamp(FDP_HANDLE_T *pFdpHandle, CNV_BUFFERHEADER_T *pCurrentBuffer,CNV_BUFFERHEADER_T *pPastBuffer, CNV_BUFFERHEADER_T *pFillBuffer)
{
	CNV_U32 nBufferReleaseFlag;
	CNV_U32 nAddTimeStamp = 0;
	
	nBufferReleaseFlag = pFdpHandle->sConvertEmptyBuffer[0].nReleaseFlag;
	
	switch(pFdpHandle->nConvertMode)
	{
	case( FDP_SEQ_PROG ):
		pFillBuffer->nTimeStamp = pCurrentBuffer->nTimeStamp;
		break;
	case( FDP_SEQ_INTERH_2D ):
		pFillBuffer->nTimeStamp = pCurrentBuffer->nTimeStamp;
		break;
	case( FDP_SEQ_INTER_2D ):
		if(nBufferReleaseFlag == CNV_BUFFER_KEEP){
			pFillBuffer->nTimeStamp = pCurrentBuffer->nTimeStamp;
		}else{
			FdpcpCore_GetFullRateTime(pCurrentBuffer,&nAddTimeStamp);
			pFillBuffer->nTimeStamp = FdpcpCore_CrateTimeStamp(pCurrentBuffer->nTimeStamp,nAddTimeStamp);
		}
		break;
	case( FDP_SEQ_INTERH ):
		pFillBuffer->nTimeStamp = pCurrentBuffer->nTimeStamp;
		break;
	case( FDP_SEQ_INTER ):
		if(nBufferReleaseFlag == CNV_BUFFER_KEEP){
			FdpcpCore_GetFullRateTime(pPastBuffer,&nAddTimeStamp);
			pFillBuffer->nTimeStamp = FdpcpCore_CrateTimeStamp(pPastBuffer->nTimeStamp,nAddTimeStamp);
		}else{
			pFillBuffer->nTimeStamp = pCurrentBuffer->nTimeStamp;
		}
		break;
	default:
		break;
	}
	return;
}
