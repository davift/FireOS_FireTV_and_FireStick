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

#ifndef _MIU_MTV19008_MT5871_M7632_STR_H_
#define _MIU_MTV19008_MT5871_M7632_STR_H_

#if (ENABLE_MTK_BD_MTV19008_MT5871_M7632 == 1)

#if !defined(CONFIG_MIU0_DRAM_NONE)
const MS_REG_INIT MIU0_DDR_PreInit_Str[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x110d30, 0x34e6),
    _RV32_2(0x110d32, 0x001a),
    _RV32_2(0x110d8a, 0x0001),
    _RV32_2(0x110d84, 0x0000),
    _RV32_2(0x110d86, 0x0006),
    _RV32_2(0x110d8c, 0x0000),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};

const MS_REG_INIT MIU0_DDR_Init_Str[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x110d22, 0x0000),
    _RV32_2(0x110d2c, 0x0114),
    _RV32_2(0x101202, 0xf200),
    _RV32_2(0x101204, 0x0054),
    _RV32_2(0x101206, 0x0000),
    _RV32_2(0x101228, 0x00c0),
    _RV32_2(0x152b20, 0x3103),
    _RV32_2(0x152b22, 0x0202),
    _RV32_2(0x152b40, 0x0e0e),
    _RV32_2(0x152b42, 0x0724),
    _RV32_2(0x152b44, 0x3208),
    _RV32_2(0x152b46, 0x100a),
    _RV32_2(0x152b48, 0x0e08),
    _RV32_2(0x152b4a, 0x0704),
    _RV32_2(0x152b4c, 0x052a),
    _RV32_2(0x152b4e, 0x0116),
    _RV32_2(0x152b50, 0xe000),
    _RV32_2(0x152b52, 0x0000),
    _RV32_2(0x152b54, 0x0e00),
    _RV32_2(0x152b5a, 0x0807),
    _RV32_2(0x152b5e, 0x04e4),
    _RV32_2(0x152b60, 0x1124),
    _RV32_2(0x152b62, 0x4004),
    _RV32_2(0x152b64, 0x8028),
    _RV32_2(0x152b66, 0xc000),
    _RV32_2(0x152b68, 0x0000),
    _RV32_2(0x152b6a, 0x0000),
    _RV32_2(0x152b6c, 0x0000),
    _RV32_2(0x152b80, 0x8040),
    _RV32_2(0x152b86, 0x0000),
    _RV32_2(0x110d02, 0xaaaa),
    _RV32_2(0x110d04, 0x0008),
    _RV32_2(0x110d0e, 0x0099),
    _RV32_2(0x110d2e, 0x1111),
    _RV32_2(0x110db6, 0x0000),
    _RV32_2(0x110ddc, 0x0007),
    _RV32_2(0x110de0, 0x0011),
    _RV32_2(0x110de2, 0x0011),
    _RV32_2(0x110de4, 0x0010),
    _RV32_2(0x110de6, 0x1111),
    _RV32_2(0x110de8, 0x1111),
    _RV32_2(0x110dea, 0x1111),
    _RV32_2(0x110dec, 0x1111),
    _RV32_2(0x110dee, 0x1111),
    _RV32_2(0x110df0, 0x2222),
    _RV32_2(0x110df2, 0x0011),
    _RV32_2(0x151500, 0x0003),
    _RV32_2(0x151502, 0x0f0f),
    _RV32_2(0x151504, 0x0000),
    _RV32_2(0x151506, 0x0f0f),
    _RV32_2(0x151508, 0x0101),
    _RV32_2(0x15150a, 0x0f0f),
    _RV32_2(0x15150c, 0x1f00),
    _RV32_2(0x151510, 0x0000),
    _RV32_2(0x151512, 0x0000),
    _RV32_2(0x151514, 0x0909),
    _RV32_2(0x151516, 0x0a0a),
    _RV32_2(0x15151e, 0x0000),
    _RV32_2(0x151520, 0x0909),
    _RV32_2(0x151522, 0x0909),
    _RV32_2(0x151524, 0x0909),
    _RV32_2(0x151526, 0x0909),
    _RV32_2(0x151528, 0x0909),
    _RV32_2(0x15152a, 0x0909),
    _RV32_2(0x15152c, 0x0909),
    _RV32_2(0x15152e, 0x0909),
    _RV32_2(0x151530, 0x0909),
    _RV32_2(0x151532, 0x0909),
    _RV32_2(0x151534, 0x0908),
    _RV32_2(0x151536, 0x0909),
    _RV32_2(0x151538, 0x0a0a),
    _RV32_2(0x15153a, 0x0a0a),
    _RV32_2(0x15153c, 0x0a0a),
    _RV32_2(0x15153e, 0x0a0a),
    _RV32_2(0x151560, 0x0707),
    _RV32_2(0x151562, 0x0707),
    _RV32_2(0x151564, 0x1414),
    _RV32_2(0x151566, 0x1414),
    _RV32_2(0x151568, 0x1414),
    _RV32_2(0x15156a, 0x1414),
#if !defined(CONFIG_ENABLE_MIU0_AUTO_DQS)
    _RV32_2(0x151580, 0x0101),
    _RV32_2(0x151582, 0x0101),
    _RV32_2(0x151584, 0x0101),
    _RV32_2(0x151586, 0x0101),
    _RV32_2(0x151588, 0x0101),
    _RV32_2(0x15158a, 0x0101),
    _RV32_2(0x15158c, 0x0101),
    _RV32_2(0x15158e, 0x0101),
    _RV32_2(0x151590, 0x0101),
    _RV32_2(0x151592, 0x0101),
    _RV32_2(0x151594, 0x0101),
    _RV32_2(0x151596, 0x0101),
    _RV32_2(0x151598, 0x0101),
    _RV32_2(0x15159a, 0x0101),
    _RV32_2(0x15159c, 0x0101),
    _RV32_2(0x15159e, 0x0101),
    _RV32_2(0x110d94, 0x0403),
    _RV32_2(0x110d96, 0x0404),
    _RV32_2(0x110da4, 0x0001),
    _RV32_2(0x110da6, 0x0004),
#endif
    _RV32_2(0x1515e0, 0x0011),
    _RV32_2(0x1515e2, 0x0011),
    _RV32_2(0x1515e4, 0x0000),
    _RV32_2(0x1515e6, 0x1111),
    _RV32_2(0x1515e8, 0x1111),
    _RV32_2(0x1515ea, 0x1111),
    _RV32_2(0x1515ec, 0x1111),
    _RV32_2(0x1515ee, 0x1111),
    _RV32_2(0x1515f2, 0x0011),
    _RV32_2(0x110d62, 0x007f),
    _RV32_2(0x110d64, 0xf000),
    _RV32_2(0x110d60, 0x00fb),
    _RV32_2(0x110d60, 0x00ff),
    _RV32_2(0x110d60, 0x00fb),
    _RV32_2(0x110d60, 0x00f3),
    _RV32_2(0x110d60, 0x00fb),
    _RV32_2(0x110d60, 0x00f3),
    _RV32_2(0x110d60, 0x00fb),
    _RV32_2(0x110d60, 0x00f2),
    _RV32_2(0x110d60, 0x00f0),
    _RV32_2(0x110d60, 0x33f8),
    _RV32_2(0x110d70, 0x0000),
    _RV32_2(0x110d98, 0x0000),
    _RV32_2(0x110d9a, 0x0000),
    _RV32_2(0x110d90, 0xf0f3),
    _RV32_2(0x110d70, 0x0800),
    _RV32_2(0x110d70, 0x0800),
    _RV32_2(0x1515a0, 0x1e1e),
    _RV32_2(0x1515a2, 0x1012),
    _RV32_2(0x1515a4, 0x1616),
    _RV32_2(0x1515a6, 0x1212),
    _RV32_2(0x1515a8, 0x1212),
    _RV32_2(0x1515aa, 0x1e1e),
    _RV32_2(0x1515ac, 0x1012),
    _RV32_2(0x1515ae, 0x1616),
    _RV32_2(0x1515b0, 0x1212),
    _RV32_2(0x1515b2, 0x1212),
    _RV32_2(0x110d6c, 0x1616),
    _RV32_2(0x110d6e, 0x1616),
    _RV32_2(0x110d74, 0x0606),
    _RV32_2(0x110d76, 0x0606),
    _RV32_2(0x110d1a, 0x8022),
    _RV32_2(0x110d1c, 0x0020),
    _RV32_2(0x110d08, 0x0000),
    _RV32_2(0x110d1e, 0x0005),
    _RV32_2(0x110d1e, 0x000f),
    _RV32_2(0x110d1e, 0x0005),
    _RV32_2(0x152bdc, 0x000a),
    _RV32_2(0x152b88, 0x0c00),
    _RV32_2(0x152b8a, 0xff12),
    _RV32_2(0x152b80, 0x80f0),
    _RV32_2(0x101230, 0xc804),
    _RV32_2(0x152b9c, 0x0100),
    _RV32_2(0x110d3e, 0x0006),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU0_DDR_Init_Str_2nd[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x151508, 0x0808),
    _RV32_2(0x151520, 0x0809),
    _RV32_2(0x151534, 0x0909),
    _RV32_2(0x151536, 0x090a),
    _RV32_2(0x15153c, 0x0b0c),
    _RV32_2(0x15153e, 0x0b0b),
    _RV32_2(0x15159e, 0x0302),
    _RV32_2(0x110d94, 0x0303),
    _RV32_2(0x110d96, 0x0303),
    _RV32_2(0x110da4, 0x2301),
    _RV32_2(0x110da6, 0x2202),
    _RV32_2(0x1515e6, 0x0000),
    _RV32_2(0x110d74, 0x0505),
    _RV32_2(0x110d76, 0x0505),
    _RV32_2(0x110d3e, 0x0007),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU0_DDR_Init_Str_3rd[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x151514, 0x0a0a),
    _RV32_2(0x151520, 0x0a0a),
    _RV32_2(0x151522, 0x0a0a),
    _RV32_2(0x151524, 0x0a09),
    _RV32_2(0x151526, 0x090a),
    _RV32_2(0x151528, 0x0a0a),
    _RV32_2(0x15152a, 0x0a0a),
    _RV32_2(0x15152c, 0x0a0a),
    _RV32_2(0x151530, 0x0a0a),
    _RV32_2(0x151532, 0x0a0a),
    _RV32_2(0x151534, 0x0a0a),
    _RV32_2(0x151536, 0x0a0a),
    _RV32_2(0x15153a, 0x090a),
    _RV32_2(0x151560, 0x0606),
    _RV32_2(0x151562, 0x0606),
    _RV32_2(0x15158a, 0x0102),
    _RV32_2(0x15158e, 0x0102),
    _RV32_2(0x151594, 0x0100),
    _RV32_2(0x151596, 0x0001),
    _RV32_2(0x110d94, 0x0102),
    _RV32_2(0x110d96, 0x0202),
    _RV32_2(0x110da6, 0x0002),
    _RV32_2(0x1515a4, 0x1212),
    _RV32_2(0x1515ae, 0x1212),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU0_DDR_Init_Str_4th[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU0_DDR_Init_Str_5th[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x151508, 0x0808),
    _RV32_2(0x151520, 0x0809),
    _RV32_2(0x151534, 0x0909),
    _RV32_2(0x151536, 0x090a),
    _RV32_2(0x15153c, 0x0b0c),
    _RV32_2(0x15153e, 0x0b0b),
    _RV32_2(0x15159e, 0x0302),
    _RV32_2(0x110d94, 0x0303),
    _RV32_2(0x110d96, 0x0303),
    _RV32_2(0x110da4, 0x2301),
    _RV32_2(0x110da6, 0x2202),
    _RV32_2(0x1515e6, 0x0000),
    _RV32_2(0x110d74, 0x0505),
    _RV32_2(0x110d76, 0x0505),
    _RV32_2(0x110d3e, 0x0007),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU0_DDR_Init_Str_6th[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU0_DDR_Init_Str_7th[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x151508, 0x0808),
    _RV32_2(0x151520, 0x0809),
    _RV32_2(0x151534, 0x0909),
    _RV32_2(0x151536, 0x090a),
    _RV32_2(0x15153c, 0x0b0c),
    _RV32_2(0x15153e, 0x0b0b),
    _RV32_2(0x15159e, 0x0302),
    _RV32_2(0x110d94, 0x0303),
    _RV32_2(0x110d96, 0x0303),
    _RV32_2(0x110da4, 0x2301),
    _RV32_2(0x110da6, 0x2202),
    _RV32_2(0x1515e6, 0x0000),
    _RV32_2(0x110d74, 0x0505),
    _RV32_2(0x110d76, 0x0505),
    _RV32_2(0x110d3e, 0x0007),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
const MS_REG_INIT MIU1_DDR_PreInit_Str[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x161630, 0x34e6),
    _RV32_2(0x161632, 0x001a),
    _RV32_2(0x16168a, 0x0001),
    _RV32_2(0x161684, 0x0000),
    _RV32_2(0x161686, 0x0006),
    _RV32_2(0x16168c, 0x0000),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};

const MS_REG_INIT MIU1_DDR_Init_Str[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x161622, 0x0000),
    _RV32_2(0x16162c, 0x0014),
    _RV32_2(0x100602, 0xf200),
    _RV32_2(0x100604, 0x0054),
    _RV32_2(0x100606, 0x0000),
    _RV32_2(0x100628, 0x00c0),
    _RV32_2(0x152c20, 0x3103),
    _RV32_2(0x152c22, 0x0202),
    _RV32_2(0x152c40, 0x0e0e),
    _RV32_2(0x152c42, 0x0724),
    _RV32_2(0x152c44, 0x3208),
    _RV32_2(0x152c46, 0x100a),
    _RV32_2(0x152c48, 0x0e08),
    _RV32_2(0x152c4a, 0x0704),
    _RV32_2(0x152c4c, 0x052a),
    _RV32_2(0x152c4e, 0x0116),
    _RV32_2(0x152c50, 0xe000),
    _RV32_2(0x152c52, 0x0000),
    _RV32_2(0x152c54, 0x0e00),
    _RV32_2(0x152c5a, 0x0807),
    _RV32_2(0x152c5e, 0x04e4),
    _RV32_2(0x152c60, 0x1124),
    _RV32_2(0x152c62, 0x4004),
    _RV32_2(0x152c64, 0x8028),
    _RV32_2(0x152c66, 0xc000),
    _RV32_2(0x152c68, 0x0000),
    _RV32_2(0x152c6a, 0x0000),
    _RV32_2(0x152c6c, 0x0000),
    _RV32_2(0x152c80, 0x8040),
    _RV32_2(0x152c86, 0x0000),
    _RV32_2(0x161602, 0xaaaa),
    _RV32_2(0x161604, 0x0008),
    _RV32_2(0x16160e, 0x0099),
    _RV32_2(0x16162e, 0x1111),
    _RV32_2(0x1616b6, 0x0000),
    _RV32_2(0x1616dc, 0x0007),
    _RV32_2(0x1616e0, 0x0011),
    _RV32_2(0x1616e2, 0x0011),
    _RV32_2(0x1616e4, 0x0010),
    _RV32_2(0x1616e6, 0x1111),
    _RV32_2(0x1616e8, 0x1111),
    _RV32_2(0x1616ea, 0x1111),
    _RV32_2(0x1616ec, 0x1111),
    _RV32_2(0x1616ee, 0x1111),
    _RV32_2(0x1616f0, 0x2222),
    _RV32_2(0x1616f2, 0x0011),
    _RV32_2(0x151600, 0x0003),
    _RV32_2(0x151602, 0x0f0f),
    _RV32_2(0x151604, 0x0000),
    _RV32_2(0x151606, 0x0f0f),
    _RV32_2(0x151608, 0x0000),
    _RV32_2(0x15160a, 0x0f0f),
    _RV32_2(0x15160c, 0x1f00),
    _RV32_2(0x151610, 0x0000),
    _RV32_2(0x151612, 0x0000),
    _RV32_2(0x151614, 0x0a0a),
    _RV32_2(0x151616, 0x0a0a),
    _RV32_2(0x15161e, 0x0000),
    _RV32_2(0x151620, 0x0a0a),
    _RV32_2(0x151622, 0x0a0a),
    _RV32_2(0x151624, 0x0a0a),
    _RV32_2(0x151626, 0x0a0a),
    _RV32_2(0x151628, 0x0a0a),
    _RV32_2(0x15162a, 0x0a0a),
    _RV32_2(0x15162c, 0x0a0a),
    _RV32_2(0x15162e, 0x0a0a),
    _RV32_2(0x151630, 0x0a0a),
    _RV32_2(0x151632, 0x0a0a),
    _RV32_2(0x151634, 0x0a0a),
    _RV32_2(0x151636, 0x0a0a),
    _RV32_2(0x151638, 0x0a09),
    _RV32_2(0x15163a, 0x0a0a),
    _RV32_2(0x15163c, 0x0a0a),
    _RV32_2(0x15163e, 0x0a0a),
    _RV32_2(0x151660, 0x0707),
    _RV32_2(0x151662, 0x0707),
    _RV32_2(0x151664, 0x1414),
    _RV32_2(0x151666, 0x1414),
    _RV32_2(0x151668, 0x1414),
    _RV32_2(0x15166a, 0x1414),
#if !defined(CONFIG_ENABLE_MIU1_AUTO_DQS)
    _RV32_2(0x151680, 0x0101),
    _RV32_2(0x151682, 0x0101),
    _RV32_2(0x151684, 0x0101),
    _RV32_2(0x151686, 0x0101),
    _RV32_2(0x151688, 0x0101),
    _RV32_2(0x15168a, 0x0101),
    _RV32_2(0x15168c, 0x0101),
    _RV32_2(0x15168e, 0x0101),
    _RV32_2(0x151690, 0x0101),
    _RV32_2(0x151692, 0x0101),
    _RV32_2(0x151694, 0x0101),
    _RV32_2(0x151696, 0x0101),
    _RV32_2(0x151698, 0x0101),
    _RV32_2(0x15169a, 0x0101),
    _RV32_2(0x15169c, 0x0101),
    _RV32_2(0x15169e, 0x0101),
    _RV32_2(0x161694, 0x0100),
    _RV32_2(0x161696, 0x0302),
    _RV32_2(0x1616a4, 0x0300),
    _RV32_2(0x1616a6, 0x0303),
#endif
    _RV32_2(0x1516e0, 0x0011),
    _RV32_2(0x1516e2, 0x0011),
    _RV32_2(0x1516e4, 0x0000),
    _RV32_2(0x1516e6, 0x1111),
    _RV32_2(0x1516e8, 0x1111),
    _RV32_2(0x1516ea, 0x1111),
    _RV32_2(0x1516ec, 0x1111),
    _RV32_2(0x1516ee, 0x1111),
    _RV32_2(0x1516f2, 0x0011),
    _RV32_2(0x161662, 0x007f),
    _RV32_2(0x161664, 0xf000),
    _RV32_2(0x161660, 0x00fb),
    _RV32_2(0x161660, 0x00ff),
    _RV32_2(0x161660, 0x00fb),
    _RV32_2(0x161660, 0x00f3),
    _RV32_2(0x161660, 0x00fb),
    _RV32_2(0x161660, 0x00f3),
    _RV32_2(0x161660, 0x00fb),
    _RV32_2(0x161660, 0x00f2),
    _RV32_2(0x161660, 0x00f0),
    _RV32_2(0x161660, 0x33f8),
    _RV32_2(0x161670, 0x0000),
    _RV32_2(0x161698, 0x0000),
    _RV32_2(0x16169a, 0x0000),
    _RV32_2(0x161690, 0xf0f3),
    _RV32_2(0x161670, 0x0800),
    _RV32_2(0x161670, 0x0800),
    _RV32_2(0x1516a0, 0x1e1e),
    _RV32_2(0x1516a2, 0x1012),
    _RV32_2(0x1516a4, 0x1616),
    _RV32_2(0x1516a6, 0x1212),
    _RV32_2(0x1516a8, 0x1212),
    _RV32_2(0x1516aa, 0x1e1e),
    _RV32_2(0x1516ac, 0x1012),
    _RV32_2(0x1516ae, 0x1616),
    _RV32_2(0x1516b0, 0x1212),
    _RV32_2(0x1516b2, 0x1212),
    _RV32_2(0x16166c, 0x1616),
    _RV32_2(0x16166e, 0x1616),
    _RV32_2(0x161674, 0x0606),
    _RV32_2(0x161676, 0x0606),
    _RV32_2(0x16161a, 0x8011),
    _RV32_2(0x16161c, 0x0020),
    _RV32_2(0x161608, 0x0000),
    _RV32_2(0x16161e, 0x0005),
    _RV32_2(0x16161e, 0x000f),
    _RV32_2(0x16161e, 0x0005),
    _RV32_2(0x152cdc, 0x000a),
    _RV32_2(0x152c88, 0x0c00),
    _RV32_2(0x152c8a, 0xff12),
    _RV32_2(0x152c80, 0x80f0),
    _RV32_2(0x100630, 0xc804),
    _RV32_2(0x152c9c, 0x0100),
    _RV32_2(0x16163e, 0x0006),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU1_DDR_Init_Str_2nd[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x151608, 0x0008),
    _RV32_2(0x151620, 0x0809),
    _RV32_2(0x151622, 0x0909),
    _RV32_2(0x151630, 0x0a09),
    _RV32_2(0x151632, 0x0909),
    _RV32_2(0x151634, 0x0a09),
    _RV32_2(0x151636, 0x0909),
    _RV32_2(0x151638, 0x0a0a),
    _RV32_2(0x1616a4, 0x0200),
    _RV32_2(0x1616a6, 0x0103),
    _RV32_2(0x1516e6, 0x1110),
    _RV32_2(0x1516a2, 0x1212),
    _RV32_2(0x1516ac, 0x1212),
    _RV32_2(0x161674, 0x0505),
    _RV32_2(0x161676, 0x0505),
    _RV32_2(0x16163e, 0x0007),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU1_DDR_Init_Str_3rd[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x152c20, 0x3003),
    _RV32_2(0x151604, 0x0f00),
    _RV32_2(0x151608, 0x010f),
    _RV32_2(0x151614, 0x0707),
    _RV32_2(0x151660, 0x0000),
    _RV32_2(0x161696, 0x0100),
    _RV32_2(0x1616a6, 0x0300),
    _RV32_2(0x1516a4, 0x1212),
    _RV32_2(0x1516ae, 0x1212),
    _RV32_2(0x100602, 0x0a00),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU1_DDR_Init_Str_4th[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU1_DDR_Init_Str_5th[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x151620, 0x0809),
    _RV32_2(0x151622, 0x0909),
    _RV32_2(0x151630, 0x0a09),
    _RV32_2(0x151634, 0x0a09),
    _RV32_2(0x151636, 0x0909),
    _RV32_2(0x151638, 0x0a0a),
    _RV32_2(0x1616a4, 0x0200),
    _RV32_2(0x1616a6, 0x0103),
    _RV32_2(0x1516a2, 0x1212),
    _RV32_2(0x1516ac, 0x1212),
    _RV32_2(0x161674, 0x0505),
    _RV32_2(0x161676, 0x0505),
    _RV32_2(0x16163e, 0x0007),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU1_DDR_Init_Str_6th[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
const MS_REG_INIT MIU1_DDR_Init_Str_7th[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x151620, 0x0809),
    _RV32_2(0x151622, 0x0909),
    _RV32_2(0x151630, 0x0a09),
    _RV32_2(0x151634, 0x0a09),
    _RV32_2(0x151636, 0x0909),
    _RV32_2(0x151638, 0x0a0a),
    _RV32_2(0x1616a4, 0x0200),
    _RV32_2(0x1616a6, 0x0103),
    _RV32_2(0x1516a2, 0x1212),
    _RV32_2(0x1516ac, 0x1212),
    _RV32_2(0x161674, 0x0505),
    _RV32_2(0x161676, 0x0505),
    _RV32_2(0x16163e, 0x0007),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
#endif

#if defined(CONFIG_ENABLE_MIU_SSC)
const MS_REG_INIT MIU0_EnableSSC_STR[] =
{
#if defined(CONFIG_MIU0_4X_MODE)
#if defined(CONFIG_MIU0_DDR3_2133)
    _RV32_2(0x110d28, 0x000d),
    _RV32_2(0x110d2a, 0x0527),
    _RV32_2(0x110d28, 0xc00d),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU0_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
#endif

#if defined(CONFIG_ENABLE_MIU_SSC)
const MS_REG_INIT MIU1_EnableSSC_STR[] =
{
#if defined(CONFIG_MIU1_4X_MODE)
#if defined(CONFIG_MIU1_DDR3_2133)
    _RV32_2(0x161628, 0x000d),
    _RV32_2(0x16162a, 0x0527),
    _RV32_2(0x161628, 0xc00d),
#else
    #error "Invalid DRAM Setting"
#endif

#elif defined(CONFIG_MIU1_8X_MODE)
    #error "Not support DRAM 8X mode "
#else
    #error "Invalid DRAM Setting"
#endif

    _END_OF_TBL32_,
    MIU_VER
};
#endif


#endif

#endif
