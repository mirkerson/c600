/*
 *  drivers/arisc/arisc.c
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "arisc_i.h"

/* local functions */
static int arisc_wait_ready(unsigned int timeout);

struct dts_cfg dts_cfg;
unsigned int arisc_debug_dram_crc_en = 0;
unsigned int arisc_debug_dram_crc_srcaddr = 0x40000000;
unsigned int arisc_debug_dram_crc_len = (1024 * 1024);
unsigned int arisc_debug_dram_crc_error = 0;
unsigned int arisc_debug_dram_crc_total_count = 0;
unsigned int arisc_debug_dram_crc_error_count = 0;
volatile const unsigned int arisc_debug_level = 2;
static unsigned char arisc_version[40] = "arisc defualt version";

static int arisc_wait_ready(unsigned int timeout)
{
	/* wait arisc startup ready */
	while (1) {
		/*
		 * linux cpu interrupt is disable now,
		 * we should query message by hand.
		 */
		struct arisc_message *pmessage = arisc_hwmsgbox_query_message();
		if (pmessage == NULL) {
			/* try to query again */
			continue;
		}
		/* query valid message */
		if (pmessage->type == ARISC_STARTUP_NOTIFY) {
			/* check arisc software and driver version match or not */
			if (pmessage->paras[0] != ARISC_VERSIONS) {
				ARISC_ERR("arisc firmware:%d and driver version:%u not matched\n", pmessage->paras[0], ARISC_VERSIONS);
				return -EINVAL;
			} else {
				/* printf the main and sub version string */
				memcpy((void *)arisc_version, (const void*)(&(pmessage->paras[1])), 40);
				ARISC_LOG("arisc version: [%s]\n", arisc_version);
			}

			/* received arisc startup ready message */
			ARISC_INF("arisc startup ready\n");
			if ((pmessage->attr & ARISC_MESSAGE_ATTR_SOFTSYN) ||
				(pmessage->attr & ARISC_MESSAGE_ATTR_HARDSYN)) {
				/* synchronous message, just feedback it */
				ARISC_INF("arisc startup notify message feedback\n");
				pmessage->paras[0] = (uint32_t)dts_cfg.image.base;
				arisc_hwmsgbox_feedback_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
			} else {
				/* asyn message, free message directly */
				ARISC_INF("arisc startup notify message free directly\n");
				arisc_message_free(pmessage);
			}
			break;
		}
		/*
		 * invalid message detected, ignore it.
		 * by superm at 2012-7-6 18:34:38.
		 */
		ARISC_WRN("arisc startup waiting ignore message\n");
		if ((pmessage->attr & ARISC_MESSAGE_ATTR_SOFTSYN) ||
			(pmessage->attr & ARISC_MESSAGE_ATTR_HARDSYN)) {
			/* synchronous message, just feedback it */
			arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
		} else {
			/* asyn message, free message directly */
			arisc_message_free(pmessage);
		}
		/* we need waiting continue */
	}

	return 0;
}

int sunxi_deassert_arisc(void)
{
	ARISC_INF("set arisc reset to de-assert state\n");
	{
		volatile unsigned long value;
		value = readl(dts_cfg.cpuscfg.base + 0x0);
		value &= ~1;
		writel(value, dts_cfg.cpuscfg.base + 0x0);
		value = readl(dts_cfg.cpuscfg.base + 0x0);
		value |= 1;
		writel(value, dts_cfg.cpuscfg.base + 0x0);
	}

	return 0;
}

static int sunxi_arisc_para_init(struct arisc_para *para)
{
	/* init para */
	memset(para, 0, sizeof(struct arisc_para));
	para->message_pool_phys = (uint32_t)dts_cfg.space.msgpool_dst;
	para->message_pool_size = (uint32_t)dts_cfg.space.msgpool_size;
	para->standby_base = (uint32_t)dts_cfg.space.standby_dst;
	para->standby_size = (uint32_t)dts_cfg.space.standby_size;
	memcpy((void *)&para->vf, (void *)dts_cfg.vf, sizeof(para->vf));
	memcpy((void *)&para->dram_para, (void *)&dts_cfg.dram_para, sizeof(para->dram_para));
	memcpy(&para->ir_key, &dts_cfg.s_cir.ir_key, sizeof(ir_key_t));
	memcpy(&para->start_os, &dts_cfg.start_os, sizeof(box_start_os_cfg_t));
	para->suart_status = dts_cfg.s_uart.status;
	para->pmu_bat_shutdown_ltf = dts_cfg.pmu.pmu_bat_shutdown_ltf;
	para->pmu_bat_shutdown_htf = dts_cfg.pmu.pmu_bat_shutdown_htf;
	para->pmu_pwroff_vol = dts_cfg.pmu.pmu_pwroff_vol;
	para->power_start = dts_cfg.pmu.power_start;
	para->powchk_used = dts_cfg.power.powchk_used;
	para->power_reg = dts_cfg.power.power_reg;
	para->system_power = dts_cfg.power.system_power;

	ARISC_LOG("arisc_para size:%llx\n", sizeof(struct arisc_para));
	ARISC_INF("msgpool base:%x, size:%u\n", para->message_pool_phys,
		para->message_pool_size);

	return 0;
}

uint32_t sunxi_load_arisc(uintptr_t image_addr, size_t image_size, void *para, size_t para_size)
{
	void *dst;
	void *src;
	size_t size;

#if 0
	/*
	 * phys addr to virt addr
	 * io space: ioremap
 	 * kernel space: phys_to_virt
 	 */
	/* sram code space */
	dst = (void *)dts_cfg.space.sram_dst;
	src = (void *)((ptrdiff_t)image_addr + (ptrdiff_t)dts_cfg.space.sram_offset);
	size = dts_cfg.space.sram_size;
	memset(dst, 0, size);
	memcpy(dst, src, size);
	flush_dcache_range((uint64_t)dst, (uint64_t)size);

	/* dram code space */
	dst = (void *)dts_cfg.space.dram_dst;
	src = (void *)((ptrdiff_t)image_addr + (ptrdiff_t)dts_cfg.space.dram_offset);
	size = dts_cfg.space.dram_size;
	memset(dst, 0, size);
	memcpy(dst, src, size);
	flush_dcache_range((uint64_t)dst, (uint64_t)size);

	ARISC_INF("load arisc image finish\n");
#endif
	/* para space */
	dst = (void *)dts_cfg.space.para_dst;
	src = para;
	size = dts_cfg.space.para_size;
	memset(dst, 0, size);
	memcpy(dst, src, size);
	ARISC_INF("setup arisc para finish\n");
	//dcsw_op_all(DCCISW);
	flush_dcache_range((uint64_t)dst, (uint64_t)size);
	isb();

#if 0
	/* relese arisc reset */
	sunxi_deassert_arisc();
	ARISC_INF("release arisc reset finish\n");

	ARISC_INF("load arisc finish\n");
#endif

	return 0;
}

#define START_OS_REG     (0x01f00108)

#define START_TYPE_WAIT_WAKEUP  0
#define START_TYPE_ENTER_KERNEL 1

#define MODE_NULL          (0x0)
#define MODE_SHUTDOWN_OS   (0x1)
#define MODE_WAIT_WAKE_UP  (0x2)
#define MODE_RUN_OS        (0xf)

int sunxi_arisc_probe(void *cfg)
{
	struct arisc_para para;

	ARISC_LOG("sunxi-arisc driver begin startup %d\n", arisc_debug_level);
	memcpy((void *)&dts_cfg, (const void *)cfg, sizeof(struct dts_cfg));

	/* init arisc parameter */
	sunxi_arisc_para_init(&para);

	/* load arisc */
	sunxi_load_arisc(dts_cfg.image.base, dts_cfg.image.size,
	                 (void *)(&para), sizeof(struct arisc_para));

	/* initialize hwspinlock */
	ARISC_INF("hwspinlock initialize\n");
	arisc_hwspinlock_init();

	/* initialize hwmsgbox */
	ARISC_INF("hwmsgbox initialize\n");
	arisc_hwmsgbox_init();

	/* initialize message manager */
	ARISC_INF("message manager initialize start:0x%llx, size:0x%llx\n", dts_cfg.space.msgpool_dst, dts_cfg.space.msgpool_size);
	arisc_message_manager_init((void *)dts_cfg.space.msgpool_dst, dts_cfg.space.msgpool_size);

	/* wait arisc ready */
	ARISC_INF("wait arisc ready....\n");
	if (arisc_wait_ready(10000)) {
		ARISC_LOG("arisc startup failed\n");
	}

	//ARISC_LOG("used:%x\n", para.start_os.used);
	//ARISC_LOG("start_type:%x\n", para.start_os.start_type);
	//ARISC_LOG("irkey_used:%x\n", para.start_os.irkey_used);
	//ARISC_LOG("pmukey_used:%x\n", para.start_os.pmukey_used);
	//ARISC_LOG("pmukey_num:%x\n", para.start_os.pmukey_num);
	//ARISC_LOG("led_power:%x\n", para.start_os.led_power);
	//ARISC_LOG("led_state:%x\n", para.start_os.led_state);
	//ARISC_LOG("RTC:%x\n", readl(START_OS_REG));

	if (para.start_os.used) {
		if (para.start_os.start_type == START_TYPE_WAIT_WAKEUP) {
			if ((readl(START_OS_REG) == MODE_NULL) || (readl(START_OS_REG) == MODE_SHUTDOWN_OS) || (readl(START_OS_REG) == MODE_WAIT_WAKE_UP)) {
				while(1) {
					ARISC_LOG("wfi\n");
					dsb();
					wfi();
				}
			}
		} else if (para.start_os.start_type == START_TYPE_ENTER_KERNEL) {
			if ((readl(START_OS_REG) == MODE_SHUTDOWN_OS) || (readl(START_OS_REG) == MODE_WAIT_WAKE_UP)) {
				while(1) {
					ARISC_LOG("wfi\n");
					dsb();
					wfi();
				}
			}
		}
	}

	//arisc_set_paras();

	/* enable arisc asyn tx interrupt */
	//arisc_hwmsgbox_enable_receiver_int(ARISC_HWMSGBOX_ARISC_ASYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);

	/* enable arisc syn tx interrupt */
	//arisc_hwmsgbox_enable_receiver_int(ARISC_HWMSGBOX_ARISC_SYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);

	/* arisc initialize succeeded */
	ARISC_LOG("sunxi-arisc driver v%s is starting\n", DRV_VERSION);

	return 0;
}

int sunxi_arisc_wait_ready(void)
{
	ARISC_INF("wait arisc ready....\n");
	if (arisc_wait_ready(10000)) {
		ARISC_LOG("arisc startup failed\n");
	}
	arisc_set_paras();
	ARISC_LOG("sunxi-arisc driver v%s startup ok\n", DRV_VERSION);
	return 0;
}

