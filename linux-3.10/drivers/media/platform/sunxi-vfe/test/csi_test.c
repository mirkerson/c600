
//zw 
//for csi & isp test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

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

#include "libjpeg/include/jpeglib.h"

// global settings
unsigned int width = 640;
unsigned int height = 480;
int continuous = 1;
unsigned char jpegQuality = 70;
char* jpegFilename = NULL;
char* jpegFilenamePart = NULL;
int img_ind=0;

//#define	__DISPLAY2
#include "sunxi_camera.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct size{
	int width;
	int height;
};
struct buffer {
	void * start;
	size_t length;
};

static char path_name[20] = {'\0'};
static char dev_name[20] = {'\0'};
static int      fd              = -1;
struct buffer *   buffers       = NULL;
static unsigned int   n_buffers	= 0;

struct size input_size;
struct size subch_size;

unsigned int  req_frame_num = 4;//8;

static void yuv420_to_yuv444(unsigned char *src, unsigned char *dst)
{
	unsigned char *base_py = src;
	unsigned char *base_pu = src+(height*width);
	unsigned char *base_pv = src+(height*width)+(height*width)/4;
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = dst;
	int line, column;
	for (line = 0; line < height; ++line) {
		for (column = 0; column < width; ++column) {	
			py = base_py+(line*width)+column;
			pu = base_pu+(line/2*width/2)+column/2;
			pv = base_pv+(line/2*width/2)+column/2;
			*tmp++ = *py;	
			*tmp++ = *pu;
			*tmp++ = *pv;
		}
	}
}


/**
  Write image to jpeg file.

  \param img image to write
 */
static void jpegWrite(unsigned char* img, char* jpegFilename)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	FILE *outfile = fopen( jpegFilename, "wb" );

	// try to open file for saving
	if (!outfile) {
		printf("jpeg\n");
	}

	// create jpeg data
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	// set image parameters
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;

	// set jpeg compression parameters to default
	jpeg_set_defaults(&cinfo);
	// and then adjust quality setting
	jpeg_set_quality(&cinfo, jpegQuality, TRUE);

	// start compress
	jpeg_start_compress(&cinfo, TRUE);

	// feed data
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &img[cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// finish compression
	jpeg_finish_compress(&cinfo);

	// destroy jpeg data
	jpeg_destroy_compress(&cinfo);

	// close output file
	fclose(outfile);
}


/**
  process image read
 */
static void imageProcess(const void* p, struct timeval timestamp)
{
	//timestamp.tv_sec
	//timestamp.tv_usec
	unsigned char* src = (unsigned char*)p;
	unsigned char* dst  = malloc(width*height*3*sizeof(char));

	//static int img_ind = 0;		
	long timestamp_long;

	//YUV420toYUV444(width, height, src, dst);
	yuv420_to_yuv444(src, dst);

	if(continuous==1) {
		timestamp_long = timestamp.tv_sec*1e6 + timestamp.tv_usec;
		img_ind++;
		sprintf(jpegFilename,"%s_%010d_%010ld.jpg",jpegFilenamePart,img_ind,timestamp_long);
	}

	// write jpeg
	jpegWrite(dst,jpegFilename);
	// free temporary image
	free(dst);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
static int read_frame (int mode)
{
	struct v4l2_buffer buf;
	void * bfstart = NULL;
	int i=mode;
	struct timespec ts;
	struct timeval timestamp;


	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 ==ioctl (fd, VIDIOC_DQBUF, &buf) )      //取buffer中的数据
	{
		printf("dqbuf error\n");
		return -1;
	}

	assert (buf.index < n_buffers);

	clock_gettime(CLOCK_MONOTONIC,&ts);
	timestamp.tv_sec = ts.tv_sec;
	timestamp.tv_usec = ts.tv_nsec/1000;
	imageProcess(buffers[buf.index].start,timestamp);
	
	if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))		//移出buffer中的数据
	{
		printf("qbuf error\n");
		return -1;
	}
	return 0;
}

static int req_frame_buffers(void)
{
	unsigned int i;
	struct v4l2_requestbuffers req;

	CLEAR (req);
	req.count	= req_frame_num;
	req.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	= V4L2_MEMORY_MMAP;	

	ioctl (fd, VIDIOC_REQBUFS, &req); 

	buffers = calloc (req.count, sizeof (*buffers));

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
		struct v4l2_buffer buf;  
		CLEAR (buf);
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;

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
	struct rot_channel_cfg rot;

	//VIDIOC_S_FMT
	CLEAR (fmt);
	fmt.type                	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       	= input_size.width; 	//640; 
	fmt.fmt.pix.height      	= input_size.height; 	//480;
	fmt.fmt.pix.pixelformat 	= V4L2_PIX_FMT_YUV420;	//V4L2_PIX_FMT_YUV422P;//V4L2_PIX_FMT_NV12;//V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       	= V4L2_FIELD_NONE;			//V4L2_FIELD_INTERLACED;//V4L2_FIELD_NONE;
	rot.rotation	    		= 0;

	if (0 == subch)
	{
		//	fmt.fmt.pix.subchannel	= NULL;	
		rot.sel_ch               = 0;
	}
	else
	{
		//fmt.fmt.pix.subchannel	= &subch_fmt;
		rot.pix			= subch_fmt;
		subch_fmt.width 	= subch_size.width;
		subch_fmt.height	= subch_size.height;
		subch_fmt.pixelformat	= V4L2_PIX_FMT_YUV420; 		//V4L2_PIX_FMT_YUV422P;//V4L2_PIX_FMT_NV12;//V4L2_PIX_FMT_YUYV;
		subch_fmt.field 	= V4L2_FIELD_NONE;			//V4L2_FIELD_INTERLACED;//V4L2_FIELD_NONE;
		rot.rotation            = angle;
		//	subch_fmt.rot_angle 	= angle;
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

	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) 
	{		
		printf ("VIDIOC_STREAMON failed\n");
		return -1;
	}
	else 
		printf ("VIDIOC_STREAMON ok\n");

	while(img_ind < 10)
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

		if(read_frame(mode)){
			printf("%s, %d\n", __func__, __LINE__);
			printf("[app_basic] read frame ====error====!\n");
			return -1;			
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

	close (fd);	

	return 0;
}

int main(int argc,char *argv[])
{
	int i,test_cnt = 1;
	int sel = 0;
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

	printf("##########width=%d, height=%d\n", width, height);

	jpegFilename ="AW1663_CSI_TEST###";
	if(continuous == 1) {
		int max_name_len = strlen(jpegFilename)+10+10+3;
		jpegFilenamePart = jpegFilename;
		jpegFilename = calloc(max_name_len,sizeof(char));
		strcpy(jpegFilename,jpegFilenamePart);
	}

	for(i = 0; i < test_cnt; i++)
	{
		if (0 == main_test(sel, mode))
			printf("*************************mode %d test done at the %d time!!\n", mode, i);
		else
			printf("*************************mode %d test failed at the %d time!!\n", mode, i);
	}
	return 0;
}
