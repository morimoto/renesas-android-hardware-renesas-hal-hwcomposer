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
 * OMX Converter plugin common file 
 *
 * \file cnvp_cmn.h
 * \attention
 */
#ifndef CNVP_COMMON_H
#define CNVP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/
#include <stdio.h>
#include <string.h>

#include "cnv_type.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define CNV_COUNT_QUEUE_ELEMENTS (64)	/**<Element count of queue*/
#define CNVP_EOS_FLAG			 (1)    /**<EOS FLAG*/

/* Default mode non */
#define CNVP_DEFAULT_CONVERT_MODE (0xFFFFFFFFUL)
	
/* Default extension base */
#define CNVP_EXTENSION_INDEX_BASE (0x7F000000L)
	
/* Reserve information */
#define CNVP_RESERVED_001 (6)
#define CNVP_RESERVED_002 (4)
#define CNVP_RESERVED_003 (2)
#define CNVP_RESERVED_004 (5)
#define CNVP_RESERVED_005 (8)

#define CNVP_PIC_STRUCT_NUM (2)

/* MASK */
#define CNVP_MASK_FLAG (0xFFFFFFFFUL)
	
#define CNVP_TIME_OFFSET (1000000)/**< micro sec order */
	
/* 1/Frame Rate [microsec]/2(Full Rate) */
#define CNVP_TIME_DEFAULT	       (16666)  /**< 1/60 = 0.0166667: */
#define CNVP_TIME_RATE_23_976	   (41708/2)/**< 24000/1001 = 0.0417083 */
#define CNVP_TIME_RATE_24		   (41666/2)/**< 1/24 = 0.0416667 */
#define CNVP_TIME_RATE_25		   (40000/2)/**< 1/25 = 0.04 */
#define CNVP_TIME_RATE_29_97       (33366/2)/**< 525/15.73427 = 33.36666 */
#define CNVP_TIME_RATE_30		   (33333/2)/**< 1/30 = 0.033333 */
#define CNVP_TIME_RATE_50		   (20000/2)/**< 1/50 = 0.02 */
#define CNVP_TIME_RATE_59_94	   (16683/2)/**< 525/2/15.73427 = 16.68333 */
#define CNVP_TIME_RATE_60		   (16666/2)/**< 1/60 = 16.66667 */
	
/* TIME OUT */
#define CNVP_MODULE_TIMEOUT			   (1000)/**< 1000 msec */
#define CNVP_THREAD_TIMEOUT			   (3000)/**< 3000 msec */
#define CNVP_MODULE_WAIT_TIME		   (1)   /**< 1 msec */
#define CNVP_THREAD_WAIT_TIME		   (10)  /**< 10 msec */

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
* \enum CNVP_THREAD_COMMAND
*/
typedef enum CNVP_THREAD_COMMAND{
	CNV_COMMAND_NONE,  		/**< nothing */
	CNV_COMMAND_CONVERT,  	/**< converter start */
	CNV_COMMAND_DONE, 		/**< converter done */
	CNV_COMMAND_FLUSHING, 	/**< converter flushing */
	CNV_COMMAND_CLOSE,		/**< converter close*/
	CNV_COMMAND_EXIT, 		/**< converter exit */
	CNV_COMMAND_WAITING,    /**< converter wait for timeout */
	CNV_COMMAND_TIMEOUT,	/**< converter timeout */
}CNVP_THREAD_COMMAND;

/**
* \enum CNVP_EVENTTYPE
*/
typedef enum {
	CNV_EVENT_NONE,
	CNV_EVENT_EMPTYBUFFER,
	CNV_EVENT_FILLBUFFER,
	CNV_EVENT_DONE,
	CNV_EVENT_CLOSE,
	CNV_EVENT_FLUSHING,
	CNV_EVENT_FATAL,
	CNV_EVENT_TIMEOUT,
	CNV_EVENT_WAITING,
	CNV_EVENT_INIT = 0xFFFFFFFF,
} CNVP_EVENTTYPE;

/**
* \enum CNVP_BUFFER_FLAGTYPE
*/
typedef enum {
	CNV_BUFFER_NONE,
	CNV_BUFFER_KEEP,
	CNV_BUFFER_RELEASE,
} CNVP_BUFFER_FLAGTYPE;

/**
* \enum CNVP_COLOR_FORMATTYPE
*/
typedef enum CNVP_COLOR_FORMATTYPE {
    CNVP_COLOR_FormatYUV411Planar 		= 17,
    CNVP_COLOR_FormatYUV411PackedPlanar = 18,
    CNVP_COLOR_FormatYUV420Planar		= 19,
    CNVP_COLOR_FormatYUV420PackedPlanar = 20,
    CNVP_COLOR_FormatYUV420SemiPlanar	= 21,
    CNVP_COLOR_FormatYUV422Planar 		= 22,
    CNVP_COLOR_FormatYUV422PackedPlanar = 23,
    CNVP_COLOR_FormatYUV422SemiPlanar   = 24,
} CNVP_COLOR_FORMATTYPE;

/**
* \enum CNVP_STATETYPE
*/
typedef enum CNVP_STATETYPE{
	CNVP_STATE_IDLE,		/**<IDLE STATE */
	CNVP_STATE_EXECUTING,	/**<EXECUTING STATE */
	CNVP_STATE_WAITING,		/**<WAITING STATE */
	CNVP_STATE_FATAL,		/**<FATAL STATE */
}CNVP_STATETYPE;

/**
* \enum CNVP_PIC_STRUCT
*/	
typedef enum {
	  CNVP_PIC_TOP_FIELD				= 0x00L
	, CNVP_PIC_BOTTOM_FIELD				= 0x01L
	, CNVP_PIC_FRAME					= 0x02L
} CNVP_PIC_STRUCT ;

/**
* \enum CNVP_FRAME_RATE_CODE
*/		
typedef enum {
	CNVP_FRAME_RATE_CODE_FORBIDDEN			= 0x00L,
	CNVP_FRAME_RATE_CODE_23_976				= 0x01L,
	CNVP_FRAME_RATE_CODE_24					= 0x02L,
	CNVP_FRAME_RATE_CODE_25					= 0x03L,
	CNVP_FRAME_RATE_CODE_29_97				= 0x04L,
	CNVP_FRAME_RATE_CODE_30					= 0x05L,
	CNVP_FRAME_RATE_CODE_50					= 0x06L,
	CNVP_FRAME_RATE_CODE_59_94				= 0x07L,
	CNVP_FRAME_RATE_CODE_60					= 0x08L,
	CNVP_FRAME_RATE_CODE_EXTENDED			= 0x0FL,
} CNVP_FRAME_RATE_CODE ;

/**
* \enum CNVP_DISP_ORDER
* \progressive_frame : picture_structure : repeat_first_field : top_field_first
*/	
typedef enum {
	 CNVP_DISP_ORDER_P		= 0x00L,/**<1 frame output: X:11:0:0 */
	 CNVP_DISP_ORDER_PP		= 0x01L,/**<2 frame output: X:11:1:0 */
	 CNVP_DISP_ORDER_PPP	= 0x02L,/**<3 frame output: X:11:1:1 */
	 CNVP_DISP_ORDER_TB		= 0x03L,/**<2 field output: 0:11:0:0 */
	 CNVP_DISP_ORDER_BT		= 0x04L,/**<2 field output: 0:11:0:1 */
	 CNVP_DISP_ORDER_TBT	= 0x05L,/**<3 field output: 1:11:1:1 */
	 CNVP_DISP_ORDER_BTB	= 0x06L,/**<3 field output: 1:11:1:0 */
	 CNVP_DISP_ORDER_T		= 0x10L,/**<1 field output: 0:01:1:0 */
	 CNVP_DISP_ORDER_B		= 0x11L,/**<1 field output: 0:10:0:0 */
	 CNVP_DISP_ORDER_XX		= 0x12L,/**<2 field output: No define */
} CNVP_DISP_ORDER ;

/**
* \struct CNVP_CONVERT_BUFFER_T
*/
typedef struct {
	CNV_BUFFERHEADER_T *pConvertBuffer;
	CNV_U32			   nReleaseFlag;
} CNVP_CONVERT_BUFFER_T;

/**
* \struct CNVP_RELEASE_BUFFER_T
*/

typedef struct {
	CNV_BUFFERHEADER_T *pConvertBuffer;
	CNV_U32			   nReleaseFlag;
} CNVP_RELEASE_BUFFER_T;

/**
* \struct CNV_BUFFER_QUEUE_T
*/
typedef struct {
	CNV_U32					nQueueSize;			/**<Queue index */
	CNV_U32					nFirstElementIndex;	/**<Head buffer */
	CNV_U32					nLastElementIndex;	/**<Last buffer */
	CNV_U32					nCountOfBuffers;	/**<Counter */
	CNV_BUFFERHEADER_T 		sBuffer[CNV_COUNT_QUEUE_ELEMENTS];	/**<Buffer information */
	CNV_BUFFERHEADER_T 		*pBuffer[CNV_COUNT_QUEUE_ELEMENTS];	/**<Buffer information */
}CNV_BUFFER_QUEUE_T;

/**
* \struct CNVP_CMN_HANDLE_T
*/
typedef struct{
	CNV_U32					nQueueMutex;		/**< Mutex */
	CNV_BUFFER_QUEUE_T 		sEmptyBufferQueue; /**<Input Buffer */
	CNV_BUFFER_QUEUE_T 		sFillBufferQueue;  /**<Output Buffer*/
	CNV_BOOL				bThreadInit;
}CNVP_CMN_HANDLE_T;

/**
* \struct CNV_PLUGIN_DICISION_RULE_T
*/
typedef struct {
	CNV_U32		nPlugInId;						/**<plugin Id */
	CNV_BOOL	bBottomFirst;					/**<BottomFirst or TopFirst*/
	CNV_BOOL	bInterlacedSequence; 			/**<Sequence mode */
	CNV_U32		nDeinterlace; 					/**<Type of IP conversion */
	CNV_BOOL	bImageFormatConversion; 		/**<Image format conversion,   CNV_TRUE: Enabled,  CNV_FALSE: Disable */
	CNV_U32		nInputFormat;					/**<Input format*/
	CNV_U32		nOutputFormat;					/**<Output format*/
	CNV_BOOL	bCropping;						/**<Cropping,   CNV_TRUE: Enabled,  CNV_FALSE: Disable */
	CNV_BOOL	bScaling;						/**<Scaling,   CNV_TRUE: Enabled,  CNV_FALSE: Disable */
	CNV_BOOL	bTileMode;
}CNV_PLUGIN_DICISION_RULE_T;

/**
* \struct CNVP_PLUGIN_FUNCS_T
*/
typedef struct {
	CNV_ERRORTYPE (*CNVP_CreateHandle)(CNV_PTR *hPluginHandle, CNV_U32 nConvertMode, CNV_SUBERRORTYPE *peSubErrorCode);
	CNV_ERRORTYPE (*CNVP_DeleteHandle)(CNV_PTR hPluginHandle);
	CNV_ERRORTYPE (*CNVP_ModuleInit)(CNV_PTR pParam, CNV_PTR *ppBuffer);
	CNV_BOOL	  (*CNVP_CheckConvertStart)(CNV_PTR pParam);
	CNV_ERRORTYPE (*CNVP_ConverterExecuting)(CNV_PTR pParam, CNV_BOOL bStart,CNV_SUBERRORTYPE *peSubErrorCode);
	CNV_ERRORTYPE (*CNVP_CheckReleaseBuffer)(CNV_PTR pParam,CNV_PTR *ppEmptyBuffer,CNV_PTR *ppFillBuffer);
	CNV_ERRORTYPE (*CNVP_GetFlushBuffer)(CNV_PTR pParam,CNV_PTR *ppEmptyBuffer,CNV_PTR *ppFillBuffer);
	CNV_ERRORTYPE (*CNVP_PluginSelected)(CNV_PLUGIN_DICISION_RULE_T *psDicisionRule, CNV_BOOL *pbResult, CNV_SUBERRORTYPE *peSubErrorCode);
}CNVP_PLUGIN_FUNCS_T;

/**
* \struct CNVP_CALLBACK_FUNCS_T
*/
typedef struct {
	CNV_HANDLE  hCnvHandle; /**< Converter IF*/
	CNV_VOID (*PlugIn_CallBackEvent)( /**< Fatal or FlushDone Callback */
		CNV_HANDLE hCnvHandle,
		CNV_CALLBACK_EVENTTYPE	eEventId
	);
} CNVP_PLUGIN_LOCAL_EVENT_T;

/*
* \struct CNVP_EXTENSION_PARAM_T
*/	
typedef struct {
	CNV_PTR		pParameterList;
	CNV_BOOL	(*CNVP_GetExtensionParameter)(CNV_PTR pParamList,CNV_STRING cIndexName,CNV_PTR *pParam);	
} CNVP_EXTENSION_PARAM_T;	

/*
* \struct CNVP_TIMEOUT_INFO_T
*/	
typedef struct {
	CNV_U32 nTimeout;
	CNV_U32 nWaitTime;
	CNV_U32 nWaitCount;
} CNVP_TIMEOUT_INFO_T;

/**
* \struct CNVP_CONTEXT_T
*/
typedef struct {
	CNV_PTR							pCnvpDepHandle;				/**<Plugin dependence handle*/
	CNV_PTR 						pCnvpCmnHandle;				/**<Plugin common handle*/
	CNVP_PLUGIN_FUNCS_T				sPluginFunc;				/**<Function pointer*/
	CNVP_PLUGIN_LOCAL_EVENT_T		sPluginlocalEvent;			/**<Loacal Event callback*/
	CNV_PLUGIN_DICISION_RULE_T		sPluginRule;				/**<Rule of select*/
	CNV_CALLBACK_FUNCS_T 	 		sCallbackFunc;				/**<Converter callback*/
	CNV_U32							nThreadId;					/**<thread ID*/
	CNV_U32							nConvertMode;				/**<Convert mode*/
	CNV_U32							nThreadState;				/**<Thread state*/
	CNV_PTR							pUserPointer;				/**<User pointer*/
	CNV_BOOL						bFatalFlag;					/**<Fatal flag*/
	CNV_BOOL						bFlushFlag;					/**<CommandFlush */
	CNV_BOOL						bCloseFlag;					/**<Close*/
	CNV_U32							nCountofInput;				/**<Count of input buffer*/
	CNV_U32							nCountofOutput;				/**<Count of output buffer*/
	CNV_OUTPUT_PIC_INFO_T			sOutputPicInfo;				/**<Output information*/
	CNV_SETTING_TILE_PARAMETER_T	sS3Param;					/**<Parameter of S3Ctrl */
	CNV_BOOL						bModuleInit;				/**<First Input Flag */
	CNVP_EXTENSION_PARAM_T			sExtensionInfo;				/**<Extension Info*/
	CNVP_TIMEOUT_INFO_T				sTimeoutInfo;				/**<Timeout Info*/
	CNV_BOOL						bDisableTimeout;			/**<Disable timeout*/
}CNVP_CONTEXT_T;

/**
* \struct CNV_RESERVE_T
*/
typedef struct {
	CNV_U8  reserve01;
	CNV_U32 reserve02;
}CNV_RESERVE_T;
	
/**
* \struct CNVP_PIC_INFOMATION_T
*/
typedef struct {
	CNV_U32						struct_size;								/**< struct_size */
	/* debug information */
	CNV_U32						reserve001;									/**< Reserved */
	CNV_U32						reserve002[ CNVP_RESERVED_001 ] ;			/**< Reserved */
	CNV_U32						reserve003;									/**< Reserved */
	CNV_U32						reserve004;									/**< Reserved */
	CNV_U32						reserve005;									/**< Reserved */
	CNV_U32						reserve006;									/**< Reserved */
	CNV_U32						reserve007;									/**< Reserved */
	CNV_U32						reserve008;									/**< Reserved */
	CNV_U32						reserve009;									/**< Reserved */
	CNV_U32						reserve010;									/**< Reserved */
	CNV_U32						reserve011;									/**< Reserved */
	CNV_U32						reserve012;									/**< Reserved */
	CNV_S32						reserve013[ CNVP_RESERVED_002 ] ;			/**< Reserved */
	CNV_S32						reserve014[ CNVP_RESERVED_002 ] ;			/**< Reserved */
	CNV_U32						reserve015;									/**< Reserved */
	CNV_U32						reserve016;									/**< Reserved */
	CNV_U32						reserve017;									/**< Reserved */
	CNV_U32						reserve018;									/**< Reserved */
	CNV_U32						reserve020;									/**< Reserved */
	CNV_U32						reserve021;									/**< Reserved */
	CNV_U32						reserve022;									/**< Reserved */
	CNV_U32						reserve023[ CNVP_RESERVED_003 ] ;			/**< Reserved */
	CNV_U32						reserve024;									/**< Reserved */
	/* picture information */
	CNVP_PIC_STRUCT				picture_structure[ CNVP_PIC_STRUCT_NUM ] ;	/**< picture_structure */
	CNVP_DISP_ORDER				plane_disp_order ;							/**< plane_disp_order */
	/* debug information */
	CNV_U32						reserve027;									/**< Reserved */
	CNV_U32						reserve028;									/**< Reserved */
	CNV_U32						reserve029;									/**< Reserved */
	CNV_U32						reserve030[ CNVP_RESERVED_003 ] ;			/**< Reserved */
	CNV_U32						aspect_ratio_code ;							/**< aspect_ratio_code */
	CNV_U32						colour_primaries ;							/**< colour_primaries */
	CNV_U32						matrix_coefficients ;						/**< matrix_coefficients */
	CNV_U32						transfer_characteristics ;					/**< transfer_characteristics */
	CNV_U32						reserve031;									/**< Reserved */
	CNV_U32						reserve032;									/**< Reserved */
	CNV_U32						reserve033;									/**< Reserved */
	CNV_U32						reserve034;									/**< Reserved */
	CNV_U32						reserve035;									/**< Reserved */
	/* frame rate */
	CNVP_FRAME_RATE_CODE		frame_rate_code ;							/**< frame_rate_code */
	CNV_U32						frame_rate_n ;								/**< frame_rate_n */
	CNV_U32						frame_rate_d ;								/**< frame_rate_d */
	/* debug information */
	CNV_U32						reserve036[ CNVP_RESERVED_003 ] ;			/**< Reserved */
	CNV_U32						reserve037[ CNVP_RESERVED_003 ] ;			/**< Reserved */
	CNV_U32						reserve038;									/**< Reserved */
	CNV_U32						reserve039;									/**< Reserved */
	CNV_U32						reserve040[ CNVP_RESERVED_003 ] ;			/**< Reserved */
	CNV_U32						reserve041[ CNVP_RESERVED_003 ] ;			/**< Reserved */
	CNV_RESERVE_T				reserve042[ CNVP_RESERVED_004 ] ;			/**< Reserved */
	CNV_U32						reserve044[ CNVP_RESERVED_005 ] ;			/**< Reserved */
	CNV_U32						reserve045;									/**< Reserved */
} CNVP_EXTEND_INFORMATION_T;

	
/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/
/**
* The CnvpCmn_CreateHandle will create a common handle.
*
* \param[out]  pParam                 common handle
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCmn_CreateHandle( CNV_PTR *pCnvpCmnHandle);

/**
* The CnvpCmn_SetSubErrorCode will set a sub error code.
*
* \param[out]  pSubErrorCode          sub error code
* \param[in]   pSubErrorCode          error code
* \return none
*/
CNV_VOID CnvpCmn_SetSubErrorCode( CNV_SUBERRORTYPE 	*pSubErrorCode,CNV_SUBERRORTYPE eErrorCode);

/**
* The CnvpCmn_CreateCnvertThread will create the converter thread.
*
* \param[in]  param                   common handle
* \param[out] pSubErrorCode           error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCmn_CreateCnvertThread( CNV_PTR param , CNV_SUBERRORTYPE *pSubErrorCode);

/**
* The CnvpCmn_SendEvent will send a converter event.
*
* \param[in]  param                   common handle
* \param[in]  eEventId                event id
* \param[in]  param                   data
* \param[out] pSubErrorCode           error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCmn_SendEvent( CNV_PTR param, CNV_U32 eEventId, CNV_PTR pData ,CNV_SUBERRORTYPE *pSubErrorCode);

/**
* The CnvpCmn_ThreadJoin will exit a converter thread.
*
* \param[in]  param                   common handle
* \param[out] pSubErrorCode           sub error code
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCmn_ThreadJoin( CNV_PTR param, CNV_SUBERRORTYPE *pSubErrorCode);


/**
* The CnvpCmn_RemoveBufferToQueue will remove a buffer to converter thread.
*
* \param[in]  pHandle                 common handle
* \param[in]  eEventId                event id
* \param[in]  pConvertData            input data( include input buffer )
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCmn_RemoveBufferToQueue(CNVP_CMN_HANDLE_T *pHandle, CNV_U32 eEventId, CNVP_CONVERT_BUFFER_T *pConvertData);

/**
* The CnvpCmn_AddBufferToQueue will add a buffer to converter thread.
*
* \param[in]  pHandle                 common handle
* \param[in]  eEventId                event id
* \param[in]  pBuffer                 input buffer
* \return CNV_ERRORTYPE
* \retval CNV_ERROR_NONE              success
* \retval others                      failure
*/
CNV_ERRORTYPE CnvpCmn_AddBufferToQueue(CNVP_CMN_HANDLE_T *pHandle,CNV_U32 eEventId,CNV_BUFFERHEADER_T *pBuffer);

/**
* The CnvpCmn_GetFrameRate will get a frame rate.
*
* \param[in]  pBuffer                 input buffer
* \param[out] nFrameTime              1/frame rate
* \return none
*/
CNV_VOID CnvpCmn_GetFrameRate(CNV_BUFFERHEADER_T *pBuffer,CNV_U32 *nFrameTime);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
