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
 * OMX Utility header file for debug
 *
 * The header file defines macro for debug.
 *
 * \file
 * \attention
 */
 

#ifndef __OMXR_DEBUG_H__
#define __OMXR_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif


#define OMXR_BUILD_LOGGER_ENABLE 1
#define OMXR_BUILD_ASSERT_ENABLE 1

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#ifdef OMXR_BUILD_ASSERT_ENABLE
#include <assert.h>
#endif

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/* Assert Macro */
#ifdef OMXR_BUILD_ASSERT_ENABLE
  #define OmxrAssert(a) assert(a)
#else
  #define OmxrAssert(a)
#endif


#ifdef OMXR_BUILD_LOGGER_ENABLE
  void OmxrLogFormat(OMX_U32 u32Level, OMX_STRING strFunction, OMX_U32 u32Lineno, OMX_STRING strString, ...);
  #define OMXR_LOGGER(...) _OMXR_LOGGER(__VA_ARGS__, "")
  #define _OMXR_LOGGER(loglevel, fmt, ...) OmxrLogFormat(loglevel, (OMX_STRING)__FUNCTION__, __LINE__, fmt, __VA_ARGS__, "")
#else
  #define OMXR_LOGGER(loglevel, fmt, ...) 
#endif

#define OMXR_ABORT() OmxrAbort((OMX_STRING)__FUNCTION__, __LINE__)

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __OMXR_DEBUG_H__ */
