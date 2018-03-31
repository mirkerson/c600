/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2011-2015, gq.yang China
*                                             All Rights Reserved
*
* File    : mem_clk.h
* By      : gq.yang
* Version : v1.0
* Date    : 2012-11-31 15:23
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_CLK_H__
#define __MEM_CLK_H__

#include "pm.h"

struct pll_factor_t {
	__u8 FactorN;
	__u8 FactorK;
	__u8 FactorM;
	__u8 FactorP;
	__u32 Pll;
};

struct clk_state {
	__ccmu_reg_list_t *CmuReg;
	__u32 ccu_reg_back[15];
};

__s32 mem_clk_save(struct clk_state *pclk_state);
__s32 mem_clk_restore(struct clk_state *pclk_state);
__ccmu_reg_list_t *mem_clk_init(__u32 mmu_flag);
__ccmu_reg_list_t *mem_get_ba(void);

static inline void mem_pio_clk_src_init(void)
{
	return;
}

static inline void mem_pio_clk_src_exit(void)
{
	return;
}

__u32 mem_clk_get_cpu_freq(void);
#endif	/*__MEM_CLK_H__*/

