#ifndef __DEV_LCD_H__
#define __DEV_LCD_H__
#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "asm-generic/int-ll64.h"
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>   
#include <linux/kthread.h> 
#include <linux/err.h> 
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>

//#include <mach/platform.h>
#include <linux/sys_config.h>

#include <linux/drv_display.h>
#include "lcd_panel_cfg.h"

extern void LCD_get_panel_funs_0(__lcd_panel_fun_t * fun);
extern void LCD_get_panel_funs_1(__lcd_panel_fun_t * fun);
extern void LCD_set_panel_funs(__lcd_panel_fun_t * lcd0_cfg, __lcd_panel_fun_t * lcd1_cfg);
extern __s32 Fb_Init(__u32 from);
extern __s32 Fb_Exit(void);
extern __s32 DRV_DISP_Init(void);
extern __s32 DRV_DISP_Exit(void);

#define OSAL_PRINTF(msg...) {printk(KERN_WARNING "[LCD] ");printk(msg);}
#define __inf(msg...)       {printk(KERN_WARNING "[LCD] ");printk(msg);}
#define __msg(msg...)       {printk(KERN_WARNING "[LCD] file:%s,line:%d:    ",__FILE__,__LINE__);printk(msg);}
#define __wrn(msg...)       {printk(KERN_WARNING "[LCD WRN] file:%s,line:%d:    ",__FILE__,__LINE__); printk(msg);}
#define __here__            {printk(KERN_WARNING "[LCD] file:%s,line:%d\n",__FILE__,__LINE__);}


#endif
