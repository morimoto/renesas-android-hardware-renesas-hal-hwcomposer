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
 * OMX Converter Configuration file
 *
 * \file cnv_config.h
 * \attention
 */
 
#ifndef CNV_CONFIG_H
#define CNV_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/**< PlugIn Index */
#define CNV_COUNT_BUILTIN_PLUGINS (2)
/**< PlugIn Extension Index number */
#define CNV_COUNT_EXTENSIONINDEX (2) 

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/**
* \struct CNV_PLUGIN_CONFIG
* \breaf OMX Converter Plugin information 
*/
typedef struct {
	CNV_U32 	PluginId;		/*< Plugin ID */
	CNV_STRING  PluginPath;		/*< Plugin Path */
}CNV_PLUGIN_CONFIG_T;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
/**
* The CnvConfig_GetPuluginIndex will get plugin index.
*
* \param[out] nIndex          the number of plugin
* \param[out] pSubErrorCode   sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CnvConfig_GetPuluginIndex(CNV_U32 *nIndex, CNV_SUBERRORTYPE *pSubErrorCode);

/**
* The CnvConfig_GetPluginDllInfo will get plugin infor.
*
* \param[in] nNum             the number of index
* \param[out] psPluginConfig  the plugin information
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE      success
* \retval others              failure
*/
CNV_ERRORTYPE CnvConfig_GetPluginDllInfo(CNV_U32 nNum, CNV_PLUGIN_CONFIG_T *psPluginConfig, CNV_SUBERRORTYPE *pSubErrorCode);

#ifdef __cplusplus
}
#endif	

#endif /* end of include guard */
