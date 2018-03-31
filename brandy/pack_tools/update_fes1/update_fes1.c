// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "boot0_v2.h"
#include "check.h"
#include "script.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH             (260)

void *script_file_decode(char *script_name);
int update_for_fes1(char *fes1_name, int storage_type);
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
	char   source_fes1_name[MAX_PATH];
	char   script_file_name[MAX_PATH];
	FILE   *src_file = NULL;
//	FILE   *script_file;
//	int    source_length, script_length;
//	int    total_length;
//	char   *pbuf_source, *pbuf_script;
	int    storage_type = 0;
	char   *script_buf = NULL;


#if 1
	if(argc == 3)
	{
		if((argv[1] == NULL) || (argv[2] == NULL))
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
	GetFullPath(source_fes1_name,  argv[1]);
	GetFullPath(script_file_name,   argv[2]);
#else
	strcpy(source_fes1_name, str1);
	strcpy(script_file_name, str2);
#endif

	printf("\n");
	printf("fes1 file Path=%s\n", source_fes1_name);
	printf("script file Path=%s\n", script_file_name);
	printf("\n");
	//初始化配置脚本
	script_buf = (char *)script_file_decode(script_file_name);
	if(!script_buf)
	{
		printf("update fes1 error: unable to get script data\n");

		goto _err_out;
	}
	script_parser_init(script_buf);
	//读取原始fes1
	update_for_fes1(source_fes1_name, storage_type);
    //获取原始脚本长度
	printf("script update fes1 ok\n");
_err_out:
	if(script_buf)
	{
		free(script_buf);
	}

	return 0;
}


int update_for_fes1(char *fes1_name, int storage_type)
{
	FILE *fes1_file = NULL;
	boot0_file_head_t  *fes1_head;
	char *fes1_buf = NULL;
	int   length = 0;
	int   i;
	int   ret = -1;
	int   value[8];
    script_gpio_set_t   gpio_set[32];

	fes1_file = fopen(fes1_name, "rb+");
	if(fes1_file == NULL)
	{
		printf("update:unable to open fes1 file\n");
		goto _err_fes1_out;
	}
	fseek(fes1_file, 0, SEEK_END);
	length = ftell(fes1_file);
	fseek(fes1_file, 0, SEEK_SET);
	if(!length)
	{
		printf("fes1 file size is 0\n");
		goto _err_fes1_out;
	}
	fes1_buf = (char *)malloc(length);
	if(!fes1_buf)
	{
		printf("unable to malloc for fes1 update\n");
		goto _err_fes1_out;
	}
	fread(fes1_buf, length, 1, fes1_file);
	rewind(fes1_file);
	fes1_head = (boot0_file_head_t *)fes1_buf;
	//检查fes1的数据结构是否完整
    ret = check_file( (unsigned int *)fes1_buf, fes1_head->boot_head.length, BOOT0_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
    	printf("stage1 check file is error\n");
		goto _err_fes1_out;
	}
	//取出数据进行修正,DRAM参数
	if(script_parser_sunkey_all("dram_para", (void *)fes1_head->prvt_head.dram_para))
	{
		printf("script fetch dram para failed\n");
		goto _err_fes1_out;
	}
	//取出数据进行修正,UART参数
	if(!script_parser_fetch("uart_para", "uart_debug_port", value))
	{
		fes1_head->prvt_head.uart_port = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("uart_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			fes1_head->prvt_head.uart_ctrl[i].port      = gpio_set[i].port;
			fes1_head->prvt_head.uart_ctrl[i].port_num  = gpio_set[i].port_num;
			fes1_head->prvt_head.uart_ctrl[i].mul_sel   = gpio_set[i].mul_sel;
			fes1_head->prvt_head.uart_ctrl[i].pull      = gpio_set[i].pull;
			fes1_head->prvt_head.uart_ctrl[i].drv_level = gpio_set[i].drv_level;
			fes1_head->prvt_head.uart_ctrl[i].data      = gpio_set[i].data;
		}
	}
	//取出数据进行修正,debugenable参数
	if(!script_parser_fetch("jtag_para", "jtag_enable", value))
	{
		fes1_head->prvt_head.enable_jtag = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("jtag_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			fes1_head->prvt_head.jtag_gpio[i].port      = gpio_set[i].port;
			fes1_head->prvt_head.jtag_gpio[i].port_num  = gpio_set[i].port_num;
			fes1_head->prvt_head.jtag_gpio[i].mul_sel   = gpio_set[i].mul_sel;
			fes1_head->prvt_head.jtag_gpio[i].pull      = gpio_set[i].pull;
			fes1_head->prvt_head.jtag_gpio[i].drv_level = gpio_set[i].drv_level;
			fes1_head->prvt_head.jtag_gpio[i].data      = gpio_set[i].data;
		}
	}
	//数据修正完毕
	//重新计算校验和
	gen_check_sum( (void *)fes1_buf );
	//再检查一次
    ret = check_file( (unsigned int *)fes1_buf, fes1_head->boot_head.length, BOOT0_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
    	printf("stage2 check file is error\n");
		goto _err_fes1_out;
	}
	fwrite(fes1_buf, length, 1, fes1_file);

_err_fes1_out:
	if(fes1_buf)
	{
		free(fes1_buf);
	}
	if(fes1_file)
	{
		fclose(fes1_file);
	}

	return ret;
}


void *script_file_decode(char *script_file_name)
{
	FILE  *script_file;
	void  *script_buf = NULL;
	int    script_length;
	//读取原始脚本
	script_file = fopen(script_file_name, "rb");
	if(!script_file)
	{
        printf("update error:unable to open script file\n");
		return NULL;
	}
    //获取原始脚本长度
    fseek(script_file, 0, SEEK_END);
	script_length = ftell(script_file);
	if(!script_length)
	{
		fclose(script_file);
		printf("the length of script is zero\n");

		return NULL;
	}
	//读取原始脚本
	script_buf = (char *)malloc(script_length);
	if(!script_buf)
	{
		fclose(script_file);
		printf("unable malloc memory for script\n");

		return NULL;;
	}
    fseek(script_file, 0, SEEK_SET);
	fread(script_buf, script_length, 1, script_file);
	fclose(script_file);

	return script_buf;
}
