#include "conga.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include "wrapper.h"
#include "grid.h"
#include "cell.h"
#include "render.h"
#include "rule.h"
#include "rand.h"
#include "input.h"

#ifdef _WIN32
#include <windows.h>
// Sleep in miliseconds
#define CONGA_SLEEP(u) Sleep(u / 1000);
#else
#include <unistd.h>
// Sleep in microseconds
#define CONGA_SLEEP(u) usleep(u);
#endif

#define FPS  60
#define TICK 1000000 / FPS

struct _Conga
{
	Grid *grid_cur;
	Grid *grid_next;
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
		.grid_cur  = grid_new (cfg->rows, cfg->cols),
		.grid_next = grid_new (cfg->rows, cfg->cols),
		.rule      = rule_new (cfg->rule),
		.rng       = rand_new (cfg->seed),
		.delay     = cfg->delay
	};

	cell_set_first_generation (game->grid_cur,
			game->rng, cfg->live_percent);
	cell_step_generation (game->grid_cur,
			game->grid_next, game->rule);

	return game;
}

static inline void
conga_swap_grids (Conga *game)
{
	Grid *tmp = game->grid_cur;
	game->grid_cur = game->grid_next;
	game->grid_next = tmp;
}

static inline void
conga_update_logic (Conga *game)
{
	conga_swap_grids (game);
	cell_step_generation (game->grid_cur,
			game->grid_next, game->rule);
}

static inline void
conga_update_graphics (Conga *game)
{
	render_draw (game->grid_cur, game->grid_next);
}

void
conga_run (Conga *game)
{
	int paused = 0;
	int done   = 0;
	int key    = 0;
	int tick   = 0;

	conga_update_graphics (game);

	while (!die && !done)
		{
			key = input_read_key ();

			switch (key)
				{
				case INPUT_ESC:
				case 'q':
				case 'Q':
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

			if (!paused && tick >= game->delay)
				{
					conga_update_logic (game);
					conga_update_graphics (game);
					tick %= game->delay;
				}

			CONGA_SLEEP (TICK);

			if (!paused)
				tick += TICK;
		}
}

void
conga_free (Conga *game)
{
	if  (game == NULL)
		return;

	grid_free (game->grid_cur);
	grid_free (game->grid_next);
	rule_free (game->rule);
	rand_free (game->rng);

	xfree (game);
}
