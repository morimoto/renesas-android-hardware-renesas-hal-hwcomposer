/*************************************************************************/ /*
 VCP driver

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

#ifndef	MCVX_API_H
#define	MCVX_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mcvx_types.h"
#include "mcvx_register.h"

/* stream_type */
#define	MCVX_MPEG4							( 0UL )
#define	MCVX_H263							( 1UL )
#define	MCVX_H264							( 2UL )
#define MCVX_SRS							( 3UL )
#define	MCVX_VC1							( 4UL )
#define	MCVX_DIVX							( 5UL )
#define	MCVX_MPEG2							( 6UL )
#define	MCVX_MPEG1							( 7UL )
#define MCVX_VP6							( 8UL )
#define MCVX_AVS							( 9UL )
#define MCVX_RLV							( 10UL )
#define MCVX_VP8							( 12UL )

/* mode */
#define	MCVX_ENC							( 0UL )
#define	MCVX_DEC							( 1UL )

#define MCVX_FALSE							( 0UL )
#define MCVX_TRUE							( 1UL )

/* not applicable */
#define	MCVX_NA								( 0UL )	

/* rtn_code */
#define MCVX_NML_END						( 0L )
#define MCVX_ERR_PARAM						( -1L )
#define MCVX_ERR_WORK						( -2L )
#define MCVX_ERR_STATE						( -3L )
#define MCVX_ERR_CONTEXT					( -4L )
#define MCVX_ERR_SEVERE						( -256L )

/* CE FSM */
#define MCVX_STATE_CE_IDLE					( 0UL )
#define MCVX_STATE_CE_RUNNING				( 1UL )

/* VLC FSM */
#define MCVX_STATE_VLC_IDLE					( 16UL )
#define MCVX_STATE_VLC_RUNNING				( 17UL )

/* FCV FSM */
#define MCVX_STATE_FCV_IDLE					( 32UL )
#define MCVX_STATE_FCV_RUNNING				( 33UL )

/* EVENT TYPES */
#define MCVX_EVENT_CE_START					( 0x00UL )
#define MCVX_EVENT_REQ_CE_STOP				( 0x01UL )

#define MCVX_EVENT_VLC_START				( 0x10UL )
#define MCVX_EVENT_REQ_VLC_STOP				( 0x11UL )

#define MCVX_EVENT_FCV_START				( 0x20UL )
#define MCVX_EVENT_REQ_FCV_STOP				( 0x21UL )

#define MCVX_CE								( 0UL )
#define MCVX_VLC							( 1UL )
#define MCVX_FCV							( 2UL )

/* cmn_pic_type */
#define	MCVX_I_PIC							( 0UL )	/* including IDR */
#define	MCVX_P_PIC							( 1UL )
#define	MCVX_B_PIC							( 2UL )
#define	MCVX_D_PIC							( 4UL )

/* cmn_pic_struct */
#define MCVX_PS_TOP							( 0UL )
#define MCVX_PS_BOT							( 1UL )
#define MCVX_PS_FRAME						( 2UL )

/* alloc_mode */
#define MCVX_ALLOC_FRAME					( 0UL )
#define MCVX_ALLOC_FIELD					( 1UL )

/* cmn_chroma_format */
#define	MCVX_CF_400							( 0UL )
#define	MCVX_CF_420							( 1UL )
#define	MCVX_CF_422							( 2UL )
#define	MCVX_CF_444							( 3UL )

#define	MCVX_MAX_NUM_REF_PIC				( 32UL )

/* yuv_format */
#define MCVX_2PL_Y_CBCR						( 0UL )
#define MCVX_2PL_Y_CRCB						( 1UL )
#define MCVX_3PL							( 2UL )

/* addressing mode */
#define	MCVX_AM_LINEAR						( 0UL )
#define	MCVX_AM_TILE						( 2UL )	/* reserved for possible future extensions */

/* dpar[] */
#define MCVX_DPAR_IDX_CAXI2_PORT_DISABLE	( 0UL )
#define MCVX_DPAR_IDX_IMR_PORT_DISABLE		( 1UL )
#define MCVX_DPAR_IDX_IMSB_ENABLE			( 2UL )
#define MCVX_DPAR_IDX_AXI_OUTPUT_DISABLE	( 3UL )
#define MCVX_DPAR_IDX_STB_INPUT_ENABLE		( 4UL )
#define MCVX_DPAR_IDX_USE_REFBUF			( 5UL )
#define MCVX_DPAR_IDX_WCMD_SEP				( 6UL )
#define MCVX_DPAR_IDX_VLC_EDT_ENABLE		( 16UL )
#define MCVX_DPAR_IDX_CE_EDT_ENABLE			( 17UL )

#define MCVX_DPAR_NOEL						( 32UL )

/* VLC error_block */
#define MCVX_VLC_ERR_BLOCK_NONE				( 0UL )
#define MCVX_VLC_ERR_BLOCK_VLCS				( 1UL )
#define MCVX_VLC_ERR_BLOCK_STX				( 2UL )
#define MCVX_VLC_ERR_BLOCK_MVD				( 3UL )

/* VLC error_level */
#define MCVX_VLC_ERR_LEVEL_NONE				( 0UL )
#define MCVX_VLC_ERR_LEVEL_WARNING			( 1UL )
#define MCVX_VLC_ERR_LEVEL_ERROR			( 2UL )
#define MCVX_VLC_ERR_LEVEL_FATAL			( 3UL )
#define MCVX_VLC_ERR_LEVEL_HUNGUP			( 0xffffffffUL )

/* VLC error_code */
#define MCVX_VLC_ERR_CODE_NONE				( 0UL )

/* CE error_code */
#define MCVX_CE_ERR_CODE_NONE				( 0UL )
#define MCVX_CE_ERR_CODE_ES_ROUND			( 0x20UL )
#define MCVX_CE_ERR_CODE_HUNGUP				( 0xffffffffUL )

/* number of elements */
#define MCVX_IMS_NOEL						( 2UL )
#define MCVX_ES_NOEL						( 2UL )
#define MCVX_DEC_MVR_NOEL					( 16UL )
#define MCVX_ENC_CP_NOEL					( 2UL )
#define MCVX_ENC_DP_COEF_NOEL				( 2UL )

/*===== call back functions =====*/
typedef MCVX_U32 (*MCVX_UDF_V_TO_P_T) (	
	void			*udp,  
	MCVX_U32		v_addr );

typedef void (*MCVX_UDF_SHOW_BAA_T) (
	void			*udp,
	MCVX_U32		module,
	void			*baa );

typedef void (*MCVX_UDF_VLC_EVENT_T) (	
	void			*udp,  
	MCVX_U32		vlc_event,
	void			*vlc_event_data );

typedef void (*MCVX_UDF_CE_EVENT_T) (	
	void			*udp,
	MCVX_U32		ce_event,
	void			*ce_event_data );

typedef void (*MCVX_UDF_FCV_EVENT_T) (	
	void			*udp,
	MCVX_U32		fcv_event,
	void			*fcv_event_data );

typedef void (*MCVX_UDF_REG_READ_T)	(	
	volatile MCVX_REG	*reg_addr, 
	MCVX_U32			*dst_addr, 
	MCVX_U32			num_reg );

typedef void (*MCVX_UDF_REG_WRITE_T) (	
	volatile MCVX_REG	*reg_addr, 
	MCVX_U32			*src_addr, 
	MCVX_U32			num_reg );

/*===== structures =====*/
/**
 *  @struct MCVX_CONTEXT_T
 *  @brief  context
******************************************************/
typedef struct {
	MCVX_U32				context_status;
	void					*cmn_info;
	void					*codec_info;
	void					*codec_ext;
	void					*udp;
} MCVX_CONTEXT_T;

/**
 *  @struct MCVX_CE_BAA_T
 *  @brief  CE base address array structure
******************************************************/
typedef struct {
	MCVX_U32				irp_v_addr;
	MCVX_U32				irp_p_addr;

	MCVX_U32				imc_buff_addr;
	MCVX_UBYTES				imc_buff_size;
	MCVX_U32				ims_buff_addr[ MCVX_IMS_NOEL ];
	MCVX_UBYTES				ims_buff_size[ MCVX_IMS_NOEL ];

	MCVX_U32				lm_ce_mbi_addr;
	MCVX_U32				lm_ce_prd_addr;
	MCVX_U32				lm_ce_ovt_addr;
	MCVX_U32				lm_ce_deb_addr;

	MCVX_U32				img_decY_addr;
	MCVX_U32				img_decC_addr;
	MCVX_U32				img_decY_bot_addr;
	MCVX_U32				img_decC_bot_addr;

	/* for deocoder */
	MCVX_U32				img_fltY_addr;
	MCVX_U32				img_fltC_addr;
	MCVX_U32				img_fltY_bot_addr;
	MCVX_U32				img_fltC_bot_addr;

	/* for encoder */
	MCVX_U32				img_encY_addr;
	MCVX_U32				img_encC_addr[ MCVX_ENC_CP_NOEL ];	/* [0]2pl or cb for 3pl [1]cr for 3pl */

	MCVX_U32				img_ref_num;
	MCVX_U32				img_refY_addr[ MCVX_MAX_NUM_REF_PIC ];
	MCVX_U32				img_refC_addr[ MCVX_MAX_NUM_REF_PIC ];

	MCVX_U32				stride_dec;
	MCVX_U32				stride_ref;
	MCVX_U32				stride_flt;
	MCVX_U32				stride_enc_y;
	MCVX_U32				stride_enc_c;

	/* for encoder */
	MCVX_U32				enc_mv_w_addr;
	MCVX_U32				enc_mv_r_addr;
} MCVX_CE_BAA_T;

/**
 *  @struct MCVX_VLC_BAA_T
 *  @brief  VLC base address array structure
******************************************************/
typedef struct {
	MCVX_U32				irp_v_addr;
	MCVX_U32				irp_p_addr;
	MCVX_U32				list_item_v_addr;
	MCVX_U32				list_item_p_addr;

	MCVX_U32				imc_buff_addr;
	MCVX_UBYTES				imc_buff_size;
	MCVX_U32				ims_buff_addr[ MCVX_IMS_NOEL ];
	MCVX_UBYTES				ims_buff_size[ MCVX_IMS_NOEL ];

	MCVX_U32				lm_vlc_mbi_addr;

	MCVX_U32				str_es_addr[ MCVX_ES_NOEL ];
	MCVX_UBYTES				str_es_size[ MCVX_ES_NOEL ];
	MCVX_U32				str_es_bit_offset;

	/* for dec */
	MCVX_U32				dec_dp_addr;
	MCVX_U32				dec_bp_addr;
	MCVX_U32				dec_prob_r_addr;
	MCVX_U32				dec_prob_w_addr;
	MCVX_U32				dec_segm_addr;
	MCVX_U32				dec_mai_addr;
	MCVX_U32				dec_mv_w_addr;
	MCVX_U32				dec_mv_r_addr[ MCVX_DEC_MVR_NOEL ];

	/* for enc */
	MCVX_U32				enc_dp_coef_addr[ MCVX_ENC_DP_COEF_NOEL ];
	MCVX_U32				enc_dp_coef_size[ MCVX_ENC_DP_COEF_NOEL ];
	MCVX_U32				enc_prob_base_addr;
	MCVX_U32				enc_prob_update_addr;
	MCVX_U32				enc_stat_addr;
} MCVX_VLC_BAA_T;

/**
 *  @struct MCVX_IP_CONFIG_T
 *  @brief  CODEC-IP configurations
******************************************************/
typedef struct {
	MCVX_U32				ip_id;
	MCVX_U32				ip_vlc_base_addr;
	MCVX_U32				ip_ce_base_addr;
	MCVX_UDF_REG_READ_T		udf_reg_read;
	MCVX_UDF_REG_WRITE_T	udf_reg_write;
} MCVX_IP_CONFIG_T;

/**
 *  @struct MCVX_IP_INFO_T
 *  @brief  CODEC-IP information
******************************************************/
typedef struct {
	MCVX_U32				ip_id;
	MCVX_UDF_REG_READ_T		udf_reg_read;
	MCVX_UDF_REG_WRITE_T	udf_reg_write;

	/* REG */
	volatile MCVX_REG_VLC_T	*REG_VLC;
	volatile MCVX_REG_CE_T	*REG_CE;

	/* VLC */
	MCVX_U32				hw_vlc_state;
	void					*vlc_handled_udp;
	MCVX_UDF_VLC_EVENT_T	udf_vlc_event;

	/* CE */
	MCVX_U32				hw_ce_state;
	void					*ce_handled_udp;
	MCVX_UDF_CE_EVENT_T		udf_ce_event;

	/* FCV */
	MCVX_U32				hw_fcv_state;
	void					*fcv_handled_udp;
	MCVX_UDF_FCV_EVENT_T	udf_fcv_event;

	MCVX_U32				vlc_hung_up;
	MCVX_U32				ce_hung_up;

} MCVX_IP_INFO_T;

/**
 *  @struct MCVX_VLC_EXE_T
 *  @brief  VLC execution parameters
******************************************************/
typedef struct {
	MCVX_REG				vp_vlc_cmd;
	MCVX_REG				vp_vlc_ctrl;
	MCVX_REG				vp_vlc_edt;
	MCVX_REG				vp_vlc_irq_enb;
	MCVX_REG				vp_vlc_clk_stop;

	MCVX_REG				vp_vlc_list_init;
	MCVX_REG				vp_vlc_list_en;
	MCVX_REG				vp_vlc_list_lden;

	MCVX_REG				vp_vlc_pbah;
	MCVX_REG				vp_vlc_pbal;

	MCVX_UBYTES				irp_size;
} MCVX_VLC_EXE_T;

/**
 *  @struct MCVX_VLC_RES_T
 *  @brief  VLC result parameters
******************************************************/
typedef struct {
	MCVX_U32				error_block;
	MCVX_U32				error_level;
	MCVX_U32				error_code;
	MCVX_U32				error_pos_x;
	MCVX_U32				error_pos_y;

	MCVX_UBYTES				pic_bytes;
	MCVX_UBYTES				is_byte;
	MCVX_U32				codec_info;
	MCVX_U32				vlc_cycle;
} MCVX_VLC_RES_T;

/**
 *  @struct MCVX_CE_EXE_T
 *  @brief  CE execution parameters
******************************************************/
typedef struct {
	MCVX_REG				vp_ce_cmd;
	MCVX_REG				vp_ce_ctrl;
	MCVX_REG				vp_ce_edt;
	MCVX_REG				vp_ce_irq_enb;
	MCVX_REG				vp_ce_clk_stop;

	MCVX_REG				vp_ce_list_init;
	MCVX_REG				vp_ce_list_en;

	MCVX_REG				vp_ce_pbah;
	MCVX_REG				vp_ce_pbal;

	MCVX_UBYTES				irp_size;
} MCVX_CE_EXE_T;

/**
 *  @struct MCVX_CE_RES_T
 *  @brief  CE result parameters
******************************************************/
typedef struct {
	MCVX_U32				error_code;
	MCVX_U32				error_pos_y;
	MCVX_U32				error_pos_x;

	MCVX_U32				ref_baa_log;
	MCVX_U32				is_byte;
	MCVX_U32				intra_mbs;
	MCVX_U32				sum_of_min_sad256;
	MCVX_U32				sum_of_intra_sad256;
	MCVX_U32				sum_of_inter_sad256;
	MCVX_U32				sum_of_act;
	MCVX_U32				act_min;
	MCVX_U32				act_max;
	MCVX_U32				sum_of_qp;
	MCVX_U32				ce_cycle;
} MCVX_CE_RES_T;

/**
 *  @struct MCVX_FCV_EXE_T
 *  @brief  FCV execution parameters
******************************************************/
typedef struct {
	MCVX_REG				vp_ce_cmd;
	MCVX_REG				vp_ce_ctrl;
	MCVX_REG				vp_ce_irq_enb;
	MCVX_REG				vp_ce_edt;

	MCVX_REG				vp_ce_fcv_picsize;
	MCVX_REG				vp_ce_fcv_inctrl;
	MCVX_REG				vp_ce_fcv_outctrl;

	MCVX_REG				vp_ce_fcv_dchr_yr;
	MCVX_REG				vp_ce_fcv_dchr_cr;
	MCVX_REG				vp_ce_fcv_dchr_yw;
	MCVX_REG				vp_ce_fcv_dchr_cw;

	MCVX_REG				vp_ce_fcv_ba_yr;
	MCVX_REG				vp_ce_fcv_ba_cr;
	MCVX_REG				vp_ce_fcv_ba_yw;
	MCVX_REG				vp_ce_fcv_ba_c0w;
	MCVX_REG				vp_ce_fcv_ba_c1w;

	MCVX_REG				vp_ce_fcv_stride_r;
	MCVX_REG				vp_ce_fcv_stride_w;

	MCVX_REG				vp_ce_fcv_ba_yrh;
	MCVX_REG				vp_ce_fcv_ba_crh;
	MCVX_REG				vp_ce_fcv_ba_ywh;
	MCVX_REG				vp_ce_fcv_ba_c0wh;

	MCVX_REG				vp_ce_list_init;
	MCVX_REG				vp_ce_list_en;
	MCVX_REG				vp_ce_pbah;
	MCVX_REG				vp_ce_pbal;
} MCVX_FCV_EXE_T;

/**
 *  @struct MCVX_FCV_RES_T
 *  @brief  FCV result parameters
******************************************************/
typedef struct {
	MCVX_U32				reserved;
} MCVX_FCV_RES_T;

/**
 *  @struct MCVX_IND_T
 *  @brief  indirect register data
******************************************************/
typedef struct {
	MCVX_U32				data_ll;	/* bit[ 31:  0]  */
	MCVX_U32				data_lh;	/* bit[ 63: 32]  */
	MCVX_U32				data_hl;	/* bit[ 95: 64]  */
	MCVX_U32				data_hh;	/* bit[127: 96]  */
} MCVX_IND_T;

/*===== functions =====*/
/* API function */
MCVX_RC mcvx_ip_init( 
	MCVX_IP_CONFIG_T	*ip_config,
	MCVX_IP_INFO_T		*ip );

MCVX_RC mcvx_get_ip_version( 
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		*ip_version );

void mcvx_vlc_interrupt_handler( MCVX_IP_INFO_T *ip );

void mcvx_ce_interrupt_handler( MCVX_IP_INFO_T *ip );

MCVX_RC mcvx_vlc_reset( 
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		reset_mode );

MCVX_RC mcvx_ce_reset( 
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		reset_mode );

MCVX_RC mcvx_vlc_start(
	MCVX_IP_INFO_T			*ip,
	void					*udp,
	MCVX_UDF_VLC_EVENT_T	udf_vlc_event,
	MCVX_VLC_EXE_T			*vlc_exe );

MCVX_RC mcvx_vlc_stop( 
	MCVX_IP_INFO_T	*ip,
	MCVX_VLC_RES_T	*vlc_res );

MCVX_RC mcvx_ce_start( 
	MCVX_IP_INFO_T			*ip,
	void					*udp,
	MCVX_UDF_CE_EVENT_T		udf_ce_event,
	MCVX_CE_EXE_T			*ce_exe );

MCVX_RC mcvx_ce_stop(
	MCVX_IP_INFO_T	*ip,
	MCVX_CE_RES_T	*ce_res );

MCVX_RC mcvx_fcv_start( 
	MCVX_IP_INFO_T			*ip,
	void					*udp,
	MCVX_UDF_FCV_EVENT_T	udf_fcv_event,
	MCVX_FCV_EXE_T			*fcv_exe );

MCVX_RC mcvx_fcv_stop(
	MCVX_IP_INFO_T	*ip,
	MCVX_FCV_RES_T	*fcv_res );

void mcvx_ce_ind_write( 
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		eid,
	MCVX_U32		ce_w_addr,
	MCVX_IND_T		*ind );

/* For debug */
void mcvx_ce_ind_read( 
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		eid,
	MCVX_U32		ce_r_addr,
	MCVX_IND_T		*ind );

#ifdef __cplusplus
}
#endif

#endif /* MCVX_API_H */
