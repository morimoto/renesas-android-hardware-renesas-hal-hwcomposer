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
 * OMX Converter fdp plugin file 
 *
 * \file cnvp_fdp_core.h
 * \attention
 */
#ifndef CNVP_FDP_CORE_H
#define CNVP_FDP_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include "fdpm_drv.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef struct {
	CNV_U32 			  nConvertMode;				/**<Converter mode*/
	CNV_U32 			  nInputIndex;				/**<Input index*/
	CNV_BOOL			  bInitFlag;				/**<Init flag*/
	CNV_BOOL			  bEosFlag;					/**<Eos flag*/
	CNV_BOOL			  bFirstInput;				/**<First input flag*/
	CNV_BOOL			  bField;					/**<CNV_FALSE(TOP) CNV_TRUE(BOTTOM)*/
	CNV_BOOL			  bReleaseFlag;				/**<Release flag*/
	CNV_U32				  nEosMode;					/**<EOS mode*/
	CNV_BUFFERHEADER_T	  sLocalInputBuffer[2];     /**<Input buffers*/
	CNV_BUFFERHEADER_T	  sLocalOutputBuffer;		/**<Output buffers*/
	CNVP_CONVERT_BUFFER_T sConvertEmptyBuffer[2]; 	/**<Input information*/
	CNVP_CONVERT_BUFFER_T sConvertFillBuffer;		/**<Output information*/
	CNV_U32				  pFdpModuleHandle;			/**<FDP plugin handle*/
	CNV_U32				  nS3ctrlId;				/**<S3Ctrl*/
	CNV_BOOL			  bS3mode;					/**<S3Ctrl T/F */
	T_FDP_CB1			  sCallBackParam1;			/**<FDPM callback param*/
	T_FDP_CB2			  sCallBackParam2;			/**<FDPM callback param*/
	CNV_U32				  nStride;					/**<stride*/
	CNV_U32				  nOpenWidth;				/**<Open width */
	CNV_U32				  nOpenHeight;				/**<Open Height */
} FDP_HANDLE_T;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
/**
* The CnvpCore_GetPluginCoreFunction will get fdp plugin functions.
*
* \param[out] CNVP_PLUGIN_FUNCS_T     the pointer of fdp plugin function
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCore_GetPluginCoreFunction(
	CNVP_PLUGIN_FUNCS_T *psPlugInFunc
);

/**
* The FdpcpCore_CreateHandle will create the fdp plugin handle.
*
* \param[out] CNVP_PLUGIN_FUNCS_T     the pointer of fdp plugin function
* \param[in]  hPluginHandle           deinterlace mode
* \param[out] peSubErrorCode          sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_CreateHandle(
	CNV_PTR 		 *hPluginHandle, 
	CNV_U32 		 nConvertMode,
	CNV_SUBERRORTYPE *peSubErrorCode
);

/**
* The FdpcpCore_DeleteHandle will destroy the fdp plugin handle.
*
* \param[in] hPluginHandle           the pointer of fdp plugin function
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_DeleteHandle(
	CNV_PTR			hPluginHandle
);

/**
* \fn CNC_ERRORTYPE FdpcpCore_CheckConvertBuffer
* \breaf FDP Plugin core function
* \param[in] pParam: The pointer of converter context
* \return CNV_TRUE or CNV_FALSE
*/

/**
* The FdpcpCore_CheckConvertBuffer will check the buffer status.
*
* If this function return CNV_TRUE, the converter try to start.
*
* \param[in] pParam                   common handle
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_BOOL FdpcpCore_CheckConvertBuffer(
	CNV_PTR			pParam
);

/**
* The FdpcpCore_Executing will start a converter process.
*
* If bStart set CNV_TRUE, the converter process will start.
*
* \param[in] pParam                   common handle
* \param[in] bStart                   CNV_FALSE:no start, CNV_TRUE: start
* \param[out] peSubErrorCode          sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_Executing (
  CNV_PTR			 	pParam,
  CNV_BOOL				bStart,
  CNV_SUBERRORTYPE 		*peSubErrorCode
);

/**
* The FdpcpCore_InitPluginParam will initialize sub modules.
*
* \param[in] pParam                   common handle
* \param[in] ppBuffer                 the pointer of input buffer 
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_InitPluginParam(
  CNV_PTR pParam,
  CNV_PTR *ppBuffer
);

/**
* The FdpcpCore_PluginSelected will decide a plugin.
*
* If pbResult is CNV_TRUE, this plugin is selected.
* If pbResult is CNV_FALSE, this plugin is not selected.
*
* \param[in]  psDicisionRule          rule of plugin
* \param[out] pbResult                result
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_PluginSelected (
  CNV_PLUGIN_DICISION_RULE_T 	*psDicisionRule,
  CNV_BOOL						*pbResult,
  CNV_SUBERRORTYPE 				*peSubErrorCode
);

/**
* The FdpcpCore_CheckReleaseBuffer will release a buffer.
*
* If input or putput buffer can release, ppEmpptyBuffer or ppFillBuffer is not NULL.
*
* \param[in]  pParam                  common handle
* \param[out] ppEmptyBuffer           buffer of EmptyBufferDone
* \param[out] ppFillBuffer            buffer of FillBufferDone
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_CheckReleaseBuffer(
  CNV_PTR			 	pParam,
  CNV_PTR    			*ppEmptyBuffer,
  CNV_PTR			    *ppFillBuffer
);
	
/**
* The FdpcpCore_CheckFlushBuffer will release a buffer.
*
* \param[in]  pParam                  common handle
* \param[out] ppEmptyBuffer           buffer of EmptyBufferDone
* \param[out] ppFillBuffer            buffer of FillBufferDone
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE FdpcpCore_CheckFlushBuffer(
  CNV_PTR			 	pParam,
  CNV_PTR    			*ppEmptyBuffer,
  CNV_PTR			    *ppFillBuffer
);

#ifdef __cplusplus
}
#endif

#endif

