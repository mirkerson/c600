/*
 * Copyright (c) 2011-2020 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include "pm_o.h"

static struct gpio_state saved_gpio_state;
static struct ccm_state saved_ccm_state;
static struct sram_state saved_sram_state;
static __mem_tmr_reg_t saved_tmr_state;

extern unsigned long suspend_time;

extern unsigned long time_to_wakeup;

static void mem_enable_nmi(void)
{
#if 0
	u32 tmp = 0;

	tmp = readl((volatile void *)0xf1c2040c);
	tmp |= ((0x0000001));
	writel(tmp, (volatile void *)0xf1c2040c);
#endif
	return;
}


void init_wakeup_src(unsigned int event, unsigned int gpio_enable_bitmap, unsigned int cpux_gpiog_bitmap)
{
	if (event & CPU0_WAKEUP_EXINT) {
		mem_enable_int(INT_SOURCE_EXTNMI);
		mem_enable_nmi();
	}

	if (event & CPU0_WAKEUP_KEY) {
		mem_key_init();
		mem_enable_int(INT_SOURCE_LRADC);
	}
	
	if (event & CPU0_WAKEUP_TIMEOUT) {
		if (standby_info.standby_para.timeout) {
			pr_info("wakeup sys in %d sec later.\n", standby_info.standby_para.timeout);
			mem_tmr_set(standby_info.standby_para.timeout);
			mem_enable_int(INT_SOURCE_TIMER0);
		}
	}


	suspend_time=0;
	mem_tmr1_suspend_time_init();

	if (event & CPU0_WAKEUP_PIO) {
		
		void __iomem *gpiod_irq_enable = ioremap(0x01C20800 + 0x210, 0x100);
		unsigned int tmp = readl(gpiod_irq_enable);
		tmp	|= (0x1 << 14) | (0x1 << 13);     //ygy for gpiod resume
		writel(tmp, gpiod_irq_enable);    
		iounmap(gpiod_irq_enable);

		mem_enable_int(INT_SOURCE_PIOD);
	}
	
#if 0	
	if (event & CPU0_WAKEUP_IR) {
		mem_ir_init();
		mem_enable_int(INT_SOURCE_IR0);
	}

	if (event & CPU0_WAKEUP_USB) {
		mem_usb_init();
		mem_enable_int(INT_SOURCE_USB0);
	}

	if (event & CPUS_WAKEUP_GPIO) {
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('A')))
			mem_enable_int(INT_SOURCE_GPIOA);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('B')))
			mem_enable_int(INT_SOURCE_GPIOB);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('C')))
			mem_enable_int(INT_SOURCE_GPIOC);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('D')))
			mem_enable_int(INT_SOURCE_GPIOD);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('E')))
			mem_enable_int(INT_SOURCE_GPIOE);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('F')))
			mem_enable_int(INT_SOURCE_GPIOF);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('G')))
			mem_enable_int(INT_SOURCE_GPIOG);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('H')))
			mem_enable_int(INT_SOURCE_GPIOH);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('I')))
			mem_enable_int(INT_SOURCE_GPIOI);
		if (cpux_gpiog_bitmap & (WAKEUP_GPIO_GROUP('J')))
			mem_enable_int(INT_SOURCE_GPIOJ);
		mem_pio_clk_src_init();
	}
#endif
	return;
}

void exit_wakeup_src(unsigned int event, unsigned int gpio_enable_bitmap, unsigned int cpux_gpiog_bitmap)
{
#if 0
	/* exit standby module */
	if (event & CPUS_WAKEUP_GPIO)
		mem_pio_clk_src_exit();

	if (event & CPU0_WAKEUP_USB)
		mem_usb_exit();

	if (event & CPU0_WAKEUP_IR)
		mem_ir_exit();

#endif

	if (standby_info.standby_para.timeout)
		time_to_wakeup = 0;

	suspend_time = mem_tmr1_suspend_time_get();

	if (event & CPU0_WAKEUP_KEY)
		mem_key_exit();

	return;
}

void mem_device_init(void)
{
	mem_tmr_init();
	mem_gpio_init();
	mem_sram_init();
	mem_int_init();
	mem_clk_init(1);

	return;
}

void mem_device_save(void)
{
	/* backup device state */
	mem_twi_save(0);
	mem_ccu_save(&(saved_ccm_state));
	mem_clk_save(&(saved_clk_state));
	mem_tmr_save(&(saved_tmr_state));
	mem_gpio_save(&(saved_gpio_state));

	mem_sram_save(&(saved_sram_state));
	mem_int_save();

	return;
}


void mem_device_restore(void)
{
	mem_sram_restore(&(saved_sram_state));
	mem_gpio_restore(&(saved_gpio_state));
	mem_tmr_restore(&(saved_tmr_state));
	mem_clk_restore(&(saved_clk_state));
	mem_ccu_restore(&(saved_ccm_state));
	mem_int_restore();
	mem_tmr_exit();
	mem_twi_restore();

	return;
}


void query_wakeup_source(struct aw_pm_info *arg)
{
	arg->standby_para.event = 0;

	arg->standby_para.event |=
	    ((mem_query_int(INT_SOURCE_EXTNMI)) ? 0 : CPU0_WAKEUP_EXINT);
	arg->standby_para.event |=
	    (mem_query_int(INT_SOURCE_USB0) ? 0 : CPU0_WAKEUP_USB);
	arg->standby_para.event |=
	    mem_query_int(INT_SOURCE_LRADC) ? 0 : CPU0_WAKEUP_KEY;
	arg->standby_para.event |=
	    mem_query_int(INT_SOURCE_IR) ? 0 : CPU0_WAKEUP_IR;
	arg->standby_para.event |=
	    mem_query_int(INT_SOURCE_TIMER0) ? 0 : CPU0_WAKEUP_TIMEOUT;
	arg->standby_para.event |=
	    mem_query_int(INT_SOURCE_PIOD) ? 0 : CPUS_WAKEUP_GPIO;
	arg->standby_para.event |=
	    mem_query_int(INT_SOURCE_PIOE) ? 0 : CPUS_WAKEUP_GPIO;
	arg->standby_para.event |=
	    mem_query_int(INT_SOURCE_PIOF) ? 0 : CPUS_WAKEUP_GPIO;
}


int fetch_and_save_dram_para(dram_para_t *pstandby_dram_para)
{
	s32 ret = -EINVAL;
#if 0
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, "allwinner,dram");
	if (IS_ERR(np)) {
		pr_err("get [allwinner, dram] device node error\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_clk",
				   &pstandby_dram_para->dram_clk);
	if (ret) {
		pr_err("standby :get dram_clk err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_type",
				   &pstandby_dram_para->dram_type);
	if (ret) {
		pr_err("standby :get dram_type err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_zq", &pstandby_dram_para->dram_zq);
	if (ret) {
		pr_err("standby :get dram_zq err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_odt_en",
				   &pstandby_dram_para->dram_odt_en);
	if (ret) {
		pr_err("standby :get dram_odt_en err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_para1",
				   &pstandby_dram_para->dram_para1);
	if (ret) {
		pr_err("standby :get dram_para1 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_para2",
				   &pstandby_dram_para->dram_para2);
	if (ret) {
		pr_err("standby :get dram_para2 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_mr0",
				   &pstandby_dram_para->dram_mr0);
	if (ret) {
		pr_err("standby :get dram_mr0 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_mr1",
				   &pstandby_dram_para->dram_mr1);
	if (ret) {
		pr_err("standby :get dram_mr1 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_mr2",
				   &pstandby_dram_para->dram_mr2);
	if (ret) {
		pr_err("standby :get dram_mr2 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_mr3",
				   &pstandby_dram_para->dram_mr3);
	if (ret) {
		pr_err("standby :get dram_mr3 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr0",
				   &pstandby_dram_para->dram_tpr0);
	if (ret) {
		pr_err("standby :get dram_tpr0 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr1",
				   &pstandby_dram_para->dram_tpr1);
	if (ret) {
		pr_err("standby :get dram_tpr1 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr2",
				   &pstandby_dram_para->dram_tpr2);
	if (ret) {
		pr_err("standby :get dram_tpr2 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr3",
				   &pstandby_dram_para->dram_tpr3);
	if (ret) {
		pr_err("standby :get dram_tpr3 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr4",
				   &pstandby_dram_para->dram_tpr4);
	if (ret) {
		pr_err("standby :get dram_tpr4 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr5",
				   &pstandby_dram_para->dram_tpr5);
	if (ret) {
		pr_err("standby :get dram_tpr5 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr6",
				   &pstandby_dram_para->dram_tpr6);
	if (ret) {
		pr_err("standby :get dram_tpr6 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr7",
				   &pstandby_dram_para->dram_tpr7);
	if (ret) {
		pr_err("standby :get dram_tpr7 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr8",
				   &pstandby_dram_para->dram_tpr8);
	if (ret) {
		pr_err("standby :get dram_tpr8 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr9",
				   &pstandby_dram_para->dram_tpr9);
	if (ret) {
		pr_err("standby :get dram_tpr9 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr9",
				   &pstandby_dram_para->dram_tpr9);
	if (ret) {
		pr_err("standby :get dram_tpr9 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr10",
				   &pstandby_dram_para->dram_tpr10);
	if (ret) {
		pr_err("standby :get dram_tpr10 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr11",
				   &pstandby_dram_para->dram_tpr11);
	if (ret) {
		pr_err("standby :get dram_tpr11 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr12",
				   &pstandby_dram_para->dram_tpr12);
	if (ret) {
		pr_err("standby :get dram_tpr12 err.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dram_tpr13",
				   &pstandby_dram_para->dram_tpr13);
	if (ret) {
		pr_err("standby :get dram_tpr13 err.\n");
		return -EINVAL;
	}
#endif
	return ret;
}

