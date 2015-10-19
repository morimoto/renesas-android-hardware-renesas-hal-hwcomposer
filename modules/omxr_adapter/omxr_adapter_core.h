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
 * OMX Adapter for Android
 *
 * OpenMAX IL Core API functions interface definition
 *
 * @file
 */
#ifndef __OMXR_ADAPTER_CORE_H__
#define __OMXR_ADAPTER_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
OMX_ERRORTYPE OmxrAdapterCore_GetHandle(
	OMX_HANDLETYPE   *hRealComponentHandle, 
	OMX_STRING       cComponentName,
	OMX_PTR          pAppData,
	OMX_CALLBACKTYPE *pCallBacks);

OMX_ERRORTYPE OmxrAdapterCore_FreeHandle(
	OMX_HANDLETYPE  hRealComponentHandle);

#ifdef __cplusplus
}
#endif

#endif /* __OMXR_ADAPTER_CORE_H__ */
