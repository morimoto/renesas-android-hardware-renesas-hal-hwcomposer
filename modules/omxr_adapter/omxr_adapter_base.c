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
 * OpenMAX IL Macro API base functions
 *
 * @file
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "omxr_adapter_base.h"
#include "omxr_adapter_log.h"

#include "omxr_adapter_core.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
/***************************************************************************
 * SUMMARY: 	OMX_ComponentInit
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_ComponentInit(
	OMX_HANDLETYPE   *pHandle, 
	OMX_STRING       cComponentName,
	OMX_PTR          pAppData,
	OMX_CALLBACKTYPE *psClientCallBacks,
	OMX_CALLBACKTYPE *psOverrideCallBacks)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext = NULL;
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMX_CALLBACKTYPE sCallBacks;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "");

	/* Check Argument */
	if (NULL == pHandle || NULL == cComponentName || NULL == psClientCallBacks) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	/* Allocate OMX Adapter Context */
	psBaseContext = (OMXR_ADAPTER_BASE_CONTEXT *)malloc(sizeof(OMXR_ADAPTER_BASE_CONTEXT));
	if (NULL == psBaseContext) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "malloc error");
		return OMX_ErrorUndefined;
	}
	memset(psBaseContext, 0, sizeof(OMXR_ADAPTER_BASE_CONTEXT));

	psBaseContext->hAdapterComponentHandle = (OMX_HANDLETYPE)malloc(sizeof(OMX_COMPONENTTYPE));
	if (NULL == psBaseContext->hAdapterComponentHandle) {
		free(psBaseContext);
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "malloc error");
		return OMX_ErrorUndefined;
	}

	psAdapterComponent = (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;

	psAdapterComponent->nVersion.nVersion   = OMXR_ADAPTER_SPECVERSION;
	psAdapterComponent->nSize               = sizeof(OMX_COMPONENTTYPE);
	psAdapterComponent->pComponentPrivate   = psBaseContext;
	psAdapterComponent->pApplicationPrivate = pAppData;

	psAdapterComponent->GetComponentVersion		= OmxrAdapterBase_GetComponentVersion;
	psAdapterComponent->SendCommand				= OmxrAdapterBase_SendCommand;
	psAdapterComponent->GetParameter			= OmxrAdapterBase_GetParameter;
	psAdapterComponent->SetParameter			= OmxrAdapterBase_SetParameter;
	psAdapterComponent->GetConfig				= OmxrAdapterBase_GetConfig;
	psAdapterComponent->SetConfig				= OmxrAdapterBase_SetConfig;
	psAdapterComponent->GetExtensionIndex		= OmxrAdapterBase_GetExtensionIndex;
	psAdapterComponent->GetState				= OmxrAdapterBase_GetState;
	psAdapterComponent->ComponentTunnelRequest	= OmxrAdapterBase_ComponentTunnelRequest;
	psAdapterComponent->UseBuffer				= OmxrAdapterBase_UseBuffer;
	psAdapterComponent->AllocateBuffer 			= OmxrAdapterBase_AllocateBuffer;
	psAdapterComponent->FreeBuffer 				= OmxrAdapterBase_FreeBuffer;
	psAdapterComponent->EmptyThisBuffer			= OmxrAdapterBase_EmptyThisBuffer;
	psAdapterComponent->FillThisBuffer 			= OmxrAdapterBase_FillThisBuffer;
	psAdapterComponent->SetCallbacks			= OmxrAdapterBase_SetCallbacks;
	psAdapterComponent->ComponentDeInit			= OmxrAdapterBase_ComponentDeInit;
	psAdapterComponent->UseEGLImage				= OmxrAdapterBase_UseEGLImage;
	psAdapterComponent->ComponentRoleEnum		= OmxrAdapterBase_ComponentRoleEnum;

	psBaseContext->sClientCallbacks.EventHandler    = psClientCallBacks->EventHandler;
	psBaseContext->sClientCallbacks.EmptyBufferDone = psClientCallBacks->EmptyBufferDone;
	psBaseContext->sClientCallbacks.FillBufferDone  = psClientCallBacks->FillBufferDone;

	sCallBacks.EventHandler    = &OmxrAdapterBase_EventHandler;
	sCallBacks.EmptyBufferDone = &OmxrAdapterBase_EmptyBufferDone;
	sCallBacks.FillBufferDone  = &OmxrAdapterBase_FillBufferDone;
	if(NULL != psOverrideCallBacks){
		if(NULL != psOverrideCallBacks->EventHandler){
			sCallBacks.EventHandler = psOverrideCallBacks->EventHandler;
		}
		if(NULL != psOverrideCallBacks->EmptyBufferDone){
			sCallBacks.EmptyBufferDone = psOverrideCallBacks->EmptyBufferDone;
		}
		if(NULL != psOverrideCallBacks->FillBufferDone){
			sCallBacks.FillBufferDone = psOverrideCallBacks->FillBufferDone;
		}
	}

	eError = OmxrAdapterCore_GetHandle(&psBaseContext->hRealComponentHandle, cComponentName, psBaseContext, &sCallBacks);
	if(OMX_ErrorNone != eError){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "OmxrAdapterCore_GetHandle = H'%08x", eError);
		free(psBaseContext->hAdapterComponentHandle);
		free(psBaseContext);
		return eError;
	}

	*pHandle = psBaseContext->hAdapterComponentHandle;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", OMX_ErrorNone);

	return OMX_ErrorNone;
}

/***************************************************************************/
/*	  OpenMAX IL Macro Function											   */
/***************************************************************************/
/***************************************************************************
 * SUMMARY: 	Get Component Version Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_GetComponentVersion(
	OMX_HANDLETYPE  hAdapterComponentHandle,
	OMX_STRING      pcComponentName,
	OMX_VERSIONTYPE *psComponentVersion,
	OMX_VERSIONTYPE *psSpecVersion,
	OMX_UUIDTYPE    *psComponentUUID)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->GetComponentVersion((OMX_HANDLETYPE)psRealComponent, pcComponentName, psComponentVersion, psSpecVersion, psComponentUUID);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Send Command Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_SendCommand(
	OMX_HANDLETYPE  hAdapterComponentHandle,
	OMX_COMMANDTYPE sCommand,
	OMX_U32         u32Param,
	OMX_PTR         pvCmdData)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_EVENT, "handle=H'%08x, sCommand=H'%08x, u32Param=H'%08x", hAdapterComponentHandle, sCommand, u32Param);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->SendCommand((OMX_HANDLETYPE)psRealComponent, sCommand, u32Param, pvCmdData);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Get Parameter Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_GetParameter(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, sindexType=H'%08x", hAdapterComponentHandle, sIndexType);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->GetParameter((OMX_HANDLETYPE)psRealComponent, sIndexType, pvParameterStructure);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Set Parameter Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_SetParameter(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, sindexType=H'%08x", hAdapterComponentHandle, sIndexType);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->SetParameter((OMX_HANDLETYPE)psRealComponent, sIndexType, pvParameterStructure);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Get Config Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_GetConfig(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvConfigStructure)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, sindexType=H'%08x", hAdapterComponentHandle, sIndexType);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->GetConfig((OMX_HANDLETYPE)psRealComponent, sIndexType, pvConfigStructure);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Set Config Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_SetConfig(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvConfigStructure)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, sindexType=H'%08x", hAdapterComponentHandle, sIndexType);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->SetConfig((OMX_HANDLETYPE)psRealComponent, sIndexType, pvConfigStructure);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Get Extension Index Function
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_GetExtensionIndex(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_STRING     pcParameterName,
	OMX_INDEXTYPE  *psIndexType)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle || NULL == pcParameterName || NULL == psIndexType){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "pcParameterName=%s", pcParameterName);

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->GetExtensionIndex((OMX_HANDLETYPE)psRealComponent, pcParameterName, psIndexType);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Get State Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_GetState(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_STATETYPE  *psState)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle || NULL == psState){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->GetState((OMX_HANDLETYPE)psRealComponent, psState);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Component Tunnel Request Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_ComponentTunnelRequest(
	OMX_HANDLETYPE      hAdapterComponentHandle,
	OMX_U32             nPortIndex,
	OMX_HANDLETYPE      hTunneledComp,
	OMX_U32             nTunneledPort,
	OMX_TUNNELSETUPTYPE *pTunnelSetup)
{
	/* This function is not called in OMX adapter layer. */
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Not Implemented");

	return OMX_ErrorNotImplemented;
}

/***************************************************************************
 * SUMMARY: 	Use Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_UseBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes,
	OMX_U8               *pBuffer)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, u32PortIndex=%d, u32SizeBytes=%d, pBuffer=H'%08x", hAdapterComponentHandle, u32PortIndex, u32SizeBytes, pBuffer);

	if(NULL == hAdapterComponentHandle || NULL == ppsBuffer || NULL == pBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->UseBuffer((OMX_HANDLETYPE)psRealComponent, ppsBuffer, u32PortIndex, pvAppPrivate, u32SizeBytes, pBuffer);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Allocate Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_AllocateBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, u32PortIndex=%d, u32SizeBytes=%d", hAdapterComponentHandle, u32PortIndex, u32SizeBytes);

	if(NULL == hAdapterComponentHandle || NULL == ppsBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->AllocateBuffer((OMX_HANDLETYPE)psRealComponent, ppsBuffer, u32PortIndex, pvAppPrivate, u32SizeBytes);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Free buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_FreeBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_U32              u32PortIndex,
	OMX_BUFFERHEADERTYPE *psBuffer)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, u32PortIndex=%d, psBuffer=H'%08x", hAdapterComponentHandle, u32PortIndex, psBuffer);

	if(NULL == hAdapterComponentHandle || NULL == psBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->FreeBuffer((OMX_HANDLETYPE)psRealComponent, u32PortIndex, psBuffer);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Empty This Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_EmptyThisBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE *psBuffer)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle || NULL == psBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_BUFFER, "psBuffer=H'%08x, pBuffer=H'%08x, nFilledLen=%d, nOffset=%d, nTimeStamp=%lld, nFlags=H'%08x", psBuffer, psBuffer->pBuffer, psBuffer->nFilledLen, psBuffer->nOffset, psBuffer->nTimeStamp, psBuffer->nFlags);

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->EmptyThisBuffer((OMX_HANDLETYPE)psRealComponent, psBuffer);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Fill This Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_FillThisBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE *psBuffer)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle || NULL == psBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_BUFFER, "psBuffer=H'%08x, pBuffer=H'%08x", psBuffer, psBuffer->pBuffer);

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->FillThisBuffer((OMX_HANDLETYPE)psRealComponent, psBuffer);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Set Callback Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_SetCallbacks(
	OMX_HANDLETYPE   hAdapterComponentHandle,
	OMX_CALLBACKTYPE *psCallbacks,
	OMX_PTR          pvAppData)
{
    OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
    OMX_COMPONENTTYPE *psAdapterComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle || NULL == psCallbacks){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psBaseContext = OmxrAdapterBase_GetContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psBaseContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}
	psAdapterComponent = (OMX_COMPONENTTYPE *)hAdapterComponentHandle;
	psAdapterComponent->pApplicationPrivate = pvAppData;

	psBaseContext->sClientCallbacks.EventHandler    = psCallbacks->EventHandler;
	psBaseContext->sClientCallbacks.EmptyBufferDone = psCallbacks->EmptyBufferDone;
	psBaseContext->sClientCallbacks.FillBufferDone  = psCallbacks->FillBufferDone;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", OMX_ErrorNone);

	return OMX_ErrorNone;
}

/***************************************************************************
 * SUMMARY: 	Compnent Deinit Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_ComponentDeInit(
	OMX_HANDLETYPE hAdapterComponentHandle)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psBaseContext = OmxrAdapterBase_GetContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psBaseContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = OmxrAdapterCore_FreeHandle((OMX_HANDLETYPE)psBaseContext->hRealComponentHandle);

	if(psBaseContext->hAdapterComponentHandle){
		free(psBaseContext->hAdapterComponentHandle);
	}
	free(psBaseContext);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Use EGL Image Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_UseEGLImage(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE **ppBufferHdr,
	OMX_U32              nPortIndex,
	OMX_PTR              pAppPrivate,
	void                 *eglImage)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x", hAdapterComponentHandle);

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->UseEGLImage((OMX_HANDLETYPE)psRealComponent, ppBufferHdr, nPortIndex, pAppPrivate, eglImage);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}


/***************************************************************************
 * SUMMARY: 	Component Role Enum Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_ERRORTYPE OmxrAdapterBase_ComponentRoleEnum(
	OMX_HANDLETYPE  hAdapterComponentHandle,
	OMX_U8          *cRole,
	OMX_U32         nIndex)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *psRealComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "handle=H'%08x, nIndex=%d", hAdapterComponentHandle, nIndex);

	if(NULL == hAdapterComponentHandle || NULL == cRole){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealComponent = OmxrAdapterBase_GetRealComponentFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psRealComponent){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
		return OMX_ErrorUndefined;
	}

	eError = psRealComponent->ComponentRoleEnum((OMX_HANDLETYPE)psRealComponent, cRole, nIndex);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x, cRole=%s", eError, cRole);

	return eError;
}

OMX_ERRORTYPE OmxrAdapterBase_EventHandler(
    OMX_HANDLETYPE hRealComponentHandle,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 nData1,
    OMX_U32 nData2,
    OMX_PTR pEventData){

    OMX_COMPONENTTYPE *psAdapterComponent;
    OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

    psBaseContext = OmxrAdapterBase_GetContextFromRealHandle(hRealComponentHandle);
    if(NULL == psBaseContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
    	return OMX_ErrorUndefined;
    }
    psAdapterComponent = (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_EVENT, "eEvent=H'%08x, nData1=H'%08x, nData2=H'%08x", eEvent, nData1, nData2);
    
    return psBaseContext->sClientCallbacks.EventHandler(psBaseContext->hAdapterComponentHandle, psAdapterComponent->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
}

OMX_ERRORTYPE OmxrAdapterBase_EmptyBufferDone(
    OMX_HANDLETYPE hRealComponentHandle,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer){

    OMX_COMPONENTTYPE *psAdapterComponent;
    OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

    psBaseContext = OmxrAdapterBase_GetContextFromRealHandle(hRealComponentHandle);
    if(NULL == psBaseContext || NULL == pBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
    	return OMX_ErrorUndefined;
    }
    psAdapterComponent = (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_BUFFER, "psBuffer=H'%08x, pBuffer=H'%08x, nFilledLen=%d", pBuffer, pBuffer->pBuffer, pBuffer->nFilledLen);
    
    return psBaseContext->sClientCallbacks.EmptyBufferDone(psBaseContext->hAdapterComponentHandle, psAdapterComponent->pApplicationPrivate, pBuffer);
}

OMX_ERRORTYPE OmxrAdapterBase_FillBufferDone(
    OMX_HANDLETYPE hRealComponentHandle,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer){

    OMX_COMPONENTTYPE *psAdapterComponent;
    OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

    psBaseContext = OmxrAdapterBase_GetContextFromRealHandle(hRealComponentHandle);
    if(NULL == psBaseContext || NULL == pBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Bad handle");
    	return OMX_ErrorUndefined;
    }
    psAdapterComponent = (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_BUFFER, "psBuffer=H'%08x, pBuffer=H'%08x, nFilledLen=%d, nOffset=%d, nTimeStamp=%lld, nFlags=H'%08x", pBuffer, pBuffer->pBuffer, pBuffer->nFilledLen, pBuffer->nOffset, pBuffer->nTimeStamp, pBuffer->nFlags);
    
    return psBaseContext->sClientCallbacks.FillBufferDone(psBaseContext->hAdapterComponentHandle, psAdapterComponent->pApplicationPrivate, pBuffer);
}

OMX_COMPONENTTYPE *OmxrAdapterBase_GetAdapterComponentFromRealHandle(OMX_HANDLETYPE hRealComponentHandle){
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMX_COMPONENTTYPE *psRealComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hRealComponentHandle){
		return NULL;
	}
	psBaseContext = OmxrAdapterBase_GetContextFromRealHandle(hRealComponentHandle);

	return (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;
}

OMX_COMPONENTTYPE *OmxrAdapterBase_GetRealComponentFromAdapterHandle(OMX_HANDLETYPE hAdapterComponentHandle){
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMX_COMPONENTTYPE *psRealComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hAdapterComponentHandle){
		return NULL;
	}
	psBaseContext = OmxrAdapterBase_GetContextFromAdapterHandle(hAdapterComponentHandle);

	return (OMX_COMPONENTTYPE *)psBaseContext->hRealComponentHandle;
}

OMXR_ADAPTER_BASE_CONTEXT *OmxrAdapterBase_GetContextFromRealHandle(OMX_HANDLETYPE hRealComponentHandle){
	OMX_COMPONENTTYPE *psRealComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hRealComponentHandle){
		return NULL;
	}
	psRealComponent = (OMX_COMPONENTTYPE *)hRealComponentHandle;
	psBaseContext = (OMXR_ADAPTER_BASE_CONTEXT *)psRealComponent->pApplicationPrivate;

	return psBaseContext;
}

OMXR_ADAPTER_BASE_CONTEXT *OmxrAdapterBase_GetContextFromAdapterHandle(OMX_HANDLETYPE hAdapterComponentHandle){
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hAdapterComponentHandle){
		return NULL;
	}
	psAdapterComponent = (OMX_COMPONENTTYPE *)hAdapterComponentHandle;
	psBaseContext = (OMXR_ADAPTER_BASE_CONTEXT *)psAdapterComponent->pComponentPrivate;

	return psBaseContext;
}

OMX_PTR OmxrAdapterBase_GetPrivateContextFromRealHandle(OMX_HANDLETYPE hRealComponentHandle){
	OMX_COMPONENTTYPE *psRealComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hRealComponentHandle){
		return NULL;
	}
	psRealComponent = (OMX_COMPONENTTYPE *)hRealComponentHandle;
	psBaseContext = (OMXR_ADAPTER_BASE_CONTEXT *)psRealComponent->pApplicationPrivate;

	return psBaseContext->pvAdapterPrivate;
}

OMX_PTR OmxrAdapterBase_GetPrivateContextFromAdapterHandle(OMX_HANDLETYPE hAdapterComponentHandle){
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hAdapterComponentHandle){
		return NULL;
	}
	psAdapterComponent = (OMX_COMPONENTTYPE *)hAdapterComponentHandle;
	psBaseContext = (OMXR_ADAPTER_BASE_CONTEXT *)psAdapterComponent->pComponentPrivate;

	return psBaseContext->pvAdapterPrivate;
}

OMX_ERRORTYPE OmxrAdapterBase_SetPrivateContext(OMX_HANDLETYPE hAdapterComponentHandle, OMX_PTR pvPrivateContext){
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;
	
	if(NULL == hAdapterComponentHandle || pvPrivateContext == NULL){
		return OMX_ErrorBadParameter;
	}
	psAdapterComponent = (OMX_COMPONENTTYPE *)hAdapterComponentHandle;
	psBaseContext = (OMXR_ADAPTER_BASE_CONTEXT *)psAdapterComponent->pComponentPrivate;

	psBaseContext->pvAdapterPrivate = pvPrivateContext;

	return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxrAdapterBase_InitStructureHeader(OMX_PTR pvStructure, OMX_U32 u32Size){
	OMX_PARAM_U32TYPE *psStruct;

	if(NULL == pvStructure){
		return OMX_ErrorBadParameter;
	}
	psStruct = (OMX_PARAM_U32TYPE *)pvStructure;
    psStruct->nSize = u32Size;
    psStruct->nVersion.nVersion = OMXR_ADAPTER_SPECVERSION;

	return OMX_ErrorNone;
}

