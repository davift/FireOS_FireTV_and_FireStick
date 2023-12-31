/*
 * Copyright (C) 2013 ARM Ltd.
 * Copyright (C) 2013 Linaro.
 *
 * This code is based on glibc cortex strings work originally authored by Linaro
 * and re-licensed under GPLv2 for the Linux kernel. The original code can
 * be found @
 *
 * http://bazaar.launchpad.net/~linaro-toolchain-dev/cortex-strings/trunk/
 * files/head:/src/aarch64/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/cache.h>

/*
 * Copy a buffer from src to dest (alignment handled by the hardware)
 *
 * Parameters:
 *	x0 - dest
 *	x1 - src
 *	x2 - n
 * Returns:
 *	x0 - dest
 */
	.macro ldrb1 ptr, regB, val
	ldrb  \ptr, [\regB], \val
	.endm

	.macro strb1 ptr, regB, val
	strb \ptr, [\regB], \val
	.endm

	.macro ldrh1 ptr, regB, val
	ldrh  \ptr, [\regB], \val
	.endm

	.macro strh1 ptr, regB, val
	strh \ptr, [\regB], \val
	.endm

	.macro ldr1 ptr, regB, val
	ldr \ptr, [\regB], \val
	.endm

	.macro str1 ptr, regB, val
	str \ptr, [\regB], \val
	.endm

	.macro ldp1 ptr, regB, regC, val
	ldp \ptr, \regB, [\regC], \val
	.endm

	.macro stp1 ptr, regB, regC, val
	stp \ptr, \regB, [\regC], \val
	.endm

	.weak memcpy
ENTRY(__memcpy)
ENTRY(memcpy)
#if defined(CONFIG_MP_PLATFORM_ARM)
	mov	x4, x0
	subs	x2, x2, #8
	b.mi	2f
1:	ldr	x3, [x1], #8
	subs	x2, x2, #8
	str	x3, [x4], #8
	b.pl	1b
2:	adds	x2, x2, #4
	b.mi	3f
	ldr	w3, [x1], #4
	sub	x2, x2, #4
	str	w3, [x4], #4
3:	adds	x2, x2, #2
	b.mi	4f
	ldrh	w3, [x1], #2
	sub	x2, x2, #2
	strh	w3, [x4], #2
4:	adds	x2, x2, #1
	b.mi	5f
	ldrb	w3, [x1]
	strb	w3, [x4]
5:	ret
#else
#include "copy_template.S"
	ret
#endif
ENDPIPROC(memcpy)
ENDPROC(__memcpy)
