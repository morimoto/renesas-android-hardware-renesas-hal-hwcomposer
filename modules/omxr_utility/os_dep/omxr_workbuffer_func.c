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
 * OMXR workbuffer allocate/free function
 *
 * This file implements the buffer allocation function.
 *
 * \file
 * \attention
 */
 
/***************************************************************************/
/*    Include Files                                                        */
/***************************************************************************/

#include <stdlib.h>
#include <omxr_dep_common.h>
#include "mmngr_user_public.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define ALLOCATE_ALIGNE_XYADDRESSING_SIZE   (0x00010000) /* 32 * 2048 */

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

typedef struct tagOMXR_WORKBUFFER_HANDLE_INFO {
    OMX_U32 u32MmHandle;        /**< Memory Manager Handle */
    OMX_U32 u32BaseVirtAddr;    /**< Address that was mapped to the CPU */
    OMX_U32 u32BaseHardIpAddr;  /**< Address that was mapped to the Hardware IP */
    OMX_U32 u32BassPhysAddr;    /**< Physcal address */
    OMX_U32 u32AddrOffset;      /**< Offset */
    OMX_U32 u32ValidSize;       /**< Valied size */
    OMX_U32 u32AllocSize;       /**< Allocated size */
} OMXR_WORKBUFFER_HANDLE_INFO;

/***************************************************************************/
/*    Function Prototypes (private)                                        */
/***************************************************************************/

/**
 * OmxrAllocateWorkBuffer_allocfunc
 *
 * This function allocates the work buffer.
 *
 * \param[in,out] psInfo Buffer handle
 * \param[in] u32ReqSize Require size
 * \param[in] u32ReqAlign Align byte
 * \param[in] strBufferDiscri Buffer discriptor
 * \param[in] u32Flags Mapping flag
 * \return OMX_ERROTTYPE
 */
static OMX_ERRORTYPE OmxrAllocateWorkBuffer_allocfunc(OMXR_WORKBUFFER_HANDLE_INFO *psInfo, OMX_U32 u32ReqSize, OMX_U32 u32ReqAlign, OMX_STRING strBufferDiscri, OMX_U32 u32Flags);

/**
 * OmxrAllocateWorkBuffer_freefunc
 *
 * This function free the work buffer.
 *
 * \param[in,out] psInfo Buffer handle
 * \return OMX_ERROTTYPE
 */
static OMX_ERRORTYPE OmxrAllocateWorkBuffer_freefunc(OMXR_WORKBUFFER_HANDLE_INFO *psInfo);

/**
 * OmxrisPowerOfTwo
 *
 * Check power of two
 *
 * \param[in] x value
 * \return OMX_U32
 */
static OMX_U32 OmxrisPowerOfTwo(
    OMX_U32 x);

/**
 * OmxrNextPowerOfTwo
 *
 * Return next power of two
 *
 * \param[in] x value
 * \return OMX_U32
 */
static OMX_U32 OmxrNextPowerOfTwo(
    OMX_U32 x);

/***************************************************************************/
/*    Variables                                                            */
/***************************************************************************/

/***************************************************************************/
/*    Functions                                                            */
/***************************************************************************/

OMX_ERRORTYPE OmxrAllocateWorkBuffer(OMXR_WORKBUFFER_HANDLE* pBufferHandle, OMX_U32 u32Size, OMX_U32 u32Align, OMX_STRING strBufferDiscri, OMX_U32 u32Flags)
{
    OMX_U32 u32CalcAlign, u32CalcSize;
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_ERRORTYPE eError;

    /* Check handle pointer */
    if (NULL == pBufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        return OMX_ErrorBadParameter;
    }
    
    /* Check allocation size */
    if (u32Size == 0) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        return OMX_ErrorBadParameter;
    }

    /* Allocate information area */
    psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)OmxrMalloc(sizeof(OMXR_WORKBUFFER_HANDLE_INFO));
    if (NULL == psInfo) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: malloc error.", "");
        return OMX_ErrorUndefined;
    }
    
    /* Check buffer type - strBufferDiscri Strings*/
    if (!OmxrStrcmp("OMXBUF.VIDEO.WORKBUFFER.DEC.XYADDRESSING", strBufferDiscri)) {
    /* XY Addressing workbuffer case*/
        /* Caluculate a power of two */
        if (!OmxrisPowerOfTwo(u32Size)) {
            u32CalcSize = OmxrNextPowerOfTwo(u32Size);
        } else {
            u32CalcSize = u32Size;
        }
        u32CalcAlign = 1;

        eError = OmxrAllocateWorkBuffer_allocfunc(psInfo, u32CalcSize, u32CalcAlign, strBufferDiscri, u32Flags);
        if (eError == OMX_ErrorNone) {
            /* check aligment */
            if ((psInfo->u32BaseHardIpAddr & (ALLOCATE_ALIGNE_XYADDRESSING_SIZE - 1UL)) != 0) {
                OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_DEBUG, "bad aligment(0x%x).", psInfo->u32BaseHardIpAddr);
                /* If it can not allocate the alignment as intended, it is retried by adding an alignment area. */
                (void)OmxrAllocateWorkBuffer_freefunc(psInfo);
                u32CalcAlign = ALLOCATE_ALIGNE_XYADDRESSING_SIZE;
                eError = OmxrAllocateWorkBuffer_allocfunc(psInfo, u32CalcSize, u32CalcAlign, strBufferDiscri, u32Flags);
            }
        }
    } else {
    /* Other case */
        u32CalcSize  = u32Size;
        u32CalcAlign = u32Align;

        eError = OmxrAllocateWorkBuffer_allocfunc(psInfo, u32CalcSize, u32CalcAlign, strBufferDiscri, u32Flags);
    }
    
    if (eError != OMX_ErrorNone) {
        OmxrFree(psInfo);
        return eError;
    }

    *pBufferHandle = psInfo;
    OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "Allocation Workbuffer Info (pid=0x%x, Handle=0x%x, RequestSize=%ld, RequestAlign=%ld)", psInfo->u32MmHandle, psInfo, u32Size, u32Align);
    OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "[Allocated Buffer Info] strBufferDiscri=%s, u32BaseVirtAddr=%p, u32BaseHardIpAddr=%p, u32BassPhysAddr=%p, u32AddrOffset=%ld, u32ValidSize=%ld, u32AllocSize=%ld, u32Flags=%d",
        strBufferDiscri,
        psInfo->u32BaseVirtAddr,
        psInfo->u32BaseHardIpAddr,
        psInfo->u32BassPhysAddr,
        psInfo->u32AddrOffset,
        psInfo->u32ValidSize,
        psInfo->u32AllocSize,
        u32Flags
    );
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE OmxrAllocateWorkBuffer_allocfunc(OMXR_WORKBUFFER_HANDLE_INFO *psInfo, OMX_U32 u32ReqSize, OMX_U32 u32ReqAlign, OMX_STRING strBufferDiscri, OMX_U32 u32Flags)
{
    MMNGR_ID pid;
    OMX_U32 flags;
    OMX_S32 ret;
    OMX_U32 u32phy, u32hard, u32virt;
    OMX_U32 u32AllocReqSize;

    u32AllocReqSize = (u32ReqSize + u32ReqAlign) - 1UL;

    /* Branch allocation area */
    if (((OMXR_WORKBUFFER_VA_SUPPORT & u32Flags) == OMXR_WORKBUFFER_VA_SUPPORT) && 
        ((OMXR_WORKBUFFER_TA_SUPPORT & u32Flags) == OMXR_WORKBUFFER_TA_SUPPORT)) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Not support flag.", "");
        return OMX_ErrorBadParameter;
    } else if (u32Flags == OMXR_WORKBUFFER_VA_SUPPORT) {
        /* Allocation heap */
        u32virt = (OMX_U32)malloc(u32AllocReqSize);
        if (0 == u32virt) {
            OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Cannot allocate a work buffer. (size=%d)", u32AllocReqSize);
            return OMX_ErrorInsufficientResources;
        }
        psInfo->u32MmHandle         = 0;
        psInfo->u32BaseVirtAddr     = u32virt;
        psInfo->u32BaseHardIpAddr   = 0;
        psInfo->u32BassPhysAddr     = 0;
        psInfo->u32AddrOffset       = ((((u32virt + u32ReqAlign) - 1UL) / u32ReqAlign) * u32ReqAlign) - u32virt;
        psInfo->u32ValidSize        = u32ReqSize;
        psInfo->u32AllocSize        = u32AllocReqSize;
    } else {
        /* Allocation via MMngr */
        if ((OMXR_WORKBUFFER_TA_SUPPORT & u32Flags) == OMXR_WORKBUFFER_TA_SUPPORT) {
            OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_DEBUG, "MMNGR_PA_SUPPORT.", "");
            flags = MMNGR_PA_SUPPORT;
        } else {
            OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_DEBUG, "MMNGR_VA_SUPPORT.", "");
            flags = MMNGR_VA_SUPPORT;
        }

        /* Because of hardware restriction of VCP3, allocate MV info area from the dedicated memory region.*/
        if(!OmxrStrcmp("OMXBUF.VIDEO.WORKBUFFER.DEC.WORK2", strBufferDiscri)){
            flags = MMNGR_PA_SUPPORT_MV;
        }
        
        if (!OmxrStrcmp("OMXBUF.VIDEO.WORKBUFFER.ENC.WORK4", strBufferDiscri)) {
            u32AllocReqSize = (((u32AllocReqSize + ALLOCATE_ALIGNE_XYADDRESSING_SIZE) - 1UL) / ALLOCATE_ALIGNE_XYADDRESSING_SIZE) * ALLOCATE_ALIGNE_XYADDRESSING_SIZE;
        }
        
        ret = mmngr_alloc_in_user(&pid, u32AllocReqSize, &u32phy, &u32hard, &u32virt, flags);
        if (R_MM_OK != ret) {
            if (R_MM_NOMEM == ret) {
                OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: mmngr no mem.", "");
                return OMX_ErrorInsufficientResources;
            } else {
                OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: mmngr fatal.", "");
                return OMX_ErrorUndefined;
            }
        } else {
            psInfo->u32MmHandle = (OMX_U32)pid;
            if ((OMXR_WORKBUFFER_VA_SUPPORT & u32Flags) == OMXR_WORKBUFFER_VA_SUPPORT) {
                psInfo->u32BaseVirtAddr = u32virt;
            } else {
                psInfo->u32BaseVirtAddr = 0;
            }
            if ((OMXR_WORKBUFFER_HA_SUPPORT & u32Flags) == OMXR_WORKBUFFER_HA_SUPPORT) {
                psInfo->u32BaseHardIpAddr = u32hard;
            } else {
                psInfo->u32BaseHardIpAddr = 0;
            }
            if ((OMXR_WORKBUFFER_TA_SUPPORT & u32Flags) == OMXR_WORKBUFFER_TA_SUPPORT) {
                psInfo->u32BassPhysAddr = u32phy;
            } else {
                psInfo->u32BassPhysAddr = 0;
            }
            psInfo->u32AddrOffset = ((((u32hard + u32ReqAlign) - 1UL) / u32ReqAlign) * u32ReqAlign) - u32hard;
            psInfo->u32ValidSize  = u32ReqSize;
            psInfo->u32AllocSize  = u32AllocReqSize;
        }
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE OmxrAllocateWorkBuffer_freefunc(OMXR_WORKBUFFER_HANDLE_INFO *psInfo)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 ret;

    if (psInfo->u32MmHandle != 0UL) {
        ret = mmngr_free_in_user((MMNGR_ID)psInfo->u32MmHandle);
        if (R_MM_OK != ret) {
            OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: mmngr fatal.", "");
            eError = OMX_ErrorUndefined;
        }
        psInfo->u32MmHandle = 0;
    } else {
        free((OMX_PTR)psInfo->u32BaseVirtAddr);
        psInfo->u32BaseVirtAddr = 0;
    }
    
    return eError;
}

OMX_ERRORTYPE OmxrFreeWorkBuffer(OMXR_WORKBUFFER_HANDLE BufferHandle)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_S32 ret;

    if (NULL == BufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        eError = OMX_ErrorBadParameter;
    } else {
        psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)BufferHandle;
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_INFO, "Free work buffer (0x%x)", psInfo);
        /* Free memory block */
        if (psInfo->u32MmHandle != 0UL) {
            ret = mmngr_free_in_user((MMNGR_ID)psInfo->u32MmHandle);
            if (R_MM_OK != ret) {
                OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: mmngr fatal.", "");
                eError = OMX_ErrorUndefined;
            }
        } else {
            free((OMX_PTR)psInfo->u32BaseVirtAddr);
        }
        OmxrFree((OMX_PTR)psInfo);
    }
    
    return eError;
}


OMX_PTR OmxrGetVirtualAddress(OMXR_WORKBUFFER_HANDLE BufferHandle)
{
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_U32 u32addr = 0;

    if (NULL == BufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        u32addr = 0;
    } else {
        psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)BufferHandle;
        if (psInfo->u32BaseVirtAddr != 0UL) {
            u32addr = psInfo->u32BaseVirtAddr + psInfo->u32AddrOffset;
        }
    }
    return (OMX_PTR)u32addr;
}


OMX_U32 OmxrGetHardwareIPAddress(OMXR_WORKBUFFER_HANDLE BufferHandle)
{
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_U32 u32addr = 0;

    if (NULL == BufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        u32addr = 0;
    } else {
        psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)BufferHandle;
        if (psInfo->u32BaseHardIpAddr != 0UL) {
            u32addr = psInfo->u32BaseHardIpAddr + psInfo->u32AddrOffset;
        }
    }
    return u32addr;
}


OMX_U32 OmxrGetTLConvertAddress(OMXR_WORKBUFFER_HANDLE BufferHandle)
{
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_U32 u32addr = 0;

    if (NULL == BufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        u32addr = 0;
    } else {
        psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)BufferHandle;
        if (psInfo->u32BassPhysAddr != 0UL) {
            /* The value of u32BasePhysAddr and the value of this return are value which shifted 12bit. */
            u32addr = psInfo->u32BassPhysAddr + (psInfo->u32AddrOffset>>12);
        }
    }
    return u32addr;
}


OMX_U32 OmxrGetMemoryManagerHandle(OMXR_WORKBUFFER_HANDLE BufferHandle)
{
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_U32 u32Handle;

    if (NULL == BufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        u32Handle = 0;
    } else {
        psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)BufferHandle;
        u32Handle = psInfo->u32MmHandle;
    }
    return u32Handle;
}


OMX_U32 OmxrGetWorkBufferValidSizeBytes(OMXR_WORKBUFFER_HANDLE BufferHandle)
{
    OMXR_WORKBUFFER_HANDLE_INFO *psInfo;
    OMX_U32 u32ValidSizeBytes;

    if (NULL == BufferHandle) {
        OMXR_LOGGER(OMXR_UTIL_LOG_LEVEL_ERROR, "error: Bad parameter.", "");
        u32ValidSizeBytes = 0;
    } else {
        psInfo = (OMXR_WORKBUFFER_HANDLE_INFO *)BufferHandle;
        u32ValidSizeBytes = psInfo->u32ValidSize;
    }
    return u32ValidSizeBytes;
}


static OMX_U32 OmxrisPowerOfTwo(OMX_U32 x)
{
    return ((x & (x -1)) == 0);
}


static OMX_U32 OmxrNextPowerOfTwo(OMX_U32 x)
{
    OMX_U32 i;
    --x;
    for (i = 1; i < 32; i <<= 1) {
        x |= (x >> i);
    }
    return (x + 1);
}
