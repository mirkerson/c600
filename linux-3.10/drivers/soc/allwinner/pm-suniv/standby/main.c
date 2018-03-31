/**********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        newbie Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-30 18:34
* Descript: platform standby fucntion.
* Update  : date                auther      ver     notes
**********************************************************************************************************/

#include <linux/kernel.h>
#include "dram.h"
#include "standby_i.h"
#include "standby_clock.h"
#include "../mem_int.h"

static void standby(void);
static int cpu_enter_lowfreq(void);
static int cpu_freq_resume(void);

extern unsigned int save_sp(void);
extern void restore_sp(unsigned int sp);

extern char *__bss_start;
extern char *__bss_end;
extern char *__standby_start;
extern char *__standby_end;

static __u32 sp_backup;
static struct pll_factor_t orig_pll;
static struct pll_factor_t local_pll;

int standby_main(void)
{
	char    *tmpPtr = (char *)&__bss_start;
	
	/* clear bss segment */
	do{*tmpPtr ++ = 0;}while(tmpPtr <= (char *)&__bss_end);

	/* flush data and instruction tlb, there is 32 items of data tlb and
	 * 32 items of instruction tlb, The TLB is normally allocated on a
	 * rotating basis. The oldest entry is always the next allocated
	 */
	//mem_flush_tlb();

	/*disable Dcache & Icache*/
	//disable_cache();

	/* preload tlb for standby */
	//mem_preload_tlb();

	/* save stack pointer registger, switch stack to sram */
	sp_backup = save_sp();

	/*ccu init for platform standby*/
	standby_clk_init();

	/* enable dram enter into self-refresh */
	dram_power_save_process();

	/* cpu reduce frequency */
	cpu_enter_lowfreq();

	/* process standby */
	standby();

	/* cpu freq resume */
	cpu_freq_resume();

	/* restore dram */
	dram_power_up_process();

	/* restore stack pointer register, switch stack back to dram */
	restore_sp(sp_backup);

	return 0;
}


static int cpu_enter_lowfreq(void)
{
	/* backup cpu freq */
	standby_clk_get_pll_factor(&orig_pll);
	/* backup bus src */
	standby_clk_bus_src_backup();

	/*lower freq from 528M to 216M */
	local_pll.FactorN = 8;
	local_pll.FactorK = 0;
	local_pll.FactorM = 0;
	local_pll.FactorP = 0;
	standby_clk_set_pll_factor(&local_pll);
	standby_mdelay(100);

	/* switch cpu clock to HOSC:
	 * after switch to HOSC, the axi, ahb freq will change also
	 * which will lead the jtag exception. */
//	standby_clk_core2hosc();
	standby_clk_core2losc();
	standby_mdelay(100);
	
	standby_clk_bus2Losc();
	standby_mdelay(100);

	//disable plls	
	standby_clk_pll_disable();
	standby_mdelay(100);

	standby_close_24M();
	standby_mdelay(100);

	return 0;
}

static int cpu_freq_resume(void)
{
	standby_open_24M();
	standby_mdelay(100);
	
	//enable plls
	standby_clk_pll_enable();
	standby_mdelay(100);

	standby_clk_busReset();
	standby_mdelay(100);
	
	/* switch clock to hosc */
	standby_clk_core2hosc();
	standby_mdelay(100);

	/* switch cpu clock to core pll */
	standby_clk_core2pll();
	standby_mdelay(100);

	/*restore freq from 216M to 528M */
	standby_clk_set_pll_factor(&orig_pll);
	standby_mdelay(100);

	return 0;
}

static void standby(void)
{
	volatile __u32 tmp= 0;
	__u32 wake_cond= 1<<INT_SOURCE_LRADC;	//for LRADC irqs

	/*wait LRADC irqs*/
//	do {
		/* cpu enter sleep, wait wakeup by interrupt */
		//	asm("WFI");
		__asm__ volatile ("mcr  p15, 0, r0, c7, c0, 4");
		
//		tmp = *(volatile __u32 *)(IO_ADDRESS(SUNXI_INTC_PBASE) + SW_INT_PENDING_REG0);
		
//		if(tmp&wake_cond)
//			break;
//	}while(1);
}

