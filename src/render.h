#pragma once

#include "grid.h"

void render_init   (void);
void render_draw   (const Grid *grid_cur, const Grid *grid_next);
void render_finish (void);
