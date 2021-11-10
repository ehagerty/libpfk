#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "liblpfk.h"

int main(int argc, char **argv) 
{
	LPFK_CTX	ctx;
	int			i;
	time_t		tm;

	if (argc < 2) {
		printf("Syntax: %s commport\n", argv[0]);
		return -1;
	}

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

//	printf("disable: %d\n", lpfk_enable(&ctx, false));
//	printf("enable : %d\n", lpfk_enable(&ctx, true));

	printf("Scanning LEDs, 1-32...\n");

	for (i=0; i<32; i++) {
		if (i>0) {
			lpfk_set_led(&ctx, i-1, false);
		}
		lpfk_set_led(&ctx, i, true);
		usleep(100000);
	}

	// Turn LEDs off and enable LPFK keystroke input
	lpfk_set_leds(&ctx, false);
	lpfk_enable(&ctx, true);

	printf("Now press the keys on the LPFK...\n");

	// scan keys for 5 seconds
	tm = time(NULL);
	do {
		// read keys
		if ((i = lpfk_read(&ctx)) >= 0) {
			// key buffered, toggle the LED
			printf("Key down: #%d\n", i);
			lpfk_set_led(&ctx, i, !lpfk_get_led(&ctx, i));
		}
	} while ((time(NULL) - tm) < 30);

	lpfk_close(&ctx);
}

