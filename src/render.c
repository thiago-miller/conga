#include "render.h"

#include <stdio.h>
#include <assert.h>

#define CURSOR_OFF     "\e[?25l"
#define CURSOR_ON      "\e[?25h"
#define CURSOR_UP      "\e[H"
#define CLEAN_SCREEN   "\e[H\e[2J"
#define CLEAN_LINE     "\e[A\e[k"
#define DEAD_STABLE    "  "
#define DEAD_TO_LIVE   "⬜"
#define ALIVE_TO_DEAD  "🟪"
#define ALIVE_STABLE   "🟦"

static const char *const status[][2] =
{
	{DEAD_STABLE,   DEAD_TO_LIVE},
	{ALIVE_TO_DEAD, ALIVE_STABLE}
};

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
				printf (status[GRID_CUR_GET (grid, i, j)][GRID_NEXT_GET (grid, i, j)]);

			printf ("\n");
		}
}
