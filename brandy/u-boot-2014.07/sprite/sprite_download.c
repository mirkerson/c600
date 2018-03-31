/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <sunxi_mbr.h>
#include "sprite_verify.h"
#include "sprite_card.h"
#include <sunxi_nand.h>
#include <sunxi_flash.h>

DECLARE_GLOBAL_DATA_PTR;

extern int sunxi_set_secure_mode(void);
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
int sunxi_sprite_download_mbr(void *buffer, uint buffer_size)
{
	int ret;

	if(buffer_size != (SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM))
	{
		printf("the mbr size is bad\n");

		return -1;
	}
	if(sunxi_sprite_init(0))
	{
		printf("sunxi sprite init fail when downlaod mbr\n");

		return -1;
	}
	if(sunxi_sprite_write(0, buffer_size/512, buffer) == (buffer_size/512))
	{
		debug("mbr write ok\n");

		ret = 0;
	}
	else
	{
		debug("mbr write fail\n");

		ret = -1;
	}
	if(uboot_spare_head.boot_data.storage_type == 2)
	{
		printf("begin to write standard mbr\n");
		if(card_download_standard_mbr(buffer))
		{
			printf("write standard mbr err\n");

			return -1;
		}
		printf("successed to write standard mbr\n");
	}

	if(sunxi_sprite_exit(0))
	{
		printf("sunxi sprite exit fail when downlaod mbr\n");

		return -1;
	}

	return ret;
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
int sunxi_sprite_download_uboot(void *buffer, int production_media, int generate_checksum)
{
	int length = 0;
	if(gd->bootfile_mode  == SUNXI_BOOT_FILE_NORMAL)
	{
    		struct spare_boot_head_t    *uboot  = (struct spare_boot_head_t *)buffer;

		//
		printf("%s\n", uboot->boot_head.magic);
		if(strncmp((const char *)uboot->boot_head.magic, UBOOT_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: uboot magic is error\n");
			return -1;
		}
		length = uboot->boot_head.length;

	}
	else
	{
		sbrom_toc1_head_info_t *toc1 = (sbrom_toc1_head_info_t *)buffer;
		printf("toc magic 0x%x\n", toc1->magic);
		if(toc1->magic != TOC_MAIN_INFO_MAGIC)
		{
			printf("sunxi sprite: toc magic is error\n");
			return -1;
		}
		length = toc1->valid_len;
		if(generate_checksum)
		{
			toc1->add_sum = sunxi_sprite_generate_checksum(buffer,
		                toc1->valid_len,toc1->add_sum);
		}
	}

	printf("uboot size = 0x%x\n", length);
	printf("storage type = %d\n", production_media);
	if(!production_media)
	{
		debug("nand down uboot\n");
#ifdef  CONFIG_SUNXI_MODULE_NAND		
		if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
		{
			printf("work_mode_boot \n");
			return nand_force_download_uboot(length,buffer);
		}
		else
		{
			return nand_download_uboot(length, buffer);
		}
#else
		return -1;
#endif
	}
	else
	{
		printf("mmc down uboot\n");
#ifdef  CONFIG_SUNXI_MODULE_SDMMC		
		return card_download_uboot(length, buffer);
#else
		return -1;
#endif
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
int sunxi_sprite_download_boot0(void *buffer, int production_media)
{
	if(gd->bootfile_mode  == SUNXI_BOOT_FILE_NORMAL || gd->bootfile_mode  == SUNXI_BOOT_FILE_PKG)
	{
		boot0_file_head_t    *boot0  = (boot0_file_head_t *)buffer;

		debug("%s\n", boot0->boot_head.magic);
		if(strncmp((const char *)boot0->boot_head.magic, BOOT0_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: boot0 magic is error\n");

			return -1;
		}

		if(sunxi_sprite_verify_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}

		if(!production_media)
		{
#ifdef 	CONFIG_SUNXI_MODULE_NAND	
			nand_uboot_get_flash_info((void *)boot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE);
#endif
		}else{
#ifdef  CONFIG_SUNXI_MODULE_SDMMC		
			extern int mmc_write_info(int dev_num,void *buffer,u32 buffer_size);
			mmc_write_info(2,(void *)boot0->prvt_head.storage_data,STORAGE_BUFFER_SIZE);
#endif			
		}
		{
			int i;
			uint *addr = (uint *)DRAM_PARA_STORE_ADDR;

			for(i=0;i<32;i++)
			{
				printf("dram para[%d] = %x\n", i, addr[i]);
			}
		}
		memcpy((void *)&boot0->prvt_head.dram_para, (void *)DRAM_PARA_STORE_ADDR, 32 * 4);
		/* regenerate check sum */
		boot0->boot_head.check_sum = sunxi_sprite_generate_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum);
		if(sunxi_sprite_verify_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
#ifdef 	CONFIG_SUNXI_MODULE_NAND		
			return nand_download_boot0(boot0->boot_head.length, buffer);
#else	
			return -1;
#endif
		}
		else
		{
#ifdef 	CONFIG_SUNXI_MODULE_SDMMC			
			return card_download_boot0(boot0->boot_head.length, buffer,production_media);
#else
			return -1;
#endif
		}
	}
	else
	{
		toc0_private_head_t  *toc0   = (toc0_private_head_t *)buffer;
		sbrom_toc0_config_t  *toc0_config = (sbrom_toc0_config_t *)(buffer + 0x80);
		int ret;

		debug("%s\n", (char *)toc0->name);
		if(strncmp((const char *)toc0->name, TOC0_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: toc0 magic is error\n");

			return -1;
		}
		//
		if(sunxi_sprite_verify_checksum(buffer, toc0->length, toc0->check_sum))
		{
			printf("sunxi sprite: toc0 checksum is error\n");

			return -1;
		}
		//update flash param
		if(!production_media)
		{
#ifdef 	CONFIG_SUNXI_MODULE_NAND		
			nand_uboot_get_flash_info((void *)toc0_config->storage_data, STORAGE_BUFFER_SIZE);
#endif
		}else{
#ifdef  CONFIG_SUNXI_MODULE_SDMMC			
			extern int mmc_write_info(int dev_num,void *buffer,u32 buffer_size);
			//storage_data[384];  // 0-159:nand info  160-255:card info
			mmc_write_info(2,(void *)(toc0_config->storage_data+160),384-160);
#endif			
		}

		//update dram param
		if((uboot_spare_head.boot_data.work_mode == WORK_MODE_CARD_PRODUCT) || ((uboot_spare_head.boot_data.work_mode == WORK_MODE_SPRITE_RECOVERY)))
		{
			memcpy((void *)toc0_config->dram_para, (void *)(uboot_spare_head.boot_data.dram_para), 32 * 4);
			//toc0_config->dram_para[4] += toc0_config->secure_dram_mbytes;
		}
		else
		{
			memcpy((void *)toc0_config->dram_para, (void *)DRAM_PARA_STORE_ADDR, 32 * 4);
		}
		{
			int i;

			for(i=0;i<32;i++)
			{
				printf("dram para[%d] = 0x%x\n", i, toc0_config->dram_para[i]);
			}
		}

		/* regenerate check sum */
		toc0->check_sum = sunxi_sprite_generate_checksum(buffer, toc0->length, toc0->check_sum);
		if(sunxi_sprite_verify_checksum(buffer, toc0->length, toc0->check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
#ifdef  CONFIG_SUNXI_MODULE_NAND		
			ret = nand_download_boot0(toc0->length, buffer);
#else 	
			ret = -1;
#endif
		}
		else
		{
#ifdef 	 CONFIG_SUNXI_MODULE_SDMMC	
			ret = card_download_boot0(toc0->length, buffer,production_media);
#else
			ret = -1;
#endif
		}
		if(!ret)
		{
			sunxi_set_secure_mode();
		}

		return ret;
	}
}

