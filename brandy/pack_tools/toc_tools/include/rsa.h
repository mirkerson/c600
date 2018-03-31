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

#ifndef __RSA_H__
#define __RSA_H__


int rsa_sign_main(	char * p_n_str ,
						char * p_d_str,
						char * p_e_str,	//for check
						char * p_text ,

						unsigned char * p_sign_str_buff,
						unsigned int sign_str_buff_len,
						unsigned int bit_width);

#endif /* !__RSA_H__ */

