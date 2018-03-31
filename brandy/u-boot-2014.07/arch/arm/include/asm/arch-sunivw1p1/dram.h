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

#ifndef  __dram_head_h__
#define  __dram_head_h__


//#define PLL_DDR_CLK				(120000000)
#define PLL_DDR_CLK				(156000000)


typedef enum __DRAM_TYPE
{
	DRAM_TYPE_SDR   =0,
    DRAM_TYPE_DDR   =1,
    DRAM_TYPE_DDR2  =2,
}__dram_type_e;

typedef enum __DRAM_BNKMOD
{
    DRAM_ACS_INTERLEAVE = 0,
    DRAM_ACS_SEQUENCE   = 1,
} __dram_bnkmod_e;

typedef enum __DRAM_PRIO_LEVEL
{
    DRAM_PRIO_LEVEL_0 = 0,
    DRAM_PRIO_LEVEL_1    ,
    DRAM_PRIO_LEVEL_2    ,
    DRAM_PRIO_LEVEL_3    ,
    DRAM_PRIO_LEVEL_

}__dram_prio_level_e;

/* 需要读取dram数据的设备名称 */
typedef enum __DRAM_DEV
{
    DRAM_DEVICE_NULL = 0,
    DRAM_DEVICE_DEFE,
    DRAM_DEVICE_DEBE,
    DRAM_DEVICE_VE,
    DRAM_DEVICE_AHB0,
    DRAM_DEVICE_AHB1,
    DRAM_DEVICE_DMA,
    DRAM_DEVICE_

}__dram_dev_e;


typedef struct __DRAM_PARA
{
    unsigned int           base;           // dram base address
    unsigned int           size;           // dram size, based on     (unit: MByte)
    unsigned int           clk;            // dram work clock         (unit: MHz)
    unsigned int           access_mode;    // 0: interleave mode 1: sequence mode
    unsigned int           cs_num;         // dram chip count  1: one chip  2: two chip
    unsigned int           ddr8_remap;     // for 8bits data width DDR 0: normal  1: 8bits
    __dram_type_e      type;           // ddr/ddr2/sdr/mddr/lpddr/...
    unsigned int           bwidth;         // dram bus width
    unsigned int           col_width;      // column address width
    unsigned int           row_width;      // row address width
    unsigned int           bank_size;      // dram bank count
    unsigned int           cas;            // dram cas
}__dram_para_t;


#if 0

typedef struct _boot_dram_para_t
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;		//dram_type			DDR2: 2				DDR3: 3				LPDDR2: 6	DDR3L: 31
    unsigned int        dram_zq;
    unsigned int		dram_odt_en;

	//control configuration
	unsigned int		dram_para1;
    unsigned int		dram_para2;

	//timing configuration
	unsigned int		dram_mr0;
    unsigned int		dram_mr1;
    unsigned int		dram_mr2;
    unsigned int		dram_mr3;
    unsigned int		dram_tpr0;
    unsigned int		dram_tpr1;
    unsigned int		dram_tpr2;
    unsigned int		dram_tpr3;
    unsigned int		dram_tpr4;
    unsigned int		dram_tpr5;
   	unsigned int		dram_tpr6;

    //reserved for future use
    unsigned int		dram_tpr7;
    unsigned int		dram_tpr8;
    unsigned int		dram_tpr9;
    unsigned int		dram_tpr10;
    unsigned int		dram_tpr11;
    unsigned int		dram_tpr12;
    unsigned int		dram_tpr13;

}__dram_para_t;
#endif

extern  int mctl_init(void *para);
extern int init_DRAM( int type, __dram_para_t *buff );
#endif


