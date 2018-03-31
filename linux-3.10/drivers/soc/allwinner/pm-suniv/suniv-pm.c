/*
 * drivers/soc/allwinner/pm-suniv/suniv-pm.c
 *
 * (C) Copyright 2011-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#if CONFIG_PM
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/major.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/power/aw_pm.h>

#include "pm_o.h"

#define AW_PM_DBG		1
#if  AW_PM_DBG
#define PM_DBG(format,args...)   printk("[pm]"format,##args)
#else
#define PM_DBG(format,args...)   do{}while(0)
#endif

standby_type_e standby_type = NON_STANDBY;
EXPORT_SYMBOL(standby_type);
standby_level_e standby_level = STANDBY_INITIAL;
EXPORT_SYMBOL(standby_level);

/*static volatile int enter_flag;*/
int standby_mode;
unsigned long time_to_wakeup = 0;
unsigned long suspend_time = 0;


extern char *standby_bin_start;
extern char *standby_bin_end;

struct aw_pm_info standby_info = {
	.standby_para = {
			 .event = CPU0_MEM_WAKEUP | CPU0_WAKEUP_KEY |
					CPU0_WAKEUP_TIMEOUT | CPU0_WAKEUP_PIO,   //ygy for gpio resume
			 .axp_event = CPUS_MEM_WAKEUP,
			 .timeout = 0,
			 },
	.pmu_arg = {
		    .twi_port = 0,
		    .dev_addr = 10,
		    },
};

static int aw_pm_valid(suspend_state_t state)
{
	PM_DBG("aw_pm_valid!\n");

	if (!((state > PM_SUSPEND_ON) && (state < PM_SUSPEND_MAX))) {
		PM_DBG("state (%d) invalid!\n", state);
		return 0;
	}

	printk("Notice: sunivw1p1 just support normal standby.\n");
	standby_mode = 0;
	standby_type = NORMAL_STANDBY;

	return 1;
}

int aw_pm_begin(suspend_state_t state)
{
	PM_DBG("aw_pm_begin!\n");

	return 0;
}

int aw_pm_prepare(void)
{
	PM_DBG("aw_pm_prepare!\n");
	return 0;
}

static int aw_pm_enter(suspend_state_t state)
{
	int (*standby) (void);
	
	PM_DBG("aw_pm_enter! enter state %d\n", state);
	mem_device_save();

	standby_info.standby_para.event = CPU0_MEM_WAKEUP | CPU0_WAKEUP_KEY |
					CPU0_WAKEUP_TIMEOUT | CPU0_WAKEUP_PIO;   //ygy for gpio resume
	standby_info.standby_para.timeout = time_to_wakeup;
	init_wakeup_src(standby_info.standby_para.event,
				standby_info.standby_para.gpio_enable_bitmap,
				standby_info.standby_para.cpux_gpiog_bitmap);	

	standby = (int (*)(void))SRAM_FUNC_START;
	
	memcpy((void *)SRAM_FUNC_START, (void *)&standby_bin_start, 
				(int)&standby_bin_end - (int)&standby_bin_start);

//	standby(&standby_info);
	standby();

	exit_wakeup_src(standby_info.standby_para.event,
				standby_info.standby_para.gpio_enable_bitmap,
				standby_info.standby_para.cpux_gpiog_bitmap);

	query_wakeup_source(&standby_info);	

	mem_device_restore();	

	return 0;
}

static void aw_pm_wake(void)
{
	PM_DBG("platform wakeup, wakesource is:0x%x\n", standby_info.standby_para.event);
}

void aw_pm_finish(void)
{
	PM_DBG("aw_pm_finish!\n");
}

void aw_pm_end(void)
{
	PM_DBG("aw_pm_end!\n");
}

void aw_pm_recover(void)
{
	PM_DBG("aw_pm_recover!\n");
}

static struct platform_suspend_ops aw_pm_ops = {
	.valid = aw_pm_valid,
	.begin = aw_pm_begin,
	.prepare = aw_pm_prepare,
	.enter = aw_pm_enter,
	.wake = aw_pm_wake,
	.finish = aw_pm_finish,
	.end = aw_pm_end,
	.recover = aw_pm_recover,
};

static int __init aw_pm_init(void)
{
	int result;

	PM_DBG("aw_pm_init!\n");

	mem_device_init();

	memset(&standby_info,0,sizeof(struct aw_pm_info));
	suspend_set_ops(&aw_pm_ops);

	return 0;

out_err:
	printk(KERN_ERR "register failed  for pmu device\n");

	return -ENODEV;
}

static void __exit aw_pm_exit(void)
{
	suspend_set_ops(NULL);

	PM_DBG("aw_pm_exit!\n");
}

module_param_named(time_to_wakeup, time_to_wakeup, ulong, S_IRUGO | S_IWUSR);
module_param_named(suspend_time, suspend_time, ulong, S_IRUGO | S_IWUSR);

//module_init(aw_pm_init);
subsys_initcall_sync(aw_pm_init);
module_exit(aw_pm_exit);
#endif
