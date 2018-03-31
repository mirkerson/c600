// update_uboot.cpp : Defines the entry point for the console application.
//

#include  "update_simg.h"

#define  MAX_PATH             (260)

#define  HASH_BUFFER_BYTES                (32 * 1024)

#define  PARTITION_BOOT_VERIFY_OFFSET     (4 * 1024)
#define  PARTITION_BOOT_VERIFY_COUNT      (20)
#define  PARTITION_BOOT_VERIFY_STEP       (256 * 1024)

#define  PARTITION_SYSTEM_VERIFY_OFFSET   (5 * 1024 * 1024)
#define  PARTITION_SYSTEM_VERIFY_COUNT    (40)
#define  PARTITION_SYSTEM_VERIFY_STEP     (5 * 1024 * 1024)

extern int unsparse_probe(char *source, unsigned int length);
extern int unsparse_direct_write(void *pbuf, int length, FILE *dfile);

unsigned int simg2img(char *s_name, char *d_name);
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
	printf("signature file_name\n\n");
}


int main(int argc, char* argv[])
{
	char   source_file_name[MAX_PATH];
	char   dest_file_name[MAX_PATH];
#if 1
	if(argc == 3)
	{
		if((argv[1] == NULL) || (argv[2] == NULL))
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
		memset(source_file_name, 0, MAX_PATH);
		memset(dest_file_name, 0, MAX_PATH);
		GetFullPath(source_file_name, argv[1]);
		GetFullPath(dest_file_name, argv[2]);
	}
	else
	{
		Usage();

		return __LINE__;
	}
#else
	strcpy(signature_file_name, str1);
#endif

	printf("source file name %s\n", source_file_name);
	printf("dest   file name %s\n", dest_file_name);

	return simg2img(source_file_name, dest_file_name);
}


unsigned int simg2img(char *s_name, char *d_name)
{
	FILE *s_file = NULL;
	FILE *d_file = NULL;
	char  *buffer = NULL;
	unsigned int s_length, tmp_slength;
	int   ret = -1;

	//打开源文件
	s_file = fopen(s_name, "rb");
	if(!s_file)
	{
		printf("simg2img error : unable to open source file\n");

		goto simg2img_err;
	}
	fseek(s_file, 0, SEEK_END);
	s_length = ftell(s_file);
	fseek(s_file, 0, SEEK_SET);
	if(!s_length)
	{
		printf("simg2img error : source file length is zero\n");

		goto simg2img_err;
	}
	printf("file size  %d bytes\n", s_length);
	//打开目标文件
	d_file = fopen(d_name, "wb");
	if(!d_file)
	{
		printf("simg2img error : unable to open source file\n");

		goto simg2img_err;
	}
	//申请内存，512k
	buffer = (char *)malloc(SIMG2IMG_BUFFER_SIZE * 2);
	if(!buffer)
	{
		printf("simg2img error : unable to malloc memory to store data\n");

		goto simg2img_err;
	}
	//读取1k
	fread(buffer, 1 * 1024, 1, s_file);
	rewind(s_file);
	//判断sparse格式
	if(unsparse_probe(buffer, 1024))
	{
		printf("simg2img error : not sparse format\n");

		goto simg2img_err;
	}
	//开始处理所有数据
	tmp_slength = s_length;
	while(tmp_slength >= SIMG2IMG_BUFFER_SIZE)
	{
		fread(buffer + SIMG2IMG_BUFFER_SIZE, SIMG2IMG_BUFFER_SIZE, 1, s_file);
		unsparse_direct_write(buffer + SIMG2IMG_BUFFER_SIZE, SIMG2IMG_BUFFER_SIZE, d_file);
		tmp_slength -= SIMG2IMG_BUFFER_SIZE;
	}
	if(tmp_slength)
	{
		fread(buffer + SIMG2IMG_BUFFER_SIZE, tmp_slength, 1, s_file);
		unsparse_direct_write(buffer + SIMG2IMG_BUFFER_SIZE, tmp_slength, d_file);
	}
	fclose(s_file);
	s_file = NULL;

	ret = 0;

simg2img_err:
	if(buffer)
	{
		free(buffer);
	}
	if(s_file)
	{
		fclose(s_file);
	}
	if(d_file)
	{
		fclose(d_file);
	}

	return ret;
}



