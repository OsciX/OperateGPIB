/*************************************************************************
 *
 * CDEMO32 - GPIB-488 Library Demo Program
 *
 * Copyright Quality Instrumentation Solutions, Inc. and its licensors, (c) 2009
 * All Rights Reserved
 *
 * File:   CDEMO32.C
 *
 * This is an example Windows program that uses the GPIB library to talk
 * to a Fluke 45 voltmeter.
 *
 * The WM_CREATE event handler initializes the meter and sets it for
 * Volts AC measurements.
 *
 * A Windows timer is set up that fires once per second.  The event handler
 * for the timer reads the current value from the meter and then
 * forces a screen update.  The WM_PAINT handler displays the current
 * voltage in a Window.
 *
 * Functions:  WinMain
 *             MainMessageHandler
 *             CleanUp
 *
 **********************************************************************/

/* Include files */
#include <windows.h>                    /* Compiler's include files's */
#include <string.h>
#include <stdio.h>
#include "gpib488.h"
#include <time.h>

/* GPIB device at pad = 1, sad = 0 on board gpib0*/
#define BOARD_NUM 0
#define PAD       9
#define SAD       0

/* Fluke 45 command strings */
#define RESET_CMD    "*RST"      /* Reset the meter */
#define ID_CMD       "*IDN?"     /* Return the meter's ID string */
#define RANGE_CMD    "VAC"       /* Set meter range to Volts AC */
#define MEASURE_CMD  "*TRG; VAL?"      /* Take a measurement and return it */

/* Size of ibrd buffer */
#define BUF_SIZE 256

/* Misc */
#define FALSE  0
#define TRUE   1
#define TIMER_NUM   1

int   InitMeter();
int   WriteCommand(int device, const char *command);
int   ReadValue(int device, char *buffer, size_t buflen);


/************************************************************************
 *
 * Name:      WinMain
 *
 * Arguments: hInstance - the instance handle for the program
 *            hPrevInstance - the class name of the application (not used)
 *            CmndLine - command line was called with (not used)
 *            CmndShow - indicates how to display window
 *
 * This is the entry point to the program. It gets called by Windows
 * to start the program up.  This routine creates the main window,
 * initializes a timer, and then falls into the main Windows Get
 * message/Dispatch message loop.
 *
 ************************************************************************/
int main() {
   char* buf[255];

   int hp3478a = InitMeter();
   float val[500];

   // DC voltage, 3V range, autozero off, 3 digits, Fast Trigger mode
   WriteCommand (hp3478a, "F1R0Z0N3D3T5");


   clock_t t;
   t = clock();


   for (int i = 0; i < 500; i++) {
      if(!(ReadValue(hp3478a, buf, 255))) {
         val[i] = atof(buf);
      }
   }

   double time_taken = ((double)clock() - t)/CLOCKS_PER_SEC; // in seconds


   printf("[");
   for (int i = 0; i < 500; i++) {
      printf("%1.3f%s", val[i], (i == 499) ? "]\n" : ", ");
   }

   fflush(stdout);

   printf("\n500 readings taken in %1.3f seconds.\nAverage reading frequency was %3.3f Hz.\n", time_taken, 500/time_taken);


}

/***************************************************************************
 *
 * Name:      InitMeter
 * Arguments: None
 * Returns:   handle to opened device
 *
 * Open the device.  If there is an error opening prints a message
 * and exit.  Otherwise set the timeout to 3 seconds. Send a reset
 * command to the meter and set the range to Volts AC.
 *
 ***************************************************************************/
int InitMeter() {
   int  device;
   char spr;
   /**/
   device = ibdev(BOARD_NUM, PAD, SAD, 12, 1, 0);
   if (device >= 0) {
      ibclr (device);
      ibrsp (device,&spr);
      ibtmo (device,T3s);                    /* Set timeout to 3 seconds */
      // WriteCommand(device, RESET_CMD);      /* Reset the meter */
      // WriteCommand(device, RANGE_CMD);      /* Set to Volts AC range */
   }
   return (device);
}




/***************************************************************************
 *
 * Name:      WriteCommand
 * Arguments: device - handle to the opened device
 *            cmd - command string to be written
 * Returns:   FALSE if succesful, else TRUE
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
