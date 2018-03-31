/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 * Copyright 2010, 2011 David Jander <david@protonic.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/spinlock.h>
#include <linux/sys_config.h>
#include <linux/device.h>


struct gpio_button_data {
	struct input_dev *input;
	struct timer_list timer;
	struct work_struct work;
	unsigned int timer_debounce;	/* in msecs */
	unsigned int gpio;
	unsigned int irq;
	spinlock_t lock;
};

static void gpio_keys_irq_timer(unsigned long _data)
{

}

static irqreturn_t gpio_keys_irq_isr(int irq, void *dev_id)
{
	struct gpio_button_data *pdata = (struct gpio_button_data *)dev_id;

	input_event(pdata->input, EV_KEY, KEY_C, 0);
	input_event(pdata->input, EV_KEY, KEY_C, 1);
	input_sync(pdata->input);

#if 0
	if (bdata->timer_debounce)
		mod_timer(&bdata->timer,
			jiffies + msecs_to_jiffies(bdata->timer_debounce));
#endif
	return IRQ_HANDLED;
}

static int gpio_key_probe(struct platform_device *pdev)
{
	int error;
	struct gpio_button_data *pdata = NULL;
	struct device_node *np = pdev->dev.of_node;
	struct gpio_config config;
	struct device *dev = &pdev->dev;
	const char *used_status = NULL;

	pdata = devm_kzalloc(&pdev->dev, sizeof(struct gpio_button_data), GFP_KERNEL);
	if(!pdata) {
		dev_err(dev, "failed to allocate memory\n");
		return -ENOMEM;
	}
	error = of_property_read_string(np, "status", &used_status);
	if (error) {
		dev_err(dev, "failed to wireless moduel status\n");
		return -EINVAL;
	}else if (!strcmp(used_status, "okay")) {
	}else {
		return -EINVAL;
	}

	pdata->gpio = of_get_named_gpio_flags(np, "gpio-key", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(pdata->gpio)) {
		dev_err(dev, "failed to get gpio\n");
		return -EINVAL;
	} 
	pdata->irq = gpio_to_irq(pdata->gpio);

	error = of_property_read_u32(np, "debounce",&pdata->timer_debounce);
	if (error < 0) {
		dev_err(dev, "failed to get debounce\n");
		return -EINVAL;
	}

	pdata->input = input_allocate_device();
	if (!pdata->input) {
		dev_err(dev, "failed to allocate input device\n");
		error = -ENOMEM;
		return error;
	}
	platform_set_drvdata(pdev, pdata);
	input_set_drvdata(pdata->input, pdata);
	pdata->input->name = pdev->name;
	pdata->input->phys = "wireless-key/input0";
	pdata->input->dev.parent = &pdev->dev;

	pdata->input->id.bustype = BUS_HOST;
	pdata->input->id.vendor = 0x0001;
	pdata->input->id.product = 0x0001;
	pdata->input->id.version = 0x0100;

//	__set_bit(EV_REP, pdata->input->evbit);
	__set_bit(EV_KEY, pdata->input->evbit);
	__set_bit(KEY_C, pdata->input->keybit);
	
	setup_timer(&pdata->timer,
			gpio_keys_irq_timer, (unsigned long)pdata);
	
//	error = gpio_request_one(pdata->gpio, GPIOF_IN, "wireless key");
//	if (error < 0) {
//		dev_err(dev, "Failed to request GPIO %d, error %d\n",
//			pdata->gpio, error);
//		goto fail1;
//	}

	error = request_irq(pdata->irq, 
						  (irq_handler_t)gpio_keys_irq_isr,
						   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "wireless_key_irq", pdata);
	if(error) {
		goto fail2;
	}

	error = input_register_device(pdata->input);
	if (error) {
		dev_err(dev, "Unable to register input device, error: %d\n",
			error);
		goto fail3;
	}

	return 0;
	
 fail3:
	free_irq(pdata->irq, pdata);
fail2:
//	gpio_free(pdata->gpio);
// fail1:
	input_free_device(pdata->input);
	platform_set_drvdata(pdev, NULL);

	return error;
}

static int gpio_key_remove(struct platform_device *pdev)
{
	struct gpio_button_data *pdata = platform_get_drvdata(pdev);
	
	input_unregister_device(pdata->input);
	input_free_device(pdata->input);
	free_irq(pdata->irq, pdata);

	return 0;
}

static struct of_device_id gpio_key_of_match[] = {
	{ .compatible = "allwinner,wireless-key", },
	{ },
};

static struct platform_driver gpio_key_device_driver = {
	.probe		= gpio_key_probe,
	.remove		= gpio_key_remove,
	.driver		= {
		.name	= "wireless-key",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(gpio_key_of_match),
	}
};

static int __init gpio_key_init(void)
{
	int ret = 0;
	
	ret = platform_driver_register(&gpio_key_device_driver);
	return ret;
}

static void __exit gpio_key_exit(void)
{
	platform_driver_unregister(&gpio_key_device_driver);
}

late_initcall(gpio_key_init);
module_exit(gpio_key_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Key driver for GPIO");
MODULE_ALIAS("platform:gpio-key");
