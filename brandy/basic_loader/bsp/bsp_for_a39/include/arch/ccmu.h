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

#define CCMU_REG_o_AVS                  0xD4

#define CCM_PLL1_C0_CTRL  	    (CCMC_REGS_BASE+0x000)
#define CCM_PLL2_C1_CTRL 	    (CCMC_REGS_BASE+0x004)
#define CCM_PLL3_VIDEO_CTRL    	(CCMC_REGS_BASE+0x008)
#define CCM_PLL4_PERP0_CTRL    	(CCMC_REGS_BASE+0x00C)
#define CCM_PLL5_VE_CTRL  		(CCMC_REGS_BASE+0x010)
#define CCM_PLL6_DDR_CTRL  		(CCMC_REGS_BASE+0x014)
#define CCM_PLL7_VIDEO1_CTRL	(CCMC_REGS_BASE+0x018)
#define CCM_PLL8_VIDEO2_CTRL  	(CCMC_REGS_BASE+0x01C)
#define CCM_PLL9_GPU_CTRL		(CCMC_REGS_BASE+0x020)

#define CCM_PLL12_PERP1_CTRL    (CCMC_REGS_BASE+0x02C)

#define CCM_CPU_SOURCECTRL		(CCMC_REGS_BASE+0x050)
#define CCM_CLUSTER0_AXI_RATIO  (CCMC_REGS_BASE+0x054)
#define CCM_CLUSTER1_AXI_RATIO  (CCMC_REGS_BASE+0x058)
#define CCM_GTCLK_RATIO_CTRL	(CCMC_REGS_BASE+0x05C)

#define CCM_AHB0_RATIO_CTRL     (CCMC_REGS_BASE+0x060)
#define CCM_AHB1_RATIO_CTRL     (CCMC_REGS_BASE+0x064)
#define CCM_AHB2_RATIO_CTRL     (CCMC_REGS_BASE+0x068)
#define CCM_APB0_RATIO_CTRL     (CCMC_REGS_BASE+0x070)
#define CCM_APB1_RATIO_CTRL     (CCMC_REGS_BASE+0x074)
#define CCM_CCI400_RATIO_CTRL   (CCMC_REGS_BASE+0x078)

#define CCM_PLL_C0CPUX_BIAS    (CCMC_REGS_BASE+0x0a0)
#define CCM_PLL_C1CPUX_BIAS    (CCMC_REGS_BASE+0x0a4)
#define CCM_PLL_AUDIO_BIAS     (CCMC_REGS_BASE+0x0a8)
#define CCM_PLL_PERIPH0_BIAS   (CCMC_REGS_BASE+0x0ac)
#define CCM_PLL_VE_BIAS        (CCMC_REGS_BASE+0x0b0)
#define CCM_PLL_DDR_BIAS       (CCMC_REGS_BASE+0x0b4)
#define CCM_PLL_VIDEO0_BIAS    (CCMC_REGS_BASE+0x0b8)
#define CCM_PLL_VIDEO1_BIAS    (CCMC_REGS_BASE+0x0bc)
#define CCM_PLL_GPU_BIAS       (CCMC_REGS_BASE+0x0c0)
#define CCM_PLL_DE_BIAS        (CCMC_REGS_BASE+0x0c4)
#define CCM_PLL_ISP_BIAS       (CCMC_REGS_BASE+0x0c8)
#define CCM_PLL_PERIPH1_BIAS   (CCMC_REGS_BASE+0x0cc)

#define CCM_NAND0_SCLK_CTRL0	(CCMM_REGS_BASE+0x000)
#define CCM_NAND0_SCLK_CTRL1	(CCMM_REGS_BASE+0x004)
#define CCM_NAND1_SCLK_CTRL0	(CCMM_REGS_BASE+0x008)
#define CCM_NAND1_SCLK_CTRL1	(CCMM_REGS_BASE+0x00C)

#define CCMU_REG_SD_MMC0		(CCMM_REGS_BASE+0x010)
#define CCMU_REG_SD_MMC1		(CCMM_REGS_BASE+0x014)
#define CCMU_REG_SD_MMC2		(CCMM_REGS_BASE+0x018)
#define CCMU_REG_SD_MMC3		(CCMM_REGS_BASE+0x01C)

#define CCM_SPI0_SCLK_CTRL		(CCMM_REGS_BASE+0x030)
#define CCM_SPI1_SCLK_CTRL		(CCMM_REGS_BASE+0x034)
#define CCM_SPI2_SCLK_CTRL		(CCMM_REGS_BASE+0x038)
#define CCM_SPI3_SCLK_CTRL		(CCMM_REGS_BASE+0x03C)

#define CCM_USHPHY0_SCLK_CTRL	readl(CCMM_REGS_BASE+0x050)

#define CCM_AVS_SCLK_CTRL       readl(CCMM_REGS_BASE+0x0D4)

#define CCM_AHB0_GATE0_CTRL     (CCMM_REGS_BASE+0x180)
#define CCM_AHB1_GATE0_CTRL     (CCMM_REGS_BASE+0x184)
#define CCM_AHB2_GATE0_CTRL     (CCMM_REGS_BASE+0x188)
#define CCM_APB0_GATE0_CTRL     (CCMM_REGS_BASE+0x190)
#define CCM_APB1_GATE0_CTRL     (CCMM_REGS_BASE+0x194)

#define CCM_AHB0_RST_REG0       (CCMM_REGS_BASE+0x1A0)
#define CCM_AHB1_RST_REG0       (CCMM_REGS_BASE+0x1A4)
#define CCM_AHB2_RST_REG0       (CCMM_REGS_BASE+0x1A8)
#define CCM_APB0_RST_REG0       (CCMM_REGS_BASE+0x1B0)
#define CCM_APB1_RST_REG0       (CCMM_REGS_BASE+0x1B4)

#define CCMU_REG_AVS					readl( CCMM_REGS_BASE + CCMU_REG_o_AVS         )



#endif    // #ifndef _CCMU_H_
