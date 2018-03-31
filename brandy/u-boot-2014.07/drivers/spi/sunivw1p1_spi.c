/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/spi.h>
#include <asm/arch/ccmu.h>
//#include <asm/arch/ccmu.h>
#include <asm/arch/dma.h>
#define SUNXI_NOR_FLASH_DEBUG 1
#ifdef  SUNXI_NOR_FLASH_DEBUG
#define SUNXI_DEBUG(fmt,args...)	printf(fmt ,##args)
#else
#define SUNXI_DEBUG(fmt,args...) do {} while(0)
#endif

static u32 g_cfg_mclk = 0;
static sunxi_dma_setting_t *spi_tx_dma;
static sunxi_dma_setting_t *spi_rx_dma;
static  uint  spi_tx_dma_hd;
static  uint  spi_rx_dma_hd;

extern void *malloc_noncache(uint num_bytes);

void ccm_module_disable_bak(u32 clk_id)
{
	switch(clk_id>>8) {
		case AHB1_BUS0:
			clr_wbit(CCMU_BUS_SOFT_RST_REG0, 0x1U<<(clk_id&0xff));
			SUNXI_DEBUG("\nread CCMU_BUS_SOFT_RST_REG0[0x%x]\n",readl(CCMU_BUS_SOFT_RST_REG0));
			break;
	}
}

void ccm_module_enable_bak(u32 clk_id)
{
	switch(clk_id>>8) {
		case AHB1_BUS0:
			set_wbit(CCMU_BUS_SOFT_RST_REG0, 0x1U<<(clk_id&0xff));
			SUNXI_DEBUG("\nread enable CCMU_BUS_SOFT_RST_REG0[0x%x]\n",readl(CCMU_BUS_SOFT_RST_REG0));
			break;
	}
}
void ccm_clock_enable_bak(u32 clk_id)
{
	switch(clk_id>>8) {
		case AHB1_BUS0:
			set_wbit(CCMU_BUS_CLK_GATING_REG0, 0x1U<<(clk_id&0xff));
			SUNXI_DEBUG("read s CCMU_BUS_CLK_GATING_REG0[0x%x]\n",readl(CCMU_BUS_CLK_GATING_REG0));
			break;
	}
}

void ccm_clock_disable_bak(u32 clk_id)
{
	switch(clk_id>>8) {
		case AHB1_BUS0:
			clr_wbit(CCMU_BUS_CLK_GATING_REG0, 0x1U<<(clk_id&0xff));
			SUNXI_DEBUG("read dis CCMU_BUS_CLK_GATING_REG0[0x%x]\n",readl(CCMU_BUS_CLK_GATING_REG0));
			break;
	}
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int spi_dma_recv_start(uint spi_no, uchar* pbuf, uint byte_cnt)
{
	flush_cache((uint)pbuf, byte_cnt);

	sunxi_dma_start(spi_rx_dma_hd, SPI_RXD, (uint)pbuf, byte_cnt);

	return 0;
}
static int spi_wait_dma_recv_over(uint spi_no)
{
	//int ret = 0;
	//count_recv ++;
	//ret = sunxi_dma_querystatus(spi_rx_dma_hd);
	//if(ret == 0)
	//	printf("dma recv end \n");
	//return ret;
	return  sunxi_dma_querystatus(spi_rx_dma_hd);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int spi_dma_send_start(uint spi_no, uchar* pbuf, uint byte_cnt)
{
	flush_cache((uint)pbuf, byte_cnt);

	sunxi_dma_start(spi_tx_dma_hd, (uint)pbuf, SPI_TXD, byte_cnt);

	return 0;
}


#if 0
static int spi_wait_dma_send_over(uint spi_no)
{
	//int ret = 0;
	//count_send++;
	//ret = sunxi_dma_querystatus(spi_tx_dma_hd);
	//if(ret == 0)
	//	printf("dma send end \n");
	//return ret ;
	return sunxi_dma_querystatus(spi_tx_dma_hd);
}
#endif

#define set_wbit(addr, v)   (*((volatile unsigned long  *)(addr)) |=  (unsigned long)(v))
#define clr_wbit(addr, v)   (*((volatile unsigned long  *)(addr)) &= ~(unsigned long)(v))

void ccm_module_reset_bak(u32 clk_id)
{

    __u32 reg_val;

    /* config ahb spi gating clock */
	reg_val  = *(volatile __u32 *)(CCMU_BUS_CLK_GATING_REG0);
	reg_val &= ~(0x1U << (20 +0));
	if (1)
		reg_val |= 0x1U <<( 20 +0);
	*(volatile unsigned int *)(CCMU_BUS_CLK_GATING_REG0) = reg_val;


 	/*  reset ahb spi module */
	reg_val  = *(volatile __u32 *)(CCMU_BUS_SOFT_RST_REG0);
	reg_val &= ~(0x1U << (20 +0));
	if (1)
		reg_val |= 0x1U << (20 +0);
	*(volatile unsigned int *)(CCMU_BUS_SOFT_RST_REG0) = reg_val;

	
}

u32 ccm_get_pll_periph_clk(void)
{
    u32 rval0,rval1,rval2;
	u32 n, k;

	//先写死，后续修改
	
	rval0 = readl(CCM_PLL6_MOD_CTRL);
	n = (0x1f & (rval0 >> 8)) + 1;
	k = (0x3 & (rval0 >> 4)) + 1;
	rval1 = 0;
	rval2 = 0;
	
	return 204000000;
}

void pattern_goto(int pos)
{
	//SUNXI_DEBUG("pos =%d\n",pos);
}

u32 spi_cfg_mclk(u32 spi_no, u32 src, u32 mclk)
{
#ifdef CONFIG_FPGA
	g_cfg_mclk = 24000000;
	return g_cfg_mclk;
#else
    u32 mclk_base = CCM_SPI0_SCLK_CTRL;
	u32 source_clk;
	u32 rval;
	u32 m, n, div;
	src  = 1;

    switch (src) {
	case 0:
		source_clk = 24000000;
		break;
	case 1:
		source_clk = ccm_get_pll_periph_clk();
		break;
	default :
		SUNXI_DEBUG("Wrong SPI clock source :%x\n", src);
	}
	SUNXI_DEBUG("SPI clock source :0x%x\n", source_clk);
	div = (source_clk + mclk - 1) / mclk;
	div = div==0 ? 1 : div;
	if (div > 128) {
		m = 1;
		n = 0;
		SUNXI_DEBUG("Source clock is too high\n");
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
	
	rval = (1U << 31) | (src << 24) | (n << 16) | (m - 1);
	writel(rval, mclk_base);
	g_cfg_mclk = source_clk / (1 << n) / (m - 1);
	SUNXI_DEBUG("spi spic->sclk =0x%x\n",g_cfg_mclk );
	return g_cfg_mclk;
#endif
}

u32 spi_get_mlk(u32 spi_no)
{
	return g_cfg_mclk ;//spicinfo[spi_no].sclk;
}

void spi_gpio_cfg(int spi_no)
{
	writel(0x2222, (0x1c20800 + 0x48));   // PIO SETTING,PortC SPI0
}

void spi_onoff(u32 spi_no, u32 onoff)
{
	u32 clkid[] = {SPI0_CKID, SPI1_CKID};
	//u32 reg_val = 0;
	spi_no = 0;
	switch (spi_no) {
	case 0:
            spi_gpio_cfg(0);	
            break;
	}
	ccm_module_reset_bak(clkid[spi_no]);
//	if (onoff)
//		ccm_clock_enable_bak(clkid[spi_no]);
//	else
//		ccm_clock_disable_bak(clkid[spi_no]);

}

void spic_set_clk(u32 spi_no, u32 clk)
{
	writel((1<<12)|2, SPI_CCR);
}



void sunxi_dma_rx(void *data)
{
	//sunxi_dma_stop(spi_rx_dma_hd);

}

void sunxi_dma_tx(void *data)
{
	//sunxi_dma_stop(spi_tx_dma_hd);

}


int spic_init(u32 spi_no)
{
	u32 rval;
	//uint reg_val, div;
	spi_rx_dma = malloc_noncache(sizeof(sunxi_dma_setting_t));
	spi_tx_dma = malloc_noncache(sizeof(sunxi_dma_setting_t));
	if(!(spi_rx_dma) || !(spi_tx_dma))
	{
		printf("no enough memory to malloc \n");
		return -1;
	}
	memset(spi_tx_dma , 0 , sizeof(sunxi_dma_setting_t));
	memset(spi_rx_dma , 0 , sizeof(sunxi_dma_setting_t));
	spi_rx_dma_hd = sunxi_dma_request(DMAC_DMATYPE_NORMAL);
	spi_tx_dma_hd = sunxi_dma_request(DMAC_DMATYPE_NORMAL);
	

	if((spi_tx_dma_hd == 0) || (spi_rx_dma_hd == 0))
	{
		printf("spi request dma failed\n");

		return -1;
	}


	spi_rx_dma->cfg.src_drq_type     = DMAC_CFG_TYPE_SPI0+spi_no; 
	spi_rx_dma->cfg.src_addr_mode    = DMAC_CFG_SRC_ADDR_TYPE_IO_MODE;
	spi_rx_dma->cfg.src_burst_length = DMAC_CFG_SRC_1_BURST;
	spi_rx_dma->cfg.src_data_width   = DMAC_CFG_SRC_DATA_WIDTH_8BIT;
	spi_rx_dma->cfg.dst_drq_type     = DMAC_CFG_TYPE_DRAM;  
	spi_rx_dma->cfg.dst_addr_mode    = DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE;
	spi_rx_dma->cfg.dst_burst_length = DMAC_CFG_DEST_1_BURST;
	spi_rx_dma->cfg.dst_data_width   = DMAC_CFG_DEST_DATA_WIDTH_8BIT;
	spi_rx_dma->cfg.wait_state       = 3;
	spi_rx_dma->cfg.continuous_mode  = 0;
	


	spi_tx_dma->cfg.src_drq_type     = DMAC_CFG_TYPE_DRAM;  
	spi_tx_dma->cfg.src_addr_mode    = DMAC_CFG_SRC_ADDR_TYPE_LINEAR_MODE;
	spi_tx_dma->cfg.src_burst_length = DMAC_CFG_SRC_1_BURST;
	spi_tx_dma->cfg.src_data_width   = DMAC_CFG_SRC_DATA_WIDTH_8BIT;
	spi_tx_dma->cfg.dst_drq_type     = DMAC_CFG_TYPE_SPI0+spi_no;  
	spi_tx_dma->cfg.dst_addr_mode    = DMAC_CFG_DEST_ADDR_TYPE_IO_MODE;
	spi_tx_dma->cfg.dst_burst_length = DMAC_CFG_DEST_1_BURST;
	spi_tx_dma->cfg.dst_data_width   = DMAC_CFG_DEST_DATA_WIDTH_8BIT;
	spi_tx_dma->cfg.wait_state       = 3;
	spi_tx_dma->cfg.continuous_mode  = 0;
	

	sunxi_dma_setting(spi_rx_dma_hd, (void *)spi_rx_dma);
	sunxi_dma_setting(spi_tx_dma_hd, (void *)spi_tx_dma);

	sunxi_dma_enable_int(spi_rx_dma_hd);
 	sunxi_dma_enable_int(spi_tx_dma_hd);

	sunxi_dma_install_int(spi_rx_dma_hd,sunxi_dma_rx,0);
	sunxi_dma_install_int(spi_tx_dma_hd,sunxi_dma_tx,0);

	
	spi_no = 0;
	spi_onoff(spi_no, 1);
	
	//spi_cfg_mclk(spi_no, SPI_CLK_SRC, SPI_MCLK);
	spic_set_clk(spi_no, SPI_DEFAULT_CLK);
	
	rval = SPI_SOFT_RST|SPI_TXPAUSE_EN|SPI_MASTER|SPI_ENABLE;
	writel(rval, SPI_GCR);
	rval = SPI_SET_SS_1|SPI_DHB|SPI_SS_ACTIVE0|SPI_MODE3;   //set ss to high,discard unused burst,SPI select signal polarity(low,1=idle)
	writel(rval, SPI_TCR);
    writel(SPI_TXFIFO_RST|(SPI_TX_WL<<16)|(SPI_RX_WL), SPI_FCR);
    return 0;    
}

int spic_rw( u32 tcnt, void* txbuf, u32 rcnt, void* rxbuf) 
{
	u32 i = 0;
	u32 timeout = 0xffffff;
	//uint ret = 0;

	u8 *tx_buffer = txbuf ;
	u8 *rx_buffer = rxbuf;


	writel(tcnt+rcnt, SPI_MBC);
	writel(tcnt, SPI_MTC);	
	writel((0<<28)|(0<<24)|tcnt, SPI_BCC);	
	writel((1<<4)|(1<<0), SPI_IER);
	writel((1<<31)|(1<<15)|(0x20<<16)|(0x20<<0),SPI_FCR);
	writel(0xffff, SPI_ISR);//clear status register

	writel(readl(SPI_TCR)|SPI_EXCHANGE, SPI_TCR);
	
	if(tcnt)
	{
		if(tcnt < 8)
    	{
			i = 0;
			while(i < tcnt)
			{
				writeb(*(tx_buffer+i),SPI_TXD);
				i++;		
			}
		}
		else
		{
			writel((readl(SPI_FCR)|SPI_TXDMAREQ_EN), SPI_FCR);
			spi_dma_send_start(0, txbuf, tcnt);
            /* wait DMA finish */
	//		while ((timeout-- > 0) && spi_wait_dma_send_over(0));
	//		if (timeout <= 0)
	//		{
	//			printf("tx wait_dma_send_over fail\n");
	//			return -1;
	//		}
		}
	}


	if(rcnt)
	{
		if(rcnt<8)
		{
			timeout = 0xffffff;
			while(!(readl(SPI_ISR)&(0x1<<12)))//wait transfer complete
			{
				timeout--;
				if (!timeout)
				{
					printf("(rcnt<8) SPI_ISR time_out \n");
					return RET_FALSE;
				}
			}			
			i = 0;
			while(i < rcnt)
			{
				//receive valid data
				*(rx_buffer+i)=readb(SPI_RXD);
				i++;
			}
			
			writel(0xfffff,SPI_ISR);  /* clear  flag */
			writel((0<<4)|(0<<0), SPI_IER); 	/*disable irq*/
			return RET_OK;	
			
		}
		else
		{	
				timeout = 0xffffff;
				writel((readl(SPI_FCR)|SPI_RXDMAREQ_EN), SPI_FCR);
				spi_dma_recv_start(0, rxbuf, rcnt);		
				while ((timeout-- > 0) && spi_wait_dma_recv_over(0));
				if (timeout <= 0)
				{
					printf("tx wait_dma_send_over fail\n");
					return -1;
				}				
		}

	}

		/* wait DMA finish */
	//	timeout = 0xfffff;
	//	while ((timeout-- > 0) && spi_wait_dma_recv_over(0));
	//	if (timeout <= 0)
	//	{
	//		printf("rx wait_dma_recv_over fail\n");
	//		return -1;
	//	}


/*
    timeout = 0xfffff;
	while(!(readl(SPI_ISR)&(0x1<<0)))//wait transfer complete
	{
		timeout--;
		if (!timeout)
		{
			printf("SPI_ISR time_out \n");
			break;
		}
	}

	if (timeout==0)	
			return RET_FAIL;
*/	
	
	timeout = 0xffffff;
	while(!(readl(SPI_ISR)&(0x1<<12)))//wait transfer complete
	{
		timeout--;
		if (!timeout)
		{
			printf("(rcnt>=8) SPI_ISR time_out \n");
			return RET_FALSE;
		}
	}


	writel(0xfffff,SPI_ISR);  /* clear  flag */
	writel((0<<4)|(0<<0), SPI_IER); 	/*disable irq*/
	return RET_OK;

}

int spic_exit(u32 spi_no)
{
	if(spi_tx_dma)
		free(spi_tx_dma);
	if(spi_rx_dma)
		free(spi_rx_dma);
	sunxi_dma_release(spi_tx_dma_hd);
	sunxi_dma_release(spi_rx_dma_hd);
	return 0;
}

