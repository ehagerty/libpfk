# libpfk
All credit for initial commit to https://www.philpem.me.uk/code/liblpfk.  I just wanted to host it here on github in addition to his mercurial repo

*** original page from philpem.me.uk ***

liblpfk -- IBM LPFK driver library
Back in 2008, I picked up a few IBM 6094-020 Lighted Program Function Keyboards from someone on the classiccmp mailing list. These are an array of keys and lights which were used as a user interface for some kind of CAD product on the RS/6000 AIX platform.

Codeninja had already reverse-engineered the protocol – but it seems their LPFK used a non-standard protocol.

Michael Brutman later obtained a copy of the LPFK Model 6094-020 commands (used on the standard LPFK) and published these online. liblpfk implements this version of the LPFK protocol.

The API is pretty simple:

connect
disconnect
enable/disable keypress notification
set/clear LEDs
get keypress
Several demos are included in the liblpfk source code:

the “LPFK Binary Clock” by Ethan Dicks
“LPFK Life” (an implementation of Conway's Game of Life using the LPFK lights and keys)
Doxygen comments are present in the code, which means documentation may be generated simply by running doxygen over it.

Mercurial repository: http://hg.philpem.me.uk/liblpfk/

LPFK connections
The LPFK uses RS232 communications, and requires around 500mA of current with all the LEDs on.

I used an FTDI TTL232 cable and a MAX232 to drive the LPFK, with power taken directly from the PC's USB port.

I also reprogrammed the FT232's EEPROM so that it declared its need for 500mA of power to the host PC (this is done with FTDI's FT-PROG tool).

The mini-DIN pinout is as follows:

Pin	Function
1	GND (signal return)
2	GND (DC return)
3	+5V
4	Reset (leave floating, +5V/GND levels)
5	TxD (connect to PC RxD)
6	RxD (connect to PC TxD)
7	No connect
8	No connect
This is the same as the pinout for the IBM Graphics Input Adapter MCA card, FRU 22F9758.

Pinouts and diagrams are in IBM document SA38-0533-04, "Adapters, Devices, and Cable Information for Micro Channel Bus Systems" on page 1-134 (PDF page 154).

See also
Daniel Drucker has written a Python library to control the LPFK.
