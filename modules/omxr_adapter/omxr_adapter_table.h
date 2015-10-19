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
 * Adapter component table definition
 *
 * @file
 */
#ifndef __OMXR_ADAPTER_TABLE_H__
#define __OMXR_ADAPTER_TABLE_H__

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
typedef OMX_ERRORTYPE (*OMXR_ADAPTER_COMPONENT_INIT_FUNC)(OMX_HANDLETYPE *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *psClientCallBacks, OMX_CALLBACKTYPE *psOverrideCallBacks);

typedef struct {
    OMX_STRING                       strComponentName;
    OMXR_ADAPTER_COMPONENT_INIT_FUNC fpComponentInit;
} OMXR_ADAPTER_IMPLEMENT_TABLE;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

/* extern for component entry functions */
extern OMX_ERRORTYPE OmxrAdapterBase_ComponentInit(OMX_HANDLETYPE   *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *psClientCallBacks, OMX_CALLBACKTYPE *psOverrideCallBacks);

extern OMX_ERRORTYPE OmxrAdapterVideoDecoder_ComponentInit(OMX_HANDLETYPE   *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *psClientCallBacks, OMX_CALLBACKTYPE *psOverrideCallBacks);

extern OMX_ERRORTYPE OmxrAdapterVideoEncoder_ComponentInit(OMX_HANDLETYPE   *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *psClientCallBacks, OMX_CALLBACKTYPE *psOverrideCallBacks);

/* component table */
static OMXR_ADAPTER_IMPLEMENT_TABLE sAdapterTable[] = {
#if OMXR_ADAPTER_BUILD_ENABLE_VIDEO_DECODER
	{ "OMX.RENESAS.VIDEO.DECODER", &OmxrAdapterVideoDecoder_ComponentInit },
#endif
#if OMXR_ADAPTER_BUILD_ENABLE_VIDEO_ENCODER
	{ "OMX.RENESAS.VIDEO.ENCODER", &OmxrAdapterVideoEncoder_ComponentInit },
#endif
	{ "OMX.RENESAS", &OmxrAdapterBase_ComponentInit },
};

#endif /* __OMXR_ADAPTER_TABLE_H__ */
