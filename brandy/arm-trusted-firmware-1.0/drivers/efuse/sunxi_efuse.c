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
#include <debug.h>
#include <efuse.h>
#include <efuse_map.h>
#include <string.h>

extern void flush_dcache_range(unsigned long addr, unsigned long len);

static efuse_key_map_t key_imformatiom[] =
{
	{"rotpk", EFUSE_ROTPK, SID_ROTPK_SIZE, SCC_ROTPK_DONTSHOW_FLAG, SCC_ROTPK_BURNED_FLAG, {0}},
	{"rssk", EFUSE_RSSK, SID_RSSK_SIZE, SCC_RSSK_DONTSHOW_FLAG, SCC_RSSK_BURNED_FLAG, {0}},
	{"ssk", EFUSE_SSK, SID_SSK_SIZE, SCC_SSK_DONTSHOW_FLAG, SCC_SSK_BURNED_FLAG, {0}},
	{"rotpk", EFUSE_ROTPK, SID_ROTPK_SIZE, SCC_ROTPK_DONTSHOW_FLAG, SCC_ROTPK_BURNED_FLAG, {0}},
	{"hdcphash", EFUSE_HDCP_HASH, SID_HDCP_HASH_SIZE, -1, SCC_HDCP_HASH_BURNED_FLAG, {0}},
	{{0} , 0, 0, 0, 0,{0}}
};

static void sunxi_dump(void *addr, unsigned int size)
{
	int i,j;
	char *buf = (char *)addr;

	for(j=0;j<size;j+=16)
	{
		for(i=0;i<16;i++)
		{
			tf_printf("%x ", buf[j+i] & 0xff);
		}
		tf_printf("\n");
	}
	tf_printf("\n");

	return ;
}
//*****************************************************************************
//	uint32_t sid_read_key(uint32_t key_index)
//  Description:
//				Read key from Efuse by software
//	Arguments:	None
//
//
//	Return Value:	Key value
//*****************************************************************************
unsigned int sid_set_burned_flag(int bit_offset)
{
    unsigned int reg_val;

    reg_val  = sid_read_key(EFUSE_CHIP_CONFIG);
    reg_val |= (0x1<<bit_offset);		//使能securebit
    sid_program_key(EFUSE_CHIP_CONFIG, reg_val);
    reg_val = (sid_read_key(EFUSE_CHIP_CONFIG) >> bit_offset) & 1;

    return reg_val;
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
int sunxi_efuse_write(void *key_buf)
{
	sunxi_efuse_key_info_t  *key_list = NULL;
	unsigned long key_data_addr;
	int i;
	unsigned int key_start_addr;			// 每一笔数据的开始地址
	unsigned int *key_once_data = 0;
	unsigned int key_data_remain_size; 				//剩下字节数

	unsigned int verify_buf[128];

	int burned_status;

	efuse_key_map_t *key_map = key_imformatiom;

	if (key_buf == NULL)
	{
		tf_printf("[efuse] error: buf is null\n");
		return -1;
	}
	key_list = (sunxi_efuse_key_info_t  *)key_buf;
	key_data_addr = ((unsigned long)key_list->key_data) & 0xffffffff ;
#ifdef EFUSE_DEBUG
		tf_printf("^^^^^^^tf_printf key_buf^^^^^^^^^^^^\n");
		tf_printf("key name=%s\n", key_list->name);
		tf_printf("key len=%d\n", key_list->len);
		tf_printf("key data:\n");
		sunxi_dump((void *)key_data_addr, key_list->len);
		tf_printf("###################\n");
#endif
	// 查字典，被允许的key才能烧写
	for (; key_map != NULL; key_map++)
	{
		if (!memcmp(key_list->name, key_map->name, strlen(key_map->name)))
		{
			tf_printf(" burn key start\n");
			tf_printf("burn key start\n");
			tf_printf("key name = %s\n", key_map->name);
			tf_printf("key index = 0x%x\n", key_map->key_index);

			//	判断是否足够空间来存放key
			if ((key_map->store_max_bit / 8) < key_list->len)
			{
				tf_printf("[efuse] error: not enough space to store the key, efuse size(%d), data size(%d)\n", key_map->store_max_bit/8, key_list->len);

				return -1;
			}
			// 判断存放key的区域是否已经烧录
			tf_printf("===== key_map->burned_bit_offset ====%d \n",key_map->burned_bit_offset);
			burned_status = (sid_read_key(EFUSE_CHIP_CONFIG) >> key_map->burned_bit_offset) & 1;
			if(burned_status)
			{
				tf_printf("key %s has been burned already\n", key_map->name);

				return -1;
			}

			break;
		}
	}

	if (key_map == NULL)
	{
		tf_printf("[efuse] error: can't burn the key (unknow)\n");

		return -1;
	}
	//烧写key
	key_start_addr = key_map->key_index;
	key_data_remain_size = key_list->len;
	key_once_data = (unsigned int *)key_data_addr;
	tf_printf("key_data_remain_size=%d\n", key_data_remain_size);
	//flush_cache((uint)pbuf, byte_cnt);
	for(;key_data_remain_size >= 4; key_data_remain_size-=4, key_start_addr += 4, key_once_data ++)
	{
		tf_printf("key_data_remain_size=%d\n", key_data_remain_size);
		tf_printf("key data=0x%x, addr=0x%p\n", *key_once_data, key_once_data);
		sid_program_key(key_start_addr, *key_once_data);

		tf_printf("[efuse] addr = 0x%x, data = 0x%x\n", key_start_addr, *key_once_data);
	}

	if(key_data_remain_size)
	{
		if(key_data_remain_size == 1)
		{
			*key_once_data &= 0x000000ff;
		}
		else if(key_data_remain_size == 2)
		{
			*key_once_data &= 0x0000ffff;
		}
		else if(key_data_remain_size == 3)
		{
			*key_once_data &= 0x00ffffff;
		}
		sid_program_key(key_start_addr, *key_once_data);

		tf_printf("[efuse] addr = 0x%x, data = 0x%x\n", key_start_addr, *key_once_data);
	}
	//读出烧录的key信息
	key_start_addr = key_map->key_index;
	key_data_remain_size = key_list->len;

	memset(verify_buf, 0, 512);
	if(key_data_remain_size & 3)
		key_data_remain_size = (key_data_remain_size + 3) & (~3);
	for(i=0;i<key_data_remain_size/4; i++)
	{
		verify_buf[i] = sid_read_key(key_start_addr);
		key_start_addr += 4;
	}
	//比较
	if(memcmp(verify_buf, (const void *)key_data_addr, key_list->len))
	{
		tf_printf("compare burned key with memory data failed\n");
		tf_printf("memory data:\n");
		sunxi_dump((void *)key_data_addr, key_list->len);
		tf_printf("burned key:\n");
		sunxi_dump(verify_buf, key_list->len);

		return -1;
	}
	//锁定
	sid_set_burned_flag(key_map->burned_bit_offset);

	tf_printf(" burn key end\n");

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
int sunxi_efuse_read(void *key_name, void *read_buf)
{
	efuse_key_map_t *key_map = key_imformatiom;
	unsigned int key_start_addr;								// 每一笔数据的开始地址
	int show_status;
	int check_buf[128];
	unsigned int key_data_remain_size; 				//剩下字节数
	int i;

	// 查字典，被允许的key才能被查看
	for (; key_map != NULL; key_map++)
	{
		if (!memcmp(key_name, key_map->name, strlen(key_map->name)))
		{
			tf_printf("read key start\n");
			tf_printf("key name = %s\n", key_map->name);
			tf_printf("key index = 0x%x\n", key_map->key_index);

			//判断key有没有烧录过
			show_status = (sid_read_key(EFUSE_CHIP_CONFIG) >> key_map->burned_bit_offset) & 1;
			if(!show_status)
			{
				tf_printf("key %s have not been burned yet\n", key_map->name);

				return -1;
			}
			// 判断存放key的区域是否允许被查看
			//如果没有此标志位，表示一定可以查看
			if(key_map->show_bit_offset < 0)
			{
				break;
			}
			//如果存在标志位，并且不允许查看，则不读出，并且返回报错
			show_status = (sid_read_key(EFUSE_CHIP_CONFIG) >> key_map->show_bit_offset) & 1;
			if(show_status)
			{
				tf_printf("key %s don't show \n", key_map->name);

				return -1;
			}
			break;
		}
	}

	if (key_map == NULL)
	{
		tf_printf("[efuse] error: can't read the key (unknow)\n");

		return -1;
	}

	//烧写key
	key_start_addr = key_map->key_index;
	key_data_remain_size = key_map->store_max_bit / 8;

	memset(check_buf, 0, 512);
	if(key_data_remain_size & 3)
		key_data_remain_size = (key_data_remain_size + 3) & (~3);
	for(i=0;i<key_data_remain_size/4; i++)
	{
		check_buf[i] = sid_read_key(key_start_addr);
		key_start_addr += 4;
	}
	sunxi_dump(check_buf, key_map->store_max_bit / 8);
	memcpy((void *)read_buf, check_buf, key_map->store_max_bit / 8);
	flush_dcache_range((unsigned long)read_buf, key_map->store_max_bit / 8);

	return 0;
}

int sunxi_efuse_probe_security_mode(void)
{
	return sid_probe_security_mode();
}


