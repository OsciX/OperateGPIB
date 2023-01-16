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
//       redo remapping using struct
//       finish PNG implementation


struct bmPx {
   unsigned int left : 4;
   unsigned int right : 4;
};




int get_device(int minor, int pad);

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

    
    
    long int px = buffer[0x0A];

    char pixeltable[480][640];
    for (int y = 0; y < 480; y++) // set row
    {
        for (int x = 0; x < 639 ; x += 2)
        {
            // first pixel represented in first four bytes, scaled to 0-255
            pixeltable[y][x] = (buffer[px] >> 4);
            // second pixel represented in next four bytes, scaled to 0-255
            pixeltable[y][x+1] = (buffer[px] & 0xF);

            //printf("(%d %d):\t%d\n(%d %d):\t%d\n", y, x, pixeltable[y][x], y, x+1, pixeltable[y][x+1]);

            px++;
        }
        //printf("\n");
    }

    // pixel table has been completed
    
    FILE* bmp = fopen(argv[2], "w");

    fwrite(buffer, 1, buffer[0x0A], bmp);
    printf("Header: %d bytes written", buffer[0x0A]);
    px = buffer[0x0A];
    for (int y = 0; y < 480; y++) // set row
        {
            for (int x = 0; x < 639 ; x += 2){
                char hold = (pixeltable[y][x]<<4 | pixeltable[y][x+1]);
                fwrite(&hold, 1, 1, bmp);
                px++;
            }
        }
            
    
    fclose(bmp);

    printf("%d KB received. (%d bytes total.) Written to %s\n", ibcnt/1024, ibcnt, argv[2]);
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