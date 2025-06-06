#include "conga.h"

#include <ncurses.h>
#include <stdlib.h>
#include <assert.h>
#include "wrapper.h"
#include "event.h"
#include "grid.h"
#include "cell.h"
#include "render.h"
#include "rule.h"
#include "rand.h"
#include "pattern.h"

#define FPS 60

struct _Conga
{
	EventQueue *queue;
	Render     *render;
	Grid       *grid_cur;
	Grid       *grid_next;
	Rule       *rule;
	Rand       *rng;
};

static void
conga_set_game_from_pattern (Conga *game, const Config *cfg)
{
	Pattern *pattern = pattern_new (cfg->pattern_file);

	const char *rule = pattern->header.rule != NULL
		? pattern->header.rule
		: cfg->rule;

	int rows = pattern->grid->rows < cfg->rows
		? cfg->rows
		: pattern->grid->rows;

	int cols = pattern->grid->cols < cfg->cols
		? cfg->cols
		: pattern->grid->cols;

	*game = (Conga) {
		.queue     = event_queue_new (FPS, cfg->delay),
		.render    = render_new      ("ponga", rows, cols),
		.grid_cur  = grid_new        (rows, cols),
		.grid_next = grid_new        (rows, cols),
		.rule      = rule_new        (rule),
		.rng       = NULL
	};

	cell_seed_from_grid (game->grid_cur, pattern->grid);

	pattern_free (pattern);
}

static void
conga_set_random_game (Conga *game, const Config *cfg)
{
	*game = (Conga) {
		.queue     = event_queue_new (FPS, cfg->delay),
		.render    = render_new      ("ponga", cfg->rows, cfg->cols),
		.grid_cur  = grid_new        (cfg->rows, cfg->cols),
		.grid_next = grid_new        (cfg->rows, cfg->cols),
		.rule      = rule_new        (cfg->rule),
		.rng       = rand_new        (cfg->seed)
	};

	cell_seed_random_generation (game->grid_cur,
			game->rng, cfg->live_percent);
}

Conga *
conga_new (const Config *cfg)
{
	assert (cfg != NULL);

	Conga *game = xcalloc (1, sizeof (Conga));

	if (cfg->pattern_file != NULL)
		conga_set_game_from_pattern (game, cfg);
	else
		conga_set_random_game (game, cfg);

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
	cell_step_generation (game->grid_next,
			game->grid_cur, game->rule);
	conga_swap_grids (game);
}

static inline void
conga_update_graphics (Conga *game, int redraw)
{
	if (redraw)
		render_force_redraw (game->render);
	render_draw (game->render, game->grid_cur);
}

static inline void
conga_input_key (Conga *game, int key, int *done, int *paused)
{
	switch (key)
		{
		case 'q':
		case 'Q':
			{
				*done = 1;
				break;
			}
		case '\n':
		case ' ' :
			{
				*paused = !(*paused);
				event_queue_pause (game->queue, *paused);
				break;
			}
		}
}

void
conga_run (Conga *game)
{
	Event event  = {0};
	int   done   = 0;
	int   paused = 0;
	int   draw   = 0;
	int   redraw = 0;

	conga_update_graphics (game, 0);

	while (!done)
		{
			event_queue_wait_for_event (game->queue, &event);

			switch (event.type)
				{
				case EVENT_NONE:
					{
						// Just remove warning
						break;
					}
				case EVENT_QUIT:
					{
						done = 1;
						break;
					}
				case EVENT_KEY:
					{
						conga_input_key (game, event.key, &done, &paused);
						break;
					}
				case EVENT_TIMER:
					{
						conga_update_logic (game);
						draw = 1;
						break;
					}
				case EVENT_WINCH:
					{
						draw   = 1;
						redraw = 1;
						break;
					}
				}

			if (draw)
				{
					conga_update_graphics (game, redraw);
					draw   = 0;
					redraw = 0;
				}
		}
}

void
conga_free (Conga *game)
{
	if  (game == NULL)
		return;

	event_queue_free (game->queue);
	render_free      (game->render);
	grid_free        (game->grid_cur);
	grid_free        (game->grid_next);
	rule_free        (game->rule);
	rand_free        (game->rng);

	xfree (game);
}
