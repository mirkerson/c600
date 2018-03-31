#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

int w,h,bpp;
#if 0
void fb_pixel(unsigned int *fbmem,int w,
		int x,int y,int color)
{
//	unsigned int *pos;
	unsigned short *fbmem_16;

	fbmem_16 = (short *)fbmem;
	if(bpp == 32)
		fbmem[y*w + x] = color;
	else if(bpp == 16)
		fbmem_16[y*w + x] = color;
//	*pos = color;
}

void fb_hline(unsigned int *fbmem,int w,
		int x1,int y,int x2,int color)
{
	int i;
	for (i = x1; i < x2; i++)
		fb_pixel(fbmem,w,i,y,color);
}
#endif
/*======================rgb====================================*/
void rgb16_to24(unsigned short *buf16,unsigned char * buf24)
{
	int r16,g16,b16;

	b16 = (buf16[0] & 0x1f);
	g16 = (buf16[0] & 0x7e0)  >> 5;
	r16 = (buf16[0] & 0xF800) >>11;
	
	buf24[0] = (r16 * 0xFF)/ 0x1F;	
	buf24[1] = (g16 * 0xFF)/ 0x3F;	
	buf24[2] = (b16 * 0xFF)/ 0x1F;	
	
}

unsigned char * buf16_to24(unsigned int * buf,int w,int h)
{
	unsigned char *buf24,*tmp;
	unsigned short *buf16;
	int i;
	int size = w * h * 3;

	buf16 = (short *)buf;

	buf24 = malloc (w * h * 3 );
	tmp = buf24;
	for (i = 0;i < w * h ;i ++)
	{
		rgb16_to24(buf16,buf24);	
		buf16++;	
		buf24 +=3;
	}

	return tmp;
}

unsigned char * rgb32to24(unsigned  int * buf,int w,int h)
{
	int i,j;
	unsigned char *rgbbuf;
	unsigned char *buf_8;

	buf_8 = (unsigned char *)buf;

	rgbbuf = malloc(w * h * 3);
	if(rgbbuf==NULL)
	{
		printf("can't malloc rgbbuf\n");
	}
	for (j = 0; j< h;j++)
	{
		for (i = 0 ; i < w ;i ++)
		{
			rgbbuf[j*w * 3 + i*3+2]     = buf_8[j*w*4+i*4 ];
			rgbbuf[j*w * 3 + i*3+1]   = buf_8[j*w*4+i*4+1];
			rgbbuf[j*w * 3 + i*3]   = buf_8[j*w*4+i*4+2];
		}
	}
	return rgbbuf;
}

int main (char argc,char **argv)
{
	int fd;
	unsigned int * fbmem;
//	int w,h,bpp;
	struct fb_var_screeninfo fb_var;
	unsigned char *rgbbuf;
    short image_w,image_h;

	if (argc < 3)
	{
		fprintf(stderr,"\nsudo to root!\n",argv[0]);
		fprintf(stderr,"usage:%s /dev/fbx *.jpg\n",argv[0]);
		exit(1);
	}

	//1.) open fb
	fd = open (argv[1],O_RDWR);
	if (fd <0)
	{
		printf ("open %s error!\n",argv[1]);
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


	printf ("Framebuffer :%d x %d-%d bpp,capture %dbits\n",w,h,bpp,bpp);

	//4. adjust bpp of display card 
	if(bpp == 16)
		rgbbuf = buf16_to24(fbmem,w,h);
	else if(bpp == 32)
		rgbbuf = rgb32to24(fbmem,w,h);

	//5. encode jpeg
	encode_jpeg(argv[2],rgbbuf,w,h);

	//6. print the information of capture picture
	printf("captured one picture that named:%s\n",argv[2]);

	close (fd);

	return 0;
}
