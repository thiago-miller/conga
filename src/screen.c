#include "screen.h"

#include <ncurses.h>
#include "error.h"

static int ncurses_started = 0;

void
screen_init (void)
{
	initscr  ();
	cbreak   ();
	noecho   ();
	curs_set (0);
	keypad   (stdscr, TRUE);
	nodelay  (stdscr, TRUE);

	if (!has_colors ())
		{
			screen_finish ();
			error (1, 0, "Your terminal emulator has no color support");
		}

	ncurses_started = 1;
}

void
screen_finish (void)
{
	if (ncurses_started)
		endwin ();
}
