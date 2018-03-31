/*
 * Device Tree support for Allwinner A1X SoCs
 *
 * Copyright (C) 2012 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clocksource.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/i2c/pcf857x.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/system_misc.h>
#include <linux/sys_config.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/of_gpio.h>


#include "sunxi.h"

static void __iomem *wdt_base;

static void sun4i_restart(char mode, const char *cmd)
{

#define SUN4I_WATCHDOG_CTRL_REG		0x00
#define SUN4I_WATCHDOG_CTRL_RESTART		(1 << 0)
#define SUN4I_WATCHDOG_MODE_REG		0x04
#define SUN4I_WATCHDOG_MODE_ENABLE		(1 << 0)
#define SUN4I_WATCHDOG_MODE_RESET_ENABLE	(1 << 1)
	if (!wdt_base)
		return;

	/* Enable timer and set reset bit in the watchdog */
	writel(SUN4I_WATCHDOG_MODE_ENABLE | SUN4I_WATCHDOG_MODE_RESET_ENABLE,
	       wdt_base + SUN4I_WATCHDOG_MODE_REG);

	/*
	 * Restart the watchdog. The default (and lowest) interval
	 * value for the watchdog is 0.5s.
	 */
	writel(SUN4I_WATCHDOG_CTRL_RESTART, wdt_base + SUN4I_WATCHDOG_CTRL_REG);

	while (1) {
		mdelay(5);
		writel(SUN4I_WATCHDOG_MODE_ENABLE | SUN4I_WATCHDOG_MODE_RESET_ENABLE,
		       wdt_base + SUN4I_WATCHDOG_MODE_REG);
	}
}

static void sun8i_restart(char mode, const char *cmd)
{

#define SUN8I_WATCHDOG_CTRL_REG	        0x10
#define SUN8I_WATCHDOG_CTRL_RESTART	    ((1 << 0) | 0xA57)
#define SUN8I_WATCHDOG_CONFIG_REG       0x14
#define SUN8I_WATCHDOG_CONFIG_WHOLE_SYS	(1 << 0)
#define SUN8I_WATCHDOG_MODE_REG	        0x18
#define SUN8I_WATCHDOG_MODE_ENABLE      (1 << 0)
	if (!wdt_base)
		return;

	printk("WARN: enter func %s, para: mode = %s, cmd = %s. \n", __func__, &mode, cmd);
	
	/* config watchdog reset whole system.*/
	writel(SUN8I_WATCHDOG_CONFIG_WHOLE_SYS,
	       wdt_base + SUN8I_WATCHDOG_CONFIG_REG);

	/*
	 * start the watchdog. The default (and lowest) interval
	 * value for the watchdog is 0.5s.
	 */
	writel(SUN8I_WATCHDOG_MODE_ENABLE, wdt_base + SUN8I_WATCHDOG_MODE_REG);
	
	return ;
}

static struct of_device_id sunxi_restart_ids[] = {
	{ .compatible = "allwinner,sun4i-wdt", .data = sun4i_restart },
	{ .compatible = "allwinner,sun8i-wdt", .data = sun8i_restart },
	{ /*sentinel*/ }
};

static void sunxi_setup_restart(void)
{
	const struct of_device_id *of_id;
	struct device_node *np;

	np = of_find_matching_node(NULL, sunxi_restart_ids);
	if (WARN(!np, "unable to setup watchdog restart"))
		return;

	wdt_base = of_iomap(np, 0);
	WARN(!wdt_base, "failed to map watchdog base address");

	of_id = of_match_node(sunxi_restart_ids, np);
	WARN(!of_id, "restart function not available");

	arm_pm_restart = of_id->data;
}

static struct map_desc sunxi_io_desc[] __initdata = {
	{
		.virtual	= (unsigned long) SUNXI_REGS_VIRT_BASE,
		.pfn		= __phys_to_pfn(SUNXI_REGS_PHYS_BASE),
		.length		= SUNXI_REGS_SIZE,
		.type		= MT_DEVICE,
	},
#ifdef CONFIG_ARCH_SUNIVW1P1
        {
                .virtual        = (unsigned long)SUNXI_SRAM_VIRT_BASE,
                .pfn            = __phys_to_pfn(SUNXI_SRAM_PHYS_BASE),
                .length         = SUNXI_SRAM_SIZE,
                .type           = MT_MEMORY_ITCM,
        },

        {
                .virtual        = (unsigned long)SUNXI_SRAM_C2_VIRT_BASE,
                .pfn            = __phys_to_pfn(SUNXI_SRAM_C2_PHYS_BASE),
                .length         = SUNXI_SRAM_C2_SIZE,
                .type           = MT_MEMORY_ITCM,
        },
#endif
};

void __init sunxi_map_io(void)
{
	iotable_init(sunxi_io_desc, ARRAY_SIZE(sunxi_io_desc));
}

static void __init sunxi_timer_init(void)
{
	of_clk_init(NULL);
#ifdef CONFIG_COMMON_CLK_ENABLE_SYNCBOOT_EARLY
	clk_syncboot();
#endif
	clocksource_of_init();
}

#if defined(CONFIG_GPIO_PCF857X) || defined(CONFIG_GPIO_PCF857X_MODULE)
static struct pcf857x_platform_data pcf857x_data[] = {
	{
		.gpio_base = IO_EXP_PIN_BASE,
	}, {

	}
};

static struct i2c_board_info sunxi_i2c0_devices[] = {
	{
		I2C_BOARD_INFO("pcf8574a", 0x20),
		.platform_data = &pcf857x_data[0],
	}, {

	},
};
#endif

static void sunxi_power_off(void)
{
	struct device_node *node = NULL;
	struct gpio_config config;
	char pin_name[128] = {0};
	int cfg_val = 0;
	int gpio = 0;
	int ret = -1;

	struct gpio_config config_usb;
	int gpio_usb = 0;


	printk("%s: sunxi_power_off\n", __func__);

	node = of_find_node_by_type(NULL, "power_ctrl");
	if (NULL == node) {
		printk(KERN_ERR "%s: fail to find power_ctrl node\n", __func__);
		return;
	}

    #if 0
    gpio_usb = of_get_named_gpio_flags(node, "usb_drvbus_down", 0, (enum of_gpio_flags *)&config_usb);
	if (!gpio_is_valid(gpio_usb))
		return;
	
	ret = gpio_request(config_usb.gpio, NULL);
	if (0 != ret) {
		printk(KERN_ERR "%s: reques gpio=%d fail, ret=%d\n", __func__, config_usb.gpio, ret);
		return;
	}

    //memset(pin_name,0,128);
	sunxi_gpio_to_name(config_usb.gpio, pin_name);
	cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, config_usb.mul_sel);
	pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	
	if (config_usb.pull != GPIO_PULL_DEFAULT) {
		cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, config_usb.pull);
		pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	}
	
	if (config_usb.drv_level != GPIO_DRVLVL_DEFAULT) {
		cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV, config_usb.drv_level);
		pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	}
	
	if (config_usb.data != GPIO_DATA_DEFAULT) {
		cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, config_usb.data);
		printk("pin_config_set = %d \n",cfg_val);
		pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	}

	gpio_free(config_usb.gpio);

    #endif

	printk("sunxi_power_off  power_on\n");

	gpio = of_get_named_gpio_flags(node, "power_on", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(gpio))
		return;
	
	ret = gpio_request(config.gpio, NULL);
	if (0 != ret) {
		printk(KERN_ERR "%s: reques gpio=%d fail, ret=%d\n", __func__, config.gpio, ret);
		return;
	}

    //memset(pin_name,0,128);
	sunxi_gpio_to_name(config.gpio, pin_name);
	cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, config.mul_sel);
	pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	
	if (config.pull != GPIO_PULL_DEFAULT) {
		cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, config.pull);
		pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	}
	
	if (config.drv_level != GPIO_DRVLVL_DEFAULT) {
		cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV, config.drv_level);
		pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	}
	
	if (config.data != GPIO_DATA_DEFAULT) {
		cfg_val = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, config.data);
		printk("pin_config_set = %d \n",cfg_val);
		pin_config_set(SUNXI_PINCTRL, pin_name, cfg_val);
	}

	gpio_free(config.gpio);

	
	return;
}

static void sunxi_power_off_prepare(void)
{
	printk("%s: prepare power off  system\n", __func__);
	return;
}


static void __init sunxi_dt_init(void)
{
	sunxi_setup_restart();
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);

#if defined(CONFIG_GPIO_PCF857X) || defined(CONFIG_GPIO_PCF857X_MODULE)
	/*add platform data to system*/
	i2c_register_board_info(0, sunxi_i2c0_devices,
				ARRAY_SIZE(sunxi_i2c0_devices));
#endif

    if (!pm_power_off)
		pm_power_off = sunxi_power_off;

	if (!pm_power_off_prepare)
		pm_power_off_prepare = sunxi_power_off_prepare;

    
}

static const char * const sunxi_board_dt_compat[] = {
	"allwinner,sun4i-a10",
	"allwinner,sun5i-a13",
	"arm,sun8iw10p1",
	"arm,sunivw1p1",
	NULL,
};

DT_MACHINE_START(SUNXI_DT, "Allwinner A1X (Device Tree)")
	.init_machine	= sunxi_dt_init,
	.map_io		= sunxi_map_io,
	.init_irq	= irqchip_init,
	.init_time	= sunxi_timer_init,
	.dt_compat	= sunxi_board_dt_compat,
MACHINE_END

