/*************************************************************************/ /*
 UVCS Common

 Copyright (C) 2013 Renesas Electronics Corporation

 License        Dual MIT/GPLv2

 The contents of this file are subject to the MIT license as set out below.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 Alternatively, the contents of this file may be used under the terms of
 the GNU General Public License Version 2 ("GPL") in which case the provisions
 of GPL are applicable instead of those above.

 If you wish to allow use of your version of this file only under the terms of
 GPL, and not to allow others to use your version of this file under the terms
 of the MIT license, indicate your decision by deleting the provisions above
 and replace them with the notice and other provisions required by GPL as set
 out in the file called "GPL-COPYING" included in this distribution. If you do
 not delete the provisions above, a recipient may use your version of this file
 under the terms of either the MIT license or GPL.

 This License is also included in this distribution in the file called
 "MIT-COPYING".

 EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
 PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


 GPLv2:
 If you wish to use this file under the terms of GPL, following terms are
 effective.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ /*************************************************************************/

#ifdef __RENESAS__
#pragma section UVCSCMN
#endif /* __RENESAS__ */


/******************************************************************************/
/*                      INCLUDE FILES                                         */
/******************************************************************************/
#include "uvcs_types.h"
#include "uvcs_cmn_internal.h"
#include "uvcs_cmn_dump.h"

/******************************************************************************/
/*                      MACROS/DEFINES                                        */
/******************************************************************************/

/******************************************************************************/
/*                      LOCAL TYPES                                           */
/******************************************************************************/
/******************************************************************************/
/*                      VARIABLES                                             */
/******************************************************************************/
/******************************************************************************/
/*                      FORWARD DECLARATION                                   */
/******************************************************************************/
/******************************************************************************/
/*                      EXTERNAL FUNCTIONS                                    */
/******************************************************************************/
#if (UVCS_DEBUG == 1)
void uvcs_c_dump_init(struct UVCS_C_INR_INFO *inr)
{
	if ((inr != NULL)
	&& (inr->init_param.debug_log_buff != NULL)
	&& (inr->init_param.debug_log_size >= UVCS_C_DUMP_BUFF_MIN)) {
		struct UVCS_C_DUMP_INFO *cmn_dump;
		inr->dump_buff = inr->init_param.debug_log_buff;
		inr->dump_max  = (inr->init_param.debug_log_size / sizeof(struct UVCS_C_DUMP_DATA)) - 1uL;

		cmn_dump = (struct UVCS_C_DUMP_INFO *)inr->dump_buff; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
		cmn_dump->n = 0;
	}
}

void uvcs_c_dump(struct UVCS_C_INR_INFO *inr, UVCS_U32 c, UVCS_U32 p0, UVCS_U32 p1) /* PRQA S 3673 *//* do not use const. */
{
	struct UVCS_C_DUMP_INFO *cmn_dump;
	struct UVCS_C_DUMP_DATA *dump_data;

	if ((inr != NULL)
	&& (inr->dump_buff != NULL)) {
		cmn_dump = (struct UVCS_C_DUMP_INFO *)inr->dump_buff; /* PRQA S 0310 *//* unavoidable in the casts of the structure. */
		dump_data = &cmn_dump->d;

		dump_data[cmn_dump->n].c  = c;	 /* PRQA S 0491 *//* substitute of the pointer arithmetic. */
		dump_data[cmn_dump->n].p0 = p0; /* PRQA S 0491 *//* substitute of the pointer arithmetic. */
		dump_data[cmn_dump->n].p1 = p1; /* PRQA S 0491 *//* substitute of the pointer arithmetic. */
		dump_data[cmn_dump->n].w  = c;	 /* PRQA S 0491 *//* substitute of the pointer arithmetic. */
		cmn_dump->n = (cmn_dump->n + 1uL) % inr->dump_max;
	}
}

#elif (UVCS_DEBUG == 2)
void uvcs_c_dump(struct UVCS_C_INR_INFO *inr, UVCS_U32 c, UVCS_U32 p0, UVCS_U32 p1)
{
	if (inr != NULL) {
		/* Please insert optional processing here, if needed. */
		uvcs_dump((UVCS_U32)(c), p0, p1);
	}
}

#else
/* not use debug buffer */
#endif /* UVCS_DEBUG */
