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
 * OMX Converter Configuration
 *
 * \file cnv_config.c
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "cnv_type.h"
#include "cnv_config.h"
#include <stdio.h>

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
/*< Plugin Information*/
static CNV_PLUGIN_CONFIG_T sPluginConfig[] = {
#ifdef CNV_BUILD_FDPM_ENABLE
	{CNV_PLUGIN_ID_FDP,"libomxr_cnvpfdp.so"},
#endif
};

#undef CNV_COUNT_BUILTIN_PLUGINS
#define CNV_COUNT_BUILTIN_PLUGINS (sizeof(sPluginConfig) / sizeof(CNV_PLUGIN_CONFIG_T))

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/
CNV_ERRORTYPE CnvConfig_GetPuluginIndex(CNV_U32 *nIndex, CNV_SUBERRORTYPE *pSubErrorCode){

	CNV_ERRORTYPE eResult = CNV_ERROR_INVALID_PARAMETER;
	
	/* Set PluginMaxIndex */
	*nIndex = CNV_COUNT_BUILTIN_PLUGINS;
	if(*nIndex < 1){
		eResult = CNV_ERROR_INVALID_PARAMETER;
		if( NULL != pSubErrorCode ){
			*pSubErrorCode = CNV_ERROR_PARAM_CONFIG_PLUGINDATA;
		}
	}else{
		eResult = CNV_ERROR_NONE;
		if( NULL != pSubErrorCode ){
			*pSubErrorCode = CNV_SUBERROR_NONE;
		}
	}
	return eResult;
}


CNV_ERRORTYPE CnvConfig_GetPluginDllInfo(CNV_U32 nNum, CNV_PLUGIN_CONFIG_T *psPluginConfig, CNV_SUBERRORTYPE *pSubErrorCode)
{
	CNV_ERRORTYPE eResult = CNV_ERROR_INVALID_PARAMETER;
	
	if( nNum > (CNV_COUNT_BUILTIN_PLUGINS-1) ){
		eResult = CNV_ERROR_INVALID_PARAMETER;
		if( NULL != pSubErrorCode ){
			*pSubErrorCode = CNV_ERROR_PARAM_CONFIG_PLUGINDATA;
		}
	}else{
		/** Set DLL load path */
		psPluginConfig->PluginId 	= sPluginConfig[nNum].PluginId;
		psPluginConfig->PluginPath	= sPluginConfig[nNum].PluginPath;
		eResult = CNV_ERROR_NONE;
		if( NULL != pSubErrorCode ){
			*pSubErrorCode = CNV_SUBERROR_NONE;
		}
		
	}
	
	return eResult;
}
