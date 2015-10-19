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
 * OMXR Extension header for video decoder common
 * 
 * This file contains vendor-defined extension definitions.
 *
 * \file OMXR_Extension_vdcmn.h
 */

#ifndef OMXR_EXTENSION_VDCMN_H
#define OMXR_EXTENSION_VDCMN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "OMXR_Extension_video.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
/* Flag indicating the cause of decoding error. */
#define OMXR_MC_VIDEO_DECODE_ERROR_FLAG_CAUTION  0x00000001 /**< If this value is set in u32DecodeError, it indicates that it has detected a minor error that have no effects on output picture data. */
#define OMXR_MC_VIDEO_DECODE_ERROR_FLAG_CONCEAL  0x00000002 /**< If this value is set in u32DecodeError, it indicates that it has detected a minor error that need error concealment. */


enum {
    OMXR_MC_IndexParamVideoReorder                   = (OMXR_MC_IndexVendorBaseVideoDecoder + 0x0000),  /* OMX.RENESAS.INDEX.PARAM.VIDEO.REORDER */
    OMXR_MC_IndexParamVideoDeinterlaceMode           = (OMXR_MC_IndexVendorBaseVideoDecoder + 0x0001)   /* OMX.RENESAS.INDEX.PARAM.VIDEO.DEINITERLACEMODE */
};


/**
 * Extended Deinterlace Mode Type.
 */
typedef enum OMXR_MC_VIDEO_DEINTERLACE_MODETYPE {
    OMXR_MC_VIDEO_DeinterlaceNone   = 0,            /**< Not applying IP */
    OMXR_MC_VIDEO_Deinterlace2DHalf = 1,            /**< 2D IP Conversion with half rate output */
    OMXR_MC_VIDEO_Deinterlace2DFull = 2,            /**< 2D IP Conversion with full rate output */
    OMXR_MC_VIDEO_Deinterlace3DHalf = 3,            /**< 3D IP Conversion with half rate output */
    OMXR_MC_VIDEO_Deinterlace3DFull = 4,            /**< 3D IP Conversion with full rate output */
    OMXR_MC_VIDEO_DeinterlaceEnd    = 0x7FFFFFFF    /**< terminate values */
} OMXR_MC_VIDEO_DEINTERLACE_MODETYPE;


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
 * Extended Reorder Type.
 */
typedef struct tagOMXR_MC_VIDEO_PARAM_REORDERTYPE {
    OMX_U32                       nSize;            /**< Size of the structure */
    OMX_VERSIONTYPE               nVersion;         /**< OMX specification version info */
    OMX_U32                       nPortIndex;       /**< Target port index */
    OMX_BOOL                      bReorder;         /**< The reorder flags */
} OMXR_MC_VIDEO_PARAM_REORDERTYPE;

/**
 * Extended Decode Result Type.
 */
typedef struct tagOMXR_MC_VIDEO_DECODERESULTTYPE {
	OMX_U32 nSize;                                  /**< Size of the structure */
	OMX_PTR pvPhysImageAddressY;                    /**< Top physical address of dacoded picture data */
	OMX_U32 u32PictWidth;                           /**< The width of the decoded picture data */
	OMX_U32 u32PictHeight;                          /**< The height of the decoded picture data */
	OMX_U32 u32DecodeError;                         /**< The decoded error flags. */
	OMX_U32 u32PhyAddr;                             /**< This member is not used */
	OMX_U32 u32Stride;                              /**< This member is not used */
	OMX_U32 u32Area;                                /**< This member is not used */
	OMX_PTR pvReserved;                             /**< This member is not used */
} OMXR_MC_VIDEO_DECODERESULTTYPE;

/**
 * Extended Deinterlace Mode Type.
 */
typedef struct tagOMXR_MC_VIDEO_PARAM_DEINTERLACE_MODETYPE {
	OMX_U32					nSize;                      /**< Size of the structure */
	OMX_VERSIONTYPE			nVersion;                   /**< OMX specification version info */
	OMX_U32					nPortIndex;                 /**< Target port index */
	OMXR_MC_VIDEO_DEINTERLACE_MODETYPE	eDeinterlace;   /**< The de-interlace mode flags */
} OMXR_MC_VIDEO_PARAM_DEINTERLACE_MODETYPE;


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMXR_EXTENSION_VDCMN_H */
