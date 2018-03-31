//*****************************************************************************
//	Allwinner Technology, All Right Reserved. 2006-2010 Copyright (c)
//
//	File: 				mctl_hal.c
//
//	Description:  This file implements basic functions for AW1673 DRAM controller
//
//	History:2014/10/13		zhuwei			0.10	Initial version
//				
//*****************************************************************************
#include "mctl_par.h"
#include "mctl_reg.h"
#include "mctl_hal.h"

extern void __usdelay (unsigned int n);

void dram_udelay (unsigned int n)
{
	__usdelay(n);
}
void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}

void dqs_dq_ac_ck_delay(__dram_para_t *para)
{
	s32 i,reg_val;

	//dq0~dq7 read and write delay
	for(i=0;i<9;i++)
	{
		reg_val = mctl_read_w(DATX0IOCR(i));
		reg_val |= (((((para->dram_tpr11)&(0xf<<0))>>0 )<<8)|((((para->dram_tpr12)&(0xf<<0))>>0 )<<0));
		mctl_write_w(reg_val,DATX0IOCR(i));
	}
	//dq8~dq15 read and write delay
	for(i=0;i<9;i++)
	{
		reg_val = mctl_read_w(DATX1IOCR(i));
		reg_val |= (((((para->dram_tpr11)&(0xf<<4))>>4 )<<8)|((((para->dram_tpr12)&(0xf<<4))>>4 )<<0));
		mctl_write_w(reg_val,DATX1IOCR(i));
	}
	//dq16~dq23 read and write delay
	for(i=0;i<9;i++)
	{
		reg_val = mctl_read_w(DATX2IOCR(i));
		reg_val |= (((((para->dram_tpr11)&(0xf<<8))>>8 )<<8)|((((para->dram_tpr12)&(0xf<<8))>>8 )<<0));
		mctl_write_w(reg_val,DATX2IOCR(i));
	}
	//dq24~dq31 read and write delay
	for(i=0;i<9;i++)
	{
		reg_val = mctl_read_w(DATX3IOCR(i));
		reg_val |= (((((para->dram_tpr11)&(0xf<<12))>>12 )<<8)|((((para->dram_tpr12)&(0xf<<12))>>12 )<<0));
		mctl_write_w(reg_val,DATX3IOCR(i));
	}
	//dqs0 read and write delay
	reg_val = mctl_read_w(DATX0IOCR(9));//dqs0_p
	reg_val |= (((((para->dram_tpr11)&(0xf<<16))>>16 )<<8)|((((para->dram_tpr12)&(0xf<<16))>>16 )<<0));
	mctl_write_w(reg_val,DATX0IOCR(9));
	reg_val = mctl_read_w(DATX0IOCR(10));//dqs0_n
	reg_val |= (((((para->dram_tpr11)&(0xf<<16))>>16 )<<8)|((((para->dram_tpr12)&(0xf<<16))>>16 )<<0));
	mctl_write_w(reg_val,DATX0IOCR(10));
	//dqs1 read and write delay
	reg_val = mctl_read_w(DATX1IOCR(9));//dqs2_p
	reg_val |= (((((para->dram_tpr11)&(0xf<<20))>>20 )<<8)|((((para->dram_tpr12)&(0xf<<20))>>20 )<<0));
	mctl_write_w(reg_val,DATX1IOCR(9));
	reg_val = mctl_read_w(DATX1IOCR(10));//dqs2_n
	reg_val |= (((((para->dram_tpr11)&(0xf<<20))>>20 )<<20)|((((para->dram_tpr12)&(0xf<<20))>>20 )<<0));
	mctl_write_w(reg_val,DATX1IOCR(10));
	//dqs2 read and write delay
	reg_val = mctl_read_w(DATX2IOCR(9));//dqs2_p
	reg_val |= (((((para->dram_tpr11)&(0xf<<24))>>24 )<<8)|((((para->dram_tpr12)&(0xf<<24))>>24 )<<0));
	mctl_write_w(reg_val,DATX2IOCR(9));
	reg_val = mctl_read_w(DATX2IOCR(10));//dqs2_n
	reg_val |= (((((para->dram_tpr11)&(0xf<<24))>>24 )<<8)|((((para->dram_tpr12)&(0xf<<24))>>24 )<<0));
	mctl_write_w(reg_val,DATX2IOCR(10));
	//dqs3 read and write delay
	reg_val = mctl_read_w(DATX3IOCR(9));//dqs3_p
	reg_val |= (((((para->dram_tpr11)&(0xfU<<28))>>28 )<<8)|((((para->dram_tpr12)&(0xfU<<28))>>28 )<<0));
	mctl_write_w(reg_val,DATX3IOCR(9));
	reg_val = mctl_read_w(DATX3IOCR(10));//dqs3_n
	reg_val |= (((((para->dram_tpr11)&(0xfU<<28))>>28 )<<8)|((((para->dram_tpr12)&(0xfU<<28))>>28 )<<0));
	mctl_write_w(reg_val,DATX3IOCR(10));
	//LPDDR2/LPDDR2 AC CK CS delay
	if((para->dram_type) == 6 || (para->dram_type) == 7)
	{
		//CA0~CA9 delay
		for(i=0;i<10;i++)
		{
			reg_val = mctl_read_w(CAIOCR(12+i));
			reg_val |= ((((para->dram_tpr10)&(0xf<<4))>>4)<<8);
			mctl_write_w(reg_val,CAIOCR(12+i));
		}
		//CK CS0 CS1 delay
		reg_val = mctl_read_w(CAIOCR(2));//CK
		reg_val |= ((para->dram_tpr10&0xf)<<8);
		mctl_write_w(reg_val,CAIOCR(2));
		reg_val = mctl_read_w(CAIOCR(3));//CS0
		reg_val |= ((para->dram_tpr10&0xf)<<8);
		mctl_write_w(reg_val,CAIOCR(3));
		reg_val = mctl_read_w(CAIOCR(28));//CS1
		reg_val |= ((para->dram_tpr10&0xf)<<8);
		mctl_write_w(reg_val,CAIOCR(28));
	}

}
//*****************************************************************************
//	void set_master_priority(void)
//  Description:		 set master priority and bandwidth limit
//  master  0 :  CPU                      
//  master  1 :  GPU                        
//  master  2 :  AHB                   
//  master  3 :  DMA                   
//  master  4 :  VE                  
//  master  5 :  CSI               
//  master  6 :  NAND                
//  master  7 :  SS                  
//  master  8 :  DE0                   
//  master  9 :  DE1                        
//*****************************************************************************
void set_master_priority(void)
{
  //enable bandwidth limit windows and set windows size 1us
  mctl_write_w(0x000101a0,MC_BWCR);
  //set cpu high priority
  mctl_write_w(0x1,MC_MAPR);
  //set cpu QoS 3 and bandwidth limit -- 160MB/S -- 100MB/S -- 80MB/S
  mctl_write_w(0x00a0000d,MC_MASTER_CFG0(0));
  mctl_write_w(0x00500064,MC_MASTER_CFG1(0));
  //set gpu QoS 2 and bandwidth limit -- 1792MB/S -- 1536MB/S -- 0MB/S
  mctl_write_w(0x07000009,MC_MASTER_CFG0(1));
  mctl_write_w(0x00000600,MC_MASTER_CFG1(1));
  //set dma QoS 2 and bandwidth limit -- 256MB/S --  100MB/S  -- 0MB/S
  mctl_write_w(0x01000009,MC_MASTER_CFG0(3));
  mctl_write_w(0x00000064,MC_MASTER_CFG1(3)); 
  //set ve  QoS 2 and bandwidth limit -- 2048MB/S -- 1600MB/S -- 0MB/S
  mctl_write_w(0x08000009,MC_MASTER_CFG0(4));
  mctl_write_w(0x00000640,MC_MASTER_CFG1(4)); 
  //set de0  QoS 2 and bandwidth limit -- no limit
  mctl_write_w(0x20000308,MC_MASTER_CFG0(8));
  mctl_write_w(0x00001000,MC_MASTER_CFG1(8)); 
  //set de1  QoS 2 and bandwidth limit --640MB/S -- 256MB/S -- 0MB/S
  mctl_write_w(0x02800009,MC_MASTER_CFG0(9));
  mctl_write_w(0x00000100,MC_MASTER_CFG1(9));  
  //set csi  QoS 3 and bandwidth limit -- 384MB/S -- 256MB/S -- 0MB/S 
  mctl_write_w(0x01800009,MC_MASTER_CFG0(5));
  mctl_write_w(0x00000100,MC_MASTER_CFG1(5));  
  //set ss   QoS 2 and bandwidth limit -- 384MS/S -- 256MB/S -- 0MB/S
  mctl_write_w(0x01800009,MC_MASTER_CFG0(7));
  mctl_write_w(0x00000100,MC_MASTER_CFG1(7));  
  //set nand QoS 2 and bandwidth limit -- 100MS/S -- 50MB/S -- 0MB/S
  mctl_write_w(0x00640009,MC_MASTER_CFG0(6));
  mctl_write_w(0x00000032,MC_MASTER_CFG1(6));  
  //set AHB  QoS 3 and bandwidth limit -- 256MB/S -- 128MB/S -- 80MB/S
  mctl_write_w(0x0100000d,MC_MASTER_CFG0(2));
  mctl_write_w(0x00500080,MC_MASTER_CFG1(2));     
	//dram_dbg("DRAM master priority setting ok.\n"); 
}
//***********************************************************************************************
//	void auto_set_timing_para(__dram_para_t *para)
//
//  Description:	auto set the timing para base on the DRAM Frequency in structure
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
void auto_set_timing_para(__dram_para_t *para)
{
	unsigned int  ctrl_freq;//half speed mode :ctrl_freq=1/2 ddr_fre
	unsigned int  type;
	unsigned int  reg_val		= 0;
	unsigned int  tdinit0		= 0;
	unsigned int  tdinit1		= 0;
	unsigned int  tdinit2		= 0;
	unsigned int  tdinit3		= 0;
	unsigned char t_rdata_en	= 1;    //ptimg0
	unsigned char wr_latency	= 1;	//ptimg0
	unsigned char tcl			= 3;	//6
	unsigned char tcwl			= 3;	//6
	unsigned char tmrw			= 0;	//0
	unsigned char tmrd			= 2;	//4;
	unsigned char tmod			= 6;	//12;
	unsigned char tccd			= 2;	//4;
	unsigned char tcke			= 2;	//3;
	unsigned char trrd			= 3;	//6;
	unsigned char trcd			= 6;	//11;
	unsigned char trc			= 20;	//39;
	unsigned char tfaw			= 16;	//32;
	unsigned char tras			= 14;	//28;
	unsigned char trp			= 6;	//11;
	unsigned char twtr			= 3;	//6;
	unsigned char twr			= 8;	//15；
	unsigned char trtp			= 3;	//6;
	unsigned char txp			= 10;	//20;
	unsigned short trefi		= 98;	//195;
	unsigned short trfc			= 128;
	unsigned char twtp			= 12;	//24;	//write to pre_charge
	unsigned char trasmax		= 27;	//54;	//54*1024ck
	unsigned char twr2rd		= 8;	//16;
	unsigned char trd2wr		= 4;	//7;
	unsigned char tckesr		= 3;	//5;
	unsigned char tcksrx		= 4;	//8;
	unsigned char tcksre		= 4;	//8;
	ctrl_freq = para->dram_clk/2;	//Controller work in half rate mode
	type      = para->dram_type;
	//add the time user define
	if(para->dram_tpr13&0x2)
	{
		//dram_dbg("User define timing parameter!\n");
		//dram_tpr0
		tccd = ( (para->dram_tpr0 >> 21) & 0x7  );//[23:21]
		tfaw = ( (para->dram_tpr0 >> 15) & 0x3f );//[20:15]
		trrd = ( (para->dram_tpr0 >> 11) & 0xf  );//[14:11]
		trcd = ( (para->dram_tpr0 >>  6) & 0x1f );//[10:6 ]
		trc  = ( (para->dram_tpr0 >>  0) & 0x3f );//[ 5:0 ]
		//dram_tpr1
		txp =  ( (para->dram_tpr1 >> 23) & 0x1f );//[27:23]
		twtr = ( (para->dram_tpr1 >> 20) & 0x7  );//[22:20]
		trtp = ( (para->dram_tpr1 >> 15) & 0x1f );//[19:15]
		twr =  ( (para->dram_tpr1 >> 11) & 0xf  );//[14:11]
		trp =  ( (para->dram_tpr1 >>  6) & 0x1f );//[10:6 ]
		tras = ( (para->dram_tpr1 >>  0) & 0x3f );//[ 5:0 ]
		//dram_tpr2
		trfc  = ( (para->dram_tpr2 >> 12)& 0x1ff);//[20:12]
		trefi = ( (para->dram_tpr2 >> 0) & 0xfff);//[11:0 ]
	}//add finish
	else
	{
		//dram_dbg("Auto calculate timing parameter!\n");
		if(type==3)
		{
			//dram_tpr0
			tccd=2;
			tfaw= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
			trrd=(10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);
			if(trrd<4) trrd=4;	//max(4ck,10ns)
			trcd= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			trc	= (53*ctrl_freq)/1000 + ( ( ((53*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns
			//dram_tpr1
			txp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(txp<3) txp = 3;//max(3ck,7.5ns)
			twtr= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(twtr<4) twtr=4;	//max(4ck,7,5ns)
			trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(trtp<4) trtp=4;	//max(4ck,7.5ns)
//			twr= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
//			if(twr<3) twr=3;
			trp = (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			tras= (38*ctrl_freq)/1000 + ( ( ((38*ctrl_freq)%1000) != 0) ? 1 :0);	//38ns;
			//dram_tpr2
			trefi	= ( (7800*ctrl_freq)/1000 + ( ( ((7800*ctrl_freq)%1000) != 0) ? 1 :0) )/32;//7800ns
			trfc = (350*ctrl_freq)/1000 + ( ( ((350*ctrl_freq)%1000) != 0) ? 1 :0);	//350ns;
		}else if(type==2)
		{
			tccd=1;
			tfaw= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
			trrd= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
			trcd= (20*ctrl_freq)/1000 + ( ( ((20*ctrl_freq)%1000) != 0) ? 1 :0);	//20ns(10ns)
			trc	= (65*ctrl_freq)/1000 + ( ( ((65*ctrl_freq)%1000) != 0) ? 1 :0);	//65ns
			//dram_tpr1
			txp	= 2;		//2nclk;
			twtr = (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);		//7.5ns;
			trtp = (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);		//7.5ns;
			twr = (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
			trp = (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns(10ns)
			tras= (45*ctrl_freq)/1000 + ( ( ((45*ctrl_freq)%1000) != 0) ? 1 :0);	//45ns;
			//dram_tpr2
			trefi = ((7800*ctrl_freq)/1000 + ( ( ((7800*ctrl_freq)%1000) != 0) ? 1 :0) )/32;//7800ns
			trfc = (328*ctrl_freq)/1000 + ( ( ((328*ctrl_freq)%1000) != 0) ? 1 :0);		//328ns;

		}else if(type==6)
		{
			tccd=1;
			tfaw	= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
			if(tfaw<4) tfaw	= 4;
			trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
			if(trrd<2) trrd	= 2;
			trcd	= (24*ctrl_freq)/1000 + ( ( ((24*ctrl_freq)%1000) != 0) ? 1 :0);	//24ns;
			if(trcd<2) trcd	= 2;
			trc	= (70*ctrl_freq)/1000 + ( ( ((70*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns
			//dram_tpr1
			txp	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(txp<2) txp = 2;//max(2ck,10ns)
			twtr= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(twtr<2) twtr=2;	//max(2ck,7,5ns)
			trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(trtp<2) trtp=2;	//max(2ck,7.5ns)
			twr= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
			if(twr<3) twr=3;
			trp = (27*ctrl_freq)/1000 + ( ( ((27*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			if(trp<2) trp=2;
			tras= (42*ctrl_freq)/1000 + ( ( ((42*ctrl_freq)%1000) != 0) ? 1 :0);	//38ns;
			//dram_tpr2
			trefi	= ( (3900*ctrl_freq)/1000 + ( ( ((3900*ctrl_freq)%1000) != 0) ? 1 :0) )/32;//7800ns
			trfc = (210*ctrl_freq)/1000 + ( ( ((210*ctrl_freq)%1000) != 0) ? 1 :0);	//210ns;
		}else if(type==7)
		{
			tccd=2;
			tfaw= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
			if(tfaw<4) tfaw	= 4;
			trrd=(10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);
			if(trrd<2) trrd=2;	//max(4ck,10ns)
			trcd= (24*ctrl_freq)/1000 + ( ( ((24*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			if(trcd<2) trcd	= 2;
			trc	= (70*ctrl_freq)/1000 + ( ( ((70*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns
			//dram_tpr1
			txp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(txp<2) txp = 2;//max(3ck,7.5ns)
			twtr= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(twtr<2) twtr=2;	//max(4ck,7,5ns)
			trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(trtp<2) trtp=2;	//max(4ck,7.5ns)
			twr= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
			if(twr<3) twr=3;
			trp = (27*ctrl_freq)/1000 + ( ( ((27*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			if(trp<2) trp=2;
			tras= (42*ctrl_freq)/1000 + ( ( ((42*ctrl_freq)%1000) != 0) ? 1 :0);	//38ns;
			//dram_tpr2
			trefi	= ( (3900*ctrl_freq)/1000 + ( ( ((3900*ctrl_freq)%1000) != 0) ? 1 :0) )/32;//7800ns
			trfc = (210*ctrl_freq)/1000 + ( ( ((210*ctrl_freq)%1000) != 0) ? 1 :0);	//350ns;
		}
		//assign the value back to the DRAM structure
		para->dram_tpr0 = (trc<<0) | (trcd<<6) | (trrd<<11) | (tfaw<<15) | (tccd<<21) ;
		//dram_dbg("para_dram_tpr0 = %x\n",para->dram_tpr0);
		para->dram_tpr1 = (tras<<0) | (trp<<6) | (twr<<11) | (trtp<<15) | (twtr<<20)|(txp<<23);
		//dram_dbg("para_dram_tpr1 = %x\n",para->dram_tpr1);
		para->dram_tpr2 = (trefi<<0) | (trfc<<12);
		//dram_dbg("para_dram_tpr2 = %x\n",para->dram_tpr2);
	}
	switch(type)
	{
	case 2://DDR2
		//the time we no need to calculate
		tmrw=0x0;
		tmrd=0x2;
		tmod=0xc;
		tcke=3;
		tcksrx=5;
		tcksre=5;
		tckesr=tcke + 1;
		trasmax =0x1b;

		tcl		= 3;	//CL   6
		tcwl	= 3;	//WL  5
		t_rdata_en  =1;
		wr_latency  =1;
		para->dram_mr0 	= 0x1A50;
		para->dram_mr2  = 0x10; //CWL=8,800M
		para->dram_mr3  = 0;

		tdinit0	= (500*para->dram_clk) + 1;	//500us
		tdinit1	= (360*para->dram_clk)/1000 + 1;//360ns
		tdinit2	= (200*para->dram_clk) + 1;	//200us
		tdinit3	= (1*para->dram_clk) + 1;	//1us
		twtp=tcwl+2+twr;//WL+BL/2+tWR
		twr2rd= tcwl+2+twtr;//WL+BL/2+tWTR
		trd2wr= tcl+2+1-tcwl;//RL+BL/2+2-WL
		//dram_dbg("tcl = %d,tcwl = %d\n",tcl,tcwl);
		break;
	case 3://DDR3
		//the time we no need to calculate
		tmrw=0x0;
		tmrd=0x4;
		tmod=0xc;
		tcke=3;
		tcksrx=5;
		tcksre=5;
		tckesr=4;
		trasmax =0x18;

		tcl		= 6;	//CL   12
		tcwl	= 4;	//CWL  8
		t_rdata_en  =4;
		wr_latency  =2;
		para->dram_mr0 	= 0x1c70;//CL=11,wr 12
		para->dram_mr2  = 0x18; //CWL=8,800M
		para->dram_mr3  = 0;

		tdinit0	= (500*para->dram_clk) + 1;	//500us
		tdinit1	= (400*para->dram_clk)/1000 + 1;//360ns
		tdinit2	= (200*para->dram_clk) + 1;	//200us
		tdinit3	= (1*para->dram_clk) + 1;	//1us
		twr = (para->dram_mr0>>9) & 0x7;
		switch(twr)
		{
			case 0:	twr = 16;	break;
			case 1:	twr = 5;	break;
			case 2:	twr = 6;	break;
			case 3:	twr = 7;	break;
			case 4:	twr = 8;	break;
			case 5:	twr = 10;	break;
			case 6:	twr = 12;	break;
			case 7:	twr = 14;	break;
			default:twr = 16;	break;
		}
		twr = twr/2 + (((twr%2) != 0) ? 1 : 0);	//tWR/2;
		twtp=tcwl+2+twr;//WL+BL/2+tWR
		twr2rd= tcwl+2+twtr;//WL+BL/2+tWTR
		trd2wr= tcl+2+1-tcwl;//RL+BL/2+2-WL
		//dram_dbg("tcl = %d,tcwl = %d\n",tcl,tcwl);
		break;
	case 6 ://LPDDR2
		tmrw=0x3;
		tmrd=0x5;
		tmod=0x5;
		tcke=2;
		tcksrx=5;
		tcksre=5;
		tckesr=5;
		trasmax =0x18;
		//according to frequency
		tcl		= 4;
		tcwl	= 2;
		t_rdata_en  =3;    //if tcl odd,(tcl-3)/2;  if tcl even ,((tcl+1)-3)/2
		wr_latency  =1;
		para->dram_mr0 = 0;
		para->dram_mr1 = 0xc3;//twr=8;bl=8
		para->dram_mr2 = 0x6;//RL=8,CWL=4

		//end
		tdinit0	= (200*para->dram_clk) + 1;	//200us
		tdinit1	= (100*para->dram_clk)/1000 + 1;	//100ns
		tdinit2	= (11*para->dram_clk) + 1;	//11us
		tdinit3	= (1*para->dram_clk) + 1;	//1us
		twtp	= tcwl + 2 + twr + 1;	// CWL+BL/2+tWR
		trd2wr	= tcl + 2 + 5 - tcwl + 1;//5?
		twr2rd	= tcwl + 2 + 1 + twtr;//wl+BL/2+1+tWTR??
		break;
	case 7 ://LPDDR3
		tmrw=0x5;
		tmrd=0x5;
		tmod=0xc;
		tcke=3;
		tcksrx=5;
		tcksre=5;
		tckesr=5;
		trasmax =0x18;
		//according to clock
		tcl		= 6;
		tcwl	= 3;
		t_rdata_en  =5;    //if tcl odd,(tcl-3)/2;  if tcl even ,((tcl+1)-3)/2
		wr_latency  =2;
		para->dram_mr0 = 0;
		para->dram_mr1 = 0xc3;//twr=8;bl=8
		para->dram_mr2 = 0xa;//RL=12,CWL=6
		//end
		tdinit0	= (200*para->dram_clk) + 1;	//200us
		tdinit1	= (100*para->dram_clk)/1000 + 1;	//100ns
		tdinit2	= (11*para->dram_clk) + 1;	//11us
		tdinit3	= (1*para->dram_clk) + 1;	//1us
		twtp	= tcwl + 4 + twr + 1;	// CWL+BL/2+tWR
		trd2wr	= tcl + 4 + 5 - tcwl + 1;	//13;
		twr2rd	= tcwl + 4 + 1 + twtr;
		break;
	default:
		break;
	}
	//set work mode register before training,include 1t/2t DDR type，BL,rank number
	reg_val=mctl_read_w(MC_WORK_MODE);
	reg_val &=~((0xfff<<12)|(0xf<<0));
	reg_val|=(0x4<<20); //LPDDR2/LPDDR3/ddr3 all use BL8
	reg_val |= ((para->dram_type & 0x7)<<16);//DRAM type
	reg_val |= (( ( (para->dram_para2) & 0x1 )? 0x0:0x1) << 12);	//DQ width
	reg_val |= ( (para->dram_para2)>>12 & 0x03 );	//rank
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);//BANK
	if((para->dram_type==6)||(para->dram_type==7))
		reg_val |= (0x1U<<19);  //LPDDR2/3 must use 1T mode
	else
		reg_val |= (((para->dram_tpr13>>5)&0x1)<<19);//2T or 1T
	mctl_write_w(reg_val,MC_WORK_MODE);
	//set mode register
	mctl_write_w((para->dram_mr0),DRAM_MR0);
	mctl_write_w((para->dram_mr1),DRAM_MR1);
	mctl_write_w((para->dram_mr2),DRAM_MR2);
	mctl_write_w((para->dram_mr3),DRAM_MR3);
	//set dram timing
	reg_val= (twtp<<24)|(tfaw<<16)|(trasmax<<8)|(tras<<0);
	//dram_dbg("DRAM TIMING PARA0 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG0);//DRAMTMG0
	reg_val= (txp<<16)|(trtp<<8)|(trc<<0);
	//dram_dbg("DRAM TIMING PARA1 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG1);//DRAMTMG1
	reg_val= (tcwl<<24)|(tcl<<16)|(trd2wr<<8)|(twr2rd<<0);
	//dram_dbg("DRAM TIMING PARA2 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG2);//DRAMTMG2
	reg_val= (tmrw<<16)|(tmrd<<12)|(tmod<<0);
	//dram_dbg("DRAM TIMING PARA3 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG3);//DRAMTMG3
	reg_val= (trcd<<24)|(tccd<<16)|(trrd<<8)|(trp<<0);
	//dram_dbg("DRAM TIMING PARA4 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG4);//DRAMTMG4
	reg_val= (tcksrx<<24)|(tcksre<<16)|(tckesr<<8)|(tcke<<0);
	//dram_dbg("DRAM TIMING PARA5 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG5);//DRAMTMG5
	//set two rank timing
	reg_val= mctl_read_w(DRAMTMG8);
	reg_val&=~(0xff<<8);
	reg_val&=~(0xff<<0);
	reg_val|=(0x66<<8);
	reg_val|=(0x10<<0);
	mctl_write_w(reg_val,DRAMTMG8);//DRAMTMG8
	//dram_dbg("DRAM TIMING PARA8 = %x\n",reg_val);
	//set phy interface time
	reg_val=(0x2<<24)|(t_rdata_en<<16)|(0x1<<8)|(wr_latency<<0);
	//dram_dbg("DRAM PHY INTERFACE PARA = %x\n",reg_val);
	mctl_write_w(reg_val,PITMG0);	//PHY interface write latency and read latency configure
	//set phy time  PTR0-2 use default
	mctl_write_w(((tdinit0<<0)|(tdinit1<<20)),PTR3);
	mctl_write_w(((tdinit2<<0)|(tdinit3<<20)),PTR4);
//	mctl_write_w(0x01e007c3,PTR0);
//	mctl_write_w(0x00170023,PTR1);
//	mctl_write_w(0x00800800,PTR3);
//	mctl_write_w(0x01000500,PTR4);
	//set refresh timing
    reg_val =(trefi<<16)|(trfc<<0);
    mctl_write_w(reg_val,RFSHTMG);
}

//***********************************************************************************************
//	unsigned int ccm_set_pll_ddr_clk(u32 pll_clk)
//
//  Description:	set-pll-ddr-clk
//
//	Return Value:	None
//***********************************************************************************************
unsigned int ccm_set_pll_ddr_clk(u32 pll_clk)
{
	unsigned int n, k, m = 1,rval;
	unsigned int div;
	unsigned int mod2, mod3;
	unsigned int min_mod = 0;

	div = pll_clk/24;
	if (div <= 32) {
		n = div;
		k = 1;
	} else {
		/* when m=1, we cann't get absolutely accurate value for follow clock:
		 * 840(816), 888(864),
		 * 984(960)
		 */
		mod2 = div&1;
		mod3 = div%3;
		min_mod = mod2;
		k = 2;
		if (min_mod > mod3) {
			min_mod = mod3;
			k = 3;
		}
		n = div / k;
	}
	rval = mctl_read_w(_CCM_PLL_DDR_REG);
	rval &= ~((0x1f << 8) | (0x3 << 4) | (0x3 << 0));
	rval = (1U << 31)  | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	mctl_write_w(rval, _CCM_PLL_DDR_REG);
	mctl_write_w(rval|(1U << 20), _CCM_PLL_DDR_REG);
	dram_udelay(20);
	return 24 * n * k / m;
}

unsigned int ccm_set_dram_div(u32 div)
{
	unsigned int  reg_val,ret;
	ret=div-1;
	reg_val =(ret|(0x1<<16));
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	while(mctl_read_w(_CCM_DRAMCLK_CFG_REG) & (0x1<<16));
	dram_udelay(10);
	return 0;
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
	//trun off mbus clk gate
	reg_val = mctl_read_w(MBUS_CLK_CTL_REG);
	reg_val &=~(1U<<31);
	mctl_write_w(reg_val, MBUS_CLK_CTL_REG);
	//mbus reset
	reg_val = mctl_read_w(MBUS_RESET_REG);
	reg_val &=~(1U<<31);
	mctl_write_w(reg_val, MBUS_RESET_REG);
	// DISABLE DRAMC BUS GATING
	reg_val = mctl_read_w(BUS_CLK_GATE_REG0);
	reg_val &= ~(1U<<14);
	mctl_write_w(reg_val, BUS_CLK_GATE_REG0);
	//DRAM BUS reset
	reg_val = mctl_read_w(BUS_RST_REG0);
	reg_val &= ~(1U<<14);
	mctl_write_w(reg_val, BUS_RST_REG0);
	//disable pll-ddr
	reg_val = mctl_read_w(_CCM_PLL_DDR_REG);
	reg_val &=~(1U<<31);
	mctl_write_w(reg_val, _CCM_PLL_DDR_REG);
	dram_udelay(10);//1us ic
	//controller reset
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	dram_udelay(1000);//1ms ic
	//set pll
	if((para->dram_tpr13>>22)&0x1)//USE PERIPR PLL
	{
		reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);//clk_src_sel pll_periph
		reg_val&=~(0x3<<20);
		reg_val|=(0x1<<20);
		mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	}
	else//USE DDR_PLL
	{
		reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);//clk_src_sel pll_ddr
		reg_val&=(~(0x3<<20));
		mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
		ccm_set_pll_ddr_clk((para->dram_clk)*2);
	}
	//Setup DRAM Clock
	ccm_set_dram_div(1);

	dram_udelay(2000);//1ms ic

	if((para->dram_tpr13>>16)&0x3f)
	{
		//********************************************
		//bit16 : 겠류0.5
		//bit17 : 겠류0.45
		//bit18 : 겠류0.4
		//bit19 : 겠류0.3
		//bit20 : 겠류0.2
		//bit21 : 겠류0.1
		//**********************************************
		if(para->dram_tpr13 & (0x1<<16))
		{
			mctl_write_w(((0x3U<<17)|(0x158U<<20)|(0x3U<<29)|(0x1U<<31)),PLL_DDR_PATTERN_CTL_REG);
			//dram_dbg("DRAM SSCG enable , range 0.5.\n");
		}
		else if(para->dram_tpr13 & (0x1<<17))
		{
			mctl_write_w((0x1999U|(0x3U<<17)|(0x135U<<20)|(0x3U<<29)|(0x1U<<31)),PLL_DDR_PATTERN_CTL_REG);
			//dram_dbg("DRAM SSCG enable , range 0.45.\n");
		}
		else if(para->dram_tpr13 & (0x1<<18))
		{
			mctl_write_w((0x3333U|(0x3<<17)|(0x120U<<20)|(0x3U<<29)|(0x1U<<31)),PLL_DDR_PATTERN_CTL_REG);
			//dram_dbg("DRAM SSCG enable , range 0.4.\n");
		}
		else if(para->dram_tpr13 & (0x1<<19))
		{
			mctl_write_w((0x6666U|(0x3U<<17)|(0xD8U<<20)|(0x3U<<29)|(0x1U<<31)),PLL_DDR_PATTERN_CTL_REG);
			//dram_dbg("DRAM SSCG enable , range 0.3.\n");
		}
		else if(para->dram_tpr13 & (0x1<<20))
		{
			mctl_write_w((0x9999U|(0x3U<<17)|(0x90U<<20)|(0x3U<<29)|(0x1U<<31)),PLL_DDR_PATTERN_CTL_REG);
			//dram_dbg("DRAM SSCG enable , range 0.2.\n");
		}
		else if(para->dram_tpr13 & (0x1<<21))
		{
			mctl_write_w((0xccccU|(0x3U<<17)|(0x48U<<20)|(0x3U<<29)|(0x1U<<31)),PLL_DDR_PATTERN_CTL_REG);
			//dram_dbg("DRAM SSCG enable , range 0.1.\n");
		}
		reg_val = mctl_read_w(_CCM_PLL_DDR_REG);//enable pll_sdm_en
		reg_val |=(0x1U<<24);
		mctl_write_w(reg_val,_CCM_PLL_DDR_REG);

		reg_val = mctl_read_w(_CCM_PLL_DDR_REG);//updata pll
		reg_val |=(0x1U<<20);
		mctl_write_w(reg_val,_CCM_PLL_DDR_REG);
		while(mctl_read_w(_CCM_PLL_DDR_REG) & (0x1<<30));
		dram_udelay(50);
	}
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val |= (0x1<<16);//updata dram clock
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	while(mctl_read_w(_CCM_DRAMCLK_CFG_REG) & (0x1<<16));

	//release DRAM ahb BUS RESET
	reg_val = mctl_read_w(BUS_RST_REG0);
	reg_val |= (1U<<14);
	mctl_write_w(reg_val, BUS_RST_REG0);
	//open AHB gating
	reg_val = mctl_read_w(BUS_CLK_GATE_REG0);
	reg_val |= (1U<<14);
	mctl_write_w(reg_val, BUS_CLK_GATE_REG0);
	//release DRAM mbus RESET
	reg_val = mctl_read_w(MBUS_RESET_REG);
	reg_val |=(1U<<31);
	mctl_write_w(reg_val, MBUS_RESET_REG);
	//open mbus gating
	reg_val = mctl_read_w(MBUS_CLK_CTL_REG);
	reg_val |=(1U<<31);
	mctl_write_w(reg_val, MBUS_CLK_CTL_REG);
	//release controller reset
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val |=(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	dram_udelay(10);//10us ic
	//enable dramc clk
	mctl_write_w(0xc00e,MC_CLKEN);
	dram_udelay(500);//500us ic

	return DRAM_RET_OK	;
}
//***********************************************************************************************
//	void mctl_com_init(__dram_para_t *para)
//
//  Description:	set ddr para and enable clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
void mctl_com_init(__dram_para_t *para)
{
	unsigned int reg_val;
	reg_val = 0;
	reg_val|=(0x4<<20);
	reg_val |= ((para->dram_type & 0x07)<<16);//DRAM type
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);//BANK
	reg_val |= ((( ( ((para->dram_para1)>>20) & 0xff) - 1) & 0xf) << 4);//Row number
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x0:0x1) << 12);	//DQ width
	reg_val |= ( (para->dram_para2)>>12 & 0x03 );	//rank
	if((para->dram_type==6)||(para->dram_type==7))
		reg_val |= (0x1U<<19);  //LPDDR2/3 must use 1T mode
	else
		reg_val |= (((para->dram_tpr13>>5)&0x1)<<19);//2T or 1T
	switch( ((para->dram_para1)>>16) & 0xf )	//MCTL_PAGE_SIZE
	{
	case 8:
		reg_val |= 0xA << 8;
		break;
	case 4:
		reg_val |= 0x9 << 8;
		break;
	case 2:
		reg_val |= 0x8 << 8;
		break;
	case 1:
		reg_val |= 0x7 << 8;
		break;
	default:
		reg_val |= 0x6 <<8;
		break;
	}
	mctl_write_w(reg_val,MC_WORK_MODE);
}


unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	unsigned int reg_val = 0,ret_val=0;
	unsigned int n = 4;
	unsigned int dqs_gating_mode =0;
	unsigned int byte0_state = 0;
	unsigned int byte1_state = 0;
	unsigned int byte2_state = 0;
	unsigned int byte3_state = 0;
	unsigned int i =0;
	unsigned int hex32_to_gray[32] = {0x0,0x1,0x2,0x3,0x6,0x7,0x4,0x5,
			                          0xc,0xd,0xe,0xf,0xa,0xb,0x8,0x9,
			                          0x18,0x19,0x1a,0x1b,0x1e,0x1f,0x1c,0x1d,
			                          0x14,0x15,0x16,0x17,0x12,0x13,0x10,0x11};
	unsigned int gray_to_hex32[32] = {0x0,0x1,0x2,0x3,0x6,0x7,0x4,0x5,
			                          0xe,0xf,0xc,0xd,0x8,0x9,0xa,0xb,
			                          0x1e,0x1f,0x1c,0x1d,0x18,0x19,0x1a,0x1b,
			                          0x10,0x11,0x12,0x13,0x16,0x17,0x14,0x15};
	dqs_gating_mode = (para->dram_tpr13>>2)&0x7;
	auto_set_timing_para(para);
	//set dram master priority
	//set_master_priority();
//****************************************************************************************************//
//setting VTC,default disable all VT
	if((para->dram_clk>400)&&((para->dram_tpr13>>9)&0x1))
	{
		//dram_dbg("DRAM VTC is enable\n");
		reg_val=mctl_read_w(PGCR0);
		reg_val |=(0x1<<30);
		reg_val |=(0x3f<<0);
		mctl_write_w(reg_val,PGCR0);
	}else
	{
		//dram_dbg("DRAM VTC is disable\n");
		reg_val=mctl_read_w(PGCR0);
		reg_val &=~(0x1<<30);
		reg_val &=~(0x3f<<0);
		mctl_write_w(reg_val,PGCR0);
		
		reg_val=mctl_read_w(PGCR1);
		reg_val |=(0x1U<<26);
		reg_val &=~(0x1U<<24);
		mctl_write_w(reg_val,PGCR1);
	}
//****************************************************************************************************//
//increase DFI_PHY_UPD CLOCK according to A80
	mctl_write_w(0x94be6fa3,MC_PROTECT);
	dram_udelay(100);

	reg_val=mctl_read_w(MX_UPD2);
	reg_val &= ~(0xfff<<16);
	reg_val |= (0x50<<16);
	mctl_write_w(reg_val,MX_UPD2);

	mctl_write_w(0x0,MC_PROTECT);
	dram_udelay(100);
//****************************************************************************************************//
//set dramc odt ,DX/DQS/DM/.   0-7 for dx0-7 ,8 for dm, 9-10 for dqs,dqs#
//set AC PDR/PDD
	if((para->dram_odt_en&0x1))
	{
		//dram_dbg("DRAMC DQS/DQ ODT enable.\n");
		ret_val = (para->dram_odt_en & 0x2)>>1;
		//dram_dbg("DRAMC read ODT type : %d (0: dynamic  1: always on).\n",ret_val);
		for(i=0;i<n;i++)
		{
			//byte 0/byte 1
			reg_val = mctl_read_w(DXnGCR0(i));
			reg_val &= ~(0x3U<<4);
			reg_val |= (ret_val<<4);
			reg_val &= ~(0x1U<<1);	// SSTL IO mode
			reg_val &= ~(0x3U<<2);	//OE mode: 0 Dynamic
			reg_val &= ~(0x3U<<12);	//Power Down Receiver: Dynamic
			reg_val &= ~(0x3U<<14);	//Power Down Driver: Dynamic
			mctl_write_w(reg_val,DXnGCR0(i));
		}
	}
	else
	{
		//dram_dbg("DRAMC DQS/DQ ODT disable.\n");
		for(i=0;i<n;i++)//for dqs/dqs#,odt always on
		{
			//byte 0/byte 1
			reg_val = mctl_read_w(DXnGCR0(i));
			reg_val &= ~(0x3U<<4);
			reg_val |= (0x2<<4);
			reg_val &= ~(0x1U<<1);	// SSTL IO mode
			reg_val &= ~(0x3U<<2);	//OE mode: 0 Dynamic
			reg_val &= ~(0x3U<<12);	//Power Down Receiver: Dynamic
			reg_val &= ~(0x3U<<14);	//Power Down Driver: Dynamic
			mctl_write_w(reg_val,DXnGCR0(i));
		}
	}	
	//AC PDR should always ON
	reg_val = mctl_read_w(CAIOCR0);
	reg_val |=(0x1<<1);
	mctl_write_w(reg_val,CAIOCR0);

//****************************************************************************************************//
//auto detect dram size setting.In this mode ,make sure set 2 rank and full DQ
	if(!(para->dram_tpr13&0x1))
	{
		//dram_dbg("DRAM SIZE AUTO SCAN ENABLE.\n");
		reg_val = mctl_read_w(MC_WORK_MODE);
		reg_val |=((0x1<<12)|(0x1<<0));
		mctl_write_w(reg_val,MC_WORK_MODE);
	}
//****************************************************************************************************//
//set DQS Gating mode: default close
	switch(dqs_gating_mode)
	{
		case 0://auto gate PD
			reg_val=mctl_read_w(PGCR2);
			reg_val|=(0x3<<6);
			mctl_write_w(reg_val,PGCR2);
			//dram_dbg("DRAM DQS gate is PD mode.\n");
			break;
		case 1://open DQS gating
			reg_val=mctl_read_w(PGCR2);
			reg_val&=~(0x3<<6);
			mctl_write_w(reg_val,PGCR2);

			reg_val=mctl_read_w(DQSGMR);
			reg_val&=~((0x1<<8)|(0x7));
			mctl_write_w(reg_val,DQSGMR);
			//dram_dbg("DRAM DQS gate is open.\n");
			//set DRAM ODT MAP when need DQS gating
			reg_val = (mctl_read_w(MC_WORK_MODE)&0x1);
			if(reg_val)
			{
				mctl_write_w(0x00000303,ODTMAP);
			}else
			{
				mctl_write_w(0x00000201,ODTMAP);
			}
			break;
		case 2://auto gating PU
			reg_val=mctl_read_w(PGCR2);
			reg_val&=~(0x3<<6);
			reg_val|=(0x2<<6);
			mctl_write_w(reg_val,PGCR2);

			ret_val =((mctl_read_w(DRAMTMG2)>>16)&0x1f)-2;
			reg_val=mctl_read_w(DQSGMR);
			reg_val&=~((0x1<<8)|(0x7));
			reg_val|=((0x1<<8)|(ret_val));
			mctl_write_w(reg_val,DQSGMR);
			//dram_dbg("DRAM DQSGMR value is %x\n",reg_val);

			reg_val =mctl_read_w(DXCCR);
			reg_val &=~(0x1U<<31);
			reg_val |=(0x1<<27);
			mctl_write_w(reg_val,DXCCR);
			//dram_dbg("DRAM DQS gate is PU mode.\n");
			break;
		default:
			reg_val=mctl_read_w(PGCR2);
			reg_val|=(0x3<<6);
			mctl_write_w(reg_val,PGCR2);
			//dram_dbg("DRAM DQS gate is PD mode.\n");
			break;
	}
//for lpddr2/lpddr3,if use gqs gate,can extend the dqs gate after training
	if((para->dram_type) == 6 || (para->dram_type) == 7)
	{
		if(dqs_gating_mode==1)//open dqs gate and need dqs training
		{
			//this mdoe ,dqs pull down ,dqs# pull up,use extend 0 to training
			reg_val =mctl_read_w(DXCCR);
			reg_val |=(0x1U<<31);
			reg_val &=~(0x1<<27);
			reg_val &=~(0x3<<6);
			mctl_write_w(reg_val,DXCCR);
		}
	}
//add for lpddr2,change pull up/down register
	if(para->dram_type == 6 || (para->dram_type) == 7)
	{
		  reg_val =mctl_read_w(DXCCR);
		  reg_val &=~(0xFU<<28);
		  reg_val &=~(0xFU<<24);
		  reg_val |= (0xaU<<28);
		  reg_val |= (0x2U<<24);
		  mctl_write_w(reg_val,DXCCR);
	}
//****************************************************************************************************//
//setting half DQ
	ret_val = ((mctl_read_w(MC_WORK_MODE)>>12)&0x7);
	if(!ret_val)
	{
		mctl_write_w(0,DXnGCR0(2));
		mctl_write_w(0,DXnGCR0(3));
		//dram_dbg("DRAM half DQ mode,disable high DQ\n");
	}
/*******************************************************************************************************
 * set according to designer advise config
 */
	reg_val = mctl_read_w(PGCR0);
	reg_val &= ~(0x3<<14);	//dx ddr_clk dynamic mode
	reg_val &= ~(0x3<<12);	//dx hdr_clk dynamic mode
	mctl_write_w(reg_val,PGCR0);

	reg_val = mctl_read_w(PGCR2);
	reg_val &= ~(0x3<<10);
	reg_val |= 0x1<<10;		//dphy & aphy phase select 270 degree
	reg_val &= ~(0x3<<8);
	reg_val |= 0x1<<8;		//dout phase select 270 degree
	mctl_write_w(reg_val,PGCR2);
//****************************************************************************************************//
	//data training configuration
	ret_val = (mctl_read_w(MC_WORK_MODE)&0x1);
	if(ret_val)
	{
		reg_val = mctl_read_w(DTCR);
		reg_val &= ~(0xf<<24);
		reg_val |= (0x3<<24);
		mctl_write_w(reg_val,DTCR);  //two rank
		//dram_dbg("DRAM two rank training is on,the value is %x\n",reg_val);
	}
	else
	{
		reg_val = mctl_read_w(DTCR);
		reg_val &= ~(0xf<<24);
		reg_val |= (0x1<<24);
		mctl_write_w(reg_val,DTCR);  //one rank
		//dram_dbg("DRAM one rank training is on,the value is %x\n",reg_val);
		}
//****************************************************************************************************//

	if((para->dram_tpr13>>23)&0x1)
	{
		dqs_dq_ac_ck_delay(para);
		__usdelay(50);
	}

	//zq calibration/training
	//set zq para  DX ODT/DX DRIVER /CK DRIVER/CA DRIVER
	reg_val = mctl_read_w(ZQCR);
	reg_val &= ~(0x00ffffff);
	reg_val |= ( (para->dram_zq) & 0xffffff );
	mctl_write_w(reg_val,ZQCR);

	mctl_write_w(0x3,PIR );	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);
	dram_udelay(10);

	reg_val = mctl_read_w(ZQnDR(0));
	//dram_dbg("ck zq calibration val is %x\n",reg_val);
	reg_val&=(~(0x1f<<8));
	reg_val&=(~(0x1f<<24));
	if(para->dram_tpr9 & 0x1)
	{
		ret_val = (reg_val & 0x1f);
		if(ret_val != 0x11)
			reg_val |= (hex32_to_gray[gray_to_hex32[ret_val] + 1])<<8;
		else
			reg_val |= ret_val<<8;
		ret_val = ((reg_val>>16) & 0x1f);
		if(ret_val != 0x11)
			reg_val |= (hex32_to_gray[gray_to_hex32[ret_val] + 1])<<24;
		else
			reg_val |= ret_val<<24;
	}
	else
	{
		reg_val|=(reg_val&0x1f)<<8;
		reg_val|=((reg_val&(0x1f<<16))>>16)<<24;
	}
	mctl_write_w(reg_val,ZQnDR(0));
	//dram_dbg("ck zq manul config val is %x\n",reg_val);

	reg_val = mctl_read_w(ZQnDR(1));
	//dram_dbg("dx0/1 zq calibration val is %x\n",reg_val);
	reg_val&=(~(0x1f<<8));
	reg_val&=(~(0x1f<<24));
	if(para->dram_tpr9 & 0x1)
	{
		ret_val = (reg_val & 0x1f);
		if(ret_val != 0x11)
			reg_val |= (hex32_to_gray[gray_to_hex32[ret_val] + 1])<<8;
		else
			reg_val |= ret_val<<8;
		ret_val = ((reg_val>>16) & 0x1f);
		if(ret_val != 0x11)
			reg_val |= (hex32_to_gray[gray_to_hex32[ret_val] + 1])<<24;
		else
			reg_val |= ret_val<<24;
	}
	else
	{
		reg_val|=(reg_val&0x1f)<<8;
		reg_val|=((reg_val&(0x1f<<16))>>16)<<24;
	}
	mctl_write_w(reg_val,ZQnDR(1));
	//dram_dbg("dx zq0/1 manul config val is %x\n",reg_val);
	
	reg_val = mctl_read_w(ZQnDR(2));
	//dram_dbg("dx2/3 zq calibration val is %x\n",reg_val);
	reg_val&=(~(0x1f<<8));
	reg_val&=(~(0x1f<<24));
	if(para->dram_tpr9 & 0x1)
	{
		ret_val = (reg_val & 0x1f);
		if(ret_val != 0x11)
			reg_val |= (hex32_to_gray[gray_to_hex32[ret_val] + 1])<<8;
		else
			reg_val |= ret_val<<8;
		ret_val = ((reg_val>>16) & 0x1f);
		if(ret_val != 0x11)
			reg_val |= (hex32_to_gray[gray_to_hex32[ret_val] + 1])<<24;
		else
			reg_val |= ret_val<<24;
	}
	else
	{
		reg_val|=(reg_val&0x1f)<<8;
		reg_val|=((reg_val&(0x1f<<16))>>16)<<24;
	}
	mctl_write_w(reg_val,ZQnDR(2));
	//dram_dbg("dx2/3 zq manul config val is %x\n",reg_val);
//*********************************zq done*********************************
#if 1
	dram_udelay(1000);
	reg_val = 0x1u<<30;
	if((para->dram_type) == 3)
		reg_val = 0x001f1;
	else
		reg_val = 0x00171;	//bit 7,DDR RESET
	//accord to DQS gating mode to choose dqs gating

	mctl_write_w(reg_val,PIR );	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);
	dram_udelay(1000);

	if(dqs_gating_mode == 1 || !(para->dram_tpr13 & (0x1)))
	{
		reg_val = 0x401;
	}
	mctl_write_w(reg_val,PIR );	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);
	dram_udelay(10);

# else
//****************************************************************************************************//
	//zq calibration/training
		//set zq para  DX ODT/DX DRIVER /CK DRIVER/CA DRIVER
		reg_val = 0x1u<<30;
		if((para->dram_type) == 3)
			reg_val = 0x001f3;
		else
			reg_val = 0x00173;	//bit 7,DDR RESET
		//accord to DQS gating mode to choose dqs gating
		if(dqs_gating_mode==1)
			reg_val |=(0x1<<10);
		//dram_dbg("DRAM PIR value is %x\n",reg_val);
		mctl_write_w(reg_val,PIR );	//for fast simulation
		while((mctl_read_w(PGSR0 )&0x1) != 0x1);
		dram_udelay(10);
#endif
/*****************************************************************************************************/
//training information
		reg_val = mctl_read_w(PGSR0);
		if((reg_val>>21)&0x7f)	//training ERROR except zq calibration
		{
			//dram_dbg("DRAM training error,PGSR0 = %x\n",reg_val);
			reg_val = mctl_read_w(DXnGSR0(0));
			//dram_dbg("DRAM byte0 DQs information,DX0GSR = %x\n",reg_val);
			byte0_state = (reg_val>>24)&0x3;
			reg_val = mctl_read_w(DXnGSR0(1));
			//dram_dbg("DRAM byte1 DQs information,DX1GSR = %x\n",reg_val);
			byte1_state = (reg_val>>24)&0x3;			
			reg_val = mctl_read_w(DXnGSR0(2));
			//dram_dbg("DRAM byte2 DQs information,DX2GSR = %x\n",reg_val);
			byte2_state = (reg_val>>24)&0x3;
			reg_val = mctl_read_w(DXnGSR0(3));
			//dram_dbg("DRAM byte3 DQs information,DX3GSR = %x\n",reg_val);
			byte3_state = (reg_val>>24)&0x3;
			
			if(!(para->dram_tpr13&0x1))
			{
				if((byte0_state==0x2)&&(byte1_state==0x2)&&(byte2_state==0x2)&&(byte3_state==0x2))
				{
					//dram_dbg("DRAM only have one rank,now set one rank!\n");
					reg_val = mctl_read_w(MC_WORK_MODE);
					reg_val &=~(0x1<<0);
					mctl_write_w(reg_val,MC_WORK_MODE);
					para->dram_para2 &=~(0xf<<12);
					//single training setting
					reg_val = mctl_read_w(DTCR);
					reg_val &= ~(0x3<<24);
					reg_val |= (0x1<<24);
					mctl_write_w(reg_val,DTCR);
					//re-train
					mctl_write_w(0x401,PIR);
					while((mctl_read_w(PGSR0 )&0x1) != 0x1);
					reg_val = mctl_read_w(PGSR0);
					if((reg_val>>21)&0x7f)
					{
						//dram_dbg("DRAM ERROR\n");
						return DRAM_RET_FAIL;
					}else
					{
						//dram_dbg("DRAM single rank training OK\n");
					}

				}else if((byte0_state==0x2)&&(byte1_state==0x2)&&(byte2_state==0x3)&&(byte3_state==0x3))
				{
					//dram_dbg("DRAM single rank half DQ!\n");
					reg_val = mctl_read_w(MC_WORK_MODE);
					reg_val &=~(0x1<<0);
					reg_val &=~(0x7<<12);
					mctl_write_w(reg_val,MC_WORK_MODE);
					mctl_write_w(0,DXnGCR0(2));
					mctl_write_w(0,DXnGCR0(3));
				
					//dram_dbg("DRAM half DQ mode,disable high DQ\n");
					para->dram_para2 &= ~(0x1<<12);
					para->dram_para2 &=~(0xf<<0);
					para->dram_para2 |= (0x1<<0);
					//single training setting
					reg_val = mctl_read_w(DTCR);
					reg_val &= ~(0x3<<24);
					reg_val |= (0x1<<24);
					mctl_write_w(reg_val,DTCR);
					//re-train
					mctl_write_w(0x401,PIR);
					while((mctl_read_w(PGSR0 )&0x1) != 0x1);
					reg_val = mctl_read_w(PGSR0);
				if((reg_val>>21)&0x7f)
					{
						//dram_dbg("DRAM ERROR\n");
						return DRAM_RET_FAIL;
					}else
					{
						//dram_dbg("DRAM single rank half DQ training OK\n");
					}
				}else
				{
					//dram_dbg("DRAM ERROR\n");
					return DRAM_RET_FAIL;
				}

			}else
			{
				//dram_dbg("DRAM ERROR!\n");
				return DRAM_RET_FAIL;
			}
		}else
		{
			if(!(para->dram_tpr13&0x1))
			{
				//dram_dbg("DRAM auto scan size success!\n");
				para->dram_para2 |= (0x1<<12);
				para->dram_para2 &=~(0xf<<0);
			}
		}

//*****************************************************************************************************//
	while((mctl_read_w(STATR )&0x1) != 0x1);

	// liuke added for refresh debug
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,RFSHCTL0);
	dram_udelay(10);
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val&=~(0x1U<<31);
	mctl_write_w(reg_val,RFSHCTL0);
	dram_udelay(10);

//for lpddr2/lpddr3,if use gqs gate,can extend the dqs gate after training
	if((para->dram_type) == 6 || (para->dram_type) == 7)
	{
		if(dqs_gating_mode==1)//open dqs gate and need dqs training
		{
			//this mdoe ,dqs pull down ,dqs# pull up,extend 1
			reg_val =mctl_read_w(DXCCR);
			reg_val &=~(0x3<<6);
			reg_val |= (0x1<<6);
			mctl_write_w(reg_val,DXCCR);
		}
	}
//*****************************************************************************************************//
//set PGCR3,CKE polarity
	mctl_write_w(0x00aa0060,PGCR3);
//power down zq	calibration module for power save
	reg_val = mctl_read_w(ZQCR);
	reg_val |= (0x1U<<31);
	mctl_write_w(reg_val, ZQCR);
//enable master access
	mctl_write_w(0xffffffff,MC_MAER);
	return DRAM_RET_OK;
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
unsigned int DRAMC_get_dram_size()
{
	unsigned int reg_val;
	unsigned int dram_size;
	unsigned int temp;

    reg_val = mctl_read_w(MC_WORK_MODE);

    temp = (reg_val>>8) & 0xf;	//page size code
    dram_size = (temp - 6);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>4) & 0xf;	//row width code
    dram_size += (temp + 1);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>2) & 0x3;	//bank number code
    dram_size += (temp + 2);	//(1<<dram_size) * 512Bytes

    temp = reg_val & 0x3;	//rank number code
    dram_size += temp;	//(1<<dram_size) * 512Bytes

    dram_size = dram_size - 11;	//(1<<dram_size)MBytes

    return (1<<dram_size);
}
void auto_detect_dram_size(__dram_para_t *para)
{
	unsigned int i=0,j=0;
	unsigned int reg_val=0,ret=0,cnt=0;
	for(i=0;i<64;i++)
	{
		mctl_write_w((i%2)?(0x40000000 + 4*i):(~(0x40000000 + 4*i)),0x40000000 + 4*i);
	}
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<8,0x6<<8);//512B
	reg_val&=(~(0x3<<2));//set 4 bank
	reg_val&=(~(0x1<<15));//rank row bank col
	reg_val|=(0xf<<4);//16 row
	mctl_write_w(reg_val,MC_WORK_MODE);
	//------------------------------row detect--------------------------------------------
	for(i=11;i<=16;i++)
	{
		ret = 0x40000000 + (1<<(i+2+9));//row-bank-column
		cnt = 0;
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
		{
			break;
		}
	}
	if(i >= 16)
		i = 16;
	paraconfig(&(para->dram_para1), 0xffU<<20, i<<20);//row width confirmed
	//--------------------------pagesize(column)detect-----------------------------------
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<4,0xa<<4);//11rows
	paraconfig(&reg_val,0xf<<8,0xa<<8);//8KB
	mctl_write_w(reg_val,MC_WORK_MODE);
	for(i=9;i<=13;i++)
	{
		ret = 0x40000000 + (0x1U<<i);//column
		cnt = 0;
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
		{
			break;
		}
	}
	if(i >= 13)
		i = 13;
	if(i==9)
		i = 0;
	else
		i = (0x1U<<(i-10));
	paraconfig(&(para->dram_para1), 0xfU<<16, i<<16);//pagesize confirmed
	//------------------------------bank detect--------------------------------------------
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<8,0x6<<8);//pagesize set 512B
	reg_val|=(0x1<<2);//set 8 bank
	mctl_write_w(reg_val,MC_WORK_MODE);
	if(para->dram_type==2)//ddr2 bank maybe 4
	{
		ret = 0x40000000 + (0x1U<<2+9);//bank-col
		cnt = 0;
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
		{
			paraconfig(&(para->dram_para1), 0xfU<<28, 0<<28);//4 bank confirmed
		}
		else
		{
			paraconfig(&(para->dram_para1), 0xfU<<28, 1<<28);//8 bank confirmed
		}
	}

}

u32 dramc_simple_wr_test(unsigned int dram_size,unsigned int test_length)
{
	/* DRAM Simple write_read test
	 * 2 ranks:  write_read rank0 and rank1;
	 * 1rank  : write_read rank0 base and half size;
	 * */
	unsigned int i;
	unsigned int half_size;
	unsigned int val;
	half_size = ((dram_size >> 1)<<20);
	for(i=0;i<test_length;i++)
	{
		mctl_write_w(0x12345678 + i,(DRAM_MEM_BASE_ADDR + i*4));//rank0
		mctl_write_w(0x87654321 + i,(DRAM_MEM_BASE_ADDR + half_size + i*4));//half size (rank1)
	}
	for(i=0;i<test_length;i++)
	{
		val = mctl_read_w(DRAM_MEM_BASE_ADDR + half_size + i*4);
		if(val !=(0x87654321 + i))	/* Write last,read first */
		{
			//dram_dbg("DRAM simple test FAIL.\n");
			//dram_dbg("%x != %x at address %x\n",val,0x87654321 + i,DRAM_MEM_BASE_ADDR + half_size + i*4);
//			pattern_goto(i);
//			while(1);
			return DRAM_RET_FAIL;
		}
		val = mctl_read_w(DRAM_MEM_BASE_ADDR + i*4);
		if(val != (0x12345678+i))
		{
			//dram_dbg("DRAM simple test FAIL.\n");
			//dram_dbg("%x != %x at address %x\n",val,0x12345678 + i,DRAM_MEM_BASE_ADDR + i*4);
//			while(1);
			return DRAM_RET_FAIL;
//			pattern_goto(i+1);
		}

	}
	//dram_dbg("DRAM simple test OK.\n");
	return DRAM_RET_OK;
}

//*****************************************************************************
//	signed int init_DRAM(int type)
//  Description:	System init dram
//
//	Arguments:		type:	0: no lock		1: get the fixed parameters & auto detect & lock
//
//	Return Value:	1: fail
//					others: pass
//*****************************************************************************
signed int init_DRAM(int type, __dram_para_t *para)
{
	unsigned int ret_val=0;
	unsigned int reg_val=0;
	unsigned int i=0;
	unsigned int dram_size = 0;
	UART_printf2("ZW DRAM DRIVE INFO: V0.1\n");
	UART_printf2("DRAM Type = %d (2:DDR2,3:DDR3,6:LPDDR2,7:LPDDR3)\n", para->dram_type);
	UART_printf2("DRAM CLK = %d MHz\n", para->dram_clk);
	UART_printf2("DRAM zq value: %x\n",para->dram_zq);
	//**********************************************************
//	mctl_write_w(0x80001300, 0x01c20000);//SET PLL1 
//	mctl_write_w(0x80041810, 0x01c20028);//SET PLL6  600M
//	mctl_write_w( 0x20001, 0x01c20050);//SET AXI 400M
// 	mctl_write_w(0x00003180, 0x01c20054);//SET AHB APB
//	mctl_write_w(0x1000000, 0x01c20058);//SET APB2
	
//	mctl_write_w(0x81000003, 0x01c2015C);//MBUS 
//	dram_udelay(2000);
	//**********************************************************
	//ddr_voltage_set(para);
	mctl_sys_init(para);
	ret_val = mctl_channel_init(0,para);
	if(ret_val == DRAM_RET_FAIL)
	{
		return DRAM_RET_FAIL;
	}

	reg_val = mctl_read_w(MC_CCCR);
	reg_val |= (0x1U)<<31;	//after initial before write or read must clear credit value
	mctl_write_w(reg_val,MC_CCCR);
	dram_udelay(10);

	if(!(para->dram_tpr13&0x1))
		auto_detect_dram_size(para);
	mctl_com_init(para);
	dram_size= DRAMC_get_dram_size();
	UART_printf2("DRAM SIZE =%d M\n", dram_size);
	para->dram_para1|=(dram_size);
	para->dram_tpr13|=(0x1<<0);
	//set DRAM ODT MAP
	reg_val = (mctl_read_w(MC_WORK_MODE)&0x1);
	if(reg_val)
	{
		mctl_write_w(0x00000303,ODTMAP);
		//dram_dbg("set two rank ODTMAP\n");
	}else
	{
		mctl_write_w(0x00000201,ODTMAP);
		//dram_dbg("set one rank ODTMAP\n");
	}
	dram_udelay(1);
	//enable lpddr3 write odt
	if((para->dram_type==7)&&((para->dram_odt_en>>2)&0x1))
	{
		mctl_write_w(0x01000b02,MRCTRL0);
		mctl_write_w(0x81000b02,MRCTRL0);
		dram_udelay(100);
		while(((mctl_read_w(MRCTRL0)>>31)&0x1) != 0x0);
		//dram_dbg("lpddr3 write odt is 120 ohm\n");
	}
			
	ret_val = dramc_simple_wr_test(dram_size,0x1000);
	if(ret_val == DRAM_RET_FAIL)
		return DRAM_RET_FAIL;
	else
		return dram_size;
}

//*****************************************************************************
//	unsigned int mctl_init()

//  Description:	FOR SD SIMULATION
//
//	Arguments:		None
//
//	Return Value:	1: Success		0: Fail
//*****************************************************************************
unsigned int mctl_init(void)
{
	signed int ret_val=0;

	__dram_para_t dram_para;
	//set the parameter
	dram_para.dram_clk			= 504;	//default:480
	dram_para.dram_type			= 3;	//dram_type	DDR2: 2	DDR3: 3	LPDDR2: 6  LPDDR3: 7
	dram_para.dram_zq			= 0x3b3bfb;
	dram_para.dram_odt_en       = 1;
	dram_para.dram_para1		= 0x10E40000;//according to design
	dram_para.dram_para2		= 0x0000;
	dram_para.dram_mr0			= 0x1840;
	dram_para.dram_mr1			= 0x40;
	dram_para.dram_mr2			= 0x18;
	dram_para.dram_mr3			= 0x2;
	dram_para.dram_tpr0 		= 0x0048A192;
	dram_para.dram_tpr1 		= 0x01C2418D;
	dram_para.dram_tpr2 		= 0x00076051;
	dram_para.dram_tpr3 		= 0x0;
	dram_para.dram_tpr4 		= 0x0;
	dram_para.dram_tpr5         = 0x0;
	dram_para.dram_tpr6         = 0x0;
	dram_para.dram_tpr7         = 0x0;
	dram_para.dram_tpr8         = 0x0;
	dram_para.dram_tpr9         = 0x0;
	dram_para.dram_tpr10       	= 0x00;	
	dram_para.dram_tpr11        = 0x00;	
	dram_para.dram_tpr12       	= 0x9999;
	dram_para.dram_tpr13       	= 0x800001;
	ret_val = init_DRAM(0, &dram_para);//signed int init_DRAM(int type, void *para);
	return ret_val;
}

