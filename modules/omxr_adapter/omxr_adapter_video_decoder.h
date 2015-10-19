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
 * Interfacte definition for video decoder component
 *
 * @file
 */
#ifndef __OMXR_ADAPTER_VIDEO_DECODER_H__
#define __OMXR_ADAPTER_VIDEO_DECODER_H__

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

OMX_ERRORTYPE OmxrAdapterVideoDecoder_ComponentInit(
	OMX_HANDLETYPE   *pAdapterComponentHandle,
	OMX_STRING       cComponentName,
	OMX_PTR          pAppData,
	OMX_CALLBACKTYPE *psClientCallBacks,
	OMX_CALLBACKTYPE *psOverrideCallBacks);

#endif /* __OMXR_ADAPTER_VIDEO_DECODER_H__ */
