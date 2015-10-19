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
 * Log function for debugging
 *
 * @file
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <OMX_Types.h>
#include "omxr_adapter_log.h"

#include <stdio.h>

#ifdef ANDROID
#define LOG_TAG "OMXR_ADAPTER"
#include <utils/Log.h>
#endif

#ifdef OMXR_ADAPTER_BUILD_LOGGER_ENABLE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <libgen.h>
#endif

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

/* Log message print level. This is a default level.
   This value will be change by the OMXR_SetLogMode() function. */
static OMX_U32 OmxrAdapterLogMode = ADAPTER_LOG_ERROR | ADAPTER_LOG_FATAL;

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
void OmxrAdapterDebug_SetLogLevel(OMX_U32 u32Level){
	OmxrAdapterLogMode = u32Level;
}

void OmxrAdapterDebug_LogVa(OMX_U32 u32Level, OMX_STRING strFunction, OMX_U32 u32Lineno, OMX_STRING strString, ...)
{
	char buf[1024];

	if (u32Level & OmxrAdapterLogMode) {
		va_list vaArgs;

		struct timeval __tv;

		va_start(vaArgs, strString);
		vsnprintf(buf, 1024, strString, vaArgs);
		va_end(vaArgs);

		gettimeofday(&__tv, NULL);
		ALOGI("ts:%ld.%06ld\tlevel:0x%x\tfunc:%s(%d)\ttid:%d\tmes:%s",
				__tv.tv_sec, __tv.tv_usec, 
				u32Level,
				strFunction,
				u32Lineno,
				gettid(),
				buf);
	}
	return;
}

