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
 * OpenMAX IL Macro API base functions interface definition
 *
 * @file
 */
#ifndef __OMXR_ADAPTER_BASE_H__
#define __OMXR_ADAPTER_BASE_H__

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
#define OMXR_ADAPTER_SPECVERSIONMAJOR  1
#define OMXR_ADAPTER_SPECVERSIONMINOR  1
#define OMXR_ADAPTER_SPECREVISION      2
#define OMXR_ADAPTER_SPECSTEP          0
#define OMXR_ADAPTER_SPECVERSION      ((OMXR_ADAPTER_SPECSTEP << 24) | \
                                      (OMXR_ADAPTER_SPECREVISION << 16) | \
                                      (OMXR_ADAPTER_SPECVERSIONMINOR << 8) | \
                                      (OMXR_ADAPTER_SPECVERSIONMAJOR))

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef struct {
	OMX_HANDLETYPE hAdapterComponentHandle;
	OMX_HANDLETYPE hRealComponentHandle;
	OMX_CALLBACKTYPE sClientCallbacks;
	OMX_PTR pvAdapterPrivate;
} OMXR_ADAPTER_BASE_CONTEXT;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_ComponentInit(
	OMX_HANDLETYPE   *pHandle, 
	OMX_STRING       cComponentName,
	OMX_PTR          pAppData,
	OMX_CALLBACKTYPE *psClientCallBacks,
	OMX_CALLBACKTYPE *psOverrideCallBacks);

OMX_ERRORTYPE OmxrAdapterBase_GetComponentVersion(
	OMX_HANDLETYPE   hComponent,
	OMX_STRING	 pcComponentName,
	OMX_VERSIONTYPE  *psComponentVersion,
	OMX_VERSIONTYPE  *psSpecVersion,
	OMX_UUIDTYPE     *psComponentUUID);

OMX_ERRORTYPE OmxrAdapterBase_SendCommand(
	OMX_HANDLETYPE   hComponent,
	OMX_COMMANDTYPE  sCommand,
	OMX_U32          u32Param,
	OMX_PTR          pvCmdData);

OMX_ERRORTYPE OmxrAdapterBase_GetParameter(
	OMX_HANDLETYPE   hComponent,
	OMX_INDEXTYPE    sIndexType,
	OMX_PTR          pvParameterStructure);

OMX_ERRORTYPE OmxrAdapterBase_SetParameter(
	OMX_HANDLETYPE   hComponent,
	OMX_INDEXTYPE    sIndexType,
	OMX_PTR          pvParameterStructure);

OMX_ERRORTYPE OmxrAdapterBase_GetConfig(
	OMX_HANDLETYPE   hComponent,
	OMX_INDEXTYPE    sIndexType,
	OMX_PTR          pvConfigStructure);

OMX_ERRORTYPE OmxrAdapterBase_SetConfig(
	OMX_HANDLETYPE   hComponent,
	OMX_INDEXTYPE    sIndexType,
	OMX_PTR          pvConfigStructure);

OMX_ERRORTYPE OmxrAdapterBase_GetExtensionIndex(
	OMX_HANDLETYPE   hComponent,
	OMX_STRING       pcParameterName,
	OMX_INDEXTYPE    *psIndexType);

OMX_ERRORTYPE OmxrAdapterBase_GetState(
	OMX_HANDLETYPE   hComponent,
	OMX_STATETYPE    *psState);

OMX_ERRORTYPE OmxrAdapterBase_ComponentTunnelRequest(
	OMX_HANDLETYPE   hComponent,
	OMX_U32          nPortIndex,
	OMX_HANDLETYPE   hTunneledComp,
	OMX_U32          nTunneledPort,
	OMX_TUNNELSETUPTYPE* pTunnelSetup);

OMX_ERRORTYPE OmxrAdapterBase_UseBuffer(
	OMX_HANDLETYPE       hComponent,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes,
	OMX_U8               *pBuffer);

OMX_ERRORTYPE OmxrAdapterBase_AllocateBuffer(
	OMX_HANDLETYPE       hComponent,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes);

OMX_ERRORTYPE OmxrAdapterBase_FreeBuffer(
	OMX_HANDLETYPE   hComponent,
	OMX_U32          u32PortIndex,
	OMX_BUFFERHEADERTYPE* psBuffer);

OMX_ERRORTYPE OmxrAdapterBase_EmptyThisBuffer(
	OMX_HANDLETYPE        hComponent,
	OMX_BUFFERHEADERTYPE  *psBuffer);

OMX_ERRORTYPE OmxrAdapterBase_FillThisBuffer(
	OMX_HANDLETYPE        hComponent,
	OMX_BUFFERHEADERTYPE  *psBuffer);

OMX_ERRORTYPE OmxrAdapterBase_SetCallbacks(
	OMX_HANDLETYPE    hComponent,
	OMX_CALLBACKTYPE  *psCallbacks,
	OMX_PTR           pvAppData);

OMX_ERRORTYPE OmxrAdapterBase_ComponentDeInit(
	OMX_HANDLETYPE hComponent);

OMX_ERRORTYPE OmxrAdapterBase_UseEGLImage(
	OMX_HANDLETYPE       hComponent,
	OMX_BUFFERHEADERTYPE **ppBufferHdr,
	OMX_U32              nPortIndex,
	OMX_PTR              pAppPrivate,
	void                 *eglImage);

OMX_ERRORTYPE OmxrAdapterBase_ComponentRoleEnum(
	OMX_HANDLETYPE   hComponent,
	OMX_U8           *cRole,
	OMX_U32          nIndex);

OMX_ERRORTYPE OmxrAdapterBase_EventHandler(
	OMX_HANDLETYPE hComponent,
	OMX_PTR pAppData,
	OMX_EVENTTYPE eEvent,
	OMX_U32 nData1,
	OMX_U32 nData2,
	OMX_PTR pEventData);

OMX_ERRORTYPE OmxrAdapterBase_EmptyBufferDone(
	OMX_IN OMX_HANDLETYPE hComponent,
	OMX_IN OMX_PTR pAppData,
	OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE OmxrAdapterBase_FillBufferDone(
	OMX_OUT OMX_HANDLETYPE hComponent,
	OMX_OUT OMX_PTR pAppData,
	OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

OMX_COMPONENTTYPE *OmxrAdapterBase_GetAdapterComponentFromRealHandle(OMX_HANDLETYPE hRealComponentHandle);

OMX_COMPONENTTYPE *OmxrAdapterBase_GetRealComponentFromAdapterHandle(OMX_HANDLETYPE hAdapterComponentHandle);

OMXR_ADAPTER_BASE_CONTEXT *OmxrAdapterBase_GetContextFromRealHandle(OMX_HANDLETYPE hRealComponentHandle);

OMXR_ADAPTER_BASE_CONTEXT *OmxrAdapterBase_GetContextFromAdapterHandle(OMX_HANDLETYPE hAdapterComponentHandle);

OMX_PTR OmxrAdapterBase_GetPrivateContextFromRealHandle(OMX_HANDLETYPE hRealComponentHandle);

OMX_PTR OmxrAdapterBase_GetPrivateContextFromAdapterHandle(OMX_HANDLETYPE hAdapterComponentHandle);

OMX_ERRORTYPE OmxrAdapterBase_SetPrivateContext(OMX_HANDLETYPE hAdapterComponentHandle, OMX_PTR pvPrivateContext);

OMX_ERRORTYPE OmxrAdapterBase_InitStructureHeader(OMX_PTR pvStructure, OMX_U32 u32Size);

#ifdef __cplusplus
}
#endif

#endif /* __OMXR_ADAPTER_BASE_H__ */
