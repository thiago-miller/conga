#include "cell.h"

#include <stdio.h>
#include <assert.h>
#include "wrapper.h"
#include "utils.h"

void
cell_set_first_generation (Grid *grid, Rand *rng, float live_percent)
{
	assert (grid != NULL);
	assert (live_percent > 0 && live_percent < 1);

	int total_cells = grid->rows * grid->cols;
	int live_cells = total_cells * live_percent;
	int i = 0;

	int *pos = xcalloc (total_cells, sizeof (int));

	for (i = 0; i < total_cells; i++)
		{
			// Get all positions to shuffle
			pos[i] = i;

			// Zero the grid matrix
			grid->cur[i] = 0;
		}

	shuffle (pos, total_cells, rng);

	for (i = 0; i < live_cells; i++)
		grid->cur[pos[i]] = 1;

	xfree (pos);
}

void
cell_step_generation (Grid *grid, Rule *rule)
{
	assert (grid != NULL);
	assert (rule != NULL);

	int neighbors = 0;
	int alive = 0;

	for (int i = 0; i < grid->rows; i++)
		{
			for (int j = 0; j < grid->cols; j++)
				{
					alive     = GRID_CUR_GET (grid, i, j);
					neighbors = grid_count_neighbors (grid, i, j);

					GRID_NEXT_SET (grid, i, j,
							rule_next_state (rule, alive, neighbors));
				}
		}
}
