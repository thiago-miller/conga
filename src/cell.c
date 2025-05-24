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
			grid->data[i] = 0;
		}

	shuffle (pos, total_cells, rng);

	for (i = 0; i < live_cells; i++)
		grid->data[pos[i]] = 1;

	xfree (pos);
}

void
cell_step_generation (Grid *grid_cur, Grid *grid_next, Rule *rule)
{
	assert (grid_cur != NULL && grid_next != NULL);
	assert (grid_cur->rows == grid_next->rows
			&& grid_cur->cols == grid_next->cols);
	assert (rule != NULL);

	int neighbors = 0;
	int alive = 0;

	for (int i = 0; i < grid_cur->rows; i++)
		{
			for (int j = 0; j < grid_cur->cols; j++)
				{
					alive     = GRID_GET (grid_cur, i, j);
					neighbors = grid_count_neighbors (grid_cur, i, j);

					GRID_SET (grid_next, i, j,
							rule_next_state (rule, alive, neighbors));
				}
		}
}
