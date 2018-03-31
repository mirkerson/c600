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
#include "sparse_format.h"
#include "superblock.h"
#include "imgheader.h"
#include "sunxi_mbr.h"
#include "crc.h"

#define  SUNXI_SIGN_READ_MAX   (1024 * 1024)

static int sunxi_rewrite_mbr(char *part_name, unsigned int sig_method);
static int signature_file(char *file_path, char *private_path, char *part_name);
static int sunxi_create_sign_file(char *source_file, char *buffer, int buffer_len);
//static int sunxi_test_sign(char *publickey_file, char *sign_buffer, int sign_len);
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
void __Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("sign filename privatekey.pem [partname]\n");
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
*    note          :  argv1  : the file which need to be verified
*
*                     argv2  : the private key
*
*                     [argv3]: the partname which the file will be download. It is optional.
*
************************************************************************************************************
*/
int main(int argc, char* argv[])
{
	char   sign_file_path[MAX_PATH];
	char   privatekey_file_path[MAX_PATH];
	char   *partname = NULL;

	if( (argc != 3) && (argc != 4) )
	{
		printf("Invalid parameters\n");
		__Usage();

		return -1;
	}
	if(argc == 4)
	{
	    partname = argv[3];
	}
	//get the file name which need to be signed
	memset(sign_file_path, 0, MAX_PATH);
	sunxi_GetFullPath(sign_file_path, argv[1]);
	//get the privatekey file name which is pem format
	memset(privatekey_file_path, 0, MAX_PATH);
	sunxi_GetFullPath(privatekey_file_path, argv[2]);

	return signature_file(sign_file_path, privatekey_file_path, partname);
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
static int sign_for_bootimg_format(char *data_buffer, char *private_key, char *encrypted_data)
{
	int  ret;
	SHA256_CTX	ctx;
	unsigned char sha256[256];

	memset(sha256, 0, 256);
	//计算sha256
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (u_int8_t *)data_buffer, sizeof(struct fastboot_boot_img_hdr));
    //sunxi_dump(data_buffer, sizeof(struct fastboot_boot_img_hdr));
	sunxi_dbg("\n");

    SHA256_Update(&ctx, (u_int8_t *)(data_buffer + CFG_FASTBOOT_MKBOOTIMAGE_SECTOR*512), sizeof(struct image_header) + 512);
    sunxi_dbg("offset=0x%x\n", CFG_FASTBOOT_MKBOOTIMAGE_SECTOR*512);
    //sunxi_dump(data_buffer + CFG_FASTBOOT_MKBOOTIMAGE_SECTOR*512, sizeof(struct image_header) + 512);
	sunxi_dbg("\n");

    SHA256_Final(sha256, &ctx);

	//sunxi_dump(sha256, 256);
	//私钥加密rsa
	ret = sunxi_rsa_privatekey_encrypt(sha256, encrypted_data, SHA256_DIGEST_LENGTH, private_key);
	if(ret < 0)
	{
		printf("sign err: when private encrypt bootimg\n");
	}

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
static int sign_for_sparse_format(char *data_buffer, char *private_key, char *encrypted_data)
{
	sparse_header_t *header = (sparse_header_t*)data_buffer;
	char  *tmp_buffer = data_buffer;
	unsigned char sha256[256];
	SHA256_CTX	ctx;
	unsigned int data_len;
	int  ret;

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
				printf("sign err: it is not a sparse format\n");

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
						printf("sign err: it is not a sparse format\n");

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
				printf("sign err: bad sparse format, two dont care chunk\n");

				return -1;
			}
			sblock = (struct ext4_super_block *)tmp_buffer;
			sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
			sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
			memset(sblock->s_last_mounted, 0, 64);

			memset(sha256, 0, 256);
			//计算sha256
			data_len = (unsigned  int)&(((struct ext4_super_block *)0)->s_snapshot_list);
		    SHA256_Init(&ctx);
		    SHA256_Update(&ctx, (u_int8_t *)tmp_buffer, data_len);
		    SHA256_Final(sha256, &ctx);
		    //sunxi_dump(sha256, 256);
			//私钥加密rsa
			ret = sunxi_rsa_privatekey_encrypt(sha256, encrypted_data, SHA256_DIGEST_LENGTH, private_key);
			if(ret < 0)
			{
				printf("sign err: when private encrypt android sparse format\n");

				return -1;
			}

			return 0;
		}
	}

	return -1;
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
static int signature_file(char *file_path, char *private_path, char *part_name)
{
	char  *buffer, *tmp_buffer;
	char   *data_buffer;
	char   encrypted_data[256];
	FILE  *file_hd;
	int    ret;
	unsigned int sig_method;

	data_buffer = malloc(SUNXI_SIGN_READ_MAX);
	memset(data_buffer, 0, SUNXI_SIGN_READ_MAX);

	file_hd = fopen(file_path, "r");
	if(file_hd == NULL)
	{
		printf("sign err: the file %s cant be open\n", file_path);

		return -1;
	}
	fread(data_buffer, SUNXI_SIGN_READ_MAX, 1, file_hd);
	fclose(file_hd);

	sunxi_dbg("ready to sign to %s\n", file_path);

	memset(encrypted_data, 0, 256);
	if(!strncmp(data_buffer, FASTBOOT_BOOT_MAGIC, strlen(FASTBOOT_BOOT_MAGIC)))		//bootimage
	{
		sunxi_dbg("sign to bootimg format\n");
		ret = sign_for_bootimg_format(data_buffer, private_path, encrypted_data);
		sig_method = 0x8000;
	}
	else  //compare android sparse format
	{
		sunxi_dbg("sign to sparse format\n");
		ret = sign_for_sparse_format(data_buffer, private_path, encrypted_data);
		sig_method = 0x8001;
	}

	sunxi_dbg("sign result %d\n", ret);

	if(ret < 0)
	{
		printf("sunxi signed error\n");
		free(data_buffer);

		return -1;
	}

	//sunxi_dump(encrypted_data, 256);

	ret = sunxi_create_sign_file(file_path, encrypted_data, 256);
	if(ret < 0)
	{
		printf("sunxi err: fail to create sign file\n");
		free(data_buffer);

		return -1;
	}

	//for test
	//sunxi_test_sign("b.key", encrypted_data, 256);

	return sunxi_rewrite_mbr(part_name, sig_method);
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
static int sunxi_rewrite_mbr(char *part_name, unsigned int sig_method)
{
	char mbr_full_path[MAX_PATH];
	const char *mbr_name = "sunxi_mbr.fex";
	FILE *mbr_file;
	char *buffer, *tmp_buffer;
	int  i,k, ret;
	sunxi_mbr_t *mbr, *tmp_mbr;
	unsigned int crc_cal, crc_origin;

	if(part_name == NULL)				//if part name is not exist, the ignore this step
	{
		printf("part name null, ignore it\n");

		return 0;
	}

	buffer = (char *)malloc(SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM);
	if(!buffer)
	{
		printf("sign err : not enough memory\n");

		return -1;
	}

	memset(mbr_full_path, 0, MAX_PATH);
	sunxi_GetFullPath(mbr_full_path, mbr_name);

	mbr_file = fopen(mbr_full_path, "rb+");	//open mbr file
	if(mbr_file == NULL)
	{
		printf("sign failed: mbr file cant be found\n");

		goto __sunxi_rewrite_mbr_err;
	}

	mbr = (sunxi_mbr_t *)buffer;
	tmp_buffer = buffer;

	fread(buffer, SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM, 1, mbr_file);
	fseek(mbr_file, 0, SEEK_SET);
	//verify the mbr file
	for(i=0;i<SUNXI_MBR_COPY_NUM;i++)
	{
		crc_cal    = calc_crc32(tmp_buffer + 4, SUNXI_MBR_SIZE - 4);
		crc_origin = *((unsigned int *)tmp_buffer);

		if(crc_cal != crc_origin)
		{
			printf("signature failed: mbr copy %d is bad\n", i);

			goto __sunxi_rewrite_mbr_err;
		}
		tmp_buffer += SUNXI_MBR_SIZE;
	}
	//compare which part is the same with the part name
	for(i=0;i<mbr->PartCount;i++)
	{
		if(!strcmp(part_name, mbr->array[i].name))
		{
			sunxi_dbg("find the %d part %s\n", i, part_name);
			sunxi_dbg("sig_method=0x%x\n", sig_method);
			tmp_mbr = (sunxi_mbr_t *)buffer;
			for(k=0;k<SUNXI_MBR_COPY_NUM;k++)
			{
				tmp_mbr->array[i].sig_verify = sig_method;

				tmp_mbr ++;
			}

			break;
		}
	}

	if(i == mbr->PartCount)
	{
		printf("sunxi sign err: part name %s cant be found\n", part_name);

		goto __sunxi_rewrite_mbr_err;
	}

	tmp_buffer = buffer;
	for(k=0;k<SUNXI_MBR_COPY_NUM;k++)
	{
		tmp_mbr = (sunxi_mbr_t *)tmp_buffer;
		tmp_mbr->lockflag = SUNXI_LOCKING;
		crc_cal = calc_crc32(tmp_buffer + 4, SUNXI_MBR_SIZE - 4);
		*(unsigned int *)tmp_buffer = crc_cal;
		tmp_buffer += SUNXI_MBR_SIZE;

		sunxi_dbg("0x%x\n", crc_cal);
	}
	fseek(mbr_file, 0, SEEK_SET);
	fwrite(buffer, SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM, 1, mbr_file);

	sunxi_dbg("rewrite sunxi mbr ok\n");

	ret = 0;

__sunxi_rewrite_mbr_err:
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

static int sunxi_create_sign_file(char *source_file, char *buffer, int buffer_len)
{
	FILE *file_hd = NULL;
	char  sign_file[MAX_PATH];
	int   index, index_p;

	memset(sign_file, 0, MAX_PATH);
	strcpy(sign_file, source_file);

	index = strlen(sign_file) - 1;

	while(sign_file[index] != '.')
	{
		sign_file[index--] = '\0';
	}
	index ++;

	sign_file[index++] = 's';
	sign_file[index++] = 'i';
	sign_file[index++] = 'g';

	file_hd = fopen(sign_file, "wb");
	if(file_hd == NULL)
	{
		printf("sunxi err: unable to create file %s to store the sign\n", sign_file);

		return -1;
	}
	fwrite(buffer, buffer_len, 1, file_hd);
	fclose(file_hd);

	return 0;
}

//static int sunxi_test_sign(char *publickey_file, char *sign_buffer, int sign_len)
//{
//	char  decryped_data[256];
//	char  pubkey_full_path[MAX_PATH];
//
//	printf("%s %d\n", __FILE__, __LINE__);
//	memset(decryped_data, 0, 256);
//	memset(pubkey_full_path, 0, MAX_PATH);
//	sunxi_GetFullPath(pubkey_full_path, publickey_file);
//	printf("%s %d\n", __FILE__, __LINE__);
//
//	sunxi_rsa_publickey_decrypt(sign_buffer, decryped_data, sign_len, pubkey_full_path);
//	printf("%s %d\n", __FILE__, __LINE__);
//	sunxi_dump(decryped_data, 32);
//	printf("%s %d\n", __FILE__, __LINE__);
//
//	return 0;
//}
