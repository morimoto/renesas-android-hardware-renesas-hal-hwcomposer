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
 * Log function interface definition
 *
 * @file
 */
#ifndef __OMXR_ADAPTER_LOG_H__
#define __OMXR_ADAPTER_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <stdio.h>

#define LOG_TAG "OMXR_ADAPTER"
#include <utils/Log.h>

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define	OMXR_ADAPTER_PROPERTY_NAME_DEBUG_ADAPTER		"debug.omxr.adapter"
#define	OMXR_ADAPTER_PROPERTY_DEFAULT_DEBUG_ADAPTER		"0x00000003"
#define	OMXR_ADAPTER_PROPERTY_NAME_DEBUG_MC				"debug.omxr.mc"
#define	OMXR_ADAPTER_PROPERTY_DEFAULT_DEBUG_MC			"0x22222200"

#ifdef OMXR_ADAPTER_BUILD_LOGGER_ENABLE
  void OmxrAdapterDebug_LogVa(OMX_U32 u32Level, OMX_STRING strFunction, OMX_U32 u32Lineno, OMX_STRING strString, ...);
  #define OMXR_ADAPTER_LOGGER(...) _OMXR_ADAPTER_LOGGER(__VA_ARGS__, "")
  #define _OMXR_ADAPTER_LOGGER(loglevel, fmt, ...) OmxrAdapterDebug_LogVa(loglevel, (OMX_STRING)__FUNCTION__, __LINE__, fmt, __VA_ARGS__, "")
#else
  #define OMXR_ADAPTER_LOGGER(loglevel, fmt, ...) 
#endif

/* Log message level */
#define ADAPTER_LOG_FATAL    0x00000001
#define ADAPTER_LOG_ERROR    0x00000002
#define ADAPTER_LOG_WARN     0x00000004
#define ADAPTER_LOG_INFO     0x00000008
#define ADAPTER_LOG_DEBUG    0x00000010

#define ADAPTER_LOG_BUFFER   0x00000100
#define ADAPTER_LOG_EVENT    0x00000200
#define ADAPTER_LOG_COMMAND  0x00000400
#define ADAPTER_LOG_PARAM    0x00000800

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
extern void OmxrAdapterDebug_SetLogLevel(OMX_U32 u32Level);

#endif /* __OMXR_ADAPTER_LOG_H__ */
