/*
**********************************************************************************************************************
*                                                    ePDK
*                                    the Easy Portable/Player Develop Kits
*                                              eBIOS Sub-System
*
*                                   (c) Copyright 2007-2009, Steven.ZGJ.China
*                                             All Rights Reserved
*
* Moudle  : ebios
* File    : timer.h
*
* By      : Steven
* Version : v1.0
* Date    : 2008-9-5 14:31:03
**********************************************************************************************************************
*/
#ifndef _TIMER_H_
#define _TIMER_H_


#include "hd_scrpt.h"


#define TMRC_AVS_CTRL			readl(TMRC_REGS_BASE + 0x80)
#define TMRC_AVS_COUNT0			readl(TMRC_REGS_BASE + 0x84)
#define TMRC_AVS_COUNT1			readl(TMRC_REGS_BASE + 0x88)
#define TMRC_AVS_DIVISOR		readl(TMRC_REGS_BASE + 0x8C)


#define WATCHDOG1_CTRL       	readl(TMRC_REGS_BASE + 0xB0)
#define WATCHDOG1_CFG       	readl(TMRC_REGS_BASE + 0xB4)
#define WATCHDOG1_MODE       	readl(TMRC_REGS_BASE + 0xB8)



#endif  /* _TMRC_H_ */

