#pragma once

#include "grid.h"

typedef struct _Render Render;

typedef struct
{
	int    alive;
	int    gen;
	double rate;
} RenderStat;

Render * render_new          (const char *title, int rows, int cols);
void     render_draw         (Render *render, const Grid *grid, const RenderStat *stat);
int      render_scroll       (Render *render, const Grid *grid, int dx, int dy);
void     render_force_resize (Render *render);
void     render_free         (Render *render);
