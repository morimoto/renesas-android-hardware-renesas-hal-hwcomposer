/*************************************************************************/ /*
 VSPM

 Copyright (C) 2013-2014 Renesas Electronics Corporation

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

#ifndef _TDDMAC_DRV_H_
#define _TDDMAC_DRV_H_

/* define maxium channel number */
#define TDDMAC_CH_MAX		(8)

/* define argument parameter */
/* T_TDDMAC_MODE.renewal */
#define TDDMAC_RNEW_NORMAL	(0x00000000)

/* T_TDDMAC_MODE.resource */
#define TDDMAC_RES_AUTO		(0x00000000)

/* T_TDDMAC_REQUEST.swap */
#define TDDMAC_SWAP_OFF		(0x00000000)
#define TDDMAC_SWAP_OLS		(0x00000040)
#define TDDMAC_SWAP_OWS		(0x00000020)
#define TDDMAC_SWAP_OBS		(0x00000010)
#define TDDMAC_SWAP_ILS		(0x00000004)
#define TDDMAC_SWAP_IWS		(0x00000002)
#define TDDMAC_SWAP_IBS		(0x00000001)

/* define driver's error code */
#define E_TDDMAC_DEF_INH			(-100)
#define E_TDDMAC_DEF_REG			(-101)
#define E_TDDMAC_NO_MEM				(-102)
#define E_TDDMAC_NO_INIT			(-103)
#define E_TDDMAC_NO_CH				(-104)
#define E_TDDMAC_INVALID_STATE		(-105)

#define E_TDDMAC_PARA_CHANNEL		(-107)
#define E_TDDMAC_PARA_PARAMS		(-109)
#define E_TDDMAC_PARA_RENEWAL		(-111)
#define E_TDDMAC_PARA_RESOURCE		(-112)
#define E_TDDMAC_PARA_SRC_ADR		(-120)
#define E_TDDMAC_PARA_DST_ADR		(-121)
#define E_TDDMAC_PARA_SRC_WIDTH		(-131)		/* not used */
#define E_TDDMAC_PARA_SRC_X_OFFSET	(-132)		/* not used */
#define E_TDDMAC_PARA_SRC_Y_OFFSET	(-133)
#define E_TDDMAC_PARA_SRC_FORMAT	(-134)
#define E_TDDMAC_PARA_RATIO			(-135)
#define E_TDDMAC_PARA_ALPHA_ENA		(-137)
#define E_TDDMAC_PARA_DST_FORMAT	(-139)
#define E_TDDMAC_PARA_DST_WIDTH		(-140)
#define E_TDDMAC_PARA_DST_HEIGHT	(-141)
#define E_TDDMAC_PARA_SRC_STRIDE	(-143)
#define E_TDDMAC_PARA_DST_STRIDE	(-144)
#define E_TDDMAC_PARA_DST_X_OFFSET	(-146)
#define E_TDDMAC_PARA_DST_Y_OFFSET	(-147)
#define E_TDDMAC_PARA_MIRROR		(-148)
#define E_TDDMAC_PARA_ROTATION		(-149)

typedef struct {
	unsigned char intlvl_1;					/* not used */
	void (*pm_req)(unsigned long, unsigned long);	/* not used */
} T_TDDMAC_INIT;

typedef struct {
	unsigned char device;		/* not used */
} T_TDDMAC_EXTEND;

typedef struct {
	unsigned char renewal;		/* Repeat or Reload. */
	unsigned char resource;		/* Resource of transfer. */
		/* For renewal is selected extended Device. */
	T_TDDMAC_EXTEND	*p_extend;
} T_TDDMAC_MODE;

typedef struct {
	long ercd;					/* Error code. */
	long sub_ercd;				/* Sub-error code. */
	unsigned long channel;		/* Channel */
	unsigned long count;		/* Transfer counter */
	void *userdata;				/* callback data */
} T_TDDMAC_CB;

typedef struct {
	void *src_adr;		/* Source address. */
	unsigned short src_stride;
	unsigned short src_x_offset;
	unsigned short src_y_offset;
	unsigned char src_format;
	unsigned char ratio;
	void *dst_adr;		/* Destination address. */
	unsigned char alpha_ena;
	unsigned char alpha;
	unsigned char dst_format;
	unsigned short dst_stride;
	unsigned short dst_x_offset;
	unsigned short dst_y_offset;
	unsigned short dst_width;
	unsigned short dst_height;
	void *cb_finished;	/* The function of callback for finish. */
	void *userdata;		/* callback data */
	unsigned long swap;
	unsigned char mirror;
	unsigned char rotation;
} T_TDDMAC_REQUEST;


/* enum argument parameter */
/* T_TDDMAC_REQUEST.src_format/dst_format */
enum {
	TDDMAC_FORMAT_Y = 0,
	TDDMAC_FORMAT_C420,
	TDDMAC_FORMAT_C422,
	TDDMAC_FORMAT_ARGB8888,
	TDDMAC_FORMAT_RGBA8888,
	TDDMAC_FORMAT_RGB888,
	TDDMAC_FORMAT_RGB565,
	TDDMAC_FORMAT_RGB332,
	TDDMAC_FORMAT_pRGB14_666,
	TDDMAC_FORMAT_pRGB4_444,
	TDDMAC_FORMAT_RGB666,
	TDDMAC_FORMAT_BGR666,
	TDDMAC_FORMAT_BGR888,
	TDDMAC_FORMAT_ABGR8888,
	TDDMAC_FORMAT_RGB0565,
	TDDMAC_FORMAT_MAX
};

/* T_TDDMAC_REQUEST.raito */
enum {
	TDDMAC_RATIO_1_1 = 0,
	TDDMAC_X_RATIO_2_1,
	TDDMAC_Y_RATIO_2_1,
	TDDMAC_XY_RATIO_2_1,
	TDDMAC_RATIO_MAX
};

/* T_TDDMAC_REQUEST.alpha_ena */
enum {
	TDDMAC_SRCALPHA_DISABLE = 0,
	TDDMAC_SRCALPHA_ENABLE,
	TDDMAC_SRCALPHA_MAX
};

/* T_TDDMAC_REQUEST.mirror */
enum {
	TDDMAC_MRR_OFF = 0,
	TDDMAC_MRR_H,
	TDDMAC_MRR_V,
	TDDMAC_MRR_HV,
	TDDMAC_MRR_MAX
};

/* T_TDDMAC_REQUEST.rotation */
enum {
	TDDMAC_ROT_OFF = 0,
	TDDMAC_ROT_90,
	TDDMAC_ROT_270,
	TDDMAC_ROT_180,
	TDDMAC_ROT_MAX
};

#endif
