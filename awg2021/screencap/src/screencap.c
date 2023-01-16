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
#include <png.h>
#include "gpib/ib.h"

#define MINOR 0

// TODO: add QoL features

int get_device(int minor, int pad);
int savePng(uint8_t* bmpBuffer, char* path);

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

     if (strcmp(".png", strrchr(argv[2], '.')) != 0) {
        fprintf(stderr, "Error: format must be '.png'.\n");
        exit(1);
    }

    // Get device, and allocate buffer to hold the screencapture.
    const int awg2021 = get_device(0,pad);
    uint8_t* buffer = malloc(153719);
    wrtC(awg2021, "*CLS");

    printf("Connection succeeded. ");
    fflush(stdout);

    // Ask for a bitmap-format screencapture.
    // This format will always be an uncompressed 4 bit grayscale 640x480 BMP image, with 2px/byte.
    wrtC(awg2021, "HCOPY:FORM BMP;DATA?");
    ibrd(awg2021, buffer, 153719);

    if (ibcnt != 153719) {
        fprintf(stderr, "Error: missing bytes!\n");
        exit(1);
    }

    FILE* bmp = fopen(argv[2], "w");
    fwrite(buffer, 1, 153719, bmp);
    fclose(bmp);

    if (savePng(buffer, argv[2]) == 0) {
        printf("Image saved at %s.\n", argv[2]);
    } else {
        printf("An error occurred in PNG conversion.\n");
    }

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


// Based on the excellent example code by Ben Bullock
// https://www.lemoda.net/c/write-png/

int savePng(uint8_t* bmpBuffer, char* path) {
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte **row_pointers = NULL;

    size_t width = 640;
    size_t height = 480;

    // fail at first, succeed when done
    int status = -1;

    // set bit depth
    int depth = 4;
    
    fp = fopen (path, "wb");
    if (! fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    
    // Error handling
    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }
    
    // Image attributes
    png_set_IHDR (png_ptr,
                  info_ptr,
                  width,
                  height,
                  depth,
                  PNG_COLOR_TYPE_GRAY,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    // Row pointers
    row_pointers = png_malloc (png_ptr, height * sizeof (png_byte *));

    for (y = 0; y < height; y++) {
        png_byte *row = png_malloc (png_ptr, sizeof (uint8_t) * (width / 2));

        // fill row pointers backwards in order to flip image vertically
        row_pointers[height-1-y] = row;

        // Pick up 4 bit-packed grayscale colors directly from bitmap buffer
        // PNG packs in the exact same way
        for (x = 0; x < width/2; x++) {
            *row++ = bmpBuffer[118 + y*(width/2) + x];
        }
    }
    
    // Write image data and indicate our success
    png_init_io (png_ptr, fp);
    png_set_compression_level(png_ptr, 9);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    status = 0;
    
    // clean up
    for (y = 0; y < height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
    // error cases
    png_failure:
    png_create_info_struct_failed:
        png_destroy_write_struct (&png_ptr, &info_ptr);
    png_create_write_struct_failed:
        fclose (fp);
    fopen_failed:
        return status;
}