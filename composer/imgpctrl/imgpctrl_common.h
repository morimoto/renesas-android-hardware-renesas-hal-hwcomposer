/*************************************************************************/ /*
  imgpctrl_common.h
      imgpctrl common header file (message log/memory manage/common...).

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

#ifndef __LOG_KERNEL_H__
#define __LOG_KERNEL_H__

#include <linux/kernel.h>

#define MSG_LVL_FATAL	(5)
#define MSG_LVL_ERROR	(4)
#define MSG_LVL_HIGH	(3)
#define MSG_LVL_TRACE	(2)
#define MSG_LVL_DEBUG	(1)
#define MSG_LVL_INFO	(0)
#define MSG_LVL_NONE	(255)

/* message level set*/
#ifndef MSG_LEVEL
#define MSG_LEVEL MSG_LVL_NONE
#endif

/* FATAL Error priority messages */
#if (MSG_LEVEL < MSG_LVL_NONE)
#define MSG_FATAL(args...)	((void)pr_alert(args))
#else
#define MSG_FATAL(args...)
#endif

/* Error priority messages */
#if (MSG_LEVEL < MSG_LVL_FATAL)
#define MSG_ERROR(args...)	((void)pr_alert(args))
#else
#define MSG_ERROR(args...)
#endif

/* High priority messages */
#if (MSG_LEVEL < MSG_LVL_ERROR)
#define MSG_HIGH(args...)	((void)pr_alert(args))
#else
#define MSG_HIGH(args...)
#endif

/* Trace messeges */
#if (MSG_LEVEL < MSG_LVL_HIGH)
#define MSG_TRACE(args...)	((void)pr_alert(args))
#else
#define MSG_TRACE(args...)
#endif

/* Trace messeges */
#if (MSG_LEVEL < MSG_LVL_TRACE)
#define MSG_DEBUG(args...)	((void)pr_alert(args))
#else
#define MSG_DEBUG(args...)
#endif

/* version number */
#define IMGPCTRL_VER		402	/* imgpctrl_v04.02 */

#define ERR_ARY_NUM		8

/* error trace use on update flag */
#define LOG_NOUPDATE		0
#define LOG_UPDATE		1

/* trace log use */
#define IMGPCTRL_LOGCNT		4	/* 1ptr=4byte */
#define IMGPCTRL_TRACE1_SIZE	3600

/* define function_id */
#define FID_SCREEN_GRAPHICS_NEW			0x0100
#define FID_SCREEN_GRAPHICS_IMAGE_BLEND		0x0101
#define FID_SCREEN_GRAPHICS_DELETE		0x0102
#define FID_MAKE_CHAIN_BLEND			0x0200
#define FID_MAIN_BLEND				0x0201
#define FID_MAKE_PRE_BLEND_PARAM		0x0202
#define FID_EXE_PRE_BLEND			0x0203
#define FID_MAIN_CB_THREAD0			0x0204
#define FID_MAIN_CB_THREAD1			0x0205
#define FID_MAIN_CB_THREAD2			0x0206
#define FID_MAIN_CB_THREAD3			0x0207
#define FID_CHK_NEXT_COMMAND			0x0208
#define FID_EXE_NEXT_COMMAND			0x0209
#define FID_MAIN_TH_BLEND			0x020A
#define FID_MAKE_BLEND_BG_PARAM			0x020B
#define FID_MAKE_BLEND_PARAM			0x020C
#define FID_EXE_BLEND				0x020D
#define FID_IMGPCTRL_REQ_COLOR_CONV		0x020E
#define FID_MAKE_COLOR_CONV_PARAM		0x020F
#define FID_IMGPCTRL_REQ_RESIZE			0x0210
#define FID_MAKE_RESIZE_PARAM			0x0211
#define FID_IMGPCTRL_REQ_ROTATE			0x0212
#define FID_MAKE_ROTATE_PARAM			0x0213
#define FID_MAKE_TMPBUF_PARAM			0x0214
#define FID_ALLOC_HEAP_BUF			0x0215
#define FID_FREE_HEAP_BUF			0x0216
#define FID_ALLOC_TMPBUF			0x0217
#define FID_FREE_TMPBUF				0x0218
#define FID_GET_USE_TDD				0x0219
#define FID_CHK_USE_COLOR_CONV			0x021A
#define FID_GET_USE_RESIZE			0x021B
#define FID_GET_SCALING_ORDER			0x021C
#define FID_GET_FORMAT_YUV_KIND			0x021D
#define FID_CHK_ROTATE_ANGLE			0x021E
#define FID_CALC_BPP_QUANT2			0x021F
#define FID_CALC_XY_OFFSET			0x0220
#define FID_DST_WIDTH_UNIT			0x0221
#define FID_IMGPCTRL_PUSH			0x0222
#define FID_IMGPCTRL_POP			0x0223
#define FID_IMGPCTRL_INIT_WAITQUEUE		0x0224
#define FID_RUN_CB_THREAD			0x0225
#define FID_FW_CTRL_CB				0x0226
#define FID_FW_BLEND_CB				0x0227
#define FID_MAKE_PRE_BLEND_PROCESS		0x0228
#define FID_GET_SCALING_SIZE			0x0229
#define FID_IMGPCTRL_CTRL_NEW			0x0500
#define FID_IMGPCTRL_CTRL_EXE			0x0501
#define FID_IMGPCTRL_CTRL_CANCEL		0x0502
#define FID_IMGPCTRL_CTRL_DELETE		0x0503
#define FID_IMGPCTRL_CTRL_WAIT			0x0504
#define FID_IMGPCTRL_VSPM_CALLBACK		0x0505
#define FID_IMGPCTRL_DMA_CALLBACK		0x0506
#define FID_IMGPCTRL_CTRL_EXE_TDD		0x0600
#define FID_IMGPCTRL_TDD_VSPM_ENTRY		0x0601
#define FID_IMGPCTRL_TDD_SETPARAM		0x0602
#define FID_IMGPCTRL_TDD_GETLAYERNUM		0x0603
#define FID_IMGPCTRL_TDD_GETLOOPNUM		0x0604
#define FID_IMGPCTRL_TDD_SETFORMAT		0x0605
#define FID_IMGPCTRL_TDD_SETFORMATC		0x0606
#define FID_IMGPCTRL_TDD_SETROTMIR		0x0607
#define FID_IMGPCTRL_CTRL_EXE_VSP		0x0700
#define FID_IMGPCTRL_CTRL_EXE_DMA		0x0701
#define FID_IMGPCTRL_VSP_PARA_GEN		0x0800
#define FID_IMGPCTRL_VSP_BLEND_CHECK_PARAM	0x0801
#define FID_IMGPCTRL_VSP_BLEND			0x0802
#define FID_IMGPCTRL_VSP_ZOOM			0x0803
#define FID_IMGPCTRL_VSP_CONVERSION		0x0804
#define FID_IMGPCTRL_VSP_FORMAT_GET		0x0805
#define FID_IMGPCTRL_VSP_IN_CSC_SET		0x0806
#define FID_IMGPCTRL_VSP_T_VSP_IN_SET		0x0807
#define FID_IMGPCTRL_VSP_IN_ALPHA_SET		0x0808
#define FID_IMGPCTRL_VSP_T_VSP_OUT_SET		0x0809
#define FID_IMGPCTRL_VSP_T_VSP_BRU_SET		0x080A
#define FID_IMGPCTRL_VSP_T_UDS_SET		0x080B
#define FID_IMGPCTRL_VSP_UDS_GETSCALINGRATIO	0x080C
#define FID_IMGPCTRL_VSP_BLEND0			0x080D
#define FID_IMGPCTRL_VSP_UDS_SCALEDOWNPROCESS	0x080E

/* trace log flag code */
#define FUNC_IN					0x00
#define VSPM_INIT_BC				0x10
#define VSPM_INIT_AC				0x11
#define VSPM_ENTRY_BC				0x12
#define VSPM_ENTRY_AC				0x13
#define VSPM_QUIT_BC				0x14
#define VSPM_QUIT_AC				0x15
#define DMA_INIT_BC				0x16
#define DMA_INIT_AC				0x17
#define DMA_MEM_BUF_BC				0x18
#define DMA_MEM_BUF_AC				0x19
#define DMA_PEND_BC				0x1A
#define DMA_PEND_AC				0x1B
#define DMA_WAIT_BC				0x1C
#define DMA_WAIT_AC				0x1D
#define DMA_QUIT_BC				0x1E
#define DMA_QUIT_AC				0x1F
#define MMNGR_ALLOC_BC				0x20
#define MMNGR_ALLOC_AC				0x21
#define MMNGR_FREE_BC				0x22
#define MMNGR_FREE_AC				0x23
#define KZALLOC_BC				0x30
#define KZALLOC_AC				0x31
#define KFREE_BC				0x32
#define KFREE_AC				0x33
#define GET_SEM_BC				0x40
#define GET_SEM_AC				0x41
#define FREE_SEM_BC				0x42
#define FREE_SEM_AC				0x43
#define GET_THREAD_BC				0x50
#define GET_THREAD_AC				0x51
#define FREE_THREAD_BC				0x52
#define FREE_THREAD_AC				0x53
#define GET_EVENT_BC				0x60
#define GET_EVENT_AC				0x61
#define FREE_EVENT_BC				0x62
#define FREE_EVENT_AC				0x63
#define CTRL_EXE_BC				0x70
#define CTRL_EXE_AC				0x71
#define ERR_VAL_SET				0xFD
#define FUNC_OUT_ERR				0xFE
#define FUNC_OUT_NRM				0xFF

/* error define */
#define ALLOC_MEM_SZ_ERR			0x00000001
#define TRACE_ID_ERR				0x00000002
#define MMNGR_ALLOC_ERR				0x00000003
#define MEM_PARAM_ERR				0x00000004
#define FREE_MEM_ERR				0x00000005
#define	MEM_CTRL_ERR				0x00000006
#define HDL_ALLOC_ERR				0x00000007
#define VSPM_INIT_NG				0x00000010
#define VSPM_ENTRY_NG				0x00000011
#define VSPM_EXE_TMOUT				0x00000012
#define VSPM_CANCEL_NG				0x00000013
#define VSPM_QUIT_ERR				0x00000014
#define ENTRY_NUM_OVER				0x00000020
#define PARAM_NULL				0x00001001
#define HDL_PTR_NULL				0x00001002
#define CB_PTR_NULL				0x00001003
#define EXEINFO_PTR_NULL			0x00001004
#define CB_ERROR				0x00001010
#define CB_NUM_OVER				0x00001011
#define ALLOC_HEAP_BUF_ERR			0x00001020
#define FREE_HEAP_BUF_ERR			0x00001021
#define MEM_ERR					0x00001022
#define BUFF_USING				0x00001023
#define MEM_OVERFLOW				0x00001024
#define CANT_ALLOC_MEM				0x00001025
#define SEM_TMOUT				0x00001030
#define KTHREAD_RUN_ERR				0x00001031
#define RUN_CB_THREAD_ERR			0x00001032
#define EV_TMOUT				0x00001033
#define COMMAND_NOT_MATCHED			0x00001040
#define HDL_DEL_ERR				0x00001041
#define OUT_OF_RANGE_ROT			0x00001050
#define OUT_OF_RANGE_MIR			0x00001051
#define UNVALID_FORMAT				0x00001052
#define INVALID_REQUEST				0x00001053
#define OUT_OF_RANGE_RAT			0x00001054
#define OUT_OF_RANGE_SZ				0x00001055
#define INVALID_LAYER				0x00001056
#define IN_LAYER_NOT_NULL			0x00001060
#define MAIN_BLEND_ERR				0x00001061
#define EXE_BLEND_ERR				0x00001062
#define STACK_LAY_EMPTY				0x00001063
#define EXE_NEXT_COMMOND_ERR			0x00001064
#define STACK_FULL				0x00001065
#define CTRL_NEW_ERR				0x00001070
#define CTRL_EXE_ERR				0x00001071
#define CTRL_DEL_ERR				0x00001072
#define EXE_VSPTDD_ERR				0x00001073
#define BLEND_ERR				0x00001080
#define UDS_ERR					0x00001081
#define LAY_OVER				0x00001082
#define MAKE_PRE_PARAM_ERR			0x00001083
#define EXE_PRE_BLEND_ERR			0x00001084
#define CHK_NEXT_COMMOND_ERR			0x00001085
#define IOREMAP_ERR				0x00001086
#define USE_TDD_ERR				0x00001087

/**********************************************************************/
/* trace structure                                                    */
/**********************************************************************/
struct t_trace_info {
	u_long	*trace1_add;	/* trace log1 address */
	u_long	*err_hdl_add;	/* error handle log address */
	u_long	*trace2_add;	/* trace log2 address */
	u_long	trace2_size;	/* trace log2 size */
	u_long	*trace1_add_phys;	/* trace log1 physical address */
	u_long	prev_virt_addr;	/* previous virtual_address */
};

struct t_err_info {
	u_long			id_line;	/* function ID */
	long			err_code;	/* error code */
	u_long			temp;		/* user matter */
	u_long			time;		/* time */
};

struct t_err_log {
	struct t_trace_info	t_info;		/* trace information */
	u_long			version;	/* imgpctrl version */
	u_long			trace_flg;	/* trace update cehck flag */
	u_long			err_cnt;	/* array number(0-7) */
	struct t_err_info	err_info[ERR_ARY_NUM];	/* error log */
};

/**********************************************************************/
/* initialize function                                                */
/**********************************************************************/

/**********************************************************************/
/* debug function                                                     */
/**********************************************************************/
extern void imgpctrl_cmn_dbg_exe_param(
				struct t_exe_info *exe_info, u_short lay);
extern void imgpctrl_cmn_dbg_ctrl_hdl_param(const struct t_ctrl_hdl *ct_hdl);
extern void imgpctrl_cmn_dbg_tdd_param(const T_TDDMAC_MODE *m2d_par,
				const T_TDDMAC_REQUEST *req_par);
extern void imgpctrl_cmn_dbg_vsp_param(struct t_vspm_vsp *vspm_ptr,
				u_short req_lay, u_short uds_num,
				u_short rq_type);
extern void imgpctrl_graphdl_log(struct t_exe_info *exe_info ,
			struct screen_grap_handle *bl_hdl);
extern void imgpctrl_error_log(u_short function_id, u_short line,
	long errorcode, u_long temp, struct screen_grap_handle *bl_hdl);
extern void imgpctrl_trace_log(u_short function_id, u_char flag, u_char size,
	u_long *ptr, struct screen_grap_handle *bl_hdl);

/* Case of RT_GRAPHICS_TRACE_ID = 0 imgpctrl_error_log() only output */

/* trace log(Check ID=2 or 4 in alloc_heap_buf())  */
#if (RT_GRAPHICS_TRACE_ID > 0)
#define IMGPCTRL_ERROR_LOG(function_id, line, errorcode, temp, bl_hdl) \
	imgpctrl_error_log(function_id, line, errorcode, temp, bl_hdl)
#else
#define IMGPCTRL_ERROR_LOG(function_id, line, errorcode, temp, bl_hdl)
#endif

#if (RT_GRAPHICS_TRACE_ID >= 2)
#define IMGPCTRL_TRACE_LOG(function_id, flag, size, ptr, bl_hdl) \
	imgpctrl_trace_log(function_id, flag, size, ptr, bl_hdl)
#else
#define IMGPCTRL_TRACE_LOG(function_id, flag, size, ptr, bl_hdl)
#endif

/* Error hdl copy*/
#if (RT_GRAPHICS_TRACE_ID >= 3)
#define IMGPCTRL_GRAPHDL_LOG(exe_info, bl_hdl) \
	imgpctrl_graphdl_log(exe_info, bl_hdl)
#else
#define IMGPCTRL_GRAPHDL_LOG(exe_info, bl_hdl)
#endif

#endif /* __LOG_KERNEL_H__ */
