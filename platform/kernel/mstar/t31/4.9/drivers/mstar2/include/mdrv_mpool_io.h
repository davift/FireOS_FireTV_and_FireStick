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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_mpool_io.h
/// @brief  Memory Pool  Driver IO Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MPOOL_IO_H_
#define _MDRV_MPOOL_IO_H_

/* Use 'M' as magic number */
#define MPOOL_IOC_MAGIC                'M'

#define MPOOL_IOC_INFO                 _IOWR(MPOOL_IOC_MAGIC, 0x00, DrvMPool_Info_t)
#define MPOOL_IOC_FLUSHDCACHE          _IOR(MPOOL_IOC_MAGIC, 0x01, DrvMPool_Info_t)
#define MPOOL_IOC_GET_BLOCK_OFFSET     _IOR(MPOOL_IOC_MAGIC, 0x02, unsigned int)
#define MPOOL_IOC_SET_MAP_CACHE        _IOR(MPOOL_IOC_MAGIC, 0x03, unsigned int)
#define MPOOL_IOC_TEST                 _IOWR(MPOOL_IOC_MAGIC, 0x06, De_Sys_Info_t)
#define MPOOL_IOC_SET                  _IOR(MPOOL_IOC_MAGIC, 0x04, DrvMPool_Info_t)
#define MPOOL_IOC_KERNEL_DETECT        _IOWR(MPOOL_IOC_MAGIC, 0x05, DrvMPool_Info_t)
#define MPOOL_IOC_FLUSHDCACHE_PAVA     _IOR(MPOOL_IOC_MAGIC, 0x07, DrvMPool_Flush_Info_t)
#define MPOOL_IOC_VERSION              _IOWR(MPOOL_IOC_MAGIC, 0x08, unsigned int)
#define MPOOL_IOC_FLUSHDCACHE_ALL      _IOR(MPOOL_IOC_MAGIC, 0x09, DrvMPool_Flush_Info_t)
#define MPOOL_IOC_SETWATCHPT           _IOW(MPOOL_IOC_MAGIC, 0x0A, DrvMPool_Watchpt_Info_t)
#define MPOOL_IOC_GETWATCHPT           _IOR(MPOOL_IOC_MAGIC, 0x0B, char*)
#define MPOOL_IOC_BA2PA                _IOWR(MPOOL_IOC_MAGIC, 0x0C, MS_PHY64)
#define MPOOL_IOC_PA2BA                _IOWR(MPOOL_IOC_MAGIC, 0x0D, MS_PHY64)

#if defined(CONFIG_COMPAT)
#define COMPAT_MPOOL_IOC_FLUSHDCACHE_PAVA _IOR(MPOOL_IOC_MAGIC, 0x07, DrvMPool_Flush_Info_t32)
#define COMPAT_MPOOL_IOC_SETWATCHPT       _IOW(MPOOL_IOC_MAGIC, 0x0A, DrvMPool_Watchpt_Info_t32)
#endif

#endif

