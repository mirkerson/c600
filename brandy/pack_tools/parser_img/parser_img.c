// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "firmware/imgdecode.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH             (260)
#define  PARSER_BUFFER_MAX    (32 * 1024 * 1024)
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
    if ( FilePath[1] == '/')
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
	printf("parser_img img_name output_file_name main_type sub_type\n\n");
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
	char   image_name[MAX_PATH];
	char   dedicate_name[MAX_PATH];

	FILE   *dedicate_file = NULL;
	long long file_len, tmp_file_len;
	char *buffer;
	uint   file_offset, read_len;
	void *img_hd, *item_hd;
	int   ret = -1;

	if(argc == 5)
	{
		if(argv[1] == NULL)
		{
			printf("parser img fail: the image file name is empty\n");

			Usage();

			return __LINE__;
		}
		if(argv[2] == NULL)
		{
			printf("parser img fail: the dedicate file name is empty\n");

			Usage();

			return __LINE__;
		}
		if(argv[3] == NULL)
		{
			printf("parser img fail: the dedicate file main type is NULL\n");

			Usage();

			return __LINE__;
		}
		if(argv[4] == NULL)
		{
			printf("parser img fail: the dedicate file sub type is NULL\n");

			Usage();

			return __LINE__;
		}
	}
	else
	{
		Usage();

		return __LINE__;
	}
	GetFullPath(image_name,  argv[1]);
	GetFullPath(dedicate_name,   argv[2]);

	printf("\n");
	printf("image file Path=%s\n", image_name);
	printf("output file Path=%s\n", dedicate_name);
	printf("main type = %s\n", argv[3]);
	printf("sub type = %s\n", argv[4]);
	printf("\n");

	//打开固件
	img_hd = Img_Open(image_name);
	if(!img_hd)
	{
		printf("parser img fail: the iamge file is invalid\n");

		return __LINE__;
	}
	//打开数据文件
	item_hd = Img_OpenItem(img_hd, argv[3], argv[4]);
	if(!item_hd)
	{
		printf("parser img fail: the wanted file is not exist\n");

		return __LINE__;
	}
	//获取文件长度
	file_len = Img_GetItemSize(img_hd, item_hd);
	if(!file_len)
	{
		printf("parser img fail: the dedicate file length is 0, bad\n");

		goto __parser_img_out;
	}
	//创建文件
	dedicate_file = fopen(dedicate_name, "wb");
	if(dedicate_file == NULL)
	{
		printf("parser img fail: unable to create the dedicate file\n");

		goto __parser_img_out;
	}
	//创建缓存
	buffer = (char *)malloc(PARSER_BUFFER_MAX);
	if(buffer == NULL)
	{
		printf("parser img fail: unable to malloc buffer to store data\n");

		goto __parser_img_out;
	}
	//读取数据
	file_offset = 0;
	tmp_file_len = file_len;
	while(tmp_file_len >= PARSER_BUFFER_MAX)
	{
		read_len = Img_ReadItem_Continue(img_hd, item_hd, buffer, PARSER_BUFFER_MAX, file_offset);
		if(read_len != PARSER_BUFFER_MAX)
		{
			printf("parser img fail: read(step1) dedicate file err\n");

			goto __parser_img_out;
		}
		fwrite(buffer, PARSER_BUFFER_MAX, 1, dedicate_file);
		file_offset += PARSER_BUFFER_MAX;
		tmp_file_len -= PARSER_BUFFER_MAX;
	}
	if(tmp_file_len)
	{
		read_len = Img_ReadItem_Continue(img_hd, item_hd, buffer, (uint)tmp_file_len, file_offset);
		if(read_len != tmp_file_len)
		{
			printf("parser img fail: read(step2) dedicate file err\n");

			goto __parser_img_out;
		}
		fwrite(buffer, (uint)tmp_file_len, 1, dedicate_file);
	}
	printf("successfully writing the dedicate file\n");
	ret = 0;
	//数据读取完毕，做关闭动作
__parser_img_out:
	//关闭写入文件
	if(dedicate_file)
	{
		fclose(dedicate_file);
	}
	//关闭申请的内存
	if(buffer)
	{
		free(buffer);
	}
	//关闭固件文件句柄
	if(item_hd)
	{
		Img_CloseItem(img_hd, item_hd);
	}
	//关闭固件句柄
	Img_Close(img_hd);

	return ret;
}


