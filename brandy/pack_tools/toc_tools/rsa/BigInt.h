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

/*****************************************************************
大数运算库头文件：BigInt.h
*****************************************************************/
//允许生成1120位（二进制）的中间结果
#define BI_MAXLEN 130
#define DEC 10
#define HEX 16
#ifndef WIN_32
	#include <inttypes.h>
#else
	typedef unsinged __int64 uint64_t;
#endif

class CBigint;
class CBigInt
{
public:
//大数在0x100000000进制下的长度
    unsigned m_nLength;
//用数组记录大数在0x100000000进制下每一位的值
    unsigned int m_ulValue[BI_MAXLEN];

    CBigInt();
    ~CBigInt();

/*****************************************************************
基本操作与运算
Mov，赋值运算，可赋值为大数或普通整数，可重载为运算符“=”
Cmp，比较运算，可重载为运算符“==”、“!=”、“>=”、“<=”等
Add，加，求大数与大数或大数与普通整数的和，可重载为运算符“+”
Sub，减，求大数与大数或大数与普通整数的差，可重载为运算符“-”
Mul，乘，求大数与大数或大数与普通整数的积，可重载为运算符“*”
Div，除，求大数与大数或大数与普通整数的商，可重载为运算符“/”
Mod，模，求大数与大数或大数与普通整数的模，可重载为运算符“%”
*****************************************************************/
    void Mov( uint64_t A);
    void Mov(const CBigInt& A);
    CBigInt Add(const CBigInt& A);
    CBigInt Sub(const CBigInt& A);
    CBigInt Mul(const CBigInt& A);
    CBigInt Div(CBigInt& A);
    CBigInt Mod(CBigInt& A);
    CBigInt Add(unsigned A);
    CBigInt Sub(unsigned A);
    CBigInt Mul(unsigned A);
    CBigInt Div(unsigned A);
    unsigned Mod(unsigned A);
    int Cmp(const CBigInt& A);
    void Random(int bits);
/*****************************************************************
输入输出
Get，从字符串按10进制或16进制格式输入到大数
Put，将大数按10进制或16进制格式输出到字符串
*****************************************************************/
    void Get(char c[513], unsigned int system=HEX);
    void Put( unsigned int system=HEX);
	void ToFile(const char *f, unsigned int system=HEX);
	void ToFile_String_hgl(char *f, unsigned int system=HEX);
	void ToFile_buff_str(char *p_buff, unsigned int buff_len,unsigned int bit_width);
	void Resu(char *f, unsigned int system=HEX);



/*****************************************************************
RSA相关运算
Rab，拉宾米勒算法进行素数测试
Euc，欧几里德算法求解同余方程
RsaTrans，反复平方算法进行幂模运算
GetPrime，产生指定长度的随机大素数
*****************************************************************/
    int Rab();
    CBigInt Euc(CBigInt& A);
	CBigInt RsaTrans_en_de(const CBigInt& A, CBigInt& B, int EN_DE, char *resu);
    CBigInt RsaTrans_x(const CBigInt& A, CBigInt& B);
    CBigInt PowerMode(const CBigInt& p,  CBigInt& m);
	CBigInt ModMul(CBigInt& A, CBigInt& B);
	CBigInt MonPro(CBigInt& A, CBigInt& B, unsigned n);
	CBigInt ModExp(CBigInt& A, CBigInt& B);
    void GetPrime(int bits);
};
//#endif
