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
#ifndef  __COMMON_H__
#define  __COMMON_H__

#define  MAX_PATH   (260)

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "openssl/rsa.h"

/* test vectors from p1ovect1.txt */

#include <stdio.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/pem.h>

#undef _DEBUG
//#define _DEBUG

#ifdef _DEBUG
#   define sunxi_dbg(fmt,args...)	printf(fmt ,##args)
#else
#   define sunxi_dbg(fmt,args...)
#endif


void sunxi_GetFullPath(char *dName, const char *sName);
void sunxi_dump(char *buf, int count);

int sunxi_rsa_privatekey_encrypt(char *source_str, char *encryped_data, int data_bytes, char *key_path);
int sunxi_rsa_publickey_decrypt(char *source_str, char *decryped_data, int data_bytes, char *key_path);

#endif   // end of __COMMON_H__
