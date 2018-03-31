#ifndef _PM_I_H
#define _PM_I_H

/*
 * Copyright (c) 2011-2015 njubie@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include "pm_def_i.h"
#include "pm_types.h"
#include "pm.h"

#include <linux/power/aw_pm.h>
#include <stdarg.h>

#if defined(CONFIG_ARCH_SUNIVW1P1)
#define readb(addr)		(*((volatile unsigned char  *)(addr)))
#define readw(addr)		(*((volatile unsigned short *)(addr)))
#define readl(addr)		(*((volatile unsigned long  *)(addr)))
#define writeb(v, addr)		(*((volatile unsigned char  *)(addr)) = (unsigned char)(v))
#define writew(v, addr)		(*((volatile unsigned short *)(addr)) = (unsigned short)(v))
#define writel(v, addr)		(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))
#endif

void standby_delay(int cycle);
void standby_delay_cycle(int cycle);

#endif /*_PM_I_H*/

