// update.cpp : Defines the entry point for the console application.
//
#include <malloc.h>
#include <getopt.h>
#include "types.h"
#include <string.h>
#include "script.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_IMAGE_SIZE (16<<20)
#define MAX_PATH  260
#define debug(fmt,args...)	//printf(fmt ,##args)

struct full_image_cfg 
{
	char* filename;
	int   offset; 
};

int IsFullName(const char *FilePath);
int load_file_from_partition_config(void* total_buffer, void *partition_cfg);

int get_file_size(const char* file_name)
{
	FILE  *file = NULL;
	int file_len;
	file = fopen(file_name, "rb");
	if(!file)
	{
		printf("unable to open  file %s\n", file_name);
		return -1;
	}
	fseek(file, 0, SEEK_END);
	file_len = ftell(file);
	fclose(file);
	return file_len;
}

int load_file_to_buffer(const char* file_name, void** file_buffer, int  *psize)
{
	FILE  *file = NULL;
	int file_len;
	unsigned char* buffer;
	
	file = fopen(file_name, "rb");
	if(!file)
	{
		printf("unable to open  file %s\n", file_name);
		return -1;
	}
	fseek(file, 0, SEEK_END);
	file_len = ftell(file);
	fseek(file, 0, SEEK_SET);
	if(file_buffer)
	{
		buffer = (char *)malloc(file_len+1);
		if(buffer == NULL)
		{
			printf("unable to malloc memory\n");
			goto _err_out;
		}
		fread(buffer, 1, file_len, file);
		*file_buffer = buffer;
	}
	fclose(file);
	if(psize)
	{
		*psize = file_len;
	}
	printf("load file: %s ok\n", file_name);
	return 0;
_err_out:
	fclose(file);
	return -1;
}

int save_buffer_to_file(const char* file_name, void* file_buffer, size_t size)
{
	FILE  *file = NULL;
	file = fopen(file_name, "wb");
	if(!file)
	{
		printf("unable to open  file %s\n", file_name);
		return -1;
	}

	if(file_buffer == NULL || size == 0)
	{
		printf("unable to write file, buffer is null or size is 0\n");
		return -1;
	}
	fwrite(file_buffer, 1, size, file);
	fclose(file);
	return 0;
}

void free_file_buffer(void* file_buffer)
{
	if(file_buffer)
		free(file_buffer);
}


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


void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("merge_package.exe --out outfile\n");
	printf("                  --boot0 boot0.fex\n");
	printf("                  --boot1 boot1.fex\n");
	printf("                  --mbr mbr.fex\n");
	printf("                  --partition partition.fex\n");
	printf("                  --logic_start [512|256]\n");
	printf("                  --help\n");
}

struct option long_options[]={
	{"out  "    ,1,NULL,'o'},
	{"boot0"    ,1,NULL,'b'},
	{"boot1"    ,1,NULL,'u'},
	{"mbr"      ,1,NULL,'m'},
	{"partition",1,NULL,'p'},
	{"logic_start",1,NULL,'l'},
	{"help"     ,0,NULL,'h'},
	{NULL       ,0,NULL,  0},
};

char* const short_options = "o:b:u:m:p:l:h";



// merge_package full_img.fex  boot0_spinor.fex  u-boot-spinor.fex sunxi_mbr.fex sys_partition.bin
int main(int argc, char* argv[])
{
	int opt;
	int ret;
	int i;
	char boot0_file[256]={0};
	char boot1_file[256]={0};
	char mbr_file[256] ={0};
	char partition_file[256]={0};
	char output_file[256]={0};
	unsigned char* file_buffer = NULL;
	unsigned char* out_buffer = NULL;
	int offset = 0;
	int file_size = 0;
	int total_part_file_size = 0;
	int logic_start = 512<<20;
	
	struct full_image_cfg image_cfg[3];

	//while(-1 != (opt = getopt(argc, argv,"b:u:m:p:h")))
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch(opt)
		{
			case 'o':
			strcpy(output_file,optarg);
			case 'b':
			strcpy(boot0_file,optarg);
			break;
			case 'u':
			strcpy(boot1_file,optarg);
			break;
			case 'm':
			strcpy(mbr_file,optarg);
			break;
			case 'l':
			logic_start=atoi(optarg)*1024;
			break;
			case 'p':
			strcpy(partition_file,optarg);
			break;
			default:
			Usage();
			return 0;
		}
	}
	debug("boot0: %s\n", boot0_file);
	debug("boot1: %s\n", boot1_file);
	debug("mbr: %s\n", mbr_file);
	debug("partition: %s\n", partition_file);
	debug("outfile: %s\n", output_file);
	debug("logic_start: %dK\n", logic_start/1024);
	
	if( 0 == strlen(boot0_file) ||
		0 == strlen(boot1_file) ||
		0 == strlen(mbr_file) ||
		0 == strlen(partition_file) ||
		0 == strlen(output_file) )
	{
		printf("para error\n");
		Usage();
		return -1;
	}
	//size 0--->512K 
	image_cfg[0].filename=boot0_file;//size 24K
	image_cfg[0].offset=0;
	image_cfg[1].filename=boot1_file;
	image_cfg[1].offset=24<<10;
	image_cfg[2].filename=mbr_file; //size 16K
	image_cfg[2].offset=logic_start;   
	//---end, next offset is logic_start + 16K
	
	
	//load boot file
	out_buffer = (char *)malloc(MAX_IMAGE_SIZE);
	memset(out_buffer,0xff,MAX_IMAGE_SIZE) ;
	if(out_buffer == NULL)
	{
		printf("unable to malloc memory\n");
		goto __err_out;
	}
	for(i = 0; i < 3; i++)
	{
		ret = load_file_to_buffer(image_cfg[i].filename,(void**)&file_buffer,&file_size);
		if(ret != 0 )
		{
			goto __err_out;
		}
		memcpy(out_buffer+image_cfg[i].offset, file_buffer, file_size);		
		if(file_buffer)
		{
			free_file_buffer(file_buffer);
			file_buffer = NULL;
		}
	}
	
	//load partition file
	offset = logic_start+(16<<10);  //offset is logic_start + mbr_size
	ret = load_file_to_buffer(partition_file,(void**)&file_buffer,NULL);
	if(ret != 0 )
	{
		goto __err_out;
	}
	total_part_file_size = load_file_from_partition_config(out_buffer+offset,file_buffer);
	if(file_buffer)
	{
		free_file_buffer(file_buffer);
		file_buffer = NULL;
	}
	
	if(total_part_file_size <= 0)
	{
		goto __err_out;
	}
	
	int total_file_len =offset+ total_part_file_size;
	total_file_len = ((total_file_len+511)/512)*512;
	if(0 == save_buffer_to_file(output_file,out_buffer,total_file_len))
	{
		printf("merge_package ok\n");
		return 0;
	}
	else
	{
		goto __err_out;
	}
	if(out_buffer) free(out_buffer);
	return 0;

__err_out:
	if(out_buffer) free(out_buffer);
	printf("merge_package fail\n");
	return -1;
}

int load_file_from_partition_config(void* out_buffer, void *partition_cfg)
{
	int  value[8];
	char filename[32];
	char partname[32];
	char fullname[260];
	int  part_handle;
	int  ret;
	int  part_size = 0;
	int  offset = 0; 
	unsigned char* partfile_buffer = NULL;
	int file_size =0;

	script_parser_init(partition_cfg);
	while(1)
	{
		file_size = 0;
		part_size = 0;
		part_handle = script_parser_fetch_partition();

		//check the last part
		if(part_handle <= 0)
		{
			break;
		}
		if(!script_parser_fetch_mainkey_sub("name", part_handle, value))
		{
			debug("part name=%s\n", (char *)value);
			memset(fullname, 0, sizeof(fullname));
			memset(filename, 0, sizeof(filename));
			ret = script_parser_fetch_mainkey_sub("downloadfile", part_handle, (int *)fullname);
			if(!ret)
			{
				strcpy(filename, (char*)fullname);
				debug("file name:%s\n", filename);
			}
			ret = script_parser_fetch_mainkey_sub("size", part_handle, (int *)value);
			if(!ret)
			{
				part_size = value[0];
				debug("part size:%d sector\n",part_size);
				part_size *= 512;
			}
			if(strlen(filename))
			{
				ret = load_file_to_buffer(filename, (void**)(&partfile_buffer), &file_size);
				if(ret==0) 
				{
					if(file_size>part_size)
					{
						printf("error: file size of %s(%d) > part size(%d)\n", filename, file_size, part_size);
						return -1;
					}
					memcpy(out_buffer+offset, partfile_buffer, file_size);
				}
				if(partfile_buffer)
				{
					free_file_buffer(partfile_buffer);
					partfile_buffer = NULL;
				}
			}
			offset += part_size;
		}
		else
		{
			printf("parser partition.fex fail\n");

			return -1;
		}
	}
	return offset;
}




