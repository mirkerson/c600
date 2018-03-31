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
int GetPrivateProfileSection(char *mainkey, char *dst_buf, int dst_buf_max, char *lpcfg)
{
	//检查参数
	if(lpcfg==NULL)
	{
		printf("GetPrivateProfileSection err: lpcfg is NULL\n");

		return -1;
	}
	if(mainkey==NULL)
	{
		printf("GetPrivateProfileSection err: mainkey is NULL\n");

		return -1;
	}
	if(dst_buf==NULL)
	{
		printf("GetPrivateProfileSection err: dst_buf is NULL\n");

		return -1;
	}
	//
	char lpcfg_full_name[MAX_PATH] = "";
	FILE *p_file;
	char line_buff[256], *p_line_buff, *ret_buff;
	char mainkey_in_cfg[128], *p_mainkey_in_cfg;
	int  pre_mode = 0, total_len = 0;

	GetFullPath(lpcfg_full_name, lpcfg);
	p_file = fopen(lpcfg_full_name, "rb");
	if(p_file == NULL)
	{
		printf("GetPrivateProfileSection err: cant open file %s\n", lpcfg_full_name);

		return -1;
	}
	fseek(p_file, 0, SEEK_SET);
	do
	{
		memset(line_buff, 0, 256);
		ret_buff = fgets(line_buff, 256, p_file);
		if(ret_buff == NULL)
		{
			if(feof(p_file))
			{
				printf("GetPrivateProfileSection read to end\n");

				break;
			}
			else
			{
				printf("GetPrivateProfileSection err: occur a err\n");

				fclose(p_file);

				return -1;
			}
		}
		p_line_buff = line_buff;
		while(1)
		{
			if((*p_line_buff == ' ') || (*p_line_buff == '	'))
				p_line_buff ++;
			else
				break;
		}
		if((*p_line_buff == ';') || (*p_line_buff == '#') || (*p_line_buff == 0xa) || (*p_line_buff == 0xd))
			continue;

		if(pre_mode == 0)
		{
			if(*p_line_buff == '[')	//main key
			{
				memset(mainkey_in_cfg, 0, 128);
				p_mainkey_in_cfg = mainkey_in_cfg;
				p_line_buff ++;
				while(1)
				{
					if((*p_line_buff == ' ') || (*p_line_buff == '	'))
						p_line_buff ++;
					else
						break;
				}
				while( (*p_line_buff != ']') && (*p_line_buff != ' ') && (*p_line_buff != '	'))
				{
					*p_mainkey_in_cfg ++ = *p_line_buff++;
				}
				if(!strcmp(mainkey_in_cfg, mainkey))
				{
					pre_mode = 1;
				}
				continue;
			}
		}
		else if(pre_mode == 1)	//现在是主键的有意义的下一行
		{
			if(*p_line_buff == '[')
				break;

			int len1 = strlen(p_line_buff);
			for(len1=0;(p_line_buff[len1]!=0xa) && (p_line_buff[len1]!=0xd);len1++);
			memcpy(dst_buf + total_len, p_line_buff, len1);
			total_len += len1 + 1;
		}
	}
	while(1);

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
int GetPrivateProfileAllLine(char *tar_buf, char **dst_buf)
{
	int i = 0;;
	char *p_tar_buf = tar_buf;

	while(1)
	{
		if(*p_tar_buf != '\0')
		{
			dst_buf[i++] = p_tar_buf;
			p_tar_buf += strlen(p_tar_buf);
		}
		else
		{
			if(*(p_tar_buf+1) == '\0')
			{
				break;
			}
			p_tar_buf ++;
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
int GetPrivateProfileLineInfo(char *tar_buf, char *name_buf, char *value_buf)
{
	int j,k;
	char *p_key = tar_buf;

	for(j=0;(p_key[j]==' ') || (p_key[j]=='	');j++);
	for(k=0;p_key[j]!='=';)
		name_buf[k++] = p_key[j++];
	for(j++;(p_key[j]==' ') || (p_key[j]=='	');j++);
	for(k=0;(p_key[j]!='#') && (p_key[j]!=';') && (p_key[j]!=0xd) && (p_key[j]!=0xa) && (p_key[j]!='\0');)
	{
		value_buf[k++] = p_key[j++];
	}

	return 0;
}/*
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
int GetPrivateEachItem(char *base, char *item, char *file_name, char *key_name)
{
	int i,j,k;
	char *p_base = base;
	char *all_item[3], *p_item;

	all_item[0] = item;
	all_item[1] = file_name;
	all_item[2] = key_name;

	j = 0;
	for(i=0;i<3;i++)
	{
		if(all_item[i] == NULL)
		{
			return 0;
		}
		p_item = all_item[i];
		for(;(p_base[j]==' ') || (p_base[j]=='	') || (p_base[j]==',');j++);
		k = 0;
		while(1)
		{
			if( (p_base[j]==' ') || (p_base[j]=='	') || (p_base[j]==',') )
			{
				break;
			}
			else if( (p_base[j]==0xa) || (p_base[j]==0xd) || (p_base[j]=='\0') )
			{
				return 0;
			}
			p_item[k++] = p_base[j++];
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
int ErasePrivateProfileSection(char *mainkey, char *lpcfg)
{
	//检查参数
	if(lpcfg==NULL)
	{
		printf("ErasePrivateProfileSection err: lpcfg is NULL\n");

		return -1;
	}
	if(mainkey==NULL)
	{
		printf("ErasePrivateProfileSection err: mainkey is NULL\n");

		return -1;
	}
	//
	int  ret = -1;
	char lpcfg_full_name[MAX_PATH] = "";
	u8   *file_buff;
	FILE *p_file;
	uint file_total_len, mainkey_file_pt=0, nextkey_file_pt=0;
	char line_buff[256], *p_line_buff, *ret_buff;
	char mainkey_in_cfg[128], *p_mainkey_in_cfg;
	int  pre_mode = 0;

	GetFullPath(lpcfg_full_name, lpcfg);
	p_file = fopen(lpcfg_full_name, "rb+");
	if(p_file == NULL)
	{
		printf("ErasePrivateProfileSection err: cant open file %s\n", lpcfg_full_name);

		return -1;
	}

	fseek(p_file, 0, SEEK_END);
	file_total_len = ftell(p_file);
	fseek(p_file, 0, SEEK_SET);

	file_buff = (u8 *)malloc(file_total_len);
	if(file_buff == NULL)
	{
		printf("ErasePrivateProfileSection err: cant malloc memory to store file data\n");

		fclose(p_file);

		return -1;
	}
	memset(file_buff, 0, file_total_len);

	do
	{
		memset(line_buff, 0, 256);
		ret_buff = fgets(line_buff, 256, p_file);
		if(ret_buff == NULL)
		{
			if(feof(p_file))
			{
				printf("ErasePrivateProfileSection read to end\n");

				goto __ErasePrivateProfileSection_err;
			}
			printf("ErasePrivateProfileSection err: occur a err\n");

			goto __ErasePrivateProfileSection_err;
		}
		p_line_buff = line_buff;
		while(1)
		{
			if((*p_line_buff == ' ') || (*p_line_buff == '	'))
				p_line_buff ++;
			else
				break;
		}
		if((*p_line_buff == ';') || (*p_line_buff == '#') || (*p_line_buff == 0xa) || (*p_line_buff == 0xd))
			continue;

		if(*p_line_buff == '[')	//main key
		{
			if(pre_mode == 0)
			{
				memset(mainkey_in_cfg, 0, 128);
				p_mainkey_in_cfg = mainkey_in_cfg;
				p_line_buff ++;

				while(1)
				{
					if((*p_line_buff == ' ') || (*p_line_buff == '	'))
						p_line_buff ++;
					else
						break;
				}
				while( (*p_line_buff != ']') && (*p_line_buff != ' ') && (*p_line_buff != '	'))
				{
					*p_mainkey_in_cfg ++ = *p_line_buff++;
				}
				if(!strcmp(mainkey_in_cfg, mainkey))
				{
					mainkey_file_pt = ftell(p_file);
					pre_mode = 1;
				}
				continue;
			}
			else if(pre_mode == 1)
			{
				nextkey_file_pt = ftell(p_file) - strlen(line_buff);
				fseek(p_file, nextkey_file_pt, SEEK_SET);
				fread(file_buff, file_total_len - nextkey_file_pt, 1, p_file);
				fseek(p_file, mainkey_file_pt, SEEK_SET);
				fwrite(file_buff, file_total_len - nextkey_file_pt, 1, p_file);
				ftruncate(fileno(p_file), ftell(p_file));
				ret = 0;

				break;
			}
		}
	}
	while(1);

__ErasePrivateProfileSection_err:
	if(file_buff)
		free(file_buff);
	if(p_file)
		fclose(p_file);

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
int WritePrivateProfileSection(char *mainkey, char *subkey, char *value, char *lpcfg)
{
	//检查参数
	if(lpcfg==NULL)
	{
		printf("WritePrivateProfileSection err: lpcfg is NULL\n");

		return -1;
	}
	if(mainkey==NULL)
	{
		printf("WritePrivateProfileSection err: mainkey is NULL\n");

		return -1;
	}
	if(subkey==NULL)
	{
		printf("WritePrivateProfileSection err: mainkey is NULL\n");

		return -1;
	}
	if(value==NULL)
	{
		printf("WritePrivateProfileSection err: mainkey is NULL\n");

		return -1;
	}
	//
	int  ret = -1;
	char lpcfg_full_name[MAX_PATH] = "";
	u8   *file_buff;
	FILE *p_file;
	uint file_total_len, mainkey_file_pt=0, nextkey_file_pt=0;
	char line_buff[256], *p_line_buff, *ret_buff;
	char mainkey_in_cfg[128], *p_mainkey_in_cfg;
	int  pre_mode = 0;

	GetFullPath(lpcfg_full_name, lpcfg);
	p_file = fopen(lpcfg_full_name, "rb+");
	if(p_file == NULL)
	{
		printf("WritePrivateProfileSection err: cant open file %s\n", lpcfg_full_name);

		return -1;
	}

	fseek(p_file, 0, SEEK_END);
	file_total_len = ftell(p_file);
	fseek(p_file, 0, SEEK_SET);

	file_buff = (u8 *)malloc(file_total_len);
	if(file_buff == NULL)
	{
		printf("WritePrivateProfileSection err: cant malloc memory to store file data\n");

		fclose(p_file);

		return -1;
	}
	memset(file_buff, 0, file_total_len);

	do
	{
		memset(line_buff, 0, 256);
		ret_buff = fgets(line_buff, 256, p_file);
		if(ret_buff == NULL)
		{
			if(feof(p_file))
			{
				printf("WritePrivateProfileSection read to end\n");

				goto __ErasePrivateProfileSection_err;
			}
			printf("WritePrivateProfileSection err: occur a err\n");

			goto __ErasePrivateProfileSection_err;
		}
		p_line_buff = line_buff;
		while(1)
		{
			if((*p_line_buff == ' ') || (*p_line_buff == '	'))
				p_line_buff ++;
			else
				break;
		}
		if((*p_line_buff == ';') || (*p_line_buff == '#') || (*p_line_buff == 0xa) || (*p_line_buff == 0xd))
			continue;

		if(*p_line_buff == '[')	//main key
		{
			if(pre_mode == 0)
			{
				memset(mainkey_in_cfg, 0, 128);
				p_mainkey_in_cfg = mainkey_in_cfg;
				p_line_buff ++;

				while(1)
				{
					if((*p_line_buff == ' ') || (*p_line_buff == '	'))
						p_line_buff ++;
					else
						break;
				}
				while( (*p_line_buff != ']') && (*p_line_buff != ' ') && (*p_line_buff != '	'))
				{
					*p_mainkey_in_cfg ++ = *p_line_buff++;
				}
				if(!strcmp(mainkey_in_cfg, mainkey))
				{
					mainkey_file_pt = ftell(p_file);
					pre_mode = 1;
				}
				continue;
			}
			else if(pre_mode == 1)
			{
				nextkey_file_pt = ftell(p_file) - strlen(line_buff);
				fseek(p_file, nextkey_file_pt, SEEK_SET);
				fread(file_buff, file_total_len - nextkey_file_pt, 1, p_file);
				fseek(p_file, nextkey_file_pt, SEEK_SET);
				//写入自己的数据
				char tmp_line_buff[1024];

				memset(tmp_line_buff, 0, 1024);
				sprintf(tmp_line_buff, "%s=\"%s\"\n", subkey, value);
				fwrite(tmp_line_buff, strlen(tmp_line_buff), 1, p_file);

				fwrite(file_buff, file_total_len - nextkey_file_pt, 1, p_file);
				ftruncate(fileno(p_file), ftell(p_file));
				ret = 0;

				break;
			}
		}
	}
	while(1);

__ErasePrivateProfileSection_err:
	if(file_buff)
		free(file_buff);
	if(p_file)
		fclose(p_file);

	return ret;
}
