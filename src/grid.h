#pragma once

#include <assert.h>

typedef struct
{
	int  rows;
	int  cols;
	int *cur;
	int *next;
} Grid;

#define GRID_RANGE_CHECK(g,r,c) ( \
		(r) >= 0 && (r) < (g)->rows && \
		(c) >= 0 && (c) < (g)->cols \
)

#define __GRID_GET(g,d,r,c) ( \
		(g)->d[(r) * (g)->cols + (c)] \
)

#define __GRID_SET(g,d,r,c,x) ( \
		__GRID_GET(g,d,r,c) = (x) \
)

#define GRID_CUR_GET(g,r,c) ( \
		__GRID_GET(g,cur,r,c) \
)

#define GRID_CUR_SET(g,r,c,x) ( \
		__GRID_SET(g,cur,r,c,x) \
)

#define GRID_NEXT_GET(g,r,c) ( \
		__GRID_GET(g,next,r,c) \
)

#define GRID_NEXT_SET(g,r,c,x) ( \
		__GRID_SET(g,next,r,c,x) \
)

#define GRID_SWAP(g) do { \
	int *tmp = (g)->cur; \
	(g)->cur = (g)->next; \
	(g)->next = tmp; \
} while (0)

Grid * grid_new             (int rows, int cols);
void   grid_free            (Grid *g);
int    grid_count_neighbors (const Grid *g, int row, int col);
