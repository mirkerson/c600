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
int sunxi_rsa_publickey_decrypt(char *source_str, char *decryped_data, int data_bytes, char *key_path)
{
    RSA *p_rsa;
    FILE *file;
    int  rsa_len, ret=-1;
	char fullname[MAX_PATH];

	memset(fullname, 0, MAX_PATH);
	sunxi_GetFullPath(fullname, key_path);

    if((file=fopen(fullname,"r"))==NULL)
    {
        printf("open key file error\n");

        return -1;
    }
    if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL)
    {
       printf("unable to get public key\n");

       goto __sunxi_rsa_publickey_decrypt_err;
    }

    rsa_len=RSA_size(p_rsa);
    ret = RSA_public_decrypt(rsa_len,(unsigned char *)source_str, (unsigned char*)decryped_data, p_rsa, RSA_NO_PADDING);

__sunxi_rsa_publickey_decrypt_err:
    RSA_free(p_rsa);
    fclose(file);

    return ret;
}
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
int sunxi_rsa_privatekey_encrypt(char *source_str, char *encryped_data, int data_bytes, char *key_path)
{
    RSA *p_rsa;
    FILE *file;
    int rsa_len, ret=-1;
	char fullname[MAX_PATH];

	memset(fullname, 0, MAX_PATH);
	sunxi_GetFullPath(fullname, key_path);

	sunxi_dbg("%s %s %d: keyname=%s\n", __FILE__, __func__, __LINE__, fullname);

    if((file=fopen(fullname,"r"))==NULL)
    {
        printf("open key file error\n");

        return -1;
    }
    if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL)
    {
        printf("unable to get private key\n");

        goto __sunxi_rsa_privatekey_encrypt_err;
    }
	sunxi_dbg("data bytes %d\n", data_bytes);

    rsa_len=RSA_size(p_rsa);
    sunxi_dbg("rsa_len=%d\n", rsa_len);
    ret = RSA_private_encrypt(rsa_len,(unsigned char *)source_str,(unsigned char*)encryped_data,p_rsa,RSA_NO_PADDING);
	sunxi_dbg("encrypt ret=%d\n", ret);
#ifdef _DEBUG
	if(ret >= 0)
	{
		sunxi_dbg("begin dump\n");
		sunxi_dump(encryped_data, 2048/8);
		sunxi_dbg("finish dump\n");
	}
#endif

__sunxi_rsa_privatekey_encrypt_err:
    RSA_free(p_rsa);
    fclose(file);

    return ret;
}


