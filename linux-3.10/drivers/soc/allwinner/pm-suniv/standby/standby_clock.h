/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby_clock.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 21:05
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __STANDBY_CLOCK_H__
#define __STANDBY_CLOCK_H__
#include "standby_i.h"

#define PLL_CTRL_REG0_OFFSET	(0x40)
#define PLL_CTRL_REG1_OFFSET	(0x44)

void standby_mdelay(__u32 ms);
__s32 standby_clk_init(void);
__s32 standby_clk_exit(void);
__s32 standby_clk_core2losc(void);
__s32 standby_clk_core2hosc(void);
__s32 standby_clk_core2pll(void);
__s32 standby_clk_set_pll_factor(struct pll_factor_t *pll_factor);
__s32 standby_clk_get_pll_factor(struct pll_factor_t *pll_factor);
__s32 standby_clk_bus_src_backup(void);
__s32 standby_clk_bus_src_set(void);
__s32 standby_clk_bus_src_restore(void);
void standby_clk_dramgating(int onoff);
__u32 standby_clk_get_cpu_freq(void);

#endif				/* __STANDBY_CLOCK_H__ */

