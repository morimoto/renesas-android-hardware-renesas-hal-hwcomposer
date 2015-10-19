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
 * OMX Utility function header file for OMX Core
 *
 * The header file defines the HW/OS dependent function.
 *
 * \file omxr_dep_common.h
 *
 */
 

#ifndef _OMXR_DEP_COMMON_H_
#define _OMXR_DEP_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include "omxr_module_common.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/**
 * Max Config File Name Length.
 */
#define OMXR_MAX_CONFIG_FILE_NAME        256

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/**
 * const string type
 */
typedef const char* const OMXR_CONST_STRING;

/* Export configuration function */
/**
 * Get extension section information of configuration file
 */
typedef OMX_ERRORTYPE (*OMXR_CONFIG_GET_EXTSECTION)(const OMX_STRING strSection, const OMX_STRING strParam, OMX_U32 **ppu32Array, OMX_U32 *pu32Num);
/**
 * Get extension parameter information of configuration file
 */
typedef OMX_ERRORTYPE (*OMXR_CONFIG_GET_EXTPARAMETER)(OMX_STRING strMcName, const OMX_STRING strParam, OMX_U32 **ppu32Array, OMX_U32 *pu32Num);

/**
 * export the function getting configuration information
 */
typedef struct tagOMXR_CONFIG_EXPORT_FUNC {
    OMXR_CONFIG_GET_EXTSECTION    OmxrConfig_GetExtSection;      /**<  pointer of GetExtSection */
    OMXR_CONFIG_GET_EXTPARAMETER  OmxrConfig_GetExtParameter;    /**<  pointer of GetExtParamter */
} OMXR_CONFIG_EXPORT_FUNC;

/****************************************************************************/
/*    Function prototypes                                                   */
/****************************************************************************/

/**
 * The OmxrConfig_GetDefaultPath function will get default path of configuration file.
 *
 * The default path of the configuration file can be defined in this function.
 *
 * \return		OMX_STRING	path name
 */
OMX_STRING OmxrConfig_GetDefaultPath(
    void);

/**
 * The OmxrConfig_RegistExportFunc function will register a function of the configuration-related that are defined in the external.
 *
 * Connect the configuration definition process included in the OMX Core.
 *
 * \param[in] psExportFunc OMXR_CONFIG_EXPORT_FUNC structure pointer
 * \return		none
 */
void OmxrConfig_RegistExportFunc(
    OMXR_CONFIG_EXPORT_FUNC* psExportFunc);

/**
 * The OmxrLoadFile function will load binary file.
 *
 * Area of the loaded data will be allocated in this function.
 *
 * \param[in] strPathname Pathname
 * \param[in] u32Align Alignment
 * \param[out] ppvData File data
 * \param[out] pu32Length File size
 * \return OMX_ERROTTYPE
 */
OMX_ERRORTYPE OmxrLoadFile(
    OMX_STRING strPathname,
    OMX_U32 u32Align,
    OMX_PTR *ppvData,
    OMX_U32 *pu32Length);

/**
 * The OmxrUnloadFile function will unload the loaded data in OmxrLoadFile.
 *
 * Area of the loaded data in OmxrLoadFile will be freed in this function.
 *
 * \param[in] pvData Top of the data
 * \return OMX_ERROTTYPE
 */
OMX_ERRORTYPE OmxrUnloadFile(
    OMX_PTR pvData);

/**
 * The OmxrFileOpen function will open text file.
 *
 * This function has the same functionality as the "fopen".
 *
 * \param[in] strPathname Pathname
 * \param[in] mode file attribute (for example "r", "w", "rw")
 * \return file handle
 */
OMX_PTR OmxrFileOpen(
    OMX_STRING strPathname,
    OMX_STRING mode);

/**
 * The OmxrFileReadLine function will get one line from the file handle.
 *
 * This function has the same functionality as the "fgets".
 *
 * \param[in] sLine buffer pointer
 * \param[in] s32MaxSize max size of buffer
 * \param[in] pvHandle file handle
 * \return start address of the line read.
 */
OMX_STRING OmxrFileReadLine(
    OMX_STRING sLine,
    OMX_S32 s32MaxSize,
    OMX_PTR pvHandle);

/**
 * The OmxrFileClose function will close text file.
 *
 * This function has the same functionality as the "fclose".
 *
 * \param[in] pvHandle file handle
 */
void OmxrFileClose(
    OMX_PTR pvHandle);

/**
 * The OmxrLoadDll function will load the dynamic link library.
 *
 * If the Media Component is provided in the dynamic link library, 
 * Media Component DLL is loaded into the memory by this function.
 *
 * \param[in] strPathname Pathname of DLL file
 * \param[out] phDll Handle of DLL
 * \return none
 */
OMX_ERRORTYPE OmxrLoadDll(
    OMX_STRING strPathname,
    OMX_U32 *phDll);

/**
 * The OmxrUnloadDll function will unload the dynamic link library.
 *
 * \param[in] hDll Handle of DLL
 * \return OMX_ERRORTYPE
 */
OMX_ERRORTYPE OmxrUnloadDll(
    OMX_U32 hDll);

/**
 * The OmxrGetDllEntry function will return the specified entry in the DLL.
 *
 * If the function the specified entry is not found, this function returns an error.
 *
 * \param[in] hDll Handle of DLL
 * \param[in] strEntryName Entry name
 * \param[out] ppvEntry Entry point
 * \return OMX_ERRORTYPE
 */
OMX_ERRORTYPE OmxrGetDllEntry(
    OMX_U32 hDll,
    OMX_STRING strEntryName,
    OMX_PTR *ppvEntry);

/**
 * The OmxrLogInit function will initialize the log function.
 *
 * \return none

*/
void OmxrLogInit(
    void);

/**
 * The OmxrLogDone function will be the end processing of the log function.
 *
 * \return none
*/
void OmxrLogDone(
    void);

/**
 * The OmxrLogSetMode function will save value of log mode.
 *
 * \return none
*/
void OmxrLogSetMode(
    OMX_U32 u32LogMode);


/**
 * The OmxrOpenOsWrapper function will initialize the os wrapper function.
 *
 * Please call only once when using the OS wrapper.
 *
 * \return none
*/
void OmxrOpenOsWrapper(
    void);

/**
 * The OmxrCloseOsWrapper function will be the end processing of the os wrapper function.
 *
 * \return none
*/
void OmxrCloseOsWrapper(
    void);

/**
 * The OmxrInitMalloc function will initialize the memory alloc function.
 *
 * This function is handled on the following cases.\n
 *  For example, if it is necessary to get a memory pool at startup.
 *
 * \return none
*/
void OmxrInitMalloc(
    void);

/**
 * The OmxrDeinitMalloc function will be the end processing of the memory alloc function.
 *
 * \return none
*/
void OmxrDeinitMalloc(void);

/**
 * The OmxrSprintfVa function will store formatted string to buffer.
 *
 * \param[out] strBuffer Buffer to store a atring.
 * \param[in] strString String
 * \param[in] arglist Arguments
 * \return OMX_S32 Formatted string byte length. (not include NUL terminator.) or -1 if error.
*/
OMX_S32 OmxrSprintfVa(
    OMX_STRING strBuffer,
    const OMX_STRING strString,
    va_list arglist);

#ifdef __cplusplus
}
#endif

#endif /* _OMXR_DEP_COMMON_H_ */
