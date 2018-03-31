/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2009, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : load_boot1_from_sdmmc.c
*
* Author      : Gary.Wang
*
* Version     : 1.1.0
*
* Date        : 2009.12.08
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2009.12.08       1.1.0        build the file
*
************************************************************************************************************************
*/
#include "boot0_i.h"
#include "bsp_mmc_for_boot/bsp_sdc_for_boot.h"
#include "load_boot1_from_sdmmc.h"

extern const boot0_file_head_t  BT0_head;
/*******************************************************************************
*函数名称: load_boot1_from_sdmmc
*函数原型：int32 load_boot1_from_sdmmc( __u8 card_no )
*函数功能: 将一份好的Boot1从sdmmc flash中载入到SRAM中。
*入口参数: void
*返 回 值: OK                         载入并校验成功
*          ERROR                      载入并校验失败
*备    注:
*******************************************************************************/
__s32 load_boot1_from_sdmmc( char *buf)
{
    __u32  length;
    __s32  card_no, i;
	uboot_file_head  *bfh;
	boot_sdcard_info_t  *sdcard_info = (boot_sdcard_info_t *)buf;

	i = BT0_head.boot_head.platform[0];
	msg("card boot number = %d\n", i);

	//for(i=0;i<4;i++)
	{
		/* open sdmmc */
		card_no = i;
		msg("card no is %d\n", card_no);
		if(card_no < 0)
		{
			msg("bad card number %d in card boot\n", card_no);

			goto __card_op_fail__;
		}
		msg("sdcard %d line count %d\n", card_no, sdcard_info->line_count[i] );
		if(!sdcard_info->line_count[i])
		{
			sdcard_info->line_count[i] = 4;
		}
		if( SDMMC_PhyInit( card_no, sdcard_info->line_count[i] ) == -1)   //高速卡，4线配置
		{
			msg("Fail in Init sdmmc.\n");
			goto __card_op_fail__;
		}
		msg("sdcard %d init ok\n", card_no);
		/* load 1k uboot head */
	    if( SDMMC_PhyRead( BOOT1_START_SECTOR_IN_SDMMC, 1024/512, (void *)UBOOT_BASE, card_no ) != (1024/512))
		{
			msg("Fail in reading uboot head.\n");
			goto __card_op_fail__;
		}
		/* check head */
		if( check_magic( (__u32 *)UBOOT_BASE, UBOOT_MAGIC ) != CHECK_IS_CORRECT )
		{
			msg("ERROR! NOT find the head of uboot.\n");
			goto __card_op_fail__;
		}
		/* check length */
		bfh = (uboot_file_head *) UBOOT_BASE;
	    length =  bfh->boot_head.length;
		msg("The size of uboot is %x.\n", length );
	    if( ( length & ( SF_ALIGN_SIZE - 1 ) ) != 0 )
	    {
	    	msg("boot0 length is NOT align.\n");
	    	goto __card_op_fail__;
	    }
	    if( SDMMC_PhyRead( BOOT1_START_SECTOR_IN_SDMMC, length/512, (void *)UBOOT_BASE, card_no )!= (length/512))
		{
			msg("Fail in reading uboot head.\n");
			goto __card_op_fail__;
		}
		/* 检查校验和 */
	    if( check_sum( (__u32 *)UBOOT_BASE, length ) != CHECK_IS_CORRECT )
	    {
	        msg("Fail in checking uboot.\n");
	       	goto __card_op_fail__;
	    }
		if(i == 0)
		{
			bfh->boot_data.storage_type = 1;
		}
		else
		{
			bfh->boot_data.storage_type = 2;
			set_mmc_para(2,(void *)&BT0_head.prvt_head.storage_data);
		}
		msg("Succeed in loading uboot from sdmmc flash.\n");

		SDMMC_PhyExit( card_no );

		return OK;

__card_op_fail__:
		SDMMC_PhyExit(card_no );
	}

	return ERROR;
}

