/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

extern void boot0_jmp_other(unsigned int addr);
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
uint rtc_region_probe_fel_flag(void)
{
	uint fel_flag, reg_value;
	int  i;

	fel_flag = readl(RTC_GENERAL_PURPOSE_REG(2));
	printf("fel flag  = 0x%x\n", fel_flag);
	for(i=0;i<8;i++)
	{
		reg_value = readl(RTC_GENERAL_PURPOSE_REG(i));
		printf("rtc[%d] value = 0x%x\n", i, reg_value);
	}

	return fel_flag;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void rtc_region_clear_fel_flag(void)
{
    writel(0, RTC_GENERAL_PURPOSE_REG(2));
}

//probe standby flag
//the flag is seted by kernel code when enter standby
static int probe_super_standby_flag(void)
{ 
        uint reg_value = 0;
	int standby_flag = 0;
        reg_value = readl(RTC_STANDBY_FLAG_REG);
        standby_flag = (reg_value & ~(0xfffe0000))>> 16; 
        //printf("RTC_Standby flag is 0x%x ,super_standby flag  = 0x%x\n",reg_value,standby_flag);
	return standby_flag;
}

void handler_super_standby(void)
{
	if(probe_super_standby_flag())
	{
		printf("find standby flag,jump to addr 0x%x \n",readl(RTC_STANDBY_SOFT_ENTRY_REG));
		boot0_jmp_other(readl(RTC_STANDBY_SOFT_ENTRY_REG));
	}
}
