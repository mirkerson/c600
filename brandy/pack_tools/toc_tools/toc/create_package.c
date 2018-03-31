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

int create_package(toc_descriptor_t *package, char *package_name)
{
	char package_full_name[MAX_PATH];
	char package_content_name[MAX_PATH];
	FILE *p_file, *src_file;
	uint offset, offset_align;
	uint file_len;
	int  ret = -1;
	int  content_count = 0, i;
	sbrom_toc1_head_info_t  *package_head;
	sbrom_toc1_item_info_t  *item_head, *p_item_head;
	char *package_content;
	toc_descriptor_t *p_package = package;

	for(i=0;i<PACKAGE_CONFIG_MAX;i++)
	{
		if(!package[i].item[0])
		{
			break;
		}
	}
	content_count = i;

	printf("content_count=%d\n", content_count);
	//申请空间用于保存文件内容
	package_content = (char *)malloc(10 * 1024 * 1024);
	if(package_content == NULL)
	{
		printf("create_package err: cant malloc memory to store file content\n");

		goto __createpackage_err;
	}
	memset(package_content, 0, 10 * 1024 * 1024);
	//package_head指针
	package_head = (sbrom_toc1_head_info_t *)package_content;
	//item_head指针
	item_head = (sbrom_toc1_item_info_t *)(package_content + sizeof(sbrom_toc1_head_info_t));

	//填充main info
	strcpy(package_head->name, "sunxi-package");
	package_head->magic    = TOC_MAIN_INFO_MAGIC;
	package_head->end      = TOC_MAIN_INFO_END;
	package_head->items_nr = content_count;
	//填充itme info
	offset = (sizeof(sbrom_toc1_head_info_t) + content_count * sizeof(sbrom_toc1_item_info_t) + 1023) & (~1023);
	//遍历
	for(p_item_head=item_head;content_count > 0;p_item_head++, p_package++, content_count--)
	{
		memset(package_content_name, 0, MAX_PATH);
		sprintf(package_content_name, "%s", p_package->bin);

		//printf("package_content_name=%s\n", package_content_name);

		src_file = fopen(package_content_name, "rb");
		if(src_file == NULL)
		{
			printf("file %s cant be open\n", package_content_name);

			goto __createpackage_err;
		}
		fseek(src_file, 0, SEEK_END);
		file_len = ftell(src_file);
		fseek(src_file, 0, SEEK_SET);
		fread(package_content + offset, file_len, 1, src_file);
		fclose(src_file);
		src_file = NULL;

		p_item_head->data_offset = offset;
		p_item_head->data_len    = file_len;
		p_item_head->type        = ITEM_TYPE_BINFILE;
		p_item_head->end         = TOC_ITEM_INFO_END;
		//p_item_head->run_addr    = file_head->run_addr;

		strcpy(p_item_head->name, p_package->item);

		offset = (offset + file_len + 1023) & (~1023);
	}

	offset_align = (offset + 16 * 1024 - 1) & (~(16*1024-1));
	package_head->valid_len = offset_align;
	package_head->add_sum = STAMP_VALUE;
	package_head->add_sum = gen_general_checksum(package_content, offset_align);

	//printf("offset_align=%d\n", offset_align);
	//创建package
	memset(package_full_name, 0, MAX_PATH);
	GetFullPath(package_full_name, package_name);
	//printf("package_full_name=%s\n", package_full_name);
	p_file = fopen(package_full_name, "wb");
	if(p_file == NULL)
	{
		printf("createpackage err: cant create package\n");

		goto __createpackage_err;
	}
	fwrite(package_content, offset_align, 1, p_file);
	fclose(p_file);
	p_file = NULL;

	ret = 0;

__createpackage_err:
	if(p_file)
		fclose(p_file);
	if(src_file)
		fclose(src_file);
	if(package_content)
		free(package_content);

	return ret;
}
