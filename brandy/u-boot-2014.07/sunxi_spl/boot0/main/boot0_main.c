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
#include <private_boot0.h>
#include <private_uboot.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer.h>
#include <asm/arch/uart.h>
#include <asm/arch/dram.h>
#include <asm/arch/rtc_region.h>
#include <private_toc.h>

extern const boot0_file_head_t  BT0_head;
static void print_version(void);
static int boot0_clear_env(void);
static void update_uboot_info(__u32 dram_size);
static void enable_power_on(void);

extern __s32 boot_set_gpio(void  *user_gpio_list, __u32 group_count_max, __s32 set_gpio);
extern void mmu_setup(u32 dram_size);
extern void  mmu_turn_off( void );
extern int load_boot1(void);
extern void set_dram_para(void *dram_addr , __u32 dram_size, __u32 boot_cpu);
extern void boot0_jump(unsigned int addr);
extern void boot0_jmp_boot1(unsigned int addr);
extern void boot0_jmp_other(unsigned int addr);
extern void boot0_jmp_monitor(void);
extern void reset_pll( void );
extern int load_fip(int *use_monitor);
extern void set_debugmode_flag(void);
extern void set_pll( void );
extern void update_flash_para(void);


extern char boot0_hash_value[64];

int mmc_config_addr = (int)(BT0_head.prvt_head.storage_data);


/*******************************************************
we should implement below interfaces if platform support
handler standby flag in boot0
*******************************************************/
void __attribute__((weak)) handler_super_standby(void)
{
	return;
}

/*******add by suwenrong begin*******/

#include "boot0_key.h"
#include "reg_base.h"

int sunxi_key_init(void)
{
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;
	unsigned int reg_val = 0;

	reg_val = sunxi_key_base->ctrl;
	reg_val &= ~((7<<1) | (0xffU << 24));
	reg_val |=  LRADC_HOLD_EN;
	reg_val |=  LRADC_EN;
	sunxi_key_base->ctrl = reg_val;

	/* disable all key irq */
	sunxi_key_base->intc = 0;
	sunxi_key_base->ints = 0x1f1f;

	return 0;
}

int sunxi_key_exit(void)
{
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;

	sunxi_key_base->ctrl = 0;
	/* disable all key irq */
	sunxi_key_base->intc = 0;
	sunxi_key_base->ints = 0x1f1f;

	return 0;
}


int sunxi_key_read(void)
{
	unsigned int ints;
	int key = -1;

	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;

	ints = sunxi_key_base->ints;
	/* clear the pending data */
	sunxi_key_base->ints |= (ints & 0x1f);
	/* if there is already data pending,
	 read it */
	if( ints & ADC0_KEYDOWN_PENDING)
	{
		if(ints & ADC0_DATA_PENDING)
		{
			key = sunxi_key_base->data & 0x3f;
			if(!key)
			{
				key = -1;
			}
		}
	}
	else if(ints & ADC0_DATA_PENDING)
	{
		key = sunxi_key_base->data & 0x3f;
		if(!key)
		{
			key = -1;
		}
	}

	if(key > 0)
	{
		printf("key pressed value=0x%x\n", key);
	}


	return key;
}

int check_update_key(void)
{
	//struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;
	//int key_val;
	int cnt = 0;
	//puts(" press a key:\n");
	//sunxi_key_base->ints = 0x1f1f;
	
	printf("do_key_test\n");
	while(sunxi_key_read() == 0x1e)
	{
		//key_val = sunxi_key_read();
		//if(key_val == 0x1e)
		if(++cnt > 3)	
			boot0_jmp_other(FEL_BASE);
		
		__msdelay(100);
	}

	return 0;

}
/*******add by suwenrong end*******/

/*******************************************************************************
main:   body for c runtime 
*******************************************************************************/
void main( void )
{
	__u32 status;
	__s32 dram_size;
	__u32 fel_flag;
	__u32 boot_cpu=0;
	int use_monitor = 0;
        
    enable_power_on();
	set_pll();
	timer_init();
	sunxi_serial_init( BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6 );
	set_debugmode_flag();
	printf("HELLO! BOOT0 is starting!\n");
	printf("boot0 commit : %s \n",boot0_hash_value);
	print_version();
	
	//suwenrong add 
	sunxi_key_init();
	printf("run key detect boot0\n");
	//sunxi_key_read();
	//printf("bbbbbbbbbbbbbbb\n");
	__msdelay(10);
	//printf("aaaaaaaaaaaaaaa\n");
	//sunxi_key_read();
	check_update_key();
	sunxi_key_exit();
#if 0
{
	int i = 0;
	printf("---------------------\n");
	printf("--magic=%s\n", BT0_head.boot_head.magic);
	printf("--check_sum=%x\n", BT0_head.boot_head.check_sum);
	printf("--length=%x\n", BT0_head.boot_head.length);
	printf("--pub_head_size=%x\n", BT0_head.boot_head.pub_head_size);
	printf("--pub_head_vsn=%s\n", BT0_head.boot_head.pub_head_vsn);
	printf("--ret_addr=%x\n", BT0_head.boot_head.ret_addr);
	printf("--run_addr=%x\n", BT0_head.boot_head.run_addr);
	printf("--run_addr=%x\n", BT0_head.boot_head.run_addr);
	printf("---------------------\n");
	printf("--prvt_head_size=%x\n", BT0_head.prvt_head.prvt_head_size);
	printf("--debug_mode=%x\n", BT0_head.prvt_head.debug_mode);
	printf("--debug_mode=%x\n", BT0_head.prvt_head.debug_mode);
	for(i = 0; i<32; i++)
	{
		printf("--dram_para[%d]=%x\n",i,BT0_head.prvt_head.dram_para[i]);
	}
	printf("--dram_para[%d]=%d\n",i,BT0_head.prvt_head.uart_port);
	for(i = 0; i< 2; i++)
	{
		printf("uart_gpio[%d].port= %d\n",i,BT0_head.prvt_head.uart_ctrl[i].port);
		printf("uart_gpio[%d].port_num= %d\n",i,BT0_head.prvt_head.uart_ctrl[i].port_num);
		printf("uart_gpio[%d].mul_sel= %d\n",i,BT0_head.prvt_head.uart_ctrl[i].mul_sel);
		printf("uart_gpio[%d].pull= %d\n",i,BT0_head.prvt_head.uart_ctrl[i].pull);
		printf("uart_gpio[%d].drv_level= %d\n",i,BT0_head.prvt_head.uart_ctrl[i].drv_level);
		printf("uart_gpio[%d].data= %d\n",i,BT0_head.prvt_head.uart_ctrl[i].data);
	}
	printf("--enable_jtag=%d\n",BT0_head.prvt_head.enable_jtag);
	for(i = 0; i< 5; i++)
	{
		printf("jtag_gpio[%d].port= %d\n",i,BT0_head.prvt_head.jtag_gpio[i].port);
		printf("jtag_gpio[%d].port_num= %d\n",i,BT0_head.prvt_head.jtag_gpio[i].port_num);
		printf("jtag_gpio[%d].mul_sel= %d\n",i,BT0_head.prvt_head.jtag_gpio[i].mul_sel);
		printf("jtag_gpio[%d].pull= %d\n",i,BT0_head.prvt_head.jtag_gpio[i].pull);
		printf("jtag_gpio[%d].drv_level= %d\n",i,BT0_head.prvt_head.jtag_gpio[i].drv_level);
		printf("jtag_gpio[%d].data= %d\n",i,BT0_head.prvt_head.jtag_gpio[i].data);
	}

for(i = 0; i< 32; i++)
	{
		printf("storage_gpio[%d].port= %d\n",i,BT0_head.prvt_head.storage_gpio[i].port);
		printf("storage_gpio[%d].port_num= %d\n",i,BT0_head.prvt_head.storage_gpio[i].port_num);
		printf("storage_gpio[%d].mul_sel= %d\n",i,BT0_head.prvt_head.storage_gpio[i].mul_sel);
		printf("storage_gpio[%d].pull= %d\n",i,BT0_head.prvt_head.storage_gpio[i].pull);
		printf("storage_gpio[%d].drv_level= %d\n",i,BT0_head.prvt_head.storage_gpio[i].drv_level);
		printf("storage_gpio[%d].data= %d\n",i,BT0_head.prvt_head.storage_gpio[i].data);
	}

}
#endif
	if( BT0_head.prvt_head.enable_jtag )
	{
		boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 5, 1);
	}

	fel_flag = rtc_region_probe_fel_flag();
	if(fel_flag == SUNXI_RUN_EFEX_FLAG)
	{
		rtc_region_clear_fel_flag();
		printf("eraly jump fel\n");
		goto __boot0_entry_err0;
	}

#ifdef FPGA_PLATFORM
	dram_size = mctl_init((void *)BT0_head.prvt_head.dram_para);
#else
	dram_size = init_DRAM(0, (void *)BT0_head.prvt_head.dram_para);
#endif
	if(dram_size)
	{
		printf("dram size =%d\n", dram_size);
	}
	else
	{
		printf("initializing SDRAM Fail.\n");
		goto  __boot0_entry_err0;
	}
	//on some platform, boot0 should handler standby flag.
	handler_super_standby();

//	mmu_setup(dram_size);
	status = load_boot1();
	if(status == 0 )
	{
		use_monitor = 0;
		status = load_fip(&use_monitor);
	}

	printf("Ready to disable icache.\n");

    // disable instruction cache
//	mmu_turn_off( ); 

	if( status == 0 )
	{
		//update bootpackage size for uboot
		update_uboot_info(dram_size);
		//update flash para
		update_flash_para();
		//update dram para before jmp to boot1
		set_dram_para((void *)&BT0_head.prvt_head.dram_para, dram_size, boot_cpu);
		printf("Jump to secend Boot.\n");
        if(use_monitor)
		{
			boot0_jmp_monitor();
		}
		else
		{
			boot0_jmp_boot1(CONFIG_SYS_TEXT_BASE);
		}
	}

__boot0_entry_err0:
	boot0_clear_env();

	boot0_jmp_other(FEL_BASE);
}






#define PIO_BASE_ADDR          (0x01c20800)


static void enable_power_on(void)
{
    	volatile int val = 0;
    	volatile int tmp_val = 0;
	volatile int cfg_reg = 0;
	volatile int data_reg = 0;

	//set PD15 as output
	cfg_reg = PIO_BASE_ADDR + 0x70; //PD15 CFG register
	val = *((volatile int *)cfg_reg);                       //read register value	
	tmp_val = val & (~(0x07 << 28));                //clear bit28~bit30
        tmp_val = tmp_val | (0x01 << 28);               //set bit28~bit30 to 1
        writel(tmp_val, cfg_reg);
        //*((volatile int *)(cfg_reg)) = tmp_val;               //write back to register

        //set PD15 output hight level
        data_reg = PIO_BASE_ADDR + 0x7c;        //PD15 data register
        val = *((volatile int *)data_reg);                      //read register value
        tmp_val = val | (0x01 << 15);                   //set bit15 to 1
        writel(tmp_val, data_reg);
        //*((volatile int *)data_reg) = tmp_val;                //write back to register

        //printf("PD cfg reg=0x%x, val=0x%x\n", cfg_reg, *((volatile int *)cfg_reg));
        //printf("PD data reg=0x%x, val=0x%x\n", data_reg, *((volatile int *)data_reg));

       return;

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
static void print_version()
{
	//brom modify: nand-4bytes, sdmmc-2bytes
	printf("boot0 version : %s\n", BT0_head.boot_head.platform + 4);

	return;
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
static int boot0_clear_env(void)
{

	reset_pll();
	mmu_turn_off();

	__msdelay(10);
    
	return 0;
}

//
static void update_uboot_info(__u32 dram_size)
{
	struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) CONFIG_SYS_TEXT_BASE;
	struct sbrom_toc1_head_info *toc1_head = (struct sbrom_toc1_head_info *)CONFIG_BOOTPKG_STORE_IN_DRAM_BASE;
	bfh->boot_data.boot_package_size = toc1_head->valid_len;
	bfh->boot_data.dram_scan_size = dram_size;
	//printf("boot package size: 0x%x\n",bfh->boot_data.boot_package_size);
}

