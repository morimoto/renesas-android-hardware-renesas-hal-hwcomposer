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
 * UVCS UDF OS Abstraction Layer Interface Definition
 *
 * \file
 * \attention
 */


#ifndef __OMXR_UVCS_UDF_OSAL_H__
#define __OMXR_UVCS_UDF_OSAL_H__

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include "OMX_Core.h"
#include "OMX_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/* UVCS UDF OSAL Handle */
typedef struct tagOMXR_UVCS_UDF_OSAL_STRUCT *OMXR_UVCS_UDF_OSAL_HANDLE;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
extern OMX_ERRORTYPE OmxrUvcsUdfOsal_Open(OMXR_UVCS_UDF_OSAL_HANDLE *handle);

extern OMX_ERRORTYPE OmxrUvcsUdfOsal_Close(OMXR_UVCS_UDF_OSAL_HANDLE handle);

extern OMX_ERRORTYPE OmxrUvcsUdfOsal_Write(OMXR_UVCS_UDF_OSAL_HANDLE handle, OMX_PTR pvParam, OMX_U32 u32Size);

extern OMX_ERRORTYPE OmxrUvcsUdfOsal_Read(OMXR_UVCS_UDF_OSAL_HANDLE handle, OMX_PTR pvParam, OMX_U32 u32Size);

extern OMX_ERRORTYPE OmxrUvcsUdfOsal_CancelRead(OMXR_UVCS_UDF_OSAL_HANDLE handle);

extern OMX_ERRORTYPE OmxrUvcsUdfOsal_GetIpInfo(OMXR_UVCS_UDF_OSAL_HANDLE handle, OMX_PTR pvIpInfo);


#ifdef __cplusplus
}
#endif

#endif /* __OMXR_UVCS_UDF_OSAL_H__ */
