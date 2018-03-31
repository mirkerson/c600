

#include "dt.h"


#define FB_WIDTH 800
#define FB_HEIGHT 480

#define SCREEN_A 1
#define SCREEN_B 0

int screenA_test(int dispfh, int screen_id)
{
    unsigned long arg[4];
    int memfh;
    unsigned long layer_hdl, fb_hdl, fbfh;
    void *mem_addr0;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    __disp_color_t bk_color;
    __disp_fb_create_para_t fb_para;

    //request fb0
    fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
    fb_para.smem_len = 800 * 480 * 4 * 2;
    fb_para.ch1_offset = 0;
    fb_para.ch2_offset = 0;
    arg[0] = screen_id;
    arg[1] = (unsigned long)&fb_para;
    fb_hdl = ioctl(dispfh,DISP_CMD_FB_REQUEST,(unsigned long)arg);
    if(fb_hdl <= 0)
    {
        printf("request fb fail\n");
    }
    
    if((fbfh = open("/dev/fb0",O_RDWR)) > 0)
    {
        printf("open fb0 ok\n");
    }
    else
    {
        printf("open fb0 fail!!!\n"); 
        
    }

    if(screen_id == 0)
    {
        if((memfh= open("./memin_0.bin",O_RDONLY)) == -1)
        {
            printf("open file /drv/memin.bin fail. \n");
            return 0;
        }
    }
    else
    {
        if((memfh= open("./memin_1.bin",O_RDONLY)) == -1)
        {
            printf("open file /drv/memin.bin fail. \n");
            return 0;
        }
    }

    ioctl(fbfh,FBIOGET_LAYER_HDL,&layer_hdl);

#if 0
    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_LCD_ON,(unsigned long)arg);
#endif

#if 1
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);
#endif

#if 0
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_HDMI_SET_MODE,(unsigned long)arg);

    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_HDMI_ON,(unsigned long)arg);
#endif

#if 0
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_VGA_SET_MODE,(unsigned long)arg);

    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_VGA_ON,(unsigned long)arg);
#endif

    bk_color.red = 0xff;
    bk_color.green = 0x00;
    bk_color.blue = 0x00;
    arg[0] = screen_id;
    arg[1] = (unsigned long)&bk_color;
    ioctl(dispfh,DISP_CMD_SET_BKCOLOR,(unsigned long)arg);

    ioctl(fbfh,FBIOGET_FSCREENINFO,&fix);
    mem_addr0 = mmap(NULL, fix.smem_len,PROT_READ | PROT_WRITE, MAP_SHARED, fbfh, 0);
    memset(mem_addr0,0xff,fix.smem_len);
    read(memfh,mem_addr0,fix.smem_len,0);
    
    ioctl(fbfh,FBIOGET_VSCREENINFO,&var);
    var.xoffset= 0;
    var.yoffset= 0;
    var.xres = 800;
    var.yres = 480;
    var.xres_virtual = FB_WIDTH;
    var.yres_virtual = FB_HEIGHT;
    var.nonstd = 0;
    var.bits_per_pixel = 32;
    var.transp.length = 8;
    var.red.length = 8;
    var.green.length = 8;
    var.blue.length = 8;
    var.activate = FB_ACTIVATE_FORCE;
    ioctl(fbfh,FBIOPUT_VSCREENINFO,&var);
    
    close(memfh);
    close(fbfh);

    return fb_hdl;
}


int screenB_test(int dispfh, int screen_id)
{
    unsigned long arg[4];
    int memfh;
    unsigned long layer_hdl, fb_hdl, fbfh;
    void *mem_addr0;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    __disp_color_t bk_color;
    __disp_rect_t scn_win;
    __disp_fb_create_para_t fb_para;
    int i=0;
    
//request fb1
    fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
    fb_para.smem_len = 800 * 480 * 4 * 2;
    fb_para.ch1_offset = 0;
    fb_para.ch2_offset = 0;
    arg[0] = screen_id;
    arg[1] = (unsigned long)&fb_para;
    fb_hdl = ioctl(dispfh,DISP_CMD_FB_REQUEST,(unsigned long)arg);
    if(fb_hdl <= 0)
    {
        printf("request fb fail\n");
    }
    
    if((fbfh = open("/dev/fb1",O_RDWR)) > 0)
    {
        printf("open fb1 ok\n");
    }
    else
    {
        printf("open fb1 fail!!!\n"); 
        
    }

    if(screen_id == 0)
    {
        if((memfh= open("./memin_0.bin",O_RDONLY)) == -1)
        {
            printf("open file /drv/memin.bin fail. \n");
            return 0;
        }
    }
    else
    {
        if((memfh= open("./memin_1.bin",O_RDONLY)) == -1)
        {
            printf("open file /drv/memin.bin fail. \n");
            return 0;
        }
    }
    
    ioctl(fbfh,FBIOGET_LAYER_HDL,&layer_hdl);


#if 0
    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_LCD_ON,(unsigned long)arg);
#endif

#if 0
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_TV_SET_MODE,(unsigned long)arg);

    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_TV_ON,(unsigned long)arg);
#endif

#if 1
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_HDMI_SET_MODE,(unsigned long)arg);

    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_HDMI_ON,(unsigned long)arg);
#endif

#if 0
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    ioctl(dispfh,DISP_CMD_VGA_SET_MODE,(unsigned long)arg);

    arg[0] = screen_id;
    ioctl(dispfh,DISP_CMD_VGA_ON,(unsigned long)arg);
#endif


    bk_color.red = 0xff;
    bk_color.green = 0x00;
    bk_color.blue = 0x00;
    arg[0] = screen_id;
    arg[1] = (unsigned long)&bk_color;
    ioctl(dispfh,DISP_CMD_SET_BKCOLOR,(unsigned long)arg);
        
    ioctl(fbfh,FBIOGET_FSCREENINFO,&fix);
    mem_addr0 = malloc(fix.smem_len);
    memset(mem_addr0,0xff,fix.smem_len);
    read(memfh,mem_addr0,fix.smem_len,0);
    write(fbfh,mem_addr0,fix.smem_len,0);
    free(mem_addr0);
    
    ioctl(fbfh,FBIOGET_VSCREENINFO,&var);
    var.xoffset= 0;
    var.yoffset= 0;
    var.xres = 800;
    var.yres = 480;
    var.xres_virtual= 800;
    var.yres_virtual= 480;
    var.nonstd = 0;
    var.bits_per_pixel = 32;
    var.transp.length = 8;
    var.red.length = 8;
    var.green.length = 8;
    var.blue.length = 8;
    var.activate = FB_ACTIVATE_FORCE;
    ioctl(fbfh,FBIOPUT_VSCREENINFO,&var);

    for(i=0; i<11; i++)
    {
        printf("press any key for screen %d,hdmi:%d\n", screen_id, i);
        getchar();

        arg[0] = screen_id;
        ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);
        
        arg[0] = screen_id;
        arg[1] = i;
        ioctl(dispfh,DISP_CMD_HDMI_SET_MODE,(unsigned long)arg);

        arg[0] = screen_id;
        ioctl(dispfh,DISP_CMD_HDMI_ON,(unsigned long)arg);
    }

    close(memfh);
    close(fbfh);

    return fb_hdl;
}


int main(void)
{
	int dispfh, fb_hdl0, fb_hdl1;
    unsigned long arg[4];

	if((dispfh = open("/dev/disp",O_RDWR)) == -1)
	{
		printf("open file /dev/disp fail. \n");
		return 0;
	}

    fb_hdl0 = screenA_test(dispfh, SCREEN_B);
    fb_hdl1 = screenB_test(dispfh, SCREEN_A);
    
    printf("press any key to exit\n");
    getchar();

    arg[0] = SCREEN_A;
    arg[1] = fb_hdl0;
    ioctl(dispfh,DISP_CMD_FB_RELEASE,(unsigned long)arg);

    arg[0] = SCREEN_B;
    arg[1] = fb_hdl1;
    ioctl(dispfh,DISP_CMD_FB_RELEASE,(unsigned long)arg);

    arg[0] = 0;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    arg[0] = 0;
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);
    arg[0] = 0;
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    arg[0] = 0;
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);

    arg[0] = 1;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);
    arg[0] = 1;
    ioctl(dispfh,DISP_CMD_TV_OFF,(unsigned long)arg);
    arg[0] = 1;
    ioctl(dispfh,DISP_CMD_VGA_OFF,(unsigned long)arg);
    arg[0] = 1;
    ioctl(dispfh,DISP_CMD_HDMI_OFF,(unsigned long)arg);

	close(dispfh);
	return 0;
}
