#pragma once

#include "grid.h"
#include "cell.h"

typedef struct _Render Render;

Render * render_new          (const char *title, int rows, int cols);
void     render_draw         (Render *render, const Grid *grid, const Cell *cell);
int      render_scroll       (Render *render, const Grid *grid, int dx, int dy);
void     render_force_resize (Render *render);
void     render_free         (Render *render);
