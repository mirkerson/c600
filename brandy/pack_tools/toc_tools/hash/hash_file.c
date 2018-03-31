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
int calchash_in_hex(char *binfile, char *hash_value)
{
	FILE *p_file;
	char *buff;
	uint  file_len;

	p_file = fopen(binfile, "rb");
	if(p_file == NULL)
	{
		printf("file %s cant be open to calc hash\n", binfile);

		return -1;
	}
	fseek(p_file, 0, SEEK_END);
	file_len = ftell(p_file);
	fseek(p_file, 0, SEEK_SET);

	buff = (char *)malloc(file_len);
	if(!buff)
	{
		printf("cant malloc memory to store file data\n");

		fclose(p_file);

		return -1;
	}
	fread(buff, file_len, 1, p_file);
	fclose(p_file);

	sha256((u8 *)buff, file_len, (u8 *)hash_value);

	free(buff);

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
int calchash(char *binfile, char *hash_value)
{
	FILE *p_file;
	char   hash256[64] ="";
	char   *buff;
	uint  file_len, k;
	char  ch, cl;

	p_file = fopen(binfile, "rb");
	if(p_file == NULL)
	{
		printf("file %s cant be open to calc hash\n", binfile);

		return -1;
	}
	fseek(p_file, 0, SEEK_END);
	file_len = ftell(p_file);
	fseek(p_file, 0, SEEK_SET);

	buff = (char *)malloc(file_len);
	if(!buff)
	{
		printf("cant malloc memory to store file data\n");

		fclose(p_file);

		return -1;
	}
	fread(buff, file_len, 1, p_file);
	fclose(p_file);

	sha256((u8 *)buff, file_len, (u8 *)hash256);
	for(k=0;k<32;k++)
	{
		ch = (hash256[k] & 0xf0) >> 4;
		cl = (hash256[k] & 0x0f) >> 0;

		if((ch >= 0) && (ch <= 9))
		{
			ch += '0';
		}
		else if(ch <= 15)
		{
			ch += 'A' - 10;
		}
		if((cl >= 0) && (cl <= 9))
		{
			cl += '0';
		}
		else if(cl <= 15)
		{
			cl += 'A' - 10;
		}

		hash_value[k*2] = ch;
		hash_value[k*2+1] = cl;
	}
	//printf("hash_value=%s\n", hash_value);

	free(buff);

	return 0;
}

