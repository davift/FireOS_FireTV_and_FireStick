/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#ifndef __LD_BTMTK_USB_H__
#define __LD_BTMTK_USB_H__

#include <mtk-bt/LD_usbbt.h>

/* Memory map for MTK BT */
//#if 0
/* SYS Control */
#define SYSCTL	0x400000

/* WLAN */
#define WLAN		0x410000

/* MCUCTL */
#define CLOCK_CTL		0x0708
#define INT_LEVEL		0x0718
#define COM_REG0		0x0730
#define SEMAPHORE_00	0x07B0
#define SEMAPHORE_01	0x07B4
#define SEMAPHORE_02	0x07B8
#define SEMAPHORE_03	0x07BC

/* Chip definition */

#define CONTROL_TIMEOUT_JIFFIES		(300)
#define DEVICE_VENDOR_REQUEST_OUT	0x40
#define DEVICE_VENDOR_REQUEST_IN	0xc0
#define DEVICE_CLASS_REQUEST_OUT	0x20
#define DEVICE_CLASS_REQUEST_IN	    0xa0

#define BTUSB_MAX_ISOC_FRAMES	10
#define BTUSB_INTR_RUNNING	0
#define BTUSB_BULK_RUNNING	1
#define BTUSB_ISOC_RUNNING	2
#define BTUSB_SUSPENDING	3
#define BTUSB_DID_ISO_RESUME	4

/* ROM Patch */
#define PATCH_HCI_HEADER_SIZE 4
#define PATCH_WMT_HEADER_SIZE 5
#define PATCH_HEADER_SIZE (PATCH_HCI_HEADER_SIZE + PATCH_WMT_HEADER_SIZE)
#define UPLOAD_PATCH_UNIT 2048
#define PATCH_INFO_SIZE 30
#define PATCH_PHASE1 1
#define PATCH_PHASE2 2
#define PATCH_PHASE3 3
#define PATCH_LEN_ILM (192 * 1024)

/**
 * 0: patch download is not complete/BT get patch semaphore fail (WiFi get semaphore success)
 * 1: patch download is complete
 * 2: patch download is not complete/BT get patch semaphore success
 */
#define PATCH_ERR -1
#define PATCH_IS_DOWNLOAD_BY_OTHER 0
#define PATCH_READY 1
#define PATCH_NEED_DOWNLOAD 2

#define LD_BT_MAX_EVENT_SIZE 260
#define BD_ADDR_LEN 6

#define WAKE_DEV_RECORD         "wake_on_ble.conf"
#define WAKE_DEV_RECORD_PATH    "misc/bluedroid"
#define WOBLE_SETTING_FILE_NAME_7663 "woble_setting_7663.bin"
#define WOBLE_SETTING_FILE_NAME "woble_setting.bin"
#define APCF_SETTING_COUNT      10

#define WOBX_TYPE_IR                0x11    /* radio cmd, attr type */
#define WOBX_TYPE_KEYCODE_MAPPING   0x47    /* radio cmd, attr type */
#define IR_PROTOCOL "IR_PROTO"
#define IR_KEYMAP_G "IR_KEYMAP_GROUP"
#define IR_KEYMAP   "IR_KEYMAP"
#define MAX_IRKMG 20

typedef enum {
    TYPE_APCF_CMD,
    TYPE_RADIOFF_CMD,
    TYPE_IRKM_NEC2RC5_CMD,  /* this is only for NEC to RC5 key transfer */
} woble_setting_type;

struct cmd_tlv {
    //u8 type
    u8 len;
    u8 *value;
};

struct woble_ir_km_char_s {
    char nec[10];
    char rc5[10];
};

struct woble_ir_km_value_s {
    unsigned char nec[4];
    unsigned char rc5[4];
};

struct LD_btmtk_usb_data {
    mtkbt_dev_t *udev; /* store the usb device informaiton */

    unsigned long flags;
    int meta_tx;
    HC_IF *hcif;

    u8 cmdreq_type;

    unsigned int sco_num;
    int isoc_altsetting;
    int suspend_count;

    /* request for different io operation */
    u8 w_request;
    u8 r_request;

    /* io buffer for usb control transfer */
    unsigned char *io_buf;

    unsigned char *fw_image;
    unsigned char *fw_header_image;
    unsigned char *fw_bin_file_name;

    unsigned char *rom_patch;
    unsigned char *rom_patch_header_image;
    unsigned char *rom_patch_bin_file_name;
    u32 chip_id;
    u8 need_load_fw;
    u8 need_load_rom_patch;
    u32 rom_patch_offset;
    u32 rom_patch_len;
    u32 fw_len;

    u8 local_addr[BD_ADDR_LEN];
    char *woble_setting_file_name;
    u8 *woble_setting;
    u32 woble_setting_len;
    u8 *wake_dev;   /* ADDR:NAP-UAP-LAP, VID/PID:Both Little endian */
    u32 wake_dev_len;
    struct cmd_tlv apcf_cmd[APCF_SETTING_COUNT];
    struct cmd_tlv radioff_cmd[APCF_SETTING_COUNT];
};


int LD_btmtk_usb_probe(mtkbt_dev_t *dev);
void LD_btmtk_usb_disconnect(mtkbt_dev_t *dev);
void LD_btmtk_usb_SetWoble(mtkbt_dev_t *dev);

#define REV_MT76x2E3        0x0022

#define MT_REV_LT(_data, _chip, _rev)	\
	is_##_chip(_data) && (((_data)->chip_id & 0x0000ffff) < (_rev))

#define MT_REV_GTE(_data, _chip, _rev)	\
	is_##_chip(_data) && (((_data)->chip_id & 0x0000ffff) >= (_rev))

/*
 *  Load code method
 */
enum LOAD_CODE_METHOD {
	BIN_FILE_METHOD,
	HEADER_METHOD,
};
#endif
