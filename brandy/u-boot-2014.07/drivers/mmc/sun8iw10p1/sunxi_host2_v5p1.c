/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * sunxi_host2_v5p1.c
 * Description: MMC  driver for  mmc2 controller operations of sun8iw10p1 
 * Author: ZhengLei
 * Date: 2015/7/24 20:32:15
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/timer.h>
#include <malloc.h>
#include <mmc.h>
#include "../mmc_def.h"
#include "../sunxi_mmc.h"
#include "sunxi_host2_v5p1.h"


#undef readl
#define  readl(a)   *(volatile uint *)(ulong)(a)
 
#undef writel
#define  writel(v, c) *(volatile uint *)(ulong)(c) = (v)


extern struct sunxi_mmc_host mmc_host[4];

static int mmc2_config_clock(struct mmc *mmc,unsigned clk);
static void mmc2_set_ddr_mode(struct mmc *mmc);
static void mmc2_v5p1_save_regs(struct mmc *mmc, void *p);
static void mmc2_v5p1_restore_regs(struct mmc *mmc, void *p);
static void mmc2_v5p1_module_reset(struct mmc *mmc);
static int mmc2_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data);
static int mmc2_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data);

static struct mmc2_reg_v5p1 reg_v5p1_bak;


static int mmc2_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	u32 rval = 0;

	writel(0xffffffff,&reg->int_sta);		/*clear all interrupt*/
	writel(0xffffffff,&reg->int_sta_en);		/*enable all interrupt status*/
	writel(0x00000000,&reg->int_sig_en);
	writel(0xffffffff,&reg->to_ctrl);		/*configure timeout*/
	
	/* enable write/read stop clock at block gap, bit8 read, bit9 write */
	rval  = readl(&reg->ctrl3);
	rval |= (0x3<<8);
	writel(rval,&reg->ctrl3);

	return 0;	
}

static unsigned mmc2_get_mclk(struct sunxi_mmc_host* mmchost)
{
	unsigned n, m, src, sclk_hz = 0;
	unsigned rval = readl(mmchost->mclkbase);

	m = rval & 0xf;
	n = (rval>>16) & 0x3;
	src = (rval>>24) & 0x3;

	if (src == 0)
		sclk_hz = 24000000;
	else if (src == 1)
		//sclk_hz = _get_pll_periph0()*2*1000000; /* use 2x pll6 */
		sclk_hz = sunxi_clock_get_pll6()*2*1000000; /* use 2x pll6 */
	else if (src == 2) {
		/*todo*/
		MMCINFO("%s: don't support parse current clock source %d now !!!\n",__func__, src);
	} else {
		MMCINFO("%s: wrong clock source %d\n",__func__, src);
	}

	return (sclk_hz / (1<<n) / (m+1) );
}

#ifndef FPGA_PLATFORM
static int mmc_set_mclk(struct sunxi_mmc_host* mmchost, u32 clk_hz)
{
	unsigned n, m, div, src, sclk_hz = 0;
	unsigned rval;

	MMCDBG("%s: mod_clk %d\n", __FUNCTION__, clk_hz);

	if (clk_hz <= 4000000) { //400000
		src = 0;
		sclk_hz = 24000000;
	} else {
		src = 1;
		sclk_hz = sunxi_clock_get_pll6()*2*1000000; /*use 2x pll-per0 clock */
	}

	div = (2 * sclk_hz + clk_hz) / (2 * clk_hz);
	div = (div==0) ? 1 : div;
	if (div > 128) {
		m = 1;
		n = 0;
		MMCINFO("%s: source clock is too high, clk %d, src %d!!!\n",
			__FUNCTION__, clk_hz, sclk_hz);
	} else if (div > 64) {
		n = 3;
		m = div >> 3;
	} else if (div > 32) {
		n = 2;
		m = div >> 2;
	} else if (div > 16) {
		n = 1;
		m = div >> 1;
	} else {
		n = 0;
		m = div;
	}

	//rval = (1U << 31) | (src << 24) | (n << 16) | (m - 1);
	rval = (src << 24) | (n << 16) | (m - 1);
	writel(rval, mmchost->mclkbase);

	return 0;
}
#endif 

static int mmc2_config_clock(struct mmc *mmc,unsigned clk)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host* )mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	u32 rval = 0;

	if ((mmc->speed_mode == HSDDR52_DDR50) || (mmc->speed_mode == HS400)) {
//		if (clk > mmc->f_max_ddr)
//			clk = mmc->f_max_ddr;
	}

	/* disable card clock */
	rval = readl(&reg->rst_clk_ctrl);
	rval &= ~(1U << 2);
	writel(rval,&reg->rst_clk_ctrl);

	/* configure clock */
	if (mmc->speed_mode == HSDDR52_DDR50)
		mmchost->mod_clk = clk <<3;
	else
		mmchost->mod_clk = clk <<2;

#ifndef FPGA_PLATFORM
	mmc_set_mclk(mmchost, mmchost->mod_clk);
#endif 

	/* get mclk */
	if (mmc->speed_mode == HSDDR52_DDR50)
		mmc->clock = mmc2_get_mclk(mmchost) >>3;
	else
		mmc->clock = mmc2_get_mclk(mmchost) >>2;

	mmchost->clock = mmc->clock; /* bankup current clock frequency at host */
	MMCDBG("get round card clk %d, mod_clk %d\n", mmc->clock, mmchost->mod_clk);

	/* re-enable mclk */
	writel(readl(mmchost->mclkbase)|(1<<31), mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));


	/* Re-enable card clock */
	rval = readl(&reg->rst_clk_ctrl);
	rval |= (1U << 2);
	writel(rval,&reg->rst_clk_ctrl);

	/* configure delay for current frequency and speed mode */
	/*
		TODO...
	*/

	return 0;	
}
static void mmc2_set_ddr_mode(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	u32 rval = reg->acmd_err_ctrl2;

	rval &= ~(0x7U<<16);
	if (mmc->speed_mode == HSDDR52_DDR50) {
		writel(0x50200000,&reg->atc);
		rval |= (0x4U<<16);
	} else if (mmc->speed_mode == HS400) {
		writel(0x30200000,&reg->atc);
		rval |= (0x5U<<16);
	} else {
		writel(0x30200000,&reg->atc);
	}
	writel(rval,&reg->acmd_err_ctrl2);
}
static void mmc2_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	u32 rval = 0;
	
	/* change clock */
	MMCDBG("mmc %d ios: bus: %d, clock: %d, speed mode: %d\n", \
		mmchost->mmc_no,mmc->bus_width, mmc->clock, mmc->speed_mode);
	/* change clock */
	if (mmc->clock && mmc2_config_clock(mmc, mmc->clock)) { //((struct sunxi_mmc_host*)mmc->priv, mmc->clock)) {
		MMCINFO("[mmc]: mmc %d update clock failed\n",mmchost->mmc_no);
		mmchost->fatal_err = 1;
		return;
	}
	
	/* change bus width */
	rval = readl(&reg->ctrl1);
	
	if (mmc->bus_width == 8) {
		rval &= ~(0x1U<<1);
		rval |= 0x1U<<5;
	} else if (mmc->bus_width == 4) {
		rval |= 0x1U<<1;
		rval &= ~(0x1<<5);
	} else {
		rval &= ~(0x1U<<1);
		rval &= ~(0x1U<<5);
	}
	writel(rval,&reg->ctrl1);
	
	/* set ddr mode*/
	mmc2_set_ddr_mode(mmc);

}

static int mmc2_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)

{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	unsigned int timeout = 0;
	int error = 0;
	unsigned int status = 0;
	unsigned int usedma = 0;
	unsigned int bytecnt = 0;
	u32 cmd_attr0 = ((cmd->cmdidx&0x3f) << 24);
	u32 cmd_attr1 = 0;

	if (mmchost->fatal_err) {
		MMCINFO("mmc %d Found fatal err,so no send cmd\n", mmchost ->mmc_no);
		return -1;
	}
	if (cmd->resp_type & MMC_RSP_BUSY)
		MMCDBG("mmc %d cmd %d check rsp busy\n", mmchost->mmc_no, cmd->cmdidx);

	if (cmd->cmdidx == 12)  
		return 0;

#ifdef MMC_TRANS_BY_DMA
	/* dma's descriptors are allocate at dram(0x42000000). smhc's dma only support descriptors and 
	data buffer at dram  */
	if(data && ((u32)(data->dest) > 0x40000000))
		usedma = 1;
#endif 
	
	/* wait command line idle */
	timeout = 0xff;
	while ((--timeout>0) && (reg->status&CmdInhibitCmd)) ;
	if (timeout == 0) {
		MMCINFO("wait Command Inhibit(CMD) timeout!\n");
		error = -1;
		goto OUT;
	}

	if (!cmd->cmdidx)
		cmd_attr1 |= SendInitSeq;

	if (cmd->resp_type & MMC_RSP_PRESENT)
	{
		if (cmd->resp_type & MMC_RSP_136)
			cmd_attr0 |= Rsp136;
		else if (cmd->resp_type & MMC_RSP_BUSY)
			cmd_attr0 |= Rsp48b;
		else
			cmd_attr0 |= Rsp48;

		if (cmd->resp_type & MMC_RSP_CRC)
			cmd_attr0 |= CheckRspCRC; /* response crc */
		if (cmd->resp_type & MMC_RSP_OPCODE)
			cmd_attr0 |= CheckRspIdx; /* response cmd index */
	}

	if (data)
	{
		/* wait data line idle */
		timeout = 0xff;
		while ((--timeout>0) && (reg->status&CmdInhibitDat)) ;
		if (timeout == 0) {
			MMCINFO("wait Command Inhibit(DAT) timeout!\n");
			error = -1;
			goto OUT;
		}

		if ((u32)data->dest & 0x3) {
			MMCINFO("mmc %d dest is not 4 byte align\n",mmchost ->mmc_no);
			error = -1;
			goto OUT;
		}

		cmd_attr0 |= DataExp;

		if (data->flags & MMC_DATA_READ)
			cmd_attr0 |= Read;

		if (usedma)
			cmd_attr0 |= DMAEn;

 		if (data->blocks > 1) {
			cmd_attr0 |= MultiBlkTrans;

			/* make sure auto cmd12 arg is 0 to avoid to be considered as HPI cmd by some emmc that support HPI  */
			cmd_attr0 |= AutoCmd12;
			reg->cmd_arg2 = 0x0;
		}

		reg->blk_cfg = ((data->blocks&0xffff)<<16) | (data->blocksize&0xfff);
	}

	reg->int_sta = 0xffffffff; /* clear int status */
	reg->cmd_arg1= cmd->cmdarg;
	reg->cmd_attr = cmd_attr1;

	MMCDBG("mmc %d, cmd %d(0x%x), arg 0x%x\n", mmchost->mmc_no, cmd->cmdidx,
		cmd_attr0, cmd->cmdarg);

	/* send cmd */
	if (!data)
	{
		reg->cmd = cmd_attr0;
	}
	else
	{
		int ret = 0;
		bytecnt = data->blocksize * data->blocks;
		MMCDBG("mmc %d trans data %d bytes\n",mmchost->mmc_no, bytecnt);
		if (usedma) 
		{	
			ret = mmc2_trans_data_by_dma(mmc, data);
			reg->cmd = cmd_attr0;
		}
		else
		{
			reg->cmd = cmd_attr0;
			ret = mmc2_trans_data_by_cpu(mmc, data);
		}

		if (ret) {
			MMCINFO("mmc %d Transfer failed\n", mmchost->mmc_no);
			error = reg->int_sta & 0x437f0000;
			if(!error)
				error = 0xffffffff;
			goto OUT;
		}
	}

	timeout = 0xffffff;
	do {
		status = reg->int_sta;
		if (!timeout-- || (status & 0x437f0000)) {
			error = status & 0x437f0000;
			if (!error)
				error = 0xffffffff; //represet software timeout
			MMCINFO("mmc %d cmd %d timeout, err %x\n",mmchost->mmc_no, cmd->cmdidx, error);
			goto OUT;
		}
	} while (!(status&CmdOverInt));

	if (data) {
		timeout = usedma ? 0xffff*bytecnt : 0xffff;

		do {
			status = reg->int_sta;
			if (!timeout-- || (status & 0x437f0000)) {
				error = status & 0x437f0000;
				if(!error)
					error = 0xffffffff;//represet software timeout
				MMCINFO("mmc %d data timeout, err %x\n",mmchost->mmc_no, error);
				goto OUT;
			}
		} while (!(status&TransOverInt));

		if (usedma) {
			timeout = 0xffffff;
			MMCDBG("mmc %d cacl rd dma timeout %x\n", mmchost->mmc_no, timeout);
			do {
				status = reg->int_sta;
				if (!timeout-- || (status & 0x437f0000)) {
					error = status & 0x437f0000;
					if(!error)
						error = 0xffffffff; //represet software timeout
					MMCINFO("mmc %d wait dma over err %x, adma err %x, current des 0x%x 0x%x\n",
						mmchost->mmc_no, error, reg->adma_err, reg->cddlw, reg->cddhw);
					goto OUT;
				}
			} while (!(status&DmaInt));
		}
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		timeout = 0x4ffffff;
		do {
			status = reg->status;
			if (!timeout--) {
				error = -1;
				MMCINFO("mmc %d busy timeout\n", mmchost->mmc_no);
				goto OUT;
			}
		} while (!(status&Dat0LineSta));
	}

	if (cmd->resp_type & MMC_RSP_136) {
		/* Response Field[127:8] <-> RESP[119:0] */
		cmd->response[0] = ((reg->resp3 & 0xffffff) << 8);
		cmd->response[0] |= ((reg->resp2 >>24) & 0xff);
		cmd->response[1] = ((reg->resp2 & 0xffffff) << 8);
		cmd->response[1] |= ((reg->resp1 >>24) & 0xff);
		cmd->response[2] = ((reg->resp1 & 0xffffff) << 8);
		cmd->response[2] |= ((reg->resp0 >>24) & 0xff);
		cmd->response[3] = ((reg->resp0 & 0xffffff) << 8);

		MMCDBG("mmc %d resp 0x%x 0x%x 0x%x 0x%x\n", mmchost->mmc_no,
			cmd->response[3], cmd->response[2],
			cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = reg->resp0;
		MMCDBG("mmc %d resp 0x%x\n", mmchost->mmc_no, cmd->response[0]);
	}


OUT:

	if (error) {
		mmc2_v5p1_save_regs(mmc, &reg_v5p1_bak);
		mmc2_v5p1_module_reset(mmc);
		mmc2_v5p1_restore_regs(mmc, &reg_v5p1_bak);
		MMCINFO("mmc %d cmd %d err %x\n",mmchost->mmc_no, cmd->cmdidx, error);
	}
	
	reg->int_sta = 0xffffffff; /*clear all interrupt*/
	reg->int_sta_en = 0xffffffff; 

	if (error)
		return -1;
	else
		return 0;
}

static void mmc2_v5p1_save_regs(struct mmc *mmc, void *p)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1* reg = (struct mmc2_reg_v5p1*)mmchost->reg;
	struct mmc2_reg_v5p1* reg_bak = (struct mmc2_reg_v5p1 *)p;

	memset(p, 0x0, sizeof(struct mmc2_reg_v5p1));

	/* don't save all registers */

	//reg_bak->cmd_arg2       = reg->cmd_arg2       ;
	//reg_bak->blk_cfg        = reg->blk_cfg        ;
	//reg_bak->cmd_arg1       = reg->cmd_arg1       ;
	//reg_bak->cmd            = reg->cmd            ;
	//reg_bak->resp0          = reg->resp0          ;
	//reg_bak->resp1          = reg->resp1          ;
	//reg_bak->resp2          = reg->resp2          ;
	//reg_bak->resp3          = reg->resp3          ;
	//reg_bak->buff           = reg->buff           ;
	//reg_bak->status         = reg->status         ;
	reg_bak->ctrl1          = reg->ctrl1          ;
	reg_bak->rst_clk_ctrl   = reg->rst_clk_ctrl   ;
	//reg_bak->int_sta        = reg->int_sta        ;
	reg_bak->int_sta_en     = reg->int_sta_en     ;
	reg_bak->int_sig_en     = reg->int_sig_en     ;
	reg_bak->acmd_err_ctrl2 = reg->acmd_err_ctrl2 ;
	//reg_bak->res_0[4]       = reg->res_0[4]       ;
	//reg_bak->set_err        = reg->set_err        ;
	//reg_bak->adma_err       = reg->adma_err       ;
	//reg_bak->adma_addr      = reg->adma_addr      ;
	//reg_bak->res_1[105]     = reg->res_1[105]     ;
	reg_bak->ctrl3          = reg->ctrl3          ;
	//reg_bak->cmd_attr       = reg->cmd_attr       ;
	reg_bak->to_ctrl        = reg->to_ctrl        ;
	reg_bak->thld           = reg->thld           ;
	reg_bak->atc            = reg->atc            ;
	reg_bak->rtc            = reg->rtc            ;
	reg_bak->ditc0          = reg->ditc0          ;
	reg_bak->ditc1          = reg->ditc1          ;
	reg_bak->tp0            = reg->tp0            ;
	reg_bak->tp1            = reg->tp1            ;
	//reg_bak->res_2[2]       = reg->res_2[2]       ;
	reg_bak->ds_dly         = reg->ds_dly         ;
	//reg_bak->res_3[3]       = reg->res_3[3]       ;
	//reg_bak->crc_sta        = reg->crc_sta        ;
	//reg_bak->tbc0           = reg->tbc0           ;
	//reg_bak->tbc1           = reg->tbc1           ;
	//reg_bak->buff_lvl       = reg->buff_lvl       ;

}

static void mmc2_v5p1_restore_regs(struct mmc *mmc, void *p)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1* reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	struct mmc2_reg_v5p1* reg_bak = (struct mmc2_reg_v5p1 *)p;

	/* don't restore all registers */

	//reg->cmd_arg2       = reg_bak->cmd_arg2       ;
	//reg->blk_cfg        = reg_bak->blk_cfg        ;
	//reg->cmd_arg1       = reg_bak->cmd_arg1       ;
	//reg->cmd            = reg_bak->cmd            ;
	//reg->resp0          = reg_bak->resp0          ;
	//reg->resp1          = reg_bak->resp1          ;
	//reg->resp2          = reg_bak->resp2          ;
	//reg->resp3          = reg_bak->resp3          ;
	//reg->buff           = reg_bak->buff           ;
	//reg->status         = reg_bak->status         ;
	reg->ctrl1          = reg_bak->ctrl1          ;
	reg->rst_clk_ctrl   = reg_bak->rst_clk_ctrl   ;
	//reg->int_sta        = reg_bak->int_sta        ;
	reg->int_sta_en     = reg_bak->int_sta_en     ;
	reg->int_sig_en     = reg_bak->int_sig_en     ;
	reg->acmd_err_ctrl2 = reg_bak->acmd_err_ctrl2 ;
	//reg->res_0[4]       = reg_bak->res_0[4]       ;
	//reg->set_err        = reg_bak->set_err        ;
	//reg->adma_err       = reg_bak->adma_err       ;
	//reg->adma_addr      = reg_bak->adma_addr      ;
	//reg->res_1[105]     = reg_bak->res_1[105]     ;
	reg->ctrl3          = reg_bak->ctrl3          ;
	//reg->cmd_attr       = reg_bak->cmd_attr       ;
	reg->to_ctrl        = reg_bak->to_ctrl        ;
	reg->thld           = reg_bak->thld           ;
	reg->atc            = reg_bak->atc            ;
	reg->rtc            = reg_bak->rtc            ;
	reg->ditc0          = reg_bak->ditc0          ;
	reg->ditc1          = reg_bak->ditc1          ;
	reg->tp0            = reg_bak->tp0            ;
	reg->tp1            = reg_bak->tp1            ;
	//reg->res_2[2]       = reg_bak->res_2[2]       ;
	reg->ds_dly         = reg_bak->ds_dly         ;
	//reg->res_3[3]       = reg_bak->res_3[3]       ;
	//reg->crc_sta        = reg_bak->crc_sta        ;
	//reg->tbc0           = reg_bak->tbc0           ;
	//reg->tbc1           = reg_bak->tbc1           ;
	//reg->buff_lvl       = reg_bak->buff_lvl       ;
}

static void mmc2_v5p1_module_reset(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 timeout = 0xff;

	mmchost->hclkbase &= (~(1U<<10)); //only for smch2
	mmchost->hclkrst  &= (~(1U<<10));

	timeout = 0xff;
	while (--timeout>0);

	mmchost->hclkbase |= (1U<<10); //only for smch2
	mmchost->hclkrst  |= (1U<<10);
}
static int mmc2_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	unsigned i;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned *buff;
	unsigned timeout = 0xffffff;
	unsigned fifo_level = 0, fifo_level_leftover = 0;

	/* use ahb to access fifo */
	reg->ctrl3 |= CPUAcessBuffEn;

	if (data->flags & MMC_DATA_READ) {
		buff = (unsigned int *)data->dest;
		i = 0;
		while (i < (byte_cnt>>2)) {//word
			if(reg->status & BuffRDEn) {
				fifo_level = reg->buff_lvl & 0x1ff;
				while (fifo_level--) {
					buff[i++] = *(volatile u32*)mmchost->database;
					timeout = 0xffffff;
				}
			}
			if (!timeout--) {
				goto out;
			}
		}
	} else {
		buff = (unsigned int *)data->src;
		i = 0;
		while (i < (byte_cnt>>2)) {//word
			if (reg->status & BuffWREn) {
				fifo_level_leftover = FIFO_DEPTH_WORD - (reg->buff_lvl & 0x1ff);
				while (fifo_level_leftover--) {
					*(volatile u32*)mmchost->database = buff[i++];
					timeout = 0xffffff;
				}
			}
			if (!timeout--) {
				goto out;
			}
		}
	}

out:
	/* use dma to access fifo */
	reg->ctrl3 &= (~CPUAcessBuffEn);

	if (timeout <= 0){
		MMCINFO("mmc %d transfer by cpu failed\n",mmchost ->mmc_no);
		return -1;
	}

	return 0;
}

static int mmc2_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc2_reg_v5p1 *reg = (struct mmc2_reg_v5p1 *)mmchost->reg;
	struct mmc2_des_v5p1 *pdes = NULL;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned char *buff;
	unsigned des_idx = 0;
	unsigned buff_frag_num = 0;
	unsigned remain;
	unsigned i, rval;
	int ret = 0;

	buff = data->flags & MMC_DATA_READ ?
			(unsigned char *)data->dest : (unsigned char *)data->src;
	buff_frag_num = byte_cnt >> SMHC2_V5P1_DES_NUM_SHIFT;
	remain = byte_cnt & (SMHC2_V5P1_DES_BUFFER_MAX_LEN-1);
	
	if (remain)
		buff_frag_num ++;
	else
		remain = SMHC2_V5P1_DES_BUFFER_MAX_LEN;

	pdes = (struct mmc2_des_v5p1 *)mmchost->pdes;

	for (i=0; i < buff_frag_num; i++, des_idx++) {
		memset((void*)&pdes[des_idx], 0, sizeof(struct mmc2_reg_v5p1));
		pdes[des_idx].valid = 1;
		pdes[des_idx].act = ACT_TRANS;

		if (buff_frag_num > 1 && i != buff_frag_num-1) {
			pdes[des_idx].length = SMHC2_V5P1_DES_BUFFER_MAX_LEN;
		} else
			pdes[des_idx].length = remain;

		pdes[des_idx].addr = (u32)buff + i * SMHC2_V5P1_DES_BUFFER_MAX_LEN;

		if (i == buff_frag_num-1) {
			pdes[des_idx].end = 1;
			pdes[des_idx].int_en = 1;
		}
		MMCDBG("mmc %d frag %d, remain %d, des[%d](%x): "
			"[0] = %x, [1] = %x\n",mmchost ->mmc_no,
			i, remain, des_idx, (u32)&pdes[des_idx],
			(u32)((u32*)&pdes[des_idx])[0], (u32)((u32*)&pdes[des_idx])[1]);
	}

	__asm("DSB");
	__asm("ISB");

	/* use dma to access fifo */
	reg->ctrl3 &= (~CPUAcessBuffEn);

	/* select dma type */
	rval = reg->ctrl1;
	rval &= ~(0x3U<<3);
	rval |= 0x3U<<3;
	reg->ctrl1 = rval;

	/* set dma desc list base addres */
	reg->adma_addr = (u32)mmchost->pdes;

#if 0
	/* reset dma. it will disable some interrupt status enable, including TransOverInt... */
	if (mmc_reset(mmc, ResetDat)) {
		mmcinfo("reset data path error!\n");
		ret = -1;
	}
#endif
	return ret;
}



const struct mmc_ops sunxi_mmc2_ops = {
	.send_cmd	= mmc2_send_cmd,
	.set_ios	= mmc2_set_ios,
	.init		= mmc2_core_init,
	//.decide_retry = sunxi_decide_rty,
	//.get_detail_errno = sunxi_detail_errno,
	//.update_phase = mmc_update_phase,
	.update_phase  = NULL,
};



