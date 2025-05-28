#pragma once

#include "grid.h"

typedef struct
{
	int   rows;
	int   cols;
	char *rule;
	Grid *grid;
} Pattern;


Pattern * pattern_new  (const char *filename);
void      pattern_free (Pattern *pattern);
