#include "render.h"

#include <ncurses.h>
#include <assert.h>
#include "wrapper.h"

#define WIN_FRAME_ROWS 2
#define WIN_FRAME_COLS 2

#define WIN_TOTAL_ROWS(rows) ((rows) + WIN_FRAME_ROWS)
#define WIN_TOTAL_COLS(cols) ((cols) * 2 + WIN_FRAME_COLS)

#define WIN_GRID_ROWS(rows) ((rows) - WIN_FRAME_ROWS)
#define WIN_GRID_COLS(cols) (((cols) - WIN_FRAME_COLS) / 2)

#define WIN_START_ROW 1
#define WIN_START_COL 1

struct _Render
{
	const char *title;
	int         view_row, view_col;
	int         win_rows, win_cols;
	int         cur_rows, cur_cols;
	WINDOW     *outer_box;
	WINDOW     *inner_box;
};

void
render_scroll (Render *render, const Grid *grid, int dx, int dy)
{
	assert (render != NULL);
	assert (grid != NULL);
	assert (grid->rows >= render->cur_rows
			&& grid->cols >= render->cur_cols);

	render->view_row += dx;
	render->view_col += dy;

	if (render->view_row < 0)
		render->view_row = 0;

	if (render->view_col < 0)
		render->view_col = 0;

	int max_x = grid->rows - render->cur_rows;
	int max_y = grid->cols - render->cur_cols;

	if (render->view_row > max_x)
		render->view_row = max_x;

	if (render->view_col > max_y)
		render->view_col = max_y;

	mvwprintw (render->inner_box, 0, 2, "[%07d,%07d]",
			render->view_row, render->view_col);
}

static inline void
render_resize_viewport (Render *render)
{
	int term_rows = 0;
	int term_cols = 0;

	getmaxyx (stdscr, term_rows, term_cols);

	resizeterm (term_rows, term_cols);
	clear ();

	wresize (render->outer_box, term_rows, term_cols);
	wclear (render->outer_box);
	box (render->outer_box, 0, 0);
	mvwprintw (render->outer_box, 0, 2, "[%s]", render->title);

	int inner_rows = term_rows - WIN_FRAME_ROWS * 2;
	int inner_cols = term_cols - WIN_FRAME_COLS * 20;
	int total_rows = WIN_TOTAL_ROWS (render->win_rows);
	int total_cols = WIN_TOTAL_COLS (render->win_cols);

	if (total_rows < WIN_FRAME_ROWS + 1 || total_rows > inner_rows)
		total_rows = inner_rows;

	if (total_cols < WIN_FRAME_COLS + 1 || total_cols > inner_cols)
		total_cols = inner_cols;

	render->cur_rows = WIN_GRID_ROWS (total_rows);
	render->cur_cols = WIN_GRID_COLS (total_cols);

	int inner_startx = 2;
	int inner_starty = 2;

	wresize (render->inner_box, total_rows, total_cols);
	wclear (render->inner_box);
	box (render->inner_box, 0, 0);
	mvwprintw (render->inner_box, 0, 2, "[%07d,%07d]",
			render->view_row, render->view_col);

	mvwin (render->inner_box, inner_starty, inner_startx);
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

Render *
render_new (const char *title, int win_rows, int win_cols)
{
	assert (title != NULL);

	start_color ();
	init_pair (1, COLOR_WHITE, COLOR_BLACK);
	init_pair (2, COLOR_BLACK, COLOR_WHITE);

	Render *render = xcalloc (1, sizeof (Render));

	*render = (Render) {
		.title     = xstrdup (title),
		.outer_box = newwin (0, 0, 0, 0),
		.inner_box = newwin (0, 0, 0, 0),
		.win_rows  = win_rows,
		.win_cols  = win_cols
	};

	wbkgd (render->outer_box, COLOR_PAIR (1));
	wbkgd (render->inner_box, COLOR_PAIR (2));

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

	xfree ((char *) render->title);
	xfree (render);
}

void
render_draw (Render *render, const Grid *grid)
{
	assert (render != NULL);
	assert (grid != NULL);

	int cell = 0;

	for (int i = 0; i < render->cur_rows; i++)
		for (int j = 0; j < render->cur_cols; j++)
			{
				cell = GRID_GET (grid, i + render->view_row, j + render->view_col);
				wattron  (render->inner_box, COLOR_PAIR (-1 * cell + 2));

				mvwaddch (render->inner_box,
						i + WIN_START_ROW,
						j * 2 + WIN_START_COL,
						' ');

				mvwaddch (render->inner_box,
						i + WIN_START_ROW,
						j * 2 + WIN_START_COL + 1,
						' ');

				wattroff (render->inner_box, COLOR_PAIR (-1 * cell + 2));
			}

	refresh  ();
	wrefresh (render->outer_box);
	wrefresh (render->inner_box);
}
