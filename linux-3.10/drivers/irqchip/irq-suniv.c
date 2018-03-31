/*
 * Allwinner A1X SoCs IRQ chip driver.
 *
 * Copyright (C) 2012 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Benn Huang <benn@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <asm/exception.h>
#include <asm/mach/irq.h>

#include "irqchip.h"

#define SUNIV_IRQ_VECTOR_REG		0x00
#define SUNIV_IRQ_NMI_CTRL_REG		0x0c
#define SUNIV_IRQ_PENDING_REG(x)	(0x10 + 0x4 * x)
#define SUNIV_IRQ_ENABLE_REG(x)		(0x20 + 0x4 * x)
#define SUNIV_IRQ_MASK_REG(x)		(0x30 + 0x4 * x)

static void __iomem *suniv_irq_base;
static struct irq_domain *suniv_irq_domain;

static asmlinkage void __exception_irq_entry suniv_handle_irq(struct pt_regs *regs);

void suniv_irq_ack(struct irq_data *irqd)
{
	unsigned int irq = irqd_to_hwirq(irqd);
	unsigned int irq_off = irq % 32;
	int reg = irq / 32;
	u32 val;

	val = readl(suniv_irq_base + SUNIV_IRQ_PENDING_REG(reg));
	writel(val | (1 << irq_off),
	       suniv_irq_base + SUNIV_IRQ_PENDING_REG(reg));
}

static void suniv_irq_mask(struct irq_data *irqd)
{
	unsigned int irq = irqd_to_hwirq(irqd);
	unsigned int irq_off = irq % 32;
	int reg = irq / 32;
	u32 val;

	val = readl(suniv_irq_base + SUNIV_IRQ_ENABLE_REG(reg));
	writel(val & ~(1 << irq_off),
	       suniv_irq_base + SUNIV_IRQ_ENABLE_REG(reg));
}

static void suniv_irq_unmask(struct irq_data *irqd)
{
	unsigned int irq = irqd_to_hwirq(irqd);
	unsigned int irq_off = irq % 32;
	int reg = irq / 32;
	u32 val;

	val = readl(suniv_irq_base + SUNIV_IRQ_ENABLE_REG(reg));
	writel(val | (1 << irq_off),
	       suniv_irq_base + SUNIV_IRQ_ENABLE_REG(reg));
}

static struct irq_chip suniv_irq_chip = {
	.name		= "suniv_irq",
	.irq_ack	= suniv_irq_ack,
	.irq_mask	= suniv_irq_mask,
	.irq_unmask	= suniv_irq_unmask,
};

static int suniv_irq_map(struct irq_domain *d, unsigned int virq,
			 irq_hw_number_t hw)
{
	irq_set_chip_and_handler(virq, &suniv_irq_chip,
				 handle_level_irq);
	set_irq_flags(virq, IRQF_VALID | IRQF_PROBE);

	return 0;
}

static struct irq_domain_ops suniv_irq_ops = {
	.map = suniv_irq_map,
	.xlate = irq_domain_xlate_onecell,
};

static int __init suniv_of_init(struct device_node *node,
				struct device_node *parent)
{
	suniv_irq_base = of_iomap(node, 0);
	if (!suniv_irq_base)
		panic("%s: unable to map IC registers\n",
			node->full_name);

	/* Disable all interrupts */
	writel(0, suniv_irq_base + SUNIV_IRQ_ENABLE_REG(0));
	writel(0, suniv_irq_base + SUNIV_IRQ_ENABLE_REG(1));

	/* Mask all the interrupts */
	writel(0, suniv_irq_base + SUNIV_IRQ_MASK_REG(0));
	writel(0, suniv_irq_base + SUNIV_IRQ_MASK_REG(1));

	/* Clear all the pending interrupts */
	writel(0xffffffff, suniv_irq_base + SUNIV_IRQ_PENDING_REG(0));
	writel(0xffffffff, suniv_irq_base + SUNIV_IRQ_PENDING_REG(1));

	/* Configure the external interrupt source type */
	writel(0x00, suniv_irq_base + SUNIV_IRQ_NMI_CTRL_REG);

	suniv_irq_domain = irq_domain_add_linear(node, 3 * 32,
						 &suniv_irq_ops, NULL);
	if (!suniv_irq_domain)
		panic("%s: unable to create IRQ domain\n", node->full_name);

	set_handle_irq(suniv_handle_irq);

	return 0;
}
IRQCHIP_DECLARE(allwinner_suniv_intc, "allwinner,suniv-intc", suniv_of_init);

static asmlinkage void __exception_irq_entry suniv_handle_irq(struct pt_regs *regs)
{
	u32 irq, hwirq;

	hwirq = readl(suniv_irq_base + SUNIV_IRQ_VECTOR_REG) >> 2;
	while (hwirq != 0) {
		irq = irq_find_mapping(suniv_irq_domain, hwirq);
		handle_IRQ(irq, regs);
		hwirq = readl(suniv_irq_base + SUNIV_IRQ_VECTOR_REG) >> 2;
	}
}
