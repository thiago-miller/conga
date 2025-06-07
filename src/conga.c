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
	struct
	{
		int   done;
		int   paused;
		int   redraw;
		int   resize;
	} status;
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

	char *title = NULL;
	asprintf (&title, "%s %s", cfg->progname, cfg->version);

	*game = (Conga) {
		.queue     = event_queue_new (FPS, cfg->delay),
		.render    = render_new      (title, rows, cols),
		.grid_cur  = grid_new        (rows, cols),
		.grid_next = grid_new        (rows, cols),
		.rule      = rule_new        (rule),
		.rng       = NULL
	};

	cell_seed_from_grid (game->grid_cur, pattern->grid);

	xfree (title);
	pattern_free (pattern);
}

static void
conga_set_random_game (Conga *game, const Config *cfg)
{
	char *title = NULL;
	asprintf (&title, "%s %s", cfg->progname, cfg->version);

	*game = (Conga) {
		.queue     = event_queue_new (FPS, cfg->delay),
		.render    = render_new      (title, cfg->rows, cfg->cols),
		.grid_cur  = grid_new        (cfg->rows, cfg->cols),
		.grid_next = grid_new        (cfg->rows, cfg->cols),
		.rule      = rule_new        (cfg->rule),
		.rng       = rand_new        (cfg->seed)
	};

	cell_seed_random_generation (game->grid_cur,
			game->rng, cfg->live_percent);

	xfree (title);
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
conga_update_graphics (Conga *game)
{
	if (game->status.resize)
		render_force_resize (game->render);
	render_draw (game->render, game->grid_cur);
}

static inline void
conga_input_key (Conga *game, int key)
{
	switch (key)
		{
		case 'q':
		case 'Q':
			{
				game->status.done = 1;
				break;
			}
		case '\n':
		case ' ' :
			{
				game->status.paused = !(game->status.paused);
				event_queue_pause (game->queue, game->status.paused);
				break;
			}
		case KEY_UP:
			{
				render_scroll (game->render, game->grid_cur, -1, 0);
				game->status.redraw = 1;
				break;
			}
		case KEY_DOWN:
			{
				render_scroll (game->render, game->grid_cur, +1, 0);
				game->status.redraw = 1;
				break;
			}
		case KEY_LEFT:
			{
				render_scroll (game->render, game->grid_cur, 0, -1);
				game->status.redraw = 1;
				break;
			}
		case KEY_RIGHT:
			{
				render_scroll (game->render, game->grid_cur, 0, +1);
				game->status.redraw = 1;
				break;
			}
		case 'g':
			{
				render_scroll (game->render, game->grid_cur,
						-1 * game->grid_cur->rows, 0);
				game->status.redraw = 1;
				break;
			}
		case 'G':
			{
				render_scroll (game->render, game->grid_cur,
						game->grid_cur->rows, 0);
				game->status.redraw = 1;
				break;
			}
		case '$':
			{
				render_scroll (game->render, game->grid_cur,
						0, game->grid_cur->cols);
				game->status.redraw = 1;
				break;
			}
		case '0':
			{
				render_scroll (game->render, game->grid_cur,
						0, -1 * game->grid_cur->cols);
				game->status.redraw = 1;
				break;
			}
		case 'o':
			{
				render_scroll (game->render, game->grid_cur,
						-1 * game->grid_cur->rows, -1 * game->grid_cur->cols);
				game->status.redraw = 1;
				break;
			}
		case 'O':
			{
				render_scroll (game->render, game->grid_cur,
						game->grid_cur->rows, game->grid_cur->cols);
				game->status.redraw = 1;
				break;
			}
		}
}

void
conga_run (Conga *game)
{
	Event event  = {0};

	conga_update_graphics (game);

	while (!game->status.done)
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
						game->status.done = 1;
						break;
					}
				case EVENT_KEY:
					{
						conga_input_key (game, event.key);
						break;
					}
				case EVENT_TIMER:
					{
						conga_update_logic (game);
						game->status.redraw = 1;
						break;
					}
				case EVENT_WINCH:
					{
						game->status.redraw = 1;
						game->status.resize = 1;
						break;
					}
				}

			if (game->status.redraw)
				{
					conga_update_graphics (game);
					game->status.redraw = 0;
					game->status.resize = 0;
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
