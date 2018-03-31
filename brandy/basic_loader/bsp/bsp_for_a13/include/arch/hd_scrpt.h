/*
************************************************************************************************************************
*                                                     suni define
*                            suni CPU hardware registers, memory, interrupts, ... define
*
*                             Copyright(C), 2009-2010, uLive Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : sunii.h
*
* Author : kevin.z
*
* Version : 1.1.0
*
* Date : 2009-9-7 10:53
*
* Description : This file provides some defination of suni's hardware registers, memory, interrupt
*             and so on. This file is very similar to file "sunii.inc"; the two files should be
*             modified at the same time to keep coherence of information.
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* kevin.z      2009-9-7 10:53    1.0.0        build the file
*
************************************************************************************************************************
*/

#ifndef  __HD_SCRIPT_H_
#define  __HD_SCRIPT_H_
#define  A13_EVB_PLATFORM

#define readl(addr)    				( *((volatile unsigned int *)(addr)))
#define writel(value, addr)			((*((volatile unsigned int *)(addr))) = (value))
/*
*********************************************************************************************************
*   hardware registers base define
*********************************************************************************************************
*/
#define REGS_BASE	   0x01C00000		//寄存器物理地址

// 物理地址
#define SRAM_REGS_BASE         ( REGS_BASE + 0x00000 )    //SRAM controller
#define DRAM_REGS_BASE         ( REGS_BASE + 0x01000 )    //SDRAM/DDR controller
#define DMAC_REGS_BASE         ( REGS_BASE + 0x02000 )    //DMA controller
//#define NAFC_REGS_BASE         ( REGS_BASE + 0x03000 )    //nand flash controller
#define TSC_REGS_BASE          ( REGS_BASE + 0x04000 )    //transport stream interface
#define SPIC0_REGS_BASE        ( REGS_BASE + 0x05000 )    //spi0
#define SPIC1_REGS_BASE        ( REGS_BASE + 0x06000 )    //spi1

#define SDMC0_REGS_BASE        ( REGS_BASE + 0x0f000 )    //sdmmc0 controller
#define SDMC1_REGS_BASE        ( REGS_BASE + 0x10000 )    //sdmmc1 controller
#define SDMC2_REGS_BASE        ( REGS_BASE + 0x11000 )    //sdmmc2 controller
#define SDMC3_REGS_BASE        ( REGS_BASE + 0x12000 )    //sdmmc3 controller

#define USBC0_REGS_BASE        ( REGS_BASE + 0x13000 )    //usb/otg 0 controller
#define USBC1_REGS_BASE        ( REGS_BASE + 0x14000 )    //usb/otg 1 controller

#define SPI0_REGS_BASE         ( REGS_BASE + 0x05000 )    //spi0
#define SPI1_REGS_BASE         ( REGS_BASE + 0x06000 )    //spi1
#define SPI2_REGS_BASE         ( REGS_BASE + 0x17000 )    //spi2

#define CCMU_REGS_BASE         ( REGS_BASE + 0x20000 )    //clock manager unit
#define INTC_REGS_BASE         ( REGS_BASE + 0x20400 )    //arm interrupt controller
#define PIOC_REGS_BASE         ( REGS_BASE + 0x20800 )    //general perpose I/O
#define TMRC_REGS_BASE         ( REGS_BASE + 0x20c00 )    //timer

#define UART0_REGS_BASE        ( REGS_BASE + 0x28000 )    //uart0 base
#define UART1_REGS_BASE        ( REGS_BASE + 0x28400 )    //uart1 base
#define UART2_REGS_BASE        ( REGS_BASE + 0x28800 )    //uart2 base
#define UART3_REGS_BASE        ( REGS_BASE + 0x28C00 )    //uart3 base

#define TWIC0_REGS_BASE        ( REGS_BASE + 0x2AC00 )    //twi0
#define TWIC1_REGS_BASE        ( REGS_BASE + 0x2B000 )    //twi1


#endif // end of #ifndef __HD_SCRIPT_H_
