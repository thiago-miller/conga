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
render_draw (const Grid *grid_cur, const Grid *grid_next)
{
	assert (grid_cur != NULL && grid_next != NULL);
	assert (grid_cur->rows == grid_next->rows
			&& grid_cur->cols == grid_next->cols);

	printf (CURSOR_UP);

	for (int i = 0; i < grid_cur->rows; i++)
		{
			for (int j = 0; j < grid_cur->cols; j++)
				printf (status[GRID_GET (grid_cur, i, j)][GRID_GET (grid_next, i, j)]);

			printf ("\n");
		}
}
