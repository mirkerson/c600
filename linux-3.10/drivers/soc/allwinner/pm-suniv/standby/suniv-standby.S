/*
 * arch/arm/mach-sun3i/pm/sun3i-standby.S
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*stack point address in sram*/
#define SP_IN_SRAM	0xf0001ff0

	.text
	.globl save_sp
save_sp:
	mov r0, r13
	ldr  r13, =SP_IN_SRAM
	mov pc, lr

	.text
	.globl restore_sp
restore_sp:
	mov r13, r0
	mov pc, lr

	.text
	.globl mem_flush_tlb
mem_flush_tlb:
	push    {r0-r3}
	MOV     r0, #0
	/*instruction entire instruction tlb*/
	mcr p15, 0, r0, c8, c5, 0
	/* invalid entire data tlb */
	mcr p15, 0, r0, c8, c6, 0
	/*invalidate entire unified TLB inner shareable*/
	mcr p15, 0, r0, c8, c7, 0

	pop     {r0-r3}
	mov pc, lr

	.text
	.globl mem_preload_tlb
mem_preload_tlb:
	push    {r0-r3}
	/*16k*/
	ldr r0, =0xf0000000
	ldr r1, [r0]
	ldr r0, =0xf0001000
	ldr r1, [r0]
	ldr r0, =0xf0002000
	ldr r1, [r0]
	ldr r0, =0xf0003000
	ldr r1, [r0]

	pop     {r0-r3}
	mov pc,lr


	.text
	.globl disable_cache
disable_cache:
	push    {r0-r3}
	MRC     p15, 0, r0, c1, c0, 0       /*read cr                  */
	BIC	    r0, r0, #0x1 << 12
	BIC	    r0, r0, #0x1 << 2
	MCR     p15, 0, r0, c1, c0, 0       /*disable cache                  */
	pop     {r0-r3}
	MOV     pc, lr

	.text
	.globl disable_dcache
disable_dcache:
	push    {r0-r3}
	MRC     p15, 0, r0, c1, c0, 0       /*read cr                  */
	BIC	    r0, r0, #(0x1 << 2)
	MCR     p15, 0, r0, c1, c0, 0       /*disable dcache                  */
	pop     {r0-r3}
	MOV     pc, lr

	.text
	.globl disable_icache
disable_icache:
	push    {r0-r3}
	MRC     p15, 0, r0, c1, c0, 0       /*read cr                  */
	BIC	    r0, r0, #(0x1 << 12)
	MCR     p15, 0, r0, c1, c0, 0       /*disable icache                  */
	pop     {r0-r3}
	MOV     pc, lr
