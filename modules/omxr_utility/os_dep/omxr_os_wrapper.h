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
 * OMXR OS wrapper header file
 *
 * The header file defines the OS wrapper function.
 *
 * \file
 * \attention
 */
 

#ifndef _OMXR_OS_WRAPPER_H_
#define _OMXR_OS_WRAPPER_H_

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

/* Prototype of the thread entry function */
typedef void (*TASK_FUNCTION)(OMX_S32);


/* Thread handle */
typedef struct tagOMXR_THREAD_HANDLE {
    pthread_t                       hPhysHandle;    /**< Physical thread handle */
    OMXR_THREAD_FUNCTION            pFunction;      /**< Thread function */
    OMX_PTR                         pvContext;      /**< Context of the thread */
    OMX_PTR                         pvThreadReturn; /**< Thread retuen value */
    OMX_U32                         u32Attribute;   /**< Attribute */
    OMX_U32                         u32RefCount;    /**< Reference counter */
    OMX_U32                         u32MessageBox;  /**< Message box to receive command */
    struct tagOMXR_THREAD_HANDLE    *Next;          /**< Linked list */
} OMXR_THREAD_HANDLE;


/* Function call message structure */
typedef struct tagOMXR_THREAD_COMMEND_MESSAGE {
    OMX_U32         u32ProcId;          /**< Processing ID */
    OMX_PTR         pvInfo;             /**< Parameter */
    OMX_U32         u32MessageBox;      /**< Message box to receive result (0=non blocking) */
} OMXR_THREAD_COMMEND_MESSAGE;


/* Function result message structure */
typedef struct tagOMXR_THREAD_RESULT_MESSAGE {
    OMX_ERRORTYPE   eErrorType;         /**< Result value */
} OMXR_THREAD_RESULT_MESSAGE;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _OMXR_OS_WRAPPER_H_ */
