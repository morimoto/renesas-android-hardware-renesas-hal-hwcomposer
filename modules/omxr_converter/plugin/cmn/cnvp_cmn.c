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
 * OMX Converter plugin common function
 *
 * \file cnvp_cmn.c
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "cnv_type.h"

#include "cnvp_cmn.h"
#include "cnv_osdep.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/**
* The CnvpCnm_ConvertThread is main thread function.
*
* \param[in] pHandle          converter handle
* \param[in] nEventId         event id
* \param[in] pData            common context
* \return none
*/
static CNV_VOID CnvpCnm_ConvertThread( CNV_PTR pHandle, CNV_U32 nEventId, CNV_PTR pData );

/**
* The CnvpCmn_CheckCommand will check a thread command.
*
* \param[in] pCnvpCmnContext  common context
* \param[in] eCommand         command id
* \return CNV_BOOL
* \retval CNV_TRUE            command OK
* \retval CNV_FALSE           this command can not use this state.
*/
static CNV_BOOL CnvpCmn_CheckCommand( CNVP_CONTEXT_T* pCnvpCmnContext, CNV_U32 eCommand);

/**
* The CnvpCmn_CheckCommand will set a thread state.
*
* \param[in] pCnvpCmnContext  common context
* \param[in] eCommand         thread state
* \return none
*/
static CNV_VOID CnvpCmn_SetState( CNVP_CONTEXT_T* pCnvpCmnContext, CNV_U32 eState);

/**
* The CnvpCmn_CheckCommand will call event callback functions.
*
* \param[in] pCnvpCmnContext  common context
* \param[in] eEventId         event id
* \param[in] eResult          sub module return
* \param[in] nSubErrorCode    sub error code
* \return none
*/
static CNV_VOID CnvpCmn_EventDone(CNVP_CONTEXT_T* pCnvpContext,CNV_CALLBACK_EVENTTYPE eEventId, CNV_ERRORTYPE eResult, CNV_SUBERRORTYPE nSubErrorCode);

/**
* The CnvpCmn_CheckCommand will set timeout mode.
*
* \param[in] pCnvpCmnContext  common context
* \return none
*/
static CNV_VOID CnvpCmv_SetTimeout(CNVP_CONTEXT_T *pCnvpContext);

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
CNV_ERRORTYPE CnvpCmn_CreateHandle(
	CNV_PTR			 *pCnvpCmnHandle)
{
	CNVP_CMN_HANDLE_T		*pHandle;
	
	pHandle = (CNVP_CMN_HANDLE_T *)Cnvdep_Malloc(sizeof(CNVP_CMN_HANDLE_T));
	if( NULL == pHandle ){
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	/* Set handle */
	*pCnvpCmnHandle = (CNV_PTR)pHandle;
	
	return CNV_ERROR_NONE;
}


CNV_ERRORTYPE CnvpCmn_CreateCnvertThread( CNV_PTR param , CNV_SUBERRORTYPE *pSubErrorCode)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE; 
	CNVP_CONTEXT_T *pCnvpContext;
	
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE);
	if( NULL == param){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_PARAM_HANDLE);
		return CNV_ERROR_INVALID_PARAMETER;
	}
	
	pCnvpContext = (CNVP_CONTEXT_T*)param;
	
	eResult = Cnvdep_CreateOsDepHandle( &pCnvpContext->nThreadId);
	if( CNV_ERROR_NONE != eResult ){
		CNV_LOGGER(CNV_LOG_ERROR,"Cnvdep_CreateOsDepHandle ID = %p",pCnvpContext->nThreadId);
	} else {
		/* Convert Thread */
		eResult = Cnvdep_CreateThread( pCnvpContext->nThreadId, &CnvpCnm_ConvertThread, param );
		CNV_LOGGER(CNV_LOG_DEBUG,"CnvpCmnCreateCnvertThread ID = %p",pCnvpContext->nThreadId);
		if( CNV_ERROR_NONE != eResult){
			CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_ABEND);	
		}
	}
	return eResult;
}


CNV_ERRORTYPE CnvpCmn_ThreadJoin( CNV_PTR param, CNV_SUBERRORTYPE *pSubErrorCode)
{
	CNV_U32 iret = 0;
	CNVP_CONTEXT_T *pCnvpContext;
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE);
	if( NULL == param){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_PARAM);
		return CNV_ERROR_INVALID_PARAMETER;
	}
	pCnvpContext = (CNVP_CONTEXT_T*)param;
	
	iret = Cnvdep_ThreadJoin(pCnvpContext->nThreadId , NULL );
	if( 0 != iret ){
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_ABEND);
	}
	return CNV_ERROR_NONE;
}


CNV_ERRORTYPE CnvpCmn_SendEvent( CNV_PTR param, CNV_U32 eEventId, CNV_PTR pData, CNV_SUBERRORTYPE *pSubErrorCode)
{
	CNV_ERRORTYPE eResult;
	CNVP_CONTEXT_T *pCnvpContext;
	
	CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_SUBERROR_NONE);
	if( NULL == param){
		return CNV_ERROR_INVALID_PARAMETER;
	}
	/* Send Event */
	pCnvpContext = (CNVP_CONTEXT_T*)param;
	eResult = Cnvdep_SendEvent(pCnvpContext->nThreadId, eEventId, pData);
	if( CNV_ERROR_NONE != eResult){
		CNV_LOGGER(CNV_LOG_ERROR,"eResult = %d ", (CNV_U32)eResult);
		CnvpCmn_SetSubErrorCode( pSubErrorCode, CNV_ERROR_SUBMODULE_ABEND);
	}
	
	return eResult;
}


CNV_VOID CnvpCmn_SetSubErrorCode( CNV_SUBERRORTYPE 	*pSubErrorCode,CNV_SUBERRORTYPE eErrorCode)
{
	if( NULL != pSubErrorCode ){
		*pSubErrorCode = eErrorCode;
	}
}


CNV_ERRORTYPE CnvpCmn_AddBufferToQueue(
	CNVP_CMN_HANDLE_T *pHandle,
	 CNV_U32 		 eEventId,
	CNV_BUFFERHEADER_T *pBuffer)
{
    CNV_U32 nIndex = 0;
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	CNV_BOOL bCheck = CNV_TRUE;
	CNV_BUFFER_QUEUE_T *pInputQueue;
	
	switch( eEventId ){
	case(CNV_EVENT_EMPTYBUFFER):
		CNV_LOGGER(CNV_LOG_DEBUG,"Add input buffer = %x",(CNV_U32)pBuffer);
		pInputQueue = &pHandle->sEmptyBufferQueue;
		break;
	case(CNV_EVENT_FILLBUFFER):
		CNV_LOGGER(CNV_LOG_DEBUG,"Add output buffer = %x",(CNV_U32)pBuffer);	
		pInputQueue = &pHandle->sFillBufferQueue;
		(void)Cnvdep_Memset(&pBuffer->sOutputConvertInfo,0,sizeof(CNV_OUTPUT_PIC_INFO_T));
		break;
	default:
		bCheck = CNV_FALSE;
		break;
	}
	/* Enqueue */
	if( (CNV_FALSE != bCheck) && (pInputQueue->nCountOfBuffers < CNV_COUNT_QUEUE_ELEMENTS) ){
		nIndex = pInputQueue->nLastElementIndex;
		/* Set Input Data */
		CNV_LOGGER(CNV_LOG_DEBUG,"Add buffer Yaddr %x",(CNV_U32)pBuffer->sTopArea.pTopAddress_Y_Area);
		(void)Cnvdep_Memcpy(&pInputQueue->sBuffer[nIndex], pBuffer,sizeof(CNV_BUFFERHEADER_T));
		Cnvdep_BufferFree((CNV_PTR)pBuffer);
		pInputQueue->nCountOfBuffers++;
		nIndex++;
    	if(nIndex < (CNV_COUNT_QUEUE_ELEMENTS-1) ){
    		pInputQueue->nLastElementIndex++;
    	} else {
    		pInputQueue->nLastElementIndex = 0;
    	}
	} else {
		if( pInputQueue->nCountOfBuffers > (CNV_COUNT_QUEUE_ELEMENTS-1) ){
			eResult = CNV_ERROR_FATAL;
			Cnvdep_BufferFree((CNV_PTR)pBuffer);
		} else {
			eResult = CNV_ERROR_NONE;
		}
	}
	
    return eResult;
}


CNV_ERRORTYPE CnvpCmn_RemoveBufferToQueue(
	CNVP_CMN_HANDLE_T *pHandle,
	CNV_U32 		  eEventId,
	CNVP_CONVERT_BUFFER_T *pConvertData)
{
    CNV_U32 nIndex = 0;
	CNV_BOOL bCheck = CNV_TRUE;
	CNV_BUFFER_QUEUE_T *pOutputQuele;
	
	if( CNV_BUFFER_KEEP == pConvertData->nReleaseFlag ){
		return CNV_ERROR_NONE;
	}
	
	switch( eEventId ){
	case(CNV_EVENT_EMPTYBUFFER):
		CNV_LOGGER(CNV_LOG_DEBUG,"Remove input buffer",0);
		pOutputQuele = &pHandle->sEmptyBufferQueue;
		break;
	case(CNV_EVENT_FILLBUFFER):
		CNV_LOGGER(CNV_LOG_DEBUG,"Remove input buffer",0);
		pOutputQuele = &pHandle->sFillBufferQueue;
		break;
	default:
		bCheck = CNV_FALSE;
		break;
	}
	
	/* dequeue */
	if( (CNV_FALSE != bCheck) && (pOutputQuele->nCountOfBuffers != 0)){
		nIndex = pOutputQuele->nFirstElementIndex;
		(void)Cnvdep_Memcpy(pConvertData->pConvertBuffer ,&pOutputQuele->sBuffer[nIndex],sizeof(CNV_BUFFERHEADER_T));
		(void)Cnvdep_Memset(&pOutputQuele->sBuffer[nIndex],0,sizeof(CNV_BUFFERHEADER_T));
		pConvertData->nReleaseFlag   = CNV_BUFFER_KEEP;

		CNV_LOGGER(CNV_LOG_DEBUG,"Remove buffer Yaddr = %x",(CNV_U32)pConvertData->pConvertBuffer->sTopArea.pTopAddress_Y_Area);
		
		pOutputQuele->nCountOfBuffers--;
		nIndex++;		
    	if( nIndex < (CNV_COUNT_QUEUE_ELEMENTS-1) ){
    		pOutputQuele->nFirstElementIndex++;
    	} else {
    		pOutputQuele->nFirstElementIndex = 0;
    	}
	} else {
		;
	}
    return CNV_ERROR_NONE;
}


static CNV_BOOL CnvpCmn_CheckCommand( CNVP_CONTEXT_T* pCnvpCmnContext, CNV_U32 eCommand)
{
	CNV_U32  nNowState;
	CNV_BOOL bResult = CNV_FALSE;
	
	nNowState = pCnvpCmnContext->nThreadState;
	
	switch( eCommand )
	{
	case( CNV_COMMAND_NONE ):
		/* Thread Fatal */
		break;
	case( CNV_COMMAND_CONVERT ):
		if( (CNVP_STATE_IDLE == nNowState) && (CNVP_STATE_FATAL != nNowState) ){
			bResult = CNV_TRUE;
		}
		break;
	case( CNV_COMMAND_DONE ):
		if( ((CNVP_STATE_EXECUTING == nNowState) || (CNVP_STATE_WAITING == nNowState)) && (CNVP_STATE_FATAL != nNowState) ){
			bResult = CNV_TRUE;
		}
		break;
	case( CNV_COMMAND_FLUSHING ):
		if( (CNVP_STATE_IDLE == nNowState) && (CNVP_STATE_FATAL != nNowState) ){
			bResult = CNV_TRUE;
		}
		break;
	case( CNV_COMMAND_CLOSE ):
		if( (CNVP_STATE_IDLE == nNowState ) || (CNVP_STATE_FATAL == nNowState) ){
			bResult = CNV_TRUE;
		}
		break;
	case( CNV_COMMAND_WAITING ):
		if( CNVP_STATE_WAITING == nNowState ){
			bResult = CNV_TRUE;
		}
		break;
	case( CNV_COMMAND_TIMEOUT ):
		if( CNVP_STATE_WAITING == nNowState ){
			bResult = CNV_TRUE;
		}
		break;
	default:
		break;
	}
	return bResult;
}


static CNV_VOID CnvpCmn_SetState( CNVP_CONTEXT_T* pCnvpCmnContext, CNV_U32 eState)
{
	pCnvpCmnContext->nThreadState = eState;
	return;
}

static CNV_VOID CnvpCmn_EventDone(CNVP_CONTEXT_T* pCnvpContext,CNV_CALLBACK_EVENTTYPE eEventId, CNV_ERRORTYPE eResult, CNV_SUBERRORTYPE nSubErrorCode)
{
	CNV_PTR pUserPtr;
	CNV_PTR pLocalParam;
	
	/* Callback parameter */
	pLocalParam = pCnvpContext->sPluginlocalEvent.hCnvHandle;
	pUserPtr    = pCnvpContext->pUserPointer;
	
	if(pCnvpContext->bFatalFlag == CNV_FALSE){
		switch(eEventId)
		{
		case(CNV_CALLBACK_EVENT_FATAL):
			pCnvpContext->bFatalFlag = CNV_TRUE;
			CnvpCmn_SetState( pCnvpContext, CNVP_STATE_FATAL);
			CNV_LOGGER(CNV_LOG_ERROR,"CNV_EVENT_FATAL nSubErrorCode = %x",(CNV_U32)nSubErrorCode);
			break;
		default:
			break;
		}
		/* local Event */
		pCnvpContext->sPluginlocalEvent.PlugIn_CallBackEvent(pLocalParam,eEventId);
		/* callback Event */
		pCnvpContext->sCallbackFunc.CNV_EventDone(pUserPtr,(CNV_U32)eEventId,eResult,nSubErrorCode);
	}
	return;
}


static CNV_VOID CnvpCmv_SetTimeout(CNVP_CONTEXT_T *pCnvpContext)
{
	CNV_PTR  pExtParam;
	CNV_BOOL bResult;
	
	OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T *pTimeParam;
	
	/* Default */
	pCnvpContext->sTimeoutInfo.nWaitTime  = CNVP_THREAD_WAIT_TIME;
	pCnvpContext->sTimeoutInfo.nWaitCount = 0;
	pCnvpContext->sTimeoutInfo.nTimeout   = CNVP_THREAD_TIMEOUT;
	
	pCnvpContext->bDisableTimeout = CNV_FALSE;
	
	/* check set parameter */
	bResult = pCnvpContext->sExtensionInfo.CNVP_GetExtensionParameter(
		pCnvpContext->sExtensionInfo.pParameterList,
		"OMX.RENESAS.INDEX.PARAM.VIDEO.CONVERTER.TIMEOUT",
		&pExtParam);
	
	if(CNV_TRUE == bResult){
		pTimeParam = (OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T*)pExtParam;
		if(pTimeParam->sHeader.nStructSize == sizeof(OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T)){
			if( (0 != pTimeParam->nWaitTime) && (pTimeParam->nWaitTime < CNVP_THREAD_WAIT_TIME) ){
				pCnvpContext->sTimeoutInfo.nWaitTime = pTimeParam->nWaitTime;
			}
			if( 0 == pTimeParam->nTimeout ){
				pCnvpContext->bDisableTimeout = CNV_TRUE;
			} else {
				if( (pTimeParam->nTimeout - CNVP_MODULE_TIMEOUT) > 0 ){
					pCnvpContext->sTimeoutInfo.nTimeout   =  pTimeParam->nTimeout - CNVP_MODULE_TIMEOUT;
				} else {
					pCnvpContext->sTimeoutInfo.nTimeout   =  CNVP_MODULE_TIMEOUT;
				}
			}
		}
	}
	CNV_LOGGER(CNV_LOG_DEBUG,"Timeout disable = %x",(CNV_U32)pCnvpContext->bDisableTimeout);
	return;
}

static CNV_VOID CnvpCnm_ConvertThread( CNV_PTR pHandle, CNV_U32 nEventId, CNV_PTR pData  )
{
	CNV_U32			  nCnt;
	CNV_ERRORTYPE     eResult = CNV_ERROR_NONE;
	CNVP_CONTEXT_T    *pCnvpContext;
	CNVP_CMN_HANDLE_T *pCnvpCmnHdl;
	
	/* Flag */
	CNV_BOOL		  bThreadExit 	= CNV_FALSE;
	
	/* return */
	CNV_BOOL		  bResult 		= CNV_FALSE;
	CNV_BOOL		  bConvertFlag 	= CNV_FALSE;
	CNV_BOOL		  bContinueFlag = CNV_FALSE;
	CNV_U32			  eCommand;
	
	CNV_BUFFERHEADER_T *psEmptyBufferDone = NULL;
	CNV_BUFFERHEADER_T *psFillBufferDone  = NULL;
	
	CNV_BUFFERHEADER_T sInputBufferDone;
	CNV_BUFFERHEADER_T sOutputBufferDone;
	
	CNVP_CONVERT_BUFFER_T sQueueEmptyBufferData;
	CNVP_CONVERT_BUFFER_T sQueueFillBufferData;

	pCnvpContext = (CNVP_CONTEXT_T*)pHandle;
	pCnvpCmnHdl  = (CNVP_CMN_HANDLE_T*)pCnvpContext->pCnvpCmnHandle;
	
	if( 0xFFFFFFFFUL == nEventId){
		/* Context Init */
		pCnvpContext->nThreadState = CNVP_STATE_IDLE;
		pCnvpContext->bFlushFlag   = CNV_FALSE;	
		pCnvpContext->bCloseFlag   = CNV_FALSE;
		pCnvpContext->bFatalFlag   = CNV_FALSE;
		pCnvpContext->bModuleInit  = CNV_FALSE;
		
		/* Convert Info */
		pCnvpContext->sOutputPicInfo.bFrameExtended			= CNV_FALSE;
		pCnvpContext->sOutputPicInfo.nFrameExtendedNum		= 0;
		pCnvpContext->sOutputPicInfo.pEmptyUserPointer		= 0;
		pCnvpContext->sOutputPicInfo.bEmptyKeepFlag			= CNV_FALSE;
		
		/* EmptyBuffer */
		pCnvpCmnHdl->sEmptyBufferQueue.nQueueSize 			= CNV_COUNT_QUEUE_ELEMENTS;
		pCnvpCmnHdl->sEmptyBufferQueue.nFirstElementIndex 	= 0;
		pCnvpCmnHdl->sEmptyBufferQueue.nLastElementIndex 	= 0;
		pCnvpCmnHdl->sEmptyBufferQueue.nCountOfBuffers		= 0;

		/* FillBuffer */
		pCnvpCmnHdl->sFillBufferQueue.nQueueSize 			= CNV_COUNT_QUEUE_ELEMENTS;
		pCnvpCmnHdl->sFillBufferQueue.nFirstElementIndex 	= 0;
		pCnvpCmnHdl->sFillBufferQueue.nLastElementIndex 	= 0;
		pCnvpCmnHdl->sFillBufferQueue.nCountOfBuffers		= 0;	
		
		for( nCnt = 0; nCnt < (CNV_COUNT_QUEUE_ELEMENTS - 1); nCnt++ ){
			pCnvpCmnHdl->sEmptyBufferQueue.pBuffer[nCnt]	= 	NULL;
			pCnvpCmnHdl->sFillBufferQueue.pBuffer[nCnt]		= 	NULL;
		}
		/* Timeout Init*/
		CnvpCmv_SetTimeout(pCnvpContext);
		
		(void)Cnvdep_SaveTreadContext(pCnvpContext->nThreadId,(CNV_PTR)pCnvpContext->nThreadId);
	}
	
	CNV_LOGGER(CNV_LOG_DEBUG,"TREAD START EVENT = %x",(CNV_U32)nEventId);

	while( CNV_TRUE != bThreadExit ){
		
		if( CNV_TRUE == bContinueFlag) {
			/* Convert continue */
			if ( CNV_TRUE == pCnvpContext->bFlushFlag ){
				nEventId = CNV_EVENT_FLUSHING;
			} else {
				if ( CNV_TRUE == pCnvpContext->bCloseFlag ){
					nEventId = CNV_EVENT_CLOSE;
				} else {
					nEventId = CNV_EVENT_NONE;
				}
			}
		}

		switch( nEventId )
		{
		case(CNV_EVENT_EMPTYBUFFER):
			/* Set Queue data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_EMPTYBUFFER EVENT = %x",(CNV_U32)nEventId);
			eCommand = CNV_COMMAND_CONVERT;
			
			/* Module Init */
			if( CNV_FALSE == pCnvpContext->bModuleInit ){
				eResult = pCnvpContext->sPluginFunc.CNVP_ModuleInit(pCnvpContext, (CNV_PTR*)pData);
				if( CNV_ERROR_NONE != eResult ){
					CNV_LOGGER(CNV_LOG_ERROR,"FATAL CNV_EVENT_EMPTYBUFFER EVENT = %x",(CNV_U32)nEventId);
					CnvpCmn_EventDone(pCnvpContext,CNV_CALLBACK_EVENT_FATAL,CNV_ERROR_FATAL,CNV_ERROR_SUBMODULE_IP);
				}
				pCnvpContext->bModuleInit = CNV_TRUE;
			}
			/* Add Queue */
			eResult = CnvpCmn_AddBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER,(CNV_BUFFERHEADER_T*)pData );
			if(CNV_ERROR_NONE != eResult){
				CNV_LOGGER(CNV_LOG_ERROR,"FATAL CNV_EVENT_EMPTYBUFFER EVENT = %x",(CNV_U32)CNV_ERROR_SUBMODULE_ABEND);
				CnvpCmn_EventDone(pCnvpContext,CNV_CALLBACK_EVENT_FATAL,eResult,CNV_ERROR_SUBMODULE_ABEND);
				eCommand = CNV_COMMAND_NONE;	
			}
			break;	
		case(CNV_EVENT_FILLBUFFER):
			/* Set Queue data */
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_FILLBUFFER EVENT = %x",(CNV_U32)nEventId);
			eCommand = CNV_COMMAND_CONVERT;

			/* Add Queue */
			eResult = CnvpCmn_AddBufferToQueue( pCnvpCmnHdl, CNV_EVENT_FILLBUFFER,(CNV_BUFFERHEADER_T*)pData );
			if(CNV_ERROR_NONE != eResult){
				CNV_LOGGER(CNV_LOG_ERROR,"FATAL CNV_EVENT_EMPTYBUFFER EVENT = %x",(CNV_U32)nEventId);
				CnvpCmn_EventDone(pCnvpContext,CNV_CALLBACK_EVENT_FATAL,eResult,CNV_ERROR_SUBMODULE_ABEND);
				eCommand = CNV_COMMAND_NONE;
			}
			break;
		case(CNV_EVENT_DONE):
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_DONE EVENT = %x",(CNV_U32)nEventId);
			eCommand = CNV_COMMAND_DONE;
			break;
		case(CNV_EVENT_CLOSE):
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_CLOSE EVENT = %x",(CNV_U32)nEventId);
			pCnvpContext->bCloseFlag = CNV_TRUE;
			/* Close */
			eCommand = CNV_COMMAND_CLOSE;
			break;
		case(CNV_EVENT_FLUSHING):
			CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_FLUSHING EVENT = %x",(CNV_U32)nEventId);
			pCnvpContext->bFlushFlag = CNV_TRUE;
			/* FLUSH */
			eCommand = CNV_COMMAND_FLUSHING;
			break;
		case(CNV_EVENT_FATAL):
			/* FATAL */
			eCommand = CNV_COMMAND_NONE;
			CNV_LOGGER(CNV_LOG_ERROR,"FATAL EVENT eCommand EVENT = %x",(CNV_U32)nEventId);
			CnvpCmn_EventDone(pCnvpContext,CNV_CALLBACK_EVENT_FATAL,eResult,CNV_ERROR_SUBMODULE_ABEND);
			break;
		case(CNV_EVENT_WAITING):
			CNV_LOGGER(CNV_LOG_WARN,"EVENT_WAITING eCommand EVENT = %x",(CNV_U32)nEventId);
			eCommand = CNV_COMMAND_NONE;
			if(CNVP_STATE_WAITING == pCnvpContext->nThreadState){
				if(pCnvpContext->sTimeoutInfo.nTimeout == pCnvpContext->sTimeoutInfo.nWaitCount){
					eCommand = CNV_COMMAND_TIMEOUT;
				} else {
					if( 0 != pCnvpContext->sTimeoutInfo.nWaitCount ){
						eCommand = CNV_COMMAND_WAITING;
					}
				}
			}
			break;
		case(CNV_EVENT_TIMEOUT):
			/* TIMEOUT Event */
			if(CNVP_STATE_EXECUTING == pCnvpContext->nThreadState){
				eCommand = CNV_COMMAND_WAITING;
				CNV_LOGGER(CNV_LOG_WARN,"TIMEOUT EVENT eCommand EVENT = %x",(CNV_U32)nEventId);
				CnvpCmn_SetState( pCnvpContext, CNVP_STATE_WAITING );
			} else {
				eCommand = CNV_COMMAND_NONE;
			}
			break;
		case(CNV_EVENT_NONE):
			eCommand = CNV_COMMAND_CONVERT;
			break;
		case(CNV_EVENT_INIT):
			eCommand = CNV_COMMAND_NONE;
			break;
		default:
			break;
		}
		
		/* Check command */
		bResult =  CnvpCmn_CheckCommand( pCnvpContext, eCommand );
		if( CNV_TRUE == bResult ){
			switch (eCommand)
			{
			case( CNV_COMMAND_CONVERT ):
				/* Converter Executing */
				CNV_LOGGER(CNV_LOG_DEBUG,"CNV_COMMAND_CONVERT START thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
				bConvertFlag = pCnvpContext->sPluginFunc.CNVP_CheckConvertStart((CNV_PTR)pCnvpContext);
				if( (CNV_TRUE == bConvertFlag) && (CNVP_STATE_IDLE == pCnvpContext->nThreadState) ){
					CNV_LOGGER(CNV_LOG_DEBUG,"bConvertFlag = %x",(CNV_U32)bConvertFlag);	
					CnvpCmn_SetState( pCnvpContext, CNVP_STATE_EXECUTING);
					
					eResult = pCnvpContext->sPluginFunc.CNVP_ConverterExecuting((CNV_PTR)pCnvpContext,bConvertFlag,NULL);
					if( eResult == CNV_ERROR_FATAL ){
						CNV_LOGGER(CNV_LOG_ERROR,"FATAL COMMAND ID = %x",(CNV_U32)CNV_COMMAND_CONVERT);
						CnvpCmn_EventDone(pCnvpContext,CNV_CALLBACK_EVENT_FATAL,eResult,CNV_ERROR_SUBMODULE_ABEND);
					}
					bContinueFlag = CNV_FALSE;
				} else {
					CNV_LOGGER(CNV_LOG_DEBUG,"CONVERETER NO START bConvertFlag = %x",(CNV_U32)bContinueFlag);	
					bContinueFlag = CNV_FALSE;
				}
				break;
			case( CNV_COMMAND_DONE ):
				CNV_LOGGER(CNV_LOG_DEBUG,"CNV_COMMAND_DONE START Thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
				
				pCnvpContext->sPluginFunc.CNVP_CheckReleaseBuffer((CNV_PTR)pCnvpContext, (CNV_PTR*)&psEmptyBufferDone, (CNV_PTR*)&psFillBufferDone );
				
				/* Callback */
				if( NULL != psEmptyBufferDone){
					CNV_LOGGER(CNV_LOG_INFO, "CNV_EmptyBufferDone",0);
					pCnvpContext->sCallbackFunc.CNV_EmptyBufferDone(psEmptyBufferDone,pCnvpContext->pUserPointer,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
					(void)Cnvdep_Memset(psEmptyBufferDone,0,sizeof(CNV_BUFFERHEADER_T));
				}
				
				if ( NULL != psFillBufferDone){
					/* Set to context info */
					CNV_LOGGER(CNV_LOG_INFO, "CNV_FillBufferDone",0);
					psFillBufferDone->sOutputConvertInfo = pCnvpContext->sOutputPicInfo;
					pCnvpContext->sCallbackFunc.CNV_FillBufferDone(psFillBufferDone,pCnvpContext->pUserPointer,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
					(void)Cnvdep_Memset(psFillBufferDone,0,sizeof(CNV_BUFFERHEADER_T));
				}
				/* Set state */
				CnvpCmn_SetState( pCnvpContext, CNVP_STATE_IDLE);
				
				/* Delete Buffer */
				psEmptyBufferDone = NULL;
				psFillBufferDone  = NULL;
				
				/* Check to continue */
				bConvertFlag = pCnvpContext->sPluginFunc.CNVP_CheckConvertStart((CNV_PTR)pCnvpContext);
				if( CNV_TRUE == bConvertFlag ){
					/* Continue */
					CNV_LOGGER(CNV_LOG_DEBUG,"Continue",0);
					bContinueFlag = CNV_TRUE;
				} else {
					/* No continue */
					bContinueFlag = CNV_FALSE;	
					CNV_LOGGER(CNV_LOG_DEBUG,"CNV_COMMAND_DONE END thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
					if( (CNV_TRUE == pCnvpContext->bFlushFlag) || (CNV_TRUE == pCnvpContext->bCloseFlag) ){
						bContinueFlag = CNV_TRUE;
					} else {
						bContinueFlag = CNV_FALSE;
					}
				}
				/* reset */
				pCnvpContext->sTimeoutInfo.nWaitCount = 0;
				
				break;
				
			case( CNV_COMMAND_FLUSHING ):
				CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_FLUSHING START thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
				/* local data */
				for( nCnt = 0; nCnt < (CNV_COUNT_QUEUE_ELEMENTS-1); nCnt++ )
				{
					pCnvpContext->sPluginFunc.CNVP_GetFlushBuffer((CNV_PTR)pCnvpContext, (CNV_PTR)&psEmptyBufferDone, (CNV_PTR)&psFillBufferDone );
					/* EmptyBuffer Done */
					if( NULL != psEmptyBufferDone){
						CNV_LOGGER(CNV_LOG_INFO, "FLUSH CNV_EmptyBufferDone",0);
						pCnvpContext->sCallbackFunc.CNV_EmptyBufferDone(psEmptyBufferDone,pCnvpContext->pUserPointer,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
						(void)Cnvdep_Memset(psEmptyBufferDone,0,sizeof(CNV_BUFFERHEADER_T));
					}
					
					/* FillBuffer Done */
					if ( NULL != psFillBufferDone){
						CNV_LOGGER(CNV_LOG_INFO, "FLUSH CNV_FillBufferDone",0);
						pCnvpContext->sCallbackFunc.CNV_FillBufferDone(psFillBufferDone,pCnvpContext->pUserPointer,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
						(void)Cnvdep_Memset(psFillBufferDone,0,sizeof(CNV_BUFFERHEADER_T));
					}
					if ( (NULL == psEmptyBufferDone) && (NULL == psFillBufferDone) ){
						break;
					}
				}
				
				/* Queue data */
				for( nCnt = 0; nCnt < (CNV_COUNT_QUEUE_ELEMENTS-1); nCnt++ )
				{
					sQueueEmptyBufferData.pConvertBuffer 	= &sInputBufferDone;
					sQueueEmptyBufferData.nReleaseFlag 		= CNV_BUFFER_NONE;
					sQueueFillBufferData.pConvertBuffer 	= &sOutputBufferDone;
					sQueueFillBufferData.nReleaseFlag 		= CNV_BUFFER_NONE;
					/* Get Queue Data */
					(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_EMPTYBUFFER, &sQueueEmptyBufferData);
					(void)CnvpCmn_RemoveBufferToQueue( pCnvpCmnHdl, CNV_EVENT_FILLBUFFER, &sQueueFillBufferData);
					
					/* EmptyBuffer Done */
					if( CNV_BUFFER_NONE != sQueueEmptyBufferData.nReleaseFlag){
						CNV_LOGGER(CNV_LOG_INFO, "FLUSH CNV_EmptyBufferDone",0);
						pCnvpContext->sCallbackFunc.CNV_EmptyBufferDone(sQueueEmptyBufferData.pConvertBuffer,pCnvpContext->pUserPointer,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
					}
					/* FillBuffer Done */
					if ( CNV_BUFFER_NONE != sQueueFillBufferData.nReleaseFlag){
						CNV_LOGGER(CNV_LOG_INFO, "FLUSH CNV_FillBufferDone",0);
						pCnvpContext->sCallbackFunc.CNV_FillBufferDone(sQueueFillBufferData.pConvertBuffer,pCnvpContext->pUserPointer,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
					}
					if ( (CNV_BUFFER_NONE == sQueueEmptyBufferData.nReleaseFlag) && (CNV_BUFFER_NONE == sQueueFillBufferData.nReleaseFlag) ){
						break;
					}
				}
				/* Flush Event */
				CnvpCmn_EventDone(pCnvpContext,CNV_CALLBACK_EVENT_FLUSHDONE,CNV_ERROR_NONE,CNV_SUBERROR_NONE);
				
				/* EmptyBuffer */
				pCnvpCmnHdl->sEmptyBufferQueue.nQueueSize 			= CNV_COUNT_QUEUE_ELEMENTS;
				pCnvpCmnHdl->sEmptyBufferQueue.nFirstElementIndex 	= 0;
				pCnvpCmnHdl->sEmptyBufferQueue.nLastElementIndex 	= 0;
				pCnvpCmnHdl->sEmptyBufferQueue.nCountOfBuffers		= 0;
				/* FillBuffer */
				pCnvpCmnHdl->sFillBufferQueue.nQueueSize 			= CNV_COUNT_QUEUE_ELEMENTS;
				pCnvpCmnHdl->sFillBufferQueue.nFirstElementIndex 	= 0;
				pCnvpCmnHdl->sFillBufferQueue.nLastElementIndex 	= 0;
				pCnvpCmnHdl->sFillBufferQueue.nCountOfBuffers		= 0;	
				
				for( nCnt = 0; nCnt < (CNV_COUNT_QUEUE_ELEMENTS - 1); nCnt++ ){
					pCnvpCmnHdl->sEmptyBufferQueue.pBuffer[nCnt]	= 	NULL;
					pCnvpCmnHdl->sFillBufferQueue.pBuffer[nCnt]	= 	NULL;
				}
				
				pCnvpContext->bFlushFlag = CNV_FALSE;
				if(  CNV_TRUE == pCnvpContext->bCloseFlag ){
					bContinueFlag = CNV_TRUE;
				} else {
					bContinueFlag = CNV_FALSE;
				}
				break;
			case( CNV_COMMAND_CLOSE ):
				CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_CLOSE START thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
				bThreadExit = CNV_TRUE;
				Cnvdep_ThreadExit(0);
				break;
			case( CNV_COMMAND_WAITING ):
				if( 0 < pCnvpContext->sTimeoutInfo.nWaitCount ){
					CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_WAITING START thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
					Cnvdep_Sleep(pCnvpContext->sTimeoutInfo.nWaitTime);
				}
				(void)CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_WAITING, NULL, NULL);
				pCnvpContext->sTimeoutInfo.nWaitCount++;
				break;
			case( CNV_COMMAND_TIMEOUT):
				CNV_LOGGER(CNV_LOG_ERROR,"CNV_EVENT_TIMEOUT thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
				(void)CnvpCmn_SendEvent( (CNV_PTR)pCnvpContext, CNV_EVENT_FATAL, NULL, NULL);
				break;
			case( CNV_COMMAND_NONE ):
				CNV_LOGGER(CNV_LOG_DEBUG,"CNV_EVENT_NONE START thread state = %x",(CNV_U32)pCnvpContext->nThreadState);
				bContinueFlag = CNV_FALSE;
				break;
			default:
				break;
			}
		} else {
			bContinueFlag = CNV_FALSE;
			eCommand      = CNV_COMMAND_NONE;
		}
		if( bThreadExit != CNV_TRUE){
			if(bContinueFlag == CNV_FALSE){
				bThreadExit = CNV_TRUE;
			} else {
				bThreadExit = CNV_FALSE;
			}
		}
		CNV_LOGGER(CNV_LOG_DEBUG,"Converter Thread bContinueFlag = %d", (CNV_U32)bContinueFlag);
		CNV_LOGGER(CNV_LOG_DEBUG,"Converter Thread bThreadExit = %d", (CNV_U32)bThreadExit);
	}
	return;
}
