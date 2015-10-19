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
 * UVCS UDF OS Abstraction Layer Implementation for Linux
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

#ifdef ANDROID
typedef unsigned long ulong;
#endif

#include "omxr_uvcs_udf_osal.h"
#include "uvcs_ioctl.h"
#include "omxr_module_common.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define OMXR_UVCS_UDF_OSAL_UVCS_DRIVER_PATH  "/dev/uvcs"

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef struct tagOMXR_UVCS_UDF_OSAL_STRUCT {
	OMX_S32 fd;
} OMXR_UVCS_UDF_OSAL_STRUCT;

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
OMX_ERRORTYPE OmxrUvcsUdfOsal_Open(OMXR_UVCS_UDF_OSAL_HANDLE *handle){
	OMXR_UVCS_UDF_OSAL_STRUCT *psHandle = NULL;

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_DEBUG, "", "");

	if(handle == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psHandle = (OMXR_UVCS_UDF_OSAL_STRUCT *)OmxrMalloc(sizeof(OMXR_UVCS_UDF_OSAL_STRUCT));
	if(psHandle == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}
	(void)OmxrMemset(psHandle, 0, sizeof(OMXR_UVCS_UDF_OSAL_STRUCT));

	psHandle->fd = open(OMXR_UVCS_UDF_OSAL_UVCS_DRIVER_PATH, (O_RDWR | O_SYNC));
	if(psHandle->fd < 0){
		OmxrFree(psHandle);
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "failed to open driver(%d, %s)\n", errno, strerror(errno) );
		return OMX_ErrorUndefined;
	}
	*handle = (OMXR_UVCS_UDF_OSAL_HANDLE)psHandle;
	
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxrUvcsUdfOsal_Close(OMXR_UVCS_UDF_OSAL_HANDLE handle){
	OMX_S32 result = 0;
	OMXR_UVCS_UDF_OSAL_STRUCT *psHandle;
	
	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_DEBUG, "", "");

	if(handle == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psHandle = (OMXR_UVCS_UDF_OSAL_STRUCT *)handle;
	
	if(psHandle->fd < 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}
	result = close(psHandle->fd);
	if(result < 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "failed to close driver(%d, %s)", errno, strerror(errno) );
	}
	OmxrFree(psHandle);
	
	if(result < 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}else{
		return OMX_ErrorNone;
	}
}

OMX_ERRORTYPE OmxrUvcsUdfOsal_Write(OMXR_UVCS_UDF_OSAL_HANDLE handle, OMX_PTR pvParam, OMX_U32 u32Size){
	OMX_S32 result = 0;
	OMXR_UVCS_UDF_OSAL_STRUCT *psHandle;
	
	if( (handle == NULL) || (pvParam == NULL) ){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psHandle = (OMXR_UVCS_UDF_OSAL_STRUCT *)handle;
	
	if(psHandle->fd < 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_INFO, "write", "");
	result = write(psHandle->fd, pvParam, u32Size);
	
	if(result <= 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "failed to write(%d, %s)", errno, strerror(errno) );
		return OMX_ErrorUndefined;
	}else{
		return OMX_ErrorNone;
	}
}

OMX_ERRORTYPE OmxrUvcsUdfOsal_Read(OMXR_UVCS_UDF_OSAL_HANDLE handle, OMX_PTR pvParam, OMX_U32 u32Size){
	OMX_S32 result = 0;
	OMXR_UVCS_UDF_OSAL_STRUCT *psHandle;
	
	if( (handle == NULL) || (pvParam == NULL) ){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psHandle = (OMXR_UVCS_UDF_OSAL_STRUCT *)handle;
	
	if(psHandle->fd < 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorUndefined;
	}

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_INFO, "read start", "");
	result = read(psHandle->fd, pvParam, u32Size);
	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_INFO, "read end", "");
	
	if(result <= 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "failed to read(%d, %s)", errno, strerror(errno) );
		return OMX_ErrorUndefined;
	}else{
		return OMX_ErrorNone;
	}
}

OMX_ERRORTYPE OmxrUvcsUdfOsal_CancelRead(OMXR_UVCS_UDF_OSAL_HANDLE handle)
{
	if(handle == NULL){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}

	/* TODO: to be implemented */
	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_WARN, "not implemented", "");

	return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxrUvcsUdfOsal_GetIpInfo(OMXR_UVCS_UDF_OSAL_HANDLE handle, OMX_PTR pvIpInfo){
	OMX_S32 result;
	OMXR_UVCS_UDF_OSAL_STRUCT *psHandle;

	OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_DEBUG, "", "");

	if( (handle == NULL) || (pvIpInfo == NULL) ){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "", "");
		return OMX_ErrorBadParameter;
	}
	psHandle = (OMXR_UVCS_UDF_OSAL_STRUCT *)handle;

	result = ioctl(psHandle->fd, UVCS_IOCTL_GET_IP_INFO, pvIpInfo);
	if(result != 0){
		OMXR_LOGGER(OMXR_VIDEO_LOG_LEVEL_ERROR, "failed to ioctl(%d, %s)", errno, strerror(errno) );
		return OMX_ErrorUndefined;
	}else{
		return OMX_ErrorNone;
	}
}

