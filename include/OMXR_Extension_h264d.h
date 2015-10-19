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
 * OMXR Extension header for H.264 decoder 
 * 
 * This file contains vendor-defined extension definitions.
 *
 * \file OMXR_Extension_h264d.h
 * 
 */

#ifndef OMXR_EXTENSION_H264D_H
#define OMXR_EXTENSION_H264D_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "OMXR_Extension_vdcmn.h"
#include "OMXR_Extension_h264.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define OMXR_MC_IndexVendorBaseH264Decoder     (OMXR_MC_IndexVendorBaseVideoDecoder + OMXR_MC_VendorBaseOffsetH264)		/**< base value of extended OMX_INDEXTYPE for H.264 decoder  */
#define OMXR_MC_EventVendorBaseH264Decoder     (OMXR_MC_EventVendorBaseVideoDecoder + OMXR_MC_VendorBaseOffsetH264)		/**< base value of extended OMX_EVENTTYPE for H.264 decoder  */
#define OMXR_MC_ErrorVendorBaseH264Decoder     (OMXR_MC_ErrorVendorBaseVideoDecoder + OMXR_MC_VendorBaseOffsetH264)		/**< base value of extended OMX_ERRORTYPE for H.264 decoder  */

/**
 * extended #OMX_INDEXTYPE for H.264 decoder
 */
enum {
    OMXR_MC_IndexParamVideoStreamStoreUnit = (OMXR_MC_IndexVendorBaseH264Decoder + 0x0000)   /**< stream store unit. parameter name:OMX.RENESAS.INDEX.PARAM.VIDEO.STREAM.STORE.UNIT */
};

/**
 * stream store unit type
 */
typedef enum OMXR_MC_VIDEO_STOREUNITTYPE {
    OMXR_MC_VIDEO_StoreUnitEofSeparated       = 0,			/**< EOF separated mode        */
    OMXR_MC_VIDEO_StoreUnitTimestampSeparated = 1,			/**< time stamp separated mode */
    OMXR_MC_VIDEO_StoreUnitEnd                = 0x7FFFFFFF	/**< definition end            */
} OMXR_MC_VIDEO_STOREUNITTYPE;


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
 * Structure to Set/Get stream store unit type
 */
typedef struct tagOMXR_MC_VIDEO_PARAM_STREAM_STORE_UNITTYPE {
    OMX_U32                       nSize;			/**< Size of the structure in bytes   */
    OMX_VERSIONTYPE               nVersion;			/**< OMX specification version info   */
    OMX_U32                       nPortIndex;		/**< Port that this struct applies to */
    OMXR_MC_VIDEO_STOREUNITTYPE   eStoreUnit;		/**< Stream store unit type           */
} OMXR_MC_VIDEO_PARAM_STREAM_STORE_UNITTYPE;


/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMXR_EXTENSION_H264D_H */
