#ifndef _DRV_DISPLAY_COMMON_H_
#define _DRV_DISPLAY_COMMON_H_

#define CONFIG_CHIP_ID 1123

#if CONFIG_CHIP_ID == 1120

#include "drv_display_sun3i.h"

#elif CONFIG_CHIP_ID == 1123

#include "drv_display_sun4i.h"

#else

#error "no chip id defined"

#endif



#endif



