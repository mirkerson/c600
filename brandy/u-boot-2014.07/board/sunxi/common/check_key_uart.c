#include <common.h>
#include <spare_head.h>
#include <private_uboot.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <asm/arch/key.h>
#include <power/sunxi/pmu.h>
#include "debug_mode.h"
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

enum _USER_MODE
{
	NORMAL_MODE = 0,
	FEL_MODE ,
	DEBUG_MODE ,
	RECOVERY_MODE
};


#ifdef CONFIG_SUNXI_KEY_SUPPORT
#define   KEY_DELAY_MAX          (8)
#define   KEY_DELAY_EACH_TIME    (40)
#define   KEY_MAX_COUNT_GO_ON    ((KEY_DELAY_MAX * 1000)/(KEY_DELAY_EACH_TIME))


/*
* fun      :       check_fel_key
* return  :       0: normal  1:enter fel
*/
static int check_config_fel_key(uint32_t key_value)
{
	uint32_t fel_key_max=0, fel_key_min=0;
	int ret = 0;
	int nodeoffset;
	nodeoffset = fdt_path_offset(working_fdt,FDT_PATH_FEL_KEY);
	if(nodeoffset < 0)
	{
		printf("fel key not configed\n");
		return NORMAL_MODE;
	}
	printf("fel key configed\n");
	ret = fdt_getprop_u32(working_fdt, nodeoffset,"fel_key_max",&fel_key_max);
	if(ret < 0)
	{
		printf ("%s:get fel_key_max error: %s\n",__func__,fdt_strerror(ret));
	    	return NORMAL_MODE;
	}
	ret = fdt_getprop_u32(working_fdt, nodeoffset,"fel_key_min",&fel_key_min);
	if(ret < 0)
	{
		printf ("%s:get fel_key_min error: %s\n",__func__,fdt_strerror(ret));
	    	return NORMAL_MODE;
	}

	if((key_value <= fel_key_max) && (key_value >= fel_key_min))
	{
		printf("fel key detected\n");
		//jum to fel
		return FEL_MODE;
	}
	printf("fel key value %d is not in the range from %d to %d\n", key_value, fel_key_min, fel_key_max);
	return NORMAL_MODE;

}


/*
* fun      :       check_user_mode    
* return  :       0: normal  1:enter fel
*note     :      press an  key and not loosen:
                     1) enter debug when  usb plug in  up to 3 times
*                    2) enter fel,if press power key  up to 3 times
*/
static int  check_user_mode(void)
{
	int time_tick = 0;
	int power_plug_count = 0;
	int new_power_status = 0;
	int old_power_status = 0;

	int count = 0;
	int power_key = 0;
    int key_value = 0;
    int rpt = 0, rpt_r = 0;
    
#ifdef 	CONFIG_SUNXI_MODULE_AXP
	old_power_status = axp_probe_power_source();
#else
	old_power_status = 0;
#endif
	//add by guoyingyang
	while( (key_value = sunxi_key_read()) > 0 ) //press key and not loosen
	{
	printf("rpt = %x, rpt_r = %x\n", rpt, rpt_r);
    
        //if(0x1e == key_value) //add by suwenrong
        //    rpt++;
        //else 
            if(0x12 == key_value)
            rpt_r++;
        else
        {
            rpt = 0;
            rpt_r = 0;
        }
            
        
        if(rpt > 10)
            return FEL_MODE;

        if(rpt_r > 10)
            return RECOVERY_MODE;

		time_tick++;
        
#ifdef  CONFIG_SUNXI_MODULE_AXP	
		//detect usb plug in&out  for debug mode	
		new_power_status = axp_probe_power_source();
#else
		new_power_status = 0;
#endif
		if(new_power_status != old_power_status)
		{
			power_plug_count++;
			old_power_status = new_power_status;
		}
		if(power_plug_count == 3)
		{
			debug_mode_set();
			return DEBUG_MODE;
		}

#ifdef CONFIG_SUNXI_MODULE_AXP
		//detect power key status for fel mode		
		power_key = axp_probe_key();
#else
		power_key = 0;
#endif
		if(power_key > 0) 
		{
			count ++;
		}
		if(count == 3)
		{
			printf("you can loosen the key to update now\n");
			//jump to fel
			return FEL_MODE;
		}

		
		__msdelay(KEY_DELAY_EACH_TIME);
		if(time_tick > KEY_MAX_COUNT_GO_ON)
		{
			printf("time out\n");
			break;
		}
	}
	printf("key not pressed anymore\n");
	return NORMAL_MODE;
}


/*
 * Read a key  when user pressed a  key 
 * return :     0:normal  -1: jump to fel
 */
int check_update_key(void)
{
	int key_value = 0;
    int rets;

    
	gd->key_pressd_value = 0;
	
	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
	    return 0;
	}
	
	sunxi_key_init();
	printf("run key detect\n");
	sunxi_key_read();
	__msdelay(10);

	key_value = sunxi_key_read();  	
	if( (key_value != 0x12) && (key_value != 0x24) )   //(key_value != 0x1e) &&           				
	{
		//printf("no key found\n");
		return 0;
	}


	//save key value for  test other boot mode, ex recovery or fastboot.
	gd->key_pressd_value = key_value;

	if(check_config_fel_key(key_value) == FEL_MODE)
	{
		return -1;
	}

    rets = check_user_mode();
	if(rets == FEL_MODE)
	{
        //spinor_erase_all_blocks(1);
        if( 0 == spinor_erase__block0() ) //add by suwenrong
		    return -1;
	}
    else if(rets == RECOVERY_MODE)
    {
        
        printf("--------RECOVERY_MODE\n");

        *(uint32_t *)(0x81ff0000) = 11250816;
    }
    
	return 0;
}
#else
int check_update_key(void)
{
	return 0;
}

#endif

/*
 * Read a char from serial when call board_init_f 
 * 
 */
int check_uart_input(void)
{
	extern int do_key_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
	int c = 0;
	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
	    return 0;
	}
	if(tstc())
	{
		c = getc();
		printf("0x%x\n", c);
	}
	else
	{
		puts("no uart input\n");
	}

	if(c == '2')
	{
		return -1;
	}
	else if(c == '3')
	{
#ifdef CONFIG_SUNXI_KEY_SUPPORT
		sunxi_key_init();
		do_key_test(NULL, 0, 1, NULL);
#endif
	}
	else if(c == 's')		//shell mode
	{
		gd->force_shell = 1;
                gd->debug_mode = 1;
	}
	return 0;
}


