/********************************************************************//**
 *
 * PURPOSE
 *   IOCTL definitions
 *
 * AUTHOR
 *   \author Renesas Electronics Corporation
 *
 * DATE
 *   $Id:$
 *
 **********************************************************************/
/*
 * Copyright(C) 2011 - 2013 Renesas Electronics Corporation. All Rights Reserved.
 * RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY
 * This program must be used solely for the purpose for which
 * it was furnished by Renesas Electronics Corporation.
 * No part of this program may be reproduced or disclosed to
 * others, in any form, without the prior written permission
 * of Renesas Electronics Corporation.
 *
 **********************************************************************/

#ifndef UVCS_IOCTL_H
#define UVCS_IOCTL_H

#define UVCS_IOCTL_BASE				(0xE0)

#define UVCS_IOCTL_SET_PREEMPT_MODE	_IOW( UVCS_IOCTL_BASE, 0x01, ulong )
#define UVCS_IOCTL_CLR_PREEMPT_MODE	_IO(  UVCS_IOCTL_BASE, 0x02 )

#define UVCS_IOCTL_DUMP_GET_SIZE	_IOR( UVCS_IOCTL_BASE, 0x11, ulong * )
#define UVCS_IOCTL_DUMP_OUTPUT		_IOR( UVCS_IOCTL_BASE, 0x12, void * )
#define UVCS_IOCTL_GET_IP_INFO		_IOR( UVCS_IOCTL_BASE, 0x13, void * )

#endif /* UVCS_IOCTL_H */
