#include "pm_i.h"

/*#define CHECK_RESTORE_STATUS*/
static __ccmu_reg_list_t *CmuReg;

static __ccmu_pll1_reg0000_t CmuReg_Pll1Ctl_tmp;
static __ccmu_sysclk_ratio_reg0050_t CmuReg_SysClkDiv_tmp;

/*
*********************************************************************************************************
*                           mem_clk_init
*
*Description: ccu init for platform mem, after switch mmu state, u need to re init.
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__ccmu_reg_list_t *mem_clk_init(__u32 mmu_flag)
{
	if (1 == mmu_flag) {
		CmuReg = (__ccmu_reg_list_t *) IO_ADDRESS(AW_CCM_BASE);
	} else {
		CmuReg = (__ccmu_reg_list_t *) (AW_CCM_BASE);
	}

	return CmuReg;
}

/*
*********************************************************************************************************
*                           mem_get_ba
*
*Description: get ccu mod base.
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__ccmu_reg_list_t *mem_get_ba(void)
{
	return CmuReg;
}

/*
*********************************************************************************************************
*                           mem_clk_save
*
*Description: save ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_save(struct clk_state *pclk_state)
{
	pclk_state->CmuReg = CmuReg = (__ccmu_reg_list_t *) IO_ADDRESS(AW_CCM_BASE);

	/*backup clk src and ldo */
	pclk_state->ccu_reg_back[0] = *(volatile __u32 *)&CmuReg->SysClkDiv;
	pclk_state->ccu_reg_back[1] = *(volatile __u32 *)&CmuReg->Ahb1Div;

	return 0;
}

/*
*********************************************************************************************************
*                           mem_clk_exit
*
*Description: restore ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_restore(struct clk_state *pclk_state)
{
	/* initialise the CCU io base */
	CmuReg = pclk_state->CmuReg;

	/*
	 * consider: pll6 already configed.
	 * config the bus to orginal status
	 */

	/*restore clk src */
	*(volatile __u32 *)&CmuReg->SysClkDiv = pclk_state->ccu_reg_back[0];
	*(volatile __u32 *)&CmuReg->Ahb1Div = pclk_state->ccu_reg_back[1];

	return 0;
}

__u32 mem_clk_get_cpu_freq(void)
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
		cpu_freq = raw_lib_udiv(24000 * FactorN * FactorK, FactorP * FactorM);
	}
	/*printk("cpu_freq = dec(%d). \n", cpu_freq); */
	/*busy_waiting(); */

	return cpu_freq;
}
