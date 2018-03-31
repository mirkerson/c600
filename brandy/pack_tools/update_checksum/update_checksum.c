// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "spare_head.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH             (260)
int  script_length;
int  align_size;

void *script_file_decode(char *script_name);
int merge_uboot(char *source_uboot_name, char *script_name);
int genchecksum_for_uboot(char *uboot_name);
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
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("update.exe script file path para file path\n\n");
}

int main(int argc, char* argv[])
{
	char   str1[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\eGon\\boot1.bin";
	char   str2[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\wboot\\bootfs\\script.bin";
	char   source_uboot_name[MAX_PATH];
//	int    source_length, script_length;
//	int    total_length;
//	char   *pbuf_source, *pbuf_script;
	char   *script_buf = NULL;

#if 1
	if(argc == 2)
	{
		if(argv[1] == NULL)
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
	}
	else
	{
		Usage();

		return __LINE__;
	}
	GetFullPath(source_uboot_name,  argv[1]);
#else
	strcpy(source_uboot_name, str1);
#endif

	printf("\n");
	printf("uboot file Path=%s\n", source_uboot_name);
	printf("\n");
	//读取原始uboot
	if(genchecksum_for_uboot(source_uboot_name))
	{
		printf("update uboot error: update error\n");

		return -1;
	}

	return 0;

}

int genchecksum_for_uboot(char *uboot_name)
{
	FILE *uboot_file = NULL;
	struct spare_boot_head_t  *uboot_head;
	char *uboot_buf = NULL;
//	int   i;
	int   ret = -1;
	int   align_size, uboot_length;
	int   origin_length;
	char  buffer[512];

	uboot_file = fopen(uboot_name, "rb+");
	if(uboot_file == NULL)
	{
		printf("genchecksum uboot error : unable to open uboot file\n");
		goto _err_uboot_out;
	}
	fseek(uboot_file, 0, SEEK_END);
	origin_length = ftell(uboot_file);
	fseek(uboot_file, 0, SEEK_SET);
	if(!origin_length)
	{
		printf("update uboot error : uboot length is zero\n");
		goto _err_uboot_out;
	}
	//获取对齐尺寸
	fread(buffer, 512, 1, uboot_file);
	uboot_head = (struct spare_boot_head_t *)buffer;
	align_size = uboot_head->boot_head.align_size;
	uboot_length = origin_length;
	if(origin_length & (align_size - 1))
	{
		uboot_length = (origin_length + align_size) & (~(align_size - 1));
	}
	//开辟空间
	uboot_buf = (char *)malloc(uboot_length);
	if(!uboot_buf)
	{
		printf("update uboot error : fail to malloc memory for uboot\n");
		goto _err_uboot_out;
	}
	memset(uboot_buf, 0xff, uboot_length);
	rewind(uboot_file);
	fread(uboot_buf, origin_length, 1, uboot_file);
	//还原文件大小
	uboot_head = (struct spare_boot_head_t *)uboot_buf;
	uboot_head->boot_head.length = uboot_length;
	uboot_head->boot_head.uboot_length = uboot_length;
    //重新生成校验和
    gen_check_sum(uboot_buf);
	//写入文件
	rewind(uboot_file);
	fwrite(uboot_buf, uboot_length, 1, uboot_file);
	ret = 0;

_err_uboot_out:
	if(uboot_buf)
	{
		free(uboot_buf);
	}
	if(uboot_file)
	{
		fclose(uboot_file);
	}

	return ret;
}




