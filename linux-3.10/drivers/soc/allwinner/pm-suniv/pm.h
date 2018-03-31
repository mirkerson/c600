#ifndef _PM_H
#define _PM_H

#if defined(CONFIG_ARCH_SUNIVW1P1)
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/of_address.h>

#include "pm_config.h"
#include "pm_types.h"
#include "pm_of.h"
#include "mem_ccmu.h"
#include "mem_clk.h"
#include "mem_divlibc.h"
#include "mem_key.h"
#include "mem_int.h"
#include "mem_twi.h"
#include "mem_tmr.h"
#include "mem_gpio.h"
//#include "pm_printk.h"
#endif

#include "standby/dram.h"
typedef __dram_para_t dram_para_t;

struct sram_state {
	__u32 sram_reg_back[SRAM_REG_LENGTH];
};

/*save module state*/
__s32 mem_sram_init(void);
__s32 mem_sram_save(struct sram_state *psram_state);
__s32 mem_sram_restore(struct sram_state *psram_state);
__s32 mem_ccu_save(struct ccm_state *ccm_reg);
__s32 mem_ccu_restore(struct ccm_state *ccm_reg);

#endif
