/*
 * Copyright 2008 Freescale Semiconductor, Inc.
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
#include <config.h>
#include <command.h>
#include <malloc.h>
#include <pmu.h>
#include <sys_config.h>
#include <asm/arch/intc.h>
#include <asm/arch/timer.h>
#include <smc.h>
#include <securestorage.h>
#ifdef CONFIG_AES_TEST
#include <asm/arch/ss.h>
#endif

struct timer_list timer0_t;
struct timer_list timer1_t;
static int timer_test_flag[2];
extern int sprite_led_init(void);
extern int sprite_led_exit(int status);

static  void  timer0_test_func(void *p)
{
	struct timer_list *timer_t;

	timer_t = (struct timer_list *)p;
	debug("timer number = %d\n", timer_t->timer_num);
	printf("this is timer test\n");

	del_timer(timer_t);
	timer_test_flag[0] = 0;

	return;
}

int do_timer_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int  base_count = 1000;

	if(timer_test_flag[0])
	{
		printf("can not test timer 0 now\n");

		return -1;
	}

	if(argc == 2)
	{
		base_count = simple_strtol(argv[1], NULL, 10);
	}
	timer0_t.data = (unsigned long)&timer0_t;
	timer0_t.expires = base_count;
	timer0_t.function = timer0_test_func;

	init_timer(&timer0_t);
	add_timer(&timer0_t);
	timer_test_flag[0] = 1;

	return 0;
}


U_BOOT_CMD(
	timer_test, 2, 0, do_timer_test,
	"do a timer and int test",
	"[delay time]"
);

static  void  timer1_test_func(void *p)
{
	struct timer_list *timer_t;

	timer_t = (struct timer_list *)p;
	debug("timer number = %d\n", timer_t->timer_num);
	printf("this is timer test\n");

	del_timer(timer_t);
	timer_test_flag[1] = 0;

	return;
}


int do_timer_test1(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int  base_count = 1000;

	if(timer_test_flag[1])
	{
		printf("can not test timer 1 now\n");

		return -1;
	}

	if(argc == 2)
	{
		base_count = simple_strtol(argv[1], NULL, 10);
	}
	timer1_t.data = (unsigned long)&timer1_t;
	timer1_t.expires = base_count;
	timer1_t.function = timer1_test_func;

	init_timer(&timer1_t);
	add_timer(&timer1_t);

	timer_test_flag[1] = 1;

	return 0;
}


U_BOOT_CMD(
	timer_test1, 2, 0, do_timer_test1,
	"do a timer and int test",
	"[delay time]"
);

#ifndef CONFIG_SUNXI_SPINOR_PLATFORM
int sunxi_usb_dev_register(uint dev_name);
void sunxi_usb_main_loop(int mode);
int sunxi_card_sprite_main(int workmode, char *name);


DECLARE_GLOBAL_DATA_PTR;


int do_sprite_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	printf("work mode=0x%x\n", uboot_spare_head.boot_data.work_mode);
	if(uboot_spare_head.boot_data.work_mode == WORK_MODE_USB_PRODUCT)
	{
		printf("run usb efex\n");
		if(sunxi_usb_dev_register(2))
		{
			printf("sunxi usb test: invalid usb device\n");
		}
		sunxi_usb_main_loop(2500);
	}
	else if(uboot_spare_head.boot_data.work_mode == WORK_MODE_CARD_PRODUCT)
	{
		printf("run card sprite\n");
		sprite_led_init();
		ret = sunxi_card_sprite_main(0, NULL);
		sprite_led_exit(ret);
		return ret;
	}
	else if(uboot_spare_head.boot_data.work_mode == WORK_MODE_USB_DEBUG)
	{
		unsigned int val;

		printf("run usb debug\n");
		if(sunxi_usb_dev_register(2))
		{
			printf("sunxi usb test: invalid usb device\n");
		}

		asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
		val &= ~(1<<2);
		asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (val) : "cc");

		sunxi_usb_main_loop(0);
	}
	else if(uboot_spare_head.boot_data.work_mode == WORK_MODE_SPRITE_RECOVERY)
	{
		printf("run sprite recovery\n");
		sprite_led_init();
		ret = sprite_form_sysrecovery();
		sprite_led_exit(ret);
		return ret;
	}
	else
	{
		printf("others\n");
	}

	return 0;
}

U_BOOT_CMD(
	sprite_test, 2, 0, do_sprite_test,
	"do a sprite test",
	"NULL"
);



int do_fastboot_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb fastboot\n");
	if(sunxi_usb_dev_register(3))
	{
		printf("sunxi usb test: invalid usb device\n");
	}
	sunxi_usb_main_loop(0);

	return 0;
}


U_BOOT_CMD(
	fastboot_test, 2, 0, do_fastboot_test,
	"do a sprite test",
	"NULL"
);

int do_mass_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb mass\n");
	if(sunxi_usb_dev_register(1))
	{
		printf("sunxi usb test: invalid usb device\n");
	}
	sunxi_usb_main_loop(0);

	return 0;
}


U_BOOT_CMD(
	mass_test, 2, 0, do_mass_test,
	"do a usb mass test",
	"NULL"
);

int do_efex_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb efex_test\n");
	if(sunxi_usb_dev_register(5))
	{
		printf("sunxi usb test: invalid usb device\n");
	}
	sunxi_usb_main_loop(2500);

	return 0;
}
U_BOOT_CMD(
	efex_test, 2, 0, do_efex_test,
	"do a usb efex test",
	"NULL"
);
#endif

int do_memcpy_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint  size = 64 * 1024 * 1024;

	tick_printf("memcpy test start\n");
	memcpy((void *)MEMCPY_TEST_DST, (void *)MEMCPY_TEST_SRC, size);
	tick_printf("memcpy test end\n");

	return 0;
}


U_BOOT_CMD(
	memcpy_test, 2, 0, do_memcpy_test,
	"do a memcpy test",
	"NULL"
);

int do_delay_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	tick_printf("delay 1000ms\n");
	__msdelay(1000);
	tick_printf("delay test end\n");

	tick_printf("delay 1000ms\n");
	__usdelay(1000 * 1000);
	tick_printf("delay test end\n");

	return 0;
}


U_BOOT_CMD(
	delay_test, 2, 0, do_delay_test,
	"do a delay test",
	"NULL"
);

int do_sysconfig(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *main_key, *sub_key;
	int  buffer[64];
	int  ret;

	memset(buffer, 0, 256);
	if(argc == 2)
	{
		char sub_name[32];
		uint main_hd;
		int  index;

		main_key = argv[1];

		main_hd = script_parser_fetch_subkey_start(main_key);
		if(!main_hd)
		{
			printf("the [%s] cant be found\n", main_key);

			return -1;
		}
		printf("[%s]:\n", main_key);
		index = 0;
		do
		{
			memset(sub_name, 0, 32);
			ret = script_parser_fetch_subkey_next(main_hd, sub_name, buffer, &index);
			if(ret < 0)
			{
				printf("find [%s] to end\n", main_key);

				break;
			}
			printf("--------- %s = 0x%x\n", sub_name, buffer[0]);
		}
		while(1);

		return 0;
	}
	else if(argc == 3)
	{
		main_key = argv[1];
		sub_key = argv[2];

		ret = script_parser_fetch(main_key, sub_key, buffer, 64);
		if(ret < 0)
		{
			printf("the [%s]:%s cant be found\n", main_key, sub_key);

			return -1;
		}
		printf("[%s]:%s = %x\n", main_key, sub_key, buffer[0]);

		return 0;
	}

	printf("the sys_config args is invalid\n");

	return -1;
}

U_BOOT_CMD(
	sys_config, 3, 0, do_sysconfig,
	"show the sys config value",
	"sys_config [main_key]\n"
	"sys_config [main_key] [sub_key]"
);


int do_power_probe(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *power_name, *pmu_type;
	int  power_value;

	if(argc == 3)
	{
		pmu_type   = argv[1];
		power_name = argv[2];

		power_value = axp_probe_supply_status_byname(pmu_type, power_name);
		if(power_value > 0)
		{
			printf("    %s %s output voltage = %d\n", pmu_type, power_name, power_value);
		}
		else
		{
			printf("    probe %s %s output voltage failed\n", pmu_type, power_name);
		}

		return 0;
	}

	cmd_usage(cmdtp);

	return 0;
}

U_BOOT_CMD(
	power_probe, 3, 0, do_power_probe,
	"probe the axp output",
	"usage: power_probe pmu_type axp_name"
);


#ifdef CONFIG_BOOT_A15

#include <asm/arch/cpu_switch.h>

extern void sunxi_set_rtc_flag(u8 flag);

int do_reboot_boot_cpu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if(argc == 1)
	{
		sunxi_board_restart(0);
	}
	else if(argc == 2)
	{
		char *boot_cpu = argv[1];

		if(!strcmp(boot_cpu, "a7"))
		{
			sunxi_set_rtc_flag(BOOT_A7_FLAG);
			sunxi_board_restart(0);
		}
		else if(!strcmp(boot_cpu, "a15"))
		{
			sunxi_set_rtc_flag(BOOT_A15_FLAG);
			sunxi_board_restart(0);
		}
	}

	cmd_usage(cmdtp);

	return -1;
}

U_BOOT_CMD(
	reboot, 3, 0, do_reboot_boot_cpu,
	"reboot by a7 or a15",
	"usage: reboot [a7] or [a15]"
);
#endif

#ifdef CONFIG_SUNXI_SECURE_SYSTEM
int do_efuse_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *efuse_name;
	char buffer[32];
	int  ret;

	if(argc != 2)
	{
		printf("the efuse item name is empty\n");

		return -1;
	}
	efuse_name = argv[1];
	printf("try to read %s\n", efuse_name);
	memset(buffer, 0, 32);
	ret = smc_efuse_readl(efuse_name, buffer);
	if(ret)
	{
		printf("read efuse key [%s] failed\n", efuse_name);
	}
	else
	{
		printf("read efuse key [%s] successed\n", efuse_name);
		sunxi_dump(buffer, 32);
	}

	return 0;
}

U_BOOT_CMD(
	efuse_read, 3, 0, do_efuse_read,
	"read efuse key",
	"usage: efuse_read efusename"
);

int do_huk_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char buffer[32];
	int  ret;

	printf("try to create huk\n");
	memset(buffer, 0, 32);
	ret = smc_create_huk(buffer, 32);
	if(ret < 0)
	{
		printf("create huk failed\n");
	}
	else if(ret == 1)
	{
		printf("create huk has been burned\n");

		sunxi_dump(buffer, 32);
	}
	else
	{
		printf("create huk successed\n");

		sunxi_dump(buffer, 32);
	}

	return 0;
}

U_BOOT_CMD(
	huk_test, 3, 0, do_huk_test,
	"create a huk as a test",
	"usage: huk_test"
);

int do_hdcp_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int  ret, data_len;
	char buffer[4096];

	printf("try to test hdcp\n");
	memset(buffer, 0, 4096);
	ret = sunxi_secure_storage_init();
	if(ret)
	{
		printf("sunxi init secure storage failed\n");
	}
	else
	{
		ret = sunxi_secure_storage_read("hdcpkey", buffer, 4096, &data_len);
		if(ret)
		{
			printf("probe hdcp key failed\n");
		}
		else
		{
			printf("source data in secure storage\n");
			sunxi_dump(buffer, (data_len+3)&(~0x3));
			ret = smc_aes_bssk_decrypt_to_keysram(buffer, data_len);
			if(ret)
			{
				printf("push hdcp key failed\n");
			}
			else
			{
				printf("push hdcp key ok\n");
			}
		}
	}

	return 0;
}

U_BOOT_CMD(
	hdcp_test, 3, 0, do_hdcp_test,
	"test the hdcp key",
	"usage: hdcp_test"
);

#endif


#ifdef CONFIG_AES_TEST

#define AES_RSSK_TEST_SOURCE_DATA_LEN  (512)
int do_hdcp_rssk_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8  source[AES_RSSK_TEST_SOURCE_DATA_LEN];
	u8  dest[AES_RSSK_TEST_SOURCE_DATA_LEN];
	u8  cmp[AES_RSSK_TEST_SOURCE_DATA_LEN];
	u8  md5_buf_keysram[16], md5_buf_dram[16];
	u32 data_len = 288, i;
	int ret;

	for(i=0;i<AES_RSSK_TEST_SOURCE_DATA_LEN;i++)
	{
		source[i] = (i&0xff) + 0x80;
	}
	memset(dest, 0, AES_RSSK_TEST_SOURCE_DATA_LEN);
	memset(cmp, 0, AES_RSSK_TEST_SOURCE_DATA_LEN);

	printf("aes rssk test: source data:\n");
	sunxi_dump(source, data_len);
	ret = sunxi_aes_decrypt_rssk_hdcp_to_keysram(source, data_len);
	if(ret)
	{
		printf("%s:fail to calculate hdcp key to keysram\n",__func__);

		return -1;
	}
	ret = sunxi_md5_keysram_calcute((void *)md5_buf_keysram, 16);
	if(ret)
	{
		printf("%s: fail to calculate md5 in keysram\n",__func__);

		return -1;
	}
	printf("aes rssk test: keysram buf md5:\n");
	sunxi_dump(md5_buf_keysram, 16);
	ret = sunxi_aes_decrypt_rssk_hdcp_to_dram(source, data_len, dest);
	if(ret)
	{
		printf("%s: fail to calculate hdcp key for source data\n",__func__ );

		return -1;
	}
	printf("aes rssk test: decrypt data:\n");
	sunxi_dump(dest, data_len);

	ret = sunxi_md5_dram_calcute((void *)dest, data_len, (void *)md5_buf_dram, 16);
	if(ret)
	{
		printf("%s: fail to calculate md5 for source data\n",__func__ );

		return -1;
	}
	printf("aes rssk test: dram buf md5:\n");
	sunxi_dump(md5_buf_dram, 16);

	ret = sunxi_aes_encrypt_rssk_hdcp_to_dram(cmp, dest, data_len);
	if(ret)
	{
		printf("%s: fail to encrypt hdcp rssk for source data\n",__func__ );

		return -1;
	}
	printf("aes rssk test: encrypt data:\n");
	sunxi_dump(cmp, data_len);

	for(i=0;i<16;i++)
	{
		if(md5_buf_keysram[i] != md5_buf_dram[i])
		{
			printf("%s: compare the md5 between keysram and source data failed\n", __func__);

			printf("md5_buf_keysram:\n");
			sunxi_dump(md5_buf_keysram, 16);

			printf("md5_buf_source:\n");
			sunxi_dump(md5_buf_dram, 16);

			return -1;
		}
		else
		{
			printf("%s: compare the md5 between keysram and source data successed\n", __func__);
		}
	}

	return 0;
}
U_BOOT_CMD(
	hdcp_rssk_test, CONFIG_SYS_MAXARGS,1, do_hdcp_rssk_test,
	"test the aes rssk to keysram method",
	"usage: hdcp_rssk_test\n"
	"now there is no any parameters\n"
);


#define AES_TEST_SOURCE_DATA_LEN   (512)
int do_aes_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8  source[AES_TEST_SOURCE_DATA_LEN];
	u8  dest[AES_TEST_SOURCE_DATA_LEN];
	u8  cmp[AES_TEST_SOURCE_DATA_LEN];
	u8  key_buf[256];
	u32 key_type=2, aes_mode=0;
	int ret, i;

	if(argc == 1)
	{
		;
	}
	else if(argc == 2)
	{
		key_type = simple_strtol(argv[1], NULL, 10);
	}
	else if(argc == 3)
	{
		key_type = simple_strtol(argv[1], NULL, 10);
		aes_mode = simple_strtol(argv[2], NULL, 10);
	}
	else
	{
		printf("Too much parameters\n");

		goto return_err;
	}

	if(key_type == 0)
	{
		printf("AES KEY: 128bit\n");
	}
	else if(key_type == 1)
	{
		printf("AES KEY: 192bit\n");
	}
	else if(key_type == 2)
	{
		printf("AES KEY: 256bit\n");
	}
	else
	{
		printf("the AES KEY TYPE is invalid\n");

		goto return_err;
	}

	if(aes_mode == 0)
	{
		printf("AES MODE: ECB\n");
	}
	else if(aes_mode == 1)
	{
		printf("AES MODE: CBC\n");
	}
	else
	{
		printf("the AES MODE is invalid\n");

		goto return_err;
	}

	for(i=0;i<AES_TEST_SOURCE_DATA_LEN;i++)
	{
		source[i] = (i&0xff) + 0x80;
	}
	for(i=0;i<256;i++)
	{
		key_buf[i] = (i&0xff) + 0x5A;
	}

	printf("aes test: source data:\n");
	sunxi_dump(source, AES_TEST_SOURCE_DATA_LEN);

	printf("try to encrypt\n");
	memset(dest , 0, AES_TEST_SOURCE_DATA_LEN);
	ret = sunxi_aes_encrypt(source, dest, AES_TEST_SOURCE_DATA_LEN, key_buf, key_type, aes_mode);
	if(ret)
	{
		printf("%s:fail to encrypt\n",__func__);

		goto return_err;
	}
	printf("the encrypt value:\n");
	sunxi_dump(dest, AES_TEST_SOURCE_DATA_LEN);

	printf("try to decrypt\n");
	memset(cmp , 0, AES_TEST_SOURCE_DATA_LEN);
	ret = sunxi_aes_decrypt(dest, cmp, AES_TEST_SOURCE_DATA_LEN, key_buf, key_type, aes_mode);
	if(ret)
	{
		printf("%s:fail to decrypt\n",__func__);

		goto return_err;
	}
	printf("the decrypt value:\n");
	sunxi_dump(cmp, AES_TEST_SOURCE_DATA_LEN);

	for(i=0;i<AES_TEST_SOURCE_DATA_LEN;i++)
	{
		if(source[i] != cmp[i])
		{
			printf("the aes fail: source[%d]:%02x is not equal to cmp[%d]:%02x\n", i, source[i], i, cmp[i]);

			goto return_err;
		}
	}

	printf("aes successed\n");

	return 0;

return_err:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	aes_test, CONFIG_SYS_MAXARGS,1, do_aes_test,
	"test the aes encrypt and decrypt",
	"usage: aes_test [key_type] [aes_mode]\n"
	"key type: 0        , 128BIT\n"
	"          1        , 192BIT\n"
	"          2 default, 256BIT\n"
	"aes mode: 0 default, ECB\n"
	"        : 1        , CBC\n"
);

int do_md5_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8  source[AES_TEST_SOURCE_DATA_LEN];
	u8  md5_buf[16];
	u32 data_len;
	int ret, i;

	memset(source, 0, AES_TEST_SOURCE_DATA_LEN);
	for(i=0;i<26;i++)
	{
		source[i] = 'a' + i;
	}
	for(i=0;i<26;i++)
	{
		source[i+26] = 'A' + i;
	}
	for(i=0;i<10;i++)
	{
		source[i+52] = '0' + i;
	}

	printf("the data to be calculate\n");
	if(argc == 1)
	{
		data_len = 32;
	}
	else if(argc == 2)
	{
		data_len = simple_strtol(argv[1], NULL, 10);
		if(data_len > 62)
		{
			printf("the data len is too long(%d), which requires smaller than 62\n", data_len);

			return cmd_usage(cmdtp);
		}
		else if(data_len == 0)
		{
			data_len = 32;
		}
	}
	else
	{
		printf("Too much parameters\n");

		return cmd_usage(cmdtp);
	}

	printf("the data bytes: %d\n", data_len);
	source[data_len] = '\0';
	printf("the source data: %s\n", source);

	ret = sunxi_md5_dram_calcute((void *)source, data_len, (void *)md5_buf, 16);
	if(ret)
	{
		printf("%s: fail to calculate md5 for source data\n",__func__ );

		return -1;
	}
	printf("md5 test: dram buf md5:\n");
	sunxi_dump(md5_buf, 16);

	return 0;
}

U_BOOT_CMD(
	md5_test, CONFIG_SYS_MAXARGS,1, do_md5_test,
	"test the md5",
	"usage: md5_test [data_len]\n"
	"data len: 0 < data_len < 62\n"
	"if null, consider as 32 "
);

#endif

