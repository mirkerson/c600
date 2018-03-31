/*Copyright George Peter Staplin 2003*/

/*You may use/copy/modify/distribute this software for any purpose
 *provided that I am given credit and you don't sue me for any bugs.
 */

/*Please contact me using GeorgePS@XMission.com if you like this, or
 *have questions.
 */
		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jpeglib.h>
#include <jerror.h>

#include <time.h>

#ifndef u_char
#define u_char unsigned char
#endif		


void jpeg_error_exit (j_common_ptr cinfo) {
  cinfo->err->output_message (cinfo);
  exit (EXIT_FAILURE);
}

/*This returns an array for a 24 bit image.*/
u_char *decode_jpeg (char *filename, short *widthPtr, short *heightPtr)
{
  register JSAMPARRAY lineBuf;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr err_mgr;
  int bytesPerPix;
  FILE *inFile;
  u_char *retBuf;
	
  inFile = fopen (filename, "rb");
  if (NULL == inFile) 
    { 
      printf ("Open file error %s\n",filename);
      return NULL;
    }

  cinfo.err = jpeg_std_error (&err_mgr);
  err_mgr.error_exit = jpeg_error_exit;	

  jpeg_create_decompress (&cinfo);
  jpeg_stdio_src (&cinfo, inFile);
  jpeg_read_header (&cinfo, 1);
  cinfo.do_fancy_upsampling = 0;
  cinfo.do_block_smoothing = 0;
  jpeg_start_decompress (&cinfo);

  *widthPtr = cinfo.output_width;
  *heightPtr = cinfo.output_height;
  bytesPerPix = cinfo.output_components;

  lineBuf = cinfo.mem->alloc_sarray ((j_common_ptr) &cinfo, JPOOL_IMAGE, (*widthPtr * bytesPerPix), 1);
  retBuf = (u_char *) malloc (3 * (*widthPtr * *heightPtr));
		
  if (NULL == retBuf) 
    {
      perror (NULL);
      return NULL;
    }
		

  if (3 == bytesPerPix) 
    {
      int x;
      int y;
      int i;
			
      for (y = 0; y < cinfo.output_height; ++y) 
	{	
	  jpeg_read_scanlines (&cinfo, lineBuf, 1);
	  memcpy ((retBuf + y * *widthPtr * 3),lineBuf[0],3 * *widthPtr);
	}
    } else if (1 == bytesPerPix) 
      { 
	unsigned int col;
	int lineOffset = (*widthPtr * 3);
	int lineBufIndex;
	int x ;
	int y;
						
	for (y = 0; y < cinfo.output_height; ++y) 
	  {
	    jpeg_read_scanlines (&cinfo, lineBuf, 1);
				
	    lineBufIndex = 0;
	    for (x = 0; x < lineOffset; ++x) {
	      col = lineBuf[0][lineBufIndex];
			
	      retBuf[(lineOffset * y) + x] = col;
	      ++x;
	      retBuf[(lineOffset * y) + x] = col;
	      ++x;
	      retBuf[(lineOffset * y) + x] = col;
				
	      ++lineBufIndex;
	    }			
	  }
      } else {
	fprintf (stderr, "Error: the number of color channels is %d.  This program only handles 1 or 3\n", bytesPerPix);
	return NULL;
      }
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  fclose (inFile);
			
  return retBuf;
}
	
int encode_jpeg(const char * filename, unsigned char * rgbbuf,short widthPtr, short heightPtr)

{
	struct jpeg_compress_struct cinfo;	// JPEG image parameters
	struct jpeg_error_mgr jerr;		// default JPEG error handler
	FILE * outfile;				// target file handler
	unsigned char ** buffer;		// points to large array of RGB data
	int row_stride,k;			// physical row width in image buffer
	
	// create/open output file
	outfile = fopen(filename, "w+b");
	if (outfile == NULL)
	{ 
		printf ("open %s error\n",filename);
		return -1;
	}
	// Set up the normal JPEG error routines
	cinfo.err = jpeg_std_error(&jerr);
	
	// Initialize the JPEG compression object
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	// Set parameters for compression
	cinfo.image_width = widthPtr; 		// set image width
	cinfo.image_height = heightPtr;		// set image height
	cinfo.input_components = 3;	// number of compents per pixel
	
	cinfo.in_color_space = JCS_RGB;
			
	jpeg_set_defaults(&cinfo);					// set rest of values default
	jpeg_set_quality(&cinfo, 75, TRUE);	// set image quality

	// Start compressor
	jpeg_start_compress(&cinfo, TRUE);

	// Now, write to scan lines as the JPEG is being created
	// JSAMPLEs per row in image_buffer
	row_stride = widthPtr * cinfo.input_components;
	
	// Create data buffer
	buffer = malloc (cinfo.image_height*4);
	buffer[0] = malloc(cinfo.image_height * row_stride);
	for (k = 0; k < (int)(cinfo.image_height); k++) {
		buffer[k] = buffer[0] + row_stride*k;
	}

	// Load input buffer with data
	int i, j;
	
	for (j = 0; j < heightPtr; j++)
		{
			for (i = 0; i < widthPtr*3; i += 3)
			{
				buffer[j][i] =   rgbbuf[j*row_stride+i];
				buffer[j][i+1] = rgbbuf[j*row_stride+i+1];
				buffer[j][i+2] = rgbbuf[j*row_stride+i+2];
			}
		}
			

	while (cinfo.next_scanline < cinfo.image_height)
    // jpeg_write_scanlines expects an array of pointers to scanlines.
    // Here the array is only one element long, but you could pass
    // more than one scanline at a time if that's more convenient.
	{
	    (void) jpeg_write_scanlines(&cinfo, &buffer[cinfo.next_scanline], 1);
	}

	// Complete compression and close output file
	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	// Delete image buffer
	free(buffer[0]);
	free(buffer);

	// Destroy compression object
	jpeg_destroy_compress(&cinfo);
	return 0;
}

	
