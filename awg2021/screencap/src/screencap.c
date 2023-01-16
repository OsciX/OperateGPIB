/*

screencap.c
Written by Ryan Davis
1/13/2023

This program retrieves a screencapture from the AWG2021 waveform generator. It uses the Tektronix-exclusive HCOPY? query to ask for a 16-color bitmap over GPIB,
reading the raw data bytes into memory. As data compression has improved greatly since 1995, the constructed bitmap is converted to PNG format to save disk space.
The resulting PNG is then saved to a path provided by the user.

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include <math.h>
#include <malloc.h>
#include <png.h>
#include "gpib/ib.h"

#define MINOR 0

// TODO: make less dangerous
//       add QoL features
//       finish PNG implementation


typedef struct pixel_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;

int get_device(int minor, int pad);
int savePng(char* bmpBuffer, char* path);

int wrtC(int ud, char* cmd) {
    return ibwrt(ud, cmd, strlen(cmd));
}

int main( int argc, char *argv[] ) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <PAD> <image path>\n", argv[0]);
        exit(1);
    }

    const int pad = atoi(argv[1]);

    if (pad < 0 || pad > 30) {
        fprintf(stderr, "Error: PAD must be between 0 and 30.\n");
        exit(1);
    }

    if ( strlen(argv[2]) <= 4 ) {
        fprintf(stderr, "Error: file name invalid.\n");
        exit(1);
    }

    const int awg2021 = get_device(0,pad);
    char* buffer = malloc(204800);
    wrtC(awg2021, "*CLS");

    printf("Connection succeeded. ");
    fflush(stdout);

    wrtC(awg2021, "HCOPY:FORM BMP;DATA?");
    ibrd(awg2021, buffer, 204800);
    savePng(buffer, argv[2]);

    free(buffer);


}

int get_device(int minor, int pad) {
	int ud;
	const int sad = 0;
	const int send_eoi = 1;
	const int eos_mode = 0;
	const int timeout = T10s;

	printf("Trying to open PAD %i on /dev/gpib%i ...\n", pad, minor);
	ud = ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);

	if(ud < 0) {
		fprintf(stderr, "ibdev error\n");
		abort();
    }

	return ud;
}

int savePng(char* bmpBuffer, char* path) {
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;

    size_t width = 640;
    size_t height = 480;

     /* "status" contains the return value of this function. At first
       it is set to a value which means 'failure'. When the routine
       has finished its work, it is set to a value which means
       'success'. */
    int status = -1;

    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = 3;
    int depth = 8;
    
    fp = fopen (path, "wb");
    if (! fp) {
        goto fopen_failed;
    }


    long int px = bmpBuffer[0x0A];
    struct pixel_t pixeltable[480][640];
    for (int y = 479; y >= 0; y--) // set row
    {
        for (int x = 0; x < 639 ; x += 2)
        {
            // first pixel represented in first four bytes, scaled to 0-255
            pixeltable[y][x].red = 255 - (((bmpBuffer[px] >> 4) + 1)*16);
            pixeltable[y][x].green = 255 - (((bmpBuffer[px] >> 4) + 1)*16);
            pixeltable[y][x].blue = 255 - (((bmpBuffer[px] >> 4) + 1)*16);

            // second pixel represented in next four bytes, scaled to 0-255
           
            pixeltable[y][x+1].red =  255 - (((bmpBuffer[px] & 0xF) + 1)*16);
            pixeltable[y][x+1].green =  255 - (((bmpBuffer[px] & 0xF) + 1)*16);
            pixeltable[y][x+1].blue =  255 - (((bmpBuffer[px] & 0xF) + 1)*16);

            px++;
        }
    }


    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    
    /* Set up error handling. */

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }
    
    /* Set image attributes. */

    png_set_IHDR (png_ptr,
                  info_ptr,
                  width,
                  height,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */

    row_pointers = png_malloc (png_ptr, height * sizeof (png_byte *));
    for (y = 0; y < height; y++) {
        png_byte *row = png_malloc (png_ptr, sizeof (uint8_t) * width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < width; x++) {
            *row++ = pixeltable[y][x].red;
            *row++ = pixeltable[y][x].green;
            *row++ = pixeltable[y][x].blue;
        }
    }
    
    /* Write the image data to "fp". */

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
       "status" to a value which indicates success. */

    status = 0;
    
    for (y = 0; y < height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return status;
}