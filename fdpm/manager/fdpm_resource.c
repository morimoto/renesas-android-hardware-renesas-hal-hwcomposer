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

#include "fdpm_api.h"
#include "fdpm_if_par.h"
#include "include/fdpm_def.h"
#include "fdp/fdp_depend.h"
#include "include/fdpm_depend.h"
#include "include/fdpm_log.h"

unsigned char fdpm_get_timer_ch(struct fdpm_resource_table *base_table);
int fdpm_resource_entry(T_FDP_R_OPEN *open_par, int *first_get,
			fdpm_pdata *fdpm_pdata);
int fdpm_resource_exit(int dev_no, fdpm_pdata *fdpm_pdata);
extern int fdpm_resource_add_list(struct fdpm_resource_table *base_table,
				  struct fdpm_resource_table **add_table);
extern int fdpm_resource_del_list(struct fdpm_resource_table *target_table);
extern int fdpm_resource_search_source_list(unsigned long source_id, int mode,
				struct fdpm_resource_table *base_table,
				struct fdpm_resource_table **target_table);
int fdpm_resource_status_update(fdpm_pdata *fdpm_pdata);

/******************************************************************************
	Function:		fdp_resource_get
	Description:	FDP H/W resource get @ drv_FDP_Open
******************************************************************************/
int fdpm_resource_get(T_FDP_R_OPEN *open_par, int *dev_no, int *first_get,
		      fdpm_pdata *fdpm_pdata)
{
	int ret = 0;
	int hw_no = 0;

	if ((open_par->ocmode == FDP_OCMODE_OCCUPY) &&
	    (open_par->vmode == FDP_VMODE_NORMAL)) {/*FDP_OCMODE_OCCUPY */
		if (fdpm_pdata->fdp_resource_count >= FDPM_FDP_NUM) {
			ret = -1;
		} else {
			fdpm_pdata->fdp_resource_count++;
			*dev_no = fdpm_resource_entry(open_par, first_get,
						      fdpm_pdata);
			ret = 0;
		}
	} else if (open_par->vmode == FDP_VMODE_NORMAL) {
		/* FDP_OCMODE_COMMON */
		/* FDP_VMODE_NORMAL */
		if (fdpm_pdata->fdp_resource_count == FDPM_FDP_NUM) {
			/* no empty H/W resource */
			if (fdpm_pdata->fdp_share_vint_count > 0) {
				/* no first time share vint mode */
				if (fdpm_pdata->fdp_share_vint_count %
				    FDPM_VINT_MODE_NUM == 0) {
					/* demand new H/W resource */
					ret = -1;
				} else { /* no demand new H/W resource */
					fdpm_pdata->fdp_share_vint_count++;
					*dev_no = fdpm_resource_entry(open_par,
								      first_get,
								   fdpm_pdata);
					ret =  0;
				}
			} else {
				/* first time share vint mode ->
				no empty H/W resource */
				ret = -1;
			}
		} else {
			/* empty H/W resource exist */
			if (fdpm_pdata->fdp_share_vint_count > 0) {
				/* no first time share vint mode */
				if (fdpm_pdata->fdp_share_vint_count ==
				    FDPM_VINT_MODE_T_NUM) {
					/* reached max share vint mode */
					ret = -1;
				} else {
					if (fdpm_pdata->fdp_share_vint_count %
					    FDPM_VINT_MODE_NUM == 0) {
						/* demand new H/W resource */
						fdpm_pdata->
							fdp_resource_count++;
					}
					fdpm_pdata->fdp_share_vint_count++;
					*dev_no = fdpm_resource_entry(open_par,
								      first_get,
								fdpm_pdata);
					ret = 0;
				}
			} else{ /* first time share vint mode */
				fdpm_pdata->fdp_resource_count++;
				fdpm_pdata->fdp_share_vint_count++;
				*dev_no = fdpm_resource_entry(open_par,
							      first_get,
							      fdpm_pdata);
				ret = 0;
			}
		}
	} else if (open_par->vmode == FDP_VMODE_VBEST) {
		/* FDP_VMODE_VBEST */
		if (fdpm_pdata->fdp_resource_count == FDPM_FDP_NUM) {
			/* no empty H/W resource */
			if (fdpm_pdata->fdp_share_be_count > 0) {
				/* no first time best effort mode */
				*dev_no = fdpm_resource_entry(open_par,
							      first_get,
							      fdpm_pdata);
				ret = 0;
			} else {
				/* first time best effort mode */
				ret = -1;
			}
		} else {
			/* empty H/W resoruce exist */
			if (fdpm_pdata->fdp_share_be_count > 0) {
				/* no first time best effort mode */
				*dev_no = fdpm_resource_entry(open_par,
							      first_get,
							      fdpm_pdata);
				ret = 0;
			} else {
				/* first time best effort mode */
				fdpm_pdata->fdp_resource_count++;
				fdpm_pdata->fdp_share_be_count = 1;
				*dev_no = fdpm_resource_entry(open_par,
							      first_get,
							      fdpm_pdata);
				ret = 0;
			}
		}
	} else { /* beset effort mode (H/W select mode) */
		switch (open_par->vmode) {
		case FDP_VMODE_VBEST_FDP0:
			hw_no = 0; break;
		case FDP_VMODE_VBEST_FDP1:
			hw_no = 1; break;
		case FDP_VMODE_VBEST_FDP2:
			hw_no = 2; break;
		default:
			hw_no = 0; break;
		}
		if ((fdpm_pdata->fdpm_hw[hw_no].use == 1) &&
		    (fdpm_pdata->fdpm_hw[hw_no].mode == FDP_VMODE_NORMAL)) {
			ret = -1;
		} else if (fdpm_pdata->fdpm_hw[hw_no].use == 0) {
			fdpm_pdata->fdp_resource_count++;
			fdpm_pdata->fdp_share_be_count++;
			*dev_no = fdpm_resource_entry(open_par,
						      first_get,
						      fdpm_pdata);
			fdpm_pdata->fdpm_hw[*dev_no].use_count++;
			ret = 0;
		} else {
		/* fdpm_pdata->fdm_hw[hw_no].mode != FDP_VMODE_NORMAL */
			*dev_no = fdpm_resource_entry(open_par,
						      first_get,
						      fdpm_pdata);
			fdpm_pdata->fdpm_hw[*dev_no].use_count++;
			ret = 0;
		}
	}
	fdpm_resource_status_update(fdpm_pdata);
	DPRINT("fdp_resource_count = %d\n",
	       fdpm_pdata->fdp_resource_count);
	DPRINT("fdp_share_vint_count = %d\n",
	       fdpm_pdata->fdp_share_vint_count);
	return ret;
}

int fdpm_table_entry(T_FDP_R_OPEN *open_par, unsigned int device_no,
		     unsigned long *source_id, struct fdpm_independ *pdata)
{
	struct fdpm_resource_table *entry_table = NULL;

	DPRINT("called\n");

	if (fdpm_resource_add_list(&pdata->fdpm_itable, &entry_table) == -1)
		return -1;

	entry_table->use = 1;
	entry_table->source_id = *source_id;
	entry_table->device_no = device_no;
	entry_table->ocmode    = open_par->ocmode;
	entry_table->vintmode  = open_par->vmode;
	if (open_par->vmode == FDP_VMODE_NORMAL)
		entry_table->timer_ch = fdpm_get_timer_ch(&pdata->fdpm_itable);
	else
		entry_table->timer_ch = 0;

	entry_table->size = (open_par->insize.width *
			     open_par->insize.height)/3200;
	memset(&entry_table->status, 0, sizeof(T_FDP_STATUS));
	entry_table->seq_status.half_tgl = 1;
	entry_table->seq_status.first_seq = 1;
	return 0;
}

int fdpm_table_release(unsigned long source_id,
		       struct fdpm_independ *fdpm_pdata)
{
	struct fdpm_resource_table *target_table = NULL;

	if (fdpm_resource_search_source_list(source_id, 1,
					     &fdpm_pdata->fdpm_itable,
					     &target_table) == -1)
		return -1;

	fdpm_resource_del_list(target_table);

	return 0;
}

/* This function called when can get resource H/W  */
int fdpm_resource_entry(T_FDP_R_OPEN *open_par,
			int *first_get,
			fdpm_pdata *fdpm_pdata)
{
	int i;
	int first_flg = 0;
	int dev_no = -1;
	int empty_dev_no = -1;
	int resource_get = 0;
	unsigned long sumsize;
	int be_hw_count = 0;
	int min_sumsize = 0;

	sumsize = (open_par->insize.width * open_par->insize.height)/3200;

	if (open_par->ocmode == FDP_OCMODE_OCCUPY) {
		if (open_par->vmode == FDP_VMODE_VBEST) {
			/* search occupy mode and best effort mode user */
			for (i = 0; i < FDPM_FDP_NUM; i++) {
				if (fdpm_pdata->fdpm_hw[i].use == 0)
					empty_dev_no = i;
				if ((fdpm_pdata->fdpm_hw[i].use ==
				     (FDP_OCMODE_OCCUPY + 1)) &&
				    (fdpm_pdata->fdpm_hw[i].mode ==
				     FDP_VMODE_VBEST)) {
					if (first_flg == 0) {
						min_sumsize =
							fdpm_pdata->fdpm_hw[i].sumsize;
						dev_no = i;
						first_flg = 1;
					} else if (min_sumsize >
						   fdpm_pdata->fdpm_hw[i].sumsize) {
						min_sumsize =
							fdpm_pdata->fdpm_hw[i].sumsize;
						dev_no = i;
					}
					be_hw_count++;
				}
			}
			if (be_hw_count >= FDPM_BE_HW_NUM) {
				fdpm_pdata->fdpm_hw[dev_no].sumsize =
					fdpm_pdata->fdpm_hw[dev_no].sumsize + sumsize;
			} else if (empty_dev_no != -1) {
				fdpm_pdata->fdpm_hw[empty_dev_no].use =
					FDP_OCMODE_OCCUPY + 1;
				fdpm_pdata->fdpm_hw[empty_dev_no].use_count++;
				fdpm_pdata->fdpm_hw[empty_dev_no].mode =
					open_par->vmode;
				fdpm_pdata->fdpm_hw[empty_dev_no].sumsize = sumsize;
				dev_no = empty_dev_no;
				*first_get = 1;
				resource_get = 1;
			} else {
				fdpm_pdata->fdpm_hw[dev_no].sumsize =
					fdpm_pdata->fdpm_hw[dev_no].sumsize + sumsize;
			}
		} else if ((open_par->vmode == FDP_VMODE_VBEST_FDP0) ||
			   (open_par->vmode == FDP_VMODE_VBEST_FDP1) ||
			   (open_par->vmode == FDP_VMODE_VBEST_FDP2)) {
			switch (open_par->vmode) {
			case FDP_VMODE_VBEST_FDP0:
				dev_no = 0; break;
#if defined(M2CONFIG) || defined(H2CONFIG)
			case FDP_VMODE_VBEST_FDP1:
				dev_no = 1; break;
#endif
#if defined(H2CONFIG)
			case FDP_VMODE_VBEST_FDP2:
				dev_no = 2; break;
#endif
			default:
				dev_no = 0; break;
			}
			if (fdpm_pdata->fdpm_hw[dev_no].use == 0) {
				fdpm_pdata->fdpm_hw[dev_no].use =
					FDP_OCMODE_OCCUPY + 1;
				fdpm_pdata->fdpm_hw[dev_no].use_count++;
				fdpm_pdata->fdpm_hw[dev_no].mode =
					open_par->vmode;
				fdpm_pdata->fdpm_hw[dev_no].sumsize = sumsize;
				*first_get = 1;
				resource_get = 1;
			} else {
				fdpm_pdata->fdpm_hw[dev_no].use_count++;
				fdpm_pdata->fdpm_hw[dev_no].sumsize =
					fdpm_pdata->fdpm_hw[dev_no].sumsize + sumsize;
				*first_get = 0;
				resource_get = 1;
			}
		} else {
			/* occupy and vint mode user */
			/* get new resource */
			for (i = 0; i < FDPM_FDP_NUM; i++) {
				if (fdpm_pdata->fdpm_hw[i].use == 0) {/* no use */
					fdpm_pdata->fdpm_hw[i].use =
						FDP_OCMODE_OCCUPY + 1;
					fdpm_pdata->fdpm_hw[i].use_count++;
					fdpm_pdata->fdpm_hw[i].mode =
						open_par->vmode;
					fdpm_pdata->fdpm_hw[i].sumsize = sumsize;
					dev_no = i;
					*first_get = 1;
					resource_get = 1;
					break;
				}
			}
		}
	} else{
		/* common use mode user */
		if (open_par->vmode == FDP_VMODE_NORMAL) {
			/* search common use and vint mode user */
			for (i = 0; i < FDPM_FDP_NUM; i++) {
				if ((fdpm_pdata->fdpm_hw[i].use ==
				     (FDP_OCMODE_COMMON + 1)) &&
				    (fdpm_pdata->fdpm_hw[i].mode ==
				     FDP_VMODE_NORMAL)) {
					if (fdpm_pdata->fdpm_hw[i].use_count ==
					    FDPM_VINT_MODE_NUM) {
						/* if reached common use max count, search next h/w */
						continue;
					}
					/* else get common use h/w */
					fdpm_pdata->fdpm_hw[i].use_count++;
					fdpm_pdata->fdpm_hw[i].sumsize =
						fdpm_pdata->fdpm_hw[i].sumsize + sumsize;
					dev_no = i;
					first_get = 0;
					resource_get = 1;
					break;
				}
			}
			/* if can not find, get new resource H/W */
			if (resource_get == 0) {
				for (i = 0; i < FDPM_FDP_NUM; i++) {
					if (fdpm_pdata->fdpm_hw[i].use == 0) {
						/* no use */
						fdpm_pdata->fdpm_hw[i].use =
							FDP_OCMODE_COMMON + 1;
						fdpm_pdata->fdpm_hw[i].use_count++;
						fdpm_pdata->fdpm_hw[i].mode =
							open_par->vmode;
						fdpm_pdata->fdpm_hw[i].sumsize =
							sumsize;
						dev_no = i;
						*first_get = 1;
						resource_get = 1;
						break;
					}
				}
			}
		}
		/* prohibit common use mode & best effort mode user */
	}
	DPRINT("get:sumsize[%d]:%ld\n",
	       dev_no, fdpm_pdata->fdpm_hw[dev_no].sumsize);
	return dev_no;
}

int fdpm_resource_release(int ocmode, int vmode, int dev_no,
			  unsigned long size, fdpm_pdata *fdpm_pdata)
{
	if (ocmode == FDP_OCMODE_OCCUPY) {
		if (vmode == FDP_VMODE_VBEST) {
			if (fdpm_pdata->fdp_share_be_count > 0) {
				if (fdpm_pdata->fdpm_hw[dev_no].use_count == 1) {
					fdpm_pdata->fdp_resource_count--;
					fdpm_pdata->fdp_share_be_count--;
					fdpm_resource_exit(dev_no, fdpm_pdata);
				} else {
					fdpm_pdata->fdpm_hw[dev_no].use_count--;
				}
			}
		} else {
			if (fdpm_pdata->fdp_resource_count > 0) {
				fdpm_pdata->fdp_resource_count--;
				fdpm_resource_exit(dev_no, fdpm_pdata);
			}
		}
	} else if (vmode == FDP_VMODE_NORMAL) {
		if (fdpm_pdata->fdp_share_vint_count > 0) {
			if (fdpm_pdata->fdp_share_vint_count == 1) {
				fdpm_pdata->fdp_resource_count--;
				fdpm_pdata->fdp_share_vint_count = 0;
				fdpm_resource_exit(dev_no, fdpm_pdata);
			} else {
				if (fdpm_pdata->fdpm_hw[dev_no].use_count == 1) {
					fdpm_pdata->fdp_resource_count--;
					fdpm_pdata->fdp_share_vint_count--;
					fdpm_resource_exit(dev_no, fdpm_pdata);
				} else {
					fdpm_pdata->fdp_share_vint_count--;
					fdpm_pdata->fdpm_hw[dev_no].use_count--;
				}
			}
		}
	} else{/* FDP_VMODE_VBEST */
		if (fdpm_pdata->fdp_share_be_count > 0) {
			if (fdpm_pdata->fdpm_hw[dev_no].use_count == 1) {
				fdpm_pdata->fdp_resource_count--;
				fdpm_pdata->fdp_share_be_count--;
				fdpm_resource_exit(dev_no, fdpm_pdata);
			} else {
				fdpm_pdata->fdpm_hw[dev_no].use_count--;
			}
		}
	}

	if (fdpm_pdata->fdpm_hw[dev_no].sumsize < size)
		fdpm_pdata->fdpm_hw[dev_no].sumsize = 0;
	else
		fdpm_pdata->fdpm_hw[dev_no].sumsize =
			fdpm_pdata->fdpm_hw[dev_no].sumsize - size;

	fdpm_resource_status_update(fdpm_pdata);
	DPRINT("fdp_resource_count = %d\n",
	       fdpm_pdata->fdp_resource_count);
	DPRINT("fdp_share_vint_count = %d\n",
	       fdpm_pdata->fdp_share_vint_count);
	DPRINT("release:sumsize[%d]:%ld\n",
	       dev_no, fdpm_pdata->fdpm_hw[dev_no].sumsize);
	return 0;
}

int fdpm_resource_exit(int dev_no, fdpm_pdata *fdpm_pdata)
{
	fdpm_pdata->fdpm_hw[dev_no].use = 0;
	fdpm_pdata->fdpm_hw[dev_no].use_count = 0;
	fdpm_pdata->fdpm_hw[dev_no].mode = 0;
	return 0;
}

unsigned char fdpm_get_timer_ch(struct fdpm_resource_table *base_table)
{
	int hit = -1;
	int min_val = 0;
	int get_ch;
	struct fdpm_resource_table *tmp = NULL;
	struct fdpm_resource_table *previous = NULL;

	tmp = base_table->next;
	previous = base_table;

	while (tmp !=  NULL) {
		if ((tmp->use > 0) && (tmp->vintmode == FDP_VMODE_NORMAL)) {
			if (min_val == tmp->timer_ch) {
				min_val++;
				tmp = base_table->next;
				previous = base_table;
				hit = 1;
				continue;
			}
		}
		previous = tmp;
		tmp = tmp->next;
	}
	if (hit == -1)
		get_ch = 0;
	else
		get_ch = min_val;

	return get_ch;
}

int fdpm_resource_status_update(fdpm_pdata *fdpm_pdata)
{
	if (fdpm_pdata->fdp_resource_count == FDPM_FDP_NUM) {
		if (fdpm_pdata->fdp_share_vint_count > 0) {
			if (fdpm_pdata->fdp_share_vint_count == FDPM_VINT_MODE_T_NUM)
				fdpm_pdata->fdpm_status = FDPM_FULL_BUSY;
			else
				fdpm_pdata->fdpm_status = FDPM_OCCUPY_BUSY;
		} else {
			fdpm_pdata->fdpm_status = FDPM_FULL_BUSY;
		}
	} else if (fdpm_pdata->fdp_resource_count > 0) {
		fdpm_pdata->fdpm_status = FDPM_RDY;
	} else {
		fdpm_pdata->fdpm_status = FDPM_IDLE;
	}
	return 0;
}
