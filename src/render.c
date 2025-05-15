#include "render.h"

#include <stdio.h>
#include <assert.h>

#define CURSOR_OFF   "\e[?25l"
#define CURSOR_ON    "\e[?25h"
#define CURSOR_UP    "\e[H"
#define CLEAN_SCREEN "\e[H\e[2J"
#define CLEAN_LINE   "\e[A\e[k"
/*#define DEAD         "\u2B1B"*/
#define DEAD         "\u26AB"
/*#define ALIVE        "\u2B1C"*/
#define ALIVE        "\u2B55"

// x26AA white
// x26AB black

static const char *const status[] = {DEAD, ALIVE};

void
render_init (void)
{
	printf (CLEAN_SCREEN);
	printf (CURSOR_OFF);
}

void
render_finish (void)
{
	printf (CURSOR_UP);
	printf (CURSOR_ON);
}

void
render_draw (const Grid *grid)
{
	assert (grid != NULL);

	printf (CURSOR_UP);

	for (int i = 0; i < grid->rows; i++)
		{
			for (int j = 0; j < grid->cols; j++)
				printf (status[GRID_CUR_GET (grid, i, j)]);

			printf ("\n");
		}
}
