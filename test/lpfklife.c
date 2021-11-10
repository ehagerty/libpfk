// lpfklife: Conway's Game of Life for the LPFK

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "liblpfk.h"

#define LIFE_ALGORITHM_NAIVE

/************************
 * copied from http://linux-sxs.org/programming/kbhit.html
 */

#include <termios.h>
#include <unistd.h>   // for read()

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int kbhit()
{
unsigned char ch;
int nread;

    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int readch()
{
char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

/***********************/

int main(void)
{
	int i, nei, x, y;
	bool old_gamegrid[6][6];
	bool gamegrid[6][6];
	bool steadyState = false;
	unsigned long iteration = 0;
	LPFK_CTX ctx;

	// initialisation
	memset(&gamegrid, 0, sizeof(gamegrid));

	init_keyboard();
	atexit(close_keyboard);

	// open lpfk port
	if ((i = lpfk_open(&ctx, "/dev/ttyUSB0")) != LPFK_E_OK) {
		// error opening lpfk
		printf("Error opening LPFK: code %d\n", i);
		return -1;
	}

	lpfk_enable(&ctx, true);

	// allow user to set up their game grid
	printf("Press the keys on the LPFK to set up the game grid, then press Enter to start the simulation.\n");
	while (!kbhit()) {
		i = lpfk_read(&ctx);

		if (i >= 0) {
#ifdef DEBUG
			printf("key %d\n", i);
#endif
			// Key down, toggle the LED
			lpfk_set_led(&ctx, i, !lpfk_get_led(&ctx, i));

			// update game grid
			if ((i >= 0) && (i < 4)) {
				gamegrid[0][i+1] = !gamegrid[0][i+1];		// +1 because 1st row is 4-column
			} else if ((i >= 4) && (i < 10)) {
				gamegrid[1][i-4] = !gamegrid[1][i-4];
			} else if ((i >= 10) && (i < 16)) {
				gamegrid[2][i-10] = !gamegrid[2][i-10];
			} else if ((i >= 16) && (i < 22)) {
				gamegrid[3][i-16] = !gamegrid[3][i-16];
			} else if ((i >= 22) && (i < 28)) {
				gamegrid[4][i-22] = !gamegrid[4][i-22];
			} else {
				gamegrid[5][(i-28)+1] = !gamegrid[5][(i-28)+1];	// +1 because last row is 4-column
			}
		}
	}
	// flush keyboard buffer
	while (kbhit()) readch();

	// disable LPFK keys
	lpfk_enable(&ctx, false);

#ifdef DEBUG
	// print the game grid: debug only
	printf("GAME GRID: [iter %lu]\n", iteration);
	for (y=0; y<6; y++) {
		for (x=0; x<6; x++) {
			if (gamegrid[y][x]) printf("* "); else printf(". ");
		}
		printf("\n");
	}
	printf("\n");
#endif

	printf("Press ENTER to stop the simulation.\n");

	// run game
	while (!kbhit() && !steadyState) {
		// increase iteration counter
		iteration++;

		// save current game grid
		memcpy(&old_gamegrid, &gamegrid, sizeof(gamegrid));

		// loop over game grid
		for (y=0; y<6; y++) {
			for (x=0; x<6; x++) {
				nei = 0;

				// count neighbours
#ifdef LIFE_ALGORITHM_NAIVE
				/// NAIVE ALGORITHM
				// Assumes every cell outside the grid is dead.
				if (y > 0) {
					if (x > 0) { if (old_gamegrid[y-1][x-1]) nei++; }
					if (old_gamegrid[y-1][x]) nei++;
					if (x < 5) { if (old_gamegrid[y-1][x+1]) nei++; }
				}

				if (x > 0) { if (old_gamegrid[y][x-1]) nei++; }
				// old_gamegrid[x][y] is us!
				if (x < 5) { if (old_gamegrid[y][x+1]) nei++; }

				if (y < 5) {
					if (x > 0) { if (old_gamegrid[y+1][x-1]) nei++; }
					if (old_gamegrid[y+1][x]) nei++;
					if (x < 5) { if (old_gamegrid[y+1][x+1]) nei++; }
				}
#else
				/// WRAPPING ALGORITHM
				// Accesses off of one side of the grid are wrapped to the
				// opposite side.
				//
				// WARNING: this algorithm has NOT been extensively tested,
				// and completely screws up the glider test.
				//
				// TODO: test and debug:
				if (y > 0) {
					if (x > 0)	{ if (old_gamegrid[y-1][x-1]) nei++; }
						else	{ if (old_gamegrid[y-1][5]) nei++; }

					if (old_gamegrid[y-1][x]) nei++;

					if (x < 5)	{ if (old_gamegrid[y-1][x+1]) nei++; }
						else	{ if (old_gamegrid[y-1][0]) nei++; }
				} else {
					if (x > 0)	{ if (old_gamegrid[5][x-1]) nei++; }
						else	{ if (old_gamegrid[5][5]) nei++; }
						
					if (old_gamegrid[y-1][x]) nei++;

					if (x < 5)	{ if (old_gamegrid[5][x+1]) nei++; }
						else	{ if (old_gamegrid[5][0]) nei++; }
				}


				if (x > 0)	{ if (old_gamegrid[y][x-1]) nei++; }
					else	{ if (old_gamegrid[y][5]) nei++; }

				// old_gamegrid[x][y] is us!

				if (x < 5)	{ if (old_gamegrid[y][x+1]) nei++; }
					else	{ if (old_gamegrid[y][0]) nei++; }


				if (y < 5) {
					if (x > 0) { if (old_gamegrid[y+1][x-1]) nei++; }
						else	{ if (old_gamegrid[y+1][5]) nei++; }	///

					if (old_gamegrid[y+1][x]) nei++;

					if (x < 5) { if (old_gamegrid[y+1][x+1]) nei++; }
						else	{ if (old_gamegrid[y+1][0]) nei++; }	///
				} else {
					if (x > 0) { if (old_gamegrid[y+1][x-1]) nei++; }
						else	{ if (old_gamegrid[y+1][5]) nei++; }	///

					if (old_gamegrid[y+1][x]) nei++;

					if (x < 5) { if (old_gamegrid[y+1][x+1]) nei++; }
						else	{ if (old_gamegrid[y-1][0]) nei++; }	///
				}
#endif

				// so what happens to our cell?
				if (old_gamegrid[y][x]) {
					// --- rules for live cells ---
					if ((nei < 2) || (nei > 3)) {
						// <2 neighbours, death due to loneliness.
						// or >3 neighbours, death due to overcrowding.
						gamegrid[y][x] = false;
					}
				} else {
					// --- rules for dead cells ---
					if (nei == 3) {
						// any dead cell with three neighbours comes to life
						gamegrid[y][x] = true;
					}
				}
			}
		}

		if (memcmp(&gamegrid, &old_gamegrid, sizeof(gamegrid)) == 0) {
			steadyState = true;
		}

#ifdef DEBUG
		printf("GAME GRID: [iter %lu]\n", iteration);
		for (y=0; y<6; y++) {
			for (x=0; x<6; x++) {
				if (gamegrid[y][x]) printf("* "); else printf(". ");
			}
			printf("\n");
		}
		printf("\n");
#endif

		// now update the LPFK from the game grid
		for (i=0; i<32; i++) {
			if (i < 4)			lpfk_set_led_cached(&ctx, i, gamegrid[0][i+1]);
			else if (i < 10)	lpfk_set_led_cached(&ctx, i, gamegrid[1][i-4]);
			else if (i < 16)	lpfk_set_led_cached(&ctx, i, gamegrid[2][i-10]);
			else if (i < 22)	lpfk_set_led_cached(&ctx, i, gamegrid[3][i-16]);
			else if (i < 28)	lpfk_set_led_cached(&ctx, i, gamegrid[4][i-22]);
			else				lpfk_set_led_cached(&ctx, i, gamegrid[5][(i-28)+1]);
		}

		// flush updates to the LPFK
		lpfk_update_leds(&ctx);

		// make sure updates aren't too fast
		sleep(1);
	}

	if (steadyState) {
		printf("Steady state reached.\n");
	}
	printf("LPFK Life ran for %lu iterations.\n", iteration);

	// close the LPFK
	lpfk_close(&ctx);
}
