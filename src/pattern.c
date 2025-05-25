#include "pattern.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <assert.h>
#include "wrapper.h"
#include "utils.h"
#include "rule.h"

typedef union
{
	int  num;
	char str[BUFSIZ];
} PatternHeaderVal;

enum PatternHeaderTokenType
{
	HROWS    = 258,
	HCOLS    = 259,
	HRULE    = 260,
	HEQUAL   = 261,
	HNUMBER  = 262,
	HSTRING  = 263,
	HSEP     = 264,
	HEOL     = 265,
	HMISTERY = 266
};

#define PATTERN_HEADER_NUM_OP 3

static const char *const pattern_header_sym[] =
{
	"y", "x", "rule", "=", NULL, NULL, ",", "\n", NULL
};

#define PATTERN_HEADER_TOKEN_SYM(t) \
	(pattern_header_sym[(t) - HROWS])

enum PatternHeaderParseState
{
	WAITING_KEY    = 258,
	WAITING_EQUAL  = 259,
	WAITING_VALUE  = 260,
	WAITING_SEP    = 261
};

static size_t
pattern_getline (FILE *fp, char **line)
{
	size_t len = 0;
	size_t nread = 0;

	while (!feof (fp))
		{
			nread = getline (line, &len, fp);

			// End of file or error
			if (nread == EOF)
				{
					if (errno)
						error (1, 1, "Failed to getline");
					break;
				}

			chomp (*line);
			trim (*line);

			// Line is empty or a comment
			if (**line == '\0' || **line == '#')
				continue;

			return 1;
		}

	return 0;
}

static const int
pattern_header_op_lookup (const char *key)
{
	for (int i = 0; i < PATTERN_HEADER_NUM_OP; i++)
		{
			if (!strncasecmp (key, pattern_header_sym[i],
						strlen (pattern_header_sym[i])))
				return HROWS + i;
		}

	return 0;
}

static int
pattern_header_lex (const char *header, const char **pp,
		PatternHeaderVal *val)
{
	if (header != NULL)
		*pp = header;

	for (char c = *(*pp)++; c != '\0'; c = *(*pp)++)
		{
			if (isdigit (c))
				{
					int num = 0;

					do
					{
						num = 10 * num + c - '0';
					}
					while (isdigit (c = *(*pp)++));

					if (val != NULL)
						val->num = num;

					// ungetc
					(*pp)--;

					return HNUMBER;
				}
			else if (isalnum (c) || c == '/')
				{
					char key[BUFSIZ] = {0};
					int i = 0, token = 0;

					do
					{
						key[i++] = c;
						c = *(*pp)++;
					}
					while (i < BUFSIZ && (isalnum (c) || c == '/'));

					key[BUFSIZ - 1] = '\0';

					// ungetc
					(*pp)--;

					if ((token = pattern_header_op_lookup (key)))
						return token;

					if (val != NULL)
						strncpy (val->str, key, BUFSIZ);

					val->str[BUFSIZ - 1] = '\0';

					return HSTRING;
				}

			switch (c)
				{
				case '='  : return HEQUAL;
				case ','  : return HSEP;
				case ' '  :
				case '\t' : break;
				case '\n' : return HEOL;
				default :
					{
						error (0, 0, "Mistery character '%c'", c);
						return HMISTERY;
					}
				}
		}

	return 0;
}

static int
pattern_header_parse (Pattern *pattern, const char *header)
{
	int mode = WAITING_KEY;
	int seeneol = 0;

	const char *p = NULL;
	PatternHeaderVal val = {0};

	int check[PATTERN_HEADER_NUM_OP] = {0};
	int token_op = 0;

	for (int token = pattern_header_lex (header, &p, &val); token;
			token = pattern_header_lex (NULL, &p, &val))
		{
			if (seeneol)
				break;

			switch (token)
				{
				case HROWS: case HCOLS: case HRULE:
					{
						switch (mode)
							{
							case WAITING_EQUAL: case WAITING_VALUE:
								{
									error (0, 0, "'%s' operator has no value",
											PATTERN_HEADER_TOKEN_SYM (token_op));
									return 0;
								}
							case WAITING_SEP:
								{
									error (0, 0, "Missing '%s' separator before '%s' operator",
											PATTERN_HEADER_TOKEN_SYM (HSEP),
											PATTERN_HEADER_TOKEN_SYM (token));
									return 0;
								}
							}

						if (check[token - HROWS])
							{
								error (0, 0, "'%s' operator is repeated",
										PATTERN_HEADER_TOKEN_SYM (token));
								return 0;
							}

						check[token - HROWS] = 1;
						token_op = token;
						mode = WAITING_EQUAL;

						break;
					}
				case HEQUAL:
					{
						switch (mode)
							{
							case WAITING_KEY:
								{
									error (0, 0, "Missing key operator for '%s'",
											PATTERN_HEADER_TOKEN_SYM (HEQUAL));
									return 0;
								}
							case WAITING_VALUE:
								{
									error (0, 0, "'%s' is repeated",
											PATTERN_HEADER_TOKEN_SYM (HEQUAL));
									return 0;
								}
							case WAITING_SEP:
								{
									error (0, 0, "'%s' is misplaced",
											PATTERN_HEADER_TOKEN_SYM (HEQUAL));
									return 0;
								}
							}

						mode = WAITING_VALUE;

						break;
					}
				case HNUMBER: case HSTRING:
					{
						switch (mode)
							{
							case WAITING_KEY: case WAITING_SEP:
								{
									if (token == HNUMBER)
										error (0, 0, "Number '%d' with no operator",
												val.num);
									else if (token == HSTRING)
										error (0, 0, "String '%s' with no operator",
												val.str);
									return 0;
								}
							case WAITING_EQUAL:
								{
									error (0, 0, "Missing '%s': Format key %s value",
											PATTERN_HEADER_TOKEN_SYM (HEQUAL),
											PATTERN_HEADER_TOKEN_SYM (HEQUAL));
									return 0;
								}
							}

						if (token == HNUMBER && token_op == HRULE)
							{
								error (0, 0, "'%s' requires a string",
										PATTERN_HEADER_TOKEN_SYM (token_op));
								return 0;
							}
						else if (token == HSTRING && (token_op == HROWS
									|| token_op == HCOLS))
							{
								error (0, 0, "'%s' requires a number",
										PATTERN_HEADER_TOKEN_SYM (token_op));
								return 0;
							}

						switch (token_op)
							{
							case HROWS: pattern->rows = val.num;           break;
							case HCOLS: pattern->cols = val.num;           break;
							case HRULE: pattern->rule = xstrdup (val.str); break;
							}

						mode = WAITING_SEP;

						break;
					}
				case HSEP:
					{
						switch (mode)
							{
							case WAITING_KEY: case WAITING_EQUAL:
								{
									error (0, 0, "'%s' separator is misplaced",
											PATTERN_HEADER_TOKEN_SYM (HSEP));
									return 0;
								}
							case WAITING_VALUE:
								{
									error (0, 0, "Missing value for '%s' operator",
											PATTERN_HEADER_TOKEN_SYM (token_op));
									return 0;
								}
							}

						mode = WAITING_KEY;

						break;
					}
				case HEOL:
					{
						seeneol = 1;
						break;
					}
				case HMISTERY:
					{
						error (0, 0, "What is that?");
						return 0;
					}
				}
		}

	if (mode != WAITING_SEP)
		{
			error (0, 0, "Premature stop");
			return 0;
		}

	int missing = 0;

	for (int i = 0; i < PATTERN_HEADER_NUM_OP; i++)
		{
			if (!check[i] && (i + HROWS) != HRULE)
				{
					error (0, 0, "Missing '%s' operator",
							pattern_header_sym[i]);
					missing = 1;
				}
		}

	return !missing;
}

enum PatternRLETokenType
{
	RDEAD    = 258,
	RALIVE   = 259,
	RCOUNT   = 260,
	RNEWROW  = 261,
	REND     = 262,
	RMISTERY = 263
};

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

					return RCOUNT;
				}

			switch (c)
				{
				case 'b' : case 'B':  return RDEAD;
				case 'o' : case 'O':  return RALIVE;
				case '$' :            return RNEWROW;
				case '!' :            return REND;
				case ' ' : case '\t': break;
				default  :
					{
						error (0, 0, "Mistery character '%c'", c);
						return RMISTERY;
					}
				}
		}

	return 0;
}

static int
pattern_rle_parse (Pattern *pattern, FILE *fp,
		int *rows, int *cols)
{
	const char *p = NULL;
	char *rle = NULL;
	int val = 0;

	int row = 0, col = 0;
	int max_col = 0;
	int count = 1;
	int rc = 1;

OUTER: while (pattern_getline (fp, &rle))
		{
			for (int token = pattern_rle_lex (rle, &p, &val); token;
					token = pattern_rle_lex (NULL, &p, &val))
				{
					switch (token)
						{
							case RALIVE: case RDEAD:
								{
									int old_col = col;
									col += count;

									assert (pattern == NULL || pattern->grid->cols >= col);

									if (pattern != NULL)
										for (int j = old_col; j < col; j++)
											GRID_SET (pattern->grid, row, j, token - RDEAD);

									count = 1;
									break;
								}
							case RCOUNT:
								{
									count = val;
									break;
								}
							case RNEWROW:
								{
									row += count;

									assert (pattern == NULL || pattern->grid->rows >= row);

									if (max_col < col)
										max_col = col;

									col = 0;
									count = 1;
									break;
								}
							case REND:
								{
									if (count > 1)
										{
											error (0, 0, "Count '%d' misplaced", count);
											rc = 0;
										}

									break OUTER;
								}
							case RMISTERY:
								{
									error (0, 0, "What is that?");
									rc = 0; break OUTER;
								}
						}
				}
		}

	if (rows != NULL)
		*rows = row + 1;

	if (cols != NULL)
		*cols = max_col < col
			? col
			: max_col;

	xfree (rle);
	return rc;
}

Pattern *
pattern_new (const char *filename)
{
	assert (filename != NULL);

	Pattern *pattern = NULL;
	FILE *fp = NULL;
	char *line = NULL;
	int rows = 0, cols = 0;
	off_t beacon = 0;

	pattern = xcalloc (1, sizeof (Pattern));
	fp = xfopen (filename, "r");

	// Get header
	if (!pattern_getline (fp, &line))
		error (1, 0, "Failed to read header from '%s'", filename);

	// Set body starting point
	beacon = ftell (fp);

	if (!pattern_header_parse (pattern, line))
		error (1, 0, "Failed to parse header from '%s'", filename);

	if (pattern->rule != NULL && !rule_is_valid (pattern->rule))
		error (1, 0, "Invalid rule or alias '%s'", pattern->rule);

	// Parse and get grid dimensions
	if (!pattern_rle_parse (NULL, fp, &rows, &cols))
		error (1, 0, "Failed to parse RLE string from '%s'", filename);

	if (rows < pattern->rows)
		rows = pattern->rows;

	if (cols < pattern->cols)
		cols = pattern->cols;

	pattern->grid = grid_new (rows, cols);

	// Rewind body
	xfseek (fp, beacon, SEEK_SET);

	// Set the grid with the pattern
	assert (pattern_rle_parse (pattern, fp, NULL, NULL));

	xfree (line);
	xfclose (fp);

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
