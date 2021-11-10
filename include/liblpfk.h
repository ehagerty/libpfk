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
 * @file	liblpfk.h
 * @brief	liblpfk library header
 */

#ifndef _liblpfk_h_included
#define _liblpfk_h_included

#include <termios.h>

/**
 * @brief	LPFK context
 *
 * Do not change any variables inside this struct, they are for liblpfk's
 * internal use only.
 */
typedef struct {
	int				fd;			///< serial port file descriptor
	struct termios	oldtio;		///< old termios setup
	int				enabled;	///< LPFK enabled
	unsigned long	led_mask;	///< lit LEDs mask
} LPFK_CTX;

/**
 * @brief	liblpfk error codes
 */
enum {
	LPFK_E_OK = 0,				///< No error, success.
	LPFK_E_NO_KEYS = -1,		///< No keys in input buffer
	LPFK_E_PORT_OPEN = -2,		///< Could not open comm port.
	LPFK_E_NOT_PRESENT = -3,	///< LPFK not present on specified port.
	LPFK_E_COMMS = -4,			///< Communication error.
	LPFK_E_PARAM = -5,			///< Invalid function parameter.
	LPFK_E_NOT_ENABLED = -6		///< Attempt to read key when LPFK disabled
};

/**
 * @brief	Open a serial port and attempt to connecct to an LPFK on that
 * 			port.
 * @param	port	Serial port path (e.g. /dev/ttyS0).
 * @param	ctx		Pointer to an LPFK_CTX struct where LPFK context will be
 * 					stored.
 * @return	LPFK_E_OK on success, LPFK_E_PORT_OPEN if port could not be
 * 			opened, LPFK_E_NOT_PRESENT if no LPFK present on specified port.
 */
int lpfk_open(LPFK_CTX *ctx, const char *port);

/**
 * @brief	Close the LPFK.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @return	LPFK_E_OK
 */
int lpfk_close(LPFK_CTX *ctx);

/**
 * @brief	Enable or disable LPFK input
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @param	val		true to enable the LPFK's keys, false to disable.
 * @return	LPFK_E_OK on success, LPFK_E_COMMS on comms error.
 */
int lpfk_enable(LPFK_CTX *ctx, const int val);

/**
 * @brief	Set or clear an LED in the cached LED mask buffer.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @param	num		LED/key number, from 0 to 31.
 * @param	state	State, true for on, false for off.
 * @return	LPFK_E_OK on success, LPFK_E_PARAM on bad parameter, LPFK_E_COMMS
 * 			on comms error.
 */
int lpfk_set_led_cached(LPFK_CTX *ctx, const int num, const int state);

/**
 * @brief	Set or clear all the LEDs in the shadow register.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @param	state	State, true for on, false for off.
 * @return	LPFK_E_OK on success.
 */
int lpfk_set_leds_cached(LPFK_CTX *ctx, const int state);

/**
 * @brief	Set the LPFK's LED state from the cached LED mask.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @return	LPFK_E_OK on success, LPFK_E_PARAM on bad parameter, LPFK_E_COMMS
 * 			on comms error.
 */
int lpfk_update_leds(LPFK_CTX *ctx);

/**
 * @brief	Set or clear an LED on the LPFK.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @param	num		LED/key number, from 0 to 31.
 * @param	state	State, true for on, false for off.
 * @return	LPFK_E_OK on success, LPFK_E_PARAM on bad parameter, LPFK_E_COMMS
 * 			on comms error.
 * @note	Equivalent to a call to lpfk_set_led_cached() followed by a call
 * 			to lpfk_update_leds().
 */
int lpfk_set_led(LPFK_CTX *ctx, const int num, const int state);

/**
 * @brief	Set or clear all the LEDs on the LPFK.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @param	state	State, true for on, false for off.
 * @return	LPFK_E_OK on success, LPFK_E_PARAM on bad parameter, LPFK_E_COMMS
 * 			on comms error.
 * @note	Equivalent to a call to lpfk_set_leds_cached() followed by a call
 * 			to lpfk_update_leds().
 */
int lpfk_set_leds(LPFK_CTX *ctx, const int state);

/**
 * @brief	Get the status of an LED on the LPFK.
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @param	num		LED/key number, from 0 to 31.
 * @return	true if LED is on, false otherwise.
 */
int lpfk_get_led(LPFK_CTX *ctx, const int num);

/**
 * @brief	Read a key from the LPFK
 * @param	ctx		Pointer to an LPFK_CTX struct initialised by lpfk_open().
 * @return	LPFK_E_NO_KEYS if no keys in buffer, 0-31 for key 1-32 down.
 */
int lpfk_read(LPFK_CTX *ctx);

#endif // _liblpfk_h_included
