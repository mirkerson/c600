#ifndef __KEY_H__
#define __KEY_H__

#include "efuse.h"

typedef struct EFUSE_KEY
{
	char name[32];							// key名称
	int key_index;							// 地址索引
	int store_max_bit;					// 允许被烧录的最大bit
	int show_bit_offset;				// key是否允许读
	int burned_bit_offset;			// key是否已经烧录了
	int reserve[4];
}
efuse_key_map_t;





#endif

