/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/clk-private.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk/sunxi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include "clk-sunxi.h"
#include "clk-factors.h"
#include "clk-periph.h"
#include "clk-sunivw1.h"
#include "clk-sunivw1_tbl.c"

#ifndef CONFIG_EVB_PLATFORM
	#define LOCKBIT(x) 31
#else
	#define LOCKBIT(x) x
#endif
static DEFINE_SPINLOCK(clk_lock);
void __iomem *sunxi_clk_base;
void __iomem *sunxi_clk_cpus_base=0;
int    sunxi_clk_maxreg =SUNXI_CLK_MAX_REG;
int cpus_clk_maxreg = 0;
#ifdef CONFIG_SUNXI_CLK_DUMMY_DEBUG
unsigned int dummy_reg[1024];
unsigned int dummy_readl(unsigned int* regaddr)
{
	unsigned int val;
	val = *regaddr;

	printk("--%s-- dummy_readl to read reg 0x%x with val 0x%x\n",__func__,((unsigned int)regaddr - (unsigned int)&dummy_reg[0]),val);
	return val;
}
void  dummy_writel(unsigned int val,unsigned int* regaddr)
{
	*regaddr = val;
	printk("--%s-- dummy_writel to write reg 0x%x with val 0x%x\n",__func__,((unsigned int)regaddr - (unsigned int)&dummy_reg[0]),val);
}

void dummy_reg_init(void)
{
	memset(dummy_reg,0x0,sizeof(dummy_reg));
	dummy_reg[PLL_CPU/4]=0x00001000;
	dummy_reg[PLL_AUDIO/4]=0x00035514;
	dummy_reg[PLL_VIDEO/4]=0x03006207;		//FIXME: copy from sun8iw10p1
	dummy_reg[PLL_VE/4]=0x03006207;
	dummy_reg[PLL_DDR/4]=0x00001800;
	dummy_reg[PLL_PERIPH/4]=0x00041811;
	
	dummy_reg[CPU_CFG/4]=0x00010300;
	dummy_reg[AHB1_CFG/4]=0x00001010;
	dummy_reg[PLL_LOCK/4]=0x000000FF;
	dummy_reg[CPU_LOCK/4]=0x000000FF;
}
#endif // of CONFIG_SUNXI_CLK_DUMMY_DEBUG

/*                                          ns  nw  ks  kw  ms  mw  ps  pw  d1s d1w d2s d2w {frac   out mode}   en-s    sdmss   sdmsw   sdmpat         sdmval*/
SUNXI_CLK_FACTORS(          pll_cpu,        8,  5,  4,  2,  0,  2,  16, 2,  0,  0,  0,  0,    0,    0,  0,      31,     24,     0,      0,    0);
SUNXI_CLK_FACTORS(          pll_audio,      8,  7,  0,  0,  0,  5,  16, 4,  0,  0,  0,  0,    0,    0,  0,      31,     24,      0,     PLL_AUDIOPAT,  0xd1303333);
SUNXI_CLK_FACTORS(          pll_video,     8,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,    1,    25, 24,     31,     20,     0,      PLL_VIDEOPAT, 0xd1303333);
SUNXI_CLK_FACTORS(          pll_ve,     8,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,    1,    25, 24,     31,     20,     0,      0, 0);
SUNXI_CLK_FACTORS_UPDATE(   pll_ddr,  8,  5,  4,  2,  0,  2,  0,  0,  0,  0,  0,  0,    0,    0,  0,      31,     0,     0,      PLL_DDRPAT,  0xf1303333 , 30);
SUNXI_CLK_FACTORS(          pll_periph,    8,  5,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,    0,    0,  0,      31,     0,      0,      0,             0);

static int get_factors_pll_cpu(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{

	int index;
	u64 tmp_rate;
	if(!factor)
		return -1;
	tmp_rate = rate>pllcpu_max ? pllcpu_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_cpu,factor, factor_pllcpu_tbl,index,sizeof(factor_pllcpu_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}

static int get_factors_pll_audio(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	if(rate == 22579200) {
		factor->factorn = 78;
		factor->factorm = 20;
		factor->factorp = 3;
	} else if(rate == 24576000) {
		factor->factorn = 85;
		factor->factorm = 20;
		factor->factorp = 3;
	} else
		return -1;

	return 0;
}

static int get_factors_pll_video(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate;
	int index;

	if(!factor)
		return -1;

	tmp_rate = rate>pllvideo_max ? pllvideo_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_video,factor, factor_pllvideo_tbl,index,sizeof(factor_pllvideo_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	if(rate == 297000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 1;
		factor->factorm = 0;
	}
	else if(rate == 270000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 0;
		factor->factorm = 0;
	} else {
		factor->frac_mode = 1;
		factor->frac_freq = 0;
	}

	return 0;
}

static int get_factors_pll_ve(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if (!factor)
		return -1;

	tmp_rate = rate > pllve_max ? pllve_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if (sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ve, factor,
					 factor_pllve_tbl, index,
					 sizeof(factor_pllve_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}


static int get_factors_pll_ddr(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if (!factor)
		return -1;

	tmp_rate = rate > pllddr_max ? pllddr_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if (sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ddr, factor,
					 factor_pllddr_tbl, index,
					 sizeof(factor_pllddr_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}

static int get_factors_pll_periph(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if(!factor)
		return -1;

	tmp_rate = rate>pllperiph_max ? pllperiph_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;

	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_periph,factor, factor_pllperiph_tbl,index,sizeof(factor_pllperiph_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;
	return 0;
}



/*    pll_cpux: 24*N*K/(M*P)    */
static unsigned long calc_rate_pll_cpu(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
	do_div(tmp_rate, (factor->factorm+1) * (1 << factor->factorp));
	return (unsigned long)tmp_rate;
}
/*    pll_audio:24*N/(M*P)    */
static unsigned long calc_rate_pll_audio(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	if((factor->factorn == 78) && (factor->factorm == 20) && (factor->factorp == 3))
		return 22579200;
	else if((factor->factorn == 85) && (factor->factorm == 20) && (factor->factorp == 3))
		return 24576000;
	else
	{
		tmp_rate = tmp_rate * (factor->factorn+1);
		do_div(tmp_rate, (factor->factorm+1) * (factor->factorp+1));
		return (unsigned long)tmp_rate;
	}
}
/*    pll_video0:24*N/M    */
static unsigned long calc_rate_video(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	if(factor->frac_mode == 0)
	{
		if(factor->frac_freq == 1)
			return 297000000;
		else
			return 270000000;
	}
	else
	{
		tmp_rate = tmp_rate * (factor->factorn+1);
		do_div(tmp_rate, factor->factorm+1);
		return (unsigned long)tmp_rate;
	}
}

/*    pll_ddr:24*N/M    */
static unsigned long calc_rate_pll_ve(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	tmp_rate = tmp_rate * (factor->factorn+1);
	do_div(tmp_rate, factor->factorm+1);
	return (unsigned long)tmp_rate;
}


/*    pll_ddr:24*N*K/M    */
static unsigned long calc_rate_pll_ddr(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);

	tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
	do_div(tmp_rate, factor->factorm+1);

	return (unsigned long)tmp_rate;
}

/*    pll_periph0:24*N*K    */
static unsigned long calc_rate_pll_periph(u32 parent_rate, struct clk_factors_value *factor)
{
	return (unsigned long)(parent_rate?(parent_rate):12000000) * (factor->factorn+1) * (factor->factork+1);
}

static const char *hosc_parents[] = {"hosc"};
//static const char *pll_24m_parents[] = {"pll_24m"};

struct factor_init_data sunxi_factos[] = {
	/* name             parent          parent_num,     flags                                               reg             lock_reg        lock_bit       pll_lock_ctrl_reg    lock_en_bit     lock_mode                config                                get_factors                     calc_rate                   priv_ops*/
	{"pll_cpu",     hosc_parents, 1,          CLK_IGNORE_DISABLE | CLK_GET_RATE_NOCACHE, PLL_CPU,     PLL_CPU,     LOCKBIT(28), 0,     0,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_cpu,     &get_factors_pll_cpu,     &calc_rate_pll_cpu,    (struct clk_ops*)NULL},
	{"pll_audio",   hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_AUDIO,   PLL_AUDIO,   LOCKBIT(28), 0,     1,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_audio,   &get_factors_pll_audio,   &calc_rate_pll_audio,  (struct clk_ops*)NULL},
	{"pll_video",  hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_VIDEO,  PLL_VIDEO,  LOCKBIT(28), 0,     2,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_video,  &get_factors_pll_video,  &calc_rate_video,      (struct clk_ops*)NULL},
	{"pll_ve",  hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_VE,  PLL_VE,  LOCKBIT(28), 0,     3,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_ve,  &get_factors_pll_ve,  &calc_rate_pll_ve,      (struct clk_ops*)NULL},
	{"pll_ddr",    hosc_parents, 1,          CLK_IGNORE_DISABLE | CLK_GET_RATE_NOCACHE, PLL_DDR,    PLL_DDR,    LOCKBIT(28), 0,     4,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_ddr,    &get_factors_pll_ddr,    &calc_rate_pll_ddr,   (struct clk_ops*)NULL},
	{"pll_periph", hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_PERIPH, PLL_PERIPH, LOCKBIT(28), 0,     5,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_periph, &get_factors_pll_periph, &calc_rate_pll_periph, (struct clk_ops*)NULL},
};

struct periph_init_data {
	const char              *name;
	unsigned long           flags;
	const char              **parent_names;
	int                     num_parents;
	struct sunxi_clk_periph *periph;
};

static const char *cpu_parents[] = {"losc", "hosc", "pll_cpu", "pll_cpu"};
static const char *pll_periphahb_parents[] = {"pll_periph"};
static const char *ahb1_parents[] = {"losc", "hosc", "cpu","pll_periphahb"};
static const char *apb1_parents[] = {"ahb1"};

static const char *sdmmc_parents[] = {"hosc","pll_periph", "", ""};
static const char *spi_parents[] = {"hosc","pll_periph", "", ""};
static const char *cir_parents[] = {"hosc","pll_periph", "", ""};



static const char *audio_parents[] = {"pll_audiox8", "pll_audiox4", "pll_audiox2", "pll_audio"};
static const char *avs_parents[] = {"hosc"};
static const char *codec_parents[] = {"pll_audio"};

static const char *de_parents[] = {"pll_video", "", "pll_periph", "", "", "", "", ""};
static const char *tcon_parents[] = {"pll_video", "", "pll_videox2", "", "", "", "", ""};
static const char *deinterlace_parents[] = {"pll_video", "", "pll_videox2", "", "", "", "", ""};
static const char *tve_clk2_parents[] = {"pll_video", "", "pll_videox2", "", "", "", "", ""};
static const char *tve_clk1_parents[] = {"tve_clk2", "tve_clk2_div2"};
static const char *tvd_parents[] = {"pll_video", "hosc", "pll_videox2", "", "", "", "", ""};


static const char *csi_m_parents[] = {"pll_video", "","","","","hosc", "", ""};
static const char *sdram_parents[] = {"pll_ddr"};
static const char *ve_parents[] = {"pll_ve"};


static const char *periphx2_parents[] = {"hosc", "pll_periph0x2","pll_periph1x2",""};
static const char *i2s_parents[] = {"pll_audiox8", "pll_audiox4", "pll_audiox2", "pll_audio", "ext_clk0", "ext_clk1", "", ""};
static const char *mbus_parents[] = {"hosc", "pll_periph0x2", "pll_ddr0", "pll_ddr1"};

static const char *periphx_parents[] = {"pll_periph0","pll_video1","","","","","",""};

static const char *csi_misc_parents[] = {"pll_24m", "hosc","",""};
static const char *adda_parents[] = {"pll_audio", "ext_clk0", "ext_clk1", ""};
static const char *wlan_parents[] = {"hosc", "ext_hosc"};
static const char *ahb1mod_parents[] = {"ahb1"};
static const char *apb1mod_parents[] = {"apb1"};
static const char *apb2mod_parents[] = {"apb2"};

static const char *usbohci_parents[] = {"usbohci_16"};
static const char *losc_parents[] = {"losc"};

struct sunxi_clk_comgate com_gates[]={
{"csi",       0,  0x7,    BUS_GATE_SHARE|RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
};

/*
SUNXI_CLK_PERIPH(		    name,			mux_reg, 	mux_sft, 	mux_wid,	div_reg,		div_msft,	div_mwid, 	div_nsft, 	div_nwid, 	gate_flag, 	en_reg,		rst_reg,		bus_gate_reg,  	drm_gate_reg,  	en_sft, 	rst_sft, 	bus_gate_sft, 	dram_gate_sft, 	lock,  		com_gate,   	com_gate_off)
*/
SUNXI_CLK_PERIPH(cpu,            		CPU_CFG,    	16,   		2,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,            		0,    			0,             		0,         			0,          	0,          	0,              		0,   				&clk_lock, 	NULL,     		0);
SUNXI_CLK_PERIPH(pll_periphahb, 	0,               	0,    			0,          		AHB1_CFG,   	6,      		2,          		0,          		0,          		0,          		0,               	0,               	0,             		0,         			0,          	0,          	0,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(ahb1,           	AHB1_CFG,    	12,   		2,          		AHB1_CFG,   	0,      		0,          		4,          		2,          		0,          		0,          		0,           		0,             		0,         			0,          	0,          	0,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(apb1,           	0,               	0,    			0,          		AHB1_CFG,  	0,      		0,          		8,          		2,          		0,          		0,           		0,             	0,             		0,         			0,          	0,          	0,              		0,   				&clk_lock, 	NULL,             	0);

SUNXI_CLK_PERIPH(sdmmc0_mod,     SD0_CFG,     	24,   		2,          		SD0_CFG,     	0,      		4,          		16,         		2,          		0,          		SD0_CFG,     	0,               	0,             		0,         			31,         	0,          	0,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(sdmmc0_bus,     	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	0,               	BUS_GATE0,     	0,         			0,          	0,          	8,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(sdmmc0_rst,     	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST0, 	0,             		0,         			0,          	8,          	0,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(sdmmc1_mod,     SD1_CFG,    	24,   		2,          		SD1_CFG,    	0,      		4,          		16,         		2,          		0,          		SD1_CFG,   	0,               	0,             		0,         			31,         	0,         	0,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(sdmmc1_bus,     	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	0,               	BUS_GATE0,     	0,         			0,          	0,          	9,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(sdmmc1_rst,     	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST0,  	0,             		0,         			0,          	9,          	0,              		0,   				&clk_lock, 	NULL,             	0);

//FIXME: SPI have no register to select source clock
SUNXI_CLK_PERIPH(spi0,           		SPI0_CFG,    	24,   		2,          		SPI0_CFG,  	0,      		4,          		16,         		2,          		0,          		SPI0_CFG,   	BUS_RST0, 	BUS_GATE0, 		0,         			31,       	20,         	20,         			0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(spi1,           		SPI1_CFG,    	24,   		2,          		SPI1_CFG,     	0,      		4,          		16,         		2,          		0,          		SPI1_CFG,    	BUS_RST0,    	BUS_GATE0,  		0,         			31,         	21,         	21,         			0,   				&clk_lock, 	NULL,             	0);

SUNXI_CLK_PERIPH(usbphy0,        	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		USB_CFG,     	USB_CFG,    	0,             		0,         			1,          	0,          	0,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(usbotg,         	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST0,    	BUS_GATE0,     	0,         			0,          	24,         	24,             		0,   				&clk_lock, 	NULL,             	0);

SUNXI_CLK_PERIPH(audio,           	AUDIO_CFG, 	16,   		2,          		0,                  	0,      		0,          		0,          		0,          		0,          		AUDIO_CFG,  	BUS_RST2,    	BUS_GATE2,     	0,         			31,         	12,         	12,             		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(avs,           		AVS_CFG, 	0,   			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		AVS_CFG,  	0,    			0,     			0,         			31,         	0,         	0,             		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(codec,           	CODEC_CFG, 	0,   			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		CODEC_CFG,  BUS_RST2,    	BUS_GATE2,     	0,         			31,         	0,         	0,             		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(spdif,          	SPDIF_CFG, 	16,   		2,          		0,          		0,      		0,          		0,          		0,          		0,          		SPDIF_CFG,    BUS_RST2,  	BUS_GATE2,     	0,         			31,         	1,          	1,              		0,   				&clk_lock, 	NULL,             	0);

SUNXI_CLK_PERIPH(debe,             	BE_CFG,       	24,   		3,          		BE_CFG,      	0,      		4,          		0,          		0,          		0,          		BE_CFG,    	BUS_RST1,   	BUS_GATE1,     	DRAM_GATE,         	31,        	12,         	12,             		26,   			&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(defe,             	FE_CFG,        	24,   		3,          		FE_CFG,       	0,      		4,          		0,          		0,          		0,         	 	FE_CFG,       	BUS_RST1,    	BUS_GATE1,     	DRAM_GATE, 		31,        	14,        	14,             		24,   			&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(tcon,          		TCON_CFG, 	24,   		3,          		0,                  	0,      		0,          		0,          		0,          		0,          		TCON_CFG,  	BUS_RST1,    	BUS_GATE1,     	0,         			31,         	4,          	4,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(deinterlace,         DEINTERLACE_CFG, 	24,   3,          		DEINTERLACE_CFG,   0,      	4,          		0,          		0,          		0,          		DEINTERLACE_CFG,  	BUS_RST1,    	BUS_GATE1,     	DRAM_GATE,  31,   5,          	5,              		2,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(tve_clk2,         	TVE_CFG, 	24,   		3,          		TVE_CFG,   	0,      		4,          		0,          		0,          		0,          		TVE_CFG,  	BUS_RST1,    	BUS_GATE1,     	0,  				31,   	10,          	10,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(tve_clk1,         	TVE_CFG, 	8,   			1,          		0,   			0,      		0,          		0,          		0,          		0,          		TVE_CFG,  	BUS_RST1,    	BUS_GATE1,     	0,  				15,   	10,          	10,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(tvd,         		TVD_CFG, 	24,   		3,          		TVD_CFG,   	0,      		4,          		0,          		0,          		0,          		TVD_CFG,  	BUS_RST1,    	BUS_GATE1,     	DRAM_GATE,  	31,   	9,          	9,              		3,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(csi_m,          	CSI_CFG,      	8,    			3,          		CSI_CFG,     	0,      		4,          		0,          		0,          		0,          		CSI_CFG,   	BUS_RST1, 	BUS_GATE1,     	DRAM_GATE, 		15,         	8,          	8,              		1,  	 			&clk_lock, 	&com_gates[0],    1);
SUNXI_CLK_PERIPH(ve,          		VE_CFG,      	0,    			0,          		0,     		0,      		0,          		0,          		0,          		0,          		VE_CFG,   	BUS_RST1, 	BUS_GATE1,     	DRAM_GATE, 		31,         	0,          	0,              		0,  	 			&clk_lock, 	NULL,    		0);

SUNXI_CLK_PERIPH(sdram,          	0,  			0,   			0,          		0,  			0,      		0,          		0,          		0,          		0,          		0, 			BUS_RST0,    	BUS_GATE0,     	0,         			0,         	14,         	14,             		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(dma,            	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST0,  	BUS_GATE0,     	0,         			0,          	6,          	6,              		0,   				&clk_lock, 	NULL,             	0);
SUNXI_CLK_PERIPH(uart0,          		0,               	0,    			0,          		0,                 	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,    	BUS_GATE2,     	0,         			0,          	20,         	20,             		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(uart1,          	0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,     	BUS_GATE2,     	0,         			0,          	21,         	21,             		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(uart2,          		0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,    	BUS_GATE2,     	0,         			0,          	22,         	22,             		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(twi0,           		0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,   	BUS_GATE2,     	0,         			0,          	16,          	16,              		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(twi1,           		0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,   	BUS_GATE2,     	0,        	 		0,          	17,          	17,              		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(twi2,           		0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,    	BUS_GATE2,     	0,         			0,          	18,          	18,              		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(rsb,           		0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,    	BUS_GATE2,     	0,         			0,          	3,          	3,              		0,   				&clk_lock, 	NULL,             0);
//xxw test 20160830//SUNXI_CLK_PERIPH(cir,           		0,               	0,    			0,          		0,                  	0,      		0,          		0,          		0,          		0,          		0,               	BUS_RST2,    	BUS_GATE2,     	0,         			0,          	2,          	2,              		0,   				&clk_lock, 	NULL,             0);
SUNXI_CLK_PERIPH(cir,           		CIR_CFG,               	24,    			2,          		CIR_CFG,                  	0,      		4,          		16,          		2,          		0,          		CIR_CFG,               	BUS_RST2,    	BUS_GATE2,     	0,         			31,          	2,          	2,              		0,   				&clk_lock, 	NULL,             0);
//SUNXI_CLK_PERIPH(cir,     CIR_CFG,     	24,   		2,          		CIR_CFG,     	0,      		4,          		16,         		2,          		0,          		CIR_CFG,     	0,               	0,             		0,         			31,         	0,          	0,              		0,   				&clk_lock, 	NULL,             	0);


struct periph_init_data sunxi_periphs_init[] = {
	{"cpu",            CLK_GET_RATE_NOCACHE, cpu_parents,            ARRAY_SIZE(cpu_parents),            &sunxi_clk_periph_cpu              },
	{"pll_periphahb", CLK_IGNORE_SYNCBOOT,  pll_periphahb_parents, ARRAY_SIZE(pll_periphahb_parents), &sunxi_clk_periph_pll_periphahb   },
	{"ahb1",           0,                    ahb1_parents,           ARRAY_SIZE(ahb1_parents),           &sunxi_clk_periph_ahb1             },
	{"apb1",           0,                    apb1_parents,           ARRAY_SIZE(apb1_parents),           &sunxi_clk_periph_apb1             },

	{"sdmmc0_mod",     0,                    sdmmc_parents,       ARRAY_SIZE(sdmmc_parents),       &sunxi_clk_periph_sdmmc0_mod       },
	{"sdmmc0_bus",     0,                    sdmmc_parents,       ARRAY_SIZE(sdmmc_parents),       &sunxi_clk_periph_sdmmc0_bus       },
	{"sdmmc0_rst",     0,                    sdmmc_parents,       ARRAY_SIZE(sdmmc_parents),       &sunxi_clk_periph_sdmmc0_rst       },
	{"sdmmc1_mod",     0,                    sdmmc_parents,       ARRAY_SIZE(sdmmc_parents),       &sunxi_clk_periph_sdmmc1_mod       },
	{"sdmmc1_bus",     0,                    sdmmc_parents,       ARRAY_SIZE(sdmmc_parents),       &sunxi_clk_periph_sdmmc1_bus       },
	{"sdmmc1_rst",     0,                    sdmmc_parents,       ARRAY_SIZE(sdmmc_parents),       &sunxi_clk_periph_sdmmc1_rst       },
	{"usbphy0", 	   0,					 hosc_parents,			 ARRAY_SIZE(hosc_parents),			 &sunxi_clk_periph_usbphy0			},
	{"usbotg",		   0,					 ahb1mod_parents,		 ARRAY_SIZE(ahb1mod_parents),		 &sunxi_clk_periph_usbotg			},

	{"spi0",           0,                    spi_parents,       ARRAY_SIZE(spi_parents),       &sunxi_clk_periph_spi0             },
	{"spi1",           0,                    spi_parents,       ARRAY_SIZE(spi_parents),       &sunxi_clk_periph_spi1             },

	{"audio",           0,                    audio_parents,            ARRAY_SIZE(audio_parents),            &sunxi_clk_periph_audio             },
	{"avs",           0,                    avs_parents,            ARRAY_SIZE(avs_parents),            &sunxi_clk_periph_avs             },
	{"codec", 		  0,		codec_parents,			ARRAY_SIZE(codec_parents),			&sunxi_clk_periph_codec			  },

	//FIXME: 1663 spec said that spdif's parent are PLL2*8/PLL2*4/PLL2*2/PLL2, but did not found.
	{"spdif",          0,                    audio_parents,            ARRAY_SIZE(audio_parents),            &sunxi_clk_periph_spdif            },

	{"debe",             0,                    de_parents,             ARRAY_SIZE(de_parents),             &sunxi_clk_periph_debe               },
	{"defe",             0,                    de_parents,             ARRAY_SIZE(de_parents),             &sunxi_clk_periph_defe               },
	{"tcon",          0,                    tcon_parents,          ARRAY_SIZE(tcon_parents),          &sunxi_clk_periph_tcon            },
	{"deinterlace",          0,          deinterlace_parents,          ARRAY_SIZE(deinterlace_parents),          &sunxi_clk_periph_deinterlace            },
	{"tve_clk2", 		 0, 		 tve_clk2_parents,		   ARRAY_SIZE(tve_clk2_parents), 		 &sunxi_clk_periph_tve_clk2},
	{"tve_clk1", 		 0, 		 tve_clk1_parents,		   ARRAY_SIZE(tve_clk1_parents), 		 &sunxi_clk_periph_tve_clk1 },
	{"tvd",		 0, 		 tvd_parents,		   ARRAY_SIZE(tvd_parents),		 &sunxi_clk_periph_tvd },
	{"csi_m",          0,                    csi_m_parents,          ARRAY_SIZE(csi_m_parents),          &sunxi_clk_periph_csi_m            },
	{"ve", 		0,		codec_parents,			ARRAY_SIZE(codec_parents),			&sunxi_clk_periph_ve	},

	{"sdram",          0,                    sdram_parents,          ARRAY_SIZE(sdram_parents),          &sunxi_clk_periph_sdram            },
	{"dma",            0,                    ahb1mod_parents,        ARRAY_SIZE(ahb1mod_parents),        &sunxi_clk_periph_dma              },
	{"uart0",          0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_uart0            },
	{"uart1",          0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_uart1            },
	{"uart2",          0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_uart2            },
	{"twi0",           0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_twi0             },
	{"twi1",           0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_twi1             },
	{"twi2",           0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_twi2             },	
	{"rsb",           0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_rsb               },
	//xxw test 20160830{"cir",           0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_cir             },
	{"cir",           0,                    cir_parents,        ARRAY_SIZE(cir_parents),        &sunxi_clk_periph_cir             },
};

void __init sunxi_init_clocks(void)
{
}

#ifdef CONFIG_OF
/**
 * set default rate for clk
 */
static int __set_clk_rates(struct device_node *node , struct clk* clk)
{
	u32 assigned_clock_rates = 0;
	bool res = -1;

	/*set pll default rate here , and make you know it is setted succeed or not*/
	if( !of_property_read_u32( node , "assigned-clock-rates" , &assigned_clock_rates) )
	{
		u32 real_clock_rate = 0;
		clk_set_rate(clk , assigned_clock_rates);
		real_clock_rate = clk_get_rate(clk);
		if( real_clock_rate != assigned_clock_rates )
		{
			pr_info("%s-set_default_rate=%u , but real_get_rate=%u failured!\n" ,
				__clk_get_name(clk) , assigned_clock_rates , real_clock_rate );
		}
		else
		{
			pr_info("%s-set_default_rate=%u success!\n",
				__clk_get_name(clk) , assigned_clock_rates);
			res = 0;
		}
	}

	return res;
}

/**
 * set default clk source for clk
 */
static int __set_clk_parents(struct device_node *node , struct clk* clk)
{
	int index = 0, rc ;
	struct of_phandle_args clkspec;
	struct clk *pclk;

	rc = of_parse_phandle_with_args(node, "assigned-clock-parents",
				"#clock-cells",    index, &clkspec);
	if (rc < 0)
	{
		/* skip empty (null) phandles */
		return rc;
	}

	pclk = of_clk_get_from_provider(&clkspec);
	if (IS_ERR(pclk))
	{
		pr_warn("clk: couldn't get parent clock %d for %s\n",
				index, node->full_name);
		return PTR_ERR(pclk);
	}

	rc = clk_set_parent(clk, pclk);
	if (rc < 0)
	{
		pr_err("%s-set_default_source=%s failed at: %d\n",
			__clk_get_name(clk), __clk_get_name(pclk), rc);
	}
	else
	{
		pr_info("%s-set_default_source=%s success!\n",
			__clk_get_name(clk) , __clk_get_name(pclk) );
	}

	return rc;
}

/**
*of_sunxi_clocks_init() - Clocks initialize
*/
void of_sunxi_clocks_init(struct device_node *node)
{
#ifdef CONFIG_SUNXI_CLK_DUMMY_DEBUG
	sunxi_clk_base = &dummy_reg[0];
	dummy_reg_init();
#else
	sunxi_clk_base = of_iomap(node ,0);
#endif

	/*do some initialize arguments here*/
	sunxi_clk_factor_initlimits();

	/*sunxi_clk_get_factors_ops(&pll_mipi_ops);
	pll_mipi_ops.get_parent = get_parent_pll_mipi;
	pll_mipi_ops.set_parent = set_parent_pll_mipi;
	pll_mipi_ops.enable = clk_enable_pll_mipi;
	pll_mipi_ops.disable = clk_disable_pll_mipi;*/
	pr_info( "%s : sunxi_clk_base[0x%lx]\n", __func__ , (unsigned long)sunxi_clk_base);

	clk_add_alias("pll1",NULL,"pll_cpu",NULL);
	clk_add_alias("pll2",NULL,"pll_audio",NULL);
	clk_add_alias("pll3",NULL,"pll_video",NULL);
	clk_add_alias("pll4",NULL,"pll_ve",NULL);
	clk_add_alias("pll5",NULL,"pll_ddr",NULL);
	clk_add_alias("pll6",NULL,"pll_periph",NULL);
}

void of_sunxi_fixed_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	u32 rate;

	if (of_property_read_u32(node, "clock-frequency", &rate))
		return;

	of_property_read_string(node, "clock-output-names", &clk_name);

	clk = clk_register_fixed_rate(NULL, clk_name, NULL, CLK_IS_ROOT, rate);
	if (!IS_ERR(clk))
	{
		clk_register_clkdev(clk, clk_name, NULL);
		of_clk_add_provider(node, of_clk_src_simple_get, clk);
	}
}

void of_sunxi_fixed_factor_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	const char *parent_name;
	u32 div, mult;

	if (of_property_read_u32(node, "clock-div", &div)) {
		pr_err("%s Fixed factor clock <%s> must have a clock-div property\n",
			__func__, node->name);
		return;
	}

	if (of_property_read_u32(node, "clock-mult", &mult)) {
		pr_err("%s Fixed factor clock <%s> must have a clokc-mult property\n",
			__func__, node->name);
		return;
	}

	of_property_read_string(node, "clock-output-names", &clk_name);
	parent_name = of_clk_get_parent_name(node, 0);

	clk = clk_register_fixed_factor(NULL, clk_name, parent_name, 0,
					mult, div);
	if (!IS_ERR(clk))
	{
		clk_register_clkdev(clk, clk_name, NULL);
		of_clk_add_provider(node, of_clk_src_simple_get, clk);
	}
}

/**
 * of_pll_clk_setup() - Setup function for pll factors clk
 */
void of_pll_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	const char *lock_mode = NULL;
	struct factor_init_data *factor;
	int i;
	int ret;

	of_property_read_string(node, "clock-output-names", &clk_name);
	ret = of_property_read_string(node, "lock-mode", &lock_mode);

	/*get pll clk init config */
	for(i=0; i<ARRAY_SIZE(sunxi_factos); i++)
	{
		factor = &sunxi_factos[i];
		if( 0 == strcmp(clk_name , factor->name) )
		{
			if (IS_ERR_VALUE(ret)) {
				factor->lock_mode = PLL_LOCK_NONE_MODE;
			} else {
				if (strcmp(lock_mode, "new") == 0) {
					factor->lock_mode = PLL_LOCK_NEW_MODE;
				} else if (strcmp(lock_mode, "old") == 0) {
					factor->lock_mode = PLL_LOCK_OLD_MODE;
				} else if (strcmp(lock_mode, "none") == 0) {
					factor->lock_mode = PLL_LOCK_NONE_MODE;
				} else {
					factor->lock_mode = PLL_LOCK_NONE_MODE;
				}
			}

			/*register clk */
			clk = sunxi_clk_register_factors( NULL ,  sunxi_clk_base , &clk_lock , factor );
			/*add to of */
			if (!IS_ERR(clk))
			{
				clk_register_clkdev(clk, clk_name, NULL);
				of_clk_add_provider(node, of_clk_src_simple_get, clk);
				__set_clk_parents(node , clk);
				__set_clk_rates(node , clk);
				/*pr_err( "%s : %s \n", __func__ , clk_name );*/
				return ;
			}

		}
	}

	pr_err("clk %s not found in %s\n",clk_name , __func__ );
}

/**
 * of_periph_clk_setup() - Setup function for periph clk
 */
void of_periph_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	struct periph_init_data *periph;
	unsigned int i;

	of_property_read_string(node, "clock-output-names", &clk_name);

	/*get periph clk init config */
	for(i=0; i<ARRAY_SIZE(sunxi_periphs_init); i++)
	{
		periph = &sunxi_periphs_init[i];
		if( 0 == strcmp(clk_name , periph->name) )
		{
			/*register clk */
			clk = sunxi_clk_register_periph( clk_name , periph->parent_names,
						periph->num_parents , periph->flags , sunxi_clk_base , periph->periph );
			/*add to of */
			if (!IS_ERR(clk))
			{
				clk_register_clkdev(clk, clk_name, NULL);
				of_clk_add_provider(node, of_clk_src_simple_get, clk);
				__set_clk_parents(node , clk);
				__set_clk_rates(node , clk);
				/*pr_err( "%s : %s \n", __func__ , clk_name );*/
				return ;
			}
		}
	}
	pr_err("clk %s not found in %s\n",clk_name , __func__ );
}

CLK_OF_DECLARE(sunxi_clocks_init, "allwinner,sunxi-clk-init", of_sunxi_clocks_init);
CLK_OF_DECLARE(sunxi_fixed_clk, "allwinner,fixed-clock", of_sunxi_fixed_clk_setup);
CLK_OF_DECLARE(pll_clk, "allwinner,sunxi-pll-clock", of_pll_clk_setup);
CLK_OF_DECLARE(sunxi_fixed_factor_clk, "allwinner,fixed-factor-clock", of_sunxi_fixed_factor_clk_setup);
CLK_OF_DECLARE(periph_clk, "allwinner,sunxi-periph-clock", of_periph_clk_setup);
#endif

