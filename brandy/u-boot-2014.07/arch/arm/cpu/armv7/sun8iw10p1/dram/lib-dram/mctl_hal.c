//*****************************************************************************
//	Allwinner Technology, All Right Reserved. 2006-2010 Copyright (c)
//
//
//	File: 				mctl_hal.c
//
//	Description:  This file implements basic functions for AW1699 DRAM controller
//		 
//	History:
//        2015/07/01    lhk			0.10	Initial version
//				2015/08/04		lhk			0.20	add standby code
//*****************************************************************************

#include "mctl_reg.h"
#include "mctl_hal.h"

#define STANDBY_FPGA

#ifdef STANDBY_FPGA
/********************************************************************************
 *FPGA boot code
 ********************************************************************************/
void local_delay (unsigned int n)
{
	while(n--);
}
//***********************************************************************************************
//	unsigned int mctl_soft_training(void)
//
//  Description:	try FPGA read pipe
//
//	Arguments:		None
//
//	Return Value:	None
//***********************************************************************************************
unsigned int mctl_soft_training(void)
{
#ifndef DRAM_FPGA_HALF_DQ_TEST
	int i, j;
	unsigned int k;
	unsigned int delay[4];
	const unsigned int words[64] = {	0x12345678, 0xaaaaaaaa, 0x55555555, 0x00000000, 0x11223344, 0xffffffff, 0x55aaaa55, 0xaa5555aa,
								0x23456789, 0x18481113, 0x01561212, 0x12156156, 0x32564661, 0x61532544, 0x62658451, 0x15564795,
								0x10234567, 0x54515152, 0x33333333, 0xcccccccc, 0x33cccc33, 0x3c3c3c3c, 0x69696969, 0x15246412,
								0x56324789, 0x55668899, 0x99887744, 0x00000000, 0x33669988, 0x66554477, 0x5555aaaa, 0x54546212,
								0x21465854, 0x66998877, 0xf0f0f0f0, 0x0f0f0f0f, 0x77777777, 0xeeeeeeee, 0x3333cccc, 0x52465621,
								0x24985463, 0x22335599, 0x78945623, 0xff00ff00, 0x00ff00ff, 0x55aa55aa, 0x66996699, 0x66544215,
								0x54484653, 0x66558877, 0x36925814, 0x58694712, 0x11223344, 0xffffffff, 0x96969696, 0x65448861,
								0x48898111, 0x22558833, 0x69584701, 0x56874123, 0x11223344, 0xffffffff, 0x99669966, 0x36544551};

	for(i=0;i<4;i++)
		delay[i]=0;
	for(i=0; i<0x10; i++)
		{
			for(j=0; j<0x4; j++)
			{
				mctl_write_w(((3-j)<<20)|((0xf-i)<<16)|0x400f,MCTL_CTL_BASE+0xc);
				for(k=0; k<0x10; k++);
				for(k=0; k<(1<<10); k++)
				{
					mctl_write_w(words[k%64],DRAM_BASE_ADDR+(k<<2));
				}

				for(k=0; k<(1<<10); k++)
				{
					if(words[k%64] != mctl_read_w(DRAM_BASE_ADDR+(k<<2)))
					break;
				}

				if(k==(1<<10))
				{
					delay[j]=((3-j)<<20)|((0xf-i)<<16)|0x400f;
				}
			}
		}

	if(delay[0]!=0)
	{
		mctl_write_w(delay[0],MCTL_CTL_BASE+0xc);
	}
	else if(delay[1]!=0)
	{
		mctl_write_w(delay[1],MCTL_CTL_BASE+0xc);
	}
	else if(delay[2]!=0)
	{
		mctl_write_w(delay[2],MCTL_CTL_BASE+0xc);
	}
	else if(delay[3]!=0)
	{
		mctl_write_w(delay[3],MCTL_CTL_BASE+0xc);
	}

		return 1;
#else
		mctl_write_w(0x27400f,MCTL_CTL_BASE+0xc);
		return 1;
#endif
}

//***********************************************************************************************
//	unsigned int mctl_sys_init(__dram_para_t *para)
//
//  Description:	set pll and dram clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
unsigned int mctl_sys_init(__dram_para_t *para)
{

	unsigned int reg_val = 0;
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);

	mctl_write_w(0x25ffff,CLKEN);

	return 0;
}

unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	unsigned int reg_val;
#if 1       //DDR2---col 10,row 14,bank 3,rank 1
	mctl_write_w(0x4219D5,MC_WORK_MODE);// 0x0x4219D5--map0 32bit //0x0x4299D5--map1 32bit
#ifdef DRAM_FPGA_HALF_DQ_TEST
	mctl_write_w(0x4208D5,MC_WORK_MODE);//map0;default 0x4208D5--16bit
#endif
	mctl_write_w(mctl_read_w(_CCM_DRAMCLK_CFG_REG)|(0x1U<<31),_CCM_DRAMCLK_CFG_REG);
	mctl_write_w(0x00070005,RFSHTMG);
	mctl_write_w(0xa63,DRAM_MR0);
	mctl_write_w(0x00,DRAM_MR1);
	mctl_write_w(0,DRAM_MR2);
	mctl_write_w(0,DRAM_MR3);
	mctl_write_w(0x01e007c3,PTR0);
	//mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	mctl_write_w(0x01000500,PTR4);
	mctl_write_w(0x01000081,DTCR);
	mctl_write_w(0x03808620,PGCR1);
	mctl_write_w(0x02010101,PITMG0);
	mctl_write_w(0x06021b02,DRAMTMG0);
	mctl_write_w(0x00020102,DRAMTMG1);
	mctl_write_w(0x03030306,DRAMTMG2);
	mctl_write_w(0x00002006,DRAMTMG3);
	mctl_write_w(0x01020101,DRAMTMG4);
	mctl_write_w(0x05010302,DRAMTMG5);
#else  //DDR3
	mctl_write_w(0x004318e4,MC_WORK_MODE);
	mctl_write_w(mctl_read_w(_CCM_DRAMCLK_CFG_REG)|(0x1U<<31),_CCM_DRAMCLK_CFG_REG);
	mctl_write_w(0x00070005,RFSHTMG);
	mctl_write_w(0x420,DRAM_MR0);
	mctl_write_w(0,DRAM_MR1);
	mctl_write_w(0,DRAM_MR2);
	mctl_write_w(0,DRAM_MR3);
	mctl_write_w(0x01e007c3,PTR0);
	mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	mctl_write_w(0x01000500,PTR4);
	mctl_write_w(0x01000081,DTCR);
	mctl_write_w(0x03808620,PGCR1);
	mctl_write_w(0x02010101,PITMG0);
	mctl_write_w(0x0b091b0b,DRAMTMG0);
	mctl_write_w(0x00040310,DRAMTMG1);
	mctl_write_w(0x03030308,DRAMTMG2);
	mctl_write_w(0x00002007,DRAMTMG3);
	mctl_write_w(0x04020204,DRAMTMG4);
	mctl_write_w(0x05050403,DRAMTMG5);
	reg_val = mctl_read_w(CLKEN);
	reg_val |= (0x3<<20);
	mctl_write_w(reg_val,CLKEN);
#endif


	reg_val = 0x000183;		//PLL enable, PLL6 should be dram_clk/2
	mctl_write_w(reg_val,PIR);	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);	//for fast simulation
	while((mctl_read_w(STATR )&0x1) != 0x1);	//init done

	reg_val = mctl_read_w(MC_CCCR);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,MC_CCCR);
	local_delay(20);

	mctl_write_w(0x00aa0060,PGCR3);//

	reg_val = mctl_read_w(RFSHCTL0);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,RFSHCTL0);
	local_delay(200);
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val&=~(0x1U<<31);
	mctl_write_w(reg_val,RFSHCTL0);
	local_delay(200);

	reg_val = mctl_read_w(MC_CCCR);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,MC_CCCR);
	local_delay(20);

	return (mctl_soft_training());

}



/*
**********************************************************************************************************************
*                                               GET DRAM SIZE
*
* Description: Get DRAM Size in MB unit;
*
* Arguments  : None
*
* Returns    : 32/64/128/...
*
* Notes      :
*
**********************************************************************************************************************
*/
unsigned int DRAMC_get_dram_size(void)
{
	unsigned int reg_val;
	unsigned int dram_size0,dram_size1 = 0;
	unsigned int temp;


	reg_val = mctl_read_w(MC_WORK_MODE);

	temp = (reg_val>>8) & 0xf;	//page size code
	dram_size0 = (temp - 6);	//(1<<dram_size) * 512Bytes

	temp = (reg_val>>4) & 0xf;	//row width code
	dram_size0 += (temp + 1);	//(1<<dram_size) * 512Bytes

	temp = (reg_val>>2) & 0x3;	//bank number code
	dram_size0 += (temp + 2);	//(1<<dram_size) * 512Bytes

	dram_size0 = dram_size0 - 11;	//(1<<dram_size)MBytes
	dram_size0 = 1<< dram_size0;

	if(reg_val & 0x3)
	{
		reg_val = mctl_read_w(MC_R1_WORK_MODE);
		if(reg_val & 0x3)
		{
			temp = (reg_val>>8) & 0xf;	//page size code
			dram_size1 = (temp - 6);	//(1<<dram_size) * 512Bytes

			temp = (reg_val>>4) & 0xf;	//row width code
			dram_size1 += (temp + 1);	//(1<<dram_size) * 512Bytes

			temp = (reg_val>>2) & 0x3;	//bank number code
			dram_size1 += (temp + 2);	//(1<<dram_size) * 512Bytes

			dram_size1 = dram_size1 - 11;	//(1<<dram_size)MBytes
			//dram_dbg("dram rank1 size is %d MB\n",0x1u<<dram_size1);
			dram_size1 = 1<< dram_size1;
		}
		else
			dram_size1 = dram_size0;
	}
	return (dram_size0 + dram_size1);
}

//*****************************************************************************
//	unsigned int dram_power_up_process()

//  Description:	for super standby dram wakeup
//
//	Arguments:		None
//
//	Return Value:	1: fail		0: Success
//*****************************************************************************
unsigned int dram_power_up_process(void)
{
	unsigned int reg_val = 0;
	mctl_write_w(0x4219D5,MC_WORK_MODE);// 0x0x4219D5--map0 32bit //0x0x4299D5--map1 32bit
	mctl_write_w(mctl_read_w(_CCM_DRAMCLK_CFG_REG)|(0x1U<<31),_CCM_DRAMCLK_CFG_REG);
	mctl_write_w(0x00070005,RFSHTMG);
	mctl_write_w(0xa63,DRAM_MR0);
	mctl_write_w(0x00,DRAM_MR1);
	mctl_write_w(0,DRAM_MR2);
	mctl_write_w(0,DRAM_MR3);
	mctl_write_w(0x01e007c3,PTR0);
	//mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	mctl_write_w(0x01000500,PTR4);
	mctl_write_w(0x01000081,DTCR);
	mctl_write_w(0x03808620,PGCR1);
	mctl_write_w(0x02010101,PITMG0);
	mctl_write_w(0x06021b02,DRAMTMG0);
	mctl_write_w(0x00020102,DRAMTMG1);
	mctl_write_w(0x03030306,DRAMTMG2);
	mctl_write_w(0x00002006,DRAMTMG3);
	mctl_write_w(0x01020101,DRAMTMG4);
	mctl_write_w(0x05010302,DRAMTMG5);

	/* 1.pad release */
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING);
	reg_val &= ~(0x3<<0);
	mctl_write_w(reg_val,VDD_SYS_PWROFF_GATING);	

	reg_val = 0x000183;		//PLL enable, PLL6 should be dram_clk/2
	mctl_write_w(reg_val,PIR);	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);	//for fast simulation
	while((mctl_read_w(STATR )&0x1) != 0x1);	//init done

	reg_val = mctl_read_w(MC_CCCR);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,MC_CCCR);
	local_delay(20);

	mctl_write_w(0x00aa0060,PGCR3);//

	reg_val = mctl_read_w(RFSHCTL0);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,RFSHCTL0);
	local_delay(200);
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val&=~(0x1U<<31);
	mctl_write_w(reg_val,RFSHCTL0);
	local_delay(200);

	reg_val = mctl_read_w(MC_CCCR);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,MC_CCCR);
	local_delay(20);
	
	mctl_write_w(0x27400f,MCTL_CTL_BASE+0xc);
	
	/*2.exit self refresh */
	reg_val = mctl_read_w(PWRCTL);
	reg_val &= ~(0x1<<0);
	reg_val &= ~(0x1<<8);
	mctl_write_w(reg_val,PWRCTL);
	//confirm dram controller has enter selfrefresh
	while(((mctl_read_w(STATR)&0x7) != 0x1));
	/*3.enable master access */
	mctl_write_w(0xffffffff,MC_MAER);
        return 0;
}

//*****************************************************************************
//	signed int init_DRAM(int type)
//  Description:	System init dram
//
//	Arguments:		type:	0: no lock		1: get the fixed parameters & auto detect & lock
//
//	Return Value:	0: fail
//					others: pass
//*****************************************************************************
signed int init_DRAM(int type, __dram_para_t *para)
{
	unsigned int ret_val=0;
	unsigned int pad_hold = 0;
	pad_hold = mctl_read_w(VDD_SYS_PWROFF_GATING)&0x3;
	mctl_sys_init(para);
	
	if(pad_hold == 0x3){
		printf("DRAM standby version V0.2\n");
		dram_power_up_process();
	}else{
		printf("DRAM boot version V0.2\n");
		ret_val=mctl_channel_init(0,para);
		if(ret_val==0)
		return 0;
	}

	ret_val = DRAMC_get_dram_size();
	return ret_val;
}

//*****************************************************************************
//	unsigned int mctl_init()

//  Description:	FPGA intial dram code
//
//	Arguments:		None
//
//	Return Value:	other: dram size		0: Fail
//*****************************************************************************
unsigned int mctl_init(void *para)
{
	unsigned int ret_val = 0;
	__dram_para_t dram_para;
	ret_val = init_DRAM(0, &dram_para);
	return ret_val;
}
#else
/********************************************************************************
 *IC boot code
 ********************************************************************************/
#endif

