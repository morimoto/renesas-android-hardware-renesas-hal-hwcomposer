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
 * OMX Converter os dependence file 
 *
 * \file cnv_private.h
 * \attention
 */
#ifndef CNV_INTERFACE_H
#define CNV_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define CNV_COUNT_SUPPORT_INPUTIMAGEFORMAT	(4)	/**< Count of input image format that Converter supports */
#define CNV_COUNT_SUPPORT_OUTPUTIMAGEFORMAT	(4)	/**< Count of output image format that Converter supports */
/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
* \enum CNV_STATE
*/
typedef enum {
	CNV_STATE_DEINITIALIZED,/**< DEINITIALIZED*/
	CNV_STATE_CLOSED,		/**< CLOSED*/
	CNV_STATE_OPENED,		/**< OPENED*/
	CNV_STATE_FLUSHING,		/**< FLUSHING*/
	CNV_STATE_FATAL			/**< FATAL */
}CNV_STATETYPE;

/**
* \struct CNV_PLUGIN_LIST_T
*/
typedef struct {
	CNV_U32					nPluginId;
	CNV_HANDLE				hPlugin;
	CNV_ERRORTYPE (*CNVP_GetPlugInFunc)(
		CNVP_PLUGIN_IF_FUNCS_T	*psPlugInFunc,
		CNV_SUBERRORTYPE 		*pSubErrorCode
	);
}CNV_PLUGIN_LIST_T;

/**
* \struct CNV_PLUGIN_SELECTED_T 
*/
typedef struct {
	CNV_U32	nPluginID;			/**<Plug-in ID */
	CNV_U32	nPluginArrayIndex;	/**<Plugin Information */
}CNV_PLUGIN_SELECTED_T;

/**
* \struct CNV_SET_PARAMETER_T
*/
typedef struct {
	CNV_U32	nPluginId;		/**<Plug-in ID */
	CNV_U32	nDeinterlace;	/**<Convert mode */
}CNV_SET_PARAMETER_T;

/**
* \struct CNV_SUPPORT_IMAGEFORMAT_T
*/
typedef struct {
	CNV_U32	nInputImageFormat[CNV_COUNT_SUPPORT_INPUTIMAGEFORMAT];
	CNV_U32	nOutputImageFormat[CNV_COUNT_SUPPORT_OUTPUTIMAGEFORMAT];
}CNV_SUPPORT_IMAGEFORMAT_T;

/**
* \struct CNV_ERROR_T
*/
typedef struct {
	CNV_ERRORTYPE		nError;
	CNV_SUBERRORTYPE	nSubError;
}CNV_ERROR_T;

/**
* \struct CNV_ERRORCODESTRING_T
*/
typedef struct {
	CNV_U32		nErrorCode;
	CNV_STRING	pErrorString;
}CNV_ERRORCODESTRING_T;

/**
* \struct CNV_HANDLE_T
*/
typedef struct {
	CNV_ERROR_T						pError;										/**<Error code */
	CNV_STATETYPE					eState;										/**<State*/
	CNV_SET_PARAMETER_T				sSetParameter;								/**<Set Parameter*/
	CNV_U32							nMutexId;									/**<API Mutex ID*/
	CNV_U32							nStateMutex;								/**<State mutex ID*/
	CNV_U32							nPluginMaxNum;								/**<DLL information*/
	CNV_U32							nPluginLocalId;								/**<Plugin ID*/
	CNV_HANDLE						hParameter;									/**<Set parameter handle*/
	CNV_PLUGIN_LIST_T				sPluginList[CNV_COUNT_BUILTIN_PLUGINS];		/**<Plug-in list */
	CNV_PLUGIN_SELECTED_T			sPluginSelected;							/**<Plug-in data*/
	CNVP_PLUGIN_IF_FUNCS_T			sPluginIF[CNV_COUNT_BUILTIN_PLUGINS];		/**<Plug-in function*/
	CNVP_EVENT_FUNCS_T	 	 		sEventFunc;									/**<Event function*/
	CNV_PLUGIN_CONFIG_T				sPluginConfig[CNV_COUNT_BUILTIN_PLUGINS];	/**<Plug-in path*/
	CNV_HANDLE						hPluginModule;								/**<Plug-in handle*/
	CNV_U32							nConvertMode;								/**<Converter Mode*/
	CNV_BOOL						bCloseFlag;									/**<close flag*/
	CNV_PTR							pParameterList;								/**<Patarameter list*/
} CNV_HANDLE_T;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CNV_INTERFACE_H */
