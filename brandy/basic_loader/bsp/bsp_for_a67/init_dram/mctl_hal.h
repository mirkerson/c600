#ifndef   _MCTL_HAL_H   
#define   _MCTL_HAL_H

#include  "types.h"
#include  "bsp.h"
#include  "dram_for_debug.h"

#ifdef DRAM_PRINK_ENABLE
#define dram_dbg(fmt,args...)	//UART_printf2(fmt ,##args)
#else
#define dram_dbg(fmt,args...)
#endif

typedef struct __DRAM_PARA
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;
    unsigned int        dram_zq;
    unsigned int		dram_odt_en;
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
    unsigned int		dram_tpr7;
    unsigned int		dram_tpr8;
    unsigned int		dram_tpr9;
    unsigned int		dram_tpr10;
    unsigned int		dram_tpr11;
    unsigned int		dram_tpr12;
    unsigned int		dram_tpr13;
}__dram_para_t;
extern void auto_detect_dram_size(__dram_para_t *para);
extern void paraconfig(unsigned int *para, unsigned int mask, unsigned int value);
extern void local_delay (unsigned int n);
extern unsigned int mctl_init(void);
extern unsigned int mctl_init_dram(void);
extern unsigned int mctl_sys_init(__dram_para_t *para);
extern void auto_set_timing_para(__dram_para_t *para);
extern void auto_set_dram_para(__dram_para_t *para);
extern signed int init_DRAM(int type, __dram_para_t *para);
extern void mctl_com_init(__dram_para_t *para);
extern unsigned int mctl_soft_training(void);
extern unsigned int ccm_set_pll_ddr_clk(u32 pll_clk);
extern unsigned int ccm_set_dram_div(u32 div);
extern unsigned int set_pll_ddr1_clk(u32 pll_clk);
extern unsigned int memtester(u32 address,u32 size,u32 dat);

#endif  //_MCTL_HAL_H
