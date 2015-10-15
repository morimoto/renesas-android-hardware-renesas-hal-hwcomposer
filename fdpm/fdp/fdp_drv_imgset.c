/*************************************************************************/ /*
 FDPM

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

#include <asm/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include "fdpm_drv.h"
#include "fdpm_if_par.h"
#include "fdp/fdp_depend.h"
#include "fdp/fdp_drv_l.h"
#include "fdp/fdp_drv_lfunc.h"
#include "include/fdpm_log.h"

/******************************************************************************
	Function:		fdp_GetInitIpcParam
	Description:		returns initial IPC setting
******************************************************************************/
long fdp_GetInitIpcParam(T_FDPD_INITIPC_REG *initipc_reg)
{
	int i;
	DPRINT("called\n");
	initipc_reg->smsk_th      = 0x02;
	initipc_reg->cmb_grad     = 0x40;
	initipc_reg->cmb_ofst     = 0x20;
	initipc_reg->cmb_max      = 0x00;
	initipc_reg->mov_coef     = 0x80;/* TYP_BLD */
	initipc_reg->stl_coef     = 0x20;/* STL_BLD */
	initipc_reg->bld_grad     = 0x80;
	initipc_reg->bld_ofst     = 0x02;
	initipc_reg->bld_max      = 0xff;
	initipc_reg->hg_grad      = 0x10;
	initipc_reg->hg_ofst      = 0x00;
	initipc_reg->hg_max       = 0xff;
	initipc_reg->sprs_grad    = 0x90;
	initipc_reg->sprs_ofst    = 0x04;
	initipc_reg->sprs_max     = 0xff;
	initipc_reg->asel45       = 0x04;
	initipc_reg->asel22       = 0x08;
	initipc_reg->asel15       = 0x0c;
	initipc_reg->ipix_max45   = 0xff;
	initipc_reg->ipix_grad45  = 0x10;
	initipc_reg->ipix_max22   = 0xff;
	initipc_reg->ipix_grad22  = 0x10;
	initipc_reg->ipix_max15   = 0xff;
	initipc_reg->ipix_grad15  = 0x10;
	initipc_reg->vfq_th       = 0x20;
	initipc_reg->hfq_th       = 0x20;
	initipc_reg->dif_th       = 0x80;
	initipc_reg->sad_th       = 0x80;
	initipc_reg->detector_sel = 0x00;
	initipc_reg->comb_th      = 0x00;
	initipc_reg->freq_th      = 0x00;

	/* dif_adj */
	initipc_reg->dif_adj[0] = 0x00;
	initipc_reg->dif_adj[1] = 0x24;
	initipc_reg->dif_adj[2] = 0x43;
	initipc_reg->dif_adj[3] = 0x5E;
	initipc_reg->dif_adj[4] = 0x76;
	initipc_reg->dif_adj[5] = 0x8C;
	initipc_reg->dif_adj[6] = 0x9E;
	initipc_reg->dif_adj[7] = 0xAF;
	initipc_reg->dif_adj[8] = 0xBD;
	initipc_reg->dif_adj[9] = 0xC9;
	initipc_reg->dif_adj[10] = 0xD4;
	initipc_reg->dif_adj[11] = 0xDD;
	initipc_reg->dif_adj[12] = 0xE4;
	initipc_reg->dif_adj[13] = 0xEA;
	initipc_reg->dif_adj[14] = 0xEF;
	initipc_reg->dif_adj[15] = 0xF3;
	initipc_reg->dif_adj[16] = 0xF6;
	initipc_reg->dif_adj[17] = 0xF9;
	initipc_reg->dif_adj[18] = 0xFB;
	initipc_reg->dif_adj[19] = 0xFC;
	initipc_reg->dif_adj[20] = 0xFD;
	initipc_reg->dif_adj[21] = 0xFE;
	initipc_reg->dif_adj[22] = 0xFE;

	for (i = 23; i < 256; i++)
		initipc_reg->dif_adj[i] = 0xFF;

	/* sad_adj */
	initipc_reg->sad_adj[0] = 0x00;
	initipc_reg->sad_adj[1] = 0x24;
	initipc_reg->sad_adj[2] = 0x43;
	initipc_reg->sad_adj[3] = 0x5E;
	initipc_reg->sad_adj[4] = 0x76;
	initipc_reg->sad_adj[5] = 0x8C;
	initipc_reg->sad_adj[6] = 0x9E;
	initipc_reg->sad_adj[7] = 0xAF;
	initipc_reg->sad_adj[8] = 0xBD;
	initipc_reg->sad_adj[9] = 0xC9;
	initipc_reg->sad_adj[10] = 0xD4;
	initipc_reg->sad_adj[11] = 0xDD;
	initipc_reg->sad_adj[12] = 0xE4;
	initipc_reg->sad_adj[13] = 0xEA;
	initipc_reg->sad_adj[14] = 0xEF;
	initipc_reg->sad_adj[15] = 0xF3;
	initipc_reg->sad_adj[16] = 0xF6;
	initipc_reg->sad_adj[17] = 0xF9;
	initipc_reg->sad_adj[18] = 0xFB;
	initipc_reg->sad_adj[19] = 0xFC;
	initipc_reg->sad_adj[20] = 0xFD;
	initipc_reg->sad_adj[21] = 0xFE;
	initipc_reg->sad_adj[22] = 0xFE;

	for (i = 23; i < 256; i++)
		initipc_reg->sad_adj[i] = 0xFF;

	/* bld_gain */
	for (i = 0; i < 256; i++)
		initipc_reg->bld_gain[i] = 0x80;

	/* dif_gain */
	for (i = 0; i < 256; i++)
		initipc_reg->dif_gain[i] = 0x80;

	/* mdet */
	for (i = 0; i < 256; i++)
		initipc_reg->mdet[i] = i;

	DPRINT("done\n");

	return 0;
}
