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
*    note          :  根据bin文件，和基准配置文件，创建一个证书文件的配置文件
*
*
************************************************************************************************************
*/
static int	__create_cert_for_toc1(char *cnf_name, char *keyname, char *cert_name)
{
	char cmdline[1024];
	int  ret;

	//printf("cnf_name=%s\n", cnf_name);
	//printf("keyname=%s\n", keyname);
	//printf("cert_name=%s\n", cert_name);
	memset(cmdline, 0, 1024);
	sprintf(cmdline, "req -new -x509 -sha256 -batch		    \
	                        -key %s.pem 					\
	                        -out %s.crt					    \
	                        -config %s      				\
	                        -extensions usr_cert",			\
	                        keyname,			            \
	                        cert_name,		                \
	                        cnf_name);
	//printf("cmdline=%s\n", cmdline);
	ret = system(cmdline);
	if(ret == 256)
	{
		printf("create_certif run cmd req err\n");

		return -1;
	}

	memset(cmdline, 0, 1024);
	sprintf(cmdline, "x509 -in							    \
	                         %s.crt					        \
	                         -inform PEM					\
	                         -out %s.der					\
	                         -outform DER",					\
	                         cert_name,		                \
	                         cert_name);
	//printf("cmdline=%s\n", cmdline);
	ret = system(cmdline);
	if(ret == 256)
	{
		printf("create_certif run cmd x509 err\n");

		return -1;
	}

	printf("Make cert toc1 %s.der success.\n", cert_name);

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
*    note          :  根据bin文件，和基准配置文件，创建一个证书文件的配置文件
*
*
************************************************************************************************************
*/
static int __createcnf(char *cnffile, char *cnfpath_base)
{
	char cmdline[1024];
	int  ret;

	//填充到新的扩展项
	memset(cmdline, 0, 1024);
	sprintf(cmdline, "cp %s %s", cnfpath_base, cnffile);
	ret = system(cmdline);
	if(ret == 256)
		return -1;

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
int create_cert_for_toc1(char *lpCfg, toc_descriptor_t *toc1, char *keypath, char *cnfpath_base)
{
	char all_toc1[1024];
	int  i;
	char *all_toc1_line[16];
	char type_name[32], line_info[256];
	int  ret;
	char hash_value[128], key_value[560];
	char current_path[MAX_PATH];
	char bin_name[64], key_name[64];

	memset(all_toc1, 0, 1024);
	memset(all_toc1_line, 0, 16 * sizeof(char *));

	if(GetPrivateProfileSection("toc1", all_toc1, 1024, lpCfg))
	{
		printf("dragoncreate_toc1_certif err in GetPrivateProfileSection\n");

		return -1;
	}
	if(all_toc1[0] == '\0')
	{
		printf("dragoncreate_toc1_certif err no content match toc1\n");

		return -1;
	}

	if(GetPrivateProfileAllLine(all_toc1, all_toc1_line))
	{
		printf("dragoncreate_toc1_certif err in GetPrivateProfileAllLine\n");

		return -1;
	}
	for(i=0;i<16;i++)
	{
		if(all_toc1_line[i])
		{
			memset(type_name, 0, 32);
			memset(line_info, 0, 256);

			GetPrivateProfileLineInfo(all_toc1_line[i], type_name, line_info);

			memset(bin_name , 0, 64);
			memset(key_name , 0, 64);

			GetPrivateEachItem(line_info, toc1[i].item, bin_name, key_name);

			if(!strcmp(type_name, "rootkey"))
			{
				toc1[i].type = ROOTKEY_TYPE;
			}
			else if(!strcmp(type_name, "onlykey"))
			{
				toc1[i].type = ONLYKEY_TYPE;
			}
			else
			{
				toc1[i].type = NORMAL_TYPE;
			}
			//保存key名称
			sprintf(toc1[i].key, "%s/%s", keypath, key_name);
			//保存证书配置名称
			memset(current_path, 0, MAX_PATH);
			GetFullPath(current_path, CNFPATH_CONST);
			sprintf(toc1[i].cnf, "%s/%s.cnf", current_path, toc1[i].item);
			ret = __createcnf(toc1[i].cnf, cnfpath_base);
			if(ret)
			{
				printf("dragoncreate_toc1_certif err in createcnf bin\n");

				return -1;
			}
			//保存证书名称
			memset(current_path , 0, MAX_PATH);
			GetFullPath(current_path, CERTPATH_CONST);
			sprintf(toc1[i].cert, "%s/%s", current_path, toc1[i].item);

			GetFullPath(toc1[i].bin, bin_name);

//			printf("toc1[i].bin=%s\n", toc1[i].bin);
//			printf("toc1[i].key=%s\n", toc1[i].key);
//			printf("toc1[i].cnf=%s\n", toc1[i].cnf);
//			printf("toc1[i].cert=%s\n", toc1[i].cert);


			if(i==0)
			{
				printf("root_cnf_file=%s\n", toc1[i].cnf);
				ret = ErasePrivateProfileSection("usr_cert", toc1[i].cnf);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in ErasePrivateProfileSection root\n");

					return -1;
				}
			}
			else
			{
				memset(hash_value, 0, 128);
				ret = calchash(toc1[i].bin, hash_value);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in calchash\n");

					return -1;
				}
				printf("cnf_file=%s\n", toc1[i].cnf);
				ret = ErasePrivateProfileSection("usr_cert", toc1[i].cnf);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in ErasePrivateProfileSection bin\n");

					return -1;
				}
				ret = WritePrivateProfileSection("usr_cert", toc1[i].item, hash_value, toc1[i].cnf);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in WritePrivateProfileSection bin\n");

					return -1;
				}
				//获取对应的key的公钥
				memset(key_value, 0, 560);
				memset(current_path, 0, MAX_PATH);
				sprintf(current_path, "%s.bin", toc1[i].key);
				printf("current_path=%s\n", current_path);
				ret = getpublickey(current_path, key_value);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in getpublickey bin\n");

					return -1;
				}
				ret = WritePrivateProfileSection("usr_cert", toc1[i].item, key_value, toc1[0].cnf);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in WritePrivateProfileSection rootkey\n");

					return -1;
				}

				ret = __create_cert_for_toc1(toc1[i].cnf, toc1[i].key, toc1[i].cert);
				if(ret < 0)
				{
					printf("dragoncreate_toc1_certif err in dragoncreatecertif rootkey\n");

					return -1;
				}
			}
		}
		else
		{
			break;
		}
	}

	if(i>0)
	{
		ret = __create_cert_for_toc1(toc1[0].cnf, toc1[0].key, toc1[0].cert);
		if(ret < 0)
		{
			printf("dragoncreate_toc1_certif err in dragoncreatecertif rootkey\n");

			return -1;
		}
	}

	return 0;
}

