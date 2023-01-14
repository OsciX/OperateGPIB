/*

acquire.c
Written by Ryan Davis
1/13/2023

This program repeatedly queries the HP 3478A multimeter. It's intended to test basic capabilities of the linux-gpib libraries.
This is primarily based off of ibtest.c in the linux-gpib examples -- I figured I would test various mechanisms for my own benefit.

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include "gpib/ib.h"

#define MINOR 0

int get_device(int minor, int pad);

int main( int argc, char *argv[] ) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <PAD> <PNG path>\n", argv[0]);
        exit(1);
    }

    const int pad = atoi(argv[1]);
    int num = 1;

    if (argc == 3) {
        num = atoi(argv[2]);
    }

    if (pad < 0 || pad > 30) {
        fprintf(stderr, "Error: PAD must be between 0 and 30.\n");
        exit(1);
    }

    const int hp3478a = get_device(0,pad);
    char* buffer = malloc(16);
    
    
    printf("[ ");

    for (int i = 0; i < num; i++) {

        memset(buffer, 0, 15);
        ibrd(hp3478a, buffer, 15);

        if (i+1 != num) {
	        printf("%f, ", atof(buffer));
        } else {
            printf("%f ]\n", atof(buffer));
        }

        fflush(stdout);
    }

    free(buffer);

}

int get_device(int minor, int pad) {
	int ud;
	const int sad = 0;
	const int send_eoi = 1;
	const int eos_mode = 0;
	const int timeout = T1s;

	printf("Trying to open PAD %i on /dev/gpib%i ...\n", pad, minor);
	ud = ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);

	if(ud < 0) {
		fprintf(stderr, "ibdev error\n");
		abort();
    }

	return ud;
}