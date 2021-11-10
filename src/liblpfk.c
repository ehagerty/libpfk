/****************************************************************************
 * Project:		liblpfk
 * Purpose:		Driver library for the IBM 6094-020 Lighted Program Function
 * 				Keyboard.
 * Version:		1.0
 * Author:		Philip Pemberton <philpem@philpem.me.uk>
 *
 * The latest version of this library is available from
 * <http://www.philpem.me.uk/code/liblpfk/>.
 *
 * Copyright (c) 2008, Philip Pemberton
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/

/**
 * @file	liblpfk.c
 * @brief	liblpfk library, main source
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "liblpfk.h"

/* lpfk_open {{{ */
int lpfk_open(LPFK_CTX *ctx, const char *port)
{
	struct termios newtio;
	int status;
	int fd;
	int i;

	// open the serial port
	fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) return LPFK_E_PORT_OPEN;

	// save current port settings
	tcgetattr(fd, &ctx->oldtio);

	// set up new parameters
	memset(&newtio, 0, sizeof(newtio));
	// 9600 baud, 8 bits, parity enabled, odd parity
	newtio.c_cflag = B9600 | CS8 | PARENB | PARODD | CLOCAL | CREAD;
	newtio.c_iflag = 0;
	newtio.c_oflag = 0;

	// set input mode -- non canonical, no echo
	newtio.c_lflag = 0;

	// inter-character timer unused
	newtio.c_cc[VTIME] = 0;
	// read does not block waiting for characters if there are none in the buffer
	newtio.c_cc[VMIN]  = 0;

	// flush input buffer
	tcflush(fd, TCIFLUSH);

	// set new port config
	tcsetattr(fd, TCSANOW, &newtio);

	// set RTS true to pull the LPFK out of reset
	ioctl(fd, TIOCMGET, &status);
	status |= TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);

	// wait a few seconds for the LPFK to become ready
	sleep(2);

	// 0x06: READ CONFIGURATION. LPFK sends 0x03 in response.
	// Try five times to wake it up.
	status = false;
	for (i=0; i<5; i++) {
		unsigned char buf;
		time_t tm;

		// Send 0x06: READ CONFIGURATION, loop on failure
		if (write(fd, "\x06", 1) < 1) {
			continue;
		}

		// save current time (in seconds)
		tm = time(NULL);

		// loop until 2 seconds have passed, or LPFK responds
		status = false;
		do {
			// read data, loop if not successful
			if (read(fd, &buf, 1) < 1) {
				continue;
			}

			// we got some data, what is it?
			if (buf == 0x03) {
				// 0x03 -- correct response. we're done.
				status = true;
			}
		} while (((time(NULL) - tm) < 2) && (!status));

		// exit loop if we got the LPFK to talk
		if (status) {
			break;
		}
	}

	// Did the LPFK respond?
	if (!status) {
		// LPFK isn't talking. Restore serial port state and exit.
		tcsetattr(fd, TCSANOW, &ctx->oldtio);
		close(fd);
		
		return LPFK_E_NOT_PRESENT;
	} else {
		// Initialise LPFK context
		ctx->led_mask = 0;
		ctx->fd = fd;

		// Disable LPFK keyboard scanning
		lpfk_enable(ctx, false);

		// Return OK status
		return LPFK_E_OK;
	}
}

/* }}} */

/* lpfk_close {{{ */
int lpfk_close(LPFK_CTX *ctx)
{
	int status;

	// 0x09: DISABLE. Stop the LPFK responding to keystrokes.
	write(ctx->fd, "\x09", 1);

	// turn all the LEDs off
	lpfk_set_leds(ctx, false);

	// set RTS false to put the LPFK into reset
	ioctl(ctx->fd, TIOCMGET, &status);
	status &= ~TIOCM_RTS;
	ioctl(ctx->fd, TIOCMSET, &status);

	// Restore the port state and close the serial port.
	tcsetattr(ctx->fd, TCSANOW, &ctx->oldtio);
	close(ctx->fd);

	// Done!
	return LPFK_E_OK;
}
/* }}} */

/* lpfk_enable {{{ */
int lpfk_enable(LPFK_CTX *ctx, const int val)
{
	if (val) {
		// val == true, enable the LPFK
		if (write(ctx->fd, "\x08", 1) != 1) {
			ctx->enabled = true;
			return LPFK_E_COMMS;
		}
	} else {
		// val == false, disable the LPFK
		if (write(ctx->fd, "\x09", 1) != 1) {
			return LPFK_E_COMMS;
		}
	}

	// update the context, return success
	ctx->enabled = val;
	return LPFK_E_OK;
}

/* }}} */

/* lpfk_set_led_cached {{{ */
int lpfk_set_led_cached(LPFK_CTX *ctx, const int num, const int state)
{
	int i;
	time_t tm;
	unsigned long mask, leds;
	unsigned char buf[5];
	unsigned char status;

	// check parameters
	if ((num < 0) || (num > 31)) {
		return LPFK_E_PARAM;
	}

	// parameters OK, now build the LED mask
	mask = (0x80 >> (num % 8)) << ((3 - (num / 8)) * 8);

	// mask the specified bit
	if (state) {
		ctx->led_mask |= mask;
	} else {
		ctx->led_mask &= ~mask;
	}

	return LPFK_E_OK;
}
/* }}} */

/* lpfk_set_leds_cached {{{ */
int lpfk_set_leds_cached(LPFK_CTX *ctx, const int state)
{
	int i;
	time_t tm;
	unsigned long leds;
	unsigned char buf[5];
	unsigned char status;

	if (state) {
		// all LEDs on
		ctx->led_mask = 0xFFFFFFFF;
	} else {
		// all LEDs off
		ctx->led_mask = 0x00000000;
	}

	return LPFK_E_OK;
}
/* }}} */

/* lpfk_update_leds {{{ */
int lpfk_update_leds(LPFK_CTX *ctx)
{
	int i;
	time_t tm;
	unsigned char buf[5];
	unsigned char status;

	// send new LED mask to the LPFK
	buf[0] = 0x94;
	buf[1] = ctx->led_mask >> 24;
	buf[2] = ctx->led_mask >> 16;
	buf[3] = ctx->led_mask >> 8;
	buf[4] = ctx->led_mask & 0xff;

	// make 5 attempts to set the LEDs
	for (i=0; i<5; i++) {
		if (write(ctx->fd, &buf, 5) < 5) {
			continue;
		}

		// check for response -- 0x81 = OK, 0x80 = retransmit
		// save current time (in seconds)
		tm = time(NULL);

		// loop until 2 seconds have passed, or LPFK responds
		status = 0x00;
		do {
			// read data, loop if not successful
			if (read(ctx->fd, &status, 1) < 1) {
				continue;
			}

			// we got some data, what is it?
			if (status == 0x81) {
				// 0x81 -- received successfully
				break;
			}
		} while ((time(NULL) - tm) < 2);

		// status OK?
		if (status == 0x81) {
			// 0x81: OK
			break;
		} else if (status == 0x80) {
			// 0x80: Retransmit request
			continue;
		}
	}

	return LPFK_E_OK;
}
/* }}} */

/* lpfk_set_led {{{ */
int lpfk_set_led(LPFK_CTX *ctx, const int num, const int state)
{
	lpfk_set_led_cached(ctx, num, state);
	return lpfk_update_leds(ctx);
}
/* }}} */

/* lpfk_set_leds {{{ */
int lpfk_set_leds(LPFK_CTX *ctx, const int state)
{
	lpfk_set_leds_cached(ctx, state);
	return lpfk_update_leds(ctx);
}
/* }}} */

/* lpfk_get_led {{{ */
int lpfk_get_led(LPFK_CTX *ctx, const int num)
{
	unsigned long mask;

	// check parameters
	if ((num < 0) || (num > 31)) {
		return false;
	}

	// parameters OK, now build the LED mask
	mask = (0x80 >> (num % 8)) << ((3 - (num / 8)) * 8);
	if (ctx->led_mask & mask) {
		return true;
	} else {
		return false;
	}
}
/* }}} */

/* lpfk_read {{{ */
int lpfk_read(LPFK_CTX *ctx)
{
	int nbytes;
	unsigned char key;

	// make sure the LPFK is enabled before trying to read a scancode
	if (!ctx->enabled) {
		return LPFK_E_NOT_ENABLED;
	}

	// try and read a byte (keycode) from the LPFK
	nbytes = read(ctx->fd, &key, 1);

	if ((nbytes < 1) || (key > 31)) {
		// no keys buffered, or keycode invalid.
		return LPFK_E_NO_KEYS;
	} else {
		// key buffered, pass it along.
		return key;
	}
}
/* }}} */

