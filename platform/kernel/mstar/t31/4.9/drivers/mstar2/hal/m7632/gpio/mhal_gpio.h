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

#ifndef _HAL_GPIO_H_
#define _HAL_GPIO_H_

#include <asm/types.h>
#include "mdrv_types.h"
#include "chip_int.h"
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
//#define MASK(x)                   (((1<<(x##_BITS))-1) << x##_SHIFT)
//#define BIT(_bit_)                (1 << (_bit_))
#define BIT_(x)                     BIT(x) //[OBSOLETED] //TODO: remove it later
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))
#define __Sizeof_GPIO_Pins() \
	(sizeof(gpio_table) /sizeof(struct gpio_setting))

#define INT_COUNT                       8
#define GPIO_PM_INT_SUPPORTED           1
extern const int gpio_IntPad[INT_COUNT];
extern const int gpio_IRQnum[INT_COUNT];
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
//the functions of this section set to initialize
extern void MHal_GPIO_Init(void);
extern void MHal_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk);
extern U8   MHal_GPIO_ReadRegBit(U32 u32Reg, U8 u8BitMsk);
extern void MHal_GPIO_Pad_Set(U8 u8IndexGPIO);
extern void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO);
extern void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO);
extern U8   MHal_GPIO_Pad_Level(U8 u8IndexGPIO);
extern U8   MHal_GPIO_Pad_InOut(U8 u8IndexGPIO);
extern void MHal_GPIO_Pull_High(U8 u8IndexGPIO);
extern void MHal_GPIO_Pull_Low(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_High(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_Low(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_Input(U8 u8IndexGPIO);
extern int MHal_GPIO_Get_Interrupt_Num(U8 u8IndexGPIO);
extern U16 MHal_GPIO_Get_Pins_Count(void);
extern int MHal_GPIO_Set_Pin_Status_Array(U32* pGPIOPinStatusList, U8 u8PinCount, U8 *upRetPinCount, U32* pin_disable);
extern int MHal_GPIO_Get_Pin_Status_Array(U32* pGPIOPinStatusList, U8 u8PinCount, U8 *upRetPinCount);
extern int MHal_GPIO_Enable_Interrupt(int gpio_num, unsigned long gpio_edge_type, irq_handler_t pCallback, void *dev_id);
extern int MHal_GPIO_Disable_Interrupt(int gpio_num, void *dev_id);

#endif // _HAL_GPIO_H_

