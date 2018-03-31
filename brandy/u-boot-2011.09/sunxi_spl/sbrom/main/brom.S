

#include "config.h"

.globl _start
_start: b	reset
	ldr	pc, __nop
	ldr	pc, __nop
	ldr	pc, __nop
	ldr	pc, __nop
	ldr	pc, __nop
	ldr	pc, __nop
	ldr	pc, __nop

reset:

	ldr sp, =CONFIG_STACK_BASE
	bl cpu_init_crit
	bl clear_bss

	bl sboot_memset
	@bl system_mode
	bl sbromsw_entry
	b .

__nop:
	b __nop

clear_bss:
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end

	mov	r2, #0x00000000		/* clear			    */

clbss_l:
	str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

	mov pc, lr


cpu_init_crit:

	@Invalidate L1 I/D
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7,  0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5,  0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5,  6	@ invalidate BP array
	mcr p15, 0, r0, c7, c10, 4	@ DSB
	mcr p15, 0, r0, c7, c5,  4	@ ISB

	@disable MMU stuff and caches
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	@enable I-cache
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0

	mov pc, lr


@system_mode:
@
@	ldr r0, =__int_vector_start
@	add r1, r0, #0x20
@
@	mov r2, #CONFIG_SYS_INIT_RAM_ADDR
@
@copy_int_loop:
@	ldmia   r0!, {r7-r8}
@	stmia   r2!, {r7-r8}
@	cmp     r0, r1
@	blo     copy_int_loop
@
@	mov pc, lr

sboot_memset:
#if defined(CONFIG_ARCH_SUN8IW6P1)
	mov r7, #0
	mov r8, #0
	mov r9, #0
	mov r10, #0

	mov r0, #0x1000

	mov r2, #CONFIG_SYS_INIT_RAM_ADDR

memset_loop:
	stmia   r2!, {r7-r10}
	sub     r0, r0, #16
	cmp     r0, r1
	blo     memset_loop

	mov pc, lr
#else
	mov pc, lr
#endif
