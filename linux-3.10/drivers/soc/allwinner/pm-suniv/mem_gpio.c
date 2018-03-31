/*
 * Copyright (c) 2011-2020 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include "pm_types.h"
#include "pm_i.h"

static void *pio_pbase;
static u32 pio_len;

/*
*********************************************************************************************************
*                                       MEM gpio INITIALISE
*
* Description: mem gpio initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gpio_init(void)
{
	u32 *base = 0;

	pm_get_dev_info("pio", 0, &base, &pio_len);
	pio_pbase = base;

	return 0;
}

/*
*********************************************************************************************************
*                                       MEM gpio INITIALISE
*
* Description: mem gpio initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gpio_save(struct gpio_state *pgpio_state)
{
	int i = 0;

	/*save all the gpio reg */
	for (i = 0; i < (GPIO_REG_LENGTH); i++) {
		pgpio_state->gpio_reg_back[i] = *(volatile __u32 *)((pio_pbase) + i * 0x04);
	}
	return 0;
}

/*
*********************************************************************************************************
*                                       MEM gpio INITIALISE
*
* Description: mem gpio initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gpio_restore(struct gpio_state *pgpio_state)
{
	int i = 0;

	/*restore all the gpio reg */
	for (i = 0; i < (GPIO_REG_LENGTH); i++) {
		*(volatile __u32 *)((pio_pbase) + i * 0x04) = pgpio_state->gpio_reg_back[i];
	}

	return 0;
}
