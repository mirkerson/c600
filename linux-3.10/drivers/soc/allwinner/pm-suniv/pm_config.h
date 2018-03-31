#ifndef _PM_CONFIG_H
#define _PM_CONFIG_H

/*
* Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 as published by
* the Free Software Foundation.
*/
#include "pm_def_i.h"

#ifdef CONFIG_ARCH_SUNIVW1P1
/*hardware resource description*/
#include "pm_config-sunivw1p1.h"
#endif

/**********************************************platform separator *****************************************/

#define SRAM_FUNC_START		(0xf0000000)
#define SRAM_FUNC_START_PA	(0x00000000)

#define AW_SRAMCTRL_BASE	(SUNXI_SRAMCTRL_PBASE)
#define AW_CCM_BASE			(SUNXI_CCM_PBASE)
#define AW_LRADC_PBASE		(SUNXI_LRADC_PBASE)
#define AW_PIO_BASE			(SUNXI_PIO_PBASE)
#define AW_UART0_BASE		(SUNXI_UART0_PBASE)
#define AW_TWI0_BASE		(SUNXI_TWI0_PBASE)
#define AW_TWI1_BASE		(SUNXI_TWI1_PBASE)
#define AW_TWI2_BASE		(SUNXI_TWI2_PBASE)

#define GPIO_REG_LENGTH		((0x258+0x4)>>2)
#define SRAM_REG_LENGTH		((0xF0+0x4)>>2)
#define CCU_REG_LENGTH		((0x2d0+0x4)>>2)

#endif /*_PM_CONFIG_H*/
