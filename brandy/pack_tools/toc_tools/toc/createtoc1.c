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

int createtoc1(toc_descriptor_t *toc1, char *toc1_name)
{
	char toc1_full_name[MAX_PATH];
	char toc1_content_name[MAX_PATH];
	FILE *p_file, *src_file;
	uint offset, offset_align;
	uint file_len;
	int  ret = -1;
	int  item_count = 0, content_count = 0, i;
	sbrom_toc1_head_info_t  *toc1_head;
	sbrom_toc1_item_info_t  *item_head, *p_item_head;
	char *toc1_content;
	toc_descriptor_t *p_toc1 = toc1;

	for(i=0;i<TOC1_CONFIG_MAX;i++)
	{
		if(!toc1[i].item[0])
		{
			break;
		}
		if(toc1[i].type == NORMAL_TYPE)
		{
			content_count ++;
		}
		content_count ++;
		item_count ++;
	}

	printf("item_count=%d\n", item_count);
	printf("content_count=%d\n", content_count);
	//申请空间用于保存文件内容
	toc1_content = (char *)malloc(10 * 1024 * 1024);
	if(toc1_content == NULL)
	{
		printf("createtoc1 err: cant malloc memory to store file content\n");

		goto __createtoc1_err;
	}
	memset(toc1_content, 0, 10 * 1024 * 1024);
	//toc1_head指针
	toc1_head = (sbrom_toc1_head_info_t *)toc1_content;
	//item_head指针
	item_head = (sbrom_toc1_item_info_t *)(toc1_content + sizeof(sbrom_toc1_head_info_t));
	//填充main info
	strcpy(toc1_head->name, "sunxi-secure");
	toc1_head->magic    = TOC_MAIN_INFO_MAGIC;
	toc1_head->end      = TOC_MAIN_INFO_END;
	toc1_head->items_nr = content_count;
	//填充itme info
	offset = (sizeof(sbrom_toc1_head_info_t) + content_count * sizeof(sbrom_toc1_item_info_t) + 1023) & (~1023);
	//遍历
	for(p_item_head=item_head;item_count > 0;p_item_head++, p_toc1++, item_count--)
	{
		memset(toc1_content_name, 0, MAX_PATH);
		sprintf(toc1_content_name, "%s.der", p_toc1->cert);

		//printf("toc1_content_name=%s\n", toc1_content_name);

		src_file = fopen(toc1_content_name, "rb");
		if(src_file == NULL)
		{
			printf("file %s cant be open\n", toc1_content_name);

			goto __createtoc1_err;
		}
		fseek(src_file, 0, SEEK_END);
		file_len = ftell(src_file);
		fseek(src_file, 0, SEEK_SET);
		fread(toc1_content + offset, file_len, 1, src_file);
		fclose(src_file);
		src_file = NULL;

		p_item_head->data_offset = offset;
		p_item_head->data_len    = file_len;
		if(p_item_head==item_head)
		{
			p_item_head->type        = ITEM_TYPE_ROOTKEY;
		}
		else
		{
			p_item_head->type        = ITEM_TYPE_BINKEY;
		}
		p_item_head->end         = TOC_ITEM_INFO_END;

		strcpy(p_item_head->name, p_toc1->item);

		offset = (offset + file_len + 1023) & (~1023);

		if(p_toc1->type == NORMAL_TYPE)
		{
			memset(toc1_content_name, 0, MAX_PATH);
			strcpy(toc1_content_name, p_toc1->bin);

			//printf("toc1_content_name=%s\n", toc1_content_name);

			src_file = fopen(toc1_content_name, "rb");
			if(src_file == NULL)
			{
				printf("file %s cant be open\n", toc1_content_name);

				goto __createtoc1_err;
			}
			fseek(src_file, 0, SEEK_END);
			file_len = ftell(src_file);
			fseek(src_file, 0, SEEK_SET);
			fread(toc1_content + offset, file_len, 1, src_file);
			fclose(src_file);
			src_file = NULL;

			p_item_head++;

			struct spare_boot_ctrl_head  *file_head = (struct spare_boot_ctrl_head *)(toc1_content + offset);

			p_item_head->data_offset = offset;
			p_item_head->data_len    = file_len;
			p_item_head->end         = TOC_ITEM_INFO_END;
			p_item_head->type        = ITEM_TYPE_BINFILE;
			p_item_head->run_addr    = file_head->run_addr;

			strcpy(p_item_head->name, p_toc1->item);

			offset = (offset + file_len + 1023) & (~1023);
		}
	}

	offset_align = (offset + 16 * 1024 - 1) & (~(16*1024-1));
	toc1_head->valid_len = offset_align;
	toc1_head->add_sum = STAMP_VALUE;
	toc1_head->add_sum = gen_general_checksum(toc1_content, offset_align);

	//printf("offset_align=%d\n", offset_align);
	//创建toc1
	memset(toc1_full_name, 0, MAX_PATH);
	GetFullPath(toc1_full_name, toc1_name);
	//printf("toc1_full_name=%s\n", toc1_full_name);
	p_file = fopen(toc1_full_name, "wb");
	if(p_file == NULL)
	{
		printf("createtoc1 err: cant create toc1\n");

		goto __createtoc1_err;
	}
	fwrite(toc1_content, offset_align, 1, p_file);
	fclose(p_file);
	p_file = NULL;

	ret = 0;

__createtoc1_err:
	if(p_file)
		fclose(p_file);
	if(src_file)
		fclose(src_file);
	if(toc1_content)
		free(toc1_content);

	return ret;
}
