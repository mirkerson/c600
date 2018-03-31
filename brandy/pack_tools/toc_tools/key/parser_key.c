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
*    note          :
*
*
************************************************************************************************************
*/
int getpublickey(char *key_file, char *publickey)
{
	FILE * p_file = NULL;
	char   line_buf[128];
	char  *p_buf;
	int    i,j=0;

	p_file = fopen((const char *)key_file, "rb");
	if(p_file == NULL)
	{
		printf("%s cant oepn file\n", __func__);

		return -1;
	}
	fseek(p_file, 0, SEEK_SET);
	memset(line_buf, 0, 128);
	fgets(line_buf, 128, p_file);
	if(!strcmp(line_buf, "Private-Key: (2048 bit)"))
	{
		printf("The first line is not Private-Key: (2048 bit)\n");

		goto __GetPublicKeyValue_err;
	}
	memset(line_buf, 0, 128);
	fgets(line_buf, 128, p_file);
	if(!strcmp(line_buf, "modulus:"))
	{
		printf("The secend line is not modulus:\n");

		goto __GetPublicKeyValue_err;
	}
	memset(line_buf, 0, 128);
	do
	{
		if(fgets(line_buf, 128, p_file) == NULL)
		{
			printf("when read file %s occur a err\n", key_file);

			goto __GetPublicKeyValue_err;
		}
		if(!memcmp(line_buf, "publicExponent", strlen("publicExponent")))
		{
			//printf("Find to the publickey end\n");

			p_buf = line_buf;
			for(i=0;p_buf[i]!='(';i++);
			i+=3;
			for(;p_buf[i]!=')';i++)
			{
				publickey[j++]=p_buf[i];
			}

			fclose(p_file);
			//printf("before return 0\n");

			return 0;
		}
		p_buf = line_buf;
		for(i=0;p_buf[i]!='\0';i++)
		{
			if((p_buf[i] != 0x20) && (p_buf[i] != 0x3a) && (p_buf[i] != 0xa))
			{
				publickey[j++]=p_buf[i];
			}
		}
	}
	while(1);

__GetPublicKeyValue_err:
	fclose(p_file);
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
int getallkey(char *key_file, char *key_n, char *key_e, char *key_d)
{
	FILE * p_file = NULL;
	char   line_buf[128];
	char  *p_buf, *ret_buff, *p_dst_buff;
	int    i,j=0;
	int    mode = 0;
	int    ret = 0;

	p_file = fopen((const char *)key_file, "rb");
	if(p_file == NULL)
	{
		printf("%s cant oepn file\n", __func__);

		return -1;
	}
	fseek(p_file, 0, SEEK_SET);
	memset(line_buf, 0, 128);
	fgets(line_buf, 128, p_file);
	if(!strcmp(line_buf, "Private-Key: (2048 bit)"))
	{
		printf("The first line is not Private-Key: (2048 bit)\n");

		goto __getallkey_err;
	}
	do
	{
		memset(line_buf, 0, 128);
		ret_buff = fgets(line_buf, 128, p_file);
		if(ret_buff == NULL)
		{
			if(feof(p_file))
			{
				printf("getallkey read to end\n");

				goto __getallkey_err;
			}
			printf("getallkey err: occur a err\n");

			ret = -1;
			goto __getallkey_err;
		}

		if(!strncmp(line_buf, "modulus:", strlen("modulus:")))
		{
			printf("find public key\n");

			p_dst_buff = key_n;
			j = 0;
			mode = 1;

			continue;
		}
		else if(!strncmp(line_buf, "publicExponent:", strlen("publicExponent:")))
		{
			printf("find e key\n");

			p_buf = line_buf;
			for(i=0;p_buf[i]!='(';i++);
			i+=3;
			j=0;
			for(;p_buf[i]!=')';i++)
			{
				key_e[j++]=p_buf[i];
			}
			continue;
		}
		else if(!strncmp(line_buf, "privateExponent:", strlen("privateExponent:")))
		{
			printf("find private key\n");

			p_dst_buff = key_d;
			mode = 3;
			j = 0;

			continue;
		}
		else if(!strncmp(line_buf, "prime", strlen("prime")))
		{
			break;
		}

		if(mode != 0)
		{
			p_buf = line_buf;
			for(i=0;p_buf[i]!='\0';i++)
			{
				if((p_buf[i] != 0x20) && (p_buf[i] != 0x3a) && (p_buf[i] != 0xa))
				{
					p_dst_buff[j++]=p_buf[i];
				}
			}
		}
	}
	while(1);
__getallkey_err:
	fclose(p_file);
	return ret;
}


