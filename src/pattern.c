#include "pattern.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "wrapper.h"
#include "error.h"
#include "utils.h"
#include "rule.h"

#ifdef HAVE_PATTERN_DEFS_H
#include "pattern_defs.h"
#else
const PatternDef pattern_defs[] =
{
	{
		{3, 3, "B3/S23"},
		"glider",
		"Smallest moving pattern; moves diagonally",
		"bob$2bo$3o!"
	},
	{ {}, NULL, NULL, NULL }
};
#endif

enum PatternTokenType
{
	DEAD    = 264,
	ALIVE   = 265,
	NEWROW  = 266,
	END     = 267,
	COUNT   = 270,
	MISTERY = 271
};

static const PatternDef *
pattern_get_def_from_alias (const char *alias)
{
	const PatternDef *def = NULL;

	for (const PatternDef *di = pattern_defs; di->name != NULL; di++)
		{
			if (strcasecmp (alias, di->name) == 0)
				{
					def = di;
					break;
				}
		}

	return def;
}

static int
pattern_rle_lex (const char *rle, const char **pp, int *val)
{
	if (rle != NULL)
		*pp = rle;

	for (char c = *(*pp)++; c != '\0'; c = *(*pp)++)
		{
			if (isdigit (c))
				{
					int count = 0;

					do
					{
						count = 10 * count + c - '0';
					}
					while (isdigit (c = *(*pp)++));

					if (val != NULL)
						*val = count;

					// ungetc
					(*pp)--;

					return COUNT;
				}

			switch (c)
				{
				case 'b'  : case 'B':  return DEAD;
				case 'o'  : case 'O':  return ALIVE;
				case '$'  :            return NEWROW;
				case '!'  :            return END;
				case ' '  : case '\t':
				case '\v' : case '\f':
				case '\n' : case '\r': break;
				default   :
					{
						error (0, 0, "Mistery character '%c'", c);
						return MISTERY;
					}
				}
		}

	return 0;
}

static int
pattern_rle_header_parse (Pattern *pattern, const char *h)
{
	int x = 0, y = 0;
	char rule[64] = {0};

	if (sscanf (h, "x = %d, y = %d, rule = %63s", &x, &y, rule) == 3
			|| sscanf (h, "x = %d, y = %d", &x, &y) == 2)
		{
			pattern->header.cols = x;
			pattern->header.rows = y;
			pattern->header.rule = strlen (rule) ? xstrdup (rule) : NULL;
			return 1;
		}
	else
		{
			error (0, 0, "Invalid header '%s'", h);
			return 0;
		}
}

static int
pattern_rle_parse (Pattern *pattern, const char *rle,
		int *rows, int *cols)
{
	const char *p = NULL;
	int val = 0;

	int row = 0, col = 0;
	int max_col = 0;
	int count = 1;
	int rc = 1, break_all = 0;

	for (int token = pattern_rle_lex (rle, &p, &val); token;
			token = pattern_rle_lex (NULL, &p, &val))
		{
			switch (token)
				{
					case ALIVE: case DEAD:
						{
							int old_col = col;
							col += count;

							assert (pattern == NULL || pattern->grid->cols >= col);

							if (pattern != NULL)
								for (int j = old_col; j < col; j++)
									GRID_SET (pattern->grid, row, j, token - DEAD);

							count = 1;
							break;
						}
					case COUNT:
						{
							count = val;
							break;
						}
					case NEWROW:
						{
							row += count;

							assert (pattern == NULL || pattern->grid->rows >= row);

							if (max_col < col)
								max_col = col;

							col = 0;
							count = 1;
							break;
						}
					case END:
						{
							if (count > 1)
								{
									error (0, 0, "Count '%d' misplaced", count);
									rc = 0;
								}

							break_all = 1;
							break;
						}
					case MISTERY:
						{
							error (0, 0, "What is that?");
							rc = 0;
							break_all = 1;
							break;
						}
				}

			if (break_all)
				break;
		}

	if (rows != NULL)
		*rows = row + 1;

	if (cols != NULL)
		*cols = max_col < col
			? col
			: max_col;

	return rc;
}

static int
pattern_get_header_and_body (char *buf, const char **h, const char **b)
{
	char *p = NULL;
	char *s = NULL;

	s = buf;

	while ((p = strchr (s, '\n')) != NULL)
		{
			*p = '\0';

			chomp (s);
			trim (s);

			if (*s != '\0' && *s != '#')
				break;

			s = p + 1;
		}

	// Empty or truncated file
	if (p == NULL || *(p + 1) == '\0')
		return 0;

	*h = s;
	*b = (p + 1);

	return 1;
}

static void
pattern_parse_from_file (Pattern *pattern, const char *filename)
{
	const char *h = NULL, *b = NULL;
	char *buf = NULL;
	int rows = 0, cols = 0;

	buf = file_slurp (filename);

	// Get header
	if (!pattern_get_header_and_body (buf, &h, &b))
		error (1, 0, "Empty or truncated file '%s'", filename);

	// Parse header
	if (!pattern_rle_header_parse (pattern, h))
		error (1, 0, "Failed to parse header from '%s'", filename);

	// Parse rule
	if (pattern->header.rule != NULL && !rule_is_valid (pattern->header.rule))
		error (1, 0, "Invalid rule or alias '%s'", pattern->header.rule);

	// Parse and get grid dimensions
	if (!pattern_rle_parse (NULL, b, &rows, &cols))
		error (1, 0, "Failed to parse RLE string from '%s'", filename);

	if (rows < pattern->header.rows)
		rows = pattern->header.rows;

	if (cols < pattern->header.cols)
		cols = pattern->header.cols;

	pattern->grid = grid_new (rows, cols);

	// Set the grid with the pattern
	assert (pattern_rle_parse (pattern, b, NULL, NULL));

	xfree (buf);
}

static void
pattern_parse_from_def (Pattern *pattern, const PatternDef *def)
{
	pattern->header.rows = def->header.rows,
	pattern->header.cols = def->header.cols,
	pattern->header.rule = xstrdup (def->header.rule);

	pattern->grid = grid_new (def->header.rows, def->header.cols);

	assert (pattern_rle_parse (pattern, def->rle, NULL, NULL));
}

Pattern *
pattern_new (const char *pattern_str)
{
	assert (pattern_str != NULL);

	Pattern *pattern = NULL;
	const PatternDef *def = NULL;

	pattern = xcalloc (1, sizeof (Pattern));

	if ((def = pattern_get_def_from_alias (pattern_str)) == NULL)
		pattern_parse_from_file (pattern, pattern_str);
	else
		pattern_parse_from_def (pattern, def);

	return pattern;
}

void
pattern_free (Pattern *pattern)
{
	if (pattern == NULL)
		return;

	xfree ((char *) pattern->header.rule);
	grid_free (pattern->grid);

	xfree (pattern);
}
