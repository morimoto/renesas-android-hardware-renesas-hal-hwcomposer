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
 * OMXR String management function
 *
 * This file implements the string function.
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

OMX_PTR OmxrMemcpy(OMX_PTR pvDest, const OMX_PTR pvSrc, OMX_U32 u32Size)
{
    return memcpy(pvDest, pvSrc, (size_t)u32Size);
}


OMX_PTR OmxrMemset(OMX_PTR pvDest, OMX_U8 u8Code, OMX_U32 u32Size)
{
    return memset(pvDest, (OMX_S32)u8Code, (size_t)u32Size);
}


OMX_U32 OmxrStrlen(const OMX_STRING strString)
{
    return (OMX_U32)strlen(strString);
}


OMX_S32 OmxrStrcmp(const OMX_STRING strString1, const OMX_STRING strString2)
{
    return (OMX_S32)strcmp(strString1, strString2);
}


OMX_S32 OmxrSprintf(OMX_STRING strBuffer, const OMX_STRING strString, ...)
{
    va_list vaArgs;
    OMX_S32 s32Result;

    va_start(vaArgs, strString);
    s32Result = OmxrSprintfVa(strBuffer, strString, vaArgs);
    va_end(vaArgs);
    return s32Result;
}


OMX_S32 OmxrSprintfVa(OMX_STRING strBuffer, const OMX_STRING strString, va_list arglist)
{
    OMX_S32 s32Result;

    s32Result = vsprintf(strBuffer, strString, arglist);
    if (EOF == s32Result) {
        s32Result = -1;
    }
    return s32Result;
}
