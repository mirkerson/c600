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
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <fastboot.h>

#include <asm/arch/nand_bsp.h>
#include <mmc.h>
#include <android_misc.h>
#include <sunxi_mbr.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config_old.h>
#include <power/sunxi/axp.h>

DECLARE_GLOBAL_DATA_PTR;
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
/* add board specific code here */
int board_init(void)
{
	gd->bd->bi_arch_number = LINUX_MACHINE_ID;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100);
	debug("board_init storage_type = %d\n",uboot_spare_head.boot_data.storage_type);

	return 0;
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
void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
        gd->bd->bi_dram[0].size  = gd->ram_size;
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
int dram_init(void)
{
	uint dram_size = 0;
	dram_size = uboot_spare_head.boot_data.dram_scan_size;
	if(dram_size)
	{
		gd->ram_size = dram_size * 1024 * 1024;
	}
	else
	{
		gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	}
	
	print_size(gd->ram_size, "");
	putc('\n');

	return 0;
}

#ifdef CONFIG_GENERIC_MMC

extern int sunxi_mmc_init(int sdc_no);

int board_mmc_init(bd_t *bis)
{
	sunxi_mmc_init(bis->bi_card_num);

	return 0;
}

void board_mmc_pre_init(int card_num)
{
    bd_t *bd;

	bd = gd->bd;
	gd->bd->bi_card_num = card_num;
	mmc_initialize(bd);
    //gd->bd->bi_card_num = card_num;
}

int board_mmc_get_num(void)
{
    return gd->boot_card_num;
}


void board_mmc_set_num(int num)
{
    gd->boot_card_num = num;
}


//int mmc_get_env_addr(struct mmc *mmc, u32 *env_addr) {
//
//	*env_addr = sunxi_partition_get_offset_byname(CONFIG_SUNXI_ENV_PARTITION);
//	return 0;
//}
#endif


int cpu0_set_detected_paras(void)
{
	return 0;
}

/**
 * platform_axp_probe -detect the pmu on  board
 * @sunxi_axp_dev_pt: pointer to the axp array
 * @max_dev: offset of the property to retrieve
 * returns:
 *	the num of pmu
 */

int platform_axp_probe(sunxi_axp_dev_t  *sunxi_axp_dev_pt[], int max_dev)
{
#if 0
	if(axp19_probe())
	{
		printf("probe axp81X failed\n");
		sunxi_axp_dev_pt[0] = &sunxi_axp_null;
		return 0;
	}
	
	/* pmu type AXP19X */
	tick_printf("PMU: AXP19X found\n");

	sunxi_axp_dev_pt[0] = &sunxi_axp_19;
	sunxi_axp_dev_pt[PMU_TYPE_19X] = &sunxi_axp_19;
#endif
	//find one axp
	sunxi_axp_dev_pt[0] = &sunxi_axp_null;
	return 0;

}

char* board_hardware_info(void)
{
	static char * hardware_info  = "sun50iw1p1";
	return hardware_info;
}
