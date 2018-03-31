/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: echi-sunxi.h
*
* Author 	: wangjx
*
* Description 	: Include file for A31 HCI Host Controller Driver
*
* Notes         :
*
* History 	:
*      <author>    		<time>       	<version >    		<desc>
*      wangjx      2013-3-13            1.0          create this file
*
*************************************************************************************
*/

#ifndef EHCI_SUNXI_H
#define EHCI_SUNXI_H

#define  DMSG_PRINT(stuff...)		printf(stuff)
#define  DMSG_ERR(...)        		(DMSG_PRINT("WRN:L%d(%s):", __LINE__, __FILE__), DMSG_PRINT(__VA_ARGS__))

#define  DMSG_DEBUG_PRINT(...)      (DMSG_PRINT("L%d(%s):", __LINE__, __FILE__), DMSG_PRINT(__VA_ARGS__))

#if 1
    #define DMSG_DEBUG         		DMSG_DEBUG_PRINT
#else
    #define DMSG_DEBUG(...)
#endif

#if 1
    #define DMSG_INFO         		DMSG_PRINT
#else
    #define DMSG_INFO(...)
#endif

#if	1
    #define DMSG_PANIC        		DMSG_ERR
#else
    #define DMSG_PANIC(...)
#endif



//---------------------------------------------------------------
//  �� ����
//---------------------------------------------------------------
#define  USBC_Readb(reg)	                    (*(volatile unsigned char *)(reg))
#define  USBC_Readw(reg)	                    (*(volatile unsigned short *)(reg))
#define  USBC_Readl(reg)	                    (*(volatile unsigned long *)(reg))

#define  USBC_Writeb(value, reg)                (*(volatile unsigned char *)(reg) = (value))
#define  USBC_Writew(value, reg)	            (*(volatile unsigned short *)(reg) = (value))
#define  USBC_Writel(value, reg)	            (*(volatile unsigned long *)(reg) = (value))

#define  USBC_REG_test_bit_b(bp, reg)         	(USBC_Readb(reg) & (1 << (bp)))
#define  USBC_REG_test_bit_w(bp, reg)   	    (USBC_Readw(reg) & (1 << (bp)))
#define  USBC_REG_test_bit_l(bp, reg)   	    (USBC_Readl(reg) & (1 << (bp)))

#define  USBC_REG_set_bit_b(bp, reg) 			(USBC_Writeb((USBC_Readb(reg) | (1 << (bp))) , (reg)))
#define  USBC_REG_set_bit_w(bp, reg) 	 		(USBC_Writew((USBC_Readw(reg) | (1 << (bp))) , (reg)))
#define  USBC_REG_set_bit_l(bp, reg) 	 		(USBC_Writel((USBC_Readl(reg) | (1 << (bp))) , (reg)))

#define  USBC_REG_clear_bit_b(bp, reg)	 	 	(USBC_Writeb((USBC_Readb(reg) & (~ (1 << (bp)))) , (reg)))
#define  USBC_REG_clear_bit_w(bp, reg)	 	 	(USBC_Writew((USBC_Readw(reg) & (~ (1 << (bp)))) , (reg)))
#define  USBC_REG_clear_bit_l(bp, reg)	 	 	(USBC_Writel((USBC_Readl(reg) & (~ (1 << (bp)))) , (reg)))

//-----------------------------------------------------------------------
//   USB register
//-----------------------------------------------------------------------
#define	SUNXI_USB_OTG_BASE   			0x1c19000
#define	SUNXI_USB_EHCI0_BASE			0x1c1a000
#define	SUNXI_USB_EHCI1_BASE			0x1c1b000
#define	SUNXI_USB_EHCI2_BASE			0x1c1c000
#define	SUNXI_USB_EHCI3_BASE			0x1c1d000


#define SUNXI_VIR_CCM_BASE			0x1c20000

#define SUNXI_USB_EHCI_BASE_OFFSET		0x00
#define SUNXI_USB_OHCI_BASE_OFFSET		0x400
#define SUNXI_USB_EHCI_LEN      		0x58
#define SUNXI_USB_OHCI_LEN      		0x58

#define SUNXI_USB_PMU_IRQ_ENABLE		0x800

struct sunxi_hci_hcd{
	void  *usb_vbase;			/* USB  base address 	*/
};

#endif   //__SUNXI_HCI_SUN6I_H__

