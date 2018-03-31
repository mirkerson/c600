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
#include "common.h"
#include "BigInt.h"
#ifdef WIN32
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include <iostream.h>
#include <fstream.h>
#else
#include <iostream>
#include <fstream>
using namespace std;
#endif
#include "timer.h"

#include <ctime>
#include <stdio.h>
#define ENCRYPT 1
#define DECRYPT 0

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
extern "C"
int rsa_sign_main(	char * p_n_str ,
						char * p_d_str,
						char * p_e_str,	//for check
						char * p_text ,

						unsigned char * p_sign_str_buff,
						unsigned int sign_str_buff_len,
						unsigned int bit_width)
{
//	CBigInt P,Q,N,D,E,x,y,z;
	CBigInt Text,RSA_n,RSA_e,RSA_d,Sign,Check_sign;
    //char *rsa_sign = "rsa_sign.bin";

	Timer mytime;   //定义时间类类型数据
	unsigned int sign_real_len = 0;

	Text.Mov(0);
	Sign.Mov(0);
	RSA_n.Mov(0);
	RSA_d.Mov(0);
	Check_sign.Mov(0);

//	=== step 2:use the generated N/E to encrypt===

	//cout << "n" <<endl;
	RSA_n.Get(p_n_str);
	//RSA_n.Put();

	//cout << "e" <<endl;
	RSA_e.Get(p_e_str);
	//RSA_e.Put();

	//cout << "d" <<endl;
	RSA_d.Get(p_d_str);
	//RSA_d.Put();

	// encrypt
	//cout << "text is" <<endl;
	Text.Get(p_text);
	//Text.Put();
	//cout<<endl;

	//cout << "now encrypting:" <<endl;
	//mytime.reset();
    //mytime.start();

  	Sign.Mov(Text.ModExp(RSA_d,RSA_n));  //加密

	//cout << "sign " <<endl;
	//Sign.Put();

    Sign.ToFile_buff_str((char *)p_sign_str_buff ,
						sign_str_buff_len,
						bit_width);

	//Sign.ToFile_String_hgl(rsa_sign);

	//check sign
	cout << "check sign " <<endl;
	Check_sign.Mov(Sign.ModExp(RSA_e,RSA_n));
	Check_sign.Put();

	//mytime.stop();
	//mytime.running();
    //cout<<endl<<"加密解密时间\t"<<mytime.seconds()<<"\tseconds"<<endl;


    return sign_real_len;

}




