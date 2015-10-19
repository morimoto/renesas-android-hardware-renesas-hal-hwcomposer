/**********************************************************************
 *
 * PURPOSE
 *   Media Component Library Header File
 *
 * AUTHOR
 *   Renesas Electronics Corporation
 *
 * DATE
 *   2010/06/30
 *
 **********************************************************************/
/*
 * Copyright (C) Renesas Electronics Corporation 2008-2011
 * RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY
 * This program must be used solely for the purpose for which
 * it was furnished by Renesas Electronics Corporation.
 * No part of this program may be reproduced or disclosed to
 * others, in any form, without the prior written permission
 * of Renesas Electronics Corporation.
 *
 **********************************************************************/


#ifndef __OMXR_EXTENSION_ANDROID_H__
#define __OMXR_EXTENSION_ANDROID_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_Image.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>

/*******************/
/* Extended Index. */
/*******************/
enum {
    OMX_IndexCustomizeStartUnused         = (OMX_IndexVendorStartUnused + 0x00100000),
    OMX_IndexEnableAndroidNativeBuffer,
    OMX_IndexAndroidNativeBufferUsage,
    OMX_IndexStoreMetaDataInBuffers,
    OMX_IndexPrependSPSPPSToIDRFrames
};

typedef struct OMX_PARAM_ANDROID_NATIVEBUFFERUSAGE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nUsage;
} OMX_PARAM_ANDROID_NATIVEBUFFERUSAGE;

typedef struct OMX_PARAM_ANDROID_STOREMATADATAINBUFFERS {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bStoreMetaData;
} OMX_PARAM_ANDROID_STOREMATADATAINBUFFERS;

typedef struct OMX_PARAM_ANDROID_ENABLENATIVEBUFFERS {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL enable;
}OMX_PARAM_ANDROID_ENABLENATIVEBUFFERS;

typedef struct OMX_PARAM_ANDROID_BUFFERMETADATA{
    OMX_U32 nSize;
    OMX_U32 u32BufferSize;
    OMX_U32 u32BufferAddress;
} OMX_PARAM_ANDROID_BUFFERMETADATA;

typedef struct OMX_PARAM_ANDROID_PREPENDSPSPPSTOIDRFRAMES{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_BOOL enable;
} OMX_PARAM_ANDROID_PREPENDSPSPPSTOIDRFRAMES;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OMXR_EXTENSION_ANDROID_H__ */

