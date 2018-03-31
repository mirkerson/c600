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

#ifndef __TOC_CONFIG__H__
#define __TOC_CONFIG__H__


typedef struct
{
	int  type;
	char item[64];
	char cnf[MAX_PATH];
	char bin[MAX_PATH];
	char key[MAX_PATH];
	char cert[MAX_PATH];
}
toc_descriptor_t;

#endif  //__TOC_CONFIG__H__
