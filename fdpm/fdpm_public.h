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

#ifndef __FDPM_PUBLIC_H__
#define __FDPM_PUBLIC_H__

typedef void (*PFN_FDPM_COMPLETE_CALLBACK)(unsigned long uwUserData);

#define R_FDPM_OK (0)
#define R_FDPM_NG (-1)

/* drv_FDPM_Init */
#define E_FDP_PARA_INTLVL        (-100)
#define E_FDP_DEF_INH            (-101)

/* drv_FDPM_Open */
#define E_FDP_PARA_CB3           (-200)
#define E_FDP_PARA_CB4           (-201)
#define E_FDP_PARA_OPENPAR       (-202)
#define E_FDP_PARA_REFBUFMODE    (-203)
#define E_FDP_PARA_BUFREF0       (-204)
#define E_FDP_PARA_BUFREF1       (-205)
#define E_FDP_PARA_BUFREF2       (-206)
#define E_FDP_PARA_BUFREFPRG     (-207)
#define E_FDP_PARA_VMODE         (-208)
#define E_FDP_PARA_CLKMODE       (-209)
#define E_FDP_PARA_OCMODE        (-210)
#define E_FDP_PARA_INSIZE        (-211)

#define E_FDP_PARA_CB1           (-250)
#define E_FDP_PARA_CB2           (-251)
#define E_FDP_PARA_VCNT          (-252)
#define E_FDP_PARA_REFBUF        (-253)
#define E_FDP_PARA_BUFADDR       (-254)
#define E_FDP_PARA_BUFADDRC      (-255)
#define E_FDP_PARA_BUFADDRC1     (-256)
#define E_FDP_PARA_BUFSTRIDE     (-257)
#define E_FDP_PARA_BUFHEIGHT     (-258)
#define E_FDP_PARA_BUFHEIGHTC    (-259)

/* drv_FDPM_Start */
#define E_FDP_PARA_STARTPAR      (-300)
#define E_FDP_PARA_FDPGO         (-301)
#define E_FDP_PARA_FPROCPAR      (-302)
#define E_FDP_PARA_SEQPAR        (-303)
#define E_FDP_PARA_IMGSETPAR     (-304)
#define E_FDP_PARA_INPIC         (-305)
#define E_FDP_PARA_OUTBUF        (-306)
#define E_FDP_PARA_SEQMODE       (-307)
#define E_FDP_PARA_TELECINEMODE  (-308)
#define E_FDP_PARA_INWIDTH       (-309)
#define E_FDP_PARA_INHEIGHT      (-310)
#define E_FDP_PARA_PICPAR        (-311)
#define E_FDP_PARA_INBUF1        (-312)
#define E_FDP_PARA_INBUF2        (-313)
#define E_FDP_PARA_PICWIDTH      (-314)
#define E_FDP_PARA_PICHEIGHT     (-315)
#define E_FDP_PARA_CHROMA        (-316)
#define E_FDP_PARA_PROGSEQ       (-317)
#define E_FDP_PARA_PICSTRUCT     (-318)
#define E_FDP_PARA_REPEATTOP     (-319)
#define E_FDP_PARA_BUFREFWR      (-320)
#define E_FDP_PARA_BUFREFRD0     (-321)
#define E_FDP_PARA_BUFREFRD1     (-322)
#define E_FDP_PARA_BUFREFRD2     (-323)
#define E_FDP_PARA_BUFIIRWR      (-324)
#define E_FDP_PARA_BUFIIRRD      (-325)
#define E_FDP_PARA_SEQOVERLAP    (-326)
#define E_FDP_PARA_FIELD_PARITY  (-327)
#define E_FDP_PARA_STATUS        (-328)
#define E_FDP_PARA_LASTSTART     (-329)
#define E_FDP_PARA_CF            (-330)
#define E_FDP_PARA_FDECODE       (-331)
#define E_FDP_PARA_OUTFORMAT     (-332)

/* drv_FDPM_Cancel */
#define E_FDP_CANCEL_NOID        (-400)
#define E_FDP_CANCEL_ID_PROCESSING (-401)

/* timer */
#define E_FDP_TIMER_CB           (-80)
#define E_FDP_TIMER_TO           (-81)

/* #define TO_VCNT    (10*10000) */
#define TO_VCNT    (1*10000)

#endif
