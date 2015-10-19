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
 * OMXR DLL module dll loader
 *
 * This file implements the dynamic link library loader function.
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
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

OMX_ERRORTYPE OmxrLoadDll(OMX_STRING strPathname, OMX_U32 *phDll)
{
    OMX_ERRORTYPE result;
    void *m_hDll;
    OMX_STRING error;

    result = (OMX_ERRORTYPE)OMXR_ErrorFileNotFound;
    m_hDll = dlopen(strPathname, RTLD_NOW);
    *phDll = (OMX_U32)m_hDll;
    if (m_hDll != NULL) {
        result = OMX_ErrorNone;
    }

    if (OMX_ErrorNone == result) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "DLL loaded. (Path=%s, Handle=%d)", strPathname, m_hDll);
    } else {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "DLL load failed. (Path=%s)", strPathname);
        error = dlerror();
        if (error != NULL) {
            OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "dlerror() %s", error);
        }
    }
    return result;
}


OMX_ERRORTYPE OmxrUnloadDll(OMX_U32 hDll)
{
    void *m_hDll;

    m_hDll = (void*)hDll;
    (void)dlclose(m_hDll);

    OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "DLL unloaded. (handle=%d)", hDll);

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxrGetDllEntry(OMX_U32 hDll, OMX_STRING strEntryName, OMX_PTR *ppvEntry)
{
    OMX_ERRORTYPE result;
    OMX_STRING error;

    result = OMX_ErrorUndefined;
    *ppvEntry = (void*)dlsym((void*)hDll, strEntryName);
    if (*ppvEntry != NULL) {
        result = OMX_ErrorNone;
    } else {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "DLL entry not found. (name=%s)", strEntryName);
        error = dlerror();
        if (error != NULL) {
            OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "dlerror() %s", error);
        }
    }
    return result;
}
