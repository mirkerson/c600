// update.cpp : Defines the entry point for the console application.
//
#include <ctype.h>
#include <sys/types.h>
#include <common.h>
#include "firmware/imgdecode.h"


#define  PARSER_BUFFER_MAX    (32 * 1024 * 1024)


static int img_parser_file(char *image_name, char *name, char *out_dir);
static int img_list_files(char *image_name);

static void usage(void)
{
	printf("*************************start**************************\n");
	printf("*                                                      *\n");
	printf(" the usage of imgdecode:\n");
	printf(" when the program can work well, it will make a directory named imgdecode to store the files.\n");
    printf(" If the directory exists already, this step would be ignored\n");
	printf(" imgdecode imgname                 decode all files in the firmware\n");
	printf(" imadecode imgname -list           list all the mainkey and subkey in the img\n");
	printf(" imadecode imgname -name SUBKEY    decode the files whose sub  key = SUBKEY\n");
	printf("*                                                      *\n");
	printf("**************************end***************************\n");

	return;
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
	char *keyname=NULL;
	int  i, list_cmd=0;
	char cmdline[1024];
	char imgfullname[MAX_PATH]="";
	char tmpdirpath[MAX_PATH];
	FILE *img_file;

	if(argc == 2)
	{
		printf("findout all files\n");
	}
	else if((argc != 3) && (argc != 4))
	{
		printf("Dragon Img Decode: not enough parameters\n");
		usage();

		return -1;
	}

	for(i=2;i<argc;i+=2)
	{
		if(!strcmp(argv[i], "-name"))
		{
			keyname = argv[i+1];
			printf("findout all files whose main name is %s\n", keyname);
		}
		else if(!strcmp(argv[i], "-list"))
		{
			i--;
			printf("try to list all the subkey in the image\n");
			printf("all the other command will be ingored\n");
			list_cmd = 1;
		}
		else
		{
			printf("Dragon Img Decode: Unknown command\n");

			usage();
			return -1;
		}
	}

	GetFullPath(imgfullname, argv[1]);
	printf("imgpath=%s\n", imgfullname);

	img_file = fopen(imgfullname, "rb");
	if(img_file == NULL)
	{
		printf("Dragon Img Decode: The file cant be open\n");
		usage();

		return -1;
	}
	fclose(img_file);

	if(list_cmd)
	{
		img_list_files(imgfullname);

		return 0;
	}

	memset(tmpdirpath, 0, MAX_PATH);
	GetFullPath(tmpdirpath, "imgout");

	memset(cmdline, 0, 1024);
	sprintf(cmdline, "rm -rf %s", tmpdirpath);
	system(cmdline);

	memset(cmdline, 0, 1024);
	sprintf(cmdline, "mkdir -p %s", tmpdirpath);
	system(cmdline);

	if(img_parser_file(imgfullname, keyname, tmpdirpath))
	{
		usage();

		return -1;
	}

	return 0;
}

static int img_parser_file(char *image_name, char *name, char *out_dir)
{
	char   outfullpath[MAX_PATH];
	FILE   *dedicate_file = NULL;
	long long file_len, tmp_file_len;
	char *buffer;
	uint   file_offset, read_len;
	void *img_hd, *item_hd;
	int   ret = -1;

	//打开固件
	img_hd = Img_Open(image_name);
	if(!img_hd)
	{
		printf("Dragon Img Decode: the iamge file is invalid\n");

		return -1;
	}
	//打开数据文件
	item_hd = Img_OpenItem(img_hd, name);
	if(!item_hd)
	{
		printf("Dragon Img Decode: the wanted file is not exist\n");

		return -1;
	}
	//获取文件长度
	file_len = Img_GetItemSize(img_hd, item_hd);
	if(!file_len)
	{
		printf("Dragon Img Decode: the dedicate file length is 0\n");

		goto __parser_img_out;
	}
	//创建文件
	memset(outfullpath, 0, MAX_PATH);
	sprintf(outfullpath, "%s/%s.bin", out_dir, name);
	printf("try to create fils %s\n", outfullpath);

	dedicate_file = fopen(outfullpath, "wb");
	if(dedicate_file == NULL)
	{
		printf("Dragon Img Decode: unable to create the dedicate file\n");

		goto __parser_img_out;
	}
	//创建缓存
	buffer = (char *)malloc(PARSER_BUFFER_MAX);
	if(buffer == NULL)
	{
		printf("Dragon Img Decode: unable to malloc buffer to store data\n");

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
			printf("Dragon Img Decode: read(step1) dedicate file err\n");

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
			printf("Dragon Img Decode: read(step2) dedicate file err\n");

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






int img_list_files(char *image_name)
{
	void *img_hd;
	u8   *name;
	int   index;

	//打开固件
	img_hd = Img_Open(image_name);
	if(!img_hd)
	{
		printf("Dragon Img Decode: the iamge file is invalid\n");

		return -1;
	}
	index = 0;
	//打开数据文件
	printf("Ready to List All Item Name\n\n");
	do
	{
		name = Img_GetItem_Subname(img_hd, index);
		if(name != NULL)
		{
			index ++;
			printf("Item %4d: %s\n", index, name);
		}
		else
		{
			break;
		}
	}
	while(1);

	//关闭固件句柄
	Img_Close(img_hd);

	return 0;
}


