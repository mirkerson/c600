/*
 * Command for mw.
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>

static int do_memory_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned long int addr_base, reg_val;
    unsigned long int * map_base;  
	int len;
 
    if(argc < 3)    
		return -1;      	
    
    addr_base = simple_strtoul(argv[1],NULL,0);
    if(addr_base % 4 != 0)
    {
        printf("address must be aligned(32b)\n");
        return -1;
    }

    reg_val = simple_strtoul (argv[2],NULL,0);

    map_base = addr_base;
    *map_base = reg_val;
    
	printf("0x%x: %08x\n", addr_base, *map_base);  
    /*
    map_base[3] = reg_val>>24;    
    map_base[2] = (reg_val>>16)&0xff;
    map_base[1] = (reg_val>>8)&0xff;
    map_base[0] = reg_val&0xff;
    
    
	printf("0x%x: %02x", addr_base, map_base[3]);  
	printf("%02x", map_base[2]);  
	printf("%02x", map_base[1]);  
    printf("%02x\n", map_base[0]);  
    */

	return 0;

}

U_BOOT_CMD(
	memw,	3,	0,	do_memory_write,
	"memory write",
	"addr val    - write value to addr"
);

