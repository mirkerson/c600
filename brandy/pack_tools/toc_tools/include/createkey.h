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

#ifndef __CREATE_KEY__H__
#define __CREATE_KEY__H__


int dragoncreatekey(char *lpCfg, char *key_dir);
int dragon_create_rotpk(char *lpCfg, char *keypath);

#endif  //__CREATE_KEY__H__
