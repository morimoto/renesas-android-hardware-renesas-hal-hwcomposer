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
 * OMXR Memory management function
 *
 * This file implements the dynamic memory allocation function.
 *
 * \file
 * \attention
 */

/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

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

void OmxrInitMalloc(void)
{
    /* nothing to do */
    return;
}


void OmxrDeinitMalloc(void)
{
    /* nothing to do */
    return;
}


OMX_PTR OmxrMalloc(OMX_U32 u32Size)
{
    OMX_PTR pvPtr;

    pvPtr = malloc(u32Size);

    return pvPtr;
}


OMX_PTR OmxrCalloc(OMX_U32 u32Nmemb, OMX_U32 u32SizeBytes)
{
    OMX_PTR pvPtr;
    OMX_U32 u32Len;
    
    if ((u32Nmemb == 0) || (u32SizeBytes == 0)) {
        return NULL;
    }
    
    if (u32Nmemb > (0xFFFFFFFFUL / u32SizeBytes)) {
        return NULL;
    }
    
    u32Len = u32Nmemb * u32SizeBytes;
    
    pvPtr = malloc(u32Len);
    if (pvPtr == NULL) {
        return NULL;
    }
    
    return memset(pvPtr, 0, u32Len);
}


void OmxrFree(OMX_PTR pvPtr)
{
    free(pvPtr);
}
