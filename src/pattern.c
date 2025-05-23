#include "pattern.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <error.h>
#include "wrapper.h"

typedef union
{
	int  num;
	char str[BUFSIZ];
} PatternHeaderVal;

enum PatternHeaderTokenType
{
	ROWS    = 258,
	COLS    = 259,
	RULE    = 260,
	EQUAL   = 261,
	NUMBER  = 262,
	STRING  = 263,
	SEP     = 264,
	EOL     = 265,
	MISTERY = 266
};

#define PATTERN_HEADER_NUM_OP 3

static const char *const pattern_header_sym[] =
{
	"y", "x", "rule", "=", NULL, NULL, ",", "\n", NULL
};

#define PATTERN_HEADER_TOKEN_SYM(t) \
	(pattern_header_sym[(t) - ROWS])

enum PatternHeaderParseState
{
	WAITING_KEY    = 258,
	WAITING_EQUAL  = 259,
	WAITING_VALUE  = 260,
	WAITING_SEP    = 261
};

static const int
pattern_header_op_lookup (const char *key)
{
	for (int i = 0; i < PATTERN_HEADER_NUM_OP; i++)
		{
			if (!strncasecmp (key, pattern_header_sym[i],
						strlen (pattern_header_sym[i])))
				return ROWS + i;
		}

	return 0;
}

static int
pattern_header_lex (const char *header, const char **pp,
		PatternHeaderVal *val)
{
	if (header != NULL)
		*pp = header;

	if (**pp == '\0')
		return 0;

	while (1)
		{
			char c = *(*pp)++;

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

					return NUMBER;
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

					return STRING;
				}

			switch (c)
				{
				case '='  : return EQUAL;
				case ','  : return SEP;
				case ' '  :
				case '\t' : break;
				case '\n' : return EOL;
				default :
					{
						error (0, 0, "Mistery character '%c'", c);
						return MISTERY;
					}
				}
		}
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
				case ROWS : case COLS : case RULE :
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
											PATTERN_HEADER_TOKEN_SYM (SEP),
											PATTERN_HEADER_TOKEN_SYM (token));
									return 0;
								}
							}

						if (check[token - ROWS])
							{
								error (0, 0, "'%s' operator is repeated",
										PATTERN_HEADER_TOKEN_SYM (token));
								return 0;
							}

						check[token - ROWS] = 1;
						token_op = token;
						mode = WAITING_EQUAL;

						break;
					}
				case EQUAL :
					{
						switch (mode)
							{
							case WAITING_KEY:
								{
									error (0, 0, "Missing key operator for '%s'",
											PATTERN_HEADER_TOKEN_SYM (EQUAL));
									return 0;
								}
							case WAITING_VALUE:
								{
									error (0, 0, "'%s' is repeated",
											PATTERN_HEADER_TOKEN_SYM (EQUAL));
									return 0;
								}
							case WAITING_SEP:
								{
									error (0, 0, "'%s' is misplaced",
											PATTERN_HEADER_TOKEN_SYM (EQUAL));
									return 0;
								}
							}

						mode = WAITING_VALUE;

						break;
					}
				case NUMBER : case STRING :
					{
						switch (mode)
							{
							case WAITING_KEY: case WAITING_SEP:
								{
									if (token == NUMBER)
										error (0, 0, "Number '%d' with no operator",
												val.num);
									else if (token == STRING)
										error (0, 0, "String '%s' with no operator",
												val.str);
									return 0;
								}
							case WAITING_EQUAL:
								{
									error (0, 0, "Missing '%s': Format key %s value",
											PATTERN_HEADER_TOKEN_SYM (EQUAL),
											PATTERN_HEADER_TOKEN_SYM (EQUAL));
									return 0;
								}
							}

						if (token == NUMBER && token_op == RULE)
							{
								error (0, 0, "'%s' requires a string",
										PATTERN_HEADER_TOKEN_SYM (token_op));
								return 0;
							}
						else if (token == STRING && (token_op == ROWS
									|| token_op == COLS))
							{
								error (0, 0, "'%s' requires a number",
										PATTERN_HEADER_TOKEN_SYM (token_op));
								return 0;
							}

						switch (token_op)
							{
							case ROWS : pattern->rows = val.num;           break;
							case COLS : pattern->cols = val.num;           break;
							case RULE : pattern->rule = xstrdup (val.str); break;
							}

						mode = WAITING_SEP;

						break;
					}
				case SEP :
					{
						switch (mode)
							{
							case WAITING_KEY : case WAITING_EQUAL :
								{
									error (0, 0, "'%s' separator is misplaced",
											PATTERN_HEADER_TOKEN_SYM (SEP));
									return 0;
								}
							case WAITING_VALUE :
								{
									error (0, 0, "Missing value for '%s' operator",
											PATTERN_HEADER_TOKEN_SYM (token_op));
									return 0;
								}
							}

						mode = WAITING_KEY;

						break;
					}
				case EOL :
					{
						seeneol = 1;
						break;
					}
				case MISTERY :
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
			if (!check[i] && (i + ROWS) != RULE)
				{
					error (0, 0, "Missing '%s' operator",
							pattern_header_sym[i]);
					missing = 1;
				}
		}

	return !missing;
}

Pattern *
pattern_new (const char *filename)
{
	Pattern *pattern = xcalloc (1, sizeof (Pattern));
	return pattern;
}

void
pattern_free (Pattern *pattern)
{
	if (pattern == NULL)
		return;

	xfree (pattern->rule);

	xfree (pattern);
}
