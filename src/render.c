#include "render.h"

#include <ncurses.h>
#include <assert.h>
#include "wrapper.h"

#define BOX_FRAME_ROWS 2
#define BOX_FRAME_COLS 2

#define BOX_TOTAL_ROWS(rows) ((rows) + BOX_FRAME_ROWS)
#define BOX_TOTAL_COLS(cols) ((cols) * 2 + BOX_FRAME_COLS)

#define BOX_GRID_ROWS(rows) ((rows) - BOX_FRAME_ROWS)
#define BOX_GRID_COLS(cols) (((cols) - BOX_FRAME_COLS) / 2)

#define BOX_START_ROW 1
#define BOX_START_COL 1

struct _Render
{
	const char *title;
	int         rows, cols;
	int         cur_rows, cur_cols;
	WINDOW     *outer_box;
	WINDOW     *inner_box;
};

static inline void
render_redraw (Render *render)
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

	int inner_rows = term_rows - BOX_FRAME_ROWS * 2;
	int inner_cols = term_cols - BOX_FRAME_COLS * 20;
	int total_rows = BOX_TOTAL_ROWS (render->rows);
	int total_cols = BOX_TOTAL_COLS (render->cols);

	if (total_rows < BOX_FRAME_ROWS + 1 || total_rows > inner_rows)
		total_rows = inner_rows;

	if (total_cols < BOX_FRAME_COLS + 1 || total_cols > inner_cols)
		total_cols = inner_cols;

	render->cur_rows = BOX_GRID_ROWS (total_rows);
	render->cur_cols = BOX_GRID_COLS (total_cols);

	int inner_startx = 2;
	int inner_starty = 2;

	wresize (render->inner_box, total_rows, total_cols);
	wclear (render->inner_box);
	box (render->inner_box, 0, 0);
	mvwprintw (render->inner_box, 0, 2, "[%d,%d]",
			render->cur_rows, render->cur_cols);

	mvwin (render->inner_box, inner_starty, inner_startx);
}

void
render_force_redraw (Render *render)
{
	assert (render != NULL);

	// Force to get the terminal size
	endwin();
	refresh();

	render_redraw (render);
}

Render *
render_new (const char *title, int rows, int cols)
{
	assert (title != NULL);

	start_color ();
	init_pair (1, COLOR_WHITE, COLOR_BLACK);
	init_pair (2, COLOR_BLACK, COLOR_WHITE);

	Render *render = xmalloc (sizeof (Render));

	*render = (Render) {
		.title     = xstrdup (title),
		.outer_box = newwin (0, 0, 0, 0),
		.inner_box = newwin (0, 0, 0, 0),
		.rows      = rows,
		.cols      = cols,
	};

	wbkgd (render->outer_box, COLOR_PAIR (1));
	wbkgd (render->inner_box, COLOR_PAIR (2));

	render_redraw (render);

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
render_draw (Render *render, const Grid *grid_cur)
{
	assert (render != NULL);
	assert (grid_cur != NULL);

	int cell = 0;

	for (int i = 0; i < render->cur_rows; i++)
		for (int j = 0; j < render->cur_cols; j++)
			{
				cell = GRID_GET (grid_cur, i, j);
				wattron  (render->inner_box, COLOR_PAIR (-1 * cell + 2));

				mvwaddch (render->inner_box,
						i + BOX_START_ROW,
						j * 2 + BOX_START_COL,
						' ');

				mvwaddch (render->inner_box,
						i + BOX_START_ROW,
						j * 2 + BOX_START_COL + 1,
						' ');

				wattroff (render->inner_box, COLOR_PAIR (-1 * cell + 2));
			}

	refresh  ();
	wrefresh (render->outer_box);
	wrefresh (render->inner_box);
}
