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
 * UVCS UDF(User Defined Function) Interface Definition
 *
 * \file
 * \attention
 */


#ifndef __OMXR_UVCS_UDF_H__
#define __OMXR_UVCS_UDF_H__

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "OMX_Core.h"
#include "OMX_Types.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
 * UVCS UDF Handle
 */
typedef struct tagOMXR_UVCS_UDF_STRUCT * OMXR_UVCS_UDF_HANDLE;

/**
 * UVCS UDF Callback Function Type
 */
typedef OMX_ERRORTYPE (*OMXR_UVCS_UDF_CALLBACK_FUNCTYPE) (
    OMXR_UVCS_UDF_HANDLE hUdfHandle, /**< UDF Handle                */
    OMX_PTR pvUserContext,           /**< User Context              */
    OMX_PTR res_info,                /**< Response from UVCS Common */
    OMX_ERRORTYPE eError             /**< Error information to notify driver access */
);

/**
 * UVCS UDF Open Parameter
 */
typedef struct OMXR_UVCS_UDF_OPEN_PARAM {
    OMXR_UVCS_UDF_CALLBACK_FUNCTYPE fCallback;  /**< Pointer to callback function  */
    OMX_U32 u32Flags;                           /**< reserve                       */
} OMXR_UVCS_UDF_OPEN_PARAM;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

/**
 * Open UVCS UDF handle
 *
 * UDF handle has a one-to-one relation with OMX component instance.
 * This function opens UVCS kernel module.
 *
 * \param[in, out] hUdfHandle     UDF handle
 * \param[in]      pvUserContext  This pointer is passed to a specified callback function
 * \param[in]      psParam        UVCS UDF Open Parameter
 * \return         OMX_ERROTTYPE
 * \attention      
 */
OMX_ERRORTYPE OmxrUvcsUdf_Open(OMXR_UVCS_UDF_HANDLE *hUdfHandle, OMX_PTR pvUserContext, OMXR_UVCS_UDF_OPEN_PARAM *psParam);

/**
 * Close UVCS UDF handle
 *
 * This function closes UVCS kernel module.
 *
 * \param[in]      hUdfHandle     UDF handle
 * \return         OMX_ERROTTYPE
 * \attention
 */
OMX_ERRORTYPE OmxrUvcsUdf_Close(OMXR_UVCS_UDF_HANDLE hUdfHandle);

/**
 * Request UVCS process
 *
 * This function is called from OMX component when UVCS decoder or encoder issues UVCS_DEC_EVENT_REQ_HW_PROC event.
 * psRequestData, which UVCS generates, must be passed to this function as is.
 * This function makes a device driver call to request hardware processing to codec hardware.
 *
 * \param[in]      hUdfHandle     UDF handle
 * \param[in]      psRequestData  UVCS request data
 * \return         OMX_ERROTTYPE
 * \attention
 */
OMX_ERRORTYPE OmxrUvcsUdf_ProcRequest(OMXR_UVCS_UDF_HANDLE hUdfHandle, OMX_PTR pvRequestData);

/**
 * Get Hardware IP Information
 *
 * This function gets CODEC hardware IP information. This information must be passed to 
 * OMX and CODEC softwares.
 *
 * \param[in]      hUdfHandle     UDF handle
 * \param[out]     pvInfo         Hardware IP information
 * \return OMX_ERROTTYPE
 * \attention
 */
OMX_ERRORTYPE OmxrUvcsUdf_GetIpInfo(OMXR_UVCS_UDF_HANDLE hUdfHandle, OMX_PTR pvIpInfo);

#endif
