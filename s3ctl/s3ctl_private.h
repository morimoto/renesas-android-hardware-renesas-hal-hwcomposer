/*************************************************************************/ /*
 S3CTL

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
#ifndef __S3CTL_PRIVATE_H__
#define __S3CTL_PRIVATE_H__

#define DEVFILE			"/dev/s3ctl"
#define DRVNAME			"s3ctl"

#define S3_IOC_MAGIC 's'
#define S3_IOC_SET_PARAM	_IOWR(S3_IOC_MAGIC, 0, struct S3_PARAM)
#define S3_IOC_CLEAR_PARAM	_IOWR(S3_IOC_MAGIC, 1, struct S3_PARAM)
#define S3_IOC_LOCK		_IOWR(S3_IOC_MAGIC, 2, struct S3_PARAM)
#define S3_IOC_UNLOCK		_IOWR(S3_IOC_MAGIC, 3, struct S3_PARAM)
#define S3_IOC_GET_PARAM	_IOWR(S3_IOC_MAGIC, 4, struct S3_PARAM)

#define S3_MAX			(8)

#define S3_ALLOC		(1)
#define S3_FREE			(0)

#define S3_ADDR			(0xE6784000)
#define S3_SIZE			(0xD00)
#define S3_OFFSET		(0xB00)

#define S3_TL_A_EN		(0x80000000)
#define S3_TL_A_ADDR_MASK	(0x007FFFFF)
#define S3_TL_B_TILE_TYPE	(0x01000000)
#define S3_TL_B_SIZE_MASK	(0x000F0000)
#define S3_TL_B_SIZE_SHIFT	(16)
#define S3_TL_B_STRIDE_MASK	(0x00000FF0)

#define S3_XYMODE_VAL		(0x00000F00)
#define S3_XYMODE_VAL_NEW	(0x00000010)

#define S3_PRR_ADDR		(0xFF000044)
#define S3_PRR_SIZE		(4)

#define S3_PRR_ESMASK		(0x000000F0)
#define S3_PRR_ES1		(0x00000000)
#define S3_PRR_ES2		(0x00000010)
#define S3_PRR_ES3		(0x00000020)
#define S3_PRR_PRODUCTMASK	(0x00007F00)
#define S3_PRR_H2		(0x00004500)
#define S3_PRR_M2W		(0x00004700)
#define S3_PRR_M2N		(0x00004B00)
#define S3_PRR_E2		(0x00004C00)

#define S3_ST_OPEN		(0)
#define S3_ST_SET		(1)
#define S3_ST_LOCK		(2)

struct S3_XYTL {
	unsigned long	a[S3_MAX];
	unsigned long	b[S3_MAX];
	unsigned long	mode;
};

struct S3_PARAM {
	unsigned long	phy_addr;
	unsigned long	stride;
	unsigned long	area;
};

struct S3_PRIVATE {
	struct S3_PARAM	param;
	int		id;
	int		st;
};

static inline void S3_WRITE(unsigned long *reg, u32 value);
static inline u32 S3_READ(unsigned long *reg);
static int map_register(void);
static void unmap_register(void);
static void set_xymodeconf(u32 value);
static void enable_s3(int id, struct S3_PARAM *p);
static void disable_s3(int id);
static int alloc_id(int *id);
static void free_id(int *id);
static int check_param(struct S3_PARAM *p);


#endif	/* __S3CTL_PRIVATE_H__ */
