#include "pattern.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <error.h>
#include "wrapper.h"

#define STRSIZ 256

typedef union
{
	int  num;
	char str[STRSIZ];
} PatternVal;

enum PatternTokenType
{
	ROWS    = 258,
	COLUNS  = 259,
	RULE    = 260,
	EQUAL   = 261,
	NUMBER  = 262,
	STRING  = 263,
	SEP     = 264,
	EOL     = 265,
	MISTERY = 266
};

static const char *const pattern_header_sym[] =
{
	"y", "x", "rule"
};

static const int
pattern_header_sym_lookup (const char *key)
{
	for (int i = 0; i < sizeof (pattern_header_sym) / sizeof (char *); i++)
		{
			if (!strncasecmp (key, pattern_header_sym[i],
						strlen (pattern_header_sym[i])))
				return ROWS + i;
		}

	return 0;
}

static int
pattern_header_lex (const char *header, const char **pp,
		PatternVal *val)
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
					char key[STRSIZ] = {0};
					int i = 0, token = 0;

					do
					{
						key[i++] = c;
						c = *(*pp)++;
					}
					while (i < STRSIZ && (isalnum (c) || c == '/'));

					key[STRSIZ - 1] = '\0';

					// ungetc
					(*pp)--;

					if ((token = pattern_header_sym_lookup (key)))
						return token;

					if (val != NULL)
						strncpy (val->str, key, STRSIZ);

					val->str[STRSIZ - 1] = '\0';

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

	xfree (pattern);
}
