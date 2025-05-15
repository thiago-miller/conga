#include "grid.h"

#include <stdio.h>
#include "wrapper.h"

#define NEIGHBORS 8

static const int delta_i[] = {-1, -1, -1,  0, 0, +1, +1, +1};
static const int delta_j[] = {-1,  0, +1, -1, +1, -1, 0, +1};

Grid *
grid_new (int rows, int cols)
{
	assert (rows * cols > 0);

	Grid *grid = xcalloc (1, sizeof (Grid));

	*grid = (Grid) {
		.rows  = rows,
		.cols  = cols,
		.cur   = xcalloc (rows * cols, sizeof (int)),
		.next  = xcalloc (rows * cols, sizeof (int))
	};

	return grid;
}

void
grid_free (Grid *grid)
{
	if (grid == NULL)
		return;

	xfree (grid->cur);
	xfree (grid->next);

	xfree (grid);
}

int
grid_count_neighbors (const Grid *grid, int i, int j)
{
	assert (grid != NULL);
	assert (GRID_RANGE_CHECK (grid, i, j));

	int ni = 0, nj = 0;
	int live_neighbors = 0;

	for (int k = 0; k < NEIGHBORS; k++)
		{
			ni = (i + delta_i[k] + grid->rows) % grid->rows;
			nj = (j + delta_j[k] + grid->cols) % grid->cols;

			if (GRID_CUR_GET (grid, ni, nj))
				live_neighbors++;
		}

	return live_neighbors;
}
