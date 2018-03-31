/*
**********************************************************************************************************************
*                                                    CCMU  FOR  1620
*                                             CCMU hardware registers definition
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : ccmu.h
*
* Author      : Jerry.Wang
*
* Version     : 1.1.0
*
* Date        : 2011-1-22 14:13:05
*
* Description : This file provides some definition of CCMU's hardware registers and BSP interfaces.
*             This file is very similar to file "ccmu.inc"; the two files should be modified at the
*             same time to keep coherence of information.
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* Jerry.Wang      2011-1-22       1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef _CCMU_H_
#define _CCMU_H_

#include  "hd_scrpt.h"

/* Offset */
#define CCMU_REG_o_PLL1_CTRL            0x00
#define CCMU_REG_o_PLL2_CTRL            0x08
#define CCMU_REG_o_PLL3_CTRL            0x10
#define CCMU_REG_o_PLL4_CTRL            0x18
#define CCMU_REG_o_PLL5_CTRL            0x20
#define CCMU_REG_o_PLL6_CTRL            0x28
#define CCMU_REG_o_PLL7_CTRL            0x30

#define CCMU_REG_o_OSC24M_CTRL			0x50
#define CCMU_REG_o_CPUAHBAPB0_RATIO		0x54
#define CCMU_REG_o_APB1_RATIO			0x58
#define CCMU_REG_o_AXI_GATING			0x5C
#define CCMU_REG_o_AHB0_GATING0			0x60
#define CCMU_REG_o_AHB0_GATING1			0x64
#define CCMU_REG_o_APB0_GATING			0x68
#define CCMU_REG_o_APB1_GATING			0x6C

#define CCMU_REG_o_NAND0                0x80
#define CCMU_REG_o_SD_MMC0              0x88
#define CCMU_REG_o_SD_MMC2              0x90

#define CCMU_REG_o_SPI0                 0xA0
#define CCMU_REG_o_SPI1                 0xA4
#define CCMU_REG_o_SPI2                 0xA8

#define CCMU_REG_o_AVS                  0x144

/* registers */
#define CCMU_REG_PLL1_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL1_CTRL   )
#define CCMU_REG_PLL2_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL2_CTRL   )
#define CCMU_REG_PLL3_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL3_CTRL   )
#define CCMU_REG_PLL4_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL4_CTRL   )
#define CCMU_REG_PLL5_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL5_CTRL   )
#define CCMU_REG_PLL6_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL6_CTRL   )
#define CCMU_REG_PLL7_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL7_CTRL   )

#define CCMU_REG_OSC24M_CTRL            ( CCMU_REGS_BASE + CCMU_REG_o_OSC24M_CTRL )
#define CCMU_REG_CPUAHBAPB0_RATIO		( CCMU_REGS_BASE + CCMU_REG_o_CPUAHBAPB0_RATIO   )
#define CCMU_REG_APB1_RATIO             ( CCMU_REGS_BASE + CCMU_REG_o_APB1_RATIO  )
#define CCMU_REG_AXI_GATING				( CCMU_REGS_BASE + CCMU_REG_o_AXI_GATING  )
#define CCMU_REG_AHB1_GATING0			( CCMU_REGS_BASE + CCMU_REG_o_AHB0_GATING0)
#define CCMU_REG_AHB1_GATING1			( CCMU_REGS_BASE + CCMU_REG_o_AHB0_GATING1)
#define CCMU_REG_APB0_GATING			( CCMU_REGS_BASE + CCMU_REG_o_APB0_GATING )
#define CCMU_REG_APB1_GATING			( CCMU_REGS_BASE + CCMU_REG_o_APB1_GATING )


#define CCMU_REG_NAND                   ( CCMU_REGS_BASE + CCMU_REG_o_NAND        )
#define CCMU_REG_SD_MMC0                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC0     )
#define CCMU_REG_SD_MMC2                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC2     )

#define CCMU_REG_SPI0                   ( CCMU_REGS_BASE + CCMU_REG_o_SPI0        )
#define CCMU_REG_SPI1                   ( CCMU_REGS_BASE + CCMU_REG_o_SPI1        )
#define CCMU_REG_SPI2                   ( CCMU_REGS_BASE + CCMU_REG_o_SPI2        )

#define CCMU_REG_AVS					( CCMU_REGS_BASE + CCMU_REG_o_AVS         )

#endif    // #ifndef _CCMU_H_
