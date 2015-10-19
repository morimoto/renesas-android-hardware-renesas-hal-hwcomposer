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
 * OpenMAX IL Macro API base functions for video decoder component
 *
 * @file
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <omxr_adapter_base.h>
#include <omxr_adapter_log.h>

#include <OMXR_Extension_android.h> /* for Android extension index */

#include <hal_public.h>
#include <gralloc_custom.h>

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define OMXR_ADAPTER_VIDEO_INPUT_PORT_INDEX (0)
#define OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX (1)

#define OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_OMX   (OMX_COLOR_FormatYUV420SemiPlanar)
#define OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_HAL   (HAL_PIXEL_FORMAT_NV12)
#define OMXR_ADAPTER_VIDEO_DECODER_ANDROID_NAVIVE_BUFFER_USAGE (GRALLOC_USAGE_HW_RENDER)

#define OMXR_ADAPTER_VIDEO_DECODER_STRING_ENABLE_ANDROID_NATIVE_BUFFERS   "OMX.google.android.index.enableAndroidNativeBuffers"
#define OMXR_ADAPTER_VIDEO_DECODER_STRING_USE_ANDROID_NATIVE_BUFFER2      "OMX.google.android.index.useAndroidNativeBuffer2"
#define OMXR_ADAPTER_VIDEO_DECODER_STRING_GET_ANDROID_NATIVE_BUFFER_USAGE "OMX.google.android.index.getAndroidNativeBufferUsage"

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef struct OMXR_ADAPTER_VIDEO_DECODER_CONTEXT {
	struct {
		OMX_BOOL bEnableNativeBuffer;
		OMX_COLOR_FORMATTYPE eDefaultColorFormat;
		OMX_U32 u32FrameWidth;
		OMX_U32 u32FrameHeight;
	} sOutputParam;
} OMXR_ADAPTER_VIDEO_DECODER_CONTEXT;

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_ComponentDeInit(
	OMX_HANDLETYPE hComponent);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_SendCommand(
	OMX_HANDLETYPE  hComponent,
	OMX_COMMANDTYPE sCommand,
	OMX_U32         u32Param,
	OMX_PTR         pvCmdData);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetParameter(
	OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_SetParameter(
	OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetExtensionIndex(
	OMX_HANDLETYPE hComponent,
	OMX_STRING     pcParameterName,
	OMX_INDEXTYPE  *psIndexType);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_UseBuffer(
	OMX_HANDLETYPE       hComponent,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes,
	OMX_U8               *pBuffer);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FreeBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_U32              u32PortIndex,
	OMX_BUFFERHEADERTYPE *psBuffer);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_EmptyThisBuffer(
	OMX_HANDLETYPE       hComponent,
	OMX_BUFFERHEADERTYPE *psBuffer);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FillThisBuffer(
	OMX_HANDLETYPE       hComponent,
	OMX_BUFFERHEADERTYPE *psBuffer);

OMX_ERRORTYPE OmxrAdapterVideoDecoder_EventHandler(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_EVENTTYPE eEvent,
	OMX_U32 nData1,
	OMX_U32 nData2,
	OMX_PTR pEventData);

OMX_ERRORTYPE OmxrAdapterVideoDecoder_EmptyBufferDone(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE OmxrAdapterVideoDecoder_FillBufferDone(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_BUFFERHEADERTYPE* pBuffer);

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_CopyBufferInformation(OMX_BUFFERHEADERTYPE *psDst, OMX_BUFFERHEADERTYPE *psSrc);

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
OMX_ERRORTYPE OmxrAdapterVideoDecoder_ComponentInit(
	OMX_HANDLETYPE   *pAdapterComponentHandle,
	OMX_STRING       cComponentName,
	OMX_PTR          pAppData,
	OMX_CALLBACKTYPE *psClientCallBacks,
	OMX_CALLBACKTYPE *psOverrideCallBacks)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMX_CALLBACKTYPE sCallBacks;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext = NULL;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == pAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	/* allocate adapter private area */
	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)malloc(sizeof(OMXR_ADAPTER_VIDEO_DECODER_CONTEXT));
	if(psPrivateContext == NULL){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "malloc error");
		return OMX_ErrorUndefined;
	}
	psPrivateContext->sOutputParam.bEnableNativeBuffer = OMX_FALSE;
	psPrivateContext->sOutputParam.eDefaultColorFormat = OMX_COLOR_FormatYUV420Planar;
	psPrivateContext->sOutputParam.u32FrameWidth  = 176;
	psPrivateContext->sOutputParam.u32FrameHeight = 144;

	/* override callbacks from real component */
	sCallBacks.EventHandler    = &OmxrAdapterVideoDecoder_EventHandler;
	sCallBacks.EmptyBufferDone = &OmxrAdapterVideoDecoder_EmptyBufferDone;
	sCallBacks.FillBufferDone  = &OmxrAdapterVideoDecoder_FillBufferDone;
	
	eError = OmxrAdapterBase_ComponentInit(pAdapterComponentHandle, cComponentName, pAppData, psClientCallBacks, &sCallBacks);
	if(OMX_ErrorNone != eError){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "OmxrAdapterBase_ComponentInit failed H'%08x", eError);
		free(psPrivateContext);
		/* TODO : cleanup */
		return eError;
	}

	psAdapterComponent = (OMX_COMPONENTTYPE *)*pAdapterComponentHandle;

	eError = OmxrAdapterBase_SetPrivateContext(*pAdapterComponentHandle, psPrivateContext);

	/* override macro api functions */
	psAdapterComponent->GetParameter      = OmxrAdapterVideoDecoder_GetParameter;
	psAdapterComponent->SetParameter      = OmxrAdapterVideoDecoder_SetParameter;
	psAdapterComponent->GetExtensionIndex = OmxrAdapterVideoDecoder_GetExtensionIndex;
	psAdapterComponent->UseBuffer         = OmxrAdapterVideoDecoder_UseBuffer;
	psAdapterComponent->FreeBuffer        = OmxrAdapterVideoDecoder_FreeBuffer;
	psAdapterComponent->EmptyThisBuffer   = OmxrAdapterVideoDecoder_EmptyThisBuffer;
	psAdapterComponent->FillThisBuffer    = OmxrAdapterVideoDecoder_FillThisBuffer;
	psAdapterComponent->ComponentDeInit   = OmxrAdapterVideoDecoder_ComponentDeInit;

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
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_ComponentDeInit(
	OMX_HANDLETYPE hAdapterComponentHandle)
{
	OMX_ERRORTYPE     eError;
	OMX_COMPONENTTYPE *psComponent;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext = NULL;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "hAdapterComponentHandle = H'%08x", hAdapterComponentHandle);

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(psPrivateContext){
		free(psPrivateContext);
	}

	return OmxrAdapterBase_ComponentDeInit(hAdapterComponentHandle);
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetParameterAndroidNativeBufferUsage(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_PARAM_ANDROID_NATIVEBUFFERUSAGE *psUsage;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	psUsage = (OMX_PARAM_ANDROID_NATIVEBUFFERUSAGE*)pvParameterStructure;

	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == psUsage->nPortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		psUsage->nUsage = OMXR_ADAPTER_VIDEO_DECODER_ANDROID_NAVIVE_BUFFER_USAGE;
		return OMX_ErrorNone;
	}else{
		return OMX_ErrorUnsupportedIndex;
	}
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetParameterPortDefinition(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_PARAM_PORTDEFINITIONTYPE *psPortDef;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	eError = OmxrAdapterBase_GetParameter(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	if(OMX_ErrorNone != eError){
		return eError;
	}

	psPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pvParameterStructure;
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == psPortDef->nPortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		/* replace color format type */
		if(psPortDef->format.video.eColorFormat == OMX_COLOR_FormatYUV420SemiPlanar){
			/* 0x100 is HAL_PIXEL_FORMAT_NV12_MEDIA */
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "replace color format yuv420semiplanar to nv12 media");
			psPortDef->format.video.eColorFormat = OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_HAL;
		} else /*if(pPortDef->format.video.eColorFormat == OMX_COLOR_FormatYUV420Planar)*/{
		}

		/* nFrameHeight must be same as nSliceHeight. choose the bigger one. */
		psPortDef->format.video.nFrameHeight = (psPortDef->format.video.nFrameHeight > psPortDef->format.video.nSliceHeight)? psPortDef->format.video.nFrameHeight: psPortDef->format.video.nSliceHeight;

		/* save frame size. The size information needed at gralloc lock */
		psPrivateContext->sOutputParam.u32FrameWidth   = psPortDef->format.video.nFrameWidth;
		psPrivateContext->sOutputParam.u32FrameHeight  = psPortDef->format.video.nFrameHeight;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetParameterVideoPortFormat(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_VIDEO_PARAM_PORTFORMATTYPE *psPortParam;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	eError = OmxrAdapterBase_GetParameter(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	if(OMX_ErrorNone != eError){
		return eError;
	}

	psPortParam = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pvParameterStructure;
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == psPortParam->nPortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		if(psPortParam->eColorFormat == OMX_COLOR_FormatYUV420SemiPlanar){
			/* 0x100 is HAL_PIXEL_FORMAT_NV12_MEDIA */
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "replace color format yuv420semiplanar to nv12");
			psPortParam->eColorFormat = OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_HAL;
		}
	}
	return eError;
}

/***************************************************************************
 * SUMMARY: 	Get Parameter Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetParameter(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE     eError;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	if(sIndexType == (OMX_INDEXTYPE)OMX_IndexAndroidNativeBufferUsage){
		return OmxrAdapterVideoDecoder_GetParameterAndroidNativeBufferUsage(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}else if(sIndexType == OMX_IndexParamPortDefinition){
		return OmxrAdapterVideoDecoder_GetParameterPortDefinition(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}else if(sIndexType == OMX_IndexParamVideoPortFormat){
		return OmxrAdapterVideoDecoder_GetParameterVideoPortFormat(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}else {
		return OmxrAdapterBase_GetParameter(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_SetParameterEnableAndroidNativeBuffer(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_PARAM_ANDROID_ENABLENATIVEBUFFERS *psEnableNativeBuffers;
	OMX_PARAM_PORTDEFINITIONTYPE sPortDef;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	psEnableNativeBuffers = (OMX_PARAM_ANDROID_ENABLENATIVEBUFFERS *)pvParameterStructure;

	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX != psEnableNativeBuffers->nPortIndex){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "not supported port index");
		return OMX_ErrorUnsupportedSetting;
	}

	/* Fit eColorFormat in OMX_PARAM_PORTDEFINITIONTYPE for output port to surface format */
	OmxrAdapterBase_InitStructureHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

	sPortDef.nPortIndex = psEnableNativeBuffers->nPortIndex;

	eError = OmxrAdapterBase_GetParameter(hAdapterComponentHandle, OMX_IndexParamPortDefinition, &sPortDef);
	if(OMX_ErrorNone != eError ){
		return eError;
	}

	if(OMX_TRUE == psEnableNativeBuffers->enable){
		/* keep the color format to change back when psEnableNativeBuffers is disabled */
		if(OMX_FALSE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
			psPrivateContext->sOutputParam.eDefaultColorFormat = sPortDef.format.video.eColorFormat;
		}
		sPortDef.format.video.eColorFormat = OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_OMX;

		/* Fit nStride and nSliceHeight to HAL alignment restriction */
		sPortDef.format.video.nStride = ALIGN(psPrivateContext->sOutputParam.u32FrameWidth, HW_ALIGN);
		sPortDef.format.video.nSliceHeight = psPrivateContext->sOutputParam.u32FrameHeight;
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "frame size (%ld x %ld )\n", sPortDef.format.video.nFrameWidth, sPortDef.format.video.nFrameHeight);
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "align size (%ld x %ld )\n", sPortDef.format.video.nStride, sPortDef.format.video.nSliceHeight);
	}else{
		if(OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
			sPortDef.format.video.eColorFormat = psPrivateContext->sOutputParam.eDefaultColorFormat;
		}
	}

	eError = OmxrAdapterBase_SetParameter(hAdapterComponentHandle,OMX_IndexParamPortDefinition, &sPortDef);
	if(OMX_ErrorNone != eError ){
		return eError;
	}

	/* update parameter */
	psPrivateContext->sOutputParam.bEnableNativeBuffer = psEnableNativeBuffers->enable;

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_SetParameterPortDefinition(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_PARAM_PORTDEFINITIONTYPE *psPortDef;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	psPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pvParameterStructure;
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == psPortDef->nPortIndex){
		/* save frame size. The size information needed at gralloc lock */
		psPrivateContext->sOutputParam.u32FrameWidth   = psPortDef->format.video.nFrameWidth;
		psPrivateContext->sOutputParam.u32FrameHeight  = psPortDef->format.video.nFrameHeight;

		if(OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
			/* Fit nStride and nSliceHeight to HAL alignment restriction */
			psPortDef->format.video.nStride = ALIGN(psPrivateContext->sOutputParam.u32FrameWidth, HW_ALIGN);
			psPortDef->format.video.nSliceHeight = psPrivateContext->sOutputParam.u32FrameHeight;
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "frame size (%ld x %ld )\n", psPortDef->format.video.nFrameWidth,psPortDef->format.video.nFrameHeight);
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "align size (%ld x %ld )\n", psPortDef->format.video.nStride,psPortDef->format.video.nSliceHeight);
		}

		if( OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer && OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_HAL == psPortDef->format.video.eColorFormat ){
			/* 0x100 is HAL_PIXEL_FORMAT_NV12_MEDIA */
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "replace color format nv12 to yuv420semiplanar");
			psPortDef->format.video.eColorFormat = OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_OMX;
		}

	}

	return OmxrAdapterBase_SetParameter(hAdapterComponentHandle, sIndexType, pvParameterStructure);
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_SetParameterVideoPortFormat(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_VIDEO_PARAM_PORTFORMATTYPE *psPortFormat;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	psPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pvParameterStructure;
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == psPortFormat->nPortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		if( OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_HAL == psPortFormat->eColorFormat ){
			/* 0x100 is HAL_PIXEL_FORMAT_NV12_MEDIA */
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "replace color format nv12 to yuv420semiplanar");
			psPortFormat->eColorFormat = OMXR_ADAPTER_VIDEO_DECODER_OUTPUT_FORMAT_OMX;
		}
	}

	return OmxrAdapterBase_SetParameter(hAdapterComponentHandle, sIndexType, pvParameterStructure);
}

/***************************************************************************
 * SUMMARY: 	Set Parameter Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_SetParameter(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_INDEXTYPE  sIndexType,
	OMX_PTR        pvParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "");

	if(NULL == hAdapterComponentHandle || NULL == pvParameterStructure){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	if(sIndexType == (OMX_INDEXTYPE)OMX_IndexEnableAndroidNativeBuffer){
		return OmxrAdapterVideoDecoder_SetParameterEnableAndroidNativeBuffer(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}else if(sIndexType == OMX_IndexParamPortDefinition){
		return OmxrAdapterVideoDecoder_SetParameterPortDefinition(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}else if(sIndexType == OMX_IndexParamVideoPortFormat){
		return OmxrAdapterVideoDecoder_SetParameterVideoPortFormat(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}else{
		return OmxrAdapterBase_SetParameter(hAdapterComponentHandle, sIndexType, pvParameterStructure);
	}
}

/***************************************************************************
 * SUMMARY: 	Get Extension Index Function
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_GetExtensionIndex(
	OMX_HANDLETYPE hAdapterComponentHandle,
	OMX_STRING     pcParameterName,
	OMX_INDEXTYPE  *psIndexType)
{
	if(NULL == hAdapterComponentHandle || NULL == psIndexType || NULL == pcParameterName){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_PARAM, "%s", pcParameterName);

	if(!strncmp(pcParameterName, OMXR_ADAPTER_VIDEO_DECODER_STRING_ENABLE_ANDROID_NATIVE_BUFFERS, strlen(OMXR_ADAPTER_VIDEO_DECODER_STRING_ENABLE_ANDROID_NATIVE_BUFFERS))){
		*psIndexType = (OMX_INDEXTYPE)OMX_IndexEnableAndroidNativeBuffer;
		return OMX_ErrorNone;
	} else if(!strncmp(pcParameterName, OMXR_ADAPTER_VIDEO_DECODER_STRING_USE_ANDROID_NATIVE_BUFFER2, strlen(OMXR_ADAPTER_VIDEO_DECODER_STRING_USE_ANDROID_NATIVE_BUFFER2))){
		*psIndexType = (OMX_INDEXTYPE)NULL;
		return OMX_ErrorNone;
	} else if(!strncmp(pcParameterName, OMXR_ADAPTER_VIDEO_DECODER_STRING_GET_ANDROID_NATIVE_BUFFER_USAGE, strlen(OMXR_ADAPTER_VIDEO_DECODER_STRING_GET_ANDROID_NATIVE_BUFFER_USAGE))){
		*psIndexType = (OMX_INDEXTYPE)OMX_IndexAndroidNativeBufferUsage;
		return OMX_ErrorNone;
	} else {
		return OmxrAdapterBase_GetExtensionIndex(hAdapterComponentHandle, pcParameterName, psIndexType);
	}
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_UseBufferAndroidNativeBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes,
	OMX_U8               *pBuffer)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *psRealBuffer;
	OMX_BUFFERHEADERTYPE *psAdapterBuffer;

	int ret;
	hw_module_t const *module = NULL;
	IMG_gralloc_module_public_t *img_gralloc_module = NULL;
	IMG_native_handle_t         *img_native_handle = NULL;
	OMX_U32 u32HardwareAddress;
	OMX_U64 u64PhysicalAddress;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "pBuffer=H'%08x", pBuffer);

	if(NULL == hAdapterComponentHandle || NULL == ppsBuffer ){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	/* get gralloc moudles */
	ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
	if (0 == ret){
		img_gralloc_module = ((IMG_gralloc_module_public_t *)module);
	}else{
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Can't find %s module err = 0x%x", GRALLOC_HARDWARE_MODULE_ID, ret);
		return OMX_ErrorUndefined;
	}

	img_native_handle = (IMG_native_handle_t *)pBuffer;

	ret = img_gralloc_module->GetPhysAddr(img_gralloc_module, img_native_handle->fd[0], (unsigned long long*)&u64PhysicalAddress);
	if (ret != 0){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "gralloc api GetPhysAddr() returned error 0x%x", ret);
		return OMX_ErrorUndefined;
	}
#ifdef USE_IPMMU
	u32HardwareAddress = mmngr_ipmmu_phys_to_virt( u64PhysicalAddress );
#else
	u32HardwareAddress = (unsigned long)u64PhysicalAddress;
#endif

	psAdapterBuffer = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
	if(NULL == psAdapterBuffer)
	{
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "malloc error");
		return OMX_ErrorUndefined;
	}

	/* 
		psRealBuffer->pAppPrivate = psAdapterBuffer;
		psRealBuffer->pPlatformPrivate : Real Component internal use
		psAdapterBuffer->pPlatformPrivate = psRealBuffer;
		psAdapterBuffer->pAppPrivate : IL Client use
	*/
	eError = OmxrAdapterBase_UseBuffer(hAdapterComponentHandle, &psRealBuffer, u32PortIndex, psAdapterBuffer, u32SizeBytes, (OMX_U8 *)u32HardwareAddress);
	if(OMX_ErrorNone != eError){
		free(psAdapterBuffer);
		return eError;
	}
	memcpy(psAdapterBuffer, psRealBuffer, sizeof(OMX_BUFFERHEADERTYPE));

	psAdapterBuffer->pAppPrivate = pvAppPrivate;
	psAdapterBuffer->pBuffer = pBuffer; /* pBuffer means IMG_native_handle_t */
	psAdapterBuffer->pPlatformPrivate = (OMX_PTR)psRealBuffer;
	
	*ppsBuffer = psAdapterBuffer;
/*
	MSG_INFO("[OMXR]   |[%s] allocate buffer infomation \n",__func__);
	MSG_INFO("[OMXR]   | pBuffer(sys)         = 0x%08lx \n" ,(OMX_U32)psBuffer->pBuffer );
	MSG_INFO("[OMXR]   | pBuffer(rt )         = 0x%08lx \n" ,sRet.pBuffer);
	MSG_INFO("[OMXR]   | nSize                = %ld \n"     ,psBuffer->nSize);
	MSG_INFO("[OMXR]   | nVersion             = 0x%08lx \n" ,psBuffer->nVersion.nVersion);
	MSG_INFO("[OMXR]   | nAllocLen            = %ld \n"     ,psBuffer->nAllocLen);
	MSG_INFO("[OMXR]   | pAppPrivate          = 0x%lx \n"   ,(OMX_U32)psBuffer->pAppPrivate);
	MSG_INFO("[OMXR]   | pPlatformPrivate     = 0x%lx \n"   ,(OMX_U32)psBuffer->pPlatformPrivate);
	MSG_INFO("[OMXR]   | hMarkTargetComponent = 0x%lx \n"   ,(OMX_U32)psBuffer->hMarkTargetComponent );
	MSG_INFO("[OMXR]   | nOutputPortIndex     = %ld \n"     ,psBuffer->nOutputPortIndex);
	MSG_INFO("[OMXR]   | nInputPortIndex      = %ld \n"     ,psBuffer->nInputPortIndex);
*/
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	Use Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_UseBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE **ppsBuffer,
	OMX_U32              u32PortIndex,
	OMX_PTR              pvAppPrivate,
	OMX_U32              u32SizeBytes,
	OMX_U8               *pBuffer)
{
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == hAdapterComponentHandle){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}
	
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == u32PortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		return OmxrAdapterVideoDecoder_UseBufferAndroidNativeBuffer(hAdapterComponentHandle, ppsBuffer, u32PortIndex, pvAppPrivate, u32SizeBytes, pBuffer);
	}else{
		return OmxrAdapterBase_UseBuffer(hAdapterComponentHandle, ppsBuffer, u32PortIndex, pvAppPrivate, u32SizeBytes, pBuffer);
	}
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FreeBufferAndroidNativeBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_U32              u32PortIndex,
	OMX_BUFFERHEADERTYPE *psAdapterBuffer)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *psRealBuffer;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == hAdapterComponentHandle || NULL == psAdapterBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psRealBuffer = (OMX_BUFFERHEADERTYPE *)psAdapterBuffer->pPlatformPrivate;

	eError = OmxrAdapterBase_FreeBuffer(hAdapterComponentHandle, u32PortIndex, psRealBuffer);

	free(psAdapterBuffer);
	
	return eError;
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FreeBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_U32              u32PortIndex,
	OMX_BUFFERHEADERTYPE *psBuffer)
{
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == hAdapterComponentHandle || NULL == psBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}
	
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == u32PortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		return OmxrAdapterVideoDecoder_FreeBufferAndroidNativeBuffer(hAdapterComponentHandle, u32PortIndex, psBuffer);
	}else{
		return OmxrAdapterBase_FreeBuffer(hAdapterComponentHandle, u32PortIndex, psBuffer);
	}
}

/***************************************************************************
 * SUMMARY: 	Empty This Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_EmptyThisBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE *psBuffer)
{
	return OmxrAdapterBase_EmptyThisBuffer(hAdapterComponentHandle, psBuffer);
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FillThisBufferNativeBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE *psAdapterBuffer)
{
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	OMX_BUFFERHEADERTYPE *psRealBuffer;

	int ret;
	native_handle_t  *native_handle = NULL;
	hw_module_t const *module = NULL;
	gralloc_module_t *gralloc_module =NULL;
	void *pvAddr;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == hAdapterComponentHandle || NULL == psAdapterBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}
	
	/* get gralloc moudles */
	ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
	if (ret != 0){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "gralloc api hw_get_module() returned error");
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Can't find %s module err = 0x%x", GRALLOC_HARDWARE_MODULE_ID, ret);
		return OMX_ErrorUndefined;
	}
	gralloc_module = ((gralloc_module_t *)module);

	native_handle = (native_handle_t*)psAdapterBuffer->pBuffer;
	ret = gralloc_module->lock(gralloc_module, native_handle, OMXR_ADAPTER_VIDEO_DECODER_ANDROID_NAVIVE_BUFFER_USAGE, 0, 0, psPrivateContext->sOutputParam.u32FrameWidth, psPrivateContext->sOutputParam.u32FrameHeight, &pvAddr);
	if(ret != 0){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "gralloc api lock() returned error 0x%x", ret);
		return OMX_ErrorUndefined;
	}

        OMXR_ADAPTER_LOGGER(ADAPTER_LOG_BUFFER, "psBuffer=H'%08x, pBuffer=H'%08x", psAdapterBuffer, psAdapterBuffer->pBuffer);

	psRealBuffer = (OMX_BUFFERHEADERTYPE *)psAdapterBuffer->pPlatformPrivate;

	return OmxrAdapterBase_FillThisBuffer(hAdapterComponentHandle, psRealBuffer);
}

/***************************************************************************
 * SUMMARY: 	Fill This Buffer Function.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FillThisBuffer(
	OMX_HANDLETYPE       hAdapterComponentHandle,
	OMX_BUFFERHEADERTYPE *psBuffer)
{
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;

	if(NULL == hAdapterComponentHandle || NULL == psBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromAdapterHandle(hAdapterComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}
	
	if(OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		return OmxrAdapterVideoDecoder_FillThisBufferNativeBuffer(hAdapterComponentHandle, psBuffer);
	}else{
		return OmxrAdapterBase_FillThisBuffer(hAdapterComponentHandle, psBuffer);
	}
}

OMX_ERRORTYPE OmxrAdapterVideoDecoder_EventHandler(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_EVENTTYPE eEvent,
	OMX_U32 nData1,
	OMX_U32 nData2,
	OMX_PTR pEventData)
{
/*
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

	psBaseContext = OmxrAdapterBase_GetContextFromRealHandle(hRealComponentHandle);
	if(NULL == psBaseContext){
		return OMX_ErrorUndefined;
	}
	psAdapterComponent = (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;
*/
	return OmxrAdapterBase_EventHandler(hRealComponentHandle, pAppData, eEvent, nData1, nData2, pEventData);
}

OMX_ERRORTYPE OmxrAdapterVideoDecoder_EmptyBufferDone(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_BUFFERHEADERTYPE* pBuffer){
/*
	OMX_COMPONENTTYPE *psAdapterComponent;
	OMXR_ADAPTER_BASE_CONTEXT *psBaseContext;

	psBaseContext = OmxrAdapterBase_GetContextFromRealHandle(hRealComponentHandle);
	if(NULL == psBaseContext){
		return OMX_ErrorUndefined;
	}
	psAdapterComponent = (OMX_COMPONENTTYPE *)psBaseContext->hAdapterComponentHandle;
*/
	return OmxrAdapterBase_EmptyBufferDone(hRealComponentHandle, pAppData, pBuffer);
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_FillBufferDoneNativeBuffer(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_BUFFERHEADERTYPE* psRealBuffer)
{
	OMX_BUFFERHEADERTYPE *psAdapterBuffer;

	native_handle_t  *native_handle = NULL;
	hw_module_t const *module = NULL;
	gralloc_module_t *gralloc_module =NULL;
	int ret;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	if(NULL == hRealComponentHandle || NULL == psRealBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psAdapterBuffer = (OMX_BUFFERHEADERTYPE *)psRealBuffer->pAppPrivate;
	if(NULL == psAdapterBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}

	/* get gralloc moudles */
	ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
	if(ret != 0){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "gralloc api hw_get_module() returned error");
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "Can't find %s module err = 0x%x", GRALLOC_HARDWARE_MODULE_ID, ret);
		return OMX_ErrorUndefined;
	}
	gralloc_module = ((gralloc_module_t *)module);

	native_handle = (native_handle_t*)psAdapterBuffer->pBuffer;

	ret = gralloc_module->unlock(gralloc_module,native_handle);
	if(ret != 0){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "gralloc api unlock() returned error 0x%x", ret);
		return OMX_ErrorUndefined;
	}
	OmxrAdapterVideoDecoder_CopyBufferInformation(psAdapterBuffer, psRealBuffer);

	return OmxrAdapterBase_FillBufferDone(hRealComponentHandle, pAppData, psAdapterBuffer);
}

OMX_ERRORTYPE OmxrAdapterVideoDecoder_FillBufferDone(
	OMX_HANDLETYPE hRealComponentHandle,
	OMX_PTR pAppData,
	OMX_BUFFERHEADERTYPE* psBuffer)
{
	OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *psPrivateContext;
	
	if(NULL == hRealComponentHandle || NULL == psBuffer){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psPrivateContext = (OMXR_ADAPTER_VIDEO_DECODER_CONTEXT *)OmxrAdapterBase_GetPrivateContextFromRealHandle(hRealComponentHandle);
	if(NULL == psPrivateContext){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad handle");
		return OMX_ErrorUndefined;
	}
	
	if(OMXR_ADAPTER_VIDEO_OUTPUT_PORT_INDEX == psBuffer->nOutputPortIndex && OMX_TRUE == psPrivateContext->sOutputParam.bEnableNativeBuffer){
		return OmxrAdapterVideoDecoder_FillBufferDoneNativeBuffer(hRealComponentHandle, pAppData, psBuffer);
	}else{
		return OmxrAdapterBase_FillBufferDone(hRealComponentHandle, pAppData, psBuffer);
	}
}

static OMX_ERRORTYPE OmxrAdapterVideoDecoder_CopyBufferInformation(OMX_BUFFERHEADERTYPE *psDst, OMX_BUFFERHEADERTYPE *psSrc){
	if(NULL == psDst || NULL == psSrc){
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}
	
	psDst->nFilledLen           = psSrc->nFilledLen;
	psDst->nOffset              = psSrc->nOffset;
	psDst->hMarkTargetComponent = psSrc->hMarkTargetComponent; /* TODO : Mark buffer should be hanlded in base function. */
	psDst->pMarkData            = psSrc->pMarkData;
	psDst->nTickCount           = psSrc->nTickCount;
	psDst->nTimeStamp           = psSrc->nTimeStamp;
	psDst->nFlags               = psSrc->nFlags;

	return OMX_ErrorNone;
}
