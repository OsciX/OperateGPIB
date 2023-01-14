/*

screencapk.c
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

#include "gpib/ib.h"

#define MINOR 0

// TODO: implement png, bitmap has been achieved
//       make less dangerous


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

    char* ext = strrchr(argv[2], '.');
    if ( ext == NULL || ((strcmp(ext, ".bmp") != 0 && strcmp(ext, ".BMP") != 0)) ) {
        fprintf(stderr, "Error: file must have .bmp extension.\n");
        exit(1);
    }

    FILE * imgPtr = fopen(argv[2], "w");
    if (imgPtr == NULL) {
        fprintf(stderr, "Error: file cannot be opened.\n");
        exit(1);
    }


    const int awg2021 = get_device(0,pad);
    int* buffer = malloc(204800);
    wrtC(awg2021, "*CLS");

    printf("Connection succeeded. ");
    fflush(stdout);

    wrtC(awg2021, "HCOPY:FORM BMP;DATA?");
    ibrd(awg2021, buffer, 204800);

    printf("%d KB received. Written to %s\n", ibcnt/1024, argv[2]);

    
    fwrite(buffer, 1, ibcnt, imgPtr);
    fclose(imgPtr);

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