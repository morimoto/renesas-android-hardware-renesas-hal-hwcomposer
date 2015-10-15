/*
 * Function        : Composer driver
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

#include "inc/blend.h"
#include "inc/debug.h"

#include "imgpctrl/imgpctrl_public.h"


/******************************************************/
/* define prototype                                   */
/******************************************************/
static void notify_graphics_image_conv_dummy(
	int result, unsigned long user_data);
static void notify_graphics_image_blend(int result, unsigned long user_data);
static int config_update_address(struct screen_grap_image_param *image,
	unsigned long rtaddr);
static inline unsigned char *config_update_address_adjustoffset(
	unsigned char offset_base[],
	unsigned long rtaddr);
static int config_convert_rtaddr(phys_addr_t p_addr, unsigned long *rt_addr);

/******************************************************/
/* define local define                                */
/******************************************************/

/******************************************************/
/* define local variables                             */
/******************************************************/
/*! information for blend */
static struct blend_info blend_info;
/*! exclusive control for access blend_info. */
static DEFINE_SPINLOCK(lock_blend_info);

/******************************************************/
/* local functions                                    */
/******************************************************/

/******************************************************/
/*! \brief callback of screen_graphics_image_conv
 *  \param[in] result    result of processing.
 *  \param[in] user_data user data passed at request.
 *  \return none
 *  \details
 *  handle callback of screen_graphics_image_conv.\n
 */
static void notify_graphics_image_conv_dummy(
	int result __maybe_unused, unsigned long user_data __maybe_unused)
{
	/* currently not implemented. */
	printk_err1("callback.");

	if (0 != result) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}
	if (0 != user_data) {
		/* from static analysis tool message (3:3206) */
		/* issue un-necessary parameter check.        */

		/* nothing to do */
	}
}

/******************************************************/
/*! \brief callback of screen_graphics_image_blend
 *  \param[in] result    result of processing.
 *  \param[in] user_data user data passed at request.
 *  \return none
 *  \details
 *  handle callback of screen_graphics_image_blend.
 *  store operation result and wakeup waiting thread.
 */
static void notify_graphics_image_blend(int result, unsigned long user_data)
{
	DBGENTER("result:%d user_data:0x%lx\n", result, user_data);

	/* confirm result code. */
	{
		struct blend_registerinfo *info;
		int    status;
		void   *arg;
		void   (*callback)(int rc, void *arg);

		info = (struct blend_registerinfo *) user_data;

		if (result != SMAP_LIB_GRAPHICS_OK) {
			printk_err("notify_graphics_image_blend result:%d %s\n",
				result, get_imgpctrlmsg_graphics(result));
			info->status = RTAPI_NOTIFY_RESULT_ERROR;
		} else {
			info->status = RTAPI_NOTIFY_RESULT_NORMAL;
		}

		status   = info->status;
		arg      = info->arg;
		callback = info->callback;

		printk_dbg2(3, "status:%d callback:%p arg:%p\n",
			status, callback, arg);

		/* wakeup waiting task */
		wake_up_all(&info->wait);

		/* callback if necessary. */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL != callback) {
			/* issue callback */
			callback(status, arg);
		}
	}

	DBGLEAVE("\n");
}

/******************************************************/
/*! \brief create handle for blend
 *  \return handle for blend
 *  \retval 0 error
 *  \retval others handle for blend
 *  \details
 *  create handle to use screen_graphics_image_blend.
 */
static void *blend_create(void)
{
	void *graphic_handle;
	struct screen_grap_new _new;

	DBGENTER("\n");

	/* update for screen_grap_new */
	_new.notify_graphics_image_conv   = &notify_graphics_image_conv_dummy;
	_new.notify_graphics_image_blend  = &notify_graphics_image_blend;

	graphic_handle = screen_graphics_new(&_new);

	printk_dbg1(1, "screen_graphics_new result:%p\n",
		graphic_handle);

	DBGLEAVE("\n");
	return graphic_handle;
}

/******************************************************/
/*! \brief delete handle for blend
 *  \param[in] graphic_handle handle for blend
 *  \return none
 *  \details
 *  delete handle to use screen_graphics_image_blend.
 */
static void blend_delete(void *graphic_handle)
{
	DBGENTER("\n");

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */

	if (NULL != graphic_handle) {
		struct screen_grap_delete _del;

		printk_dbg1(1, "screen_graphics_delete %p\n", graphic_handle);

		_del.handle   = graphic_handle;

#if _LOG_DBG >= 1
		if (5 <= debug)
			dump_screen_grap_delete(&_del);
#endif

		screen_graphics_delete(&_del);
	}

	DBGLEAVE("\n");
	return;
}

/******************************************************/
/*! \brief request blend
 *  \param[in] graphic_handle handle for blend
 *  \param[in] blend     pointer to structure screen_grap_image_blend
 *  \param[in] user_data user data.
 *  \param[in] callback  callback function
 *  \return result of processing
 *  \retval CMP_OK success
 *  \retval CMP_NG error. not supported format.
 *  \details
 *  search unused blend_info and register blend request information.
 *  call screen_graphics_image_blend function and wait complete blending.
 *  if blend become error, call dump_screen_grap_image_blend()
 *  to print argument of blend.
 *  \msc
 *    composer, ipc;
 *    |||;
 *    --- [label="initialize"];
 *    composer=>ipc
 *        [label="create handle", URL="\ref ::blend_create"];
 *    ipc box ipc
 *        [label="screen_graphics_new"];
 *    ipc=>composer
 *        [label="handle for blend"];
 *    --- [label="blend"];
 *    composer box composer
 *        [label="resolve buffer's address"];
 *    composer box composer
 *        [label="configure parameters", URL="\ref ::blend_config"];
 *    composer=>ipc
 *        [label="request blend", URL="\ref ::blend_request"];
 *    ipc box ipc
 *        [label="screen_graphics_image_blend"];
 *    composer->composer
 *        [label="wait complete blending"];
 *    ipc box ipc
 *        [label="do blending"];
 *    ipc->composer
 *        [label="notice complete",  URL="\ref ::notify_graphics_image_blend"];
 *    --- [label="finalize"];
 *    composer=>ipc
 *        [label="delete handle", URL="\ref ::blend_delete"];
 *    ipc box ipc
 *        [label="screen_graphics_delete"];
 *  \endmsc
 */
static int blend_request(void *graphic_handle,
	struct screen_grap_image_blend *blend,
	void *user_data, void (*callback)(int rc, void *arg))
{
	int rc = CMP_NG;
	int ret_rc;

	size_t i;
	struct blend_registerinfo *info = NULL;

	DBGENTER("blend:%p user_data:%p callback:%p\n",
		blend, user_data, callback);

	/* check argument */

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == blend) {
		printk_dbg2(3, "arg blend invalid.\n");
		goto err_exit;
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL == graphic_handle) {
		printk_dbg2(3, "arg handle invalid.\n");
		goto err_exit;
	}

	/* select free slot */
	spin_lock(&lock_blend_info);
	for (i = 0; i < ARRAY_SIZE(blend_info.info); i++) {

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (NULL == blend_info.info[i].blend) {
			/* select free slot */
			printk_dbg2(3, "select %d\n", i);
			info = &blend_info.info[i];
			break;
		}
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (NULL != info) {
		info->blend = blend;
		info->arg   = user_data;
		info->callback = callback;
	} else {
		/* error_report */
		printk_err("no free slot\n");
	}
	spin_unlock(&lock_blend_info);

	/* request blending */
	if (NULL == info) {
		/* no space to free-blend_info */
		goto err_exit;
	}

	printk_dbg1(1, "blending handle:%p\n", graphic_handle);

	init_waitqueue_head(&info->wait);
	info->status  = RTAPI_NOTIFY_RESULT_UNDEFINED;

	blend->handle    = graphic_handle;
	blend->user_data = (unsigned long)info;

#if _LOG_DBG >= 1
	if (5 <= debug)
		dump_screen_grap_image_blend(blend);
#endif

	ret_rc = screen_graphics_image_blend(blend);
	if (ret_rc != SMAP_LIB_GRAPHICS_OK) {
		printk_err("screen_graphics_image_blend return by %d %s.\n",
			ret_rc, get_imgpctrlmsg_graphics(ret_rc));
#if _ERR_DBG >= 1 && _LOG_DBG >= 1
		dump_screen_grap_image_blend(blend);
#endif
		goto fin_exit;
	}

	/* wait complete */
	wait_event(
		info->wait,
		info->status != RTAPI_NOTIFY_RESULT_UNDEFINED);

	if (info->status == RTAPI_NOTIFY_RESULT_NORMAL) {
		/* image blend successed. */
		rc = CMP_OK;
	} else {
		printk_err1("callback result is not normal.\n");
#if _ERR_DBG >= 1 && _LOG_DBG >= 1
		dump_screen_grap_image_blend(blend);
#endif
	}
fin_exit:
	info->blend = NULL;

	blend->handle = NULL;

err_exit:

	DBGLEAVE("rc:%d\n", rc);
	return rc;
}

/******************************************************/
/*! \brief update address information
 *  \param[in] offset_base offset value of base pointer.
 *  \param[in] rtaddr      top address of this image
 *  \return pointer of buffer address
 *  \details
 *  update address of physical address uses offset.
 *  This function is work-aroung of static analysis
 *  tool message(4:0488).
 *  it is "Format that is allowed in pointer arithmetic,
 *  must only array indexing."
 */
static inline unsigned char *config_update_address_adjustoffset(
	unsigned char offset_base[],
	unsigned long rtaddr)
{
	return &offset_base[rtaddr];
}

/******************************************************/
/*! \brief update address information
 *  \param[in] image pointer to screen_grap_image_param.
 *  \param[in] rtaddr top address of this image
 *  \return result of processing
 *  \retval CMP_OK success
 *  \retval CMP_NG error. not supported format.
 *  \details
 *  update address of physical address uses in screen_graphics_image_blend.
 */
static int config_update_address(struct screen_grap_image_param *image,
	unsigned long rtaddr)
{
	unsigned int num_layer;
	int rc = CMP_OK;

	switch (image->format) {
	case RT_GRAPHICS_COLOR_YUV420PL:
		num_layer = 0b111;
		break;
	case RT_GRAPHICS_COLOR_YUV422SP:
	case RT_GRAPHICS_COLOR_YUV420SP:
	case RT_GRAPHICS_COLOR_YUV420SP_NV21:
		num_layer = 0b011;
		break;
	case RT_GRAPHICS_COLOR_RGB565:
	case RT_GRAPHICS_COLOR_RGB888:
	case RT_GRAPHICS_COLOR_ARGB8888:
	case RT_GRAPHICS_COLOR_XRGB8888:
	case RT_GRAPHICS_COLOR_ABGR8888:
	case RT_GRAPHICS_COLOR_XBGR8888:
	case RT_GRAPHICS_COLOR_YUV422I_UYVY:
		num_layer = 0b001;
		break;
	default:
		printk_err("not supported format\n");
		num_layer = 0b000;
		rc = CMP_NG;
		break;
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != (num_layer & 0b001)) {
		image->address = config_update_address_adjustoffset(
			image->address, rtaddr);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != (num_layer & 0b010)) {
		image->address_c0 = config_update_address_adjustoffset(
			image->address_c0, rtaddr);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (0 != (num_layer & 0b100)) {
		image->address_c1 = config_update_address_adjustoffset(
			image->address_c1, rtaddr);
	}

	/* from static analysis tool message (4:3344).         */
	/* Conditional expression require a boolean expression */
	if (CMP_OK != rc) {
		/* address set to NULL to be able to detect error */
		image->address = NULL;
	}
	return rc;
}

 /******************************************************/
/*! \brief convert and return rt-address
 *  \param[in]     p_addr   physical address.
 *  \param[out]    rt_addr  rt-address
 *  \return result of processing
 *  \retval CMP_OK success
 *  \retval CMP_NG error. p_addr is invalid
 *  \details
 *  convert physical address to rt-address.
 */
static int config_convert_rtaddr(phys_addr_t p_addr, unsigned long *rt_addr)
{
	int result = CMP_OK;

	if (0 == p_addr) {
		printk_err2("physical address %llx invalid.",
			(uint64_t) p_addr);

		*rt_addr = 0;
		result = CMP_NG;
	} else {
		/* rt_addr is same as physical address */
		*rt_addr = (unsigned long)p_addr;

#if 0
/* this check is not work, because buffer_address not phys_addr */
#ifdef CONFIG_ARM_LPAE
		/* check physical address is less than FFFFFFFF */
		if ((phys_addr_t)(*rt_addr) != p_addr) {
			printk_err2("physical address %llx invalid.",
				(uint64_t) p_addr);
			result = CMP_NG;
		}
#endif
#endif
	}

	return result;
}

/******************************************************/
/*! \brief configure blend parameters
 *  \param[in]     rh   pointer to structure composer_rh
 *  \param[in,out] post pointer to structure cmp_postdata
 *  \return result of processing
 *  \retval CMP_OK success
 *  \retval CMP_NG error. invalid config
 *  \details
 *  configure blend parameter of screen_graphics_image_blend.
 *  if physical address of used buffer is not unknown or
 *  config_update_address() return error, then this function return error.
 */
static int  blend_config(struct composer_rh *rh, struct cmp_postdata *post)
{
	struct cmp_data_compose_blend *data = &rh->data;
	int                         rc = CMP_NG;

	DBGENTER("rh:%p post:%p\n", rh, post);

	data->valid = false;

	if (post->operation_type != 0 && post->operation_type != 1) {
		/* unknown operation type ignored */
		rc = CMP_OK;
	} else if (post->num_buffer <= 0) {
		/* not supported.   */
		rc = CMP_OK;
	} else {
		int i;
		struct screen_grap_image_blend *blend = &data->blend;
		struct screen_grap_layer       *layer;
		unsigned long rtaddr;

		/* handle blend information */
		if (post->num_buffer > CMP_DATA_NUM_GRAP_LAYER) {
			printk_err2("buffer num %d invalid.\n",
				post->num_buffer);
			goto finish;
		}

		/* clear information */

		/* static analysis tool message (4:3200):           */
		/* in order to make it clear that did not check     */
		/* the return value of memset, cast to void         */
		(void) memset(data, 0, sizeof(*data));

		/* create blend parameter */
		blend->output_image     = post->buffer[0].image;
		blend->background_color = post->bgcolor;


		/* buffer_index for output_image not necessary */

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (CMP_OK != config_convert_rtaddr(
			(phys_addr_t)rh->buffer_address[0], &rtaddr)) {
			printk_err2("output address invalid.\n");
			goto finish;
		}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
		if (CMP_OK != config_update_address(
			&blend->output_image, rtaddr)) {
			printk_err2("address invalid.\n");
			goto finish;
		}

		for (i = 1; i < post->num_buffer; i++) {
			layer = &data->layer[i-1];
			blend->input_layer[i-1] = layer;

			*layer = post->buffer[i];

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			if (CMP_OK != config_convert_rtaddr(
				(phys_addr_t)rh->buffer_address[i], &rtaddr)) {
				printk_err2("address invalid.\n");
				goto finish;
			}

		/* from static analysis tool message (4:3344).         */
		/* Conditional expression require a boolean expression */
			if (CMP_OK != config_update_address(
				&layer->image, rtaddr)) {
				printk_err2("address invalid.\n");
				goto finish;
			}
		}

		/* set blend type */
		if (post->operation_type) {
			/* operation type is limit.  */
			blend->mode = RT_GRAPHICS_ALLGPU_MODE;
		} else {
			/* operation type is normal. */
			blend->mode = RT_GRAPHICS_NORMAL_MODE;
		}

		data->valid = true;
		rc = CMP_OK;
	}
finish:

	DBGLEAVE("%d\n", rc);
	return rc;
}
