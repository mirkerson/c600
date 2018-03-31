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
#include "include.h"

int createtoc0(toc_descriptor_t *toc0, char *toc0_name)
{
	char toc0_full_name[MAX_PATH];
	char toc0_content_name[MAX_PATH];
	FILE *p_file, *src_file;
	uint offset, offset_align;
	uint file_len;
	int  ret = -1;
	sbrom_toc0_head_info_t  *toc0_head;
	sbrom_toc0_item_info_t  *item_head, *p_item_head;
	char *toc0_content;
	toc_descriptor_t *p_toc0 = toc0;

	//申请空间用于保存文件内容
	toc0_content = (char *)malloc(10 * 1024 * 1024);
	if(toc0_content == NULL)
	{
		printf("createtoc0 err: cant malloc memory to store file content\n");

		goto __createtoc0_err;
	}
	memset(toc0_content, 0, 10 * 1024 * 1024);
	//toc0_head指针
	toc0_head = (sbrom_toc0_head_info_t *)toc0_content;
	//item_head指针
	item_head = (sbrom_toc0_item_info_t *)(toc0_content + sizeof(sbrom_toc0_head_info_t));
	//填充main info
	strcpy(toc0_head->name, TOC0_MAGIC);
	toc0_head->magic    = TOC_MAIN_INFO_MAGIC;
	toc0_head->end      = TOC_MAIN_INFO_END;
	toc0_head->items_nr = 2;
	//填充itme info
	offset = ((sizeof(sbrom_toc0_head_info_t) + 2 * sizeof(sbrom_toc0_item_info_t) + 31) & (~31)) + 1024;
	printf("offset=%d\n", offset);
	//遍历
	memset(toc0_content_name, 0, MAX_PATH);
	sprintf(toc0_content_name, "%s.crtpt", p_toc0->cert);

	//读取第一个文件，为证书文件
	printf("toc0_content_name=%s\n", toc0_content_name);
	src_file = fopen(toc0_content_name, "rb");
	if(src_file == NULL)
	{
		printf("file %s cant be open\n", toc0_content_name);

		goto __createtoc0_err;
	}
	fseek(src_file, 0, SEEK_END);
	file_len = ftell(src_file);
	fseek(src_file, 0, SEEK_SET);
	fread(toc0_content + offset, file_len, 1, src_file);
	fclose(src_file);
	src_file = NULL;

	p_item_head = item_head;
	p_item_head->name        = ITEM_NAME_SBROMSW_CERTIF;
	p_item_head->data_offset = offset;
	p_item_head->data_len    = file_len;
	p_item_head->type        = 1;
	p_item_head->end         = TOC_ITEM_INFO_END;

	offset = (offset + file_len + 15) & (~15);
	printf("offset=%d\n", offset);

	memset(toc0_content_name, 0, MAX_PATH);
	strcpy(toc0_content_name, p_toc0->bin);

	p_item_head++;
	//读取第二个文件，为sboot文件
	printf("toc0_content_name=%s\n", toc0_content_name);
	src_file = fopen(toc0_content_name, "rb");
	if(src_file == NULL)
	{
		printf("file %s cant be open\n", toc0_content_name);

		goto __createtoc0_err;
	}
	fseek(src_file, 0, SEEK_END);
	file_len = ftell(src_file);
	fseek(src_file, 0, SEEK_SET);
	fread(toc0_content + offset, file_len, 1, src_file);
	fclose(src_file);
	src_file = NULL;

	struct standard_Boot_file_head  *file_head = (struct standard_Boot_file_head *)(toc0_content + offset);

	p_item_head->name        = ITEM_NAME_SBROMSW_FW;
	p_item_head->data_offset = offset;
	p_item_head->data_len    = file_len;
	p_item_head->end         = TOC_ITEM_INFO_END;
	p_item_head->type        = 2;
	p_item_head->run_addr    = file_head->run_addr;
	//文件读取完毕
	offset = (offset + file_len + 15) & (~15);
	printf("offset=%d\n", offset);
	offset_align = (offset + 16 * 1024 - 1) & (~(16*1024-1));
	toc0_head->valid_len = offset_align;
	toc0_head->add_sum = STAMP_VALUE;
	toc0_head->add_sum = gen_general_checksum(toc0_content, offset_align);

	printf("offset_align=%d\n", offset_align);
	//创建toc1
	memset(toc0_full_name, 0, MAX_PATH);
	GetFullPath(toc0_full_name, toc0_name);
	printf("toc0_full_name=%s\n", toc0_full_name);
	p_file = fopen(toc0_full_name, "wb");
	if(p_file == NULL)
	{
		printf("createtoc0 err: cant create toc0\n");

		goto __createtoc0_err;
	}
	fwrite(toc0_content, offset_align, 1, p_file);
	fclose(p_file);
	p_file = NULL;

	ret = 0;

__createtoc0_err:
	if(p_file)
		fclose(p_file);
	if(src_file)
		fclose(src_file);
	if(toc0_content)
		free(toc0_content);

	return ret;
}
