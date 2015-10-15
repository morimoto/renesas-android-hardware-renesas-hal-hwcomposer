/*************************************************************************/ /*
 imgpctrl_ctrl_vsp.h
   imgpctrl ctrl_vsp header file

 Copyright (C) 2013 Renesas Electronics Corporation

 License        GPLv2

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

#ifndef __IMGPCTRL_CTRL_VSP_H__
#define __IMGPCTRL_CTRL_VSP_H__

#define IMGPCTRL_FULL_TRANSPARENT	0
#define IMGPCTRL_FULL_OPAQUE		0xFF

#define IMGPCTRL_VSP_YUV		1
#define IMGPCTRL_VSP_RGB		2

#define IMGPCTRL_INPUT			1
#define IMGPCTRL_OUTPUT			2

#define IMGPCTRL_FIXALPHA		0xFF000000U

#define IMGPCTRL_MANTPRE_MIN		1
#define	IMGPCTRL_MANTPRE_MID		2
#define	IMGPCTRL_MANTPRE_MAX		4
#define IMGPCTRL_RF_ALFAMAX		0x0000FFFF
#define IMGPCTRL_RF_ALFAMIN		0x00000100
#define IMGPCTRL_RF_ALFACEN		0x00001000
#define IMGPCTRL_RF_MNTMAX		0x0000000F

/* imgpctrl_ctrl_vsp(internal function)  */
/* imgpctrl_ctrl_vsp.c imgpctrl_ctrl_vsp_para_gen.c */
extern long imgpctrl_vsp_para_gen(struct t_exe_info *exe_info,
	struct t_vspm_vsp *vspm_p,
	struct screen_grap_handle *bl_hdl);

#endif /* __IMGPCTRL_CTRL_VSP_H__ */
