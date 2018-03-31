// update_uboot.cpp : Defines the entry point for the console application.
//

#include "signature.h"
#include "superblock.h"
#include "imgheader.h"
#include "crc.h"
#include "sparse_format.h"
#include "sunxi_mbr.h"

#define  MAX_PATH             (260)

int signature_all_parts(char *mbr_path, char *dl_path);
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
    if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, MAX_PATH ) == NULL)
   {
        perror( "getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("signature sunxi_mbr.fex dlinfo.fex\n\n");
}


int main(int argc, char* argv[])
{
	char   mbr_file_path[MAX_PATH];
	char   dlinfo_file_path[MAX_PATH];
	int    ret = -1;

	if(argc == 3)
	{
		if((argv[1] == NULL) || (argv[2] == NULL))
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
		memset(mbr_file_path,    0, MAX_PATH);
		memset(dlinfo_file_path, 0, MAX_PATH);

		GetFullPath(mbr_file_path,    argv[1]);
		GetFullPath(dlinfo_file_path, argv[2]);

		if((!strcmp(argv[1], "sunxi_mbr.fex")) && (!strcmp(argv[2], "dlinfo.fex")) )
		{
			ret = signature_all_parts(mbr_file_path, dlinfo_file_path);
		}
		else if((!strcmp(argv[2], "sunxi_mbr.fex")) && (!strcmp(argv[1], "dlinfo.fex")) )
		{
			ret = signature_all_parts(mbr_file_path, dlinfo_file_path);
		}
		else
		{
			Usage();

			printf("Invalid file name to signature\n");
		}
	}
	else
	{
		Usage();

		printf("Invalid parameters\n");
	}

	return ret;
}


static int signature_compute(char *data_file_name, unsigned int *sig_value, unsigned int *sig_method)
{
	FILE *data_file = NULL;
	char  data_buffer[8 * 1024];
	char  *tmp_buffer = data_buffer;
	unsigned int hash_value;
	unsigned int h_value[4], s_value[4];

	printf("compute signature for datafile %s\n", data_file_name);
	data_file = fopen(data_file_name, "rb");
	if(data_file == NULL)
	{
		printf("signature failed: data file is not exist\n");

		return -1;
	}
	fread(data_buffer, 8 * 1024, 1, data_file);
	fclose(data_file);

	if(!strncmp(data_buffer, FASTBOOT_BOOT_MAGIC, strlen(FASTBOOT_BOOT_MAGIC)))
	{
		//android格式签名
		HashString_init();
		hash_value = HashString(data_buffer, 1, sizeof(struct fastboot_boot_img_hdr));	//1类hash
		hash_value = HashString(data_buffer + CFG_FASTBOOT_MKBOOTIMAGE_SECTOR*512, 1, sizeof(struct image_header));	//1类hash

		rsa_init();
		h_value[0] = (hash_value>>0) & 0xff;
		h_value[1] = (hash_value>>8) & 0xff;
		h_value[2] = (hash_value>>16) & 0xff;
		h_value[3] = (hash_value>>24) & 0xff;

		rsa_encrypt( h_value, 4, s_value);

		sig_value[0] = s_value[0];
		sig_value[1] = s_value[1];
		sig_value[2] = s_value[2];
		sig_value[3] = s_value[3];

		*sig_method = 0x8000;
	}
	else
	{
		sparse_header_t *header = (sparse_header_t*)data_buffer;

		if (header->magic == SPARSE_HEADER_MAGIC)
		{
			if ((header->major_version == SPARSE_HEADER_MAJOR_VER) &&
	    		(header->file_hdr_sz   == sizeof(sparse_header_t)) &&
	    		(header->chunk_hdr_sz  == sizeof(chunk_header_t)))
			{
				struct ext4_super_block  *sblock;
				chunk_header_t *chunk;

				tmp_buffer += sizeof(sparse_header_t);
				chunk = (chunk_header_t *)tmp_buffer;
				if((chunk->chunk_type == CHUNK_TYPE_DONT_CARE) || (chunk->chunk_type == CHUNK_TYPE_FILL))
				{
					tmp_buffer += sizeof(chunk_header_t);
					chunk = (chunk_header_t *)tmp_buffer;
				}
				else if(chunk->chunk_type != CHUNK_TYPE_RAW)
				{
					printf("bad sparse format\n");

					return -1;
				}
				if(chunk->chunk_type == CHUNK_TYPE_RAW)
				{
					if( (chunk->total_sz - sizeof(chunk_header_t) ) <= 1024)
					{
						tmp_buffer += chunk->total_sz;
						chunk = (chunk_header_t *)tmp_buffer;

						if(chunk->chunk_type != CHUNK_TYPE_RAW)
						{
							printf("bad sparse format\n");

							return -1;
						}
						tmp_buffer += sizeof(chunk_header_t);
					}
					else
					{
						tmp_buffer += sizeof(chunk_header_t) + 1024;
					}
				}
				else
				{
					printf("bad sparse format, two dont care chunk\n");

					return -1;
				}

				sblock = (struct ext4_super_block *)tmp_buffer;
				sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
				sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
				memset(sblock->s_last_mounted, 0, 64);

				HashString_init();
				hash_value = HashString((unsigned char *)tmp_buffer, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash
				rsa_init();
				h_value[0] = (hash_value>>0) & 0xff;
				h_value[1] = (hash_value>>8) & 0xff;
				h_value[2] = (hash_value>>16) & 0xff;
				h_value[3] = (hash_value>>24) & 0xff;

				rsa_encrypt( h_value, 4, s_value);

				sig_value[0] = s_value[0];
				sig_value[1] = s_value[1];
				sig_value[2] = s_value[2];
				sig_value[3] = s_value[3];

				*sig_method = 0x8001;
			}
		}
		else
		{
			struct ext4_super_block  *sblock;

			tmp_buffer += 1024;
			sblock = (struct ext4_super_block *)tmp_buffer;
			sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
			sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
			memset(sblock->s_last_mounted, 0, 64);

			HashString_init();
			hash_value = HashString((unsigned char *)tmp_buffer, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash

			rsa_init();
			h_value[0] = (hash_value>>0) & 0xff;
			h_value[1] = (hash_value>>8) & 0xff;
			h_value[2] = (hash_value>>16) & 0xff;
			h_value[3] = (hash_value>>24) & 0xff;

			rsa_encrypt( h_value, 4, s_value);

			sig_value[0] = s_value[0];
			sig_value[1] = s_value[1];
			sig_value[2] = s_value[2];
			sig_value[3] = s_value[3];

			*sig_method = 0x8001;
		}
	}

	return 0;
}

static void __trans_cfg_to_str(char *src, char *dst)
{
	char ch;
	int  i;

	for(i=0;i<16;i++)
	{
		ch = src[i];

		if((ch >= 'A') && (ch <= 'Z'))
		{
			dst[i] = ch + 'a' - 'A';
		}
		else if(ch == '_')
		{
			dst[i] = '.';
		}
		else if(ch == '0')
		{
			break;
		}
	}
}

int signature_all_parts(char *mbr_path, char *dl_path)
{
	char  *buffer, *tmp_buffer;
	char   dl_buffer[SUNXI_DL_SIZE];
	sunxi_mbr_t          *mbr, *tmp_mbr;
	sunxi_download_info  *dl_info;
	FILE *dl_file = NULL;
	FILE *mbr_file = NULL;
	unsigned int crc_cal, crc_origin;
	char  part_download_file[260];
	int   i,j,k;
	int   ret = -1, ret0;
	unsigned int sig_value[4], sig_method;

	buffer = (char *)malloc(SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM);
	if(!buffer)
	{
		printf("signature failed: not enough memory\n");

		return -1;
	}

	dl_file = fopen(dl_path, "r");
	if(dl_file == NULL)
	{
		printf("signature failed: dlinfo file cant be found\n");

		goto __signature_all_parts_failed;
	}
	fread(dl_buffer, SUNXI_DL_SIZE, 1, dl_file);

	dl_info = (sunxi_download_info *)dl_buffer;
	crc_cal    = calc_crc32(dl_buffer + 4, SUNXI_DL_SIZE - 4);
	crc_origin = *((unsigned int *)dl_buffer);

	if(crc_cal != crc_origin)
	{
		printf("signature failed: dlinfo copy is bad\n");

		goto __signature_all_parts_failed;
	}
	fclose(dl_file);

	mbr_file = fopen(mbr_path, "rb+");
	if(mbr_file == NULL)
	{
		printf("signature failed: mbr file cant be found\n");

		goto __signature_all_parts_failed;
	}

	mbr = (sunxi_mbr_t *)buffer;
	tmp_buffer = buffer;

	fread(buffer, SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM, 1, mbr_file);
	fseek(mbr_file, 0, SEEK_SET);

	for(i=0;i<SUNXI_MBR_COPY_NUM;i++)
	{
		crc_cal    = calc_crc32(tmp_buffer + 4, SUNXI_MBR_SIZE - 4);
		crc_origin = *((unsigned int *)tmp_buffer);

		if(crc_cal != crc_origin)
		{
			printf("signature failed: mbr copy %d is bad\n", i);

			goto __signature_all_parts_failed;
		}
		tmp_buffer += SUNXI_MBR_SIZE;
	}

	for(i=0;i<mbr->PartCount;i++)
	{
		if((mbr->array[i].sig_verify & 0x8000) == 0x8000)
		{
			for(j=0;j<dl_info->download_count;j++)
			{
				if(!strcmp(mbr->array[i].name, dl_info->one_part_info[j].name))
				{
					if(dl_info->one_part_info[j].dl_filename[0] != '\0')
					{
						char  file_name[16];

						memset(part_download_file, 0, 260);
						memset(file_name, 0, 16);
						__trans_cfg_to_str(dl_info->one_part_info[j].dl_filename, file_name);

						GetFullPath(part_download_file, file_name);
						ret0 = signature_compute(part_download_file, sig_value, &sig_method);
						if(ret0 < 0)
						{
							printf("compute signature fail\n");
							goto __signature_all_parts_failed;
						}
						tmp_buffer = buffer;
						tmp_mbr = (sunxi_mbr_t *)tmp_buffer;
						for(k=0;k<SUNXI_MBR_COPY_NUM;k++)
						{
							memcpy(tmp_mbr->array[i].sig_value, sig_value, 4 * 4);
							tmp_mbr->array[i].sig_verify = sig_method;

							tmp_mbr ++;
						}
					}
				}
			}
		}
	}
	tmp_buffer = buffer;
	tmp_mbr = (sunxi_mbr_t *)tmp_buffer;
	for(k=0;k<SUNXI_MBR_COPY_NUM;k++)
	{
		tmp_mbr->lockflag = SUNXI_LOCKING;
		crc_cal = calc_crc32(tmp_buffer + 4, SUNXI_MBR_SIZE - 4);
		*(unsigned int *)tmp_buffer = crc_cal;
		tmp_buffer += SUNXI_MBR_SIZE;
	}
	fseek(mbr_file, 0, SEEK_SET);
	fwrite(buffer, SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM, 1, mbr_file);

	ret = 0;

__signature_all_parts_failed:
	if(buffer)
	{
		free(buffer);
	}
	if(mbr_file)
	{
		fclose(mbr_file);
	}

	return ret;
}
