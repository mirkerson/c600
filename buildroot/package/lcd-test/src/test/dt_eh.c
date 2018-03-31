
#include "dt.h"


#define FB_WIDTH 800
#define FB_HEIGHT 480
#define DOUBLE_BUFFER 0 //0/1
#define SCREEN_A 0

int main(void)
{
	unsigned long arg[4];
	int dispfh,memfh,fbfh0,fbfh1;
	unsigned long layer_hdl0, layer_hdl1;
	void *mem_addr0;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	__disp_color_t bk_color;
	__disp_fb_create_para_t fb_para;
	__disp_rect_t scn_win;
	__s32 i = 0;

	
	if((dispfh = open("/dev/disp",O_RDWR)) == -1)
	{
		printf("open file /dev/disp fail. \n");
		return 0;
	}

	
	if((memfh= open("./memin.bin",O_RDONLY)) == -1)
	{
		printf("open file /drv/memin.bin fail. \n");
		return 0;
	}


#if 1
    arg[0] = SCREEN_A;
	ioctl(dispfh,DISP_CMD_LCD_ON,(unsigned long)arg);
#endif


	bk_color.red = 0xff;
	bk_color.green = 0x00;
	bk_color.blue = 0x00;
	arg[0] = SCREEN_A;
	arg[1] = (unsigned long)&bk_color;
	ioctl(dispfh,DISP_CMD_SET_BKCOLOR,(unsigned long)arg);

//fb0
	fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
	fb_para.smem_len = FB_WIDTH * FB_HEIGHT * 4/*32bpp*/ * (DOUBLE_BUFFER+1);
	fb_para.ch1_offset = 0;
	fb_para.ch2_offset = 0;
	arg[0] = SCREEN_A;
	arg[1] = (unsigned long)&fb_para;
	layer_hdl0 = ioctl(dispfh,DISP_CMD_FB_REQUEST,(unsigned long)arg);
	if(layer_hdl0 <= 0)
	{
		printf("request fb fail\n");
	}

	
	printf("open frame buffer device.");
	if((fbfh0 = open("/dev/fb0",O_RDWR)) > 0)
	{
		printf("open fb0 ok\n");
	}
	else
	{
            printf("open fb0 fail!!!\n");	
            close(dispfh);
            return 0;
	}

    ioctl(fbfh0,FBIOGET_FSCREENINFO,&fix);

    mem_addr0 = mmap(NULL, fix.smem_len,PROT_READ | PROT_WRITE, MAP_SHARED, fbfh0, 0);
    memset(mem_addr0,0x80,fix.smem_len);
    read(memfh,mem_addr0,fix.smem_len/(DOUBLE_BUFFER+1),0);
	
    ioctl(fbfh0,FBIOGET_VSCREENINFO,&var);
    var.xoffset= 0;
    var.yoffset= 0;
    var.xres = 800;
    var.yres = 480;
    var.xres_virtual = FB_WIDTH;
    var.yres_virtual = FB_HEIGHT*(DOUBLE_BUFFER+1);
    var.nonstd = 0;
    var.bits_per_pixel = 32;
    var.transp.length = 8;
    var.red.length = 8;
    var.green.length = 8;
    var.blue.length = 8;
    //var.reserved[0] = DISP_MOD_INTERLEAVED;
    //var.reserved[1] = DISP_FORMAT_ARGB8888;
    //var.reserved[2] = DISP_SEQ_BGRA;
    //var.reserved[3] = 0;
    var.activate = FB_ACTIVATE_FORCE;
    ioctl(fbfh0,FBIOPUT_VSCREENINFO,&var);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_ENHANCE_ON,(unsigned long)arg);

    for(i=0; i<100; i+=10)
    {
        printf("press for bright %d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = i;
        ioctl(dispfh,DISP_CMD_SET_BRIGHT,(unsigned long)arg);
    }
    arg[0] = SCREEN_A;
    arg[1] = 50;
    ioctl(dispfh,DISP_CMD_SET_BRIGHT,(unsigned long)arg);
        
    for(i=0; i<100; i+=10)
    {
    printf("press for contrast %d\n", i);
    getchar();

    arg[0] = SCREEN_A;
    arg[1] = i;
    ioctl(dispfh,DISP_CMD_SET_CONTRAST,(unsigned long)arg);
    }
    arg[0] = SCREEN_A;
    arg[1] = 50;
    ioctl(dispfh,DISP_CMD_SET_CONTRAST,(unsigned long)arg);

    for(i=0; i<100; i+=10)
    {
        printf("press for saturation %d\n", i);
        getchar();

        arg[0] = SCREEN_A;
        arg[1] = i;
        ioctl(dispfh,DISP_CMD_SET_SATURATION,(unsigned long)arg);
    }
    arg[0] = SCREEN_A;
    arg[1] = 50;
    ioctl(dispfh,DISP_CMD_SET_SATURATION,(unsigned long)arg);

    printf("press any key to exit\n");
    getchar();

    arg[0] = SCREEN_A;
    arg[1] = layer_hdl0;
    ioctl(dispfh,DISP_CMD_FB_RELEASE,(unsigned long)arg);

    arg[0] = SCREEN_A;
    ioctl(dispfh,DISP_CMD_LCD_OFF,(unsigned long)arg);

    close(memfh);
    close(fbfh0);
    close(dispfh);
    return 0;
}
