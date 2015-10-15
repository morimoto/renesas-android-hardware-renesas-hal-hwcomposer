/*************************************************************************/ /*
 imgpctrl_ctrl_dma.c
   imgpctrl ctrl_dma  function file

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


#include "imgpctrl_private.h"
#include "imgpctrl_common.h"
#include "imgpctrl_ctrl_vsp.h"

#include <linux/dmaengine.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

/* imgpctrl_dma_init						*/
/* return value : bool true					*/
/* argument : chan : dma_channel				*/
/*            param not used					*/
/* Description: DMA filter setting				*/
/* This is sample program					*/
static bool imgpctrl_dma_filter(struct dma_chan *chan, void *param)
{

	struct platform_device *pdev = to_platform_device(chan->device->dev);

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	chan->private = NULL;

	if ((pdev->id == 2) || (pdev->id == 3)) {
		MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
		return true;
	}

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);
	return false;
}


/* imgpctrl_dma_init						*/
/* return value : stat	IMGPCTRL_R_OK				*/
/*			IMGPCTRL_R_NG				*/
/* argument : ctrl_hdl  -- IMGPCTRL control handle information	*/
/*            dma_info  --  dma_information			*/
/* Description: DMA						*/
long imgpctrl_dma_init(struct t_dma_info *dma_info)
{
	dma_cap_mask_t mask;
	struct dma_chan		*channel;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);


	channel = dma_request_channel(mask, imgpctrl_dma_filter,
					    &(dma_info->slave));


	if (channel == NULL) {
		MSG_ERROR("[%s] ERR: dma request channel is null.Line:%d\n",
			__func__, __LINE__);

		return IMGPCTRL_R_NG;
	}
	dma_info->chan = channel;
	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);

	return IMGPCTRL_R_OK;

}


/* imgpctr_dma_async_memory_buf_to_buf				*/
/* return value : cookie					*/
/* argument : chan -- DMA channel to offload copy to		*/
/*            dest -- destination address (virtual)		*/
/*	      src  --  source address (virtual)			*/
/*            len  --  length					*/
/*            ct_hdl -- handle information			*/
/* Description: This function is base on			*/
/*		dma_async_memcpy_buf_to_buf of dmaengine.c.	*/
/*		We want to use callback setting, therefore	*/
/*		we change the code				*/
/*		Plase refer dmaengine.c				*/
dma_cookie_t
imgpctrl_dma_async_memcpy_buf_to_buf(struct dma_chan *chan, void *dest,
			void *src, size_t len, struct t_ctrl_hdl *ct_hdl)
{
	struct dma_device *dev = chan->device;
	struct dma_async_tx_descriptor *tx;
	dma_addr_t dma_dest, dma_src;
	dma_cookie_t cookie;
	unsigned long flags;

	dma_src = dma_map_single(dev->dev, src, len, DMA_TO_DEVICE);
	dma_dest = dma_map_single(dev->dev, dest, len, DMA_FROM_DEVICE);
	flags = DMA_CTRL_ACK |
		DMA_COMPL_SRC_UNMAP_SINGLE |
		DMA_COMPL_DEST_UNMAP_SINGLE;
	tx = dev->device_prep_dma_memcpy(chan, dma_dest, dma_src, len, flags);

	if (tx == NULL) {
		dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
		dma_unmap_single(dev->dev, dma_dest, len, DMA_FROM_DEVICE);
		return -ENOMEM;
	}

	tx->callback = imgpctrl_dma_callback;
	tx->callback_param = ct_hdl;
	cookie = tx->tx_submit(tx);


	preempt_disable();
	__this_cpu_add(chan->local->bytes_transferred, len);
	__this_cpu_inc(chan->local->memcpy_count);
	preempt_enable();

	return cookie;
}



/* imgpctrl_ctrl_exe_dma -					*/
/* return value : stat	IMGPCTRL_R_OK				*/
/*			IMGPCTRL_R_NG				*/
/*			IMGPCTRL_R_PARA_ERR			*/
/*			IMGPCTRL_R_SEQ_ERR			*/
/*                      IMGPCTRL_R_VSPM_INIT_NG			*/
/*                      IMGPCTRL_R_VSPM_ENTRY_NG		*/
/* argument : ctrl_hdl  -- IMGPCTRL control handle information  */
/*            exe_info --  control execute information		*/
/* Description: DMA setting function				*/

long imgpctrl_ctrl_exe_dma(u_long ctrl_hdl, struct t_exe_info *exe_info)
{
	long err;
	long ret = IMGPCTRL_R_OK;
	u_long  jobid = IMGPCTRL_INVALID_JOBID;
	struct t_ctrl_hdl *ct_hdl;
	struct t_ctrl_hdl_vsp *ct_vsp;
	struct screen_grap_handle *ghp;
	u_long debug_data = 0;

	struct screen_grap_image_param *in_image;
	struct screen_grap_image_param *out_image;
	void *src_buf_addr[IMGPCTRL_SYSDMAC_MAX];
	void *dest_buf_addr[IMGPCTRL_SYSDMAC_MAX];
	u_long offset;
	u_int  buf_size;
	u_int  buf_size_c;
	u_int  tmp_buf_size[IMGPCTRL_SYSDMAC_MAX];
	dma_cookie_t cookie[IMGPCTRL_SYSDMAC_MAX];
	struct t_dma_info *dma_info;
	FW_CTRL_CB fw_cb_func;
	int i;
	int use_chan_num_local = 0;
	struct dma_chan		*channel;

	MSG_TRACE("[%s] ST:in. Line:%d\n", __func__, __LINE__);

	if (ctrl_hdl == ((u_long)NULL)) {
		ret = IMGPCTRL_R_SEQ_ERR;
		MSG_ERROR("[%s] ERR: dma ctrl_hdl is null.Line:%d\n",
			__func__, __LINE__);
		return ret;
	}

	ct_hdl = (struct t_ctrl_hdl *)ctrl_hdl;
	ct_vsp = (struct t_ctrl_hdl_vsp *)ct_hdl->vspm_param;
	fw_cb_func = ct_hdl->fw_ctrl_cb_fp;


	ghp = ((struct screen_grap_handle *)(ct_hdl->udata));


	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, FUNC_IN, 0,
		(u_long *)NULL , ghp);

	if (ct_vsp == ((struct t_ctrl_hdl_vsp *)NULL)) {
		ret = IMGPCTRL_R_SEQ_ERR;
		MSG_ERROR("[%s] ERR: dma parameter is null.Line:%d\n",
			 __func__, __LINE__);
		debug_data = __LINE__;
		/* output error & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
		(u_short)debug_data ,
		ret , HDL_PTR_NULL, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ret;
	}
	if (exe_info == ((struct t_exe_info *)NULL)) {
		ret = IMGPCTRL_R_SEQ_ERR;
		MSG_ERROR(
		"[%s] ERR: exe info(dma) parameter is null.Line:%d\n",
			 __func__, __LINE__);
		debug_data = __LINE__;
		/* output trace & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
		(u_short)debug_data ,
		ret , PARAM_NULL, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ret;
	}


	/* Ready transfer */
	in_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->input_layer[0].image));
	out_image =
	 (struct screen_grap_image_param *)
	 (&(exe_info->output_layer[0].image));


	 if (in_image->format == RT_GRAPHICS_COLOR_YUV420PL)
		ghp->use_chan_num = 3;
	else
		ghp->use_chan_num = 2;

	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, DMA_INIT_BC, 0,
		(u_long *)NULL , ghp);

	for (i = 0; i < ghp->use_chan_num; i++) {
		dma_info = &(ghp->dma_info_p[i]);
		dma_info->slave.shdma_slave.slave_id = -1;
		err = imgpctrl_dma_init(dma_info);
		if (err == IMGPCTRL_R_OK)
			use_chan_num_local++;
		else
			MSG_ERROR("[%s] Line:%d err=%d\n",
				__func__, __LINE__, (int)err);
	}

	if (use_chan_num_local == 0) {	/* DMAC cannot allocate channel */
		ret = IMGPCTRL_R_NG;
		debug_data = __LINE__;
		/* output trace & trace log */
		IMGPCTRL_ERROR_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
		(u_short)debug_data ,
		ret , PARAM_NULL, ghp);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
		IMGPCTRL_GRAPHDL_LOG(exe_info, ghp);
		return ret;
	} else if ((ghp->use_chan_num == 3) && (use_chan_num_local == 2)) {
		/*if acqure channel num == 2 */
		/* release 2channel(1channel use) */
		channel = (ghp->dma_info_p[1].chan);
		dma_release_channel(channel);
		use_chan_num_local = 1;
	}


	IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, DMA_INIT_AC, 0,
		(u_long *)NULL , ghp);


	/* buffer calcutate */
	if (in_image->format == RT_GRAPHICS_COLOR_YUV422SP) {
		buf_size = ((in_image->stride) * (in_image->height));
		buf_size_c = ((in_image->stride_c) * (in_image->height));

		tmp_buf_size[0] = buf_size;
		tmp_buf_size[1] = buf_size_c;


		offset = (u_long)in_image->address;
		src_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)in_image->address_c0;
		src_buf_addr[1] = phys_to_virt(offset);

		offset = (u_long)out_image->address;
		dest_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)out_image->address_c0;
		dest_buf_addr[1] = phys_to_virt(offset);


	} else if ((in_image->format == RT_GRAPHICS_COLOR_YUV420SP) ||
		(in_image->format == RT_GRAPHICS_COLOR_YUV420SP_NV21)) {
		buf_size = ((in_image->stride) * (in_image->height));
		buf_size_c = ((in_image->stride_c) * ((in_image->height)/2));

		tmp_buf_size[0] = buf_size;
		tmp_buf_size[1] = buf_size_c;

		offset = (u_long)in_image->address;
		src_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)in_image->address_c0;
		src_buf_addr[1] = phys_to_virt(offset);

		offset = (u_long)out_image->address;
		dest_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)out_image->address_c0;
		dest_buf_addr[1] = phys_to_virt(offset);


	} else if (in_image->format == RT_GRAPHICS_COLOR_YUV420PL) {
		buf_size = ((in_image->stride) * (in_image->height));
		buf_size_c = ((in_image->stride_c) * ((in_image->height)/2));

		tmp_buf_size[0] = buf_size;
		tmp_buf_size[1] = buf_size_c;
		tmp_buf_size[2] = buf_size_c;

		offset = (u_long)in_image->address;
		src_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)in_image->address_c0;
		src_buf_addr[1] = phys_to_virt(offset);
		offset = (u_long)in_image->address_c1;
		src_buf_addr[2] = phys_to_virt(offset);

		offset = (u_long)out_image->address;
		dest_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)out_image->address_c0;
		dest_buf_addr[1] = phys_to_virt(offset);
		offset = (u_long)out_image->address_c1;
		dest_buf_addr[2] = phys_to_virt(offset);


	} else {
		buf_size = ((in_image->stride) * (in_image->height));

		/* address set */
		tmp_buf_size[0] = buf_size/2;
		tmp_buf_size[1] = tmp_buf_size[0];

		offset = (u_long)in_image->address;
		src_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)in_image->address + tmp_buf_size[0];
		src_buf_addr[1] = phys_to_virt(offset);

		offset = (u_long)out_image->address;
		dest_buf_addr[0] = phys_to_virt(offset);
		offset = (u_long)out_image->address + tmp_buf_size[0];
		dest_buf_addr[1] = phys_to_virt(offset);
	}

	for (i = 0; i < ghp->use_chan_num; i++) {

		if (ghp->use_chan_num == use_chan_num_local) {
			/* same request_channel == acquire_channel */
			dma_info = &(ghp->dma_info_p[i]);

			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
				DMA_MEM_BUF_BC, 0,
				(u_long *)NULL , ghp);

			cookie[i] = imgpctrl_dma_async_memcpy_buf_to_buf
				((dma_info->chan), dest_buf_addr[i],
				src_buf_addr[i], tmp_buf_size[i], ct_hdl);

			ghp->cookie[i] = cookie[i];


		} else {
			/* different request_channel == acquire_channel */

			dma_info = &(ghp->dma_info_p[0]);

			IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
				DMA_MEM_BUF_BC, 0,
				(u_long *)NULL , ghp);

			cookie[i] = imgpctrl_dma_async_memcpy_buf_to_buf
				((dma_info->chan), dest_buf_addr[i],
				src_buf_addr[i], tmp_buf_size[i], ct_hdl);

			ghp->cookie[0] = cookie[i];

		}

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
			DMA_MEM_BUF_AC, 0,
			(u_long *)NULL , ghp);

		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA,
			DMA_PEND_BC, 0,
			(u_long *)NULL , ghp);
		dma_async_issue_pending(dma_info->chan);
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, DMA_PEND_AC, 0,
			(u_long *)NULL , ghp);


		/* Reversed JobID */
		/* entry_num++ */
		ct_hdl->entry_ret[0] = ret;
		ct_hdl->entry_jobid[0] = jobid;
		ct_hdl->entry_num++;


	}
	ghp->use_chan_num = use_chan_num_local;

	MSG_TRACE("[%s] ST:out. Line:%d\n", __func__, __LINE__);

	if (ret == IMGPCTRL_R_OK) {
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, FUNC_OUT_NRM, 0,
			(u_long *)NULL , ghp);
	} else {
		debug_data = __LINE__;
		IMGPCTRL_TRACE_LOG(FID_IMGPCTRL_CTRL_EXE_DMA, FUNC_OUT_ERR, 1,
			(u_long *)&debug_data , ghp);
	}

	return ret;
}


