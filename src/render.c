#include "render.h"

#include <stdio.h>
#include <assert.h>

#define CURSOR_OFF   "\e[?25l"
#define CURSOR_ON    "\e[?25h"
#define CURSOR_UP    "\e[H"
#define CLEAN_SCREEN "\e[H\e[2J"
#define CLEAN_LINE   "\e[A\e[k"
#define DEAD         "\u2B1B"
#define ALIVE        "\u2B1C"

static char *status[] = {DEAD, ALIVE};

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
render_draw (const Grid *g)
{
	assert (g != NULL);

	printf (CURSOR_UP);

	for (int r = 0; r < g->rows; r++)
		{
			for (int c = 0; c < g->cols; c++)
				printf (status[GRID_CUR_GET (g, r, c)]);

			printf ("\n");
		}
}
