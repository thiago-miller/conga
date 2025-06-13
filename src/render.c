#include "render.h"

#include <ncurses.h>
#include <assert.h>
#include <math.h>
#include "wrapper.h"
#include "utils.h"

#define WIN_FRAME_ROWS 2
#define WIN_FRAME_COLS 2

#define WIN_TOTAL_ROWS(rows) ((rows) + WIN_FRAME_ROWS)
#define WIN_TOTAL_COLS(cols) ((cols) * 2 + WIN_FRAME_COLS)

#define WIN_GRID_ROWS(rows) ((rows) - WIN_FRAME_ROWS)
#define WIN_GRID_COLS(cols) (((cols) - WIN_FRAME_COLS) / 2)

#define WIN_START_ROW 1
#define WIN_START_COL 1

#define COLOR_LIGHT_GRAY 8
#define COLOR_DARK_GRAY  9

#define COLOR_PAIR_OUTER_BOX   1
#define COLOR_PAIR_INNER_BOX   2
#define COLOR_PAIR_STATUS_BOX  3
#define COLOR_PAIR_CELL_00     4
#define COLOR_PAIR_CELL_25     5
#define COLOR_PAIR_CELL_50     6
#define COLOR_PAIR_CELL_75     7

struct _Render
{
	const char *title;

	int         view_row, view_col;
	int         win_rows, win_cols;
	int         cur_rows, cur_cols;

	int         scale;

	int         update_scroll;
	int         clear_grid;

	WINDOW     *outer_box;
	WINDOW     *inner_box;
	WINDOW     *status_box;
};

int
render_scroll (Render *render, const Grid *grid, int dx, int dy)
{
	assert (render != NULL);
	assert (grid != NULL);
	assert (grid->rows >= render->cur_rows
			&& grid->cols >= render->cur_cols);

	int view_row = render->view_row;
	int view_col = render->view_col;

	int fac   = exp2 (render->scale);
	int max_x = fmax (0, (grid->rows / fac - render->cur_rows) * fac);
	int max_y = fmax (0, (grid->cols / fac - render->cur_cols) * fac);

	int rc = 0;

	view_row += dx * fac;
	view_col += dy * fac;

	if (view_row < 0)
		view_row = 0;
	else if (view_row > max_x)
		view_row = max_x;

	if (view_col < 0)
		view_col = 0;
	else if (view_col > max_y)
		view_col = max_y;

	if (render->view_row != view_row
			|| render->view_col != view_col)
		{
			render->view_row = view_row;
			render->view_col = view_col;
			rc = 1;
		}

	return rc;
}

int
render_scale (Render *render, const Grid *grid, int dx)
{
	assert (render != NULL);
	assert (grid != NULL);

	int max_scale = log2 (fmin (grid->rows, grid->cols));
	int scale = render->scale;
	int rc = 0;

	scale += dx;

	if (scale < 0)
		scale = 0;
	else if (scale > max_scale)
		scale = max_scale;

	if (render->scale != scale)
		{
			render->scale = scale;
			render->clear_grid = 1;
			render->update_scroll = 1;
			rc = 1;
		}

	return rc;
}

static inline void
render_resize_viewport (Render *render)
{
	int term_rows = 0;
	int term_cols = 0;

	getmaxyx (stdscr, term_rows, term_cols);

	// Resize terminal
	resizeterm (term_rows, term_cols);
	clear ();

	// Resize outer_box
	wresize (render->outer_box, term_rows, term_cols);
	wclear (render->outer_box);
	box (render->outer_box, 0, 0);

	// Place title
	wmove (render->outer_box, 0, 2);
	waddch (render->outer_box, ACS_URCORNER);
	wattron (render->outer_box, A_BOLD);
	wprintw (render->outer_box, "%s", render->title);
	wattroff (render->outer_box, A_BOLD);
	waddch (render->outer_box, ACS_ULCORNER);

	// Get inner_rows dimensions
	int inner_rows = term_rows - WIN_FRAME_ROWS * 2 - 1;
	int inner_cols = term_cols - WIN_FRAME_COLS * 20;
	int total_rows = WIN_TOTAL_ROWS (render->win_rows);
	int total_cols = WIN_TOTAL_COLS (render->win_cols);

	// Set inner_box limits
	if (total_rows < WIN_TOTAL_ROWS (1))
		total_rows = WIN_TOTAL_ROWS (1);
	else if (total_rows > inner_rows)
		total_rows = inner_rows;

	if (total_cols < WIN_TOTAL_COLS (1))
		total_cols = WIN_TOTAL_COLS (1);
	else if (total_cols > inner_cols)
		total_cols = inner_cols;

	render->cur_rows = WIN_GRID_ROWS (total_rows);
	render->cur_cols = WIN_GRID_COLS (total_cols);

	// Set inner_box position at the center left
	int inner_startx = WIN_FRAME_COLS;
	int inner_starty = (term_rows - total_rows) / 2;

	// Draw inner_box
	wresize (render->inner_box, total_rows, total_cols);
	wclear (render->inner_box);
	box (render->inner_box, 0, 0);
	mvwin (render->inner_box, inner_starty, inner_startx);

	// Draw status_box
	wresize (render->status_box,
			1, term_cols - 2 * WIN_FRAME_COLS);
	wclear (render->status_box);
	mvwin (render->status_box,
			term_rows - WIN_FRAME_ROWS, WIN_FRAME_COLS);

	// Scroll next render_update_grid in order
	// to avoid view_row/view_col out of range
	render->update_scroll = 1;
}

void
render_force_resize (Render *render)
{
	assert (render != NULL);

	// Force ncurses to update terminal size info
	// (workaround for SIGWINCH)
	endwin();
	refresh();

	render_resize_viewport (render);
}

static void
render_init_colors (void)
{
	start_color ();

	init_color (COLOR_RED,        700, 0,   0);
	init_color (COLOR_LIGHT_GRAY, 500, 500, 500);
	init_color (COLOR_DARK_GRAY,  250, 250, 250);

	init_pair (COLOR_PAIR_OUTER_BOX,  COLOR_WHITE, COLOR_BLACK);
	init_pair (COLOR_PAIR_INNER_BOX,  COLOR_BLACK, COLOR_WHITE);
	init_pair (COLOR_PAIR_STATUS_BOX, COLOR_WHITE, COLOR_RED);
	init_pair (COLOR_PAIR_CELL_00,    COLOR_BLACK, COLOR_WHITE);
	init_pair (COLOR_PAIR_CELL_25,    COLOR_BLACK, COLOR_LIGHT_GRAY);
	init_pair (COLOR_PAIR_CELL_50,    COLOR_BLACK, COLOR_DARK_GRAY);
	init_pair (COLOR_PAIR_CELL_75,    COLOR_BLACK, COLOR_BLACK);
}

Render *
render_new (const char *title, int win_rows, int win_cols)
{
	assert (title != NULL);

	render_init_colors ();

	Render *render = xcalloc (1, sizeof (Render));

	*render = (Render) {
		.title      = xstrdup (title),
		.outer_box  = newwin (0, 0, 0, 0),
		.inner_box  = newwin (0, 0, 0, 0),
		.status_box = newwin (0, 0, 0, 0),
		.win_rows   = win_rows,
		.win_cols   = win_cols
	};

	wbkgd (render->outer_box,
			COLOR_PAIR (COLOR_PAIR_OUTER_BOX));
	wbkgd (render->inner_box,
			COLOR_PAIR (COLOR_PAIR_INNER_BOX));
	wbkgd (render->status_box,
			COLOR_PAIR (COLOR_PAIR_STATUS_BOX));

	render_resize_viewport (render);

	return render;
}

void
render_free (Render *render)
{
	if (render == NULL)
		return;

	delwin (render->outer_box);
	delwin (render->inner_box);
	delwin (render->status_box);

	xfree ((char *) render->title);
	xfree (render);
}

static inline void
render_clear_grid (Render *render)
{
	wclear (render->inner_box);
	box (render->inner_box, 0, 0);
}

static inline void
render_update_grid (Render *render, const Grid *grid)
{
	int fac = exp2 (render->scale);
	int max_i = fmin (render->cur_rows * fac, grid->rows);
	int max_j = fmin (render->cur_cols * fac, grid->cols);

	for (int i = 0, row = 0; i < max_i; i += fac, row++)
		for (int j = 0, col = 0; j < max_j; j += fac, col++)
			{
				int view_row_shift = i + render->view_row;
				int view_col_shift = j + render->view_col;
				int acm = 0;

				for (int x = 0; x < fac && x + view_row_shift < grid->rows; x++)
					for (int y = 0; y < fac && y + view_col_shift < grid->cols; y++)
						acm += GRID_GET (grid, x + view_row_shift, y + view_col_shift);

				int color_pair = COLOR_PAIR_CELL_00 + ceil (acm * 4.0 / fac);

				wattron  (render->inner_box, COLOR_PAIR (color_pair));

				mvwaddch (render->inner_box,
						row + WIN_START_ROW,
						col * 2 + WIN_START_COL,
						' ');

				mvwaddch (render->inner_box,
						row + WIN_START_ROW,
						col * 2 + WIN_START_COL + 1,
						' ');

				wattroff (render->inner_box, COLOR_PAIR (color_pair));
			}
}

static inline void
render_update_status (Render *render, const Grid *grid,
		const RenderStat *stat)
{
	// Clean windows
	wmove(render->status_box, 0, 0);
	for (int i = 0; i < WIN_TOTAL_COLS (render->cur_cols); i++)
		waddch (render->status_box, ' ');

	int fac = exp2 (render->scale);

	wmove (render->status_box, 0, 0);
	wprintw (render->status_box, "[%d,%d] ",
			render->view_row/fac, render->view_col/fac);

	wattron (render->status_box, A_BOLD);
	wprintw (render->status_box, "View:");
	wattroff (render->status_box, A_BOLD);
	wprintw (render->status_box, "%dx%d ",
			render->cur_rows, render->cur_cols);

	wattron (render->status_box, A_BOLD);
	wprintw (render->status_box, "Grid:");
	wattroff (render->status_box, A_BOLD);
	wprintw (render->status_box, "%dx%d ",
			grid->rows, grid->cols);

	wattron (render->status_box, A_BOLD);
	wprintw (render->status_box, "Alive:");
	wattroff (render->status_box, A_BOLD);
	wprintw (render->status_box, "%u ",
			stat->alive);

	wattron (render->status_box, A_BOLD);
	wprintw (render->status_box, "Gen:");
	wattroff (render->status_box, A_BOLD);
	wprintw (render->status_box, "%u ",
			stat->gen);

	wattron (render->status_box, A_BOLD);
	wprintw (render->status_box, "Gen/s:");
	wattroff (render->status_box, A_BOLD);
	wprintw (render->status_box, "%.2f ",
			stat->rate);

	wattron (render->status_box, A_BOLD);
	wprintw (render->status_box, "Scale:");
	wattroff (render->status_box, A_BOLD);
	wprintw (render->status_box, "1:%d ", fac);
}

void
render_draw (Render *render, const Grid *grid, const RenderStat *stat)
{
	assert (render != NULL);
	assert (grid != NULL);

	if (render->update_scroll)
		{
			render_scroll (render, grid, 0, 0);
			render->update_scroll = 0;
		}

	if (render->clear_grid)
		{
			render_clear_grid (render);
			render->clear_grid = 0;
		}

	render_update_grid   (render, grid);
	render_update_status (render, grid, stat);

	refresh  ();
	wrefresh (render->outer_box);
	wrefresh (render->inner_box);
	wrefresh (render->status_box);
}
