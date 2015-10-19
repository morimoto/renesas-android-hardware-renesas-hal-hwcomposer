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
 * OMX Converter os dependence file 
 *
 * \file cnv_osdep.h
 * \attention
 */
#ifndef CNV_OS_DEPENDENCE_H
#define CNV_OS_DEPENDENCE_H

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
/**
* \enum CNV_LOG_MODE
*/
typedef enum {
	CNV_LOG_DEBUG = 0,
	CNV_LOG_WARN,
	CNV_LOG_INFO,
	CNV_LOG_ERROR
} CNV_LOG_MODE;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

/**
* The Cnvdep_Malloc function will allocate unused space.
*
* \param[in] u32Size   size
* \return Pointer to allocated memory (NULL=cannot allocated)
*/
CNV_PTR Cnvdep_Malloc(CNV_U32 nSize);

/**
* The Cnvdep_BufferMalloc function will allocate unused space.
*
* \param[in] u32Size   size
* \return Pointer to allocated memory (NULL=cannot allocated)
*/
CNV_PTR Cnvdep_BufferMalloc(CNV_U32 nSize);

/**
* The Cnvdep_Free function will free allocated space by the Cnvdep_Malloc.
*
* \param[in] pvPtr     pointer to memory
* \return none
*/
CNV_VOID Cnvdep_Free(CNV_PTR pvPtr);

/**
* The Cnvdep_BufferFree function will free allocated space by the Cnvdep_BufferFree.
*
* \param[in] pvPtr     pointer to memory
* \return none
*/
CNV_VOID Cnvdep_BufferFree(CNV_PTR pvPtr);

/**
 * The Cnvdep_Memcpy function will copy count bytes of src to dest.
 *
 * \param[in] pvDest   pointer to destination.
 * \param[in] pvSrc    pointer to source.
 * \param[in] nSize    memory to memory copy size.
 * \return destination ( destination's pointer.)
*/
CNV_PTR Cnvdep_Memcpy(CNV_PTR pvDest, CNV_PTR pvSrc, CNV_U32 nSize);

/**
 * The Cnvdep_Memset function will fill memory with a constant byte.
 *
 * \param[in] pvDest   pointer to destination.
 * \param[in] nCode    character to set.
 * \param[in] nSize    number of characters.
 * \return destination ( destination's pointer.)
*/
CNV_PTR Cnvdep_Memset(CNV_PTR pvDest, CNV_U8 nCode, CNV_U32 nSize);
	
/**
* The Cnvdep_LoadDll function will load the dynamic link library.
*
* \param[in]  path     pathname of DLL file
* \param[out] handle   handle of DLL
* \return none
*/
CNV_ERRORTYPE Cnvdep_LoadDll(CNV_STRING path, CNV_HANDLE*handle);

/**
* The Cnvdep_GetDllEntry function will return the specified entry in the DLL.
*
* \param[in]  handle   Handle of DLL
* \param[in]  name     entry name
* \param[out] entry    entry point
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_GetDllEntry(CNV_HANDLE handle, CNV_STRING name, CNV_PTR*entry);

/**
* The Cnvdep_UnloadDll function will unload the dynamic link library.
*
* \param[in] hangle    handle of DLL
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_UnloadDll(CNV_HANDLE handle);

/**
* The CnvdepThreadExit function will exit from current thread.
*
* \param[in] retval     Thread exit code
*/
CNV_VOID Cnvdep_ThreadExit( CNV_VOID *retval );

/**
* The Cnvdep_ThreadJoin function will suspend execution of the calling thread until the target thread terminates.
*
* \param[in] nThreadID   thread id
* \param[out] retval     thread exit code
*/
CNV_U32 Cnvdep_ThreadJoin( CNV_U32 nThreadID, CNV_PTR *retval );

/**
* The CnvdepCreateOsDepHandle function will create handle of osal( Operating system abstraction layer ).
*
* \param[out] nThreadID   handle of converter thread.
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_CreateOsDepHandle( CNV_U32 *nThreadID);

/**
* The Cnvdep_CreateThread function will create the thead of converter.
*
* \param[in] nThreadID         handle of converter thread
* \param[in] thread_function   converter main thread
* \param[in] pParam            local parameter
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_CreateThread( CNV_U32 nThreadID,  CNV_VOID (*thread_function)(CNV_PTR pHandle, CNV_U32 nEventId, CNV_PTR pData) , CNV_PTR pParam );

/**
* The CnvdepCreateMutex function will create object of mutex.
*
* \param[out] nMutex    handle of mutex
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_CreateMutex( CNV_U32 *nMutex );

/**
* The Cnvdep_DeleateMutex function will destroy object of mutex.
*
* \param[in]  nMutex    handle of mutex
* \return CNV_ERRORTYPE
*/
CNV_VOID Cnvdep_DeleateMutex( CNV_U32 nMutex );

/**
* The Cnvdep_LockMutex function will lock object of mutex.
*
* \param[in] nMutex    handle of mutex
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_VOID Cnvdep_LockMutex( CNV_U32 nMutex );

/**
* The Cnvdep_UnlockMutex function will unlock object of mutex.
*
* \param[in] nMutex    handle of mutex
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_VOID Cnvdep_UnlockMutex( CNV_U32 nMutex );

/**
* The Cnvdep_Sleep function will sleep the thread.
* \breaf
* \param[in] nSleepTime   milliseconds
* \return none
*/
CNV_VOID Cnvdep_Sleep(CNV_U32 nSleepTime);

/**
* The Cnvdep_SaveTreadContext function will save the context of the thread.
*
* \param[in] nThreadID   thread id
* \param[in] pParam      save parameter
* \return OMX_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_SaveTreadContext(CNV_U32 nThreadID,  CNV_PTR pParam);

/**
* The CnvdepSendEvent function will call thread function.
*
* \param[in] nThreadId   handle of converter thread
* \param[in] u32ProcId   processing id(other than 0xFFFFFFFB - 0xFFFFFFFF)
* \param[in] pvParam     send parameter
* \return OMX_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE Cnvdep_SendEvent(CNV_U32 nThreadId, CNV_U32 nProcId, CNV_PTR pvParam);

/**
* The Cnvdep_LogFunc will display a log message.
* \param[in] u32Level      message level
* \param[in] strFunction   function name
* \param[in] u32Lineno     line number
* \param[in] strString     strings
* \param[in] Value         value
* \return none
*/
CNV_VOID Cnvdep_LogFunc(CNV_U32 u32Level, CNV_STRING strFunction, CNV_U32 u32Lineno, CNV_STRING strString, CNV_U32 Value);

/**
* The Cnvdep_StrLogFunc will display a log message.
* \param[in] u32Level      message level
* \param[in] strFunction   function name
* \param[in] u32Lineno     line number
* \param[in] strString     strings
* \param[in] strLog        strings
* \return none
*/
CNV_VOID Cnvdep_StrLogFunc(CNV_U32 u32Level, CNV_STRING strFunction, CNV_U32 u32Lineno, CNV_STRING strString, CNV_STRING strLog);

/**
* The CnvdepOpen will be debug function. (no use)
*/
CNV_VOID Cnvdep_Open(CNV_VOID);

/**
* The CnvdepClose will be debug function. (no use)
*/
CNV_VOID Cnvdep_Close(CNV_VOID);

#define CNV_LOGGER(loglevel, fmt, value) Cnvdep_LogFunc(loglevel, (CNV_STRING)__FUNCTION__, __LINE__, fmt, value)
#define CNV_LOGGER_STRING(loglevel, fmt, log) Cnvdep_StrLogFunc(loglevel, (CNV_STRING)__FUNCTION__, __LINE__, fmt, log)

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
