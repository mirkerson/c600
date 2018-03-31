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

#include "common.h"
#include "mmc.h"
#include "spare_head.h"
#include "private_toc.h"

int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);
int sunxi_mmc_init(int sdc_no, unsigned bus_width, normal_gpio_cfg *gpio_info, int offset);
unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
int load_toc1_from_nand( void );

extern sbrom_toc0_config_t *toc0_config;

int load_toc1_from_sdmmc(int boot_type, int start_sector)
{
	u8  *tmp_buff = (u8 *)CONFIG_TOC1_STORE_IN_DRAM_BASE;
	uint head_size;
	sbrom_toc1_head_info_t	*toc1_head;
	int  sunxi_flash_mmc_card_no;
	int ret =0;

	if(boot_type == BOOT_FROM_SD0)
	{
		sunxi_flash_mmc_card_no = 0;
	}
	else
	{
		sunxi_flash_mmc_card_no = 2;
	}
	ret = sunxi_mmc_init(sunxi_flash_mmc_card_no, 4, toc0_config->storage_gpio + 24, 8);
	if(ret <= 0)
	{
		printf("sunxi_flash_init err: sunxi_mmc_init failed\n");
		goto __ERROR_EXIT;;
	}
	//一次读取64k数据
	ret = mmc_bread(sunxi_flash_mmc_card_no, start_sector, 64, tmp_buff);
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
		ret = mmc_bread(sunxi_flash_mmc_card_no, start_sector + 64, (head_size - 64*512 + 511)/512, tmp_buff);
		if(!ret)
		{
			printf("PANIC : sunxi_flash_init() error --3--\n");
			goto __ERROR_EXIT;
		}
	}
	sunxi_mmc_exit(sunxi_flash_mmc_card_no,toc0_config->storage_gpio + 24,8);
	return 0;
__ERROR_EXIT:
	sunxi_mmc_exit(sunxi_flash_mmc_card_no,toc0_config->storage_gpio + 24,8);
	return -1;

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
//mode : 0 -- normal 1:backup
int sunxi_flash_init(int boot_type, int start_sector)
{
	int ret;
	if((boot_type == BOOT_FROM_SD0) || (boot_type == BOOT_FROM_SD2))
	{
		ret = load_toc1_from_sdmmc(boot_type,start_sector);
		return ret;
	}
	else if(boot_type == BOOT_FROM_NFC)
	{
		if(load_toc1_from_nand())
		{
			printf("sunxi_flash_init err: nand init failed\n");

			return -1;
		}
		return 0;
	}
	else if(boot_type == BOOT_FROM_SPI)
	{
		printf("PANIC:NVM_init() : spi not support now\n");
		return -1;
	}
	else
	{
		printf("PANIC:NVM_init() : nvm_id = %d not support now\n",boot_type);

		return -1;
	}
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
int sunxi_flash_read(u32 start_sector, u32 blkcnt, void *buff)
{
	memcpy(buff, (void *)(CONFIG_TOC1_STORE_IN_DRAM_BASE + 512 * start_sector), 512 * blkcnt);

	return blkcnt;
}
