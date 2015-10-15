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

#ifndef __FDPM_LFUNC_H__
#define __FDPM_LFUNC_H__

int fdpm_init(struct fdpm_privdata *priv);
int fdpm_lib_driver_initialize(struct platform_device *pdev,
			       struct fdpm_privdata *priv,
			       long *sub_ercd);
long fdp_ins_allocate_memory(T_FDPD_MEM **prv);
int fdpm_quit(struct fdpm_privdata *priv);
int fdpm_lib_driver_quit(struct fdpm_privdata *priv);
int fdpm_close_func(int stream_id,
		    struct fdpm_private *fdpm_private_data);

int fdpm_drv_set_thread(void *thread);
int fdpm_post_thread(void *thread);
int icb2(void *thread);

int fdpm_request_entry_init(struct fdpm_drvdata *fdpm_pdata);
int fdpm_request_entry(struct fdpm_resource_table *table,
		       struct fdpm_start_req *req,
		       struct fdpm_privdata *fdpm_pdata,
		       struct fdpm_private *fdpm_private_data);
void fdpm_queue_del_entry(fdpm_queue_entry *entry);
int fdpm_post_entry_init(struct fdpm_drvdata *fdpm_pdata);
int fdpm_post_entry(fdpm_queue_entry *req_entry,
		    fdpm_wait_entry *post_entry,
		    struct fdpm_drvdata *fdpm_pdata);
void fdpm_post_del_entry(fdpm_wait_entry *entry);
int fdpm_request_entry_check(struct fdpm_resource_table *table,
			     unsigned int source_id,
			     struct fdpm_privdata *fdpm_pdata);
int fdpm_post_entry_check(struct fdpm_resource_table *table,
			  unsigned int source_id,
			  struct fdpm_privdata *fdpm_pdata);

int fdpm_resource_get(T_FDP_R_OPEN *open_par,
		      int *dev_no,
		      int *first_get,
		      fdpm_pdata *fdpm_pdata);
int fdpm_table_entry(T_FDP_R_OPEN *open_par,
		     unsigned int device_no,
		     unsigned long *source_id,
		     struct fdpm_independ *pdata);
int fdpm_table_release(unsigned long source_id,
		       struct fdpm_independ *fdpm_pdata);
int fdpm_resource_entry(T_FDP_R_OPEN *open_par,
			int *first_get,
			fdpm_pdata *fdpm_pdata);
int fdpm_resource_release(int ocmode,
			  int vmode,
			  int dev_no,
			  unsigned long size,
			  fdpm_pdata *fdpm_pdata);
int fdpm_resource_exit(int dev_no,
		       fdpm_pdata *fdpm_pdata);
unsigned char fdpm_get_timer_ch(struct fdpm_resource_table *base_table);
int fdpm_resource_add_list(struct fdpm_resource_table *base_table,
			   struct fdpm_resource_table **add_table);
int fdpm_resource_del_list(struct fdpm_resource_table *target_table);
int fdpm_resource_search_source_list(unsigned long source_id,
				     int mode,
				     struct fdpm_resource_table *base_table,
				     struct fdpm_resource_table **target_table);
int fdpm_resource_search_dev_list(int device_no,
				  struct fdpm_resource_table *base_table,
				  struct fdpm_resource_table **target_table);
int fdpm_resource_del_all_list(struct fdpm_resource_table *base_table);

void fdpm_status_update(T_FDP_R_START *start_par,
			T_FDP_SEQ_STATUS *fdp_seq_status,
			T_FDP_HW_STATUS *fdp_hw_status,
			T_FDP_STATUS *fdp_status,
			fdpm_pdata *fdpm_pdata,
			struct fdp_independ *fdp_independ);
void fdpm_fdp_cb1_update(T_FDP_R_START *start_par,
			 T_FDP_R_CB1 *fdp_cb1,
			 struct fdp_independ *fdp_independ,
			 int stlmsk_flg);

void fdpm_stlmsk_control(T_FDP_R_START *start_par,
			 struct fdp_independ *fdp_independ,
			 struct fdpm_resource_table *target_table);
void fdpm_pd_control(T_FDP_R_START *start_par,
		     struct fdpm_resource_table *resource_table);
int fdpm_check_decodeinfo(T_FDPD_PICPAR previous_pic_par,
			  T_FDP_R_PICPAR *current_pic_par);
void fdpm_seq_control(T_FDP_R_START *start_par,
		      struct fdpm_resource_table *resource_table);
int fdpm_dec_decodeinfo(T_FDP_R_PICPAR *pic_par);
int fdpm_dec_decodeinfo2(T_FDPD_PICPAR *pic_par);
void fdpm_decode_count_update(T_FDP_SEQ_STATUS *fdp_seq_status,
			      int cclr);
void fdpm_update_picpar(struct fdpm_resource_table *resource_table,
			T_FDP_R_FPROC *fproc_par);
void fdpm_seq_write(T_FDP_R_START *start_par,
		    struct fdpm_resource_table *target_table);
int fdpm_ioctl_open_func(unsigned long arg,
			 struct fdpm_private *fdpm_private_data);
int fdpm_ioctl_close_func(unsigned long arg,
			  struct fdpm_private *fdpm_private_data);
int fdpm_ioctl_start_func(unsigned long arg,
			  struct fdpm_private *fdpm_private_data);
int fdpm_ioctl_cancel_func(unsigned long arg,
			   struct fdpm_private *fdpm_private_data);
int fdpm_ioctl_status_func(unsigned long arg,
			   struct fdpm_private *fdpm_private_data);
int fdpm_ioctl_wait_func(unsigned long arg,
			 struct fdpm_private *fdpm_private_data);
int fdpm_ioctl_reg_status_func(unsigned long arg,
			       struct fdpm_private *fdpm_private_data);
long fdpm_ioctl(struct file *fp,
		unsigned int cmd,
		unsigned long arg);

long FDPM_lib_DriverInitialize(unsigned long *handle);
#endif
