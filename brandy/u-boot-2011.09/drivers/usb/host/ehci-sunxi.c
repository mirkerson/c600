/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: ehci_sunxi.c
*
* Author 		:
*
* Description 	: SoftWinner EHCI Driver
*
* Notes         :
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*    				    2013-3-13            1.0          create this file
*
*************************************************************************************
*/
#include <common.h>
#include <pci.h>
#include <usb.h>
#include <asm/io.h>
#include <usb/ehci-fsl.h>
#include <sys_config.h>

#include "ehci.h"
#include "ehci-core.h"
#include "ehci-sunxi.h"

static uint32_t usb_vbase = SUNXI_USB_EHCI0_BASE;
static uint32_t usb0_vbus_handle = 0;
static uint32_t usb1_vbus_handle = 0;
/*
*******************************************************************************
*                     pin_init
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static u32 alloc_pin(void)
{

	usb0_vbus_handle = gpio_request_ex("usbc0", "usb_drv_vbus_gpio");
	if(usb0_vbus_handle == 0){
		printf("ERR: gpio_request failed\n");
		return 0;
	}
	/* set config, ouput */
	gpio_set_one_pin_io_status(usb0_vbus_handle, 1, NULL);

	/* reserved is pull down */
	gpio_set_one_pin_pull(usb0_vbus_handle, 2, NULL);

	usb1_vbus_handle = gpio_request_ex("usbc1", "usb_drv_vbus_gpio");
	if(usb1_vbus_handle == 0){
                printf("ERR : gpio_request failed\n");
                return 0;
        }
	gpio_set_one_pin_io_status(usb1_vbus_handle,1,NULL);
        
        gpio_set_one_pin_pull(usb1_vbus_handle,2,NULL);
        return 1;
}

/*
*******************************************************************************
*                     pin_exit
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void free_pin(void)
{
	if(usb0_vbus_handle)
		gpio_release(usb0_vbus_handle, 2);

	if(usb1_vbus_handle)
		gpio_release(usb1_vbus_handle, 2);
	return;
}


/*
*******************************************************************************
*                     open_usb_clock
*
* Description:
*
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static u32 open_usb_clock(void)
{
	u32 reg_value = 0;
	u32 ccmu_base = SUNXI_VIR_CCM_BASE;

	//Gating AHB clock for USB_phy1
	reg_value = USBC_Readl(ccmu_base + 0x60);
	reg_value |= (1 << 25);
	reg_value |= (1 << 29);
	reg_value |= (1 << 24);
	reg_value |= (1 << 28);
	USBC_Writel(reg_value, (ccmu_base + 0x60));

	reg_value = USBC_Readl(ccmu_base + 0x2c0);
	reg_value |= (1 << 25);
	reg_value |= (1 << 29);
	reg_value |= (1 << 24);
	reg_value |= (1 << 28);
	USBC_Writel(reg_value, (ccmu_base + 0x2c0));

	//Enable module clock for USB phy1
	reg_value = USBC_Readl(ccmu_base + 0xcc);
	reg_value |= (1 << 9);
	reg_value |= (1 << 8);
	reg_value |= (1 << 1);
	reg_value |= (1 << 0);
	USBC_Writel(reg_value, (ccmu_base + 0xcc));

	return 0;
}

/*
*******************************************************************************
*                     close_usb_clock
*
* Description:
*
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static u32 close_usb_clock(void)
{
	u32 reg_value = 0;
	u32 ccmu_base = SUNXI_VIR_CCM_BASE;

	//Gating AHB clock for USB_phy0
	reg_value = USBC_Readl(ccmu_base + 0x2c0);
	reg_value &= ~(1 << 24);
	reg_value &= ~(1 << 28);
	reg_value &= ~(1 << 25);
	reg_value &= ~(1 << 29);
	USBC_Writel(reg_value, (ccmu_base + 0x2c0));

	reg_value = USBC_Readl(ccmu_base + 0x60);
	reg_value &= ~(1 << 24);
	reg_value &= ~(1 << 28);
	reg_value &= ~(1 << 25);
	reg_value &= ~(1 << 29);
	USBC_Writel(reg_value, (ccmu_base + 0x60));

	//Enable module clock for USB phy0
	reg_value = USBC_Readl(ccmu_base + 0xcc);
	reg_value &= ~(1 << 9);
	reg_value &= ~(1 << 1);
	reg_value &= ~(1 << 8);
	reg_value &= ~(1 << 0);
	USBC_Writel(reg_value, (ccmu_base + 0xcc));

	return 0;
}

/*
*******************************************************************************
*                     enable_usb_passby
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_passby(u32 ehci_vbase, u32 enable)
{
	unsigned long reg_value = 0;

	reg_value = USBC_Readl(SUNXI_USB_OTG_BASE + 0x420);
	reg_value &= ~(0x01);
	USBC_Writel(reg_value, (SUNXI_USB_OTG_BASE + 0x420));

	reg_value = USBC_Readl(ehci_vbase + 0x810);
	reg_value &= ~(0x01<<1);
	USBC_Writel(reg_value, (ehci_vbase + 0x810));

	reg_value = USBC_Readl(ehci_vbase + SUNXI_USB_PMU_IRQ_ENABLE);

	if(enable){
		reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
		reg_value |= (1 << 9);     	/* AHB Master interface burst type INCR4 enable */
		reg_value |= (1 << 8);     	/* AHB Master interface INCRX align enable */
		reg_value |= (1 << 0);     	/* ULPI bypass enable */
	}else if(!enable){
		reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
		reg_value &= ~(1 << 9);     /* AHB Master interface burst type INCR4 disable */
		reg_value &= ~(1 << 8);     /* AHB Master interface INCRX align disable */
		reg_value &= ~(1 << 0);     /* ULPI bypass disable */
	}
        USBC_Writel(reg_value, (ehci_vbase + SUNXI_USB_PMU_IRQ_ENABLE));

	return;
}

static void sunxi_set_vbus(int on_off)
{
	if(usb0_vbus_handle)
	        gpio_write_one_pin_value(usb0_vbus_handle, on_off, NULL);
        
        if(usb1_vbus_handle)
		gpio_write_one_pin_value(usb1_vbus_handle, on_off, NULL);
	return;
}

/*
*******************************************************************************
*                     sunxi_start_ehci
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void sunxi_start_ehci(u32 ehci_vbase)
{
	alloc_pin();
	open_usb_clock();
	usb_passby(ehci_vbase, 1);
	sunxi_set_vbus(1);
	__msdelay(800);

	return;
}

/*
*******************************************************************************
*                     sunxi_stop_ehci
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void sunxi_stop_ehci(u32 ehci_vbase)
{
	sunxi_set_vbus(0);
	usb_passby(ehci_vbase, 0);
	close_usb_clock();
	free_pin();
	return;
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 *
 * Excerpts from linux ehci sunxi driver.
 */
int ehci_hcd_init(void)
{
        sunxi_start_ehci(usb_vbase);
	hccr = (struct ehci_hccr *)usb_vbase;
	hcor = (struct ehci_hcor *)((uint32_t) hccr +
				HC_LENGTH(ehci_readl(&hccr->cr_capbase)));
	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	sunxi_stop_ehci(usb_vbase);
	return 0;
}
