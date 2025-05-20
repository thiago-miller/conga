#include "conga.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include "wrapper.h"
#include "grid.h"
#include "cell.h"
#include "render.h"
#include "rule.h"
#include "rand.h"
#include "input.h"

struct _Conga
{
	Grid *grid;
	Rule *rule;
	Rand *rng;
	int   delay;
};

static volatile sig_atomic_t die = 0;

static void
shutdown (int signum)
{
	die = 1;
}

void
conga_startup (void)
{
	signal (SIGINT,  shutdown);
	signal (SIGQUIT, shutdown);
	signal (SIGTERM, shutdown);

	input_init ();
	render_init ();
}

void
conga_shutdown (void)
{
	input_finish ();
	render_finish ();
}

Conga *
conga_new (const Config *cfg)
{
	assert (cfg != NULL);

	Conga *game = xcalloc (1, sizeof (Conga));

	*game = (Conga) {
		.grid  = grid_new (cfg->rows, cfg->cols),
		.rule  = rule_new (cfg->rule),
		.rng   = rand_new (cfg->seed),
		.delay = cfg->delay
	};

	cell_set_first_generation (game->grid, game->rng,
			cfg->live_percent);

	return game;
}

void
conga_run (Conga *game)
{
	int paused = 0;
	int done = 0;
	int key = 0;

	while (!die && !done)
		{
			key = input_read_key ();

			switch (key)
				{
				case INPUT_ESC:
					{
						done = 1;
						continue;
					}
				case INPUT_ENTER:
				case INPUT_SPACE:
					{
						paused = !paused;
						break;
					}
				}

			if (!paused)
				{
					cell_step_generation (game->grid, game->rule);
					render_draw (game->grid);
					GRID_SWAP (game->grid);
				}

			usleep (game->delay);
		}
}

void
conga_free (Conga *game)
{
	if  (game == NULL)
		return;

	grid_free (game->grid);
	rule_free (game->rule);
	rand_free (game->rng);

	xfree (game);
}
