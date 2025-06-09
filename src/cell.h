#pragma once

#include "grid.h"
#include "rule.h"
#include "rand.h"

typedef struct
{
	int alive;
	int gen;
} Cell;

void cell_seed_random_generation (Grid *grid, Rand *rng, float live_percent, Cell *cell);
void cell_seed_from_grid         (Grid *grid_to, const Grid *grid_from, Cell *cell);
void cell_step_generation        (Grid *grid_next, const Grid *grid_cur, Rule *rule, Cell *cell);
