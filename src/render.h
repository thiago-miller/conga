#pragma once

#include "grid.h"

typedef struct _Render Render;

Render * render_new          (const char *title, int rows, int cols);
void     render_draw         (Render *render, const Grid *grid_cur);
void     render_force_redraw (Render *render);
void     render_free         (Render *render);
