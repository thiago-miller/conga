#pragma once

#include "grid.h"
#include "rule.h"

void cell_set_first_generation (Grid *g, float live_percent);
void cell_step_generation      (Grid *g, Rule *r);
