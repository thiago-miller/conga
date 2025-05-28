#include "pattern.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <error.h>
#include <assert.h>
#include "wrapper.h"
#include "utils.h"
#include "rule.h"

enum PatternTokenType
{
	DEAD    = 264,
	ALIVE   = 265,
	NEWROW  = 266,
	END     = 267,
	COUNT   = 270,
	MISTERY = 271
};

{


				{
					break;
				}
		}

}

static int
pattern_header_parse (Pattern *pattern, const char *header)
{
	int x = 0, y = 0;
	char rule[64] = {0};

	if (sscanf (header, "x = %d, y = %d, rule = %63s", &x, &y, rule) == 3
			|| sscanf (header, "x = %d, y = %d", &x, &y) == 2)
		{
			pattern->cols = x;
			pattern->rows = y;
			pattern->rule = strlen (rule) ? xstrdup (rule) : NULL;
			return 1;
		}
	else
		{
			error (0, 0, "Invalid header '%s'", header);
			return 0;
		}
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
pattern_rle_parse (Pattern *pattern, const char *rle,
		int *rows, int *cols)
{
	const char *p = NULL;
	int val = 0;

	int row = 0, col = 0;
	int max_col = 0;
	int count = 1;
	int rc = 1;

OUTER: for (int token = pattern_rle_lex (rle, &p, &val); token;
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

							break OUTER;
						}
					case MISTERY:
						{
							error (0, 0, "What is that?");
							rc = 0; break OUTER;
						}
				}
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

Pattern *
pattern_new (const char *filename)
{
	assert (filename != NULL);

	Pattern *pattern = NULL;
	const char *header = NULL, *body = NULL;
	char *buf = NULL;
	int rows = 0, cols = 0;

	pattern = xcalloc (1, sizeof (Pattern));
	buf = file_slurp (filename);

	// Get header
	if (!pattern_get_header_and_body (buf, &header, &body))
		error (1, 0, "Empty or truncated file '%s'", filename);

	// Parse header
	if (!pattern_header_parse (pattern, header))
		error (1, 0, "Failed to parse header from '%s'", filename);

	// Parse rule
	if (pattern->rule != NULL && !rule_is_valid (pattern->rule))
		error (1, 0, "Invalid rule or alias '%s'", pattern->rule);

	// Parse and get grid dimensions
	if (!pattern_rle_parse (NULL, body, &rows, &cols))
		error (1, 0, "Failed to parse RLE string from '%s'", filename);

	if (rows < pattern->rows)
		rows = pattern->rows;

	if (cols < pattern->cols)
		cols = pattern->cols;

	pattern->grid = grid_new (rows, cols);

	// Set the grid with the pattern
	assert (pattern_rle_parse (pattern, body, NULL, NULL));

	xfree (buf);

	return pattern;
}

void
pattern_free (Pattern *pattern)
{
	if (pattern == NULL)
		return;

	grid_free (pattern->grid);
	xfree (pattern->rule);

	xfree (pattern);
}
