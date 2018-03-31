/*
 * drivers/usb/sunxi_usb/manager/usb_hcd_servers.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2011-4-14, create this file
 *
 * usb host contoller driver, service function set.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include  "../include/sunxi_usb_config.h"
#include  "usb_hcd_servers.h"

int sunxi_usb_disable_hcd0(void);
int sunxi_usb_enable_hcd0(void);

int sunxi_usb_disable_hcd(__u32 usbc_no)
{
#ifndef  SUNXI_USB_FPGA
	if (usbc_no == 0) {
		#if defined(CONFIG_USB_SUNXI_USB0_OTG) || defined(USB_SUNXI_USB0_HOST_ONLY)
			sunxi_usb_disable_hcd0();
		#endif
	} 
#endif
	return 0;
}
EXPORT_SYMBOL(sunxi_usb_disable_hcd);

int sunxi_usb_enable_hcd(__u32 usbc_no)
{
#ifndef  SUNXI_USB_FPGA
	if (usbc_no == 0) {
		#if defined(CONFIG_USB_SUNXI_USB0_OTG) || defined(USB_SUNXI_USB0_HOST_ONLY)
			sunxi_usb_enable_hcd0();
		#endif
	} 
#endif
	return 0;
}
EXPORT_SYMBOL(sunxi_usb_enable_hcd);

