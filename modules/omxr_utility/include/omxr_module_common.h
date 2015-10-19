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
 * OMX Utility function header file for OMX Core/MC
 *
 * The header file defines the HW/OS dependent function.
 *
 * \file omxr_module_common.h
 *
 */
 

#ifndef _OMXR_MODULE_COMMON_H_
#define _OMXR_MODULE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include <OMXR_Extension.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#include "omxr_debug.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
/* static macro for unittest */
#ifndef OMXR_BUILD_UNIT_TEST
#define OMXR_STATIC static      /**< static */
#else
#define OMXR_STATIC             /**< for test */
#endif

/**
 * MCtrl DLL module initialize entry name
 */
#define OMXR_MCTRL_DLL_INIT_ENTRY_NAME      "OMX_ComponentInit"

/**
 * MCtrl DLL module role enum entry name
 */
#define OMXR_MCTRL_DLL_ROLE_ENTRY_NAME      "OmxrMcComponentRoleEnum"

/**
 * Thread attribute
 */
#define OMXR_THREAD_ATTR_SHARED         0x1

/* Thread processing ID */
#define OMXR_THREAD_PROCID_CREATED      0xFFFFFFFFUL    /**< create thread */
#define OMXR_THREAD_PROCID_SHARED       0xFFFFFFFEUL    /**< shared thread */
#define OMXR_THREAD_PROCID_TERMINATE    0xFFFFFFFDUL    /**< terminate thread */
#define OMXR_THREAD_PROCID_REMAINED     0xFFFFFFFCUL    /**< remain thread */
#define OMXR_THREAD_PROCID_RESERVED     0xFFFFFFFBUL    /**< reserved */

#if !defined(NULL)
#define NULL  ((void*)0)
#endif

/**
 * max port num
 */
#define OMXR_MAX_PORTS                  8

/* WorkBuffer Allocation flags */
#define OMXR_WORKBUFFER_VA_SUPPORT      0x1     /**< virtual address area */
#define OMXR_WORKBUFFER_HA_SUPPORT      0x2     /**< hardware IP address area */
#define OMXR_WORKBUFFER_TA_SUPPORT      0x4     /**< TL(physical) address area */

/* Utility Error Code */
#define OMXR_Util_ErrorBusy             (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000000)     /**< Busy */
#define OMXR_Util_ErrorNoSignal         (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000001)     /**< no signal */
#define OMXR_Util_ErrorNothingToDo      (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000002)     /**< nothing do */
#define OMXR_Util_ErrorOsWrapper        (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000003)     /**< error of wrapper module*/
#define OMXR_Util_ErrorParamNotFound    (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000004)     /**< parameter is not found */
#define OMXR_Util_ErrorSectionNotFound  (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000005)     /**< section is not found */
#define OMXR_Util_ErrorThreadShared     (OMX_S32)(OMXR_UTIL_ErrorBase + 0x00000006)     /**< thread shared */


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
 * defined char type
 */
typedef char OMXR_CHAR;

/**
 * defined WORKBUFFER handle
 */
typedef OMX_PTR OMXR_WORKBUFFER_HANDLE;

/**
 * defined Thread function
 */
typedef OMX_ERRORTYPE (*OMXR_THREAD_FUNCTION)(OMX_U32 u32ProcId, OMX_PTR pvParam, OMX_PTR pvContext);

/****************************************************************************/
/*    Function prototypes                                                   */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The OmxrGetConfigExtSection function will get the information of extension section of the configuration file.
 * 
 * If the section does not exist, this function returns OMXR_Util_ErrorSectionNotFound.
 * If the parameter of section does not exist, this function returns OMXR_Util_ErrorParamNotFound.
 *
 * \param[in]	strSection		Section name
 * \param[in]	strParam		Parameter name
 * \param[out]	**ppu32Array	Pointer to data array
 * \param[out]	*pu32Num		Number of data in the array
 * \return		OMX_ERRORTYPE	
 */
OMX_ERRORTYPE OmxrConfig_GetExtSection(
    const OMX_STRING strSection,
    const OMX_STRING strParam,
    OMX_U32 **ppu32Array,
    OMX_U32 *pu32Num);

/**
 * The OmxrGetConfigExtSection function will get the information of extension parameter of the configuration file.
 *
 * If the parameter does not exist, this function returns OMXR_Util_ErrorParamNotFound.
 *
 * \param[in]	strMcName		Component Name
 * \param[in]	strParam		Parameter name
 * \param[out]	**ppu32Array	Pointer to data array
 * \param[out]	*pu32Num		Number of data in the array
 * \return		OMX_ERRORTYPE	
 */
OMX_ERRORTYPE OmxrConfig_GetExtParameter(
    OMX_STRING strMcName,
    const OMX_STRING strParam,
    OMX_U32 **ppu32Array,
    OMX_U32 *pu32Num);

/**
 * The OmxrMalloc function will allocate unused space.
 *
 * This function has the same functionality as the "malloc".
 *
 * \param[in] u32Size Size
 * \return Pointer to allocated memory (NULL=cannot allocated)
*/
OMX_PTR OmxrMalloc(
    OMX_U32 u32Size);

/**
 * The OmxrCalloc function will allocate unused space and clear allocated space.
 *
 * This function has the same functionality as the "calloc".
 *
 * \param[in] u32Nmemb memory block
 * \param[in] u32SizeBytes Size
 * \return Pointer to allocated memory (NULL=cannot allocated)
 * \attention The memory is set to zero. If the value of u32Nmemb or u32SizeBytes is 0, then calloc() returns either NULL. 
*/
OMX_PTR OmxrCalloc(
    OMX_U32 u32Nmemb,
    OMX_U32 u32SizeBytes);

/**
 * The OmxrFree function will free allocated space by the OmxrMalloc.
 *
 * This function has the same functionality as the "free".
 *
 * \param[in] pvPtr Pointer to memory
 * \return none
*/
void OmxrFree(
    OMX_PTR pvPtr);

/**
 * The OmxrAllocateWorkBuffer function will allocate a buffer of work for Decode/Encode.
 *
 * Method of mapping follow the u32Flags.
 * u32Flags can set OMXR_WORKBUFFER_VA_SUPPORT, OMXR_WORKBUFFER_HA_SUPPORT and OMXR_WORKBUFFER_TA_SUPPORT.
 * OMXR_WORKBUFFER_VA_SUPPORT and OMXR_WORKBUFFER_TA_SUPPORT can not be set in combination.
 * strBufferDiscri specifies a string identifier of the buffer.
 * 
 * \param[out] pBufferHandle Buffer handle
 * \param[in] u32Size Require size
 * \param[in] u32Align Align byte
 * \param[in] strBufferDiscri Buffer discriptor
 * \param[in] u32Flags Mapping flag
 * \return OMX_ERROTTYPE
 * \note OMXR_WORKBUFFER_VA_SUPPORT:Allocate memory from kernel management domain. Mapping to cpu address area.
 *       OMXR_WORKBUFFER_HA_SUPPORT:Allocate memory from kernel management domain. mapping to HWIP address area.
 *       OMXR_WORKBUFFER_TA_SUPPORT:Allocate memory from kernel outside domain.(contiguous physical memory)
 */
OMX_ERRORTYPE OmxrAllocateWorkBuffer(
    OMXR_WORKBUFFER_HANDLE* pBufferHandle,
    OMX_U32 u32Size,
    OMX_U32 u32Align,
    OMX_STRING strBufferDiscri,
    OMX_U32 u32Flags);

/**
 * The OmxrFreeWorkBuffer function will free the work buffer allocated by the OmxrAllocateWorkBuffer.
 *
 * \param[in] BufferHandle Buffer handle
 * \return OMX_ERROTTYPE
 */
OMX_ERRORTYPE OmxrFreeWorkBuffer(
    OMXR_WORKBUFFER_HANDLE BufferHandle);

/**
 * The OmxrGetVirtualAddress function will return the virtual address of the work buffer that allocated by the OmxrAllocateWorkBuffer. 
 *
 * If OMXR_WORKBUFFER_VA_SUPPORT is not included in the value of u32Flags of OmxrAllocateWorkBuffer, the return value is 0.
 *
 * \param[in] BufferHandle Buffer handle
 * \return OMX_U32 virtual address
 */
OMX_PTR OmxrGetVirtualAddress(
    OMXR_WORKBUFFER_HANDLE BufferHandle);

/**
 * The OmxrGetHardwareIPAddress function will return the hardware address of the work buffer that allocated by the OmxrAllocateWorkBuffer. 
 *
 * If OMXR_WORKBUFFER_HA_SUPPORT is not included in the value of u32Flags of OmxrAllocateWorkBuffer, the return value is 0.
 *
 * \param[in] BufferHandle Buffer handle
 * \return OMX_U32 hardware IP address
 */
OMX_U32 OmxrGetHardwareIPAddress(
    OMXR_WORKBUFFER_HANDLE BufferHandle);

/**
 * The OmxrGetTLConvertAddress function will return the physical address of the work buffer that allocated by the OmxrAllocateWorkBuffer. 
 *
 * If OMXR_WORKBUFFER_TA_SUPPORT is not included in the value of u32Flags of OmxrAllocateWorkBuffer, the return value is 0.
 *
 * \param[in] BufferHandle Buffer handle
 * \return OMX_PTR physical address
 */
OMX_U32 OmxrGetTLConvertAddress(
    OMXR_WORKBUFFER_HANDLE BufferHandle);

/**
 * The OmxrGetMemoryManagerHandle function will return the handle of MemoryManager.
 *
 * MemoryManager is a low-level module that performs memory allocation.
 *
 * \param[in] BufferHandle Buffer handle
 * \return OMX_PTR memory manager handle
 */
OMX_U32 OmxrGetMemoryManagerHandle(
    OMXR_WORKBUFFER_HANDLE BufferHandle);

/**
 * The OmxrGetWorkBufferValidSizeBytes function will return an accessible effective size allocated by the OmxrAllocateWorkBuffer.
 *
 * The value returned is usually the same as the size that is set to OmxrAllocateWorkBuffer. 
 * When the buffer size is extended by certain conditions, extended values is returned.
 * Certain conditions will be determined by the argument of strBufferDiscri of OmxrAllocateWorkBuffer.
 *
 * \param[in] BufferHandle Buffer Handle
 * \return OMX_U32 Valid buffer size
 */
OMX_U32 OmxrGetWorkBufferValidSizeBytes(
    OMXR_WORKBUFFER_HANDLE BufferHandle);

/**
 * The OmxrCreateThread function will create a new thread.
 *
 * Thread priority is indicated by strPriority.
 * This function adjusts the priority according to strPriority string.\n
 * If the value of u32Attribute is OMXR_THREAD_ATTR_SHARED, thread is shared.
 * In the case of thread function that specifies the pvFunction is already created, 
 * it does not create a new thread, it operates in an existing thread.\n
 *
 * \param[out] pu32ThreadId Created thread id stored in the location pointed
 * \param[in] pFunction Function entry
 * \param[in] pvParam Parameter to passed to the function
 * \param[in] strPriority Thread priority string
 * \param[in] u32Attribute Attribute
 * \return OMX_ERRORTYPE
 * \note The following values are stored in the argument u32ProcId OMXR_THREAD_FUNCTION type of function.\n
 * 0xFFFFFFFF(-1) : Thread is generated by OmxrCreateThread.\n
 * 0xFFFFFFFE(-2) : Thread is shared by OmxrCreateThread.\n
 * 0xFFFFFFFD(-3) : Thread is destroyed by OmxrTerminateThread.\n
 * 0xFFFFFFFC(-4) : Destroying a shared thread is requested by OmxrTerminateThread (will not be discarded).\n
*/
OMX_ERRORTYPE OmxrCreateThread(
    OMX_U32 *pu32ThreadId,
    OMXR_THREAD_FUNCTION pFunction,
    OMX_PTR pvParam,
    OMX_STRING strPriority,
    OMX_U32 u32Attribute);

/**
 * The OmxrCreateThreadEx function will create a new thread.
 *
 * This is the same as the OmxrCreateThread function expect that stack area of thread can be specified.
 *
 * \param[out] pu32ThreadId Created thread id stored in the location pointed
 * \param[in] pFunction Function entry
 * \param[in] pvParam Parameter to passed to the function
 * \param[in] strPriority Thread priority string
 * \param[in] u32Attribute Attribute
 * \param[in] pvStackTop Top of the stack area (NULL=system allocates automatically)
 * \param[in] u32StackSize Size of the stack area (0=use default size)
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrCreateThreadEx(
    OMX_U32 *pu32ThreadId,
    OMXR_THREAD_FUNCTION pFunction,
    OMX_PTR pvParam,
    OMX_STRING strPriority,
    OMX_U32 u32Attribute,
    OMX_PTR pvStackTop,
    OMX_U32 u32StackSize);

/**
 * The OmxrSaveThreadContext function will save the context of the thread.
 *
 * Thread start-up, please save the context of the thread by using this function.
 * When a thread function is called in OmxrSendEvent, save context is passed as an argument of the thread function.\n
 * Thread ID can be obtained by OmxrGetThreadId.\n
 *
 * \param[in] u32ThreadId Thread ID
 * \param[in] pvContext Context
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrSaveThreadContext(
    OMX_U32 u32ThreadId,
    OMX_PTR pvContext);

/**
 * The OmxrSendEvent function will call thread function.
 *
 * When a caller calls this function, processing function of the thread is called.
 * The OmxrSendEvent function will return immediately(non blocking).
 *
 * \param[in] u32ThreadId Thread ID
 * \param[in] u32ProcId Processing ID(other than 0xFFFFFFFB - 0xFFFFFFFF)
 * \param[in] pvParam Parameter
 * \return OMX_ERRORTYPE
 * \attention This function can not be used thread that was created without the use of OmxrCreateThreadEx OmxrCreateThread function and function.
*/
OMX_ERRORTYPE OmxrSendEvent(
    OMX_U32 u32ThreadId,
    OMX_U32 u32ProcId,
    OMX_PTR pvParam);

/**
 * The OmxrExitThread function will exit from current thread.
 *
 * pvReturn is transmitted to the OmxrJoinThread function.
 *
 * \param[in] pvReturn Thread exit code
 * \return OMX_ERRORTYPE
 * \attention Thread to terminate itself will call the OmxrExitThread function. 
 * If it returns OMX_ErrorNone, the context of the thread can be freed.
 * If it returns OMXR_MGR_ErrorThreadShared, the context of the thread can not free. 
 * OMXR_MGR_ErrorThreadShared indicates that the other is using a thread yet.
*/
OMX_ERRORTYPE OmxrExitThread(
    OMX_PTR pvReturn);

/**
 * The OmxrTerminateThread function will terminate the other thread.
 *
 * This function is called, the function of processing thread to terminate is called.
 * The argument u32ProdId of processing function, 0xFFFFFFFD(= -3) or  0xFFFFFFFC(= -4) is stored.
 *
 * \param[in] u32ThreadId Thread ID to terminate
 * \return OMX_ERRORTYPE
 * \attention 'u32ThreadId' must not self thread ID.
 * It is not possible to perform the 'OmxrSendEvent' function after that delete target thread.
 * Please design so that it does not call the 'OmxrSendEvent' function after the 'OmxrTerminateThead' function call.
*/
OMX_ERRORTYPE OmxrTerminateThread(
    OMX_U32 u32ThreadId);

/**
 * The OmxrJoinThread function will suspend execution of the calling thread until the target thread terminates.
 *
 * \param[in] u32ThreadId Threa ID
 * \param[out] ppvThreadReturn Thread exit code
 * \return OMX_ERRORTYPE
 * \attention Please do not call the OmxrJoinThread function under the following conditions.\n
 * - When OmxrTerminateThread function returns OMXR_MGR_ErrorThreadShared.
 * - When OmxrExitThread function returns OMXR_MGR_ErrorThreadShared.
*/
OMX_ERRORTYPE OmxrJoinThread(
    OMX_U32 u32ThreadId,
    OMX_PTR *ppvThreadReturn);

/**
 * The OmxrGetThreadId function will return the ID of the calling thread.
 *
 * \param[out] pu32ThreadId Thread ID
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrGetThreadId(
    OMX_U32 *pu32ThreadId);

/**
 * The OmxrSleep function will sleep thread.
 *
 * This function cause the calling thread to be suspended from execution until either the number of milliseconds specified by the argument has elapsed
 *
 * \param[in] u32msec Milliseconds
 * \return none
*/
void OmxrSleep(
    OMX_U32 u32msec);

/**
 * The OmxrCreateMutex function will create object of mutex.
 *
 * If the value of bInitialOwner is OMX_TRUE, mutex is locked immediately after creation.
 *
 * \param[out] pu32MutexId Mutex ID
 * \param[in] bInitialOwner Initial owner flag
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrCreateMutex(
    OMX_U32 *pu32MutexId,
    OMX_BOOL bInitialOwner);

/**
 * The OmxrDestroyMutex function will destroy object of mutex.
 *
 * \param[in] u32MutexId Mutex ID
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrDestroyMutex(
    OMX_U32 u32MutexId);

/**
 * The OmxrLockMutex function will lock object of mutex.
 *
 * If the Mutex is already locked by another thread, the behavior is different by bBlocking argument.
 * If the value of bBlocking is OMX_TRUE, this function is blocked until the lock is released.
 * If the value of bBlocking is OMX_FALSE, this function returns OMXR_MGR_ErrorBusy.
 *
 * \param[in] u32MutexId Mutex ID
 * \param[in] bBlocking Blocking mode flag
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrLockMutex(
    OMX_U32 u32MutexId,
    OMX_BOOL bBlocking);

/**
 * The OmxrUnlockMutex function will unlock object of mutex.
 *
 * \param[in] u32MutexId  Mutex ID
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrUnlockMutex(
    OMX_U32 u32MutexId);

/**
 * The OmxrCreateSignal function will create object of signal.
 *
 * If the value of bSignaled is OMX_TRUE, signal is activated immediately after creation.
 *
 * \param[out] pu32SignalId Signal ID
 * \param[in] bSignaled Initial state
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrCreateSignal(
    OMX_U32 *pu32SignalId,
    OMX_BOOL bSignaled);

/**
 * The OmxrDestroySignal function will destroy object of signal.
 *
 * \param[in] u32SignalId Signal ID
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrDestroySignal(
    OMX_U32 u32SignalId);

/**
 * The OmxrWaitSignal function will wait for the activation of the signal.
 *
 * S32Timeout of argument sets the timeout in milliseconds time.
 * If the value of S32Timeout is 0, the function returns OMXR_MGE_ErrorNoSignal if the signal is not activated.
 * If the value of S32Timeout is -1, the function blocks until the signal is activated.
 * If the value of S32Timeout is one or more, the function returns OMXR_MGE_ErrorNoSignal if the signal is not activated in time.
 *
 * \param[in] u32SignalId Signal ID
 * \param[in] s32Timeout Timeout period
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrWaitSignal(
    OMX_U32 u32SignalId,
    OMX_S32 s32Timeout);

/**
 * The OmxrSignal function will activate the signal.
 *
 * \param[in] u32SignalId Signal ID
 * \return OMX_ERRORTYPE
*/
OMX_ERRORTYPE OmxrSignal(
    OMX_U32 u32SignalId);

/**
 * The OmxrMemcpy function will copy count bytes of src to dest.
 *
 * This function has the same functionality as the "memcpy".
 *
 * \param[in] pvDest Pointer to destination.
 * \param[in] pvSrc Pointer to source.
 * \param[in] u32Size memory to memory copy size.
 * \return dest (Destinaion's pointer.)
*/
OMX_PTR OmxrMemcpy(
    OMX_PTR pvDest,
    const OMX_PTR pvSrc,
    OMX_U32 u32Size);

/**
 * The OmxrMemset function will fill memory with a constant byte.
 *
 * This function has the same functionality as the "memset".
 *
 * \param[in] pvDest Pointer to destination.
 * \param[in] u8Code Character to set.
 * \param[in] u32Size Number of characters.
 * \return dest (Destinaion's pointer.)
*/
OMX_PTR OmxrMemset(
    OMX_PTR pvDest,
    OMX_U8 u8Code,
    OMX_U32 u32Size);

/**
 * The OmxrStrlen function will calculate the length of a string.
 *
 * This function has the same functionality as the "strlen".
 *
 * \param[in] strString Pointer to string.
 * \return OMX_U32 (The number of characters in string.)
*/
OMX_U32 OmxrStrlen(
    const OMX_STRING strString);

/**
 * The OmxrStrcmp function will compare two strings.
 *
 * This function has the same functionality as the "strcmp".
 *
 * \param[in] strString1 Pointer to string1.
 * \param[in] strString2 Pointer to string2.
 * \return OMX_S32
 *               ( < 0 : string1 less than string2.      )
 *               (   0 : string1 identical to string2.   )
 *               ( > 0 : string1 greater than string2.   )
*/
OMX_S32 OmxrStrcmp(
    const OMX_STRING strString1,
    const OMX_STRING strString2);

/**
 * The OmxrSprintf function will store formatted string to buffer.
 *
 * This function has the same functionality as the "sprintf".
 *
 * \param[out] strBuffer Buffer to store a atring.
 * \param[in] strString String
 * \param[in] ... and parameters
 * \return OMX_S32 Formatted string byte length. (not include NUL terminator.) or -1 if error.
*/
OMX_S32 OmxrSprintf(
    OMX_STRING strBuffer,
    const OMX_STRING strString,
    ...);

/**
 * The OmxrLog function will display a log message.
 *
 * \param[in] u32Level Message level
 * \param[in] strString Strings
 * \return none
*/
void OmxrLog(
    OMX_U32 u32Level,
    const OMX_STRING strString,
    ...);

/**
 * The OmxrLogVa function will display a log message.
 *
 * This function accepts an argument list by va_list.
 *
 * \param[in] u32Level Message level
 * \param[in] strString Strings
 * \param[in] arglist Arguments
 * \return none
*/
void OmxrLogVa(
    OMX_U32 u32Level,
    const OMX_STRING strString,
    va_list arglist);

/**
 * The OmxrLogCheckLevel function will check log level.
 *
 * In the case of a valid log level, this function returns OMX_TRUE.
 *
 * \param[in] u32Level Message level
 * \return OMX_BOOL
*/
OMX_BOOL OmxrLogCheckLevel(
    OMX_U32 u32Level);

/**
 * The OmxrLogWithoutLevel function will display a log message without checking the log level.
 *
 * \param[in] strString Strings
 * \return none
*/
void OmxrLogWithoutLevel(
    const OMX_STRING strString,
    ...);

/**
 * The OmxrAbort function will do the abort processing.
 *
 * To inform the abort generation point, this function sets the occurrence number of lines and the function strings.
 *
 * \param[in] strString Function strings
 * \param[in] u32LineNum  Line number
 * \return none
*/
void OmxrAbort(
    const OMX_STRING strString,
    OMX_U32 u32LineNum);

#ifdef __cplusplus
}
#endif

#endif /* _OMXR_MODULE_COMMON_H_ */
