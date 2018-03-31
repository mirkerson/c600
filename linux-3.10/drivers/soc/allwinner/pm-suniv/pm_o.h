#ifndef _PM_O_H
#define _PM_O_H

/*
 * Copyright (c) 2011-2015 njubie@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include "pm.h"
#include <linux/module.h>
#include <linux/suspend.h>
#include <asm/suspend.h>
#include <linux/cpufreq.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/major.h>
#include <linux/device.h>
#include <linux/console.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/cpu_pm.h>
#include <asm/system_misc.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/tlbflush.h>
#include <linux/power/aw_pm.h>
#include <asm/cacheflush.h>
#include <linux/arisc/arisc.h>

#include <linux/power/scenelock.h>
#include <linux/kobject.h>
#include <linux/ctype.h>
#include <linux/regulator/consumer.h>
#include <linux/power/axp_depend.h>
#include "../../../../kernel/power/power.h"

static struct clk_state saved_clk_state;

extern void init_wakeup_src(unsigned int event, unsigned int gpio_enable_bitmap, unsigned int cpux_gpiog_bitmap);
extern void exit_wakeup_src(unsigned int event, unsigned int gpio_enable_bitmap, unsigned int cpux_gpiog_bitmap);
extern void mem_device_init(void);
extern void mem_device_save(void);
extern void mem_device_restore(void);

extern void query_wakeup_source(struct aw_pm_info *arg);
extern struct aw_pm_info standby_info;

#endif
