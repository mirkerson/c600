/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2009, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : boot0_i.h
*
* Author      : Gary.Wang
*
* Version     : 1.1.0
*
* Date        : 2009.09.13
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2009.09.13       1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef  __boot0_misc_h
#define  __boot0_misc_h

#include "boot0_i.h"

#define BOOT0_PRVT_HEAD_VERSION         "1230"    // X.X.XX
#define BOOT0_FILE_HEAD_VERSION         "1230"    // X.X.XX

#define BOOT0_VERSION                   "1230"    // X.X.XX



#define PAGE_BUF_FOR_BOOT0              ( EGON2_DRAM_BASE + SZ_1M )

#define DEBUG

#ifdef DEBUG
#   define msg(fmt,args...)				UART_printf2(fmt ,##args)
#else
#   define msg(fmt,args...)
#endif

#define HERE                            msg("file:%s, line:%u.\n", __FILE__, __LINE__);


#define OK                    0
#define ERROR                 1



#endif     //  ifndef __boot0_misc_h

/* end of boot0_misc.h */
