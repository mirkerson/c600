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

#ifndef __CREATE_CONFIG__H__
#define __CREATE_CONFIG__H__


int GetPrivateProfileSection(char *mainkey, char *dst_buf, int dst_buf_max, char *lpcfg);
int GetPrivateProfileAllLine(char *tar_buf, char **dst_buf);
int GetPrivateProfileLineInfo(char *tar_buf, char *name_buf, char *value_buf);
int GetPrivateEachItem(char *base, char *item, char *file_name, char *key_name);

int ErasePrivateProfileSection(char *mainkey, char *lpcfg);
int WritePrivateProfileSection(char *mainkey, char *subkey, char *value, char *lpcfg);


int getpublickey(char *key_file, char *publickey);
int getallkey(char *key_file, char *key_n, char *key_e, char *key_d);

void sunxi_dump(void *addr, unsigned int size);

#endif  //__CREATE_CONFIG__H__
