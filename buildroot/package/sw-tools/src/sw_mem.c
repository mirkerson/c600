#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define DEVKMEM         "/dev/kmem"
#define MAX_MEM_SIZE    0x100000

void show_help()
{
	printf("Options:\n");
	printf("  -b [base_address]  kernel logical address\n");
	printf("  -s [size]          in bytes\n");
	printf("  -B                 byte mode, word mode by default\n");
	printf("  -h                 display this help message\n\n");
	printf("For example:\n");
	printf("  ./sw_mem -b 0xc0000000 -s 0x1000\n");
	printf("  you can cat /proc/kallsyms to know the symbol address\n\n");
}

void dump_byte(unsigned long ka, char *base, unsigned long size)
{
	unsigned long i;
	for (i=0; i<size; i++) {
		if (i%16 == 0){
			printf("\n");
			printf("0x%08x: ", ka+i);
		}
		printf("%02X ", base[i]);
	}

	printf("\n\n");
}

void dump_word(unsigned long ka, char *base, unsigned long size)
{
	int i;
	unsigned int *p = (unsigned int *)base;
	for (i=0; i<(size/4); i++) {
		if (i%8 == 0){
			printf("\n");
			printf("0x%08x: ", ka+i*4);
		}
		printf("%08X ", p[i]);
	}

	printf("\n\n");
}

int main(int argc, char* argv[])
{
	int fd, opt=-1, byte_mode=0;
	unsigned long  kaddr = 0;
	unsigned long  size = 0;
	char *va = NULL;

	while ((opt = getopt(argc, argv, "b:s:Bh")) != -1) {
		switch (opt) {
		case 'h':
			show_help();
			return 0;
		case 'b':
			kaddr = (unsigned long)strtoll(optarg, NULL, 16);
			break;
		case 's':
			size = (unsigned long)strtoll(optarg, NULL, 16);
			break;
		case 'B':
			byte_mode = 1;
			break;
		default:
			show_help();
			return 0;
		}
	}	

	fd = open(DEVKMEM, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	printf("addr=%x, size=%x\n", kaddr, size);

	va = mmap(0, size, PROT_READ, MAP_SHARED, fd, kaddr);
	if (va == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	printf("va:%p\n", va);

	if (byte_mode)
		dump_byte(kaddr, va, size);
	else
		dump_word(kaddr, va, size);

	close(fd);
	munmap(va, size);

	return 0;
}
