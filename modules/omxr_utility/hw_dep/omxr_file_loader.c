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
 * OMXR File loader
 *
 * This file implements the binary file loader function.
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <omxr_dep_common.h>

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

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/

OMX_ERRORTYPE OmxrLoadFile(OMX_STRING strPathname, OMX_U32 u32Align, OMX_PTR *ppvData, OMX_U32 *pu32Length)
{
    FILE *strm;

    /* Open a file */
    strm = fopen(strPathname, "rb");
    if (strm == NULL) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "File load failed. [fopen] (path=%s)", strPathname);
        return (OMX_ERRORTYPE)OMXR_ErrorFileNotFound;
    }
    /* Get file size */
    if (fseek(strm, 0, SEEK_END) != 0) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "File load failed. [fseek] (path=%s)", strPathname);
        (void)fclose(strm);
        return (OMX_ERRORTYPE)OMXR_ErrorFileRead;
    }
    *pu32Length = (OMX_U32)ftell(strm);
    if (fseek(strm, 0, SEEK_SET) != 0) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "File load failed. [ftell] (path=%s)", strPathname);
        (void)fclose(strm);
        return (OMX_ERRORTYPE)OMXR_ErrorFileRead;
    }
    /* Get memory to data store */
    *ppvData = malloc(*pu32Length);
    if (*ppvData == NULL) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "File load failed. [malloc] (path=%s)", strPathname);
        (void)fclose(strm);
        return OMX_ErrorUndefined;
    }
    /* Read from file */
    if (fread(*ppvData, *pu32Length, 1, strm) != 1) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "File load failed. [fread] (path=%s)", strPathname);
        free(*ppvData);
        (void)fclose(strm);
        return (OMX_ERRORTYPE)OMXR_ErrorFileRead;
    }
    /* Close file handle */
    (void)fclose(strm);

    OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "File loaded. (path=%s, Adrs=0x%X)", strPathname, *ppvData);

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxrUnloadFile(OMX_PTR pvData)
{
    OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "File unloaded. (Adrs=0x%X)", pvData);

    /* free memory */
    free(pvData);
    return OMX_ErrorNone;
}

OMX_PTR OmxrFileOpen(OMX_STRING strPathname, OMX_STRING mode)
{
    FILE *strm;
    
    strm = fopen(strPathname, mode);
    if (NULL == strm) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_WARN, "File open failed. [fopen] (path=%s)", strPathname);

    } else {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "File is opened. (path=%s, handle=%p)", strPathname, strm);
	}
	
	return (OMX_PTR)strm;
}
    
OMX_STRING OmxrFileReadLine(OMX_STRING sLine, OMX_S32 s32MaxSize, OMX_PTR pvHandle)
{
    FILE *strm = (FILE *)pvHandle;

	return (OMX_STRING)(fgets(sLine, s32MaxSize, strm));
}
    
void OmxrFileClose(OMX_PTR pvHandle)
{
    FILE *strm = (FILE *)pvHandle;

    OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "File is cloesed. (handle=%p)", pvHandle);

	(void)fclose(strm);
	
	return ;
}
