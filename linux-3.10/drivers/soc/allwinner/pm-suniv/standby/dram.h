/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                               DRAM CSP Module
*
*                                    (c) Copyright 2006-2010, kevin.z.m China
*                                             All Rights Reserved
*
* File    : dram_i.h
* By      : kevin.z.m
* Version : v1.0
* Date    : 2011-1-25 14:47
* Descript: dram csp for aw1619;
* Update  : date                auther      ver     notes
*           2011-1-25 14:47     kevin.z.m   1.0     build the file;
*           2011-1-26 14:00     cpl         1.1     modify for aw1619 system
*********************************************************************************************************
*/
#ifndef __DRAM_H__
#define __DRAM_H__

//#include <stdint.h>
#include <linux/kernel.h>
//#define  DRAM_FUNCTION_TEST
//#define DRAM_REG_TEST
//#define DRAM_BASIC_RW_TEST
//#define DRAM_SELFREFRESH_TEST
//#define DRAM_DELAY_TEST
//#define DRAM_PAD_TEST
//#define DRAM_POWERDOWN_TSET
//#define DRAM_EMRS_TEST
//#define DRAM_INTERNAL_REF_TEST
//#define DRAM_CHANGE_CLK_TEST
//#define DRAM_DDR_SDR_TEST
//#define DRAM_ADDRESS_MAPPING_TEST
//#define DRAM_PAD_MODE0_TEST
//#define DRAM_PAD_MODE2_TEST
//#define STANDBY_TEST
//#define DRAM_FUNCTION_TEST



#define DRAMC_PHY_BASE      			0xf1C01000
#define DRAMC_MEM_SIZE      			0x400

#define __REG(x)    					(*(volatile unsigned int   *)(x))

#define DRAM_REG_SCONR                  __REG(DRAMC_PHY_BASE + 0x00)
#define DRAM_REG_STMG0R                 __REG(DRAMC_PHY_BASE + 0x04)
#define DRAM_REG_STMG1R                 __REG(DRAMC_PHY_BASE + 0x08)
#define DRAM_REG_SCTLR                  __REG(DRAMC_PHY_BASE + 0x0c)
#define DRAM_REG_SREFR                  __REG(DRAMC_PHY_BASE + 0x10)
#define DRAM_REG_SEXTMR                 __REG(DRAMC_PHY_BASE + 0x14)
#define DRAM_REG_DDLYR                  __REG(DRAMC_PHY_BASE + 0x24)
#define DRAM_REG_DADRR                  __REG(DRAMC_PHY_BASE + 0x28)
#define DRAM_REG_DVALR                  __REG(DRAMC_PHY_BASE + 0x2c)
#define DRAM_REG_DRPTR0                 __REG(DRAMC_PHY_BASE + 0x30)
#define DRAM_REG_DRPTR1                 __REG(DRAMC_PHY_BASE + 0x34)
#define DRAM_REG_DRPTR2                 __REG(DRAMC_PHY_BASE + 0x38)
#define DRAM_REG_DRPTR3                 __REG(DRAMC_PHY_BASE + 0x3c)
#define DRAM_REG_SEFR                   __REG(DRAMC_PHY_BASE + 0x40)
#define DRAM_REG_MAE	                __REG(DRAMC_PHY_BASE + 0x44)
#define DRAM_REG_ASPR                   __REG(DRAMC_PHY_BASE + 0x48)
#define DRAM_REG_SDLY0                  __REG(DRAMC_PHY_BASE + 0x4C)
#define DRAM_REG_SDLY1                  __REG(DRAMC_PHY_BASE + 0x50)
#define DRAM_REG_SDLY2                  __REG(DRAMC_PHY_BASE + 0x54)
#define DRAM_REG_MCR0					__REG(DRAMC_PHY_BASE + 0x100 + 4*0)
#define DRAM_REG_MCR1					__REG(DRAMC_PHY_BASE + 0x100 + 4*1)
#define DRAM_REG_MCR2					__REG(DRAMC_PHY_BASE + 0x100 + 4*2)
#define DRAM_REG_MCR3					__REG(DRAMC_PHY_BASE + 0x100 + 4*3)
#define DRAM_REG_MCR4					__REG(DRAMC_PHY_BASE + 0x100 + 4*4)
#define DRAM_REG_MCR5					__REG(DRAMC_PHY_BASE + 0x100 + 4*5)
#define DRAM_REG_MCR6					__REG(DRAMC_PHY_BASE + 0x100 + 4*6)
#define DRAM_REG_MCR7					__REG(DRAMC_PHY_BASE + 0x100 + 4*7)
#define DRAM_REG_MCR8					__REG(DRAMC_PHY_BASE + 0x100 + 4*8)
#define DRAM_REG_MCR9					__REG(DRAMC_PHY_BASE + 0x100 + 4*9)
#define DRAM_REG_MCR10					__REG(DRAMC_PHY_BASE + 0x100 + 4*10)
#define DRAM_REG_MCR11					__REG(DRAMC_PHY_BASE + 0x100 + 4*11)   
#define DRAM_REG_BWCR                   __REG(DRAMC_PHY_BASE + 0x140)


//PIO register for dram
#define DRAM_PIO_BASE       			0xf1c20800
#define DRAM_PIO_MEMSIZE    			0x400
#define SDR_PAD_DRV_REG                 __REG(DRAM_PIO_BASE + 0x2C0)
#define SDR_PAD_PUL_REG                 __REG(DRAM_PIO_BASE + 0x2C4)
#define SDR_VREF                        __REG(DRAM_PIO_BASE + 0x24)

//CCM register for dram
#define DRAM_CCM_BASE       			0xf1c20000
#define DRAM_CCM_SDRAM_PLL_REG          __REG(DRAM_CCM_BASE + 0x20)
#define DRAM_CCM_AHB1_GATE_REG          __REG(DRAM_CCM_BASE + 0x60)
#define DRAM_CCM_DRAM_GATING_REG		__REG(DRAM_CCM_BASE + 0x100)
#define DRAM_CCM_SIGMA_REG			    __REG(DRAM_CCM_BASE + 0x290)
#define DRAM_CCM_AHB1_RST_REG			__REG(DRAM_CCM_BASE + 0x2C0)

#define SDR_T_CAS               		0x2//CL
#define SDR_T_RAS               		0x8//120000ns>=tRAS>=42ns  SDR166
#define SDR_T_RCD               		0x3//tRCD>=15ns   SDR166
#define SDR_T_RP                		0x3//tRP>=15ns   SDR166
#define SDR_T_WR                		0x3//tWR>=15ns   SDR166
#define SDR_T_RFC              		    0xd//tRFC>=60ns   SDR166
#define SDR_T_XSR               		0xf9//tXSRD>=200CK
#define SDR_T_RC                		0xb//tRC>=60ns  SDR166
#define SDR_T_INIT              		0x8//
#define SDR_T_INIT_REF          		0x7
#define SDR_T_WTR               		0x2//tWTR>=2CK   SDR166
#define SDR_T_RRD               		0x2//tRRD>=12ns  SDR166
#define SDR_T_XP                		0x0// one clk is ok

typedef enum __DRAM_TYPE
{
    DRAM_TYPE_SDR=0,
    DRAM_TYPE_DDR=1,
    DRAM_TYPE_MDDR=2,

}__dram_type_t;


typedef struct __DRAM_PARA
{
    __u32           base;           // dram base address
    __u32           size;           // dram size, based on     (unit: MByte)
    __u32           clk;            // dram work clock         (unit: MHz)
    __u32           access_mode;    // 0: interleave mode 1: sequence mode
    __u32           cs_num;         // dram chip count  1: one chip  2: two chip
    __u32           ddr8_remap;     // for 8bits data width DDR 0: normal  1: 8bits
    __dram_type_t   sdr_ddr;           // ddr/ddr2/sdr/mddr/lpddr/...
    __u32           bwidth;         // dram bus width
    __u32           col_width;      // column address width
    __u32           row_width;      // row address width
    __u32           bank_size;      // dram bank count
    __u32           cas;            // dram cas
}__dram_para_t;

static int DRAMC_initial(void);
extern int DRAMC_delay_scan(void);
static void DRAMC_delay_tune(__u32 clk);
void CSP_DRAMC_set_autofresh_cycle(__u32 clk);
static __u32 DRAMC_para_setup(__dram_para_t *para);
static __u32 DRAMC_check_delay(__u32 bwidth);
extern __u32 CSP_DRAMC_check_type(__dram_para_t *para);
extern __u32 CSP_DRAMC_get_dram_size(__dram_para_t *para);
extern __s32 CSP_DRAMC_exit(void);
extern __u32 CSP_DRAMC_scan_readpipe(__dram_para_t *para);
extern void CSP_DRAMC_set_pad_drive(__u32 clk_freq);
int mctl_init(void);
extern __s32 CSP_DRAMC_init(__dram_para_t *dram_para, __s32 mode);
//extern uint32 dram_test(__dram_para_t *para);
//extern uint32 mctl_delay_test(void);
//extern void mctl_set_dq_delay(uint32 dq_num, uint32 delay_num);
//extern void mctl_set_dqs_delay(uint32 dqs_num, uint32 delay_num);
//extern uint32 mctl_pad_test(void);
extern void gate_dram_PLL(__u32 set);
extern __u32 CSP_DRAMC_set_emrs(__u32 emrs_id, __u32 emrs_val);
extern void CSP_DRAMC_enter_selfrefresh(void);
extern void CSP_DRAMC_exit_selfrefresh(void);
extern void CSP_DRAMC_power_down(__u32 down);
extern void mctl_change_clock_test(__dram_para_t *para,unsigned int clk);
//extern void dram_power_save_process(__dram_para_t *para);
extern void dram_power_save_process(void);
extern void  mctl_init_stadby(__dram_para_t *para);
extern unsigned int save_dram_delay[];

unsigned int memtester(u32 address,u32 size,u32 dat);

#endif  //__DRAM_I_H__
