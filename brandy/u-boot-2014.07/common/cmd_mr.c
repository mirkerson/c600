/*
 * Command for mr.
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>

static int do_memory_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned long int addr_base;
    unsigned long int* map_base;  
	int len;
 
    if(argc < 2 || argc > 3)    
		return -1;      	
    
    addr_base = simple_strtoul(argv[1],NULL,0);
    if(addr_base % 16 != 0)
    {
        printf("address must be aligned(16B)\n");
        return -1;
    }
    
    if(2 == argc)
    	len = 0x100;
    else
      len = simple_strtoul(argv[2],NULL,0);

    if( (len % 4 != 0) || (len <= 0) )
    {
        printf("len must be aligned(32b) > 0\n");
        return -1;
    }
    
    map_base = addr_base;
    
    int i,j;  
    for (i = 0; i < len/4; i++)  
    {  
		if(0 == i%4)
			printf("\n0x%x: ", addr_base + 4*i);
				
		//j=3;
		//while(j>=0)
        //{   
	        printf("%08x", map_base[i]);  
	    //    j--;			
        //}
        
        printf(" ");
    }  
    
    printf("\n");

	return 0;

}

U_BOOT_CMD(
	memr,	3,	0,	do_memory_read,
	"memory read",
	"addr len    - read value of addr"
);
