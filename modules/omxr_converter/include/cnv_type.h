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
 * OMX Converter public file 
 *
 * @file cnv_type.h
 * @attention
 */
#ifndef CNV_TYPE_H
#define CNV_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#ifndef __bool_true_false_are_defined
typedef int bool;
#define false 0
#define true 1
#define __bool_true_false_are_defined 1
#endif
	
#ifndef CNV_IN
#define CNV_IN
#endif

#ifndef CNV_OUT
#define CNV_OUT
#endif

#ifndef CNV_INOUT
#define CNV_INOUT
#endif

/* Truth-values */
#define CNV_FALSE false
#define CNV_TRUE  true

/* ID of Plug-in */
#define CNV_PLUGIN_ID_SWC  (0x00000001)
#define CNV_PLUGIN_ID_VSP  (0x00000002)
#define CNV_PLUGIN_ID_FDP  (0x00000004)

/* IP Conversion Types */
#define CNV_DEINT_NONE        (0x00000000)
#define CNV_DEINT_2D_HALFRATE (0x00000001)
#define CNV_DEINT_2D_FULLRATE (0x00000002)
#define CNV_DEINT_3D_HALFRATE (0x00000005)
#define CNV_DEINT_3D_FULLRATE (0x00000006)


/* Response to a Query about Image Format */
#define CNV_QUERY_IMAGE_NOT_SUPPORTED           (0x00000001)
#define CNV_QUERY_IMAGE_SUPPOERTED_INPUT        (0x00000002)
#define CNV_QUERY_IMAGE_SUPPOERTED_OUTPUT       (0x00000004)
#define CNV_QUERY_IMAGE_SUPPOERTED_INPUT_OUTPUT (0x00000006)

/* Image Formats */
#define CNV_YUV420P  (1)
#define CNV_YUV420SP (2)
#define CNV_YVU420P  (3)
#define CNV_YVU420SP (4)

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef void 				CNV_VOID;
typedef char 				CNV_S8;
typedef unsigned char 		CNV_U8;
typedef short 				CNV_S16;
typedef unsigned short 		CNV_U16;
typedef signed long 		CNV_S32;
typedef unsigned long 		CNV_U32;
typedef signed long long 	CNV_S64;
typedef unsigned long long 	CNV_U64;
typedef unsigned long		CNV_INDEXTYPE;
typedef void 				*CNV_PTR;
typedef char 				*CNV_STRING;
typedef bool 				CNV_BOOL;
typedef void 				*CNV_HANDLE;

#ifndef CNV_SKIP64BIT
typedef CNV_S64 CNV_TICKS;
#else
typedef struct CNV_TICKS
{
    CNV_U32 nLowPart;    /**< low bits of the signed 64 bit tick value */
    CNV_U32 nHighPart;   /**< high bits of the signed 64 bit tick value */
} CNV_TICKS;
#endif

/**
* \enum CNV_ERRORTYPE 
*/
typedef enum{
    CNV_ERROR_NONE				 = 0x00000000,          /**< Success */
    CNV_ERROR_INVALID_SEQUENCE	 = (CNV_S32)0x80010000, /**< Sequence error */
    CNV_ERROR_INVALID_PARAMETER  = (CNV_S32)0x80020000, /**< Parameter error */
    CNV_ERROR_FATAL              = (CNV_S32)0x80040000, /**< Fatal error */
	CNV_ERROR_FLUSHING           = (CNV_S32)0x80050000, /**< Flushing  */
	CNV_ERROR_UNDEFINE 			 = (CNV_S32)0xFFFFFFFF  /**< undefine */
} CNV_ERRORTYPE;

/**
* \enum CNV_CALLBACK_EVENTTYPE
*/
typedef enum {
	CNV_CALLBACK_EVENT_FLUSHDONE,
	CNV_CALLBACK_EVENT_FATAL
} CNV_CALLBACK_EVENTTYPE;

/**
* \enum CNV_SUBERRORTYPE
*/
typedef enum{
  /**< Error None */
  CNV_SUBERROR_NONE 					= 0x00000001,	

  CNV_ERROR_SEQUENCE_OUTOFORDER			= (CNV_S32)0x80001001,
	
  /**< The value set into a pHandle argument is invalid. */
  CNV_ERROR_PARAM_HANDLE				= (CNV_S32)0x80002001,

  /**< The value set into Createhandle is failed. */
  CNV_ERROR_PARAM_CREATEHANDLE,

  /**< The value set into a cPrameterName argument is invalid. */
  CNV_ERROR_PARAM_PARAMETERNAME,

  /**< The value set into a pIndexType argument is invalid. */
  CNV_ERROR_PARAM_INDEXTYPE,

  /**< The value set into a pProcessingParameterStructure argument is invalid. */
  CNV_ERROR_PARAM_PROCESSINGPARAMETERSTRUCTURE,

  /**< The value set into a Plugin Index number is invalid. */
  CNV_ERROR_PARAM_CONFIG_PLUGINDATA,

  /**< The value set into DLL_Load is failed. */
  CNV_ERROR_PARAM_DLL_LOAD,

  /**< The value set into DLL_UnLoad is failed. */
  CNV_ERROR_PARAM_DLL_UNLOAD,

  /**< The value set into DLL_Entry is failed. */
  CNV_ERROR_PARAM_DLL_Entry,
	
  /**< The value set into Get_Function is failed. */	
  CNV_ERROR_PARAM_GET_FUNCTION,

  /**< The value set into a pResponse argument is invalid. */
  CNV_ERROR_PARAM_RESPONSE,

  /**< The value set into a pContraints is invalid. */
  CNV_ERROR_PARAM_CONSTRAINTS,

  /**< The value set into a nInputImage_ColorFormat argument is invalid. */
  CNV_ERROR_PARAM_NINPUTIMAGE_COLORFORMAT,

  /**< The value set into a select_plugin is invalid. */
  CNV_ERROR_PARAM_UNSELECT_PLUGIN,
	
  /**< The value set into a pCallbackFuncs argument is invalid. */
  CNV_ERROR_PARAM_CALLBACKFUNCS,
	
  /**< The value set into a EmtyBufferDone argument is invalid. */
  CNV_ERROR_PARAM_EMTPYBUFFERDONE,

  /**< The value set into a FillBufferDone argument is invalid. */
  CNV_ERROR_PARAM_FILLBUFFERDONE,

  /**< The value set into a FlushBufferDone argument is invalid. */
  CNV_ERROR_PARAM_FLUSHBUFFERDONE,

  /**< The value set into a pBuffer argument is invalid. */
  CNV_ERROR_PARAM_BUFFER,
	
  /* Submodule error */
  /**< Parameter error */
  CNV_ERROR_SUBMODULE_PARAM 				= (CNV_S32)0x80004001,

  /**< ExtensionIndex Parameter error */
  CNV_ERROR_SUBMODULE_PARAM_EXTENSIONINDEX,
	
  /**< plugin CreateHnadle error */	
  CNV_ERROR_SUBMODULE_PARAM_CREATE_HANDLE,

  /**< plugin localfunction error */
  CNV_ERRPR_SUBMODULE_PARAM_GET_FUNCTION,

  /**< local callback error */
  CNV_ERRPR_SUBMODULE_PARAM_CALLBACKFUNCS,

  /**< IP error */
  CNV_ERROR_SUBMODULE_IP,

  /**< Abnormal end */
  CNV_ERROR_SUBMODULE_ABEND,

  /**< Timeout error */
  CNV_ERROR_SUBMODULE_TIMEOUT,
	
  /**< Flushing run */
  CNV_ERROR_FLUSHING_RUN					= (CNV_S32)0x80005001,

  CNV_SUBERROR_UNDEFINE 					= (CNV_S32)0xFFFFFFFF

} CNV_SUBERRORTYPE;

/**
* \struct CNV_CONSTRAINTS_T
*/
typedef struct {
	CNV_U32		nImageCropPosXBoundary;
	CNV_U32		nImageCropPosYBoundary;
	CNV_U32		nImageCropWidthBoundary;
	CNV_U32		nImageCropHeightBoundary;
	CNV_U32 	nBufferWidthMultiplesOf;
	CNV_U32 	nBufferHeightMultiplesOf;
	CNV_U32		nBufferTopAddressBoundary;
	CNV_U32		nMaxNumberOfBuffersPerProcessing;
 } CNV_CONSTRAINTS_T;

/**
* \struct CNV_BUFFER_AREA_INFO_T
*/
typedef struct {
	CNV_PTR pTopAddress_Y_Area;
	CNV_PTR pTopAddress_C_Area0;
	CNV_PTR pTopAddress_C_Area1;
	CNV_U32 nStride_Y_Area;
	CNV_U32 nStride_C_Area;
	CNV_U32 nHeight_Y_Area;
	CNV_U32 nHeight_C_Area;
} CNV_BUFFER_AREA_INFO_T;

/**
* \struct CNV_IMAGE_INFO_T
*/
typedef struct {
	CNV_U32		nWidth;
	CNV_U32		nHeight;
	CNV_U32		nOffsetX;
	CNV_U32		nOffsetY;
} CNV_IMAGE_INFO_T;

/**
* \struct CNV_OUTPUT_PIC_INFO_T
*/
typedef struct {
	CNV_BOOL		bFrameExtended;
	CNV_U32			nFrameExtendedNum;
	CNV_U32			pEmptyUserPointer;
	CNV_BOOL		bEmptyKeepFlag;
} CNV_OUTPUT_PIC_INFO_T;

/**
* \struct CNV_BUFFERHEADER_T
*/
typedef struct {
	CNV_BUFFER_AREA_INFO_T 	sTopArea;
	CNV_IMAGE_INFO_T 		sTopAreaImage;
	CNV_BUFFER_AREA_INFO_T 	sBottomArea;
	CNV_IMAGE_INFO_T 		sBottomAreaImage;
	CNV_BOOL 				bFrameFormat;
	CNV_U32					nFlag;
	CNV_OUTPUT_PIC_INFO_T	sOutputConvertInfo;
	CNV_TICKS				nTimeStamp;
	CNV_U32					nErrorFlag;
	CNV_PTR					pUserPointer;
	CNV_PTR					pExtendedInfo;
 } CNV_BUFFERHEADER_T;

/**
* \struct CNV_SETTING_TILE_PARAMETER_T 
*/
typedef struct {
	CNV_U32		nPhyAddr;
	CNV_U32		nStride;
	CNV_U32		nArea;
	CNV_PTR		pReserve;
} CNV_SETTING_TILE_PARAMETER_T;

/**
* \struct CNV_SETTING_IMAGECONVERSION_T 
*/
typedef struct {
	CNV_BOOL						bInterlacedSequence;
	CNV_BOOL						bBottomFirst;
	CNV_U32							nInputImage_ColorFormat;
	CNV_U32							nOutputImage_ColorFormat;
	CNV_BOOL						bScaling;
	CNV_BOOL						bCropping;
	CNV_BOOL						bTileMode;
	CNV_SETTING_TILE_PARAMETER_T 	sTileParam;
} CNV_SETTING_IMAGECONVERSION_T;

/**
* \struct CNV_STRUCT_HEADER_T 
*/
typedef struct {
	CNV_U32		nStructSize;
} CNV_STRUCT_HEADER_T;

/**
* \struct OMXR_MC_VIDEO_CONVERTER_T 
*/
typedef struct {
	CNV_STRUCT_HEADER_T	sHeader;
	CNV_U32				nPluginID;
} OMXR_MC_VIDEO_CONVERTER_T;

/**
* \struct OMXR_MC_VIDEO_DEINTERLACEMODE_T 
*/
typedef struct {
	CNV_STRUCT_HEADER_T	sHeader;
	CNV_U32		nDeinterlace;
} OMXR_MC_VIDEO_DEINTERLACEMODE_T;

/**
* \struct OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T 
* millisecond order
*/
typedef struct {
	CNV_STRUCT_HEADER_T	sHeader;
	CNV_U32		nWaitTime;/**<nWaitTime can not set more than 10. */
	CNV_U32		nTimeout; /**<If nTimeout is set 0, timeout function will be disable.*/
} OMXR_MC_VIDEO_SUBMODULE_TIMEOUT_T;


/**
* \struct CNV_CALLBACK_FUNCS_T 
*/
typedef struct {
	CNV_VOID (* CNV_EmptyBufferDone)(
		CNV_BUFFERHEADER_T		*pBufferInfo,
		CNV_PTR					pUserPointer,
		CNV_ERRORTYPE			nResult,
		CNV_SUBERRORTYPE		nSubErrorCode);
	CNV_VOID (* CNV_FillBufferDone)(
		CNV_BUFFERHEADER_T		*pBufferInfo,
		CNV_PTR					pUserPointer,
		CNV_ERRORTYPE			nResult,
		CNV_SUBERRORTYPE		nSubErrorCode);
	CNV_VOID (* CNV_EventDone) (
		CNV_PTR					pUserPointer,
		CNV_U32					nEventID,
		CNV_ERRORTYPE			nResult,
		CNV_SUBERRORTYPE		nSubErrorCode);
} CNV_CALLBACK_FUNCS_T;

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
