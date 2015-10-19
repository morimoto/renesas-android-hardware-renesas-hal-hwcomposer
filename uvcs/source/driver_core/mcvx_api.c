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

#ifdef __RENESAS__
#pragma section MCVX
#endif /* __RENESAS__ */

#include "mcvx_api.h"

/**
 *
 * Function Name : mcvx_ip_init
 *
 * @brief			Initialize hardware IP
 *
 * @param[in]		ip_config				: IP config information
 * @param[out]		ip						: IP context
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 * @retval			MCVX_ERR_SEVERE			: severe error
 *
******************************************************************************/
MCVX_RC mcvx_ip_init(
	MCVX_IP_CONFIG_T	*ip_config,
	MCVX_IP_INFO_T		*ip)
{
	MCVX_RC				rtn_code;
	MCVX_REG			reg_data;

	if (ip_config == NULL) {
		rtn_code				= MCVX_ERR_PARAM;
	} else if (ip == NULL) {
		rtn_code				= MCVX_ERR_PARAM;
	} else if ((ip_config->udf_reg_read  == NULL) || (ip_config->udf_reg_write == NULL)) {
		rtn_code				= MCVX_ERR_PARAM;
	} else {
		ip->vlc_handled_udp		= NULL;
		ip->ce_handled_udp		= NULL;
		ip->fcv_handled_udp		= NULL;

		ip->udf_vlc_event		= NULL;
		ip->udf_ce_event		= NULL;
		ip->udf_fcv_event		= NULL;

		ip->hw_vlc_state		= MCVX_STATE_VLC_IDLE;
		ip->hw_ce_state			= MCVX_STATE_CE_IDLE;
		ip->hw_fcv_state		= MCVX_STATE_FCV_IDLE;

		ip->ip_id				= ip_config->ip_id;
		ip->udf_reg_read		= ip_config->udf_reg_read;
		ip->udf_reg_write		= ip_config->udf_reg_write;

		ip->REG_VLC				= (MCVX_REG_VLC_T *)ip_config->ip_vlc_base_addr;
		ip->REG_CE				= (MCVX_REG_CE_T *)ip_config->ip_ce_base_addr;

		ip->vlc_hung_up			= MCVX_FALSE;
		ip->ce_hung_up			= MCVX_FALSE;

		/* check VLC base address by VCID */
		reg_data					= 0UL;
		ip->udf_reg_read(&ip->REG_VLC->VP_VLC_VCRH, &reg_data, MCVX_REGS_SINGLE);
		if ((reg_data & MCVX_M_16BIT) != MCVX_VCID) {
			rtn_code				= MCVX_ERR_SEVERE;
		} else {
			/* check CE base address by VCID */
			reg_data				= 0UL;
			ip->udf_reg_read(&ip->REG_CE->VP_CE_VCRH, &reg_data, MCVX_REGS_SINGLE);
			if ((reg_data & MCVX_M_16BIT) != MCVX_VCID) {
				rtn_code			= MCVX_ERR_SEVERE;
			} else { /* VLC base address and CE base address are ok */

				/* clear interrupt related register begin */
				reg_data			= 0UL;
				/* VLC */
				ip->udf_reg_write(&ip->REG_VLC->VP_VLC_IRQ_ENB, &reg_data, MCVX_REGS_SINGLE);
				ip->udf_reg_write(&ip->REG_VLC->VP_VLC_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);

				/* CE */
				ip->udf_reg_write(&ip->REG_CE->VP_CE_IRQ_ENB, &reg_data, MCVX_REGS_SINGLE);
				ip->udf_reg_write(&ip->REG_CE->VP_CE_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);
				/* clear interrupt related register end */

				/* reset vlc */ /* rtn_code can be ignored */
				(void)mcvx_vlc_reset(ip, MCVX_CMD_VLC_RESET_MODULE);

				/* reset ce */  /* rtn_code can be ignored */
				(void)mcvx_ce_reset(ip, MCVX_CMD_CE_RESET_MODULE);

				rtn_code = MCVX_NML_END;
			}
		}
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_get_ip_version
 *
 * @brief			Get IP version
 *
 * @param[in]		ip						: IP context
 * @param[out]		ip_version				: IP version (from VP_VLC_VCRL register)
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_get_ip_version(
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		*ip_version)
{
	MCVX_RC			rtn_code;

	if ((ip == NULL) || (ip_version == NULL)) {
		rtn_code	= MCVX_ERR_PARAM;
	} else {
		/*----------------------*/
		/* VP_VLC_VCRL          */
		/*----------------------*/
		ip->udf_reg_read(&ip->REG_VLC->VP_VLC_VCRL, ip_version, MCVX_REGS_SINGLE);
		rtn_code	= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_vlc_interrupt_handler
 *
 * @brief			VLC interrupt handler
 *
 * @param[in]		ip						: IP context
 *
 * @retval			void					: none
 *
******************************************************************************/
void mcvx_vlc_interrupt_handler(MCVX_IP_INFO_T *ip)
{
	MCVX_REG		reg_data;
	MCVX_REG		edt_reset;

	edt_reset		= 0UL;

	if (ip != NULL) {
		/*----------------------*/
		/* VP_VLC_IRQ_STA       */
		/*----------------------*/
		/* read IRQ status */
		ip->udf_reg_read(&ip->REG_VLC->VP_VLC_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);

		/*----------------------*/
		/* VP_VLC_EDT           */
		/*----------------------*/
		/* cleaer EDT counter */
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_EDT, &edt_reset, MCVX_REGS_SINGLE);

		/*-----------------------------------------------------------------------------------------------------------------------*/
		/* Note that the VL (VLCS termination) bit and VED (VLCS system error detection) bit might be asserted at the same time. */
		/* In this case the VED flag can be ignored and software can treat as a normal termination.                              */
		/*-----------------------------------------------------------------------------------------------------------------------*/
		if ((reg_data & MCVX_VLC_PROC_END_EVENT) != MCVX_NO_EVENT) { /* VLC end */
			/* nomarl termination */
			ip->vlc_hung_up	= MCVX_FALSE;

			if (ip->udf_vlc_event != NULL) { /* fail safe */
				ip->udf_vlc_event(ip->vlc_handled_udp, MCVX_EVENT_REQ_VLC_STOP, &reg_data);
			}
		} else if ((reg_data & MCVX_VLC_HUNG_END_EVENT) != MCVX_NO_EVENT) {	/* VLC error detect */
			/* error termination */
			ip->vlc_hung_up	= MCVX_TRUE;

			if (ip->udf_vlc_event != NULL) { /* fail safe */
				ip->udf_vlc_event(ip->vlc_handled_udp, MCVX_EVENT_REQ_VLC_STOP, &reg_data);
			}

			/* VLC module reset */
			(void)mcvx_vlc_reset(ip, MCVX_CMD_VLC_RESET_MODULE);
		} else {
			/* Nothing to do */
		}

		/*----------------------*/
		/* VP_VLC_IRQ_STA       */
		/*----------------------*/
		if (reg_data != MCVX_NO_EVENT) {
			/* clear IRQ status */
			reg_data	= ~reg_data; /* 0:clear, 1:stay */

			/* clear interrupt status */
			ip->udf_reg_write(&ip->REG_VLC->VP_VLC_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);

			/* dummy read */
			ip->udf_reg_read(&ip->REG_VLC->VP_VLC_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);
		}
	}
}

/**
 *
 * Function Name : mcvx_ce_interrupt_handler
 *
 * @brief			CE interrupt handler
 *
 * @param[in]		ip						: IP context
 *
 * @retval			void					: none
 *
******************************************************************************/
void mcvx_ce_interrupt_handler(MCVX_IP_INFO_T *ip)
{
	MCVX_REG		reg_data;
	MCVX_REG		edt_reset;

	edt_reset		= 0UL;

	if (ip != NULL) {
		/*----------------------*/
		/* VP_CE_IRQ_STA        */
		/*----------------------*/
		/* read IRQ status */
		ip->udf_reg_read(&ip->REG_CE->VP_CE_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);

		/*----------------------*/
		/* VP_CE_EDT            */
		/*----------------------*/
		/* cleaer EDT counter */
		ip->udf_reg_write(&ip->REG_CE->VP_CE_EDT, &edt_reset, MCVX_REGS_SINGLE);

		/*-----------------------------------------------------------------------------------------------------------------------*/
		/* Note that the CE (CE termination) bit and CED (CE system error detection) bit might be asserted at the same time.     */
		/* In this case the CED flag can be ignored and software can treat as a normal termination.                              */
		/*-----------------------------------------------------------------------------------------------------------------------*/
		if ((reg_data & MCVX_CE_PROC_END_EVENT) != MCVX_NO_EVENT) {	/* CE end */
			/* nomarl termination */
			ip->ce_hung_up	= MCVX_FALSE;

			if (ip->udf_ce_event != NULL) { /* fail safe */
				ip->udf_ce_event(ip->ce_handled_udp, MCVX_EVENT_REQ_CE_STOP, &reg_data);
			}
		} else if ((reg_data & MCVX_CE_HUNG_END_EVENT) != MCVX_NO_EVENT) {	/* CE error detect */
			/* error termination */
			ip->ce_hung_up	= MCVX_TRUE;

			if (ip->udf_ce_event != NULL) { /* fail safe */
				ip->udf_ce_event(ip->ce_handled_udp, MCVX_EVENT_REQ_CE_STOP, &reg_data);
			}

			/* CE module reset */
			(void)mcvx_ce_reset(ip, MCVX_CMD_CE_RESET_MODULE);
		} else {
			/* Nothing to do */
		}

		/* FCV end */
		if ((reg_data & MCVX_FCV_PROC_END_EVENT) != MCVX_NO_EVENT) {
			if (ip->udf_fcv_event != NULL) {	/* fail safe */
				ip->udf_fcv_event(ip->fcv_handled_udp, MCVX_EVENT_REQ_FCV_STOP, &reg_data);
			}
		}

		/*----------------------*/
		/* VP_CE_IRQ_STA        */
		/*----------------------*/
		if (reg_data != MCVX_NO_EVENT) {
			/* clear IRQ status */
			reg_data	= ~reg_data; /* 0:clear, 1:stay */

			/* clear interrupt status */
			ip->udf_reg_write(&ip->REG_CE->VP_CE_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);

			/* dummy read */
			ip->udf_reg_read(&ip->REG_CE->VP_CE_IRQ_STA, &reg_data, MCVX_REGS_SINGLE);
		}
	}
}

/**
 *
 * Function Name : mcvx_vlc_reset
 *
 * @brief			Reset VLC
 *
 * @param[in]		ip						: IP context
 * @param[in]		reset_mode				: reset mode
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_vlc_reset(
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		reset_mode)
{
	MCVX_REG		reg_data;
	MCVX_RC			rtn_code;

	if (ip == NULL) {
		rtn_code	= MCVX_ERR_PARAM;
	} else {
		reg_data	= reset_mode;

		/* reset */
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_CMD, &reg_data, MCVX_REGS_SINGLE);

		/* dummy read */
		ip->udf_reg_read(&ip->REG_VLC->VP_VLC_STATUS, &reg_data, MCVX_REGS_SINGLE);

		/*----------------------*/
		/* VP_VLC_CLK_STOP      */
		/*----------------------*/
		reg_data	= 0UL;
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_CLK_STOP, &reg_data, MCVX_REGS_SINGLE);

		rtn_code	= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_ce_reset
 *
 * @brief			Reset CE
 *
 * @param[in]		ip						: IP context
 * @param[in]		reset_mode				: reset mode
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_ce_reset(
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		reset_mode)
{
	MCVX_REG		reg_data;
	MCVX_RC			rtn_code;

	if (ip == NULL) {
		rtn_code	= MCVX_ERR_PARAM;
	} else {
		reg_data	= reset_mode;

		/* reset */
		ip->udf_reg_write(&ip->REG_CE->VP_CE_CMD, &reg_data, MCVX_REGS_SINGLE);

		/* dummy read */
		ip->udf_reg_read(&ip->REG_CE->VP_CE_STATUS, &reg_data, MCVX_REGS_SINGLE);

		/*----------------------*/
		/* VP_CE_CLK_STOP       */
		/*----------------------*/
		reg_data	= 0UL;
		ip->udf_reg_write(&ip->REG_CE->VP_CE_CLK_STOP, &reg_data, MCVX_REGS_SINGLE);

		rtn_code	= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_vlc_start
 *
 * @brief			Start VLC
 *
 * @param[in]		ip						: IP context
 * @param[in]		udp						: user define pointer
 * @param[in]		udf_vlc_event			: VLC event callback function
 * @param[in]		vlc_exe					: VLC execute parameter
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_vlc_start(
	MCVX_IP_INFO_T			*ip,
	void					*udp,
	MCVX_UDF_VLC_EVENT_T	udf_vlc_event,
	MCVX_VLC_EXE_T			*vlc_exe)
{
	MCVX_REG		reg_data_table[MCVX_REG_TABLE_SIZE];
	MCVX_RC			rtn_code;

	if ((ip == NULL) || (udf_vlc_event == NULL) || (vlc_exe == NULL)) {
		rtn_code			= MCVX_ERR_PARAM;
	} else {
		/* VLC module reset (VLC only) */
		(void)mcvx_vlc_reset(ip, MCVX_CMD_VLC_RESET_MODULE);

		ip->vlc_hung_up		= MCVX_FALSE;
		ip->vlc_handled_udp	= udp;
		ip->udf_vlc_event	= udf_vlc_event;

		reg_data_table[0]	= vlc_exe->vp_vlc_list_init;
		reg_data_table[1]	= vlc_exe->vp_vlc_list_en;
		reg_data_table[2]	= vlc_exe->vp_vlc_list_lden;
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_LIST_INIT,	reg_data_table, 3UL);

		reg_data_table[0]	= vlc_exe->vp_vlc_pbah;
		reg_data_table[1]	= vlc_exe->vp_vlc_pbal;
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_PBAH,		reg_data_table, 2UL);

		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_EDT,			&vlc_exe->vp_vlc_edt,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_IRQ_ENB,		&vlc_exe->vp_vlc_irq_enb,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_CLK_STOP,	&vlc_exe->vp_vlc_clk_stop,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_CTRL,		&vlc_exe->vp_vlc_ctrl,		MCVX_REGS_SINGLE);

		/* call back */
		ip->udf_vlc_event(ip->vlc_handled_udp, MCVX_EVENT_VLC_START, NULL);

		/* update HW-VLC-state */
		ip->hw_vlc_state	= MCVX_STATE_VLC_RUNNING;

		ip->udf_reg_write(&ip->REG_VLC->VP_VLC_CMD, &vlc_exe->vp_vlc_cmd, MCVX_REGS_SINGLE);

		rtn_code			= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_vlc_stop
 *
 * @brief			Get VLC result
 *
 * @param[in]		ip						: IP context
 * @param[out]		vlc_res					: VLC-result information
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_vlc_stop(
	MCVX_IP_INFO_T	*ip,
	MCVX_VLC_RES_T	*vlc_res)
{
	MCVX_REG		reg_data_table[MCVX_REG_TABLE_SIZE];
	MCVX_RC			rtn_code;

	if ((ip == NULL) || (vlc_res == NULL)) {
		rtn_code	= MCVX_ERR_PARAM;
	} else {
		/* read register */
		ip->udf_reg_read(&ip->REG_VLC->VP_VLC_TB, reg_data_table, MCVX_REGS_RES_VLC);

		vlc_res->pic_bytes 			= reg_data_table[0];
		/* reg_data_table[1] VP_VLC_ERR0H (ignored) */
		/* reg_data_table[2] VP_VLC_ERR0L (ignored) */
		vlc_res->is_byte	 		= reg_data_table[3];
		/* reg_data_table[4] resereved */
		vlc_res->codec_info			= reg_data_table[5];

		/* highest error info */
		/* VP_VLC_ERR1H */
		vlc_res->error_block		= MCVX_M_02BIT & (reg_data_table[6] >> 29);
		if (ip->vlc_hung_up == MCVX_FALSE) {
			vlc_res->error_level	= MCVX_M_02BIT & (reg_data_table[6] >> 27);
		} else {
			vlc_res->error_level	= MCVX_VLC_ERR_LEVEL_HUNGUP;
		}
		vlc_res->error_pos_x 		= MCVX_M_12BIT & (reg_data_table[6] >> 12);
		vlc_res->error_pos_y 		= MCVX_M_12BIT & (reg_data_table[6] >>  0);
		/* VP_VLC_ERR1L */
		vlc_res->error_code 		= MCVX_M_08BIT & (reg_data_table[7] >> 24);

		ip->udf_reg_read(&ip->REG_VLC->VP_VLC_CNT, reg_data_table, 1);
		vlc_res->vlc_cycle			= reg_data_table[0];

		/* update HW-VLC-state */
		ip->hw_vlc_state			= MCVX_STATE_VLC_IDLE;
		ip->vlc_hung_up				= MCVX_FALSE; /* clear */

		rtn_code					= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_ce_start
 *
 * @brief			Start CE
 *
 * @param[in]		ip						: IP context
 * @param[in]		udp						: user define pointer
 * @param[in]		udf_ce_event			: CE event callback function
 * @param[in]		ce_exe					: CE execute parameter
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_ce_start(
	MCVX_IP_INFO_T			*ip,
	void					*udp,
	MCVX_UDF_CE_EVENT_T		udf_ce_event,
	MCVX_CE_EXE_T			*ce_exe)
{
	MCVX_REG		reg_data_table[MCVX_REG_TABLE_SIZE];
	MCVX_RC			rtn_code;

	if ((ip == NULL) || (udf_ce_event == NULL) || (ce_exe == NULL)) {
		rtn_code			= MCVX_ERR_PARAM;
	} else {
		ip->ce_hung_up		= MCVX_FALSE;
		ip->ce_handled_udp	= udp;
		ip->udf_ce_event	= udf_ce_event;

		reg_data_table[0]	= ce_exe->vp_ce_list_init;
		reg_data_table[1]	= ce_exe->vp_ce_list_en;
		ip->udf_reg_write(&ip->REG_CE->VP_CE_LIST_INIT,	reg_data_table, 2UL);

		reg_data_table[0]	= ce_exe->vp_ce_pbah;
		reg_data_table[1]	= ce_exe->vp_ce_pbal;
		ip->udf_reg_write(&ip->REG_CE->VP_CE_PBAH,			reg_data_table, 2UL);

		ip->udf_reg_write(&ip->REG_CE->VP_CE_EDT,			&ce_exe->vp_ce_edt,			MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_IRQ_ENB,		&ce_exe->vp_ce_irq_enb,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_CLK_STOP,		&ce_exe->vp_ce_clk_stop,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_CTRL,			&ce_exe->vp_ce_ctrl,		MCVX_REGS_SINGLE);

		/*----------------------*/
		/* VP_CE_CMD            */
		/*----------------------*/
		/* call back */
		ip->udf_ce_event(ip->ce_handled_udp, MCVX_EVENT_CE_START, NULL);

		/* update HW-CE-state */
		ip->hw_ce_state		= MCVX_STATE_CE_RUNNING;

		ip->udf_reg_write(&ip->REG_CE->VP_CE_CMD, &ce_exe->vp_ce_cmd, MCVX_REGS_SINGLE);

		rtn_code			= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_ce_stop
 *
 * @brief			Get CE result
 *
 * @param[in]		ip						: IP context
 * @param[out]		ce_res					: CE-result information
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_ce_stop(
	MCVX_IP_INFO_T	*ip,
	MCVX_CE_RES_T	*ce_res)
{
	MCVX_REG		reg_data_table[MCVX_REG_TABLE_SIZE];
	MCVX_RC			rtn_code;

	if ((ip == NULL) || (ce_res == NULL)) {
		rtn_code					= MCVX_ERR_PARAM;
	} else {
		/* read register */
		ip->udf_reg_read(&ip->REG_CE->VP_CE_REF_LOG, reg_data_table, 1UL);
		ce_res->ref_baa_log			= reg_data_table[0];

		/* read register */
		ip->udf_reg_read(&ip->REG_CE->VP_CE_ERRH, reg_data_table, 5UL);
		if (ip->ce_hung_up == MCVX_FALSE) {
			ce_res->error_code		= MCVX_M_08BIT & (reg_data_table[0] >> 24);
		} else {
			ce_res->error_code		= MCVX_CE_ERR_CODE_HUNGUP;
		}
		ce_res->error_pos_y			= MCVX_M_12BIT & (reg_data_table[0] >> 12);
		ce_res->error_pos_x			= MCVX_M_12BIT & (reg_data_table[0] >>  0);
		ce_res->is_byte 			= reg_data_table[2];
		/* reg_data_table[3] reserved  */
		ce_res->intra_mbs 			= reg_data_table[4];

		/* read register */
		ip->udf_reg_read(&ip->REG_CE->VP_CE_QP_SUM, reg_data_table, 9UL);
		ce_res->sum_of_qp			= MCVX_M_22BIT & (reg_data_table[0] >>  0);
		/* reg_data_table[1] reserved  */
		ce_res->sum_of_act			= MCVX_M_24BIT & (reg_data_table[2] >>  0);
		ce_res->act_max				= MCVX_M_10BIT & (reg_data_table[3] >> 16);
		ce_res->act_min				= MCVX_M_10BIT & (reg_data_table[3] >>  0);
		/* reg_data_table[4] reserved  */
		/* reg_data_table[5] reserved  */
		/* reg_data_table[6] reserved  */
		ce_res->sum_of_min_sad256	= reg_data_table[6];
		ce_res->sum_of_intra_sad256	= reg_data_table[7];
		ce_res->sum_of_inter_sad256	= reg_data_table[8];

		ip->udf_reg_read(&ip->REG_CE->VP_CE_CNT, reg_data_table, 1);
		ce_res->ce_cycle			= reg_data_table[0];

		/* update HW-CE-state */
		ip->hw_ce_state				= MCVX_STATE_CE_IDLE;
		ip->ce_hung_up				= MCVX_FALSE; /* clear */

		rtn_code					= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_fcv_start
 *
 * @brief			Start FCV(in CE)
 *
 * @param[in]		ip						: IP context
 * @param[in]		udp						: user define pointer
 * @param[in]		udf_fcv_event			: FCV event callback function
 * @param[in]		fcv_exe					: FCV execute parameter
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_fcv_start(
	MCVX_IP_INFO_T			*ip,
	void					*udp,
	MCVX_UDF_FCV_EVENT_T	udf_fcv_event,
	MCVX_FCV_EXE_T			*fcv_exe)
{
	MCVX_RC			rtn_code;
	MCVX_IND_T		ind;

	if ((ip == NULL) || (udf_fcv_event == NULL) || (fcv_exe == NULL)) {
		rtn_code			= MCVX_ERR_PARAM;
	} else {
		ip->fcv_handled_udp	= udp;
		ip->udf_fcv_event	= udf_fcv_event;

		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_YRH,	&fcv_exe->vp_ce_fcv_ba_yrh,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_YR,		&fcv_exe->vp_ce_fcv_ba_yr,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_CRH,	&fcv_exe->vp_ce_fcv_ba_crh,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_CR,		&fcv_exe->vp_ce_fcv_ba_cr,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_YWH,	&fcv_exe->vp_ce_fcv_ba_ywh,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_YW,		&fcv_exe->vp_ce_fcv_ba_yw,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_C0WH,	&fcv_exe->vp_ce_fcv_ba_c0wh,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_C0W,	&fcv_exe->vp_ce_fcv_ba_c0w,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_BA_C1W,	&fcv_exe->vp_ce_fcv_ba_c1w,		MCVX_REGS_SINGLE);

		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_DCHR_YR,	&fcv_exe->vp_ce_fcv_dchr_yr,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_DCHR_CR,	&fcv_exe->vp_ce_fcv_dchr_cr,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_DCHR_YW,	&fcv_exe->vp_ce_fcv_dchr_yw,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_DCHR_CW,	&fcv_exe->vp_ce_fcv_dchr_cw,	MCVX_REGS_SINGLE);

		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_PICSIZE,	&fcv_exe->vp_ce_fcv_picsize,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_INCTRL,	&fcv_exe->vp_ce_fcv_inctrl,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_FCV_OUTCTRL,	&fcv_exe->vp_ce_fcv_outctrl,	MCVX_REGS_SINGLE);

		ip->udf_reg_write(&ip->REG_CE->VP_CE_LIST_INIT,		&fcv_exe->vp_ce_list_init,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_LIST_EN,		&fcv_exe->vp_ce_list_en,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_PBAH,			&fcv_exe->vp_ce_pbah,			MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_PBAL,			&fcv_exe->vp_ce_pbal,			MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_IRQ_ENB,		&fcv_exe->vp_ce_irq_enb,		MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_CTRL,			&fcv_exe->vp_ce_ctrl,			MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_EDT,			&fcv_exe->vp_ce_edt,			MCVX_REGS_SINGLE);

		{
			/* indirect register write */
			ind.data_hh		= 0x00000000UL; /* data_hh should be 0 */
			ind.data_hl		= 0x00000000UL; /* data_hl should be 0 */

			ind.data_lh		= fcv_exe->vp_ce_fcv_stride_r;
			ind.data_ll		= fcv_exe->vp_ce_fcv_ba_yr;
			mcvx_ce_ind_write(ip, 0x08, 0x00AB, &ind);

			ind.data_lh		= fcv_exe->vp_ce_fcv_stride_r;
			ind.data_ll		= fcv_exe->vp_ce_fcv_ba_cr;
			mcvx_ce_ind_write(ip, 0x08, 0x00AC, &ind);

			ind.data_lh		= fcv_exe->vp_ce_fcv_stride_w;
			ind.data_ll		= fcv_exe->vp_ce_fcv_ba_yw;
			mcvx_ce_ind_write(ip, 0x08, 0x00AD, &ind);

			ind.data_lh		= fcv_exe->vp_ce_fcv_stride_w;
			ind.data_ll		= fcv_exe->vp_ce_fcv_ba_c0w;
			mcvx_ce_ind_write(ip, 0x08, 0x00AE, &ind);

			ind.data_lh		= fcv_exe->vp_ce_fcv_stride_w;
			ind.data_ll		= fcv_exe->vp_ce_fcv_ba_c1w;
			mcvx_ce_ind_write(ip, 0x08, 0x00AF, &ind);
		}

		/*----------------------*/
		/* VP_CE_CMD            */
		/*----------------------*/
		/* call back */
		ip->udf_fcv_event(ip->fcv_handled_udp, MCVX_EVENT_FCV_START, NULL);

		/* update HW-FCV-state */
		ip->hw_fcv_state	= MCVX_STATE_FCV_RUNNING;

		ip->udf_reg_write(&ip->REG_CE->VP_CE_CMD, &fcv_exe->vp_ce_cmd, MCVX_REGS_SINGLE);

		rtn_code			= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_fcv_stop
 *
 * @brief			Get FCV result
 *
 * @param[in]		ip						: IP context
 * @param[out]		fcv_res					: FCV-result information
 *
 * @retval			MCVX_NML_END			: normal end
 * @retval			MCVX_ERR_PARAM			: parameter error
 *
******************************************************************************/
MCVX_RC mcvx_fcv_stop(
	MCVX_IP_INFO_T	*ip,
	MCVX_FCV_RES_T	*fcv_res)
{
	MCVX_RC			rtn_code;

	if ((ip == NULL) || (fcv_res == NULL)) {
		rtn_code			= MCVX_ERR_PARAM;
	} else {
		/* reserved (there is no fcv result information) */
		fcv_res->reserved	= 0UL;

		/* update HW-FCV-state */
		ip->hw_fcv_state	= MCVX_STATE_FCV_IDLE;

		rtn_code			= MCVX_NML_END;
	}

	return rtn_code;
}

/**
 *
 * Function Name : mcvx_ce_ind_write
 *
 * @brief			Write 128bit value into CE internal register
 *
 * @param[in]		ip						: IP context
 * @param[in]		eid						: element ID
 * @param[in]		ce_w_addr				: CE internal write address (16bit)
 * @param[in]		ind						: write data
 *
 * @retval			void					: none
 *
******************************************************************************/
void mcvx_ce_ind_write(
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		eid,
	MCVX_U32		ce_w_addr,
	MCVX_IND_T		*ind)
{
	MCVX_REG		reg_data;

	if ((ip != NULL) && (ind != NULL)) {
		reg_data	= 0x80000000UL | ((MCVX_M_06BIT & eid) << 16) | ((MCVX_M_16BIT & ce_w_addr) << 0);

		ip->udf_reg_write(&ip->REG_CE->VP_CE_IND_WA,	&reg_data,		MCVX_REGS_SINGLE);

		/* write 128bit */
		ip->udf_reg_write(&ip->REG_CE->VP_CE_IND_WDLL, &ind->data_ll,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_IND_WDLH, &ind->data_lh,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_IND_WDHL, &ind->data_hl,	MCVX_REGS_SINGLE);
		ip->udf_reg_write(&ip->REG_CE->VP_CE_IND_WDHH, &ind->data_hh,	MCVX_REGS_SINGLE);	/* should be set at last */
	}
}

/**
 *
 * Function Name : mcvx_ce_ind_read
 *
 * @brief			Read 128bit value from CE internal register
 *
 * @param[in]		ip						: IP context
 * @param[in]		eid						: element ID
 * @param[in]		ce_r_addr				: CE internal read address (16bit)
 * @param[in]		ind						: read data
 *
 * @retval			void					: none
 *
******************************************************************************/
void mcvx_ce_ind_read(
	MCVX_IP_INFO_T	*ip,
	MCVX_U32		eid,
	MCVX_U32		ce_r_addr,
	MCVX_IND_T		*ind)
{
	MCVX_REG		reg_data;

	if ((ip != NULL) && (ind != NULL)) {
		reg_data	= 0x80000000UL | ((MCVX_M_06BIT & eid) << 16) | ((MCVX_M_16BIT & ce_r_addr) << 0);

		ip->udf_reg_write(&ip->REG_CE->VP_CE_IND_RA,	&reg_data,		MCVX_REGS_SINGLE);	/* should be set at first */

		/* read 128bit */
		ip->udf_reg_read(&ip->REG_CE->VP_CE_IND_RDLL, &ind->data_ll,	MCVX_REGS_SINGLE);
		ip->udf_reg_read(&ip->REG_CE->VP_CE_IND_RDLH, &ind->data_lh,	MCVX_REGS_SINGLE);
		ip->udf_reg_read(&ip->REG_CE->VP_CE_IND_RDHL, &ind->data_hl,	MCVX_REGS_SINGLE);
		ip->udf_reg_read(&ip->REG_CE->VP_CE_IND_RDHH, &ind->data_hh,	MCVX_REGS_SINGLE);
	}
}
