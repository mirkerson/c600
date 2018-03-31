/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : check.c
*
* Author : Gary.Wang
*
* Version : 1.1.0
*
* Date : 2007.10.12
*
* Description : This file provides a function to check Boot0 and Boot1.
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang       2007.10.12      1.1.0        build the file
*
************************************************************************************************************************
*/
#include "lib_i.h"
#include "string.h"


//#pragma arm section  code="check_magic"
/********************************************************************************
*函数名称: check_magic
*函数原型: __s32 check_magic( __u32 *mem_base, const char *magic )
*函数功能: 使用“算术和”来校验内存中的一段数据
*入口参数: mem_base       Boot文件在内存中的起始地址
*          magic          Boot的magic
*返 回 值: CHECK_IS_CORRECT      校验正确
*          CHECK_IS_WRONG        校验错误
*备    注:
********************************************************************************/
__s32 check_magic( __u32 *mem_base, const char *magic )
{
	uboot_file_head *bfh;

	bfh = (uboot_file_head *)mem_base;
	if(!(strncmp((const char *)bfh->boot_head.magic, magic, 8)))
	{
		return CHECK_IS_CORRECT;
	}

	return CHECK_IS_WRONG;
}

//#pragma arm section




//#pragma arm section  code="check_sum"
/********************************************************************************
*函数名称: check_sum
*函数原型: __s32 check_sum( __u32 *mem_base, __u32 size, const char *magic )
*函数功能: 使用“算术和”来校验内存中的一段数据
*入口参数: mem_base           待校验的数据在内存中的起始地址（必须是4字节对齐的）
*          size               待校验的数据的个数（以字节为单位，必须是4字节对齐的）
*返 回 值: CHECK_IS_CORRECT   校验正确
*          CHECK_IS_WRONG     校验错误
*备    注:
********************************************************************************/
__s32 check_sum( __u32 *mem_base, __u32 size )
{
	__u32 *buf;
	__u32 count;
	__u32 src_sum;
	__u32 sum;
	uboot_file_head  *bfh;


	bfh = (uboot_file_head *)mem_base;

	/* 生成校验和 */
	src_sum = bfh->boot_head.check_sum;                  // 从Boot_file_head中的“check_sum”字段取出校验和
	bfh->boot_head.check_sum = STAMP_VALUE;              // 将STAMP_VALUE写入Boot_file_head中的“check_sum”字段

	count = size >> 2;                         // 以 字（4bytes）为单位计数
	sum = 0;
	buf = (__u32 *)mem_base;
	do
	{
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
	}while( ( count -= 4 ) > (4-1) );

	while( count-- > 0 )
		sum += *buf++;

	bfh->boot_head.check_sum = src_sum;                  // 恢复Boot_file_head中的“check_sum”字段的值

	msg("sum=%x\n", sum);
	msg("src_sum=%x\n", src_sum);

	if( sum == src_sum )
		return CHECK_IS_CORRECT;               // 校验成功
	else
		return CHECK_IS_WRONG;                 // 校验失败
}

//#pragma arm section



//#pragma arm section  code="check_file"
/********************************************************************************
*函数名称: check_file
*函数原型: __s32 check_file( __u32 *mem_base, __u32 size, const char *magic )
*函数功能: 使用“算术和”来校验内存中的一段数据
*入口参数: mem_base       待校验的数据在内存中的起始地址（必须是4字节对齐的）
*          size           待校验的数据的个数（以字节为单位，必须是4字节对齐的）
*          magic          magic number, 待校验文件的标识码
*返 回 值: CHECK_IS_CORRECT       校验正确
*          CHECK_IS_WRONG         校验错误
*备    注:
********************************************************************************/
__s32 check_file( __u32 *mem_base, __u32 size, const char *magic )
{
	if( check_magic( mem_base, magic ) == CHECK_IS_CORRECT
        &&check_sum( mem_base, size  ) == CHECK_IS_CORRECT )
        return CHECK_IS_CORRECT;
    else
    	return CHECK_IS_WRONG;
}

//#pragma arm section
