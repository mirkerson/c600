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
#include "common.h"
#include "spare_head.h"
#include "private_boot0.h"
#include "private_uboot.h"
#include <private_toc.h>
#include <asm/arch/mmc_boot0.h>

extern __s32 check_magic( __u32 *mem_base, const char *magic );
extern __s32 check_sum( __u32 *mem_base, __u32 size );

extern const boot0_file_head_t  BT0_head;


typedef struct _boot_sdcard_info_t
{
	__s32	card_ctrl_num;                //总共的卡的个数
	__s32	boot_offset;                  //指定卡启动之后，逻辑和物理分区的管理
	__s32	card_no[4];                   //当前启动的卡号, 16-31:GPIO编号，0-15:实际卡控制器编号
	__s32	speed_mode[4];                //卡的速度模式，0：低速，其它：高速
	__s32	line_sel[4];                  //卡的线制，0: 1线，其它，4线
	__s32	line_count[4];                //卡使用线的个数
}
boot_sdcard_info_t;

void update_flash_para(void)
{
	//struct sbrom_toc1_head_info  *toc1_head = NULL;
	//struct sbrom_toc1_item_info  *toc1_item = NULL;
	//toc1_head = (struct sbrom_toc1_head_info *)CONFIG_TOC1_STORE_IN_DRAM_BASE;
	//toc1_item = (struct sbrom_toc1_item_info *)(CONFIG_TOC1_STORE_IN_DRAM_BASE + sizeof(struct sbrom_toc1_head_info));
	int card_num;
	card_num = BT0_head.boot_head.platform[0] & 0xf;
	card_num = (card_num == 1)? 3: card_num;
	struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) CONFIG_SYS_TEXT_BASE;
	if(card_num == 0)
	{
		bfh->boot_data.storage_type = STORAGE_SD;
	}
	else if(card_num == 2)
	{
		bfh->boot_data.storage_type = STORAGE_EMMC;
		set_mmc_para(2,(void *)&BT0_head.prvt_head.storage_data);
	}
        else if(card_num == 3)
        {
                bfh->boot_data.storage_type = STORAGE_EMMC3;
                set_mmc_para(3,(void *)&BT0_head.prvt_head.storage_data);
        }
}

int load_toc1_from_sdmmc(char *buf)
{
	u8  *tmp_buff = (u8 *)CONFIG_BOOTPKG_STORE_IN_DRAM_BASE;
	uint head_size;
	sbrom_toc1_head_info_t	*toc1_head;
	int  card_no;
	int ret =0;
	int start_sector;
	boot_sdcard_info_t  *sdcard_info = (boot_sdcard_info_t *)buf;

	//card num: 0-sd 1-card3 2-emmc 
	card_no = BT0_head.boot_head.platform[0] & 0xf;
	card_no = (card_no == 1)? 3: card_no;
	start_sector = UBOOT_START_SECTOR_IN_SDMMC;

	printf("card no is %d\n", card_no);
	if(card_no < 0)
	{
		printf("bad card number %d in card boot\n", card_no);
		goto __ERROR_EXIT;
	}

	if(!sdcard_info->line_sel[card_no])
	{
		sdcard_info->line_sel[card_no] = 4;
	}
	printf("sdcard %d line count %d\n", card_no, sdcard_info->line_sel[card_no] );

	if( sunxi_mmc_init(card_no, sdcard_info->line_sel[card_no], BT0_head.prvt_head.storage_gpio, 16, (void *)(sdcard_info) ) == -1) 
	{
		printf("sunxi_flash_init err: sunxi_mmc_init failed\n");
		goto __ERROR_EXIT;;
	}
	//read 64 sectors 
	ret = mmc_bread(card_no, start_sector, 64, tmp_buff);
	if(!ret)
	{
		printf("PANIC : sunxi_flash_init() error --1--\n");
		goto __ERROR_EXIT;
	}
	toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
	if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
	{
		printf("PANIC : sunxi_flash_init() error --2--,toc1 magic error\n");
		goto __ERROR_EXIT;
	}
	head_size = toc1_head->valid_len;
	if(head_size > 64 * 512)
	{
		tmp_buff += 64*512;
		ret = mmc_bread(card_no, start_sector + 64, (head_size - 64*512 + 511)/512, tmp_buff);
		if(!ret)
		{
			printf("PANIC : sunxi_flash_init() error --3--\n");
			goto __ERROR_EXIT;
		}
	}
	printf("Succeed in loading uboot from sdmmc flash.\n");
	sunxi_mmc_exit( card_no, BT0_head.prvt_head.storage_gpio, 16 );
	return 0;

__ERROR_EXIT:
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16 );
	return -1;

}


int load_boot1(void)
{
	memcpy((void *)DRAM_PARA_STORE_ADDR, (void *)BT0_head.prvt_head.dram_para, 
		SUNXI_DRAM_PARA_MAX * 4);

	return load_toc1_from_sdmmc((char *)BT0_head.prvt_head.storage_data);
}
