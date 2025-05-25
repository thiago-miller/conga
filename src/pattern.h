#pragma once

#include "grid.h"

typedef struct
{
	int   rows;
	int   cols;
	char *rule;
	Grid *grid;
} Pattern;
