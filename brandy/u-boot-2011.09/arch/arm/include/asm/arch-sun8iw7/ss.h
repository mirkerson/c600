/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/

#ifndef _SS_H_
#define _SS_H_

#include "platform.h"

#define SS_N_BASE			   SS_BASE      //non security
#define SS_S_BASE             (SS_BASE+0x800)      //security

#define SS_TDQ				  (SS_N_BASE + 0x00 + 0x800*ss_base_mode)
#define SS_CTR				  (SS_N_BASE + 0x04 + 0x800*ss_base_mode)
#define SS_ICR				  (SS_N_BASE + 0x08 + 0x800*ss_base_mode)
#define SS_ISR				  (SS_N_BASE + 0x0C + 0x800*ss_base_mode)
#define SS_TLR                (SS_N_BASE + 0x10 + 0x800*ss_base_mode)
#define SS_TSR                (SS_N_BASE + 0x14 + 0x800*ss_base_mode)
#define SS_ERR                (SS_N_BASE + 0x18 + 0x800*ss_base_mode)
#define SS_TPR                (SS_N_BASE + 0x1C + 0x800*ss_base_mode)
#define SS_PKEY               (SS_N_BASE + 0x30 + 0x800*ss_base_mode)
#define SS_PCTL               (SS_N_BASE + 0x34 + 0x800*ss_base_mode)


#define SS_S_TDQ				(SS_S_BASE + 0x00)
#define SS_S_CTR				(SS_S_BASE + 0x04)
#define SS_S_ICR				(SS_S_BASE + 0x08)
#define SS_S_ISR				(SS_S_BASE + 0x0C)
#define SS_S_TLR                (SS_S_BASE + 0x10)
#define SS_S_TSR                (SS_S_BASE + 0x14)
#define SS_S_ERR                (SS_S_BASE + 0x18)
#define SS_S_TPR                (SS_S_BASE + 0x1C)
#define SS_S_PKEY               (SS_S_BASE + 0x30)
#define SS_S_PCTL               (SS_S_BASE + 0x34)

#define SS_N_TDQ				(SS_N_BASE + 0x00)
#define SS_N_CTR				(SS_N_BASE + 0x04)
#define SS_N_ICR				(SS_N_BASE + 0x08)
#define SS_N_ISR				(SS_N_BASE + 0x0C)
#define SS_N_TLR                (SS_N_BASE + 0x10)
#define SS_N_TSR                (SS_N_BASE + 0x14)
#define SS_N_ERR                (SS_N_BASE + 0x18)
#define SS_N_TPR                (SS_N_BASE + 0x1C)
#define SS_N_PKEY               (SS_N_BASE + 0x30)
#define SS_N_PCTL               (SS_N_BASE + 0x34)


#define SS_INT_ENABLE           1
#define SS_INT_DISABLE          0

#define SS_METHOD_AES			0
#define SS_METHOD_DES			1
#define SS_METHOD_3DES			2
#define SS_METHOD_MD5			16
#define SS_METHOD_SHA1			17
#define SS_METHOD_SHA224		18
#define SS_METHOD_SHA256		19
#define SS_METHOD_SHA38 		20
#define SS_METHOD_SHA512		21
#define SS_METHOD_HMAC  		22
#define SS_METHOD_RSA			32
#define SS_METHOD_PRNG			49
#define SS_METHOD_TRNG			48
#define SS_METHOD_ECC			64

#define SS_DIR_ENCRYPT           0
#define SS_DIR_DECRYPT           1

#define SS_IV_MODE_CONSTANTS    (0<<16)
#define SS_IV_MODE_ARBITIARY    (1<<16)

#define SS_INT_ENABLE_OFS           (31)
#define SS_IV_MODE_OFS              (16)
#define SS_HMACSHA1_LASTP_FLAG_OFS  (15)
#define SS_OP_DIR_OFS               (8)

#define SS_KEY_SELECT_INPUT        (0)
#define SS_KEY_SELECT_SSK          (1)
#define SS_KEY_SELECT_HUK          (2)
#define SS_KEY_SELECT_RSSK         (3)

#define SS_CFB_WIDTH_1BIT       (0)
#define SS_CFB_WIDTH_8BIT       (1)
#define SS_CFB_WIDTH_64BIT      (2)
#define SS_CFB_WIDTH_128BIT     (3)

#define SS_AES_MODE_ECB         (0)
#define SS_AES_MODE_CBC         (1)
#define SS_AES_MODE_CTR         (2)
#define SS_AES_MODE_CTS         (3)
#define SS_AES_MODE_OFB         (4)
#define SS_AES_MODE_CFB         (5)
#define SS_AES_MODE_CBCMAC      (6)

#define SS_CTR_16BIT            (0)
#define SS_CTR_32BIT            (1)
#define SS_CTR_64BIT            (2)
#define SS_CTR_128BIT           (3)

#define SS_AES_KEY_128BIT       (0)
#define SS_AES_KEY_192BIT       (1)
#define SS_AES_KEY_256BIT       (2)

#define SS_KEY_SELECT_OFS           (20)
#define SS_CFB_MODE_WIDTH_OFS       (18)
#define SS_AESCTS_LASTP_FLAG_OFS    (16)
#define SS_OP_MODE_OFS              (8)
#define SS_CTR_WITDH_OFS            (2)
#define SS_AES_KEY_SIZE_OFS         (0)


#define SS_RSA_KEY_512BIT       (1)
#define SS_RSA_KEY_1024BIT      (2)
#define SS_RSA_KEY_2048BIT      (3)
#define SS_RSA_KEY_3072BIT      (3)
#define SS_RSA_KEY_4096BIT      (4)

#define SS_RSA_PUB_MODULUS_WIDTH_512	512
#define SS_RSA_PUB_MODULUS_WIDTH_1024	1024
#define SS_RSA_PUB_MODULUS_WIDTH_2048	2048
#define SS_RSA_PUB_MODULUS_WIDTH_3072	3072
#define SS_RSA_PUB_MODULUS_WIDTH_4096	4096

#define SS_ECC_GFP_POINTMUL     (0)
#define SS_ECC_GFP_POINTADD     (1)
#define SS_ECC_GFP_POINTDOU     (2)
#define SS_ECC_GFP_POINTVER     (3)
#define SS_ECC_GFP_ENCRYPT      (4)
#define SS_ECC_GFP_DECRYPT      (5)
#define SS_ECC_GFP_SIGNING      (6)

#define SS_RSA_KEY_SIZE_OFS         (28)
#define SS_RSA_BIG_OP_OFS           (16)
#define SS_ECC_PARAMETER_OFS        (12)
#define SS_ECC_GFP_OP_OFS           (4)

#define SS_SEED_SIZE			24

#define HDCP_KEYSRAM_BASE    (0x01C0B000)

typedef struct sg
{
   uint addr;
   uint length;
}sg;

typedef struct descriptor_queue
{
	uint task_id;
	uint common_ctl;
	uint symmetric_ctl;
	uint asymmetric_ctl;
	uint key_descriptor;
	uint iv_descriptor;
	uint ctr_descriptor;
	uint data_len;
	sg   source[8];
	sg   destination[8];
	uint next_descriptor;
	uint reserved[3];
}task_queue;


void sunxi_ss_open(void);
void sunxi_ss_close(void);
int  sunxi_sha_calc(u8 *dst_addr, u32 dst_len,
					u8 *src_addr, u32 src_len);

s32 sunxi_rsa_calc(u8 * n_addr,   u32 n_len,
				   u8 * e_addr,   u32 e_len,
				   u8 * dst_addr, u32 dst_len,
				   u8 * src_addr, u32 src_len);

int sunxi_aes_encrypt_rssk_hdcp_to_dram(u8 *src_addr, u8 *dst_addr, u32 dst_len);
int sunxi_aes_decrypt_rssk_hdcp_to_keysram(u8 *src_addr, u32 src_len);
int sunxi_aes_decrypt_rssk_hdcp_to_dram(u8 *src_addr, u32 src_len, u8 *dst_addr);

int sunxi_md5_keysram_calcute(void *md5_buf, int md5_buf_len);
int sunxi_md5_dram_calcute(void *src_buf, int src_len, void *md5_buf, int md5_buf_len);

int sunxi_aes_encrypt(u8 *src_addr, u8 *dst_addr, u32 data_bytes, u8 *key_buf, u32 aes_key_mode, u32 aes_mode);
int sunxi_aes_decrypt(u8 *src_addr, u8 *dst_addr, u32 data_bytes, u8 *key_buf, u32 aes_key_mode, u32 aes_mode);

#endif    /*  #ifndef _SS_H_  */
