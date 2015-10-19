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
 * OpenMAX IL Core API implementation
 *
 * @file
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "omxr_adapter_core.h"
#include "omxr_adapter_log.h"

#include "omxr_adapter_table.h"

#include <dlfcn.h>
#include <stdlib.h>

#ifdef ANDROID
#include <cutils/properties.h>
#endif /* ANDROID */

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define OMX_CORE_LIBRARY_NAME    "libomxr_core.so"

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef OMX_ERRORTYPE (*InitFunc)();
typedef OMX_ERRORTYPE (*DeinitFunc)();
typedef OMX_ERRORTYPE (*ComponentNameEnumFunc)(
			OMX_STRING, OMX_U32, OMX_U32);
typedef OMX_ERRORTYPE (*GetHandleFunc)(
			OMX_HANDLETYPE *, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);
typedef OMX_ERRORTYPE (*FreeHandleFunc)(
			OMX_HANDLETYPE *);
typedef OMX_ERRORTYPE (*GetComponentsOfRoleFunc)(
			OMX_STRING, OMX_U32 *, OMX_U8 **);
typedef OMX_ERRORTYPE (*GetRolesOfComponentFunc)(
			OMX_STRING, OMX_U32 *, OMX_U8 **);
typedef OMX_ERRORTYPE (*SetLogModeFunc)(
			OMX_U32);

/* OMX Core(OMX Service API Core) handle */
typedef struct OMXR_ADAPTER_CORE_HANDLE {
	OMX_U32                 u32InitCount;
	OMX_PTR                 hLibHandle;
	InitFunc                fpInit;
	DeinitFunc              fpDeinit;
	ComponentNameEnumFunc   fpComponentNameEnum;
	GetHandleFunc           fpGetHandle;
	FreeHandleFunc          fpFreeHandle;
	GetComponentsOfRoleFunc fpGetComponentsOfRole;
	GetRolesOfComponentFunc fpGetRolesOfComponent;
	SetLogModeFunc          fpSetLogMode;
} OMXR_ADAPTER_CORE_HANDLE;

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/
static OMX_ERRORTYPE OmxrAdapterCore_UpdateMCLogMode(void);
static OMX_ERRORTYPE OmxrAdapterCore_UpdateAdapterLogMode(void);

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/
/* OMX Core(OMX Service API Core) handle */
static OMXR_ADAPTER_CORE_HANDLE *psOmxrCoreHandle = NULL;

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
static void OmxrAdapterCore_Cleanup(void)
{
	psOmxrCoreHandle->u32InitCount--;

	if (0 == psOmxrCoreHandle->u32InitCount) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "deinit");

		/* Free Library Handle */
		dlclose(psOmxrCoreHandle->hLibHandle);

		/* Free OMX Core Handle */
		free(psOmxrCoreHandle);

		psOmxrCoreHandle = NULL;
	}else{
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "not deinitialized. rest init count is %d", psOmxrCoreHandle->u32InitCount);
	}
}

/***************************************************************************
 * SUMMARY: 	OMX_Init
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Init(void)
{
	OMX_ERRORTYPE	eError;

	OmxrAdapterCore_UpdateAdapterLogMode();

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "init count = %d");

	if (NULL == psOmxrCoreHandle) {
		/* Create OMX Service API handle */
		psOmxrCoreHandle = malloc(sizeof(OMXR_ADAPTER_CORE_HANDLE));
		if (NULL == psOmxrCoreHandle) {
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "malloc error");
			return OMX_ErrorUndefined;
		}
		memset(psOmxrCoreHandle, 0, sizeof(OMXR_ADAPTER_CORE_HANDLE));

		/* Get Library Handle from OMX Core */
		psOmxrCoreHandle->hLibHandle = dlopen(OMX_CORE_LIBRARY_NAME, RTLD_NOW);
		if (NULL == psOmxrCoreHandle->hLibHandle) {
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "cannot open library : %s (%s)", OMX_CORE_LIBRARY_NAME, dlerror());
			free(psOmxrCoreHandle);
			return OMX_ErrorUndefined;
		}

		/* Get Function Pointer from OMX Core */
		psOmxrCoreHandle->fpInit                = (InitFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_Init");
		psOmxrCoreHandle->fpDeinit              = (DeinitFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_Deinit");
		psOmxrCoreHandle->fpComponentNameEnum   = (ComponentNameEnumFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_ComponentNameEnum");
		psOmxrCoreHandle->fpGetHandle           = (GetHandleFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_GetHandle");
		psOmxrCoreHandle->fpFreeHandle          = (FreeHandleFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_FreeHandle");
		psOmxrCoreHandle->fpGetComponentsOfRole = (GetComponentsOfRoleFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_GetComponentsOfRole");
		psOmxrCoreHandle->fpGetRolesOfComponent = (GetRolesOfComponentFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMX_GetRolesOfComponent");
		psOmxrCoreHandle->fpSetLogMode          = (SetLogModeFunc)dlsym(psOmxrCoreHandle->hLibHandle, "OMXR_SetLogMode");
	
		if(NULL == psOmxrCoreHandle->fpInit ||
			NULL == psOmxrCoreHandle->fpDeinit ||
			NULL == psOmxrCoreHandle->fpComponentNameEnum ||
			NULL == psOmxrCoreHandle->fpGetHandle ||
			NULL == psOmxrCoreHandle->fpFreeHandle ||
			NULL == psOmxrCoreHandle->fpGetComponentsOfRole ||
			NULL == psOmxrCoreHandle->fpGetRolesOfComponent ||
			NULL == psOmxrCoreHandle->fpSetLogMode ){
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "cannot load symbols %s", OMX_CORE_LIBRARY_NAME);
			dlclose(psOmxrCoreHandle->hLibHandle);
			free(psOmxrCoreHandle);
			return OMX_ErrorUndefined;
		}
	}

	/* Increment init count */
	psOmxrCoreHandle->u32InitCount++;

	OmxrAdapterCore_UpdateMCLogMode();

	/* Do OMX_Init function */
	eError = (*psOmxrCoreHandle->fpInit)();
	if (OMX_ErrorNone != eError) {
		OmxrAdapterCore_Cleanup();
	}

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x, init count = %d", eError, psOmxrCoreHandle->u32InitCount);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	OMX_Deinit
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit(void)
{
	OMX_ERRORTYPE eError;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "");

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Do OMX_Deinit function */
	eError = (*psOmxrCoreHandle->fpDeinit)();

	OmxrAdapterCore_Cleanup();

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	OMX_ComponentNameEnum
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
	OMX_OUT OMX_STRING cComponentName,
	OMX_IN  OMX_U32    nNameLength,
	OMX_IN  OMX_U32    nIndex)
{
	OMX_ERRORTYPE eError;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "nIndex=%d", nIndex);

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Do OMX_ComponentNameEnum function */
	eError = (*psOmxrCoreHandle->fpComponentNameEnum)(cComponentName, nNameLength, nIndex);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "cComponentName=%s", cComponentName);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	OMX_GetHandle
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(
	OMX_OUT OMX_HANDLETYPE   *pHandle, 
	OMX_IN  OMX_STRING       cComponentName,
	OMX_IN  OMX_PTR          pAppData,
	OMX_IN  OMX_CALLBACKTYPE *pCallBacks)
{
	OMX_ERRORTYPE     eError;

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Check Argument */
	if (NULL == pHandle || NULL == cComponentName || NULL == pCallBacks) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "cComponentName=%s", cComponentName);

	{
		OMX_U32 u32Index;
		OMXR_ADAPTER_IMPLEMENT_TABLE *psTable;
		
		/* search a corresponding adapter implementation for component name */
		psTable = NULL;
		for(u32Index = 0; u32Index < sizeof(sAdapterTable)/sizeof(OMXR_ADAPTER_IMPLEMENT_TABLE); u32Index++){
			OMX_U32 u32CompareLength;
			
			psTable = &sAdapterTable[u32Index];

			u32CompareLength = strnlen(psTable->strComponentName, OMX_MAX_STRINGNAME_SIZE);
			
			if(strncmp(cComponentName, psTable->strComponentName, u32CompareLength) == 0){
				break;
			}
		}
		if(NULL == psTable){
			OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "not found");
			eError = OMX_ErrorUndefined;
		}else{
			eError = psTable->fpComponentInit(pHandle, cComponentName, pAppData, pCallBacks, NULL);
		}
	}

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x, *pHandle=H'%08x", eError, *pHandle);

	return eError;
}

OMX_ERRORTYPE OmxrAdapterCore_GetHandle(
	OMX_HANDLETYPE   *hRealComponentHandle, 
	OMX_STRING       cComponentName,
	OMX_PTR          pAppData,
	OMX_CALLBACKTYPE *pCallBacks){

	OMX_ERRORTYPE     eError;

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Check Argument */
	if (NULL == hRealComponentHandle || NULL == cComponentName || NULL == pCallBacks) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	OmxrAdapterCore_UpdateMCLogMode();
	OmxrAdapterCore_UpdateAdapterLogMode();

	eError = psOmxrCoreHandle->fpGetHandle(hRealComponentHandle, cComponentName, pAppData, pCallBacks);
	
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

/***************************************************************************
 * SUMMARY: 	OMX_FreeHandle
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(
	OMX_IN  OMX_HANDLETYPE  hAdapterComponentHandle)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_COMPONENTTYPE *psAdapterComponent;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "hAdapterComponentHandle=H'%08x", hAdapterComponentHandle);

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Check Component Handle */
	if (NULL == hAdapterComponentHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	psAdapterComponent = (OMX_COMPONENTTYPE *)hAdapterComponentHandle;

	eError = psAdapterComponent->ComponentDeInit(hAdapterComponentHandle);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

OMX_ERRORTYPE OmxrAdapterCore_FreeHandle(
	OMX_IN  OMX_HANDLETYPE  hRealComponentHandle)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Check Argument */
	if (NULL == hRealComponentHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "parameter error");
		return OMX_ErrorBadParameter;
	}

	/* Do OMX_FreeHandle function */
	eError = psOmxrCoreHandle->fpFreeHandle(hRealComponentHandle);
	
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}
/***************************************************************************
 * SUMMARY: 	OMX_SetupTunnel
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_SetupTunnel(
	OMX_IN  OMX_HANDLETYPE hOutput,
	OMX_IN  OMX_U32        nPortOutput,
	OMX_IN  OMX_HANDLETYPE hInput,
	OMX_IN  OMX_U32        nPortInput)
{
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "not implemented");

	return OMX_ErrorNotImplemented;
}

/***************************************************************************
 * SUMMARY: 	OMX_GetContentPipe
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_GetContentPipe(
	OMX_OUT OMX_HANDLETYPE *hPipe,
	OMX_IN  OMX_STRING     szURI)
{
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "not implemented");

	return OMX_ErrorNotImplemented;
}

/***************************************************************************
 * SUMMARY: 	OMX_GetComponentsOfRole
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_GetComponentsOfRole ( 
	OMX_IN    OMX_STRING role,
	OMX_INOUT OMX_U32    *pNumComps,
	OMX_INOUT OMX_U8     **compNames)
{
	OMX_ERRORTYPE eError;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "role = H'%08x", role);

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Do OMX_GetComponentsOfRole function */
	eError = psOmxrCoreHandle->fpGetComponentsOfRole(role, pNumComps, compNames);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return OMX_ErrorNone;
}

/***************************************************************************
 * SUMMARY: 	OMX_GetRolesOfComponent
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_GetRolesOfComponent ( 
	OMX_IN    OMX_STRING compName, 
	OMX_INOUT OMX_U32    *pNumRoles,
	OMX_OUT   OMX_U8     **roles)
{
	OMX_ERRORTYPE eError;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "ret = H'%08x", eError);

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "bad psOmxrCoreHandle");
		return OMX_ErrorUndefined;
	}

	/* Do OMX_GetRolesOfComponent function */
	eError = psOmxrCoreHandle->fpGetRolesOfComponent(compName, pNumRoles, roles);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return OMX_ErrorNone;
}


/***************************************************************************
 * SUMMARY: 	Set log mode.
 * PARAMETERS:	
 * RETURNS: 	
 * DESCRIPTION: 
 * NOTES:
 ***************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMXR_SetLogMode(
	OMX_IN  OMX_U32  u32LogMode)
{
	OMX_ERRORTYPE eError;

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_INFO, "u32LogMode=H'%08x", u32LogMode);

	/* Check Core Handle */
	if (NULL == psOmxrCoreHandle) {
		return OMX_ErrorNone;
	}

	/* Do OMXR_SetLogMode function */
	eError = psOmxrCoreHandle->fpSetLogMode(u32LogMode);

	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_DEBUG, "ret = H'%08x", eError);

	return eError;
}

static OMX_ERRORTYPE OmxrAdapterCore_UpdateMCLogMode(void)
{
	OMX_ERRORTYPE eError;
	OMX_S8 value[PROPERTY_VALUE_MAX];
	OMX_U32 u32LogLevel;

	property_get(OMXR_ADAPTER_PROPERTY_NAME_DEBUG_MC, (char *)value , OMXR_ADAPTER_PROPERTY_DEFAULT_DEBUG_MC);

	u32LogLevel = strtoul(value, NULL, 0);

	eError = (*psOmxrCoreHandle->fpSetLogMode)(u32LogLevel);
	if (OMX_ErrorNone != eError) {
		OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "fail to set mc log mode");
	}

	return eError;
}

static OMX_ERRORTYPE OmxrAdapterCore_UpdateAdapterLogMode(void)
{
	OMX_ERRORTYPE eError;
	OMX_S8 value[PROPERTY_VALUE_MAX];
	OMX_U32 u32LogLevel;

	property_get(OMXR_ADAPTER_PROPERTY_NAME_DEBUG_ADAPTER, (char *)value , OMXR_ADAPTER_PROPERTY_DEFAULT_DEBUG_ADAPTER);

	u32LogLevel = strtoul(value, NULL, 0);

	OmxrAdapterDebug_SetLogLevel(u32LogLevel);

	return eError;
}

OMX_ERRORTYPE OMXR_GetConfiguration(OMX_STRING cConfigName, OMX_U32* pu32Length){
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "OMXR_GetConfiguratio is not supported");
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXR_SetConfiguration(OMX_STRING cConfigName){
	OMXR_ADAPTER_LOGGER(ADAPTER_LOG_ERROR, "OMXR_SetConfiguratio is not supported");
	return OMX_ErrorNone;
}

