/*
 * Copyright (c) 2011-2020 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include <linux/kernel.h>
#include "standby_clock.h"
#include "../mem_ccmu-sunivw1p1.h"
#include "../pm_config.h"

static __ccmu_reg_list_t *CmuReg;

/*==============================================================================*/
/* CLOCK SET FOR SYSTEM STANDBY*/
/*==============================================================================*/
static __ccmu_pll1_reg0000_t CmuReg_Pll1Ctl_tmp;
static __ccmu_sysclk_ratio_reg0050_t CmuReg_SysClkDiv_tmp;
static __u32 pio_int_deb_back;
static __ccmu_ahb1_ratio_reg0054_t CmuReg_ahb1_tmp;
static __ccmu_ahb1_ratio_reg0054_t CmuReg_ahb1_backup;
static __u32 CmuReg_fe_backup;
static __u32 CmuReg_be_backup;
static standby_pll_change_flag_t pll_flage_list;

static __u32 cpu_ms_loopcnt = 2000;

void standby_delay(__u32 ms)
{
	int i;

	for(i=ms;i>0;i--);

	return ;
}

void standby_mdelay(__u32 ms)
{
    standby_delay(ms * cpu_ms_loopcnt);
}

/*
*********************************************************************************************************
*                           standby_clk_init
*
*Description: ccu init for platform standby
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 standby_clk_init(void)
{
	CmuReg = (__ccmu_reg_list_t *) IO_ADDRESS(AW_CCM_BASE);

	return 0;
}

/*
*********************************************************************************************************
*                           standby_clk_exit
*
*Description: ccu exit for platform standby
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 standby_clk_exit(void)
{
	return 0;
}

static __u32 ccmu_reg_ahb1;

__s32 standby_clk_bus2Losc(void)
{

	ccmu_reg_ahb1 = readl(&(CmuReg->Ahb1Div));

	CmuReg->Ahb1Div.bits.Ahb1ClkSrc = 0;
	
	CmuReg->Ahb1Div.bits.Apb1Div = 0;
	CmuReg->Ahb1Div.bits.Ahb1PreDiv = 0;
	CmuReg->Ahb1Div.bits.Ahb1Div = 0;
	CmuReg->Ahb1Div.bits.HclkcDiv = 0;
		
	return 0;
}

__s32 standby_clk_busReset(void)
{
#if 1
	writel(ccmu_reg_ahb1, &(CmuReg->Ahb1Div));
#else
	CmuReg->Ahb1Div.bits.Apb1Div  = 1;
	CmuReg->Ahb1Div.bits.Ahb1PreDiv = 2;
	CmuReg->Ahb1Div.bits.Ahb1Div = 0;
	CmuReg->Ahb1Div.bits.HclkcDiv = 1;

	CmuReg->Ahb1Div.bits.Ahb1ClkSrc = 3;
#endif

	return 0;
}

__s32 standby_clk_pll_disable(void)
{
	if(CmuReg->Pll1Ctl.bits.PLLEn)
	{
		pll_flage_list.pllCpu = 1;
		CmuReg->Pll1Ctl.bits.PLLEn = 0;
	}

	if(CmuReg->Pll2Ctl.PLLEn)
	{
		pll_flage_list.pllAudio = 1;
		CmuReg->Pll2Ctl.PLLEn = 0;
	}
	
	if(CmuReg->Pll3Ctl.PLLEn)
	{
		pll_flage_list.pllVideo0 = 1;
		CmuReg->Pll3Ctl.PLLEn = 0;
	}
	
	if(CmuReg->Pll4Ctl.PLLEn)
	{
		pll_flage_list.pllVe = 1;
		CmuReg->Pll4Ctl.PLLEn = 0;
	}

	if(CmuReg->Pll5Ctl.PLLEn)
	{
		pll_flage_list.pllDram = 1;
		CmuReg->Pll5Ctl.PLLEn = 0;
	}

	if(CmuReg->Pll6Ctl.PLLEn)
	{
		pll_flage_list.pllPeriph0 = 1;
		CmuReg->Pll6Ctl.PLLEn = 0;
	}
}

__s32 standby_clk_pll_enable(void)
{
	if(pll_flage_list.pllCpu)
	{
		pll_flage_list.pllCpu = 0;
		CmuReg->Pll1Ctl.bits.PLLEn = 1;
	}

	if(pll_flage_list.pllAudio)
	{
		pll_flage_list.pllAudio = 0;
		CmuReg->Pll2Ctl.PLLEn = 1;
	}
	
	if(pll_flage_list.pllVideo0)
	{
		pll_flage_list.pllVideo0 = 0;
		CmuReg->Pll3Ctl.PLLEn = 1;
	}
	
	if(pll_flage_list.pllVe)
	{
		pll_flage_list.pllVe = 0;
		CmuReg->Pll4Ctl.PLLEn = 1;
	}

	if(pll_flage_list.pllDram)
	{
		pll_flage_list.pllDram = 0;
		CmuReg->Pll5Ctl.PLLEn = 1;
	}

	if(pll_flage_list.pllPeriph0)
	{
		pll_flage_list.pllPeriph0 = 0;
		CmuReg->Pll6Ctl.PLLEn = 1;
	}
}

/*
*********************************************************************************************************
*                                     standby_clk_core2losc
*
* Description: switch core clock to 32k low osc.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_core2losc(void)
{
	unsigned int tmp;
	/*CmuReg->SysClkDiv.CpuClkSrc = 0; */
	/* cpu frequency is internal Losc hz */
	tmp = readl(&(CmuReg->SysClkDiv));
	tmp &= (~(0x00030000));
	writel(tmp, &(CmuReg->SysClkDiv));

	//cpu frequency is 32k hz
	cpu_ms_loopcnt = 1;

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_core2hosc
*
* Description: switch core clock to 24M high osc.
*
* Arguments  : none
*
*********************************************************************************************************
*/
__s32 standby_clk_core2hosc(void)
{
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	CmuReg_SysClkDiv_tmp.bits.CpuClkSrc = AC327_CLKSRC_HOSC;	/*24M OSC */
	CmuReg->SysClkDiv.dwval = CmuReg_SysClkDiv_tmp.dwval;

	//cpu frequency is 24M hz
	cpu_ms_loopcnt = 600;

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_core2pll
*
* Description: switch core clock to core pll.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_core2pll(void)
{
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	CmuReg_SysClkDiv_tmp.bits.CpuClkSrc = AC327_CLKSRC_PLL1;	/*pll cpu */
	CmuReg->SysClkDiv.dwval = CmuReg_SysClkDiv_tmp.dwval;

	//cpu frequency is 60M hz
	cpu_ms_loopcnt = 2000;

	return 0;
}

static __u32 sys_reg_a4;

__s32 standby_close_24M(void)
{	
	sys_reg_a4 = (*(volatile __u32 *)0xf1c000a4);
	(*(volatile __u32 *)0xf1c000a4) |= (0xa7<<24);
	(*(volatile __u32 *)0xf1c000a4) &= ~(7<<0);
	return 0;
}

__s32 standby_open_24M(void)
{
	(*(volatile __u32 *)0xf1c000a4) = sys_reg_a4;

	return 0;
}



/*
*********************************************************************************************************
*                                     standby_clk_ldodisable
*
* Description: disable LDO.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_ldodisable(void)
{
	unsigned int tmp;
	/*0xf1c00000 + 0xf4 (system_ctrl: pll ctrl reg1) */
	/*bit2: hosc; */
	/*bit1: ldo for analog; */
	/*bit0: ldo for digital */

	/*disable ldo */
	tmp = readl((0xf1c00000 + 0xf4));		// ????
	tmp &= (~(0x00000003));			// ????
	writel(tmp, (0xf1c00000 + 0xf4));		// ????

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_ldoenable
*
* Description: enable LDO.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_ldoenable(void)
{
	unsigned int tmp;
	/*0xf1c00000 + 0xf4 (system_ctrl: pll ctrl reg1) */
	/*bit2: hosc; */
	/*bit1: ldo for analog; */
	/*bit0: ldo for digital */

	/*enable ldo */
	tmp = readl((0xf1c00000 + 0xf4));  	// ????
	tmp |= ((0x00000003));				// ????	
	writel(tmp, (0xf1c00000 + 0xf4));		// ????

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_set_pll_factor
*
* Description: set pll factor, target cpu freq is 384M hz
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 standby_clk_set_pll_factor(struct pll_factor_t *pll_factor)
{
	__ccmu_pll1_reg0000_t pll1_ctrl;

	if (!pll_factor) {
		return -1;
	}

	pll1_ctrl.dwval = readl(&(CmuReg->Pll1Ctl));

	pll1_ctrl.bits.FactorN = pll_factor->FactorN;
	pll1_ctrl.bits.FactorK = pll_factor->FactorK;
	pll1_ctrl.bits.FactorM = pll_factor->FactorM;
	pll1_ctrl.bits.FactorP = pll_factor->FactorP;

	writel(pll1_ctrl.dwval, &(CmuReg->Pll1Ctl));
	/*busy_waiting(); */

	return 0;
}

/*
 *********************************************************************************************************
 *                                     standby_clk_get_pll_factor
 *
 * Description:
 *
 * Arguments  : none
 *
 * Returns    : 0;
 *********************************************************************************************************
 */

__s32 standby_clk_get_pll_factor(struct pll_factor_t *pll_factor)
{
	__ccmu_pll1_reg0000_t pll1_ctrl;

	if (!pll_factor) {
		return -1;
	}

	pll1_ctrl.dwval = readl(&(CmuReg->Pll1Ctl));

	pll_factor->FactorN = pll1_ctrl.bits.FactorN;
	pll_factor->FactorK = pll1_ctrl.bits.FactorK;
	pll_factor->FactorM = pll1_ctrl.bits.FactorM;
	pll_factor->FactorP = pll1_ctrl.bits.FactorP;

	/*busy_waiting(); */

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_bus_src_backup
*
* Description: switch ahb2->?
*		      ahb1->?.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_bus_src_backup(void)
{
	/*backup bus src cfg */
	/* backup ahb clk src */
	CmuReg_ahb1_backup.dwval = CmuReg->Ahb1Div.dwval;

	/* backup de/ee clk src */
	CmuReg_be_backup = CmuReg->Be0;
	CmuReg_fe_backup = CmuReg->Fe0;

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_bus_src_set
*
* Description: switch ahb2->ahb1->axi.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 standby_clk_bus_src_set(void)
{
	/* change ahb1 clock to axi */
	CmuReg_ahb1_tmp.dwval = CmuReg->Ahb1Div.dwval;
	CmuReg_ahb1_tmp.bits.Ahb1ClkSrc = AHB1_CLKSRC_AXI;
	CmuReg->Ahb1Div.dwval = CmuReg_ahb1_tmp.dwval;
	/* printk("CmuReg_ahb1_backup, %x!\n", CmuReg_ahb1_backup); */

#ifdef CONFIG_ARCH_SUN8IW10P1
	/* switch de clk to ahb */
	CmuReg->Be0 = 0x82000000;
#endif
	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_bus_src_restore
*
* Description: switch ahb2->?
*		      ahb1->?.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_bus_src_restore(void)
{
	/* restore ahb clk src */
	CmuReg->Ahb1Div.dwval = CmuReg_ahb1_backup.dwval;

	/* retore be/fe clk */
	CmuReg->Be0 = CmuReg_be_backup;
	CmuReg->Fe0 = CmuReg_fe_backup;

	return 0;
}

/*
*********************************************************************************************************
*                                     standby_clk_dramgating
*
* Description: gating dram clock.
*
* Arguments  : onoff    dram clock gating on or off;
*
* Returns    : 0;
*********************************************************************************************************
*/
void standby_clk_dramgating(int onoff)
{
	unsigned int tmp;
	tmp = readl(&(CmuReg->Pll5Ctl));

	if (onoff) {
		tmp |= (0x80000000);
	} else {
		tmp &= (~0x80000000);
	}

	writel(tmp, &(CmuReg->Pll5Ctl));

	return;
}

__u32 raw_lib_udiv(__u32 dividend, __u32 divisior)
{
	__u32 tmpDiv = (__u32) divisior;
	__u32 tmpQuot = 0;
	__s32 shift = 0;

	if (!divisior) {
		/* divide 0 error abort */
		return 0;
	}

	while (!(tmpDiv & ((__u32) 1 << 31))) {
		tmpDiv <<= 1;
		shift++;
	}

	do {
		if (dividend >= tmpDiv) {
			dividend -= tmpDiv;
			tmpQuot = (tmpQuot << 1) | 1;
		} else {
			tmpQuot = (tmpQuot << 1) | 0;
		}
		tmpDiv >>= 1;
		shift--;
	} while (shift >= 0)
		;

	return tmpQuot;
}

__u32 standby_clk_get_cpu_freq(void)
{
	__u32 FactorN = 1;
	__u32 FactorK = 1;
	__u32 FactorM = 1;
	__u32 FactorP = 1;
	__u32 reg_val = 0;
	__u32 cpu_freq = 0;

	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	/*get runtime freq: clk src + divider ratio */
	/*src selection */
	reg_val = CmuReg_SysClkDiv_tmp.bits.CpuClkSrc;
	if (0 == reg_val) {
		/*32khz osc */
		cpu_freq = 32;

	} else if (1 == reg_val) {
		/*hosc, 24Mhz */
		cpu_freq = 24000;	/*unit is khz */
	} else if (2 == reg_val || 3 == reg_val) {
		CmuReg_Pll1Ctl_tmp.dwval = CmuReg->Pll1Ctl.dwval;
		FactorN = CmuReg_Pll1Ctl_tmp.bits.FactorN + 1;
		FactorK = CmuReg_Pll1Ctl_tmp.bits.FactorK + 1;
		FactorM = CmuReg_Pll1Ctl_tmp.bits.FactorM + 1;
		FactorP = 1 << (CmuReg_Pll1Ctl_tmp.bits.FactorP);

		cpu_freq =
		    raw_lib_udiv(24000 * FactorN * FactorK, FactorP * FactorM);
	}
	/*printk("cpu_freq = dec(%d). \n", cpu_freq); */
	/*busy_waiting(); */

	return cpu_freq;
}
