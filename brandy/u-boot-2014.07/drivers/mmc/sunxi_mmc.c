/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 *
 * MMC driver for allwinner sunxi platform.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/timer.h>
#include <malloc.h>
#include <mmc.h>
#include "mmc_def.h"
#include "sunxi_mmc.h"

#ifdef CONFIG_ARCH_SUN8IW10P1
#include "sun8iw10p1/sunxi_host_mmc.h"
#elif defined(CONFIG_ARCH_SUN50IW1P1)
#include "sun50iw1p1/sunxi_host_mmc.h"
#elif defined(CONFIG_ARCH_SUN8IW11P1)
#include "sun8iw11p1/sunxi_host_mmc.h"
#elif defined(CONFIG_ARCH_SUNIVW1P1)
#include "sunivw1p1/sunxi_host_mmc.h"
#endif

DECLARE_GLOBAL_DATA_PTR;
//#define SUNXI_MMCDBG
//#undef SUNXI_MMCDBG
//#define MMCINFO(fmt...)	printf("[mmc]: "fmt)


#ifdef SUNXI_MMCDBG
//#define MMCDBG(fmt...)	printf("[mmc]: "fmt)

void dumphex32(char* name, char* base, int len)
{
	__u32 i;

	MMCPRINT("dump %s registers:", name);
	for (i=0; i<len; i+=4) {
		if (!(i&0xf))
			MMCPRINT("\n0x%p : ", base + i);
		MMCPRINT("0x%08x ", readl((ulong)base + i));
	}
	MMCPRINT("\n");
}

/*
static void dumpmmcreg(struct sunxi_mmc *reg)
{
	printf("dump mmc registers:\n");
	printf("gctrl     0x%08x\n", reg->gctrl     );
	printf("clkcr     0x%08x\n", reg->clkcr     );
	printf("timeout   0x%08x\n", reg->timeout   );
	printf("width     0x%08x\n", reg->width     );
	printf("blksz     0x%08x\n", reg->blksz     );
	printf("bytecnt   0x%08x\n", reg->bytecnt   );
	printf("cmd       0x%08x\n", reg->cmd       );
	printf("arg       0x%08x\n", reg->arg       );
	printf("resp0     0x%08x\n", reg->resp0     );
	printf("resp1     0x%08x\n", reg->resp1     );
	printf("resp2     0x%08x\n", reg->resp2     );
	printf("resp3     0x%08x\n", reg->resp3     );
	printf("imask     0x%08x\n", reg->imask     );
	printf("mint      0x%08x\n", reg->mint      );
	printf("rint      0x%08x\n", reg->rint      );
	printf("status    0x%08x\n", reg->status    );
	printf("ftrglevel 0x%08x\n", reg->ftrglevel );
	printf("funcsel   0x%08x\n", reg->funcsel   );
	printf("dmac      0x%08x\n", reg->dmac      );
	printf("dlba      0x%08x\n", reg->dlba      );
	printf("idst      0x%08x\n", reg->idst      );
	printf("idie      0x%08x\n", reg->idie      );
	printf("cbcr      0x%08x\n", reg->cbcr      );
	printf("bbcr      0x%08x\n", reg->bbcr      );
}
*/
#else
//#define MMCDBG(fmt...)
//#define dumpmmcreg(fmt...)
//#define  dumphex32(fmt...)
void dumphex32(char* name, char* base, int len) {};

#endif /* SUNXI_MMCDBG */

#define MMC_CLK_400K			0
#define MMC_CLK_25M			1
#define MMC_CLK_50M			2
#define MMC_CLK_50MDDR			3
#define MMC_CLK_50MDDR_8BIT		4
#define MMC_CLK_100M			5
#define MMC_CLK_200M			6
#define MMC_CLK_MOD_NUM			7

int sunxi_mmc_init(int sdc_no)
{
#ifdef CONFIG_ARCH_SUN8IW10P1
	return sunxi_sun8iw10p1_mmc_init(sdc_no);
#elif defined(CONFIG_ARCH_SUN50IW1P1)
	return sunxi_sun50iw1p1_mmc_init(sdc_no);
#elif defined(CONFIG_ARCH_SUN8IW11P1)
	return sunxi_sun8iw11p1_mmc_init(sdc_no);
#elif defined(CONFIG_ARCH_SUNIVW1P1)
	return sunxi_sunivw1p1_mmc_init(sdc_no);
#else
	#error "*******not support platform !!!" 
#endif
}

int sunxi_mmc_exit(int sdc_no)
{
#ifdef CONFIG_ARCH_SUN8IW10P1
	sunxi_sun8iw10p1_mmc_exit(sdc_no);
#elif defined(CONFIG_ARCH_SUN50IW1P1)
	sunxi_sun50iw1p1_mmc_exit(sdc_no);
#elif defined(CONFIG_ARCH_SUN8IW11P1)
	sunxi_sun8iw11p1_mmc_exit(sdc_no);
#elif defined(CONFIG_ARCH_SUNIVW1P1)
	sunxi_sunivw1p1_mmc_exit(sdc_no);
#else
	#error "*******not support platform !!!" 	
#endif
	MMCDBG("sunxi mmc%d exit\n", sdc_no);
	return 0;
}
