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

#ifndef	__FDP_DEPEND_H__
#define	__FDP_DEPEND_H__
/*	FDP	logical	driver	status	*/
typedef	struct	{
	char	name[30];
	unsigned long	reg_value;
}	T_FDPD_REG_STATUS;

typedef	struct	{
	unsigned char	status;
	unsigned long	delay;
	unsigned long	vcycle;
	unsigned short	vintcnt;
	unsigned char	seq_lock;
	unsigned char	in_enable;
	unsigned long	in_picid;
	unsigned char	in_left;
	unsigned char	out_enable;
	unsigned long	out_picid;
	unsigned char	out_left;
	unsigned char	out_req;
	unsigned int	sinfo[18];
	T_FDPD_REG_STATUS fdp_reg_status[60];
}	T_FDPD_DRV_OBJ;

/* for drv_FDP_Start sequence parameter */
typedef	struct {
	unsigned char	seq_mode;
	unsigned char	scale_mode;
	unsigned char	filter_mode;
	unsigned char	telecine_mode;
	unsigned long	in_width;
	unsigned long	in_height;
	unsigned long	imgtop;
	unsigned long	imgleft;
	unsigned long	imgwidth;
	unsigned long	imgheight;
	unsigned long	out_width;
	unsigned long	out_height;
	}	T_FDPD_SEQ;

typedef	struct {
	unsigned short	width;
	unsigned short	height;
	unsigned char	chroma_format;
	unsigned char	progressive_sequence;
	unsigned char	progressive_frame;
	unsigned char	picture_structure;
	unsigned char	repeat_first_field;
	unsigned char	top_field_first;
}	T_FDPD_PICPAR;

struct	fdp_independ	{
	T_FDPD_DRV_OBJ	fdp_state;
	T_FDPD_DRV_OBJ	fdp_state_pre;
	T_FDPD_SEQ	fdp_seq_par;
	T_FDPD_PICPAR	fdp_pic_par;
	T_FDPD_PICPAR   fdp_pic_par_pre;
	unsigned int	seq_count;
	unsigned char	seq_mode;
	int	decode_val;
	int	decode_count;
	int	count_max;
	unsigned long	stlmsk_adr;
	unsigned char	smw;
	unsigned char	wr;
	unsigned char	chact;
};
#endif
