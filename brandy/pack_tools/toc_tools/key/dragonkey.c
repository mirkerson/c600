/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2014, Allwinner Technology Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/

#include "common.h"
#include "include.h"

int __sunxi_bytes_merge(u8 *dst, u32 dst_len, u8 *src, uint src_len);
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
int dragoncreatekey(char *lpCfg, char *key_dir)
{
	char all_key[1024];
	char cmdline[1024];
	int  i, ret;
	char *all_key_line[16];
	char keyname[32], keyvalue[256];

	memset(all_key, 0, 1024);
	memset(all_key_line, 0, 16 * sizeof(char *));

	GetPrivateProfileSection("key_rsa", all_key, 1024, lpCfg);
	GetPrivateProfileAllLine(all_key, all_key_line);
	for(i=0;i<16;i++)
	{
		if(all_key_line[i])
		{
			memset(keyname, 0, 32);
			memset(keyvalue, 0, 256);

			GetPrivateProfileLineInfo(all_key_line[i], keyname, keyvalue);

			if(keyvalue[0] == '\0')
			{
				continue;
			}

			memset(cmdline, 0, 1024);
			sprintf(cmdline, "genrsa -out %s/%s.pem 2048", key_dir, keyvalue);
			printf("create for %s\n", keyvalue);
			ret = system(cmdline);
			if(ret == 256)
			{
				printf("create private for %s.pem failed\n", keyvalue);

				return -1;
			}

			memset(cmdline, 0, 1024);
			sprintf(cmdline, "rsa -in %s/%s.pem -text -modulus -out %s/%s.bin", key_dir, keyvalue, key_dir, keyvalue);
			system(cmdline);
			if(ret == 256)
			{
				printf("create key der format %s.bin failed\n", keyvalue);

				return -1;
			}
		}
		else
		{
			break;
		}
	}

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
int dragon_create_rotpk(char *lpCfg, char *keypath)
{
	char all_toc0[1024];
	char type_name[32], line_info[256];
	int  ret;
	char hash_value[32];
	char rotpk[RSA_BIT_WITDH/8 * 2 + 16];
	char key_n[560], key_e[560], key_d[560], *p_key_n_tmp;
	unsigned int key_n_len, key_e_len;
	char current_path[MAX_PATH];
	char item_name[64], bin_name[64], key_name[64];

	memset(all_toc0, 0, 1024);
	if(GetPrivateProfileSection("toc0", all_toc0, 1024, lpCfg))
	{
		printf("dragon_create_rotpk err in GetPrivateProfileSection\n");

		return -1;
	}
	if(all_toc0[0] == '\0')
	{
		printf("dragon_create_rotpk err no content match toc0\n");

		return -1;
	}

	memset(type_name, 0, 32);
	memset(line_info, 0, 256);

	printf("all_toc0=%s\n", all_toc0);

	GetPrivateProfileLineInfo(all_toc0, type_name, line_info);

	printf("type_name=%s\n", type_name);
	printf("line_info=%s\n", line_info);

	memset(item_name, 0, 64);
	memset(bin_name , 0, 64);
	memset(key_name , 0, 64);

	GetPrivateEachItem(line_info, item_name, bin_name, key_name);

	printf("item=%s\n", item_name);
	printf("bin_name=%s\n", bin_name);
	printf("key_name=%s\n", key_name);
	//获取key名称
	memset(current_path, 0, MAX_PATH);
	sprintf(current_path, "%s/%s.bin", keypath, key_name);
	printf("key=%s\n", current_path);

	//获取对应的key的公钥
	memset(key_n, 0, 560);
	memset(key_e, 0, 560);
	memset(key_d, 0, 560);
	ret = getallkey(current_path, key_n, key_e, key_d);
	if(ret < 0)
	{
		printf("dragon_create_rotpk err in getpublickey bin\n");

		return -1;
	}
	p_key_n_tmp = key_n;
	while(*p_key_n_tmp == '0')
		p_key_n_tmp ++;

	key_n_len = strlen((const char *)p_key_n_tmp);
	key_e_len = strlen((const char *)key_e);
	//转换字符类型的key为数字类型，分别是n值和e值
	memset(rotpk, 0x91, sizeof(rotpk));
	if(__sunxi_bytes_merge((u8 *)rotpk, RSA_BIT_WITDH/8, (u8 *)p_key_n_tmp, key_n_len))
	{
		printf("dragon_create_rotpk err in sunxi_bytes_merge for keyn value\n");

		return -1;
	}
	if(__sunxi_bytes_merge((u8 *)rotpk+RSA_BIT_WITDH/8, RSA_BIT_WITDH/8, (u8 *)key_e, key_e_len))
	{
		printf("dragon_create_rotpk err in sunxi_bytes_merge for keye value\n");

		return -1;
	}

	sunxi_dump(rotpk, RSA_BIT_WITDH/8*2);

	memset(hash_value, 0, 32);
	sha256((const u8 *)rotpk, RSA_BIT_WITDH/8*2, (u8 *)hash_value);

	FILE *rotpk_file;

	memset(current_path, 0, MAX_PATH);
	sprintf(current_path, "%s/rotpk.bin", keypath);
	rotpk_file = fopen(current_path, "wb");
	if(rotpk_file == NULL)
	{
		printf("dragon_create_rotpk err in create rotpk file\n");

		return -1;
	}
	fwrite(hash_value, 256/8, 1, rotpk_file);
	fclose(rotpk_file);

	return 0;
}

