#pragma once

typedef struct
{
	int  rows;
	int  cols;
	int *data;
} Grid;

#define GRID_GET(g,r,c) ( \
		(g)->data[(r) * (g)->cols + (c)] \
)

#define GRID_SET(g,r,c,x) ( \
		GRID_GET(g,r,c) = (x) \
)

#define GRID_RANGE_CHECK(g,r,c) ( \
		(r) >= 0 && (r) < (g)->rows && \
		(c) >= 0 && (c) < (g)->cols \
)

Grid * grid_new             (int rows, int cols);
void   grid_free            (Grid *grid);
int    grid_count_neighbors (const Grid *grid, int i, int j);
