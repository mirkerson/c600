//*****************************************************************************
//	Allwinner Technology, All Right Reserved. 2006-2010 Copyright (c)
//
//	File: 				mctl_hal.h
//
//	Description:  This file implements basic functions for AW1650 DRAM controller
//
//	History:
//              2013/04/18      Berg Xing       0.10    Initial version
//*****************************************************************************

#ifndef   _MCTL_HAL_H
#define   _MCTL_HAL_H

#include  "types.h"
#include  "bsp.h"
#include  "dram_for_debug.h"

//#define FPGA_PLATFORM
//#define LPDDR2_FPGA_S2C_2CS_2CH

#ifdef DRAM_PRINK_ENABLE
#  define dram_dbg(fmt,args...)	UART_printf2(fmt ,##args)
#else
#  define dram_dbg(fmt,args...)
#endif

typedef struct __DRAM_PARA
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;		//dram_type			DDR2: 2				DDR3: 3				LPDDR2: 6	DDR3L: 31
    unsigned int        dram_zq;
    unsigned int		dram_odt_en;

	//control configuration
	unsigned int		dram_para1;
    unsigned int		dram_para2;

	//timing configuration
	unsigned int		dram_mr0;
    unsigned int		dram_mr1;
    unsigned int		dram_mr2;
    unsigned int		dram_mr3;
    unsigned int		dram_tpr0;
    unsigned int		dram_tpr1;
    unsigned int		dram_tpr2;
    unsigned int		dram_tpr3;
    unsigned int		dram_tpr4;
    unsigned int		dram_tpr5;
   	unsigned int		dram_tpr6;

    //reserved for future use
    unsigned int		dram_tpr7;
    unsigned int		dram_tpr8;
    unsigned int		dram_tpr9;
    unsigned int		dram_tpr10;
    unsigned int		dram_tpr11;
    unsigned int		dram_tpr12;
    unsigned int		dram_tpr13;

}__dram_para_t;

extern __dram_para_t *dram_para;

extern void __msdelay(unsigned int delay);
extern void mctl_delay(unsigned int time);
extern unsigned int DRAMC_init(__dram_para_t *para);
extern void mctl_ch_init(__dram_para_t *para);
extern void mctl_sys_init(__dram_para_t *para);
extern void mctl_com_init(__dram_para_t *para);
extern signed int init_DRAM(int type, void *para);
extern void paraconfig(unsigned int *para, unsigned int mask, unsigned int value);
extern unsigned int dram_get_ahb1_clk(void);
extern unsigned int dram_get_axi_clk(void);
extern unsigned int dram_get_pll_periph_clk(void);
extern unsigned int dram_get_pll_cpux_clk(void);
extern void mctl_init(void);
extern void host_config(void);

#endif  //_MCTL_HAL_H










