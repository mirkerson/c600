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
#include "common.h"
#include "asm/io.h"
#include "asm/arch/ccmu.h"
#include "asm/arch/ss.h"
#include "asm/arch/efuse.h"

DECLARE_GLOBAL_DATA_PTR;

static int ss_base_mode = 0;
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
static u32 __aw_endian4(u32 data)
{
	u32 d1, d2, d3, d4;
	d1= (data&0xff)<<24;
	d2= (data&0xff00)<<8;
	d3= (data&0xff0000)>>8;
	d4= (data&0xff000000)>>24;

	return (d1|d2|d3|d4);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :     hash_mode: 0: MD5   1:其他sha
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static u32 __sha_padding(u32 data_size, u8* text, u32 hash_mode)
{
	u32 i;
	u32 k, q;
	u32 size;
	u32 padding_buf[16];
    u8 *ptext;

	k = data_size/64;
	q = data_size%64;

	ptext = (u8*)padding_buf;
	memset(padding_buf, 0, 16 * sizeof(u32));
	if(q==0){
		padding_buf[0] = 0x00000080;

		if(hash_mode)
		{
			padding_buf[14] = data_size>>29;
			padding_buf[15] = data_size<<3;
			padding_buf[14] = __aw_endian4(padding_buf[14]);
			padding_buf[15] = __aw_endian4(padding_buf[15]);
		}
		else
		{
			padding_buf[14] = data_size<<3;
			padding_buf[15] = data_size>>29;
		}

		for(i=0; i<64; i++){
			text[k*64 + i] = ptext[i];
		}
		size = (k + 1)*64;
	}else if(q<56)
	{
		for(i=0; i<q; i++){
			ptext[i] = text[k*64 + i];
		}
		ptext[q] = 0x80;

		if(hash_mode)
		{
			padding_buf[14] = data_size>>29;
			padding_buf[15] = data_size<<3;
			padding_buf[14] = __aw_endian4(padding_buf[14]);
			padding_buf[15] = __aw_endian4(padding_buf[15]);
		}
		else
		{
			padding_buf[14] = data_size<<3;
			padding_buf[15] = data_size>>29;
		}

		for(i=0; i<64; i++){
			text[k*64 + i] = ptext[i];
		}
		size = (k + 1)*64;
	}else{
		for(i=0; i<q; i++){
			ptext[i] = text[k*64 + i];
		}
		ptext[q] = 0x80;
		for(i=0; i<64; i++){
			text[k*64 + i] = ptext[i];
		}

		//send last 512-bits text to SHA1/MD5
		for(i=0; i<16; i++){
			padding_buf[i] = 0x0;
		}

		if(hash_mode)
		{
			padding_buf[14] = data_size>>29;
			padding_buf[15] = data_size<<3;
			padding_buf[14] = __aw_endian4(padding_buf[14]);
			padding_buf[15] = __aw_endian4(padding_buf[15]);
		}
		else
		{
			padding_buf[14] = data_size<<3;
			padding_buf[15] = data_size>>29;
		}

		for(i=0; i<64; i++){
			text[(k + 1)*64 + i] = ptext[i];
		}
		size = (k + 2)*64;
	}

	return size;
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
static void __ss_encry_decry_end(uint task_id)
{
	uint int_en;

	int_en = readl(SS_ICR) & 0xf;
	int_en = int_en&(0x01<<task_id);
	if(int_en!=0)
	{

	   while((readl(SS_ISR)&(0x01<<task_id))==0) {};
	}
}

static void __ss_secure_encry_decry_end(uint task_id)
{
	uint int_en;

	int_en = readl(SS_S_ICR) & 0xf;
	int_en = int_en&(0x01<<task_id);
	if(int_en!=0)
	{

	   while((readl(SS_S_ISR)&(0x01<<task_id))==0) {};
	}
}
//align & padding
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
static void __rsa_padding(u8 *dst_buf, u8 *src_buf, u32 data_len, u32 group_len)
{
	int i = 0;

	memset(dst_buf, 0, group_len);
	for(i = group_len - data_len; i < group_len; i++)
	{
		dst_buf[i] = src_buf[group_len - 1 - i];
	}
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
void sunxi_ss_open(void)
{
	u32  reg_val;

	//enable SS working clock
	reg_val = readl(CCM_SS_SCLK_CTRL); //SS CLOCK
	reg_val &= ~(0xf<<24);
	reg_val |= 0x1<<24;
	reg_val &= ~(0x3<<16);
	reg_val |= 0x0<<16;			// /1
	reg_val &= ~(0xf);
	reg_val |= (4 -1);			// /4
	reg_val |= 0x1U<<31;
	writel(reg_val,CCM_SS_SCLK_CTRL);
	//enable SS AHB clock
	reg_val = readl(CCM_AHB1_GATE0_CTRL);
	reg_val |= 0x1<<5;		//SS AHB clock on
	writel(reg_val,CCM_AHB1_GATE0_CTRL);
	//del-assert SS reset
	reg_val = readl(CCM_AHB1_RST_REG0);
	reg_val |= 0x1<<5;		//SS AHB clock reset
	writel(reg_val,CCM_AHB1_RST_REG0);

	if((gd->securemode == SUNXI_NORMAL_MODE) || (gd->securemode == SUNXI_SECURE_MODE))
	{
		ss_base_mode = 1;
	}
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
void sunxi_ss_close(void)
{
}
//src_addr		//32B 对齐
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
int  sunxi_sha_calc(u8 *dst_addr, u32 dst_len,
					u8 *src_addr, u32 src_len)
{
	u32 reg_val = 0;
	u32 total_len = 0;
	u32 md_size = 32;
	s32 i = 0;
	u8  sign_buff[32 + 32], *p_sign;
	task_queue task0;

	memset(sign_buff, 0, sizeof(sign_buff));
	memset((u8 *)&task0 , 0 ,sizeof(task_queue));
	p_sign =  (u8 *)(((u32)sign_buff + 31)&(~31));

	total_len = __sha_padding(src_len, (u8 *)src_addr, 1)/4;	//计算明文长度

    task0.task_id = 0;
	task0.common_ctl = (19)|(1U << 31);
	task0.symmetric_ctl = 0;
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = 0;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;
	task0.data_len = total_len;

	task0.source[0].addr = (uint)src_addr;
	task0.source[0].length = total_len;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;

	task0.destination[0].addr = (uint)p_sign;
	task0.destination[0].length = 32/4;
	for(i=1;i<8;i++)
		 task0.destination[i].length = 0;
	task0.next_descriptor = 0;

	flush_cache((uint)&task0, sizeof(task0));
	flush_cache((uint)sign_buff, 64);
	flush_cache((uint)src_addr, total_len * 4);

	writel((uint)&task0, SS_TDQ); //descriptor address
	//enable SS end interrupt
	writel(0x1<<(task0.task_id), SS_ICR);
	//start SS
	writel(0x1, SS_TLR);
	//wait end
	__ss_encry_decry_end(task0.task_id);
	//copy data
	for(i=0; i< md_size; i++)
	{
	    dst_addr[i] = p_sign[i];   //从目的地址读生成的消息摘要
	}
	//clear pending
	reg_val = readl(SS_ISR);
	if((reg_val&(0x01<<task0.task_id))==(0x01<<task0.task_id))
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01<<task0.task_id);
	}
	writel(reg_val, SS_ISR);
	//SS engie exit
	writel(readl(SS_TLR) & (~0x1), SS_TLR);

	return 0;
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
s32 sunxi_rsa_calc(u8 * n_addr,   u32 n_len,
				   u8 * e_addr,   u32 e_len,
				   u8 * dst_addr, u32 dst_len,
				   u8 * src_addr, u32 src_len)
{
#define	TEMP_BUFF_LEN	((2048>>3) + 32)
	uint   i;
	task_queue task0;
	u32 reg_val = 0;
	u8	temp_n_addr[TEMP_BUFF_LEN],   *p_n;
	u8	temp_e_addr[TEMP_BUFF_LEN],   *p_e;
	u8	temp_src_addr[TEMP_BUFF_LEN], *p_src;
	u8	temp_dst_addr[TEMP_BUFF_LEN], *p_dst;
	u32 mod_bit_size = 2048;

	u32 mod_size_len_inbytes = mod_bit_size/8;

	p_n = (u8 *)(((u32)temp_n_addr + 31)&(~31));
	p_e = (u8 *)(((u32)temp_e_addr + 31)&(~31));
	p_src = (u8 *)(((u32)temp_src_addr + 31)&(~31));
	p_dst = (u8 *)(((u32)temp_dst_addr + 31)&(~31));

	__rsa_padding(p_src, src_addr, src_len, mod_size_len_inbytes);
	__rsa_padding(p_n, n_addr, n_len, mod_size_len_inbytes);
	memset(p_e, 0, mod_size_len_inbytes);
	memcpy(p_e, e_addr, e_len);

	task0.task_id = 0;
	task0.common_ctl = (32 | (1U<<31));      //ss method:rsa
	task0.symmetric_ctl = 0;
	task0.asymmetric_ctl = (2<<28);
	task0.key_descriptor = (uint)p_e;
	task0.iv_descriptor = (uint)p_n;
	task0.ctr_descriptor = 0;
	task0.data_len = mod_size_len_inbytes/4;     //word in uint
	task0.source[0].addr= (uint)p_src;
	task0.source[0].length = mod_size_len_inbytes/4;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (uint)p_dst;
	task0.destination[0].length = mod_size_len_inbytes/4;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;

	flush_cache((uint)&task0, sizeof(task0));
	flush_cache((uint)temp_n_addr, TEMP_BUFF_LEN);
	flush_cache((uint)temp_e_addr, TEMP_BUFF_LEN);
	flush_cache((uint)temp_src_addr, TEMP_BUFF_LEN);
	flush_cache((uint)temp_dst_addr, TEMP_BUFF_LEN);

	writel((uint)&task0, SS_TDQ); //descriptor address
	//enable SS end interrupt
	writel(0x1<<(task0.task_id), SS_ICR);
	//start SS
	writel(0x1, SS_TLR);
	//wait end
	__ss_encry_decry_end(task0.task_id);

	__rsa_padding(dst_addr, p_dst, mod_bit_size/64, mod_bit_size/64);
	//clear pending
	reg_val = readl(SS_ISR);
	if((reg_val&(0x01<<task0.task_id))==(0x01<<task0.task_id))
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01<<task0.task_id);
	}
	writel(reg_val, SS_ISR);
	//SS engie exit
	writel(readl(SS_TLR) & (~0x1), SS_TLR);

	return 0;
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
int sunxi_aes_encrypt_rssk_hdcp_to_dram(u8 *src_addr, u8 *dst_addr, u32 dst_len)
{
	u32 reg_val;
	u8  tmp_src_align[4096 +64], *src_align;
	u8  tmp_dst_align[4096 +64], *dst_align;
	u8  tmp_key_map[256 +64],    *key_map;
	u32 *key_pt;
	task_queue task0;
	int i;
	u32 cts_size;
    int key_start_addr  = 0;

	src_align = (u8 *)(((u32)tmp_src_align + 63)&(~63));
	dst_align = (u8 *)(((u32)tmp_dst_align + 63)&(~63));
	key_map = (u8 *)(((u32)tmp_key_map + 63)&(~63));

	memset(key_map, 0, 256);
    memset((void *)&task0,0x00,sizeof(task_queue));
    memcpy(src_align, src_addr, dst_len);
	//set encrypt mode
	cts_size = ((dst_len + 15) & (~15))>>2;
	memset(src_align + dst_len, 0, 16);

	key_start_addr = EFUSE_RSSK;
	key_pt = (u32 *)key_map;
	for(i=0;i<32;i+=4)
	{
		*key_pt++ = sid_read_key(key_start_addr);
		key_start_addr += 4;
	}

	task0.task_id = 0;
	task0.common_ctl = (1U<<31) | (SS_DIR_ENCRYPT<<8) | SS_METHOD_AES;
	task0.symmetric_ctl = (SS_KEY_SELECT_RSSK<<20) | (SS_AES_KEY_256BIT<<0)  | (SS_AES_MODE_ECB<<8) | \
	                      (SS_CTR_32BIT<<2)     | (SS_CFB_WIDTH_8BIT<<18) | (1<<16);
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = (u32)key_map;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len = cts_size;     //word in byte
	task0.source[0].addr= (u32)src_align;
	task0.source[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)dst_align;
	task0.destination[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush&clean cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)key_map, 256);
	flush_cache((uint)src_align, dst_len);
	flush_cache((uint)dst_align, dst_len);
	//set the task addr
	writel((u32)&task0, SS_S_TDQ);
	//enable irq
	reg_val = readl(SS_S_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_S_ICR);
	//start
	writel(0x1, SS_S_TLR);
	//wait finish
	__ss_secure_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_S_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_S_ISR);
	//stop
	writel(0x0, SS_S_TLR);
	//disable irq
	reg_val = readl(SS_S_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_S_ICR);
	//
	flush_cache((uint)dst_align, 64);
	memcpy(dst_addr, dst_align, cts_size*4);

	return 0;
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
int sunxi_aes_decrypt_rssk_hdcp_to_keysram(u8 *src_addr, u32 src_len)
{
	u32 reg_val;
	u8  tmp_src_align[4096 + 64], *src_align;
	u8  tmp_key_map[256 + 64], *key_map;
	task_queue task0;
	int i;
	u32 cts_size;

	src_align = (u8 *)(((u32)tmp_src_align + 64)&(~63));
	key_map = (u8 *)(((u32)tmp_key_map + 64)&(~63));

	memset(key_map, 0, 256);
    memset((void *)&task0,0x00,sizeof(task_queue));
    memcpy(src_align, src_addr, src_len);
	//set decrypt mode
	cts_size = ((src_len + 15) & (~15))>>2;
	memset(src_align + src_len, 0, 16);

	task0.task_id = 0;
	task0.common_ctl = (1U<<31) | (SS_DIR_DECRYPT<<8) | SS_METHOD_AES;
	task0.symmetric_ctl = (SS_KEY_SELECT_RSSK<<20) | (SS_AES_KEY_256BIT<<0)  | (SS_AES_MODE_ECB<<8) | \
	                      (SS_CTR_32BIT<<2)     | (SS_CFB_WIDTH_8BIT<<18) | (1<<16);
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = (u32)key_map;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len = cts_size;     //word in byte
	task0.source[0].addr= (u32)src_align;
	task0.source[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)HDCP_KEYSRAM_BASE;
	task0.destination[0].length = 0x50;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush&clean cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)tmp_key_map, 256);
	flush_cache((uint)tmp_src_align, src_len);
	//set the task addr
	writel((u32)&task0, SS_S_TDQ);
	//enable irq
	reg_val = readl(SS_S_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_S_ICR);
	//start
	writel(0x1, SS_S_TLR);
	//wait finish
	__ss_secure_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_S_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_S_ISR);
	//stop
	writel(0x0, SS_S_TLR);
	//disable irq
	reg_val = readl(SS_S_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_S_ICR);

	return 0;
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
int sunxi_aes_decrypt_rssk_hdcp_to_dram(u8 *src_addr, u32 src_len, u8 *dst_addr)
{
	u32 reg_val;
	u8  tmp_src_align[4096 + 64], *src_align;
	u8  tmp_key_map[256 + 64], *key_map;
	u32 *key_pt;
	u8  tmp_ret_align[512 + 64], *ret_align;
	task_queue task0;
	int i;
	u32 cts_size, key_start_addr;

	src_align = (u8 *)(((u32)tmp_src_align + 64)&(~63));
	key_map = (u8 *)(((u32)tmp_key_map + 64)&(~63));
	ret_align = (u8 *)(((u32)tmp_ret_align + 64)&(~63));

    memcpy(src_align, src_addr, src_len);
    memset((void *)&task0,0x00,sizeof(task_queue));
    memset(ret_align,0x00,512);
	memcpy(src_align, src_addr, src_len);

	key_start_addr = EFUSE_RSSK;
	key_pt = (u32 *)key_map;
	for(i=0;i<32;i+=4)
	{
		*key_pt++ = sid_read_key(key_start_addr);
		key_start_addr += 4;
	}
	//set decrypt mode
	cts_size = ((src_len + 15) & (~15))>>2;
	memset(src_align + src_len, 0, 16);

	task0.task_id = 0;
	task0.common_ctl = (1U<<31) | (SS_DIR_DECRYPT<<8) | SS_METHOD_AES;
	task0.symmetric_ctl = (SS_KEY_SELECT_INPUT<<20) | (SS_AES_KEY_256BIT<<0)  | (SS_AES_MODE_ECB<<8) | \
	                      (SS_CTR_32BIT<<2)       | (SS_CFB_WIDTH_8BIT<<18) | (1<<16);
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = (u32)key_map;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len = cts_size;     //word in byte
	task0.source[0].addr= (u32)src_align;
	task0.source[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)ret_align;
	task0.destination[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush&clean cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)key_map, 256);
	flush_cache((uint)src_align, src_len);
	flush_cache((uint)ret_align, 512);
	//set the task addr
	writel((u32)&task0, SS_S_TDQ);
	//enable irq
	reg_val = readl(SS_S_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_S_ICR);
	//start
	writel(0x1, SS_S_TLR);
	//wait finish
	__ss_secure_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_S_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_S_ISR);
	//stop
	writel(0x0, SS_S_TLR);
	//disable irq
	reg_val = readl(SS_S_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_S_ICR);
	//
	flush_cache((u32)ret_align, 512);
	memcpy(dst_addr, ret_align, cts_size<<2);
	//

	return 0;
}

int sunxi_md5_keysram_calcute(void *md5_buf, int md5_buf_len)
{
	u32  reg_val;
	u8  tmp_dst_align[64 + 64], *dst_align;
	task_queue task0;
	int i;

	if(md5_buf_len < 16)
	{
		printf("md5 calcute failed: the dst memory is not long enough\n");

		return -1;
	}
	dst_align = (u8 *)(((u32)tmp_dst_align + 64)&(~63));

    memset((void *)&task0,0x00,sizeof(task_queue));
	task0.task_id = 0;
	task0.common_ctl = (SS_METHOD_MD5 | (1U<<SS_INT_ENABLE_OFS));
	task0.symmetric_ctl = 0;
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = 0;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len         = 80;     //word in uint
	task0.source[0].addr   = (u32)HDCP_KEYSRAM_BASE;
	task0.source[0].length = 80;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)dst_align;
	task0.destination[0].length = 4;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)dst_align, 64);
	//set the task addr
	writel((u32)&task0, SS_S_TDQ);
	//enable irq
	reg_val = readl(SS_S_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_S_ICR);
	//start
	writel(0x1, SS_S_TLR);
	//wait finish
	__ss_secure_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_S_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_S_ISR);
	//stop
	writel(0x0, SS_S_TLR);
	//disable irq
	reg_val = readl(SS_S_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_S_ICR);
	//memcpy
	flush_cache((uint)dst_align, 64);
	memcpy(md5_buf, dst_align, 16);

	return 0;
}


int sunxi_md5_dram_calcute(void *src_buf, int src_len, void *md5_buf, int md5_buf_len)
{
	u32  reg_val;
	u32  total_len;
	u8  tmp_dst_align[64 + 64], *dst_align;
	u8  tmp_src_align[4096 + 64], *src_align;
	task_queue task0;
	int i;

	if(md5_buf_len < 16)
	{
		printf("md5 calcute failed: the dst memory is not long enough\n");

		return -1;
	}
	dst_align = (u8 *)(((u32)tmp_dst_align + 64)&(~63));
	src_align = (u8 *)(((u32)tmp_src_align + 64)&(~63));

	memcpy(src_align, src_buf, src_len);
    memset((void *)&task0,0x00,sizeof(task_queue));
	memset(dst_align, 0, 64);

	total_len = __sha_padding(src_len, src_align, 0)/4;
	//
	task0.task_id = 0;
	task0.common_ctl = (SS_METHOD_MD5 | (1U<<SS_INT_ENABLE_OFS));
	task0.symmetric_ctl = 0;
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = 0;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len         = total_len;     //word in uint
	task0.source[0].addr   = (u32)src_align;
	task0.source[0].length = total_len;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)dst_align;
	task0.destination[0].length = 4;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)dst_align, 64);
	flush_cache((uint)src_align, total_len * 4);
	//set the task addr
	writel((u32)&task0, SS_TDQ);
	//enable irq
	reg_val = readl(SS_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_ICR);
	//start
	writel(0x1, SS_TLR);
	//wait finish
	__ss_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_ISR);
	//stop
	writel(0x0, SS_TLR);
	//disable irq
	reg_val = readl(SS_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_ICR);
	//memcpy
	flush_cache((uint)dst_align, 64);
	memcpy(md5_buf, dst_align, 16);

	return 0;
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
int sunxi_aes_encrypt(u8 *src_addr, u8 *dst_addr, u32 data_bytes, u8 *key_buf, u32 aes_key_mode, u32 aes_mode)
{
	u32 reg_val;
	u8  tmp_src_align[4096 +64], *src_align;
	u8  tmp_dst_align[4096 +64], *dst_align;
	u8  tmp_key_map[256 +64],    *key_map;
	task_queue task0;
	u32 cts_size, i;

	if(data_bytes & 15)
	{
		printf("sunxi_aes_encrypt err: the input data must be 16 bytes align\n");

		return -1;
	}

	src_align = (u8 *)(((u32)tmp_src_align + 63)&(~63));
	dst_align = (u8 *)(((u32)tmp_dst_align + 63)&(~63));
	key_map = (u8 *)(((u32)tmp_key_map + 63)&(~63));

    memset((void *)&task0,0x00,sizeof(task_queue));
    memcpy(src_align, src_addr, data_bytes);
	//set encrypt mode
	cts_size = data_bytes>>2;

	if(aes_key_mode == SS_AES_KEY_128BIT)
	{
		memcpy(key_map, key_buf, 128/8);
	}
	else if(aes_key_mode == SS_AES_KEY_192BIT)
	{
		memcpy(key_map, key_buf, 192/8);
	}
	else if(aes_key_mode == SS_AES_KEY_256BIT)
	{
		memcpy(key_map, key_buf, 256/8);
	}
	else
	{
		printf("sunxi_aes_encrypt err: the input key type is invalid\n");
		printf("must be SS_AES_KEY_128BIT or SS_AES_KEY_192BIT or SS_AES_KEY_256BIT\n");

		return -1;
	}

	if((aes_mode != SS_AES_MODE_ECB) && (aes_mode != SS_AES_MODE_CBC))
	{
		printf("sunxi_aes_encrypt err: the aes mode is invalid\n");
		printf("must be SS_AES_MODE_ECB or SS_AES_MODE_CBC\n");

		return -1;
	}

	task0.task_id = 0;
	task0.common_ctl = (1U<<31) | (SS_DIR_ENCRYPT<<8) | SS_METHOD_AES;
	task0.symmetric_ctl = (SS_KEY_SELECT_INPUT<<20) | (aes_key_mode<<0)  | (aes_mode<<8) | \
	                      (SS_CTR_32BIT<<2)     | (SS_CFB_WIDTH_8BIT<<18) | (1<<16);
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = (u32)key_map;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len = cts_size;     //word in byte
	task0.source[0].addr= (u32)src_align;
	task0.source[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)dst_align;
	task0.destination[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush&clean cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)key_map, 256);
	flush_cache((uint)src_align, data_bytes);
	flush_cache((uint)dst_align, data_bytes);
	//set the task addr
	writel((u32)&task0, SS_TDQ);
	//enable irq
	reg_val = readl(SS_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_ICR);
	//start
	writel(0x1, SS_TLR);
	//wait finish
	__ss_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_ISR);
	//stop
	writel(0x0, SS_TLR);
	//disable irq
	reg_val = readl(SS_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_ICR);
	//
	flush_cache((uint)dst_align, data_bytes);
	memcpy(dst_addr, dst_align, data_bytes);

	return 0;
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
int sunxi_aes_decrypt(u8 *src_addr, u8 *dst_addr, u32 data_bytes, u8 *key_buf, u32 aes_key_mode, u32 aes_mode)
{
	u32 reg_val;
	u8  tmp_src_align[4096 +64], *src_align;
	u8  tmp_dst_align[4096 +64], *dst_align;
	u8  tmp_key_map[256 +64],    *key_map;
	task_queue task0;
	u32 cts_size, i;

	if(data_bytes & 15)
	{
		printf("sunxi_aes_encrypt err: the input data must be 16 bytes align\n");

		return -1;
	}

	src_align = (u8 *)(((u32)tmp_src_align + 63)&(~63));
	dst_align = (u8 *)(((u32)tmp_dst_align + 63)&(~63));
	key_map = (u8 *)(((u32)tmp_key_map + 63)&(~63));

    memset((void *)&task0,0x00,sizeof(task_queue));
    memcpy(src_align, src_addr, data_bytes);

	//set encrypt mode
	cts_size = data_bytes>>2;

	if(aes_key_mode == SS_AES_KEY_128BIT)
	{
		memcpy(key_map, key_buf, 128/8);
	}
	else if(aes_key_mode == SS_AES_KEY_192BIT)
	{
		memcpy(key_map, key_buf, 192/8);
	}
	else if(aes_key_mode == SS_AES_KEY_256BIT)
	{
		memcpy(key_map, key_buf, 256/8);
	}
	else
	{
		printf("sunxi_aes_encrypt err: the input key type is invalid\n");
		printf("must be SS_AES_KEY_128BIT or SS_AES_KEY_192BIT or SS_AES_KEY_256BIT\n");

		return -1;
	}

	if((aes_mode != SS_AES_MODE_ECB) && (aes_mode != SS_AES_MODE_CBC))
	{
		printf("sunxi_aes_encrypt err: the aes mode is invalid\n");
		printf("must be SS_AES_MODE_ECB or SS_AES_MODE_CBC\n");

		return -1;
	}

	task0.task_id = 0;
	task0.common_ctl = (1U<<31) | (SS_DIR_DECRYPT<<8) | SS_METHOD_AES;
	task0.symmetric_ctl = (SS_KEY_SELECT_INPUT<<20) | (aes_key_mode<<0)  | (aes_mode<<8) | \
	                      (SS_CTR_32BIT<<2)     | (SS_CFB_WIDTH_8BIT<<18) | (1<<16);
	task0.asymmetric_ctl = 0;
	task0.key_descriptor = (u32)key_map;
	task0.iv_descriptor = 0;
	task0.ctr_descriptor = 0;

	task0.data_len = cts_size;     //word in byte
	task0.source[0].addr= (u32)src_align;
	task0.source[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.source[i].length = 0;
	task0.destination[0].addr= (u32)dst_align;
	task0.destination[0].length = cts_size;
	for(i=1;i<8;i++)
		task0.destination[i].length = 0;
	task0.next_descriptor = 0;
	//flush&clean cache
	flush_cache((uint)&task0, sizeof(task_queue));
	flush_cache((uint)key_map, 256);
	flush_cache((uint)src_align, data_bytes);
	flush_cache((uint)dst_align, data_bytes);
	//set the task addr
	writel((u32)&task0, SS_TDQ);
	//enable irq
	reg_val = readl(SS_ICR);
	reg_val |= (1 << 0);
	writel(reg_val, SS_ICR);
	//start
	writel(0x1, SS_TLR);
	//wait finish
	__ss_encry_decry_end(0);
	//clear pending
	reg_val = readl(SS_ISR);
	if((reg_val&0x01)==0x01)
	{
	   reg_val &= ~(0x0f);
	   reg_val |= (0x01);
	}
	writel(reg_val, SS_ISR);
	//stop
	writel(0x0, SS_TLR);
	//disable irq
	reg_val = readl(SS_ICR);
	reg_val &= ~(1 << 0);
	writel(reg_val, SS_ICR);

	flush_cache((uint)dst_align, data_bytes);
	memcpy(dst_addr, dst_align, data_bytes);

	return 0;
}
