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

#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/completion.h>

#include "vspm_public.h"
#include "vspm_private.h"
#include "vspm_main.h"
#include "vspm_log.h"

extern struct vspm_drvdata *p_vspm_drvdata;

/*
 * set_vsp_src_par - Copy the input image settings(src_par)
 * @dst_param:
 * @src_param:
 * Description: ptVsp->src1_par
 *              ptVsp->src2_par
 *              ptVsp->src3_par
 *              ptVsp->src4_par
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_src_par(
	struct vspm_entry_vsp_in *dst_param, T_VSP_IN *src_param)
{
	void *clut = 0;
	int ercd;

	DPRINT("called\n");

	/* copy T_VSP_IN parameter */
	if (copy_from_user(
			&dst_param->in,
			(void __user *)src_param,
			sizeof(dst_param->in))) {
		APRINT("failed to copy of T_VSP_IN\n");
		ercd = -EFAULT;
		goto exit;
	}

	if (dst_param->in.osd_lut) {
		/* copy T_VSP_OSDLUT parameter */
		if (copy_from_user(
				&dst_param->osdlut,
				(void __user *)dst_param->in.osd_lut,
				sizeof(dst_param->osdlut))) {
			APRINT("failed to copy of T_VSP_OSDLUT\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* make lut buffer */
		if ((dst_param->osdlut.size) && (dst_param->osdlut.clut)) {
			/* allocate */
			clut = kzalloc(
				sizeof(*dst_param->osdlut.clut) *
					dst_param->osdlut.size,
				GFP_KERNEL);
			if (!clut) {
				APRINT("failed to kzalloc of OSD lut buffer\n");
				ercd = -ENOMEM;
				goto exit;
			}

			/* copy */
			if (copy_from_user(
					clut,
					(void __user *)dst_param->osdlut.clut,
					sizeof(*dst_param->osdlut.clut) *
						dst_param->osdlut.size)) {
				APRINT("failed to copy of OSD lut buffer\n");
				ercd = -EFAULT;
				goto exit;
			}
		}

		/* set lut buffer */
		dst_param->osdlut.clut = clut;

		dst_param->in.osd_lut = &dst_param->osdlut;
	}

	/* copy T_VSP_ALPHA parameter */
	if (dst_param->in.alpha_blend) {
		if (copy_from_user(&dst_param->alpha,
				(void __user *)dst_param->in.alpha_blend,
				sizeof(dst_param->alpha))) {
			APRINT("failed to copy of T_VSP_ALPHA\n");
			ercd = -EFAULT;
			goto exit;
		}
		dst_param->in.alpha_blend = &dst_param->alpha;
	}

	/* copy T_VSP_CLRCNV parameter */
	if (dst_param->in.clrcnv) {
		if (copy_from_user(&dst_param->clrcnv,
				(void __user *)dst_param->in.clrcnv,
				sizeof(dst_param->clrcnv))) {
			APRINT("failed to copy of T_VSP_CLRCNV\n");
			ercd = -EFAULT;
			goto exit;
		}
		dst_param->in.clrcnv = &dst_param->clrcnv;
	}

	DPRINT("done\n");
	return 0;
exit:
	/* free */
	if (clut)
		kfree(clut);

	dst_param->osdlut.clut = 0;

	return ercd;
}

/*
 * set_vsp_bru_par - Copy the BRU settings
 * @dst_param:
 * @src_param:
 * Description: ptVsp->ctrl_par->bru
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_bru_par(
	struct vspm_entry_vsp_bru *dst_param, T_VSP_BRU *src_param)
{
	T_VSP_BLEND_CONTROL * *(src_blend[4]);
	T_VSP_BLEND_CONTROL * (dst_blend[4]);
	int i;

	DPRINT("called\n");

	/* copy T_VSP_BRU parameter */
	if (copy_from_user(
			&dst_param->bru,
			(void __user *)src_param,
			sizeof(dst_param->bru))) {
		APRINT("failed to copy of T_VSP_BRU\n");
		return -EFAULT;
	}

	/* copy T_VSP_BLEND_VIRTUAL parameter */
	if (dst_param->bru.blend_virtual) {
		if (copy_from_user(
				&dst_param->blend_virtual,
				(void __user *)dst_param->bru.blend_virtual,
				sizeof(dst_param->blend_virtual))) {
			APRINT("failed to copy of T_VSP_BLEND_VIRTUAL\n");
			return -EFAULT;
		}
		dst_param->bru.blend_virtual = &dst_param->blend_virtual;
	}

	src_blend[0] = &dst_param->bru.blend_control_a;
	src_blend[1] = &dst_param->bru.blend_control_b;
	src_blend[2] = &dst_param->bru.blend_control_c;
	src_blend[3] = &dst_param->bru.blend_control_d;

	dst_blend[0] = &dst_param->blend_control_a;
	dst_blend[1] = &dst_param->blend_control_b;
	dst_blend[2] = &dst_param->blend_control_c;
	dst_blend[3] = &dst_param->blend_control_d;

	/* copy T_VSP_BLEND_CONTROL parameter */
	for (i = 0; i < 4; i++) {
		if (*src_blend[i]) {
			if (copy_from_user(
					dst_blend[i],
					(void __user *)*src_blend[i],
					sizeof(*dst_blend[i]))) {
				APRINT(
					"failed to copy of T_VSP_BLEND_CONTROL\n");
				return -EFAULT;
			}
			*src_blend[i] = dst_blend[i];
		}
	}

	/* copy T_VSP_BLEND_ROP parameter */
	if (dst_param->bru.blend_rop) {
		if (copy_from_user(
				&dst_param->blend_rop,
				(void __user *)dst_param->bru.blend_rop,
				sizeof(dst_param->blend_rop))) {
			APRINT("failed to copy of T_VSP_BLEND_ROP\n");
			return -EFAULT;
		}
		dst_param->bru.blend_rop = &dst_param->blend_rop;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * set_vsp_lut_par - Copy the LUT settings
 * @dst_param:
 * @src_param:
 * Description: ptVsp->ctrl_par->lut
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_lut_par(T_VSP_LUT *dst_param, T_VSP_LUT *src_param)
{
	void *lut = 0;
	int ercd;

	DPRINT("called\n");

	/* copy T_VSP_LUT parameter */
	if (copy_from_user(
			dst_param,
			(void __user *)src_param,
			sizeof(*dst_param))) {
		APRINT("failed to copy of T_VSP_LUT\n");
		ercd = -EFAULT;
		goto exit;
	}

	/* make lut buffer */
	if ((dst_param->size) && (dst_param->lut)) {
		/* allocate */
		lut = kzalloc(
			sizeof(*dst_param->lut) * dst_param->size, GFP_KERNEL);
		if (!lut) {
			APRINT("failed to kzalloc of LUT buffer\n");
			ercd = -ENOMEM;
			goto exit;
		}

		/* copy */
		if (copy_from_user(
				lut,
				(void __user *)dst_param->lut,
				sizeof(*dst_param->lut) * dst_param->size)) {
			APRINT("failed to copy of LUT buffer\n");
			ercd = -EFAULT;
			goto exit;
		}
	}

	/* set lut buffer */
	dst_param->lut = lut;

	DPRINT("done\n");
	return 0;
exit:
	/* free */
	if (lut)
		kfree(lut);

	dst_param->lut = 0;

	return ercd;
}

/*
 * set_vsp_clu_par - Copy the CLU settings
 * @dst_param:
 * @src_param:
 * Description: ptVsp->ctrl_par->clu
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_clu_par(T_VSP_CLU *dst_param, T_VSP_CLU *src_param)
{
	void *clu_data = 0;
	void *clu_addr = 0;
	int ercd;

	DPRINT("called\n");

	/* ptVsp->ctrl_par->clu */
	if (copy_from_user(
			dst_param,
			(void __user *)src_param,
			sizeof(*dst_param))) {
		APRINT("failed to copy of T_VSP_CLU\n");
		ercd = -EFAULT;
		goto exit;
	}

	/* make clut data */
	if ((dst_param->size) && (dst_param->clu_data)) {
		/* allocate */
		clu_data = kzalloc(
			sizeof(*dst_param->clu_data) *
			dst_param->size, GFP_KERNEL);
		if (!clu_data) {
			APRINT("failed to kzalloc of CLU data\n");
			ercd = -ENOMEM;
			goto exit;
		}

		/* copy */
		if (copy_from_user(
			clu_data,
			(void __user *)dst_param->clu_data,
			sizeof(*dst_param->clu_data) * dst_param->size)) {
				APRINT("failed to copy of CLU data\n");
				ercd = -EFAULT;
				goto exit;
		}
	}

	/* set clut data */
	dst_param->clu_data = clu_data;

	/* make clut address */
	if ((dst_param->size) && (dst_param->clu_addr)) {
		/* allocate */
		clu_addr = kzalloc(
			sizeof(*dst_param->clu_addr) *
			dst_param->size, GFP_KERNEL);
		if (!clu_addr) {
			APRINT("failed to kzalloc of CLU address\n");
			ercd = -ENOMEM;
			goto exit;
		}

		/* copy */
		if (copy_from_user(
			clu_addr,
			(void __user *)dst_param->clu_addr,
			sizeof(*dst_param->clu_addr) * dst_param->size)) {
				APRINT("failed to copy of CLU address\n");
				ercd = -EFAULT;
				goto exit;
		}
	}

	/* set clut address */
	dst_param->clu_addr = clu_addr;

	DPRINT("done\n");
	return 0;
exit:
	/* free */
	if (clu_data)
		kfree(clu_data);

	dst_param->clu_data = 0;

	if (clu_addr)
		kfree(clu_addr);

	dst_param->clu_addr = 0;

	return ercd;
}

/*
 * set_vsp_hgo_par - Copy the HGO settings
 * @dst_param:
 * @src_param:
 * Description: ptVsp->ctrl_par->hgo
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_hgo_par(
	struct vspm_entry_vsp_hgo *dst_param, T_VSP_HGO *src_param)
{
	DPRINT("called\n");

	/* copy from user area */
	if (copy_from_user(
			&dst_param->hgo,
			(void __user *)src_param,
			sizeof(dst_param->hgo))) {
		APRINT("failed to copy of T_VSP_HGO\n");
		return -EFAULT;
	}

	if (dst_param->hgo.addr) {
		/* check access_ok */
		if (!access_ok(
				VERIFY_WRITE,
				(void __user *)dst_param->hgo.addr,
				768)) {
			APRINT("failed to access_ok of T_VSP_HGO\n");
			return -EFAULT;
		}

		/* stack user area address */
		dst_param->user_addr = dst_param->hgo.addr;

		/* memory allocate */
		dst_param->hgo.addr = kzalloc(768, GFP_KERNEL);
		if (dst_param->hgo.addr == NULL) {
			APRINT("failed to kzalloc of T_VSP_HGO\n");
			return -ENOMEM;
		}
	}

	DPRINT("done\n");
	return 0;
}


/*
 * set_vsp_hgt_par - Copy the HGT settings
 * @dst_param:
 * @src_param:
 * Description: ptVsp->ctrl_par->hgt
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_hgt_par(
	struct vspm_entry_vsp_hgt *dst_param, T_VSP_HGT *src_param)
{
	DPRINT("called\n");

	/* copy from user area */
	if (copy_from_user(
			&dst_param->hgt,
			(void __user *)src_param,
			sizeof(dst_param->hgt))) {
		APRINT("failed to copy of T_VSP_HGT\n");
		return -EFAULT;
	}

	if (dst_param->hgt.addr) {
		/* check access_ok */
		if (!access_ok(
				VERIFY_WRITE,
				(void __user *)dst_param->hgt.addr,
				768)) {
			APRINT("failed to access_ok of T_VSP_HGT\n");
			return -EFAULT;
		}

		/* stack user area address */
		dst_param->user_addr = dst_param->hgt.addr;

		/* memory allocate */
		dst_param->hgt.addr = kzalloc(768, GFP_KERNEL);
		if (dst_param->hgt.addr == NULL) {
			APRINT("failed to kzalloc of T_VSP_HGT\n");
			return -ENOMEM;
		}
	}

	DPRINT("done\n");
	return 0;
}

/*
 * set_vsp_ctrl_par - Copy the control table
 * @dst_param:
 * @src_param:
 * Description: ptVsp->ctrl_par
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_ctrl_par(
	struct vspm_entry_vsp_ctrl *dst_param, T_VSP_CTRL *src_param)
{
	int ercd;

	DPRINT("called\n");

	/* copy T_VSP_CTRL parameter */
	if (copy_from_user(
			&dst_param->ctrl,
			(void __user *)src_param,
			sizeof(dst_param->ctrl))) {
		APRINT("failed to copy of T_VSP_CTRL\n");
		return -EFAULT;
	}

	/* copy T_VSP_BRU parameter */
	if (dst_param->ctrl.bru) {
		ercd = set_vsp_bru_par(&dst_param->bru, dst_param->ctrl.bru);
		if (ercd)
			return ercd;
		dst_param->ctrl.bru = &dst_param->bru.bru;
	}

	/* copy T_VSP_SRU parameter */
	if (dst_param->ctrl.sru) {
		if (copy_from_user(
				&dst_param->sru,
				(void __user *)dst_param->ctrl.sru,
				sizeof(dst_param->sru))) {
			APRINT("failed to copy of T_VSP_SRU\n");
			return -EFAULT;
		}
		dst_param->ctrl.sru = &dst_param->sru;
	}

	/* copy T_VSP_UDS parameter */
	if (dst_param->ctrl.uds) {
		if (copy_from_user(
				&dst_param->uds,
				(void __user *)dst_param->ctrl.uds,
				sizeof(dst_param->uds))) {
			APRINT("failed to copy of T_VSP_UDS\n");
			return -EFAULT;
		}
		dst_param->ctrl.uds = &dst_param->uds;
	}

	/* copy T_VSP_UDS1 parameter */
	if (dst_param->ctrl.uds1) {
		if (copy_from_user(
				&dst_param->uds1,
				(void __user *)dst_param->ctrl.uds1,
				sizeof(dst_param->uds1))) {
			APRINT("failed to copy of T_VSP_UDS1\n");
			return -EFAULT;
		}
		dst_param->ctrl.uds1 = &dst_param->uds1;
	}

	/* copy T_VSP_UDS2 parameter */
	if (dst_param->ctrl.uds2) {
		if (copy_from_user(
				&dst_param->uds2,
				(void __user *)dst_param->ctrl.uds2,
				sizeof(dst_param->uds2))) {
			APRINT("failed to copy of T_VSP_UDS2\n");
			return -EFAULT;
		}
		dst_param->ctrl.uds2 = &dst_param->uds2;
	}

	/* copy T_VSP_LUT parameter */
	if (dst_param->ctrl.lut) {
		ercd = set_vsp_lut_par(&dst_param->lut, dst_param->ctrl.lut);
		if (ercd)
			return ercd;
		dst_param->ctrl.lut = &dst_param->lut;
	}

	/* copy T_VSP_CLU parameter */
	if (dst_param->ctrl.clu) {
		ercd = set_vsp_clu_par(&dst_param->clu, dst_param->ctrl.clu);
		if (ercd)
			return ercd;
		dst_param->ctrl.clu = &dst_param->clu;
	}

	/* copy T_VSP_HST parameter */
	if (dst_param->ctrl.hst) {
		if (copy_from_user(
				&dst_param->hst,
				(void __user *)dst_param->ctrl.hst,
				sizeof(dst_param->hst))) {
			APRINT("failed to copy of T_VSP_HST\n");
			return -EFAULT;
		}
		dst_param->ctrl.hst = &dst_param->hst;
	}

	/* copy T_VSP_HSI parameter */
	if (dst_param->ctrl.hsi) {
		if (copy_from_user(
				&dst_param->hsi,
				(void __user *)dst_param->ctrl.hsi,
				sizeof(dst_param->hsi))) {
			APRINT("failed to copy of T_VSP_HSI\n");
			return -EFAULT;
		}
		dst_param->ctrl.hsi = &dst_param->hsi;
	}

	/* copy T_VSP_HGO parameter */
	if (dst_param->ctrl.hgo) {
		ercd = set_vsp_hgo_par(&dst_param->hgo, dst_param->ctrl.hgo);
		if (ercd)
			return ercd;
		dst_param->ctrl.hgo = &dst_param->hgo.hgo;
	}

	/* copy T_VSP_HGT parameter */
	if (dst_param->ctrl.hgt) {
		ercd = set_vsp_hgt_par(&dst_param->hgt, dst_param->ctrl.hgt);
		if (ercd)
			return ercd;
		dst_param->ctrl.hgt = &dst_param->hgt.hgt;
	}

	DPRINT("done\n");
	return 0;
}


/*
 * free_vsp_par - Free VSP parameter area
 * @ip_param:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int free_vsp_par(struct vspm_entry_ip_par *ip_param)
{
	struct vspm_entry_vsp *entry = &ip_param->ip_param.vsp;
	struct vspm_entry_vsp_in *in = &entry->in[0];
	struct vspm_entry_vsp_ctrl *ctrl = &entry->ctrl;
	int i;

	DPRINT("called\n");

	for (i = 0; i < 4; i++) {
		if (in->osdlut.clut) {
			kfree(in->osdlut.clut);
			in->osdlut.clut = 0;
		}
		in++;
	}

	if (ctrl->lut.lut) {
		kfree(ctrl->lut.lut);
		ctrl->lut.lut = 0;
	}

	if (ctrl->clu.clu_data) {
		kfree(ctrl->clu.clu_data);
		ctrl->clu.clu_data = 0;
	}

	if (ctrl->clu.clu_addr) {
		kfree(ctrl->clu.clu_addr);
		ctrl->clu.clu_addr = 0;
	}

	if (ctrl->hgo.hgo.addr) {
		kfree(ctrl->hgo.hgo.addr);
		ctrl->hgo.hgo.addr = 0;
	}

	if (ctrl->hgt.hgt.addr) {
		kfree(ctrl->hgt.hgt.addr);
		ctrl->hgt.hgt.addr = 0;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * set_vsp_par - Copy the VSP parameter
 * @dst_param:
 * @src_param:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_vsp_par(
	struct vspm_entry_ip_par *dst_param, VSPM_VSP_PAR *src_param)
{
	struct vspm_entry_vsp *vsp = &dst_param->ip_param.vsp;
	int ercd;

	T_VSP_IN **(in_param[4]);
	int i;

	DPRINT("called\n");

	if (!src_param) {
		APRINT("VSPM_VSP_PAR is NULL\n");
		ercd = -EFAULT;
		goto exit;
	}

	/* copy VSP parameter */
	if (copy_from_user(
			&vsp->par,
			(void __user *)src_param,
			sizeof(vsp->par))) {
		APRINT("failed to copy of VSPM_VSP_PAR\n");
		ercd = -EFAULT;
		goto exit;
	}

	in_param[0] = &vsp->par.src1_par;
	in_param[1] = &vsp->par.src2_par;
	in_param[2] = &vsp->par.src3_par;
	in_param[3] = &vsp->par.src4_par;

	/* copy T_VSP_IN parameter */
	for (i = 0; i < 4; i++) {
		if (*in_param[i]) {
			ercd = set_vsp_src_par(&vsp->in[i], *in_param[i]);
			if (ercd)
				goto exit;

			*in_param[i] = &vsp->in[i].in;
		}
	}

	/* copy T_VSP_OUT parameter */
	if (vsp->par.dst_par) {
		if (copy_from_user(
				&vsp->out.out,
				(void __user *)vsp->par.dst_par,
				sizeof(vsp->out.out))) {
			APRINT("failed to copy of T_VSP_OUT\n");
			ercd = -EFAULT;
			goto exit;
		}
		vsp->par.dst_par = &vsp->out.out;
	}

	/* copy T_VSP_CTRL parameter */
	if (vsp->par.ctrl_par) {
		ercd = set_vsp_ctrl_par(&vsp->ctrl, vsp->par.ctrl_par);
		if (ercd)
			goto exit;
		vsp->par.ctrl_par = &vsp->ctrl.ctrl;
	}

	DPRINT("done\n");
	return 0;
exit:
	/* release memory */
	free_vsp_par(dst_param);
	return ercd;
}

/*
 * set_tddmac_mode_par - Copy the request mode settings
 * @dst_param:
 * @src_param:
 * Description: pt2dDmac->ptTdDmacMode
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_tddmac_mode_par(
	struct vspm_entry_tddmac_mode *dst_param, T_TDDMAC_MODE *src_param)
{
	DPRINT("called\n");

	/* copy T_TDDMAC_MODE parameter */
	if (copy_from_user(
			&dst_param->mode,
			(void __user *)src_param,
			sizeof(dst_param->mode))) {
		APRINT("failed to copy of T_TDDMAC_MODE\n");
		return -EFAULT;
	}

	/* copy T_TDDMAC_EXTEND parameter */
	if (dst_param->mode.p_extend) {
		if (copy_from_user(
				&dst_param->extend,
				(void __user *)dst_param->mode.p_extend,
				sizeof(dst_param->extend))) {
			APRINT("failed to copy of T_TDDMAC_EXTEND\n");
			return -EFAULT;
		}
		dst_param->mode.p_extend = &dst_param->extend;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * set_tddmac_par - Copy the 2DDMAC parameter
 * @ip_param:
 * @src_param:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static int set_tddmac_par(
	struct vspm_entry_ip_par *dst_param, VSPM_2DDMAC_PAR *src_param)
{
	struct vspm_entry_tddmac *tddmac = &dst_param->ip_param.tddmac;
	int ercd;

	DPRINT("called\n");

	if (!src_param) {
		APRINT("VSPM_2DDMAC_PAR is NULL\n");
		return -EFAULT;
	}

	/* copy 2DDMAC parameter */
	if (copy_from_user(
			&tddmac->par,
			(void __user *)src_param,
			sizeof(tddmac->par))) {
		APRINT("failed to copy of VSPM_2DDMAC_PAR\n");
		return -EFAULT;
	}

	/* copy T_TDDMAC_MODE parameter */
	if (tddmac->par.ptTdDmacMode) {
		ercd = set_tddmac_mode_par(
			&tddmac->mode, tddmac->par.ptTdDmacMode);
		if (ercd)
			return ercd;
		tddmac->par.ptTdDmacMode = &tddmac->mode.mode;
	}

	/* copy T_TDDMAC_REQUEST parameter */
	if (tddmac->par.ptTdDmacRequest) {
		if (copy_from_user(
				&tddmac->request,
				(void __user *)tddmac->par.ptTdDmacRequest,
				sizeof(tddmac->request))) {
			APRINT("failed to copy of T_TDDMAC_REQUEST\n");
			return -EFAULT;
		}
		tddmac->par.ptTdDmacRequest = &tddmac->request;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * set_cb_vsp_hgo_rsp - Copy the VSP(HGO) response data
 * @dst_param:
 * @src_param:
 */
static void set_cb_vsp_hgo_rsp(
	struct vspm_cb_rsp_data *dst_param,
	struct vspm_entry_vsp_hgo *src_param)
{
	void *data;

	dst_param->vsp_hgo.user_addr = NULL;
	dst_param->vsp_hgo.knel_addr = NULL;

	if ((src_param->user_addr) && (src_param->hgo.addr)) {
		/* allocate */
		data = kzalloc(768, GFP_KERNEL);
		if (!data) {
			APRINT("could not allocate HGO response data area\n");
			return;
		}

		/* copy */
		memcpy(data, src_param->hgo.addr, 768);

		/* set */
		dst_param->vsp_hgo.user_addr = src_param->user_addr;
		dst_param->vsp_hgo.knel_addr = data;
	}
}

/*
 * set_cb_vsp_hgt_rsp - Copy the VSP(HGT) response data
 * @dst_param:
 * @src_param:
 */
static void set_cb_vsp_hgt_rsp(
	struct vspm_cb_rsp_data *dst_param,
	struct vspm_entry_vsp_hgt *src_param)
{
	void *data;

	dst_param->vsp_hgt.user_addr = NULL;
	dst_param->vsp_hgt.knel_addr = NULL;

	if ((src_param->user_addr) && (src_param->hgt.addr)) {
		/* allocate */
		data = kzalloc(768, GFP_KERNEL);
		if (!data) {
			APRINT("could not allocate HGT response data area\n");
			return;
		}

		/* copy */
		memcpy(data, src_param->hgt.addr, 768);

		/* set */
		dst_param->vsp_hgt.user_addr = src_param->user_addr;
		dst_param->vsp_hgt.knel_addr = data;
	}
}


/*
 * set_cb_vsp_rsp - Copy the VSP response data
 * @rsp_data:
 * @entry_vsp:
 */
static void set_cb_vsp_rsp(
	struct vspm_cb_rsp_data *rsp_data, struct vspm_entry_vsp *entry_vsp)
{
	if (entry_vsp->par.use_module & VSP_HGO_USE) {
		/* make response for HGO histgram */
		set_cb_vsp_hgo_rsp(rsp_data, &entry_vsp->ctrl.hgo);
	}

	if (entry_vsp->par.use_module & VSP_HGT_USE) {
		/* make response for HGT histgram */
		set_cb_vsp_hgt_rsp(rsp_data, &entry_vsp->ctrl.hgt);
	}
}


/**
 * VSPM_lib_Entry - Entry of various IP operations
 * @handle:            handle (not used)
 * @puwJobId:          destination address of the job id
 * @bJobPriority:      job priority
 * @ptIpParam:         pointer to IP parameter
 * @uwUserData:        user data
 * @pfnNotifyComplete: pointer to complete callback function
 * Description: Accepts requests to various IP and executed sequentially.
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_QUE_FULL : request queue full
 * R_VSPM_PARAERR : illegal parameter
 * R_VSPM_NG : other errors
 */
long VSPM_lib_Entry(
	unsigned long handle,
	unsigned long *puwJobId,
	char bJobPriority,
	VSPM_IP_PAR *ptIpParam,
	unsigned long uwUserData,
	PFN_VSPM_COMPLETE_CALLBACK pfnNotifyComplete)
{
	struct vspm_privdata *priv = (struct vspm_privdata *)handle;
	long ercd;

	DPRINT("called\n");

	/* check parameter */
	if (priv == NULL)
		return R_VSPM_PARAERR;

	if (priv->pdrv != p_vspm_drvdata)
		return R_VSPM_PARAERR;

	/* execute entry */
	ercd = vspm_lib_entry(
		handle,
		puwJobId,
		bJobPriority,
		ptIpParam,
		uwUserData,
		pfnNotifyComplete);
	if (ercd)
		EPRINT("failed to vspm_lib_entry() %ld\n", ercd);

	DPRINT("done\n");
	return ercd;
}
EXPORT_SYMBOL(VSPM_lib_Entry);

/*
 * vspm_comlete_cb - complete callback function
 * @uwJobId:
 * @wResult:
 * @uwUserData:
 */
static void vspm_comlete_cb(
	unsigned long uwJobId, long wResult, unsigned long uwUserData)
{
	struct vspm_entry_req_data *req_data =
		(struct vspm_entry_req_data *)uwUserData;
	struct vspm_cb_rsp_data *rsp_data = 0;
	struct vspm_privdata *priv = req_data->priv;
	unsigned long lock_flag;

	/* allocate response data */
	rsp_data = kzalloc(sizeof(*rsp_data), GFP_KERNEL);
	if (!rsp_data) {
		APRINT("could not allocate response data area\n");
		goto exit;
	}

	/* make response data */
	rsp_data->rsp.end = 0;
	rsp_data->rsp.type = (int)VSPM_CBTYPE_COMPLETE;

	rsp_data->rsp.cb_data.complete.cb = req_data->entry.pfnNotifyComplete;
	rsp_data->rsp.cb_data.complete.uwJobId = uwJobId;
	rsp_data->rsp.cb_data.complete.wResult = wResult;
	rsp_data->rsp.cb_data.complete.uwUserData = req_data->entry.uwUserData;

	if (req_data->entry.ptIpParam->uhType != VSPM_TYPE_2DDMAC_AUTO) {
		/* make VSP response */
		set_cb_vsp_rsp(rsp_data, &req_data->ip_par->ip_param.vsp);
	}

	/* addition list */
	spin_lock_irqsave(&priv->lock, lock_flag);
	list_add_tail(&rsp_data->list, &priv->rsp_head.list);
	spin_unlock_irqrestore(&priv->lock, lock_flag);

	DPRINT("callback thread up()\n");

	complete(&priv->comp);

exit:
	if (req_data) {
		if (req_data->entry.ptIpParam) {
			if (req_data->ip_par) {
				if (req_data->entry.ptIpParam->uhType !=
						VSPM_TYPE_2DDMAC_AUTO)
					free_vsp_par(req_data->ip_par);

				kfree(req_data->ip_par);
			}

			kfree(req_data->entry.ptIpParam);
		}

		kfree(req_data);
	}

	DPRINT("done\n");
}

/*
 * vspm_ioctl_entry - VSPM_IOC_CMD_ENTRY command processing
 * @priv:
 * @vspm_ioc_cmd:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static long vspm_ioctl_entry(
	struct vspm_privdata *priv, struct vspm_ioc_cmd *vspm_ioc_cmd)
{
	int ercd = 0;
	struct vspm_entry_rsp rsp;

	struct vspm_entry_req_data *req_data = 0;
	VSPM_IP_PAR *vspm_ip_par = 0;
	struct vspm_entry_ip_par *ip_par = 0;

	long handle = (long)priv;

	DPRINT("called\n");

	memset(&rsp, 0, sizeof(rsp));

	/* allocate request data area */
	req_data = kzalloc(sizeof(*req_data), GFP_KERNEL);
	if (!req_data) {
		APRINT("ENTRY: could not allocate request data area\n");
		ercd = -ENOMEM;
		goto exit;
	}

	/* copy entry request parameter from user area */
	if (copy_from_user(
			&req_data->entry,
			(void __user *)vspm_ioc_cmd->req,
			sizeof(req_data->entry))) {
		APRINT("ENTRY: failed to copy the request data\n");
		ercd = -EFAULT;
		goto exit;
	}

	/* allocate entry.VSPM_IP_PAR area */
	vspm_ip_par = kzalloc(sizeof(*vspm_ip_par), GFP_KERNEL);
	if (!vspm_ip_par) {
		APRINT("ENTRY: could not allocate VSPM_IP_PAR area\n");
		ercd = -ENOMEM;
		goto exit;
	}

	/* copy IP parameter from user area */
	if (copy_from_user(
			vspm_ip_par,
			(void __user *)req_data->entry.ptIpParam,
			sizeof(*vspm_ip_par))) {
		APRINT("ENTRY: failed to copy VSPM_IP_PAR\n");
		ercd = -EFAULT;
		goto exit;
	}
	req_data->entry.ptIpParam = vspm_ip_par;

	/* allocate vspm_entry_ip_par area */
	ip_par = kzalloc(sizeof(*ip_par), GFP_KERNEL);
	if (!ip_par) {
		APRINT("ENTRY: could not allocate ip_param area\n");
		ercd = -ENOMEM;
		goto exit;
	}
	req_data->ip_par = ip_par;

	if (vspm_ip_par->uhType == VSPM_TYPE_2DDMAC_AUTO) {
		/* set 2DDMAC parameter */
		ercd = set_tddmac_par(
			ip_par, vspm_ip_par->unionIpParam.pt2dDmac);
		if (ercd) {
			EPRINT("failed to set_tddmac_par()\n");
			goto exit;
		}

		/* change kernel address */
		vspm_ip_par->unionIpParam.pt2dDmac =
			&ip_par->ip_param.tddmac.par;
	} else {
		if ((vspm_ip_par->uhType == VSPM_TYPE_VSP_AUTO) ||
			(vspm_ip_par->uhType &
				(VSPM_TYPE_VSP_VSPS |
				 VSPM_TYPE_VSP_VSPR |
				 VSPM_TYPE_VSP_VSPD0 |
				 VSPM_TYPE_VSP_VSPD1))) {
			/* set VSP parameter */
			ercd = set_vsp_par(
				ip_par, vspm_ip_par->unionIpParam.ptVsp);
			if (ercd) {
				EPRINT("failed to set_vsp_par()\n");
				goto exit;
			}

			/* change kernel address */
			vspm_ip_par->unionIpParam.ptVsp =
				&ip_par->ip_param.vsp.par;
		} else {
			APRINT("ENTRY: invalid type!! uhType=0x%04x\n",
				vspm_ip_par->uhType);
			ercd = -EINVAL;
			goto exit;
		}
	}

	req_data->priv = priv;
	req_data->pid = current->pid;

	rsp.rtcd = VSPM_lib_Entry(
		handle,
		&rsp.uwJobId,
		req_data->entry.bJobPriority,
		req_data->entry.ptIpParam,
		(unsigned long)req_data,
		vspm_comlete_cb);
	if (rsp.rtcd) {
		if (vspm_ip_par->uhType != VSPM_TYPE_2DDMAC_AUTO)
			free_vsp_par(ip_par);

		kfree(ip_par);
		kfree(vspm_ip_par);
		kfree(req_data);
	}
	DPRINT("VSPM_lib_Entry() ercd=%ld, uwJobId=0x%lx\n",
		rsp.rtcd, rsp.uwJobId);

	/* copy response data to user area */
	if (copy_to_user((void __user *)vspm_ioc_cmd->rsp, &rsp, sizeof(rsp))) {
		APRINT("ENTRY: failed to copy the response data\n");
		ercd = -EFAULT;
		goto exit;
	}

	DPRINT("done\n");
	return 0;

exit:
	if (req_data) {
		if (vspm_ip_par) {
			if (ip_par) {
				if (vspm_ip_par->uhType !=
						VSPM_TYPE_2DDMAC_AUTO)
					free_vsp_par(ip_par);

				kfree(ip_par);
			}
			kfree(vspm_ip_par);
		}
		kfree(req_data);
	}

	return ercd;
}

/**
 * VSPM_lib_Cancel - Cancel the job
 * @handle:  handle (not used)
 * @uwJobId: job id
 * Description: Cancel the job
 * Returns: On success R_VSPM_OK is returned. On error, the following error
 * is returned.
 * R_VSPM_PARAERR : illegal parameter
 * VSPM_STATUS_ACTIVE:   running
 * VSPM_STATUS_NO_ENTRY: no entry
 */
long VSPM_lib_Cancel(unsigned long handle, unsigned long uwJobId)
{
	struct vspm_privdata *priv = (struct vspm_privdata *)handle;
	long ercd;

	DPRINT("called\n");

	/* check parameter */
	if (priv == NULL)
		return R_VSPM_PARAERR;

	if (priv->pdrv != p_vspm_drvdata)
		return R_VSPM_PARAERR;

	/* execute cancel */
	ercd = vspm_lib_cancel(uwJobId);
	if (ercd) {
		NPRINT("failed to vspm_lib_cancel() %ld job_id=%08lx\n",
			ercd, uwJobId);
	}

	DPRINT("done\n");
	return ercd;
}
EXPORT_SYMBOL(VSPM_lib_Cancel);

/*
 * vspm_ioctl_cancel - VSPM_IOC_CMD_CANCEL command processing
 * @priv:
 * @vspm_ioc_cmd:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static long vspm_ioctl_cancel(
	struct vspm_privdata *priv, struct vspm_ioc_cmd *vspm_ioc_cmd)
{
	struct vspm_cancel_req req;
	struct vspm_cancel_rsp rsp;

	long handle = (long)priv;

	DPRINT("called\n");

	memset(&req, 0, sizeof(req));
	memset(&rsp, 0, sizeof(rsp));

	if (copy_from_user(
			&req, (void __user *)vspm_ioc_cmd->req, sizeof(req))) {
		APRINT("CANCEL: failed to copy the request data\n");
		return -EFAULT;
	}

	rsp.rtcd = VSPM_lib_Cancel(handle, req.uwJobId);
	DPRINT("VSPM_lib_Cancel = %ld\n", rsp.rtcd);
	if (copy_to_user(
			(void __user *)vspm_ioc_cmd->rsp, &rsp, sizeof(rsp))) {
		APRINT("CANCEL: failed to copy the response data\n");
		return -EFAULT;
	}

	DPRINT("done\n");
	return 0;
}

/*
 * vspm_ioctl_cb - VSPM_IOC_CMD_CB command processing
 * @priv:
 * @vspm_ioc_cmd:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static long vspm_ioctl_cb(
	struct vspm_privdata *priv, struct vspm_ioc_cmd *vspm_ioc_cmd)
{
	int ercd = 0;
	struct vspm_cb_rsp rsp;
	struct vspm_cb_rsp_data *rsp_data = 0;
	unsigned long lock_flag;

	DPRINT("called\n");

	/* get user process information */
	priv->cb_thread = current;
	complete(&priv->cb_start_comp);

	DPRINT("CB: callback thread down()\n");

	/* wait callback */
	if (wait_for_completion_interruptible(&priv->comp)) {
		DPRINT("CB: INTR\n");
		ercd = -EINTR;
		goto exit;
	}

	if (list_empty(&priv->rsp_head.list)) {
		/* set response data (end = -1) */
		memset(&rsp, 0, sizeof(rsp));
		rsp.end = -1;

		/* copy response data to user area */
		if (copy_to_user(
				(void __user *)vspm_ioc_cmd->rsp,
				&rsp,
				sizeof(rsp))) {
			APRINT("CB: failed to copy the response data\n");
			ercd = -EFAULT;
			goto exit;
		}

		DPRINT("CB: end = -1\n");
	} else {
		/* get response data */
		spin_lock_irqsave(&priv->lock, lock_flag);
		rsp_data = list_first_entry(
			&priv->rsp_head.list, struct vspm_cb_rsp_data, list);
		list_del(&rsp_data->list);
		spin_unlock_irqrestore(&priv->lock, lock_flag);

		/* check response type */
		if (rsp_data->rsp.type != VSPM_CBTYPE_COMPLETE) {
			APRINT("CB: unknown cb_type = %d\n",
				rsp_data->rsp.type);
			ercd = -EFAULT;
			goto exit;
		}

		/* copy response data to user area */
		if (copy_to_user(
				(void __user *)vspm_ioc_cmd->rsp,
				&rsp_data->rsp,
				sizeof(rsp_data->rsp))) {
			APRINT("CB: failed to copy the response data\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* copy HGO response to user area */
		if ((rsp_data->vsp_hgo.user_addr) &&
			(rsp_data->vsp_hgo.knel_addr)) {
			if (copy_to_user((void __user *)
					rsp_data->vsp_hgo.user_addr,
					rsp_data->vsp_hgo.knel_addr,
					768)) {
				APRINT("CB: failed to copy the HGO data\n");
				ercd = -EFAULT;
				goto exit;
			}
		}

		/* copy HGT response to user area */
		if ((rsp_data->vsp_hgt.user_addr) &&
			(rsp_data->vsp_hgt.knel_addr)) {
			if (copy_to_user((void __user *)
					rsp_data->vsp_hgt.user_addr,
					rsp_data->vsp_hgt.knel_addr,
					768)) {
				APRINT("CB: failed to copy the HGT data\n");
				ercd = -EFAULT;
				goto exit;
			}
		}

		DPRINT("CB: end = %d\n", rsp_data->rsp.end);
		if (rsp_data->vsp_hgt.knel_addr)
			kfree(rsp_data->vsp_hgt.knel_addr);
		if (rsp_data->vsp_hgo.knel_addr)
			kfree(rsp_data->vsp_hgo.knel_addr);
		kfree(rsp_data);
	}

	DPRINT("done\n");
	return 0;

exit:
	if (rsp_data) {
		if (rsp_data->vsp_hgt.knel_addr)
			kfree(rsp_data->vsp_hgt.knel_addr);
		if (rsp_data->vsp_hgo.knel_addr)
			kfree(rsp_data->vsp_hgo.knel_addr);
		kfree(rsp_data);
	}
	return ercd;
}

/*
 * vspm_ioctl_wait_cb_start - VSPM_IOC_CMD_WAIT_CB_START command processing
 * @priv:
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
static long vspm_ioctl_wait_cb_start(struct vspm_privdata *priv)
{
	DPRINT("called\n");

	/* wait for callback thread of user */
	if (wait_for_completion_interruptible(&priv->cb_start_comp)) {
		EPRINT("WAIT_CB_START: INTR\n");
		return -EINTR;
	}

	/* wait for running state of callback thread  */
	while ((!priv->cb_thread) || (priv->cb_thread->state == TASK_RUNNING))
		schedule();

	DPRINT("WAIT_CB_START: callback thread start\n");

	DPRINT("done\n");
	return 0;
}

/*
 * vspm_ioctl_cb_end - VSPM_IOC_CMD_CB_END command processing
 * @priv:
 */
static void vspm_ioctl_cb_end(struct vspm_privdata *priv)
{
	struct vspm_cb_rsp_data *rsp_data;
	struct vspm_cb_rsp_data *next;
	unsigned long lock_flag;

	DPRINT("called\n");

	spin_lock_irqsave(&priv->lock, lock_flag);
	list_for_each_entry_safe(rsp_data, next, &priv->rsp_head.list, list) {
		list_del(&rsp_data->list);
		if (rsp_data->vsp_hgt.knel_addr)
			kfree(rsp_data->vsp_hgt.knel_addr);
		if (rsp_data->vsp_hgo.knel_addr)
			kfree(rsp_data->vsp_hgo.knel_addr);
		kfree(rsp_data);
	}
	spin_unlock_irqrestore(&priv->lock, lock_flag);
	DPRINT("CB_END: callback thread up()\n");

	complete(&priv->comp);

	DPRINT("done\n");
}

/*
 * vspm_ioctl - ioctl processing of VSPM driver
 * Returns: On success 0 is returned. On error, a nonzero error number is
 * returned.
 */
long vspm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ercd = 0;
	struct vspm_privdata *priv;
	struct vspm_ioc_cmd vspm_ioc_cmd;

	DPRINT("called\n");

	priv = (struct vspm_privdata *)filp->private_data;

	switch (cmd) {
	case VSPM_IOC_CMD_ENTRY:
		/* check access */
		if (!access_ok(
				VERIFY_WRITE,
				(void __user *)arg,
				_IOC_SIZE(cmd))) {
			APRINT("failed to access_ok\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* copy from user area */
		if (copy_from_user(
				&vspm_ioc_cmd,
				(void __user *)arg,
				sizeof(vspm_ioc_cmd))) {
			APRINT("failed to copy the command data\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* entry */
		ercd = vspm_ioctl_entry(priv, &vspm_ioc_cmd);
		break;
	case VSPM_IOC_CMD_CANCEL:
		/* check access */
		if (!access_ok(
				VERIFY_WRITE,
				(void __user *)arg,
				_IOC_SIZE(cmd))) {
			APRINT("failed to access_ok\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* copy from user area */
		if (copy_from_user(
				&vspm_ioc_cmd,
				(void __user *)arg,
				sizeof(vspm_ioc_cmd))) {
			APRINT("failed to copy the command data\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* cancel */
		ercd = vspm_ioctl_cancel(priv, &vspm_ioc_cmd);
		break;
	case VSPM_IOC_CMD_CB:
		/* check access */
		if (!access_ok(
				VERIFY_WRITE,
				(void __user *)arg,
				_IOC_SIZE(cmd))) {
			APRINT("failed to access_ok\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* copy from user area */
		if (copy_from_user(
				&vspm_ioc_cmd,
				(void __user *)arg,
				sizeof(vspm_ioc_cmd))) {
			APRINT("failed to copy the command data\n");
			ercd = -EFAULT;
			goto exit;
		}

		/* callback function */
		ercd = vspm_ioctl_cb(priv, &vspm_ioc_cmd);
		break;
	case VSPM_IOC_CMD_WAIT_CB_START:
		/* start callback */
		ercd = vspm_ioctl_wait_cb_start(priv);
		break;
	case VSPM_IOC_CMD_CB_END:
		/* end callback */
		vspm_ioctl_cb_end(priv);
		break;
	default:
		ercd = -ENOTTY;
		break;
	}

	DPRINT("done\n");
exit:
	return ercd;
}

