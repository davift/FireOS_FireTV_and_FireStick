/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include "audio_messenger_ipi.h"

#include <linux/spinlock.h>

#ifdef CONFIG_MTK_AUDIODSP_SUPPORT
#include <adsp_ipi.h>
#endif

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
#include <adsp_ipi.h>
#include "adsp_ipi_platform.h"
#endif

#include <adsp_ipi_queue.h>

#include "audio_log.h"
#include "audio_assert.h"

#include "audio_task.h"
#include "audio_ipi_queue.h"
#include "audio_ipi_platform.h"

#ifdef CONFIG_SND_SOC_MTK_AUDIO_DSP
#include <audio_playback_msg_id.h>
#endif




/*
 * =============================================================================
 *                     log
 * =============================================================================
 */

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "[IPI][MSG] %s(), " fmt "\n", __func__



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#ifdef ipi_dbg
#undef ipi_dbg
#endif

#if 0
#define ipi_dbg(x...) pr_info(x)
#else
#define ipi_dbg(x...)
#endif

/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

recv_message_t recv_message_array[TASK_SCENE_SIZE];


/*
 * =============================================================================
 *                     private functions - declaration
 * =============================================================================
 */

/* queue related */
static void audio_ipi_msg_dispatcher(int id,
				     void *data,
				     unsigned int len);
static uint16_t current_idx;

/*
 * =============================================================================
 *                     utilities
 * =============================================================================
 */

uint16_t get_message_buf_size(const struct ipi_msg_t *p_ipi_msg)
{
	if (p_ipi_msg->data_type == AUDIO_IPI_MSG_ONLY)
		return IPI_MSG_HEADER_SIZE;
	else if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD)
		return (IPI_MSG_HEADER_SIZE + p_ipi_msg->payload_size);
	else if (p_ipi_msg->data_type == AUDIO_IPI_DMA)
		return (IPI_MSG_HEADER_SIZE + IPI_MSG_DMA_INFO_SIZE);
	else
		return 0;
}


int check_msg_format(const struct ipi_msg_t *p_ipi_msg, unsigned int len)
{
	if (p_ipi_msg->magic != IPI_MSG_MAGIC_NUMBER) {
		pr_notice("magic 0x%x error!!", p_ipi_msg->magic);
		return -1;
	}

	if (p_ipi_msg->task_scene >= TASK_SCENE_SIZE) {
		pr_notice("task_scene %d error!!", p_ipi_msg->task_scene);
		return -1;
	}

	if (p_ipi_msg->source_layer >= AUDIO_IPI_LAYER_FROM_SIZE) {
		pr_notice("source_layer %d error!!", p_ipi_msg->source_layer);
		return -1;
	}

	if (p_ipi_msg->target_layer >= AUDIO_IPI_LAYER_TO_SIZE) {
		pr_notice("target_layer %d error!!", p_ipi_msg->target_layer);
		return -1;
	}

	if (p_ipi_msg->data_type >= AUDIO_IPI_TYPE_SIZE) {
		pr_notice("data_type %d error!!", p_ipi_msg->data_type);
		return -1;
	}

	if (p_ipi_msg->ack_type > AUDIO_IPI_MSG_DIRECT_SEND &&
	    p_ipi_msg->ack_type != AUDIO_IPI_MSG_CANCELED) {
		pr_notice("ack_type %d error!!", p_ipi_msg->ack_type);
		return -1;
	}

	if (get_message_buf_size(p_ipi_msg) > len) {
		pr_notice("len 0x%x error!!", len);
		return -1;
	}

	if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
		if (p_ipi_msg->payload_size == 0 ||
		    p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
			DUMP_IPI_MSG("payload_size error!!", p_ipi_msg);
			return -1;
		}
	}

	if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
		if (p_ipi_msg->dma_addr == NULL) {
			DUMP_IPI_MSG("dma addr null!!", p_ipi_msg);
			return -1;
		}
		if (p_ipi_msg->dma_info.data_size == 0 &&
		    p_ipi_msg->dma_info.hal_buf.data_size == 0) {
			DUMP_IPI_MSG("dma data_size error!!", p_ipi_msg);
			return -1;
		}
	}

	return 0;
}


bool check_print_msg_info(const struct ipi_msg_t *p_ipi_msg)
{
	if (p_ipi_msg == NULL)
		return false;

#if 0 /* dont dump ack messages by default */
	if (p_ipi_msg->ack_type == AUDIO_IPI_MSG_NEED_ACK ||
	    p_ipi_msg->ack_type == AUDIO_IPI_MSG_ACK_BACK)
		return true;
#endif
	return false;
}



/*
 * =============================================================================
 *                     private functions - implementation
 * =============================================================================
 */

static void audio_ipi_msg_dispatcher(int id,
				     void *data,
				     unsigned int len)
{
	struct ipi_msg_t *p_ipi_msg = NULL;
	struct ipi_queue_handler_t *handler = NULL;

	AUD_LOG_V("data = %p, len = %u", data, len);

	if (data == NULL) {
		pr_info("drop msg due to data = NULL");
		return;
	}
	if (len < IPI_MSG_HEADER_SIZE || len > MAX_IPI_MSG_BUF_SIZE) {
		pr_info("drop msg due to len(%u) error!!", len);
		return;
	}

	p_ipi_msg = (struct ipi_msg_t *)data;
	if (check_msg_format(p_ipi_msg, len) != 0) {
		pr_info("drop msg due to ipi fmt err");
		return;
	}

	if (p_ipi_msg->ack_type == AUDIO_IPI_MSG_ACK_BACK) {
		if (check_print_msg_info(p_ipi_msg) == true)
			DUMP_IPI_MSG("ack back", p_ipi_msg);
		handler = get_ipi_queue_handler(p_ipi_msg->task_scene);
		if (handler != NULL)
			send_message_ack(handler, p_ipi_msg);
	} else if (p_ipi_msg->data_type == AUDIO_IPI_DMA &&
		   p_ipi_msg->target_layer == AUDIO_IPI_LAYER_TO_HAL)
		audio_ipi_dma_msg_to_hal(p_ipi_msg);
	else {
		if (recv_message_array[p_ipi_msg->task_scene] == NULL)
			DUMP_IPI_MSG("task not reg cbk!!", p_ipi_msg);
		else
			recv_message_array[p_ipi_msg->task_scene](p_ipi_msg);
	}
}



/*
 * =============================================================================
 *                     public functions - implementation
 * =============================================================================
 */

void audio_messenger_ipi_init(void)
{
	int i = 0;
#ifdef CONFIG_MTK_AUDIODSP_SUPPORT
	int ret_adsp = 0;
#endif
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	int ret_adsp = 0;
#endif

	current_idx = 0;

#ifdef CONFIG_MTK_AUDIODSP_SUPPORT
	ret_adsp = adsp_ipi_registration(
			   ADSP_IPI_AUDIO,
			   audio_ipi_msg_dispatcher,
			   "audio");
	if (ret_adsp != 0)
		pr_notice("adsp_ipi_registration fail!!");
#endif

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	ret_adsp = adsp_ipi_registration(
			   ADSP_IPI_AUDIO,
			   audio_ipi_msg_dispatcher,
			   "audio");
	if (ret_adsp != 0)
		pr_notice("adsp_ipi_registration fail!!");
#endif

	for (i = 0; i < TASK_SCENE_SIZE; i++)
		recv_message_array[i] = NULL;
}


void audio_reg_recv_message(uint8_t task_scene, recv_message_t recv_message)
{
	if (task_scene >= TASK_SCENE_SIZE) {
		pr_info("not support task_scene %d!!", task_scene);
		return;
	}

	recv_message_array[task_scene] = recv_message;
}


int audio_send_ipi_msg(
	struct ipi_msg_t *p_ipi_msg,
	uint8_t task_scene, /* task_scene_t */
	uint8_t target_layer, /* audio_ipi_msg_target_layer_t */
	uint8_t data_type, /* audio_ipi_msg_data_t */
	uint8_t ack_type, /* audio_ipi_msg_ack_t */
	uint16_t msg_id,
	uint32_t param1, /* data_size for payload & dma */
	uint32_t param2,
	void    *data_buffer) /* buffer for payload & dma */
{
	struct ipi_queue_handler_t *handler = NULL;
	struct ipi_msg_dma_info_t *dma_info = NULL;

	uint32_t ipi_msg_len = 0;
	int ret = 0;

	if (p_ipi_msg == NULL) {
		pr_notice("p_ipi_msg = NULL, return");
		return -1;
	}

	if (target_layer != AUDIO_IPI_LAYER_TO_DSP) {
		pr_notice("target_layer %d in kernel", target_layer);
		return -1;
	}


	memset(p_ipi_msg, 0, MAX_IPI_MSG_BUF_SIZE);

	p_ipi_msg->magic        = IPI_MSG_MAGIC_NUMBER;
	p_ipi_msg->task_scene   = task_scene;
	p_ipi_msg->source_layer = AUDIO_IPI_LAYER_FROM_KERNEL;
	p_ipi_msg->target_layer = target_layer;
	p_ipi_msg->data_type    = data_type;
	p_ipi_msg->ack_type     = ack_type;
	p_ipi_msg->msg_id       = msg_id;
	p_ipi_msg->param1       = param1;
	p_ipi_msg->param2       = param2;

	if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
		if (data_buffer == NULL) {
			pr_notice("payload data_buffer NULL, return");
			return -1;
		}
		if (p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
			pr_notice("payload_size %u error!!",
				  p_ipi_msg->payload_size);
			return -1;
		}

		memcpy(p_ipi_msg->payload,
		       data_buffer,
		       p_ipi_msg->payload_size);
	} else if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
		if (data_buffer == NULL) {
			pr_notice("dma data_buffer NULL, return");
			return -1;
		}
		p_ipi_msg->dma_addr = (char *)data_buffer;

		if (param1 > 1) {
			dma_info = &p_ipi_msg->dma_info;
			dma_info->data_size = param1;

			ret = audio_ipi_dma_write_region(
				      p_ipi_msg->task_scene,
				      data_buffer,
				      dma_info->data_size,
				      &dma_info->rw_idx);
		}
	}

	if (ret != 0) {
		DUMP_IPI_MSG("dma fail!!", p_ipi_msg);
		return ret;
	}

	ipi_msg_len = get_message_buf_size(p_ipi_msg);

	if (check_msg_format(p_ipi_msg, ipi_msg_len) != 0) {
		pr_info("drop msg due to ipi fmt err");
		return -1;
	}

	handler = get_ipi_queue_handler(p_ipi_msg->task_scene);
	if (handler == NULL) {
		pr_notice("handler = NULL, return");
		return -1;
	}

	return send_message(handler, p_ipi_msg);
}


int audio_send_ipi_filled_msg(struct ipi_msg_t *p_ipi_msg)
{
	struct ipi_queue_handler_t *handler = NULL;

	if (p_ipi_msg == NULL) {
		pr_notice("p_ipi_msg = NULL, return");
		return -1;
	}
	if (check_msg_format(p_ipi_msg, get_message_buf_size(p_ipi_msg)) != 0) {
		pr_info("drop msg due to ipi fmt err");
		return -1;
	}

	handler = get_ipi_queue_handler(p_ipi_msg->task_scene);
	if (handler == NULL) {
		AUD_LOG_E("handler = NULL, return");
		return -1;
	}

	return send_message(handler, p_ipi_msg);
}


int send_message_to_scp(const struct ipi_msg_t *p_ipi_msg)
{
	int send_status = 0;
	uint32_t wait_ms = 0;

	uint32_t opendsp_id = 0;
	uint32_t ipi_id = 0;


	/* error handling */
	if (p_ipi_msg == NULL) {
		pr_notice("p_ipi_msg = NULL, return");
		return -1;
	}

	/* wait until IPC done */
	wait_ms = (p_ipi_msg->ack_type == AUDIO_IPI_MSG_DIRECT_SEND)
		  ? 0
		  : ADSP_IPI_QUEUE_DEFAULT_WAIT_MS;

	opendsp_id = audio_get_opendsp_id(p_ipi_msg->task_scene);
	ipi_id = audio_get_ipi_id(p_ipi_msg->task_scene);

	if (opendsp_id == AUDIO_OPENDSP_USE_HIFI4)
		opendsp_id = MT8570_ADSP_ID;

	send_status = scp_send_msg_to_queue(
			      opendsp_id,
			      ipi_id,
			      (void *)p_ipi_msg,
			      get_message_buf_size(p_ipi_msg),
			      wait_ms);

	if (send_status != 0) {
		pr_notice("scp_ipi_send error %d", send_status);
		DUMP_IPI_MSG("fail", p_ipi_msg);
	}

	return send_status;
}


