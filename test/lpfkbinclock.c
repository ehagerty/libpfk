/*
 * binclock.c - simple binary clock using an IBM LPFK as a display device
 *
 * Written by Ethan Dicks <ethan.dicks@gmail.com>
 *
 * Revision History
 *
 * 27-Aug-2008	0.1	Ethan Dicks	Initial version
 * 27 Aug-2008	0.2	Ethan Dicks	Corrected serial init bits (thanks to
 *					Phil Pemberton!) and proper logic tests
 * 08-Sep-2008	0.3	Ethan Dicks	Converted to work with Phil Pemberton's
 *					lpfklib.
 * 
 *
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "liblpfk.h"

//#define TEST

int main(int argc, char **argv) 
{
	LPFK_CTX	ctx;
	int		i;

	/* Bits for masking off digits of time */
	int bcdmask[] = { 0x01, 0x02, 0x04, 0x08 };

	// strftime stuff
        static const char *timeFormat = "%H%M%S";
        time_t thetime;
        struct tm *rtime;
        char timestr[40];

	/* vars for converting ASCII time to individual bits */
	int timedigit, timebit;
	int maploc;

	/* Import our serial port name */
	if (argc < 2) {
		printf("Syntax: %s commport\n", argv[0]);
		return -1;
	}

	/* Open the LPFK library and retain the handle in ctx */
#ifndef TEST
	if ((i = lpfk_open(&ctx, argv[1])) != LPFK_E_OK) {
		switch(i) {
			case LPFK_E_PORT_OPEN:
				printf("Error opening comm port.\n");
				break;
			case LPFK_E_NOT_PRESENT:
				printf("LPFK not connected to specified port.\n");
				break;
			case LPFK_E_COMMS:
				printf("LPFK communications error.\n");
				break;
		}
		return -2;
	}
#endif /* TEST */

	/* Turn LEDs off, just in case they were left on by the last application */
#ifndef TEST
	lpfk_set_leds(&ctx, false);
#endif /* TEST */

	//
	// loop forever while collecting, reformatting and sending the time
	//
	while(1) {

		/* get time */
		time(&thetime);
		rtime = localtime(&thetime);

		/* reformat time from raw time to BCD HHMMSS to make it easy to parse next */
	        if (strftime(timestr, sizeof(timestr), timeFormat, rtime) == 0)
        		*timestr = '\0';

#ifdef TEST
		printf("The time is '%s'\n", timestr);
#endif /* TEST */

		/* start with seconds and work backwards */
		for (timedigit = 5; timedigit > -1; timedigit--) {
			/* count bits from 0001 to 1000 forwards */
			for (timebit = 0; timebit < 4; timebit++) {
				// ugly!
				maploc = 5 + 6 * (3 - timebit) + timedigit - 1;
#ifndef TEST
				lpfk_set_led_cached(&ctx, maploc, timestr[timedigit] & bcdmask[timebit]);
#endif /* TEST */

#ifdef TEST
				printf("Turning LED %d to %d\n", maploc, timestr[timedigit] & bcdmask[timebit]);
#endif /* TEST */
			}
		}

		/* Update all the LEDs at once */
#ifndef TEST
		lpfk_update_leds(&ctx);
#endif /* TEST */

		/*  no need to do this hundreds of times per second */
		sleep(1);  // make it 1 sec for now; screw with nanosleep() later
	}

}


#if 0
	/* Future cleanup code to be called by a SIGHUP trap */

	/* Turn LEDs off and close the port */
	lpfk_set_leds(&ctx, false);
	lpfk_close(&ctx);
#endif

