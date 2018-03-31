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
#include <sys/stat.h>
#include <dirent.h>


#define   SYSCONFIG_ALIGN_SIZE      (16 * 1024)
#define   UBOOT_MAX_SIZE            (2 * 1024 * 1024)

static int align_uboot(char *uboot_mem, char *config_full_name, char *config_file_name);

static void usage(void)
{
	printf("Usage: update_uboot_v2  arg1 arg2\n");
	printf("arg1: the name of uboot\n");
	printf("arg2: the name of the config directory\n");

	return;
}


static int dir_list( char *path, char *uboot_buff)
{
    struct dirent* ent = NULL;
    DIR     *pDir;
    char    dir[1024];
    struct  stat   statbuf;

    if((pDir=opendir(path))==NULL)
    {
        printf("Cannot open directory: %s\n", path);

        return -1;
    }

    while(  (ent=readdir(pDir))!=NULL  )
    {
        //得到读取文件的绝对路径名
        memset(dir, 0, 1024);
        sprintf( dir, "%s/%s", path, ent->d_name );
        //得到文件信息
        lstat(dir, &statbuf);
        //判断是目录还是文件
        printf("full name: %s\n", dir);
        if( S_ISDIR(statbuf.st_mode) )
        {
            //排除当前目录和上级目录
            continue;
        }
        else
        {
            int len = strlen(dir) - 1;
            char *ext = dir + len;

            while ((*ext != '.') && (*ext != '/') && (len > 0))
            {
            	ext --;
            	len --;
            }
            if(!strncmp(ext, ".fex", 4))
            {
            	char cmdline[1024];
            	unsigned offset;

            	memset(cmdline, 0, 1024);
				sprintf(cmdline, "busybox unix2dos %s", dir);
				system(cmdline);

				memset(cmdline, 0, 1024);
				sprintf(cmdline, "script %s", dir);
				printf("cmdline=%s\n", cmdline);

				if(system(cmdline) == -1)
				{
					printf("script %s failed\n", dir);
					closedir(pDir);

					return -1;
				}

				offset = ext - dir + 1;
				dir[offset + 0] = 'b';
				dir[offset + 1] = 'i';
				dir[offset + 2] = 'n';
				if(align_uboot(uboot_buff, dir, ent->d_name))
				{
					closedir(pDir);

					return -1;
				}
            	printf("full name: %s\n", dir);
            }
        }
    }//while
    closedir(pDir);

	return 0;
}

int prepare_uboot(char *uboot_mem, unsigned uboot_len)
{
	unsigned   total_length, uboot_source_len;
	struct spare_boot_ctrl_head   *head;

	//读取原始uboot
	head = (struct spare_boot_ctrl_head *)uboot_mem;
	uboot_source_len = head->uboot_length;

	printf("uboot source = %d\n", uboot_source_len);
	printf("uboot input = %d\n", uboot_len);
	if(!uboot_source_len)
	{
		uboot_source_len = uboot_len;
	}

	printf("uboot source = %d\n", uboot_source_len);
	printf("uboot align = %d\n", head->align_size);
	if(uboot_source_len & (head->align_size - 1))
	{
		printf("uboot align = 0x%x\n", ~(head->align_size - 1));
		total_length = (uboot_source_len + head->align_size) & (~(head->align_size - 1));
	}
	else
	{
		total_length = uboot_source_len;
	}
	printf("uboot total = %d\n", total_length);

	head->uboot_length = total_length;
	head->length = total_length;

	head->check_sum = sunxi_sprite_generate_checksum(uboot_mem, head->length, head->check_sum);

	return 0;
}

static int align_uboot(char *uboot_mem, char *config_full_name, char *config_file_name)
{
	FILE      *config_file;
	unsigned  config_len, i;
	char      *config_mem;
	unsigned   config_total_len;
	struct spare_boot_ctrl_head   *head;
	script_head_t  *config_head;
	int    ret = -1;

	printf("config_full_name=%s\n", config_full_name);
	config_file = fopen(config_full_name, "rb");
	if(config_file == NULL)
	{
		printf("update_uboot_v2 failed: cant open the config file %s\n", config_full_name);

		return -1;
	}
	fseek(config_file, 0, SEEK_END);
	config_len = ftell(config_file);
	rewind(config_file);

	config_mem = (char *)malloc(config_len);
	if(config_mem == NULL)
	{
		printf("update_uboot_v2 failed: cant malloc memory for config\n");

		fclose(config_file);
	}
	fread(config_mem, 1, config_len, config_file);
	fclose(config_file);

	//读取原始uboot
	head = (struct spare_boot_ctrl_head *)uboot_mem;
	//读取配置
	config_head = (script_head_t *)config_mem;

	printf("config1=%d\n", config_len);
	printf("mainkey=%d\n", config_head->main_key_count);
	printf("length=%d\n", config_head->length);
	if(config_len & (SYSCONFIG_ALIGN_SIZE - 1))
	{
		config_total_len = (config_len + SYSCONFIG_ALIGN_SIZE) & (~(SYSCONFIG_ALIGN_SIZE - 1));
	}
	else
	{
		config_total_len = config_len;
	}
	config_head->length = config_total_len;

	memset(config_head->name, 0, 8);
	for(i=0;i<8;i++)
	{
		if(config_file_name[i]!='.')
		{
			config_head->name[i] = config_file_name[i];
		}
		else
		{
			break;
		}
	}
	printf("config length = %d\n", config_total_len);

	memcpy(uboot_mem + head->length, config_mem, config_len);
	head->length +=config_total_len;

	printf("head->length1=%d\n", head->uboot_length);
	printf("head->length2=%d\n", config_total_len);
	printf("head->length=%d\n", head->length);

	head->check_sum = sunxi_sprite_generate_checksum(uboot_mem, head->length, head->check_sum);

	free(config_mem);

	ret = 0;

	return ret;
}

int main(int argc, char *argv[])
{
	char  config_fullpath[512];
	char  uboot_fullpath[512];
	FILE   *uboot_file;
	char  *uboot_mem;
	unsigned uboot_len;
	struct spare_boot_ctrl_head   *head;

	if(argc != 3)
	{
		usage();

		return -1;
	}

	memset(uboot_fullpath, 0, 512);
	memset(config_fullpath, 0, 512);

	GetFullPath(uboot_fullpath, argv[1]);
	uboot_file = fopen(uboot_fullpath, "rb+");
	if(uboot_file == NULL)
	{
		printf("the uboot file cant be opend, please make sure %s exist\n", uboot_fullpath);

		return -1;
	}
	fseek(uboot_file, 0, SEEK_END);
	uboot_len = ftell(uboot_file);
	rewind(uboot_file);

	uboot_mem = (char *)malloc(UBOOT_MAX_SIZE);
	if(uboot_mem == NULL)
	{
		printf("update_uboot_v2 failed: cant malloc memory for uboot\n");

		fclose(uboot_file);
	}
	memset(uboot_mem, 0xff, UBOOT_MAX_SIZE);
	fread(uboot_mem, 1, uboot_len, uboot_file);
	prepare_uboot(uboot_mem, uboot_len);

	GetFullPath(config_fullpath, argv[2]);

	if(!dir_list(config_fullpath, uboot_mem))
	{
		head = (struct spare_boot_ctrl_head *)uboot_mem;

		printf("dir_list ok\n");
		rewind(uboot_file);
		fwrite(uboot_mem, 1, head->length, uboot_file);
	}
	else
	{
		printf("dir_list bad\n");
	}

	free(uboot_mem);
	fclose(uboot_file);

	return 0;
}

