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
 *  OMX Converter os dependence function
 *
 * \file cnv_osdep.c
 * \attention 
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include "cnv_type.h"
#include "cnv_osdep.h"

#include "omxr_dep_common.h"
#include "omxr_module_common.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/**
* \struct CNV_THREAD_HANDLE_T
*/
typedef struct {
	CNV_U32 nThreadID;
	CNV_VOID (*thread_function)(CNV_PTR pHandle, CNV_U32 nEventId, CNV_PTR pData);
	CNV_PTR pvParam;
	CNV_PTR pHandle;
	CNV_U32 nCommand;
} CNV_THREAD_HANDLE_T;

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/**
* The Cnvdep_ThreadEntry is the main thread function.
*
* \param[in] u32ProcId        thread handle
* \param[in] pvParam          converter handle
* \param[in] pvContext        common context
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
static OMX_ERRORTYPE Cnvdep_ThreadEntry(OMX_U32 u32ProcId, OMX_PTR pvParam, OMX_PTR pvContext);

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/

CNV_VOID Cnvdep_Open(CNV_VOID){
	OmxrOpenOsWrapper();
}

CNV_VOID Cnvdep_Close(CNV_VOID){
	OmxrCloseOsWrapper();
}

CNV_PTR Cnvdep_Malloc(CNV_U32 nSize)
{
	return (CNV_PTR)OmxrMalloc((OMX_U32)nSize);
}


CNV_VOID Cnvdep_Free(CNV_PTR pvPtr)
{
	OmxrFree((OMX_PTR)pvPtr);
	return;
}


CNV_PTR Cnvdep_BufferMalloc(CNV_U32 nSize)
{
	return (CNV_PTR)OmxrMalloc((OMX_U32)nSize);
}


CNV_VOID Cnvdep_BufferFree(CNV_PTR pvPtr)
{
	OmxrFree((OMX_PTR)pvPtr);
	return;
}


CNV_PTR Cnvdep_Memcpy(CNV_PTR pvDest, CNV_PTR pvSrc, CNV_U32 nSize)
{
	return OmxrMemcpy((OMX_PTR)pvDest,(OMX_PTR)pvSrc,nSize);
}


CNV_PTR Cnvdep_Memset(CNV_PTR pvDest, CNV_U8 nCode, CNV_U32 nSize)
{
	return OmxrMemset((OMX_PTR)pvDest,(OMX_U8)nCode,(OMX_U32)nSize);
}


CNV_ERRORTYPE Cnvdep_LoadDll(CNV_STRING path, CNV_HANDLE *handle)
{
	OMX_ERRORTYPE eError;

	eError = OmxrLoadDll((OMX_STRING)path, (OMX_U32 *)handle);
	if (eError != OMX_ErrorNone) {
   		CNV_LOGGER_STRING(CNV_LOG_DEBUG,"DLL entry not found failed.= %s",path);
		return CNV_ERROR_FATAL;
	} else {
		return CNV_ERROR_NONE;
	}
}


CNV_ERRORTYPE Cnvdep_GetDllEntry(CNV_HANDLE handle, CNV_STRING name, CNV_PTR *entry)
{
    OMX_ERRORTYPE eError;
	
	eError = OmxrGetDllEntry((OMX_U32)handle, (OMX_STRING)name, (OMX_PTR *)entry);
	if (eError != OMX_ErrorNone) {
		CNV_LOGGER_STRING(CNV_LOG_ERROR,"DLL entry not found failed.= %s",name);
		return CNV_ERROR_FATAL;
	} else {
		return CNV_ERROR_NONE;
	}
}


CNV_ERRORTYPE Cnvdep_UnloadDll(CNV_HANDLE handle)
{
	OMX_ERRORTYPE eError;

	eError = OmxrUnloadDll((OMX_U32)handle);
	if(eError != OMX_ErrorNone){
		CNV_LOGGER(CNV_LOG_DEBUG,"DLL unloaded.= %p",(CNV_U32)handle);	
		return CNV_ERROR_FATAL;
	}else{
		return CNV_ERROR_NONE;
	}
}


CNV_ERRORTYPE Cnvdep_CreateMutex( CNV_U32 *nMutex )
{
	OMX_ERRORTYPE eError;
	
	eError = OmxrCreateMutex((OMX_U32 *)nMutex, OMX_FALSE);

	if(eError != OMX_ErrorNone){
		return CNV_ERROR_FATAL;	
	}else{
		return CNV_ERROR_NONE;
	}
}	


CNV_VOID Cnvdep_DeleateMutex( CNV_U32 nMutex )
{
	OMX_ERRORTYPE eError;
	
	eError = OmxrDestroyMutex((OMX_U32)nMutex);

	if(eError != OMX_ErrorNone){
		return;
	}else{
		return;
	}
}


CNV_VOID Cnvdep_LockMutex( CNV_U32 nMutex )
{
	OMX_ERRORTYPE eError;
	
	eError = OmxrLockMutex((OMX_U32)nMutex, OMX_TRUE);

	if(eError != OMX_ErrorNone){
		return;
	}else{
		return;
	}
}


CNV_VOID Cnvdep_UnlockMutex( CNV_U32 nMutex )
{
	OMX_ERRORTYPE eError;
	
	eError = OmxrUnlockMutex((OMX_U32)nMutex);

	if(eError != OMX_ErrorNone){
		return;
	}else{
		return;
	}
}


CNV_ERRORTYPE Cnvdep_CreateOsDepHandle( CNV_U32 *nThreadID)
{
	CNV_THREAD_HANDLE_T *psHandle;
	CNV_ERRORTYPE		eResult = CNV_ERROR_NONE;
	psHandle = (CNV_THREAD_HANDLE_T *)Cnvdep_Malloc( sizeof(CNV_THREAD_HANDLE_T) );
	if ( NULL == psHandle ) {
		CNV_LOGGER(CNV_LOG_ERROR,"Handle failed.= %x",(CNV_U32)psHandle);
		return CNV_ERROR_FATAL;
	} else {
		(void)OmxrMemset(psHandle,0,sizeof(CNV_THREAD_HANDLE_T));
	}
	*nThreadID = (CNV_U32)psHandle;
	
	return eResult;
}

static OMX_ERRORTYPE Cnvdep_ThreadEntry(OMX_U32 u32ProcId, OMX_PTR pvParam, OMX_PTR pvContext){
	CNV_THREAD_HANDLE_T *psHandle;
	
	if( 0xFFFFFFFFUL == u32ProcId ){
		psHandle = (CNV_THREAD_HANDLE_T *)pvParam;
		psHandle->thread_function(psHandle->pHandle,u32ProcId,psHandle->pvParam);
	} else {
		psHandle = (CNV_THREAD_HANDLE_T *)pvContext;
		psHandle->thread_function(psHandle->pHandle,u32ProcId,pvParam);
	}
	return OMX_ErrorNone;
}

CNV_ERRORTYPE Cnvdep_CreateThread( CNV_U32 nThreadID,  CNV_VOID (*thread_function)(CNV_PTR pHandle, CNV_U32 nEventId, CNV_PTR pData) , CNV_PTR pParam )
{
	CNV_THREAD_HANDLE_T *psHandle;
	OMX_ERRORTYPE 		eError = OMX_ErrorNone;
	CNV_ERRORTYPE       eResult = CNV_ERROR_NONE;
	psHandle = (CNV_THREAD_HANDLE_T *)nThreadID;

	psHandle->thread_function = thread_function;
	psHandle->pHandle = pParam;
	
	eError = OmxrCreateThread((OMX_U32 *)&psHandle->nThreadID, &Cnvdep_ThreadEntry, psHandle,"PRIORITY.HIGH.CNV", 0);
	if ( OMX_ErrorNone != eError ) {
		CNV_LOGGER(CNV_LOG_ERROR,"OmxrCreateThread failed.= %x",(CNV_U32)eError);
		eResult = CNV_ERROR_FATAL;
	}
	
	return eResult;
}

CNV_ERRORTYPE Cnvdep_SaveTreadContext(CNV_U32 nThreadID,  CNV_PTR pParam){
	OMX_ERRORTYPE 		eError = OMX_ErrorNone;
	CNV_THREAD_HANDLE_T *psHandle;
	
	psHandle = (CNV_THREAD_HANDLE_T *)nThreadID;
	
	eError = OmxrSaveThreadContext(psHandle->nThreadID,pParam);
	if( OMX_ErrorNone != eError){
		return CNV_ERROR_FATAL;
	}
	
	return CNV_ERROR_NONE;
}

CNV_VOID Cnvdep_ThreadExit( CNV_VOID *retval )
{
	(void)OmxrExitThread(retval);
	return;
}


CNV_U32 Cnvdep_ThreadJoin( CNV_U32 nThreadID, CNV_PTR *retval )
{	
	OMX_ERRORTYPE iret;
	CNV_U32	func_ret = 0;
	CNV_THREAD_HANDLE_T *psHandle;
	
	psHandle = (CNV_THREAD_HANDLE_T *)nThreadID;
	
	iret = OmxrJoinThread((OMX_U32)psHandle->nThreadID, NULL );
	if ( iret != OMX_ErrorNone ) {
		CNV_LOGGER(CNV_LOG_ERROR,"nThreadID.= %d",psHandle->nThreadID);
		func_ret = 0xFFFFFFFFUL;
	}
	/* Free Handle */
	Cnvdep_Free((CNV_PTR)psHandle);
	
	return func_ret;
}


CNV_ERRORTYPE Cnvdep_SendEvent(CNV_U32 nThreadId, CNV_U32 nProcId, CNV_PTR pvParam)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_NONE;
	OMX_ERRORTYPE 		eError = OMX_ErrorNone;
	CNV_THREAD_HANDLE_T *psHandle = (CNV_THREAD_HANDLE_T*)nThreadId;

	psHandle->pvParam = pvParam;
	psHandle->nCommand =nProcId;

	eError = OmxrSendEvent((OMX_U32)psHandle->nThreadID, (OMX_U32)nProcId, (OMX_PTR)pvParam);
	if( OMX_ErrorNone != eError ){
		CNV_LOGGER(CNV_LOG_ERROR,"SendEvent error.= %d", (OMX_U32)eError);
	  	eResult = CNV_ERROR_FATAL;
	}
   return eResult;
}


CNV_VOID Cnvdep_Sleep(CNV_U32 nSleepTime)
{
	OmxrSleep(nSleepTime);
	return;
}


CNV_VOID Cnvdep_LogFunc(CNV_U32 u32Level, CNV_STRING strFunction, CNV_U32 u32Lineno, CNV_STRING strString, CNV_U32 Value)
{
	switch(u32Level)
	{
	case(CNV_LOG_DEBUG):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_DEBUG, (OMX_STRING)strFunction,u32Lineno, strString, Value, "");
		break;
	case(CNV_LOG_WARN):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_WARN, (OMX_STRING)strFunction,u32Lineno, strString, Value, "");
		break;
	case(CNV_LOG_INFO):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_INFO, (OMX_STRING)strFunction,u32Lineno, strString, Value, "");
		break;
	case(CNV_LOG_ERROR):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_ERROR, (OMX_STRING)strFunction,u32Lineno, strString, Value, "");
		break;
	default:
		break;
	}
	return;
}

CNV_VOID Cnvdep_StrLogFunc(CNV_U32 u32Level, CNV_STRING strFunction, CNV_U32 u32Lineno, CNV_STRING strString, CNV_STRING strLog)
{
	switch(u32Level)
	{
	case(CNV_LOG_DEBUG):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_DEBUG, (OMX_STRING)strFunction,u32Lineno, strString, strLog, "");
		break;
	case(CNV_LOG_WARN):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_WARN, (OMX_STRING)strFunction,u32Lineno, strString, strLog, "");
		break;
	case(CNV_LOG_INFO):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_INFO, (OMX_STRING)strFunction,u32Lineno, strString, strLog, "");
		break;
	case(CNV_LOG_ERROR):
		(void)OmxrLogFormat(OMXR_CNV_LOG_LEVEL_ERROR, (OMX_STRING)strFunction,u32Lineno, strString, strLog, "");
		break;
	default:
		break;
	}
	return;
}
