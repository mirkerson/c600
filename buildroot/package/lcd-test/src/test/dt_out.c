
#include "dt.h"

#define FB_WIDTH 800
#define FB_HEIGHT 480
#define DOUBLE_BUFFER 1 //0/1
#define SCREEN_A 0

#define DISP_OUT_LCD
//#define DISP_OUT_HDMI
//#define DISP_OUT_VGA
//#define DISP_OUT_TV

int main(void)
{
	int dispfh, fbfh0;
    __disp_fb_create_para_t fb_para;
    unsigned long arg[4];
    int memfh;
    unsigned long layer_hdl;
    void *mem_addr0;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    __disp_color_t bk_color;
    int i = 0;
    int ret = 0;
    __disp_rect_t scn_win;
    
	if((dispfh = open("/dev/disp",O_RDWR)) == -1)
	{
		printf("open file /dev/disp fail. \n");
		return 0;
	}

    if((memfh= open("pic/memin_shg.bin",O_RDONLY)) == -1)
    {
        printf("open picture file  fail. \n");
        return 0;
    }

#ifdef DISP_OUT_LCD
    arg[0] = SCREEN_A;
	ioctl(dispfh,DISP_CMD_LCD_ON,(unsigned long)arg);
#endif

#ifdef DISP_OUT_HDMI
    arg[0] = SCREEN_A;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_HDMI_SET_MODE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_HDMI_ON,(unsigned long)arg);
#endif

#ifdef DISP_OUT_VGA
    arg[0] = SCREEN_A;
	arg[1] = DISP_VGA_H800_V600;
	ioctl(dispfh,DISP_CMD_VGA_SET_MODE,(unsigned long)arg);

	arg[0] = SCREEN_A;
	ioctl(dispfh,DISP_CMD_VGA_ON,(unsigned long)arg);
#endif

#ifdef DISP_OUT_TV
    arg[0] = SCREEN_A;
    arg[1] = DISP_TV_MOD_1080P_60HZ;
    ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);
#endif

    bk_color.red = 0xff;
    bk_color.green = 0x00;
    bk_color.blue = 0x00;
    arg[0] = SCREEN_A;
    arg[1] = (unsigned long)&bk_color;
    ioctl(dispfh,DISP_CMD_SET_BKCOLOR,(unsigned long)arg);

//request fb0
	fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
	fb_para.smem_len = FB_WIDTH * FB_HEIGHT * 4/*32bpp*/ * (DOUBLE_BUFFER+1);
	fb_para.ch1_offset = 0;
	fb_para.ch2_offset = 0;
	arg[0] = SCREEN_A;
	arg[1] = (unsigned long)&fb_para;
	layer_hdl = ioctl(dispfh,DISP_CMD_FB_REQUEST,(unsigned long)arg);
    if(layer_hdl <= 0)
    {
        printf("request fb fail\n");
    }
    
    if((fbfh0 = open("/dev/fb0",O_RDWR)) > 0)
    {
        printf("open fb0 ok\n");
    }
    else
    {
        printf("open fb0 fail!!!\n"); 
        
    }
    ioctl(fbfh0,FBIOGET_FSCREENINFO,&fix);
    mem_addr0 = mmap(NULL, fix.smem_len,PROT_READ | PROT_WRITE, MAP_SHARED, fbfh0, 0);
    memset(mem_addr0,0xff,fix.smem_len/(DOUBLE_BUFFER+1));
    read(memfh,mem_addr0,fix.smem_len/(DOUBLE_BUFFER+1),0);
    ioctl(fbfh0,FBIOGET_VSCREENINFO,&var);
    var.xoffset= 0;
    var.yoffset= 0;
    var.xres = FB_WIDTH;
    var.yres = FB_HEIGHT;
    var.xres_virtual= FB_WIDTH;
    var.yres_virtual= FB_HEIGHT * (DOUBLE_BUFFER+1);
    var.nonstd = 0;
    var.bits_per_pixel = 32;
    var.transp.length = 8;
    var.red.length = 8;
    var.green.length = 8;
    var.blue.length = 8;
    var.activate = FB_ACTIVATE_FORCE;
    ioctl(fbfh0,FBIOPUT_VSCREENINFO,&var);

    scn_win.x = 0;
    scn_win.y = 0;
    scn_win.width = FB_WIDTH;
    scn_win.height = FB_HEIGHT;
    arg[0] = SCREEN_A;
    arg[1] = (unsigned long)layer_hdl;
    arg[2] = (unsigned long)&scn_win;
    ioctl(dispfh,DISP_CMD_LAYER_SET_SCN_WINDOW,(unsigned long)arg);

#if 0
    arg[0] = SCREEN_A;
    arg[1] = (unsigned long)layer_hdl;
    ioctl(dispfh,DISP_CMD_LAYER_ENHANCE_ON,(unsigned long)arg);

    for(i=0; i<64; i+=4)
    {
        printf("contrat:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = (unsigned long)layer_hdl;
        arg[2] = i;
        ioctl(dispfh,DISP_CMD_LAYER_SET_CONTRAST,(unsigned long)arg);
    }
    arg[0] = SCREEN_A;
    arg[1] = (unsigned long)layer_hdl;
    arg[2] = 32;
    ioctl(dispfh,DISP_CMD_LAYER_SET_CONTRAST,(unsigned long)arg);
#endif

#if 0
    printf("open vpp\n");
    getchar();

    arg[0] = SCREEN_A;
    arg[1] = (unsigned long)layer_hdl;
    ioctl(dispfh,DISP_CMD_LAYER_VPP_ON,(unsigned long)arg);

    for(i=4; i>=0; i--)
    {
        printf("luma sharp:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = (unsigned long)layer_hdl;
        arg[2] = i;
        ioctl(dispfh,DISP_CMD_LAYER_SET_LUMA_SHARP_LEVEL,(unsigned long)arg);
    }
     for(i=4; i>=0; i--)
    {
        printf("chroma sharp:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = (unsigned long)layer_hdl;
        arg[2] = i;
        ioctl(dispfh,DISP_CMD_LAYER_SET_CHROMA_SHARP_LEVEL,(unsigned long)arg);
    }
     for(i=4; i>=0; i--)
    {
        printf("white extern:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = (unsigned long)layer_hdl;
        arg[2] = i;
        ioctl(dispfh,DISP_CMD_LAYER_SET_WHITE_EXTERN_LEVEL,(unsigned long)arg);
    }
     for(i=4; i>=0; i--)
    {
        printf("black extern:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = (unsigned long)layer_hdl;
        arg[2] = i;
        ioctl(dispfh,DISP_CMD_LAYER_SET_BLACK_EXTERN_LEVEL,(unsigned long)arg);
    }
#endif

#if 0
    for(i=0; i<11; i++)
    {
        printf("press any key to vga %d\n",i);
        getchar();
        arg[0] = SCREEN_A;
        ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);

        arg[0] = SCREEN_A;
    	arg[1] = i;
    	ioctl(dispfh,DISP_CMD_VGA_SET_MODE,(unsigned long)arg);

    	arg[0] = SCREEN_A;
    	ioctl(dispfh,DISP_CMD_VGA_ON,(unsigned long)arg);
    }
#endif

#if 0
    //for(i=0; i<11; i++)
    {
        printf("press any key for hdmi:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);
        
        arg[0] = SCREEN_A;
        arg[1] = DISP_TV_MOD_720P_50HZ;
        ioctl(dispfh,DISP_CMD_HDMI_SET_MODE,(unsigned long)arg);

        arg[0] = SCREEN_A;
        ioctl(dispfh,DISP_CMD_HDMI_ON,(unsigned long)arg);
    }
#endif

#if 0
    for(i=0; i<11; i++)
    {
        printf("press any key for tv:%d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
        ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);
        
        arg[0] = SCREEN_A;
        arg[1] = i;
        ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

        arg[0] = SCREEN_A;
        ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);
    }
#endif

#if 0
    printf("press any key to vga\n");
    getchar();
    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);

    arg[0] = SCREEN_A;
    arg[1] = DISP_VGA_H800_V600;
    ioctl(dispfh,DISP_CMD_VGA_SET_MODE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_VGA_ON,(unsigned long)arg);

    printf("press any key to pal\n");
    getchar();
    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);

    arg[0] = SCREEN_A;
    arg[1] = DISP_TV_MOD_PAL;
    ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);

    printf("press any key to pal\n");
    getchar();
    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);

    arg[0] = SCREEN_A;
    arg[1] = DISP_TV_MOD_PAL;
    ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);


    printf("press any key to pal\n");
    getchar();
    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);

    arg[0] = SCREEN_A;
    arg[1] = DISP_TV_MOD_PAL;
    ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);

#endif
		
    printf("press any key to exit\n");
    getchar();

    arg[0] = SCREEN_A;
    arg[1] = layer_hdl;
    ioctl(dispfh,DISP_CMD_FB_RELEASE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);

    close(memfh);
    close(dispfh);
    close(fbfh0);
    return 0;
}
