#include "conga.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include "grid.h"
#include "cell.h"
#include "render.h"

static Grid *grid = NULL;
static int done = 0;
static int delay = 1000000;

static void
shutdown (int signum)
{
	done = 1;
}

void
conga_init (const Config *c)
{
	assert (c != NULL);

	signal (SIGINT,  shutdown);
	signal (SIGQUIT, shutdown);
	signal (SIGTERM, shutdown);

	srand (c->seed);

	grid = grid_new (c->rows, c->cols);
	cell_set_first_generation (grid, c->live_percent);

	delay = c->delay;

	render_init ();
}

void
conga_run (void)
{
	while (!done)
		{
			render_draw (grid);
			cell_step_generation (grid);
			usleep (delay);
		}
}

void
conga_finish (void)
{
	grid_free (grid);
	render_finish ();
}
