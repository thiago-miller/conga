#pragma once

#include "grid.h"

typedef struct
{
	int            rows;
	int            cols;
	const char    *rule;
} PatternHeader;

typedef struct
{
	PatternHeader  header;
	Grid          *grid;
} Pattern;

typedef struct
{
	PatternHeader  header;
	const char    *name;
	const char    *desc;
	const char    *rle;
} PatternDef;

extern const PatternDef pattern_defs[];

Pattern * pattern_new            (const char *pattern_str);
int       pattern_file_is_valid  (const char *pattern_file);
int       pattern_alias_is_valid (const char *pattern_alias);
void      pattern_free           (Pattern *pattern);
