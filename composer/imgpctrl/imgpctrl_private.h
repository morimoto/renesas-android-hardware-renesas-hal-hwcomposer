/*************************************************************************/ /*
   imgpctrl_private.h
     imgpctrl private header.

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

#ifndef __IMGPCTRL_PRIVATE_H__
#define __IMGPCTRL_PRIVATE_H__

#include <linux/kthread.h>
#include <linux/semaphore.h>

#include <linux/dmaengine.h>
#include <linux/sh_dma.h>

#include "imgpctrl_public.h"
#include "vspm_public.h"
#include "tddmac_drv.h"
#include "vsp_drv.h"

/* IMGPCTRL internal define */

#define	IMGPCTRL_R_OK			(0)
#define	IMGPCTRL_R_NG			(-1)
#define	IMGPCTRL_R_PARA_ERR		(-2)
#define	IMGPCTRL_R_SEQ_ERR		(-3)
#define	IMGPCTRL_R_MEM_ERR		(-4)
#define	IMGPCTRL_R_TIME_OUT		(-106)
#define	IMGPCTRL_R_VSPM_INIT_NG		(-51)
#define	IMGPCTRL_R_VSPM_ENTRY_NG	(-52)

#define	IMGPCTRL_TYPE_VSPS		VSPM_TYPE_VSP_VSPS
#define	IMGPCTRL_TYPE_VSPR		VSPM_TYPE_VSP_VSPR
#define	IMGPCTRL_TYPE_VSPD0		VSPM_TYPE_VSP_VSPD0
#define	IMGPCTRL_TYPE_VSPD1		VSPM_TYPE_VSP_VSPD1
#define	IMGPCTRL_TYPE_VSP		\
				(IMGPCTRL_TYPE_VSPD1 | IMGPCTRL_TYPE_VSPD0 | \
				 IMGPCTRL_TYPE_VSPR | IMGPCTRL_TYPE_VSPS)
#define	IMGPCTRL_TYPE_TDD		VSPM_TYPE_2DDMAC_AUTO
#define	IMGPCTRL_TYPE_SYSDMAC		\
				((~(IMGPCTRL_TYPE_VSP | IMGPCTRL_TYPE_TDD)) \
				& 0xFFFFU)

#ifndef IMGPCTRL_JOB_PRIO
#define	IMGPCTRL_JOB_PRIO		(126)
#endif

#define	IMGPCTRL_INVALID_JOBID		(0)


#define	IMGPCTRL_REQ_LAYER_0		1U
#define	IMGPCTRL_REQ_LAYER_1		2U
#define	IMGPCTRL_REQ_LAYER_2		4U
#define	IMGPCTRL_REQ_LAYER_3		8U

#define	IMGPCTRL_REQ_NO_COMMAND		(0x0U)
#define IMGPCTRL_REQ_BLEND		(0x1U)
#define IMGPCTRL_REQ_RESIZE		(0x2U)
#define IMGPCTRL_REQ_COLOR_CONVERT	(0x4U)
#define IMGPCTRL_REQ_TRIMMING		(0x8U)
#define IMGPCTRL_REQ_ROTATE		(0x10U)

#define IMGPCTRL_ST_INIT		(0x0)
#define IMGPCTRL_ST_READY		(0x1)
#define IMGPCTRL_ST_EXE			(0x2)
#define IMGPCTRL_ST_END			(0x3)
#define IMGPCTRL_ST_NEXT		(0x4)

#define IMGPCTRL_1BIT_SHIFT		1U
#define IMGPCTRL_4BIT_SHIFT		4U
#define IMGPCTRL_8BIT_SHIFT		8U
#define IMGPCTRL_12BIT_SHIFT		12U
#define IMGPCTRL_16BIT_SHIFT		16U

#define IMGPCTRL_1BIT_CHECK		0x01

#define IMGPCTRL_1BIT			0x0001U

#define IMGPCTRL_LAYER_MAX		(0x4)
#define IMGPCTRL_STACK_MAX		(0x5)
#define IMGPCTRL_SYSDMAC_MAX		(0x3)

#define IMGPCTRL_NOT_YUV		(0x0)
#define IMGPCTRL_YUV420SP		(0x1)
#define IMGPCTRL_YUV422SP		(0x2)
#define IMGPCTRL_YUV420PL		(0x3)
#define IMGPCTRL_YUV420SP_NV21		(0x4)
#define IMGPCTRL_YUV422I_UYVY		(0x5)

#define IMGPCTRL_MAX_UDS		3

#ifndef RT_GRAPHICS_EXTRA_BUFF_NUM
#define	RT_GRAPHICS_EXTRA_BUFF_NUM	(0)
#endif

#ifndef RT_GRAPHICS_TRACE_ID
#define	RT_GRAPHICS_TRACE_ID		(0)
#endif

#define IMGPCTRL_LOG_ADDRESS_SIZE	(sizeof(struct t_trace_info))
#define IMGPCTRL_ERROR_LOG_SIZE		(36*4)	/* byte */

/* Phase1-1 Threshold value */
#define RT_GRAPHICS_AREA_THRESHOLD_VALUE	2284544

#define IMGPCTRL_INPUT_MAX_ANOTHER_LENGTH		\
	((RT_GRAPHICS_MAX_IMAGE_AREA)/(RT_GRAPHICS_MAX_IMAGE_LENGTH))

#define IMGPCTRL_INPUT_MAX_TEMP_AREA			\
	(((((RT_GRAPHICS_MAX_IMAGE_LENGTH)+16)		\
	*((IMGPCTRL_INPUT_MAX_ANOTHER_LENGTH)+16)+15)/16)*16)

/* imgpctrl_ext_layer parameter */
struct t_ext_layer {
	u_short	x_offset;
	u_short	y_offset;
};

/* imgpctrl_exe_info parameter */
struct t_exe_info {
	u_short			hw_type;	/* request H/W IP(VSP/TDD) */
	u_short			rq_type;	/* request type(rot/col/.) */
	u_short			rq_layer;	/* request layer */
	struct screen_grap_layer	input_layer[IMGPCTRL_LAYER_MAX];
	struct screen_grap_layer	output_layer[IMGPCTRL_LAYER_MAX];
	struct t_ext_layer	ext_input[IMGPCTRL_LAYER_MAX];
	u_long			bg_color;	/* background color */
};

/* tmp buf information */
struct t_buf_info {
	u_long	size;
	u_long	p_addr;
	u_long	h_addr;
	u_long	v_addr;
	u_long	flag;
};

/* imgpctrl_chain_info parameter */
struct t_exe_extinfo {
	struct t_exe_info	param;		/* execute parameter */
	u_long			ct_hdl;
	struct t_buf_info	inbuf;
	struct t_buf_info	outbuf;
	short			status;		/* the status */
};

/* imgpctrl_chain_info parameter */
struct t_chain_info {
	struct t_exe_extinfo	chain[IMGPCTRL_STACK_MAX];
						/* layer parameter per dir */
	short			ptr;
	short			num;
};

/* memory control parameter */
struct t_memory_info {
	struct t_buf_info	buf_info;
	u_long	buf_offset;
	u_long	start_ptr;
	u_long	end_ptr;
	u_char	buf_use_flag;
	struct	semaphore	mem_sem;
};

/* spinlock control */
struct t_spinlock_info {
	spinlock_t	bl_lock;
};

/* imgpctrl conrtol semaphore */
struct t_imgpctrl_info {
	struct semaphore	imgpctrl_sem;
};

/* layer information */
struct t_layer_info {
	long	sts;
	long	cb_sts;
};

/* screen_grap_handle parameter */
typedef void (*IMGPCTRL_BLEND_CB)(int result, u_long user_data);

/* dma info for imgpctrl */
struct t_dma_info {
	struct dma_chan		*chan;

	struct sh_dmae_slave	slave;
};

struct screen_grap_handle {
	struct t_chain_info		stack[IMGPCTRL_LAYER_MAX];
	short				layer_num;
	u_short				layer_1st_part_bit;
	u_short				layer_blend_bit;
	bool				blend_req_flag;
	struct screen_grap_layer	input_layer[IMGPCTRL_LAYER_MAX];
	struct screen_grap_image_param	output_image;
	u_long				bg_color;
	IMGPCTRL_BLEND_CB		notify_graphics_image_blend;
	u_long				user_data;	/* user data */
	struct task_struct		*cb_thread[4];
	struct task_struct		*cb_bl_thread;
	wait_queue_head_t		fw_cb_queue0;
	wait_queue_head_t		fw_cb_queue1;
	wait_queue_head_t		fw_cb_queue2;
	wait_queue_head_t		fw_cb_queue3;
	wait_queue_head_t		fw_bl_queue;
	struct t_memory_info		mem_info;
	struct t_spinlock_info		spin_info;
	struct t_imgpctrl_info		imgpctrl_info;
	u_short				o_width[IMGPCTRL_LAYER_MAX];
	u_short				o_height[IMGPCTRL_LAYER_MAX];
	struct t_layer_info		info[IMGPCTRL_LAYER_MAX];
	struct t_spinlock_info		trace_spin;	/* tracelog spinlock */
	struct t_imgpctrl_info		err_sem;	/* errorlog semaphore */
	u_long				*log_start_ptr;
	short				mode;
	struct t_dma_info		dma_info_p[IMGPCTRL_SYSDMAC_MAX];
	dma_cookie_t			cookie[IMGPCTRL_SYSDMAC_MAX];
	int				use_chan_num;
};

/* callback function pointer */
typedef void (*FW_CTRL_CB)(u_long ctrl_hdl, u_long udata, long c_ret);

/* imgpctrl_ctrl_thread_handle parameter */
struct t_ctrl_hdl {
	struct task_struct	*ctrl_th;		/* ctrl thread task */
	FW_CTRL_CB		fw_ctrl_cb_fp;		/* framework cb addr */
	u_long			udata;			/* framework udata */
	u_short			rq_layer;		/* request layer */
	u_long			vspm_hdl;		/* VSPM handle */
	u_short			entry_num;		/* Entry num */
	u_long			entry_jobid[2];		/* Entry job ID */
	long			entry_ret[2];		/* Entry return */
	u_short			cb_num;			/* Callback num */
	u_long			cb_jobid[2];		/* Callback job ID */
	long			cb_ret[2];		/* Callback return */
	void			*vspm_param;		/* VSPM param */
	wait_queue_head_t	ctrl_main_queue;	/* ctrl wait queue */
};

/* vspm_param member saved to imgpctrl ctrl handle(for vsp) */
struct t_vspm_tdd {
	VSPM_IP_PAR		vspm_ip;
	VSPM_2DDMAC_PAR		pt2dDmac;
	T_TDDMAC_MODE		p_tddmac_mode;
	T_TDDMAC_REQUEST	p_tddmac_request;
};

struct t_ctrl_hdl_tdd {
	struct t_vspm_tdd		vspm_tdd[2];
};

/* vspm_param member saved to imgpctrl ctrl handle(for vsp) */
struct t_vspm_vsp {
	VSPM_IP_PAR		vspm_ip;
	VSPM_VSP_PAR		p_vsp;

	T_VSP_IN		src_par[IMGPCTRL_LAYER_MAX];
	T_VSP_OUT		dst_par;
	T_VSP_CTRL		ctrl_par;
	T_VSP_ALPHA		src_alpha[IMGPCTRL_LAYER_MAX];

	T_VSP_UDS		uds_par[IMGPCTRL_MAX_UDS];

	T_VSP_BRU		bru_par;
	T_VSP_BLEND_CONTROL	bru_ctrl_par[IMGPCTRL_LAYER_MAX];
	T_VSP_BLEND_ROP	bru_rop;
	T_VSP_BLEND_VIRTUAL	bru_vir_par;
};

struct t_ctrl_hdl_vsp {
	struct t_vspm_vsp		vspm_vsp;
};

/* Function */
extern int alloc_heap_buf(struct screen_grap_handle *bl_hdl);
extern int free_heap_buf(struct screen_grap_handle *bl_hdl);

extern int make_blend_chain(struct screen_grap_handle *bl_hdl);
extern int main_blend(struct screen_grap_handle *bl_hdl);

extern void fw_blend_cb(u_long ctrl_hdl, u_long udata, long c_ret);

extern long imgpctrl_ctrl_new(u_long *p_ctrl_hdl, FW_CTRL_CB fw_ctrl_cb_fp,
	struct screen_grap_handle *bl_hdl);
extern long imgpctrl_ctrl_exe(u_long ctrl_hdl,
			struct t_exe_info *exe_info,
			u_long udata);
extern long imgpctrl_ctrl_cancel(u_long ctrl_hdl,
	struct screen_grap_handle *bl_hdl);
extern long imgpctrl_ctrl_delete(u_long ctrl_hdl,
	struct screen_grap_handle *bl_hdl);
extern void imgpctrl_vspm_callback(
	u_long uw_job_id, long w_result, u_long uw_user_data);

extern long imgpctrl_ctrl_exe_tdd(u_long ctrl_hdl,
			struct t_exe_info *exe_info);
extern long imgpctrl_ctrl_exe_vsp(u_long ctrl_hdl,
	struct t_exe_info *exe_info);
extern long imgpctrl_ctrl_exe_dma(
	u_long ctrl_hdl,
	struct t_exe_info *exe_info);
extern void imgpctrl_dma_callback(void *completion);

#endif /* __IMGPCTRL_PRIVATE_H__ */

