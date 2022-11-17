/*************************************************************************
 *
 * HP 3478A Improved Demo
 * 
 * Written by Ryan Davis in 2022
 * (Utilizing library from Quality Instrumentation Solutions)
 * 
 * File:   3478a_improved.c
 *
 * This is a Windows program that uses a GPIB-488 PCI card to talk to an
 * HP 3478A benchtop multimeter. As the command syntax of the meter is 
 * quite terse, a helper function assists in setting parameters.
 *  
 *
 **********************************************************************/

/* Include files */
#include <windows.h>                    /* Compiler's include files's */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "gpib488.h"


/* Size of ibrd buffer */
#define BUF_SIZE 256

/* Misc */
#define FALSE  0
#define TRUE   1
#define TIMER_NUM   1

char* ArbPlotter(char* );
int   InitDevice(uint8_t boardNum, uint8_t address);
int   WriteCommand(int device, const char *command);
int   ReadValue(int device, char *buffer, size_t buflen);

/************************************************************************
 *
 * Name:      main
 *
 * Arguments: None
 *
 * This is the entry point to the program. Since this is a console program,
 * all processing is performed here, in the console.
 *
 ************************************************************************/
int main(int argc, char *argv[]) {
    char buf[16];
    int wav275 = InitDevice(0, 2);


    // 1khz sawtooth
    WriteCommand(wav275, "S9.9E-6A5D0B0C6XB20XH120P1Y4000K20I'LOADING WAVEFORM'");
    WriteCommand(wav275, "K20L0");
    WriteCommand(wav275, "K120L2000XK");
}

/***************************************************************************
 *
 * Name:      InitDevice
 * Arguments: boardNum - GPIB board number
 *            PAD - main GPIB address of the device
 * Returns:   handle to opened device
 *
 * Open the device. If there is an error opening, print a message and exit.
 * Otherwise set the timeout to 3 seconds.
 *
 ***************************************************************************/
int InitDevice(uint8_t boardNum, uint8_t PAD) {
   int  device;
   char spr;

   // 3 second timeout, assert EOI, 0 as string terminator (see reference)
   // notably this function also sets SAD to 0
   device = ibdev(boardNum, PAD, 0, 12, 1, 10);

   if (device >= 0) {
      ibclr (device);
      ibrsp (device,&spr);
      ibtmo (device,T3s);                    /* Set timeout to 3 seconds */
      WriteCommand(device, "*RST");          /* Reset the device */
   }
   return (device);
}

/***************************************************************************
 *
 * Name:      WriteCommand
 * Arguments: device - handle to the opened device
 *            cmd - command string to be written
 * Returns:   FALSE if successful, else TRUE
 *
 * Send a command to a GPIB device and checks for error.
 *
 ***************************************************************************/
int WriteCommand (int device, const char *cmd) {
   size_t cmdlength;
   int    ReturnVal;
   /**/
   cmdlength = strlen (cmd);
   ibwrt  (device, cmd, cmdlength);
   if (Ibcnt()!=cmdlength || Ibsta()&ERR)
      ReturnVal = TRUE;
   else
      ReturnVal = FALSE;
   return (ReturnVal);
}

/***************************************************************************
 *
 * Name:      ReadValue
 * Arguments: device - handle to the opened device
 *            buffer - buffer to read into
 *            bufsize - size of buffer (in bytes)
 * Returns:   FALSE if succesful, TRUE if error
 *
 * Reads a string from a GPIB device and checks for errors.  If no
 * errors it terminates the string.
 *
 ***************************************************************************/
int ReadValue(int device, char *buffer, size_t bufsize) {
   int rtrnval;
   /**/
   ibrd (device,buffer,bufsize);
   if (Ibcnt()==bufsize || Ibcnt()==0 || Ibsta()&ERR)
      rtrnval = TRUE;
   else
   {
      buffer[Ibcnt()-1] = '\0';    /* Terminate string */
      rtrnval = FALSE;
   }
   return (rtrnval);
}
