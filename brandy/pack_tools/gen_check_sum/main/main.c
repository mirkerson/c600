// update.cpp : Defines the entry point for the console application.
//

#include "common.h"
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
	char   dest_file_name[MAX_PATH];
	FILE   *src_file = NULL, *dst_file=NULL;
	u32    length;
	int    ret = -1;
	char   *src_buf = NULL;
	char   tmp_buf[512];
	standard_boot_file_head_t  *head_p;
	u32    align_length, align_bytes;
	int    i;


	if(argc != 3)
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
	GetFullPath(dest_file_name,   argv[2]);

	printf("\n");
	printf("fes1 file Path=%s\n", source_file_name);
	printf("script file Path=%s\n", dest_file_name);
	printf("\n");
	//打开源文件
	src_file = fopen(source_file_name, "rb+");
	if(src_file == NULL)
	{
		printf("gen check sum err: unable to open source file\n");

		goto __gen_check_sum_err;
	}
	//打开目标文件
	dst_file = fopen(dest_file_name, "wb");
	if(dst_file == NULL)
	{
		printf("gen check sum err: unable to create dest file\n");

		goto __gen_check_sum_err;
	}
	//读取源文件数据
	fseek(src_file, 0, SEEK_END);
	length = ftell(src_file);
	fseek(src_file, 0, SEEK_SET);
	if(!length)
	{
		printf("fes1 file size is 0\n");

		goto __gen_check_sum_err;
	}
	//对齐
	fread(tmp_buf, 512, 1, src_file);
	head_p = (standard_boot_file_head_t *)tmp_buf;

	align_bytes = head_p->length;
	align_length = (length + align_bytes - 1) & (~(align_bytes - 1));
	head_p->length = align_length;

	src_buf = (char *)malloc(align_length);
	if(!src_buf)
	{
		printf("gen check sum err: unable to malloc memory for source file\n");

		goto __gen_check_sum_err;
	}

	//memset(src_buf, 0xff, align_length);
	for(i = 0; i < align_length/4; i++)
	{
		*((int *)(src_buf)+i) = rand();
	}

	memcpy(src_buf, tmp_buf, 512);
	fread(src_buf + 512, length - 512, 1, src_file);
	fclose(src_file);
	src_file = NULL;
	//检查源文件
	ret = check_magic( src_buf, BOOT0_MAGIC);
	if(ret)
	{
		ret = check_magic( src_buf, TOC0_MAGIC);
		if(ret)
		{
			printf("gen check sum err: the input file is invalid\n");

			goto __gen_check_sum_err;
		}
	}
	//计算校验和
	gen_check_sum(src_buf);
	//写到新文件
	fwrite(src_buf, align_length, 1, dst_file);
	fclose(dst_file);
	dst_file = NULL;
	ret = 0;

__gen_check_sum_err:
	if(src_buf)
	{
		free(src_buf);
	}
	if(src_file)
	{
		fclose(src_file);
	}
	if(dst_file)
	{
		fclose(dst_file);
	}

	return ret;
}



