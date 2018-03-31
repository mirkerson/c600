
//zw 
//for csi & isp test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>            

#include <fcntl.h>             
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>         
#include <linux/videodev2.h>
#include <time.h>

#define	__DISPLAY2

#if defined(__DISPLAY2)
#include <linux/fb.h>  
#include <sys/mman.h> 
#endif /* __DISPLAY2 */


#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

struct size{
	int width;
	int height;
};

struct buffer {
    void * start;
    size_t length;
};

#if defined(__DISPLAY2)

#define WIDTH  640
#define HEIGHT 480
unsigned char rgb[WIDTH*HEIGHT*3];

#endif /* __DISPLAY2 */


static char path_name[20] = {'\0'};
static char dev_name[20] = {'\0'};
static int      fd              = -1;
struct buffer *   buffers       = NULL;
static unsigned int   n_buffers	= 0;

struct size input_size;
struct size subch_size;

unsigned int  req_frame_num = 8;
unsigned int  read_num = 20;
unsigned int  count;

int buf_size[3]={0};

#if defined(__DISPLAY2)

typedef struct fb_dev   {  
    int fd;             /* 帧缓冲设备硬件描述符 */  
    void *pfb;          /* 指向帧缓冲映射到用户空间的首地址 */  
    int xres;           /* 一帧图像的宽度 */  
    int yres;           /* 一帧图像的高度 */  
    int size;           /* 一帧图像的大小 */  
    int bits_per_pixel; /* 每个像素的大小 */  
} fb_dev_t;  

fb_dev_t st_fb_dev;
fb_dev_t *fbd = NULL;

int disp2_init(void)
{
	struct fb_var_screeninfo vinfo;  
	
	fbd = &st_fb_dev;
	
	#define FB_DEVICE_NAME "/dev/fb0"  
	
	if((fbd->fd = open(FB_DEVICE_NAME, O_RDWR)) == -1)  {	
		printf("Error: Cannot open framebuffer device.\n");  
		_exit(EXIT_FAILURE);  
	}  

	/* 获取LCD 的可变参数 */  
	ioctl(fbd->fd, FBIOGET_VSCREENINFO, &vinfo);  

	fbd->xres = vinfo.xres;  
	fbd->yres = vinfo.yres;  
	fbd->bits_per_pixel = vinfo.bits_per_pixel;  

	/* 计算一帧图像的大小 */  
	fbd->size = fbd->xres * fbd->yres * fbd->bits_per_pixel / 8;  

	printf("%s: xres=%d, yres=%d, bpp=%d, fb_size = %d\n", \
		__func__, fbd->xres,fbd->yres,fbd->bits_per_pixel,fbd->size);  

	/* 将帧映射到内存 */  
	/* mmap的应用 */  
	/* mmap可以把文件内容映射到一段内存中，准确说是虚拟内存，通过对这段内存的读取和修改，实现对文件的读取和修改。 */  
	/* addr:指定映射的起始地址，通常为NULL，由系统指定 */  
	/* length:将文件的多大长度映射到内存 */  
	/* prot:映射区的保护方式，可以是可被执行(PROT_EXEC)，可被写入(PROT_WRITE)，可被读取(PROT_READ)，映射区不能存取(PROT_NONE) */  
	/* flags:映射区的特性，对映射区的写入数据会复制回文件，且允许其他映射文件的进城共享(MAP_SHARED)，对映射区的写入操作会产生一个映射的复制，对此区域所做的修改不会写会源文件(MAP_PRIVATE) */  
	/* fd:由open返回的文件描述符，代表要映射的文件 */  
	/* offset:以文件开始出的偏移，必须是分页大小的整数倍，通常为0，表示从头开始映射 */	

	/* 注意:在修改映射文件时，只能在原长度上修改，不能增加文件长度，因为内存是已经分配好的 */  
	  
	fbd->pfb = mmap(NULL, fbd->size, PROT_READ | PROT_WRITE, MAP_SHARED, fbd->fd, 0);  

	if((int)fbd->pfb == -1)  {	
		printf("Error: Failed to map frambuffer device to memory!\n");	
		return -1;
	}  

	printf("mmap ok, addr=0x%lx\n", (unsigned long)fbd->pfb);

	return 0;

}

int disp2_show(char *rgb888)  
{  
	int x,y;
	unsigned long color = 0;
	unsigned long size; 

	if(fbd == NULL)
		return -1;
	
	size = fbd->size;

	printf("xres=%d, yres=%d\n", fbd->xres, fbd->yres);
	#if 1
	for (y = 0; y < (480 - 1); y++) {
		for (x = 0; x < (640 - 1); x++) {
	#else
	for (y = 0; y < (fbd->yres - 1); y++) {
		for (x = 0; x < (fbd->xres - 1); x++) {
	#endif
			//*((unsigned int *)(fbd->pfb) + y*fbd->xres +x) = 0xffffffff;
			*((unsigned char *)((fbd->pfb) + y*3*fbd->xres + (x*3))) = *(rgb888 + y*3*640 + x*3);
			*((unsigned char *)((fbd->pfb) + y*3*fbd->xres + (x*3)+ 1)) = *(rgb888 + y*3*640 + x*3 + 1);
			*((unsigned char *)((fbd->pfb) + y*3*fbd->xres + (x*3)+ 2)) = *(rgb888 + y*3*640 + x*3 + 2);
		}
	}
	
	return 0;  
} 

int disp2_exit(void)
{
	if(fbd){
		munmap(fbd->pfb,fbd->size);  
  
    	/* 关闭设备文件 */  
    	close(fbd->fd);
	}
	return 0;
}

#endif /* __DISPLAY2 */

/********************YUV420P to RGB24*************************/   
// Conversion from YUV420 to RGB24   
static long int crv_tab[256];  
static long int cbu_tab[256];  
static long int cgu_tab[256];  
static long int cgv_tab[256];  
static long int tab_76309[256];  
static unsigned char clp[1024];   //for clip in CCIR601   

/****Initialize conversion table for YUV420 to RGB****/   
void InitConvertTable(void)    
{  
   long int crv,cbu,cgu,cgv;  
   int i,ind;    
      
   crv = 104597; cbu = 132201;  /* fra matrise i global.h */  
   cgu = 25675;  cgv = 53279;  
   
   for (i = 0; i < 256; i++) {  
      crv_tab[i] = (i-128) * crv;  
      cbu_tab[i] = (i-128) * cbu;  
      cgu_tab[i] = (i-128) * cgu;  
      cgv_tab[i] = (i-128) * cgv;  
      tab_76309[i] = 76309*(i-16);  
   }  
   
   for (i=0; i<384; i++)  
   clp[i] =0;  
   ind=384;  
   for (i=0;i<256; i++)  
    clp[ind++]=i;  
   ind=640;  
   for (i=0;i<384;i++)  
    clp[ind++]=255;  
}  
/****Convert from YUV420 to RGB24****/   
void yuv420_to_rgb24(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,int width,int height)  
{  
 int y1,y2,u,v;  
 unsigned char *py1,*py2;  
 int i,j, c1, c2, c3, c4;  
 unsigned char *d1, *d2;  
  
 py1=src0;  
 py2=py1+width;  
 d1=dst_ori;  
 d2=d1+3*width;  
  for (j = 0; j < height; j += 2) {  
  for (i = 0; i < width; i += 2) {  

  #if 1
   u = *src2++;	
   v = *src1++; 
  #else
   u = *src1++;  
   v = *src2++;  
  #endif
  
   c1 = crv_tab[v];  
   c2 = cgu_tab[u];  
   c3 = cgv_tab[v];  
   c4 = cbu_tab[u];  
  
   //up-left   
            y1 = tab_76309[*py1++];   
   *d1++ = clp[384+((y1 + c1)>>16)];   
   *d1++ = clp[384+((y1 - c2 - c3)>>16)];  
            *d1++ = clp[384+((y1 + c4)>>16)];  
  
   //down-left   
   y2 = tab_76309[*py2++];  
   *d2++ = clp[384+((y2 + c1)>>16)];   
   *d2++ = clp[384+((y2 - c2 - c3)>>16)];  
            *d2++ = clp[384+((y2 + c4)>>16)];  
  
   //up-right   
   y1 = tab_76309[*py1++];  
   *d1++ = clp[384+((y1 + c1)>>16)];   
   *d1++ = clp[384+((y1 - c2 - c3)>>16)];  
   *d1++ = clp[384+((y1 + c4)>>16)];  
  
   //down-right   
   y2 = tab_76309[*py2++];  
   *d2++ = clp[384+((y2 + c1)>>16)];   
   *d2++ = clp[384+((y2 - c2 - c3)>>16)];  
            *d2++ = clp[384+((y2 + c4)>>16)];  
  }  
  d1 += 3*width;  
  d2 += 3*width;  
  py1+=   width;  
  py2+=   width;  
 }        
}  

static int read_frame (int mode)
{
	struct v4l2_buffer buf;
	char fdstr[30];
	void * bfstart = NULL;
	FILE *file_fd = NULL;
	int i,num;
	
	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 ==ioctl (fd, VIDIOC_DQBUF, &buf) )   
		return -1;
	    
	assert (buf.index < n_buffers);
	
#if defined(__DISPLAY2)
		bfstart = buffers[buf.index].start;	
		yuv420_to_rgb24((unsigned char *)bfstart,
						(unsigned char *)(bfstart + buf_size[i]*2/3),
						(unsigned char *)(bfstart + buf_size[i]*2/3 + buf_size[i]/6),							
						rgb,WIDTH,HEIGHT);
		disp2_show(rgb);
#endif

#if 0
	if (count == read_num/2)
    {
		printf("file length = %d\n",buffers[buf.index].length);
		printf("file start = %x\n",buffers[buf.index].start); 
        
		num = (mode > 2) ? 2 : mode;
		bfstart = buffers[buf.index].start;
	  	
		for (i = 0; i <= num; i++)
		{	
			printf("file %d start = %p\n", i, bfstart); 
	
			sprintf(fdstr,"%s/fb%d_y%d.bin",path_name,i+1,mode);
			file_fd = fopen(fdstr,"w");
			fwrite(bfstart, buf_size[i]*2/3, 1, file_fd); 
			fclose(file_fd);

			sprintf(fdstr,"%s/fb%d_u%d.bin",path_name,i+1,mode);
			file_fd = fopen(fdstr,"w");
			fwrite(bfstart + buf_size[i]*2/3, buf_size[i]/6, 1, file_fd); 
			fclose(file_fd);

			sprintf(fdstr,"%s/fb%d_v%d.bin",path_name,i+1,mode);
			file_fd = fopen(fdstr,"w");
			fwrite(bfstart + buf_size[i]*2/3 + buf_size[i]/6, buf_size[i]/6, 1, file_fd); 
			fclose(file_fd);

			bfstart += ALIGN_4K( buf_size[i] );	
		}
	}
#endif	
	if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))
		return -1;
	return 0;
}

static int req_frame_buffers(void)
{
	unsigned int i;
	struct v4l2_requestbuffers req;
	
	CLEAR (req);
	req.count		= req_frame_num;
	req.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	= V4L2_MEMORY_MMAP;	
	
	ioctl (fd, VIDIOC_REQBUFS, &req); 

	buffers = calloc (req.count, sizeof (*buffers));

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
		struct v4l2_buffer buf;  
		CLEAR (buf);
		buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index		= n_buffers;

		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)) 
			printf ("VIDIOC_QUERYBUF error\n");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start  = mmap (NULL /* start anywhere */, 
								         buf.length,
								         PROT_READ | PROT_WRITE /* required */,
								         MAP_SHARED /* recommended */,
								         fd, buf.m.offset);
	
		if (MAP_FAILED == buffers[n_buffers].start)
		{
			printf ("mmap failed\n");
			return -1;
		}

	}

	for (i = 0; i < n_buffers; ++i) 
	{
		struct v4l2_buffer buf;
		CLEAR (buf);

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= i;

		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))
		{
			printf ("VIDIOC_QBUF failed\n");
			return -1;
		}
	}
	return 0;

}

static int free_frame_buffers(void)
{
	unsigned int i;

	for (i = 0; i < n_buffers; ++i) {
		if (-1 == munmap (buffers[i].start, buffers[i].length)) {
			printf ("munmap error");
			return -1;
		}
	}
	return 0;
}

static int camera_init(int sel, int mode)
{
	struct v4l2_input inp;
	struct v4l2_streamparm parms;
	
	fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	
	if(!fd) {
		printf("open falied\n");
		return -1;	
	}
	
	inp.index = sel;	
	if (-1 == ioctl (fd, VIDIOC_S_INPUT, &inp))
	{
		printf("VIDIOC_S_INPUT %d error!\n",sel);
		return -1;
	}
	
	//VIDIOC_S_PARM			
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator =30;
	parms.parm.capture.capturemode = V4L2_MODE_VIDEO; //V4L2_MODE_IMAGE
					
	if (-1 == ioctl (fd, VIDIOC_S_PARM, &parms)) 
	{
		printf ("VIDIOC_S_PARM error\n");	
		return -1;
	}
			
	return 0;
			
}

static int camera_fmt_set(int subch, int angle)
{
	struct v4l2_format fmt;
	struct v4l2_pix_format subch_fmt;
	
	//VIDIOC_S_FMT
	CLEAR (fmt);
	fmt.type                	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       	= input_size.width; 	//640; 
	fmt.fmt.pix.height      	= input_size.height; 	//480;
	fmt.fmt.pix.pixelformat 	= V4L2_PIX_FMT_YUV420;		//V4L2_PIX_FMT_YUV422P;//V4L2_PIX_FMT_NV12;//V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       	= V4L2_FIELD_NONE;			//V4L2_FIELD_INTERLACED;//V4L2_FIELD_NONE;
	fmt.fmt.pix.rot_angle	    = 0;

	if (0 == subch)
		fmt.fmt.pix.subchannel	= NULL;	
	else
	{
		fmt.fmt.pix.subchannel	= &subch_fmt;
		subch_fmt.width 		= subch_size.width;
		subch_fmt.height		= subch_size.height;
		subch_fmt.pixelformat	= V4L2_PIX_FMT_YUV420; 		//V4L2_PIX_FMT_YUV422P;//V4L2_PIX_FMT_NV12;//V4L2_PIX_FMT_YUYV;
		subch_fmt.field 		= V4L2_FIELD_NONE;			//V4L2_FIELD_INTERLACED;//V4L2_FIELD_NONE;
		subch_fmt.rot_angle 	= angle;
	}
			
	if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt))
	{
		printf("VIDIOC_S_FMT error!\n");
		return -1;
	}
	
	//Test VIDIOC_G_FMT	
	if (-1 == ioctl (fd, VIDIOC_G_FMT, &fmt)) 
	{
		printf("VIDIOC_G_FMT error!\n");
		return -1;
	}
	else
	{
		printf("resolution got from sensor = %d*%d\n",fmt.fmt.pix.width,fmt.fmt.pix.height);
	}
	return 0;

}

static int main_test (int sel, int mode)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	int subch = 0 ;
	int angle = 0;
                    
	if (mode >= 1) {
		subch = 1;
		if (mode == 2)
			angle = 90;
		else
			angle = 270;
	}

	if (-1== camera_init(sel, mode))	//camera select
		return -1;
	if (-1 == camera_fmt_set(subch, angle)) 
		return -1;
	if (-1 ==req_frame_buffers())
		return -1;

#if defined(__DISPLAY2)
	disp2_init();
	InitConvertTable();
#endif	
	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) 
	{		
		printf ("VIDIOC_STREAMON failed\n");
		return -1;
	}
	else 
		printf ("VIDIOC_STREAMON ok\n");
	        			
	count = read_num;	    
	while (count-->0)
	{
		for (;;) 
		{
			fd_set fds;
			struct timeval tv;
			int r;
				
			FD_ZERO (&fds);
			FD_SET (fd, &fds);
		
			tv.tv_sec = 2;			/* Timeout. */
			tv.tv_usec = 0;
			
			r = select (fd + 1, &fds, NULL, NULL, &tv);
			
			if (-1 == r) {
				if (EINTR == errno)
					continue;
				printf ("select err\n");
			}
			if (0 == r) {
				fprintf (stderr, "select timeout\n");
				return -1;
			}
			#if defined(__DISPLAY2)
			if(read_frame (mode)){
				printf("[app_basic] read frame ====error====!\n");
				return -1;			
			}
			#else
			if (!read_frame (mode))
				break;
			else 
				return -1;
			#endif
		}
	}

	if (-1 == ioctl (fd, VIDIOC_STREAMOFF, &type))
	{		
		printf ("VIDIOC_STREAMOFF failed\n");
		return -1;
	}
	else
		printf ("VIDIOC_STREAMOFF ok\n");

	if ( -1 == free_frame_buffers())
		return -1;

#if defined(__DISPLAY2)
	disp2_exit();
#endif

	close (fd);	
	
	return 0;
}

int main(int argc,char *argv[])
{
	int i,test_cnt = 1;
	int sel = 0;
	int width = 640;
	int height = 480;
    int mode = 1;
	
	CLEAR (dev_name);
    CLEAR (path_name);
    if( argc == 1 ) {
		sprintf(dev_name,"/dev/video0");
        sprintf(path_name,"/mnt/sdcard");
    }
    else if( argc == 3 ) {
		sel = atoi(argv[1]);
		sprintf(dev_name,"/dev/video%d",sel);
		sel = atoi(argv[2]);
        sprintf(path_name,"/mnt/sdcard");
    }
    else if( argc == 5 ) {
		sel = atoi(argv[1]);
		sprintf(dev_name,"/dev/video%d",sel);
		sel = atoi(argv[2]);
		width = atoi(argv[3]);
		height = atoi(argv[4]);
        sprintf(path_name,"/mnt/sdcard");
    }
    else if( argc == 6 ) {
		sel = atoi(argv[1]);
		sprintf(dev_name,"/dev/video%d",sel);
		sel = atoi(argv[2]);
		width = atoi(argv[3]);
		height = atoi(argv[4]);
        sprintf(path_name,"%s",argv[5]);
    }
    else if( argc == 7 ) {
		sel = atoi(argv[1]);
		sprintf(dev_name,"/dev/video%d",sel);
		sel = atoi(argv[2]);
		width = atoi(argv[3]);
		height = atoi(argv[4]);
        sprintf(path_name,"%s",argv[5]);
        mode = atoi(argv[6]);
	}
	else if( argc == 8 ) {
		sel = atoi(argv[1]);
		sprintf(dev_name,"/dev/video%d",sel);
		sel = atoi(argv[2]);
		width = atoi(argv[3]);
		height = atoi(argv[4]);
        sprintf(path_name,"%s",argv[5]);
        mode = atoi(argv[6]);
		test_cnt = atoi(argv[7]);
	}
	else{
		printf("please select the video device: 0-video0 1-video1 ......\n"); 	//select the video device
		scanf("%d", &sel);
		sprintf(dev_name,"/dev/video%d",sel);
			
		printf("please select the camera: 0-dev0 1-dev1 ......\n"); 	//select the camera
		scanf("%d", &sel);
		
		printf("please input the resolution: width height......\n");		//input the resolution
		scanf("%d %d", &width, &height);

        printf("please input the frame saving path......\n");		//input the frame saving path
		scanf("%15s", path_name);

        printf("please input the test mode: 1~4......\n");		//input the test mode
		scanf("%d", &mode);

		printf("please input the test_cnt: >=1......\n");		//input the test count
		scanf("%d", &test_cnt);
	}
	
	input_size.width = width;
	input_size.height = height;
	
	subch_size.width = input_size.width >> 1;
	subch_size.height = input_size.height >> 1;
	
	buf_size[0] = ALIGN_16B(input_size.width)*input_size.height*3/2;
	buf_size[1] = ALIGN_16B(subch_size.width)*subch_size.height*3/2;
	buf_size[2] = ALIGN_16B(subch_size.height)*subch_size.width*3/2;

	for(i = 0; i < test_cnt; i++)
	{
		if (0 == main_test(sel, mode))
			printf("*************************mode %d test done at the %d time!!\n", mode, i);
		else
			printf("*************************mode %d test failed at the %d time!!\n", mode, i);
	}
	return 0;
}
