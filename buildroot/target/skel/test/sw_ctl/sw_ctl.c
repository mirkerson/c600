#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <mach/script_v2.h>
#include <linux/sw_sys.h>

struct sw_script_para para;

unsigned int cmd_array[] = {
	SW_SYS_IOC_GET_TOTAL_MAINKEY,
	SW_SYS_IOC_GET_TOTAL_SUBKEY,
	SW_SYS_IOC_GET_KEY_VALUE,
	SW_SYS_IOC_GET_TOTAL_GPIO,
	SW_SYS_IOC_GET_GPIO_CFG
};

void show_help(char *argv[])
{
	printf("Usage: %s -f /dev/sw_ctl -m main_name -s sub_name\n", argv[0]);
}

int main( int argc, char *argv[] ) 
{
	int opt =-1, fd = -1;
	
	unsigned int cmd = 2;
	char *file = NULL, *main = NULL, *sub = NULL;
	
	if( argc < 7 ) {
		show_help(argv);
		exit(1);
	}
	while ((opt = getopt(argc, argv, "f:m:s:x:h")) != -1) {
		switch (opt) {
		case 'f':
			file = optarg;
			break;
		case 'm':
			main = optarg;
			break;
		case 's':
			sub = optarg;
			break;
		case 'x':
			cmd = atoi(optarg);
			break;
		case 'h':
		default:
			show_help(argv);
			return 0;
		}
	}

	strncpy(para.main_name, main, SW_SCRIPT_PARA_MAX_NAME_LEN);
	strncpy(para.sub_name, sub, SW_SCRIPT_PARA_MAX_NAME_LEN);
	
	//printf("%s: %s.%s\n", file, para.main_name, para.sub_name);

	fd = open(file, O_RDWR);

	if ( fd < 0) {
		printf("Open sw_sys error \n");
		return fd;
	}

	if ( ioctl(fd, cmd_array[cmd], &para) < 0 ) {
        //printf("SW_SYS_IOC_GET_KEY_VALUE fail\n");
        printf("No value found for [%s]->%s\n", para.main_name, para.sub_name);
        return -1;
	}
	
	switch(para.value_type) {
		case SCIRPT_PARSER_VALUE_TYPE_STRING:
			printf("%s\n", (char *)para.value);
			break;
		case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
			printf("%d\n", para.value[0]);
			break;
		case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
			printf("multi word");
			break;
		case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
			printf("gpio word\n");
			break;
		default:
			printf("error: unknown value type! this shouldn't happen!\n");
			break;
	}

	close(fd);
	
	return 0;
}
