#include "cell.h"

#include <stdio.h>
#include <assert.h>
#include "wrapper.h"
#include "utils.h"

void
cell_seed_random_generation (Grid *grid, Rand *rng, float live_percent, Cell *cell)
{
	assert (grid != NULL);
	assert (live_percent > 0 && live_percent < 1);

	int total_cells = grid->rows * grid->cols;
	int cells_alive = total_cells * live_percent;
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

	for (i = 0; i < cells_alive; i++)
		grid->data[pos[i]] = 1;

	xfree (pos);

	if (cell != NULL)
		cell->alive = cells_alive;
}

void
cell_seed_from_grid (Grid *grid_to, const Grid *grid_from, Cell *cell)
{
	assert (grid_to != NULL && grid_from != NULL);
	assert (grid_to->rows >= grid_from->rows
			&& grid_to->cols >= grid_from->cols);

	int offset_row = (grid_to->rows - grid_from->rows) / 2;
	int offset_col = (grid_to->cols - grid_from->cols) / 2;
	int cells_alive = 0;
	int alive = 0;

	for (int i = 0; i < grid_from->rows; i++)
		for (int j = 0; j < grid_from->cols; j++)
			{
				alive = GRID_GET (grid_from, i, j);
				GRID_SET (grid_to, offset_row + i, offset_col + j, alive);
				cells_alive += alive;
			}

	if (cell != NULL)
		cell->alive = cells_alive;
}

void
cell_step_generation (Grid *grid_next, const Grid *grid_cur, Rule *rule, Cell *cell)
{
	assert (grid_next != NULL && grid_cur != NULL);
	assert (grid_next->rows == grid_cur->rows
			&& grid_next->cols == grid_cur->cols);
	assert (rule != NULL);

	int cells_alive = 0;
	int neighbors = 0;
	int alive = 0;

	for (int i = 0; i < grid_next->rows; i++)
		{
			for (int j = 0; j < grid_next->cols; j++)
				{
					alive     = GRID_GET (grid_cur, i, j);
					neighbors = grid_count_neighbors (grid_cur, i, j);

					GRID_SET (grid_next, i, j,
							rule_next_state (rule, alive, neighbors));

					cells_alive += alive;
				}
		}

	if (cell != NULL)
		{
			cell->alive = cells_alive;
			cell->gen += 1;
		}
}
