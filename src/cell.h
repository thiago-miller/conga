#pragma once

#include "grid.h"
#include "rule.h"
#include "rand.h"

void cell_set_first_generation (Grid *grid, Rand *rng, float live_percent);
void cell_step_generation      (Grid *grid_cur, Grid *grid_next, Rule *rule);
