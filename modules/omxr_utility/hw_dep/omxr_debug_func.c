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
 * OMXR Debugging functions
 *
 * This file implements the debug function.
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include <stdio.h>
#include <omxr_dep_common.h>

#ifdef ANDROID
#define LOG_TAG "OMXR"
#include <utils/Log.h>
#endif

#ifdef OMXR_BUILD_LOGGER_ENABLE
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
static OMX_U32 OmxrLogMode = (
    OMXR_CORE_LOG_LEVEL_ERROR |
    OMXR_UTIL_LOG_LEVEL_ERROR |
    OMXR_CMN_LOG_LEVEL_ERROR |
    OMXR_VIDEO_LOG_LEVEL_ERROR|
    OMXR_AUDIO_LOG_LEVEL_ERROR|
    OMXR_CNV_LOG_LEVEL_ERROR );

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/

void OmxrLogInit(void)
{
    return;
}


void OmxrLogDone(void)
{
    return;
}


void OmxrLogSetMode(OMX_U32 u32LogMode)
{
	OmxrLogMode = u32LogMode;
    return;
}


OMX_BOOL OmxrLogCheckLevel(OMX_U32 u32Level)
{
    if ((u32Level & OmxrLogMode) == u32Level) {
        return OMX_TRUE;
    }
    
    return OMX_FALSE;
}


void OmxrLogWithoutLevel(const OMX_STRING strString, ...)
{
    va_list vaArgs;

    va_start(vaArgs, strString);
    OmxrLogVa(0xffffffffUL, strString, vaArgs);
    va_end(vaArgs);
    
    return;
}


void OmxrLog(OMX_U32 u32Level, const OMX_STRING strString, ...)
{
    va_list vaArgs;

    if (OmxrLogCheckLevel(u32Level) == OMX_TRUE) {
        va_start(vaArgs, strString);
        OmxrLogVa(u32Level, strString, vaArgs);
        va_end(vaArgs);
    }
    return;
}



void OmxrLogVa(OMX_U32 u32Level, const OMX_STRING strString, va_list arglist)
{
    OMX_S8 buf[1024];

    (void)vsnprintf(buf, 1024, strString, arglist);
    (void)ALOGE("%s", buf);

    return;
}

#ifdef OMXR_BUILD_LOGGER_ENABLE
void OmxrLogFormat(OMX_U32 u32Level, OMX_STRING strFunction, OMX_U32 u32Lineno, OMX_STRING strString, ...)
{
    OMX_S8 buf[1024];

    if (OmxrLogCheckLevel(u32Level) == OMX_TRUE) {
    	va_list vaArgs;

        struct timeval tval;

    	va_start(vaArgs, strString);
	    (void)vsnprintf(buf, 1024, strString, vaArgs);
    	va_end(vaArgs);

        (void)gettimeofday(&tval, NULL);
        (void)ALOGI("ts:%ld.%06ld\tlevel:0x%x\tfunc:%s(%d)\ttid:%d\tmes:%s",
                tval.tv_sec, tval.tv_usec, 
                u32Level,
                strFunction,
                u32Lineno,
                gettid(),
                buf);
    }
    return;
}
#endif

void OmxrAbort(const OMX_STRING strString, OMX_U32 u32LineNum)
{
    OmxrLogWithoutLevel("OmxrAbort:%s, line:%d\n", strString, u32LineNum);
}
