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

	Grid *g = xcalloc (1, sizeof (Grid));

	*g = (Grid) {
		.rows  = rows,
		.cols  = cols,
		.cur   = xcalloc (rows * cols, sizeof (int)),
		.next  = xcalloc (rows * cols, sizeof (int))
	};

	return g;
}

void
grid_free (Grid *g)
{
	if (g == NULL)
		return;

	xfree (g->cur);
	xfree (g->next);

	xfree (g);
}

int
grid_count_neighbors (const Grid *g, int i, int j)
{
	assert (g != NULL);
	assert (GRID_RANGE_CHECK (g, i, j));

	int ni = 0, nj = 0;
	int live_neighbors = 0;

	for (int k = 0; k < NEIGHBORS; k++)
		{
			ni = (i + delta_i[k] + g->rows) % g->rows;
			nj = (j + delta_j[k] + g->cols) % g->cols;

			if (GRID_CUR_GET (g, ni, nj))
				live_neighbors++;
		}

	return live_neighbors;
}
