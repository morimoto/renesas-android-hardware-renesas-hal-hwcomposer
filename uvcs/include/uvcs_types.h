/*************************************************************************/ /*
 UVCS Common

 Copyright (C) 2013 Renesas Electronics Corporation

 License        Proprietary

 This program must be used solely for the purpose for which
 it was furnished by Renesas Electronics Corporation.
 No part of this program may be reproduced or disclosed to
 others, in any form, without the prior written permission
 of Renesas Electronics Corporation.
*/ /*************************************************************************/

#ifndef UVCS_TYPES_H
#define UVCS_TYPES_H
/******************************************************************************/
/*                      INCLUDE FILES                                         */
/******************************************************************************/


/******************************************************************************/
/*                      MACROS/DEFINES                                        */
/******************************************************************************/
#if !defined(NULL)
#if !defined(__cplusplus)
# define NULL ((void *)0)
#else
# define NULL (0)
#endif
#endif

/******************************************************************************/
/*               TYPE DEFINITION                                              */
/******************************************************************************/
typedef unsigned char				UVCS_U8 ;
typedef unsigned long				UVCS_U32 ;
typedef signed long					UVCS_S32 ;
typedef void*						UVCS_PTR ;

typedef enum {
	UVCS_FALSE	= 0,
	UVCS_TRUE	= 1
}	UVCS_BOOL ;

typedef enum {
	UVCS_RTN_OK						= 0x00L,
	UVCS_RTN_INVALID_HANDLE			= 0x01L,
	UVCS_RTN_INVALID_STATE			= 0x02L,
	UVCS_RTN_PARAMETER_ERROR		= 0x03L,
	UVCS_RTN_NOT_SUPPORTED			= 0x04L,
	UVCS_RTN_NOT_CONFIGURED			= 0x05L,
	UVCS_RTN_NOT_INITIALISE			= 0x06L,
	UVCS_RTN_ALREADY_INITIALISED	= 0x07L,
	UVCS_RTN_SYSTEM_ERROR			= 0x08L,
	UVCS_RTN_BUSY					= 0x09L,
	UVCS_RTN_CONTINUE				= 0x0AL
}	UVCS_RESULT ;


#endif /* UVCS_TYPES_H */
