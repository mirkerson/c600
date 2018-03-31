/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_int.h
* By      : gq.yang
* Version : v1.0
* Date    : 2012-11-3 20:13
* Descript: intterupt bsp for platform mem.
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_INT_H__
#define __MEM_INT_H__
#include "pm_config-sunivw1p1.h"

/* define interrupt source */
enum interrupt_source_e {
	INT_SOURCE_EXTNMI   = 0,   
	INT_SOURCE_UART0    = 1,   
	INT_SOURCE_UART1    = 2,   
	INT_SOURCE_UART2    = 3,   
	INT_SOURCE_SPDIF    = 5,   
	INT_SOURCE_IR       = 6,   
	INT_SOURCE_TWI0     = 7,   
	INT_SOURCE_TWI1     = 8,   
	INT_SOURCE_TWI2     = 9,   
	INT_SOURCE_SPI0     = 10,   
	INT_SOURCE_SPI1     = 11,   
	INT_SOURCE_TIMER0   = 13,  
	INT_SOURCE_TIMER1   = 14, 	
	INT_SOURCE_TIMER2   = 15,  
	INT_SOURCE_TOUCHPNL = 20,  
	INT_SOURCE_LRADC    = 22,  
	INT_SOURCE_USB0     = 26,  
	INT_SOURCE_PIOD     = 38,  
	INT_SOURCE_PIOE     = 39,  
	INT_SOURCE_PIOF     = 40,  
};

#define SW_INT_VECTOR_REG		(0x00)
#define SW_INT_BASE_ADR_REG		(0x04)
#define SW_NMI_INT_CTRL_REG		(0x0C)

#define SW_INT_PENDING_REG0		(0x10)
#define SW_INT_PENDING_REG1		(0x14)

#define SW_INT_ENABLE_REG0		(0x20)
#define SW_INT_ENABLE_REG1		(0x24)

#define SW_INT_MASK_REG0		(0x30)
#define SW_INT_MASK_REG1		(0x34)

#define SW_INT_RESP_REG0			(0x40)
#define SW_INT_RESP_REG1			(0x44)

#define SW_INT_FORCE_REG0		(0x50)
#define SW_INT_FORCE_REG1		(0x54)

#define SW_INT_PRIO_REG0			(0x60)
#define SW_INT_PRIO_REG1			(0x64)
#define SW_INT_PRIO_REG2			(0x68)
#define SW_INT_PRIO_REG3			(0x6C)

typedef struct __STANDBY_INT_REG
{
    volatile __u32   Vector;       /*0x00*/
    volatile __u32   BaseAddr;     /*0x04*/
    volatile __u32   reserved0;    /*0x08*/    
    volatile __u32   NmiIntCtrl;   /*0x0c*/
    volatile __u32   Pending0;     /*0x10*/
    volatile __u32   Pending1;     /*0x14*/
    volatile __u32   reserved1[2]; /*0x18*/    
    volatile __u32   Enable0;      /*0x20*/
    volatile __u32   Enable1;      /*0x24*/
    volatile __u32   reserved2[2]; /*0x28*/    
    volatile __u32   Mask0;        /*0x30*/
    volatile __u32   Mask1;        /*0x34*/
    volatile __u32   reserved3[2]; /*0x38*/        
    volatile __u32   Resp0;        /*0x40*/
    volatile __u32   Resp1;        /*0x44*/
    volatile __u32   reserved4[2]; /*0x48*/           
    volatile __u32   Force0;       /*0x50*/
    volatile __u32   Force1;       /*0x54*/
    volatile __u32   reserved5[2]; /*0x58*/         
    volatile __u32   Priority[4];  /*0x60*/
} __standby_int_reg_t;

extern __s32 mem_int_init(void);
extern __s32 mem_int_save(void);
extern __s32 mem_int_restore(void);
extern __s32 mem_enable_int(enum interrupt_source_e src);
extern __s32 mem_query_int(enum interrupt_source_e src);

#endif	/*__MEM_INT_H__*/

