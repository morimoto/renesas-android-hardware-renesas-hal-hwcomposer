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
 * OMX utility function (configuration file)
 *
 * This file implements the export function of the configuration file.
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include "omxr_dep_common.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

#ifndef OMXR_DEFAULT_CONFIG_FILE_NAME
#ifdef OMXR_BUILD_BIN_CONFIG
#define OMXR_DEFAULT_CONFIG_FILE_NAME "omxr_default_config.cfg"
#else
#define OMXR_DEFAULT_CONFIG_FILE_NAME "omxr_default_config.txt"
#endif
#endif

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/
static OMXR_CONFIG_EXPORT_FUNC gExportFuncEntry = {NULL, NULL};

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/

OMX_STRING OmxrConfig_GetDefaultPath(void)
{
   static OMXR_CHAR OmxrConfigdefaultFileName[OMXR_MAX_CONFIG_FILE_NAME] = OMXR_DEFAULT_CONFIG_FILE_NAME;
   return (OMX_STRING)OmxrConfigdefaultFileName;
}

void OmxrConfig_RegistExportFunc(OMXR_CONFIG_EXPORT_FUNC* psExportFunc)
{
	gExportFuncEntry.OmxrConfig_GetExtSection   = psExportFunc->OmxrConfig_GetExtSection;
	gExportFuncEntry.OmxrConfig_GetExtParameter = psExportFunc->OmxrConfig_GetExtParameter;
}

OMX_ERRORTYPE OmxrConfig_GetExtSection(const OMX_STRING strSection, const OMX_STRING strParam, OMX_U32 **ppu32Array, OMX_U32 *pu32Num)
{
    OMX_ERRORTYPE eError;

	if (gExportFuncEntry.OmxrConfig_GetExtSection  == NULL) {
		OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: GetExtSetction function pointer is null.", "");
		eError = OMX_ErrorUndefined;
	} else {
        eError = (gExportFuncEntry.OmxrConfig_GetExtSection)(strSection, strParam, ppu32Array, pu32Num);
	}

	return eError;
}

OMX_ERRORTYPE OmxrConfig_GetExtParameter(OMX_STRING strMcName, const OMX_STRING strParam, OMX_U32 **ppu32Array, OMX_U32 *pu32Num)
{
    OMX_ERRORTYPE eError;

	if (gExportFuncEntry.OmxrConfig_GetExtParameter  == NULL) {
		OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: GetExtParameter function pointer is null.", "");
		eError = OMX_ErrorUndefined;
	} else {
        eError = (gExportFuncEntry.OmxrConfig_GetExtParameter)(strMcName, strParam, ppu32Array, pu32Num);
	}

	return eError;
}

