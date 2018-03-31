#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

void fb_pixel(unsigned short *fbmem,int w,
		int x,int y,int color)
{
	unsigned short *pos;
	pos  = fbmem + y*w + x;
	*pos = color;
}

void fb_hline(unsigned short *fbmem,int w,
		int x1,int y,int x2,int color)
{
	int i;
	for (i = x1; i < x2; i++)
		fb_pixel(fbmem,w,i,y,color);
}
/*======================rgb====================================*/
unsigned short rgb24_to_rgb16(unsigned char r,unsigned char g,unsigned char b)
{
	int r16,g16,b16;
	unsigned short rgb16;

	b16 = (b * 0x1f) / 0xff ;
	g16 = (g * 0x3f) / 0xff ;
	r16 = (r * 0x1f) / 0xff ;
	
	
	rgb16 = (r16<<11)|(g16<<5)|(b16);
	
	return rgb16;
}

unsigned char * buf24_to16(unsigned char * buf,int w,int h)
{
	unsigned char *buf24;
	unsigned short *buf16;
	int i;
	int size = w * h * 3;

	buf16 = buf;

	for (i = 0;i < size ;i += 3)
	{
		*buf16 = rgb24_to_rgb16(buf[i],buf[i+1],buf[i+2]);	
		buf16++;	
	}

}
int main (void)
{
	int fd;
	unsigned short * fbmem;
	int w,h,bpp;
	struct fb_var_screeninfo fb_var;
	unsigned char *buf;
        short image_w,image_h;	

	//1.) open fb
	fd = open ("/dev/fb0",O_RDWR);
	if (fd <0)
	{
		printf ("open /dev/fb0 error!\n");
		return -1;
	}
	//2. get framebuffer information
	ioctl(fd,FBIOGET_VSCREENINFO, &fb_var);
	
	w   = fb_var.xres;
	h   = fb_var.yres;
	bpp = fb_var.bits_per_pixel;

	//3. mmap framebuffer;
	fbmem = mmap (0,w * h * bpp/8,PROT_READ|PROT_WRITE,
			MAP_SHARED,fd,0);
	
	if(fbmem <0)
	{
		printf ("mmap fb error!\n");
		return 0;
	}

	//4. draw pixel
	fb_pixel (fbmem,w,100,200,0xF800);
	fb_hline (fbmem,w,10,100,200,0x07E0);

	//5. decode jpeg
	buf = decode_jpeg("test.jpg",&image_w,&image_h);
	buf24_to16(buf,image_w,image_h);
	printf ("#define IMAGEW %d\n#define IMAGEH %d\n\n",image_w,image_h);
	printf ("unsigned char image[]={\n"); 
	{
		int i,j;
		for (j=0;j<image_h;j++)
		{
			for (i=0;i<image_w *2;i+=2)
			{
				printf ("0x%02x ,",buf[i + j*image_w *2]);	
				printf ("0x%02x ,",buf[i + j*image_w *2 + 1]);
				if (i%16 == 14)
					printf ("\n");	
			}
					
		}
	}
	      
	printf ("\n};\n"); 
	close (fd);
	return 0;
}
