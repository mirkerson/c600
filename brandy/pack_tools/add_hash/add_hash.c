// update.cpp : Defines the entry point for the console application.
//

//#include <common.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "private_boot0.h"
#include "private_uboot.h"

#define MAX_PATH  260

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("add git log hash into boot file \n");
	printf("eg : add_hash source_file dest_file file_flag \n");
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
    if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, MAX_PATH ) == NULL)
   {
        perror( "getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	char   source_file_name[MAX_PATH];
	char   log_file_name[MAX_PATH];
	FILE   *src_file = NULL, *log_file=NULL;
	unsigned int    length, log_length;
	int    ret = -1;
	char   *log_buf = NULL;
	char   *tmp_buf = NULL;
	boot0_file_head_t *boot0_head = NULL;
	boot_file_head_t *sboot_head = NULL;

	struct spare_boot_head_t *uboot_head = NULL;

	if(argc != 4)
	{
		Usage();

		return __LINE__;
	}
	if(!strcmp(argv[1], argv[2]))
	{
		Usage();

		return __LINE__;
	}

	GetFullPath(source_file_name, argv[1]);
	GetFullPath(log_file_name,   argv[2]);

	printf("\n");
	printf("source_file_name file Path=%s\n", source_file_name);
	printf("log_file_name file Path=%s\n", log_file_name);
	printf("\n");
	//打开源文件
	src_file = fopen(source_file_name, "rb+");
	if(src_file == NULL)
	{
		printf("add_hash err: unable to open source file\n");

		goto __add_hash_err;
	}
	//打开目标文件
	log_file = fopen(log_file_name, "rb+");
	if(log_file == NULL)
	{
		printf("add_hash err: unable to open log file\n");

		goto __add_hash_err;
	}
	//读取源文件数据
	fseek(src_file, 0, SEEK_END);
	length = ftell(src_file);
	fseek(src_file, 0, SEEK_SET);
	if(!length)
	{
		printf("src_file file size is 0\n");

		goto __add_hash_err;
	}
	fseek(log_file, 0, SEEK_END);
	log_length = ftell(log_file);
	fseek(log_file, 0, SEEK_SET);
	if(!log_length)
	{
		printf("log_file file size is 0\n");

		goto __add_hash_err;
	}
	tmp_buf = (char *)malloc(length);
	if(!tmp_buf)
	{
		printf("can not malloc memory for tmp_buf \n");
		goto __add_hash_err;
	}
	log_buf = (char *)malloc(log_length);
	if(!log_buf)
	{
		printf("can not malloc memory for log_buf \n");
		goto __add_hash_err;
	}

	fread(tmp_buf, length, 1, src_file);
	fseek(src_file, 0, SEEK_SET);
	fread(log_buf,log_length,1,log_file);
	fseek(log_file, 0, SEEK_SET);

	if(!strcmp(argv[3], "uboot"))	//处理
	{
		printf("add hash to uboot file \n");
		uboot_head = (struct spare_boot_head_t *)tmp_buf;
		memcpy((char*)uboot_head+sizeof(struct spare_boot_head_t),log_buf,log_length);
	}
	else if(!strcmp(argv[3], "boot0"))
	{
		printf("add hash to boot0 file \n");
		boot0_head = (boot0_file_head_t *)tmp_buf;
		memcpy((char*)boot0_head + sizeof(boot0_file_head_t),log_buf,log_length);
	}
	else if(!strcmp(argv[3],"sboot"))
	{
		printf("add hash to sboot file \n");
		sboot_head = (boot_file_head_t *)tmp_buf;
		memcpy((char*)sboot_head + sizeof(boot_file_head_t),log_buf,log_length);
	}
	fseek(log_file, 0, SEEK_SET);
	//回写文件
	fwrite(tmp_buf, length, 1, src_file);
	printf("successed \n");
	ret = 0;
__add_hash_err:
	if(tmp_buf)
	{
		free(tmp_buf);
	}
	if(log_buf)
	{
		free(log_buf);
	}
	if(src_file)
	{
		fclose(src_file);
	}
	if(log_file)
	{
		fclose(log_file);
	}

	return ret;
}
