#ifndef   _MCTL_HAL_H   
#define   _MCTL_HAL_H

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/dram.h>

extern unsigned int mctl_init(void *para);
extern signed int init_DRAM(int type, __dram_para_t *para);

#endif  //_MCTL_HAL_H
