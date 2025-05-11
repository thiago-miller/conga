#include "cell.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "wrapper.h"
#include "utils.h"

void
cell_set_first_generation (Grid *g, float live_percent)
{
	assert (g != NULL);
	assert (live_percent > 0 && live_percent < 1);

	int total_cells = g->rows * g->cols;
	int live_cells = total_cells * live_percent;
	int i = 0;

	int *pos = xcalloc (total_cells, sizeof (int));

	for (i = 0; i < total_cells; i++)
		{
			// Get all positions to shuffle
			pos[i] = i;

			// Zero the grid matrix
			g->cur[i] = 0;
		}

	shuffle (pos, total_cells, rand);

	for (i = 0; i < live_cells; i++)
		g->cur[pos[i]] = 1;

	xfree (pos);
}

void
cell_step_generation (Grid *g)
{
	assert (g != NULL);

	int neighbors = 0;
	int alive = 0;

	for (int r = 0; r < g->rows; r++)
		{
			for (int c = 0; c < g->cols; c++)
				{
					alive = GRID_CUR_GET (g, r, c);

					neighbors = grid_count_neighbors (g, r, c);

					if (alive && (neighbors == 2 || neighbors == 3))
						GRID_NEXT_SET (g, r, c, 1);
					else if (!alive && neighbors == 3)
						GRID_NEXT_SET (g, r, c, 1);
					else
						GRID_NEXT_SET (g, r, c, 0);
				}
		}

	GRID_SWAP (g);
}
