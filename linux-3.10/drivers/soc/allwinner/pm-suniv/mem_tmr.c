#include "pm_i.h"

static __mem_tmr_reg_t *TmrReg;
static __u32 WatchDog1_Mod_Reg_Bak;

/* =================================================use for normal standby =======*/
/*
*****************************************************************************
*                                     TIMER INIT
*
* Description: initialise timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*****************************************************************************
*/
__s32 mem_tmr_init(void)
{
	u32 *base = 0;
	u32 len = 0;
	pm_get_dev_info("timer", 0, &base, &len);

	/* set timer register base */
	TmrReg = (__mem_tmr_reg_t *) (base);

	return 0;
}

/*
*****************************************************************************
*                                     TIMER EXIT
*
* Description: exit timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*****************************************************************************
*/
__s32 mem_tmr_exit(void)
{
	return 0;
}

/*
*****************************************************************************
*                                     TIMER save
*
* Description: save timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*****************************************************************************
*/
__s32 mem_tmr_save(__mem_tmr_reg_t *ptmr_state)
{
	/* backup timer registers */
	ptmr_state->IntCtl = TmrReg->IntCtl;
	ptmr_state->Tmr0Ctl = TmrReg->Tmr0Ctl;
	ptmr_state->Tmr0IntVal = TmrReg->Tmr0IntVal;
	ptmr_state->Tmr0CntVal = TmrReg->Tmr0CntVal;
	ptmr_state->Tmr1Ctl = TmrReg->Tmr1Ctl;
	ptmr_state->Tmr1IntVal = TmrReg->Tmr1IntVal;
	ptmr_state->Tmr1CntVal = TmrReg->Tmr1CntVal;
	ptmr_state->WDog1_Mode_Reg = TmrReg->WDog1_Mode_Reg;

	return 0;
}

/*
*****************************************************************************
*                                     TIMER restore
*
* Description: restore timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*****************************************************************************
*/
__s32 mem_tmr_restore(__mem_tmr_reg_t *ptmr_state)
{
	/* restore timer parameters */
	TmrReg->Tmr0IntVal = ptmr_state->Tmr0IntVal;
	TmrReg->Tmr0CntVal = ptmr_state->Tmr0CntVal;
	TmrReg->Tmr0Ctl = ptmr_state->Tmr0Ctl;
	TmrReg->Tmr1IntVal = ptmr_state->Tmr1IntVal;
	TmrReg->Tmr1CntVal = ptmr_state->Tmr1CntVal;
	TmrReg->Tmr1Ctl = ptmr_state->Tmr1Ctl;
	TmrReg->IntCtl = ptmr_state->IntCtl;
	TmrReg->WDog1_Mode_Reg = ptmr_state->WDog1_Mode_Reg;

	return 0;
}

/*
*****************************************************************************
*                           mem_tmr_set
*
*Description: set timer for wakeup system.
*
*Arguments  : second    time value for wakeup system.
*
*Return     : result, 0 - successed, -1 - failed;
*
*Notes      :
*
*****************************************************************************
*/
__s32 mem_tmr_set(__u32 second)
{
	/* config timer0 interrrupt */
	TmrReg->IntCtl = 0x1;
	TmrReg->IntSta = 0x1;

	/* config timer0 for mem */
	TmrReg->Tmr0Ctl = 0;
	TmrReg->Tmr0IntVal = second << 10;

	TmrReg->Tmr0IntVal += (TmrReg->Tmr0IntVal/10);
	TmrReg->Tmr0IntVal += (TmrReg->Tmr0IntVal/100);
	TmrReg->Tmr0IntVal += (2*TmrReg->Tmr0IntVal/1000 - 2);
	TmrReg->Tmr0IntVal += (52*TmrReg->Tmr0IntVal/10000);
	
	TmrReg->Tmr0Ctl &= ~(0x3 << 2);		/* clk src: 32K */
	TmrReg->Tmr0Ctl = (1 << 7) | (5 << 4);	/* single mode | prescale= /32; */
	TmrReg->Tmr0Ctl |= (1 << 1);			/* reload timer 0 interval value; */
	TmrReg->Tmr0Ctl |= (1 << 0);			/* start */

	return 0;
}




__u32 mem_tmr1_suspend_time_init(void)
{
        /* config timer1 interrrupt */
        TmrReg->IntCtl |= 0x2;
        TmrReg->IntSta |= 0x2;

        /* config timer1 for mem */
        TmrReg->Tmr1Ctl = 0;
        TmrReg->Tmr1IntVal = 0x7FFFFFFF;
        TmrReg->Tmr1Ctl &= ~(0x3 << 2);         /* clk src: 32K */
        TmrReg->Tmr1Ctl = (1 << 7) | (5 << 4);  /* single mode | prescale= /32; */
        TmrReg->Tmr1Ctl |= (1 << 1);                    /* reload timer 1 interval value; */
        TmrReg->Tmr1Ctl |= (1 << 0);                    /* start */

        return 0;
}

__u32 mem_tmr1_suspend_time_get(void)
{
	__u32 time = 0;

	time = (0x7FFFFFFF - TmrReg->Tmr1CntVal) >> 10;

	time -= (time/10);
	time -= (time/100);
	time -= (2*time/1000 - 2);
	time -= (52*time/10000);
	
	if(time > 2)
		time -= 2;
		
	return time;
}

