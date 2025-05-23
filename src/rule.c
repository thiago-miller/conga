#include "rule.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <error.h>
#include "wrapper.h"

#define STATES    2
#define NEIGHBORS 9

typedef int RuleTable[STATES][NEIGHBORS];

struct _Rule
{
	RuleTable table;
};

enum RuleTokenType
{
	SEP       = 258,
	BORN      = 259,
	SURVIVE   = 260,
	NUMBER    = 261,
	MISTERY   = 262
};

enum RuleParseState
{
	WAITING_OP      = 258,
	READING_NUMBERS = 259
};

const RuleAlias rule_aliases[] =
{
	{"2x2",                "B36/S125"      },
	{"anneal",             "B4678/S35678"  },
	{"conway",             "B3/S23"        },
	{"day_and_night",      "B3678/S34678"  },
	{"diamoeba",           "B35678/S5678"  },
	{"highlife",           "B36/S23"       },
	{"life",               "B3/S23"        },
	{"life34",             "B34/S34"       },
	{"life_without_death", "B3/S012345678" },
	{"maze",               "B3/S12345"     },
	{"mazectric",          "B3/S1234"      },
	{"morley",             "B368/S245"     },
	{"photon",             "B25/S4"        },
	{"replicator",         "B1357/S1357"   },
	{"seeds",              "B2/S"          },
	{NULL,                 NULL            }
};

static const char *
rule_get_rule_from_alias (const char *alias)
{
	const char *rule = NULL;

	for (const RuleAlias *a = rule_aliases; a->name != NULL; a++)
		{
			if (strcasecmp (alias, a->name) == 0)
				{
					rule = a->rule;
					break;
				}
		}

	return rule;
}

static int
rule_lex (const char *str, const char **pp, int *val)
{
	if (str != NULL)
		*pp = str;

	if (**pp == '\0')
		return 0;

	char c = *(*pp)++;

	if (isdigit (c))
		{
			if (val != NULL)
				*val = c - '0';
			return NUMBER;
		}

	switch (c)
		{
		case 'b': case 'B': return BORN;
		case 's': case 'S': return SURVIVE;
		case '/': return SEP;
		default :
			{
				error (0, 0, "Mistery character '%c'", c);
				return MISTERY;
			}
		}
}

static int
rule_parse_into_table (RuleTable table, const char *str)
{
	// RuleParserState
	int mode = WAITING_OP;

	// B/S index
	int state = 0;

	// avoid B/S and / duplication and ensure
	// that they will be counted
	int check[STATES + 1] = {0};
	char op[STATES] = {'B', 'S'};

	const char *p = NULL;
	int val = 0;

	if (table != NULL)
		memset (table, 0, sizeof (RuleTable));

	for (int token = rule_lex (str, &p, &val); token;
			token = rule_lex (NULL, &p, &val))
		{
			switch (token)
				{
				case BORN:
				case SURVIVE:
					{
						state = token - BORN;

						if (check[state])
							{
								error (0, 0, "'%c' operator is repeated",
										op[state]);
								return 0;
							}

						if (mode != WAITING_OP)
							{
								error (0, 0, "'%c' operator does not appear after the / separator",
										op[state]);
								return 0;
							}

						mode = READING_NUMBERS;
						check[state] = 1;

						break;
					}
				case SEP:
					{
						if (check[2])
							{
								error (0, 0, "'/' separator is repeated");
								return 0;
							}

						if (mode == WAITING_OP)
							{
								error (0, 0, "'/' separator appears at the beginning");
								return 0;
							}

						mode = WAITING_OP;
						check[2] = 1;

						break;
					}
				case NUMBER:
					{
						if (mode != READING_NUMBERS)
							{
								error (0, 0, "Number without operator 'B' or 'S'");
								return 0;
							}

						if (!(val >= 0 && val < NEIGHBORS))
							{
								error (0, 0, "Number '%d' not at range [0, %d)",
										val, NEIGHBORS);
								return 0;
							}

						if (table != NULL)
							table[state][val] = 1;

						break;
					}
				case MISTERY:
					{
						error (0, 0, "What is that?");
						return 0;
					}
				}
		}

	for (int i = 0; i < STATES; i++)
		if (!check[i])
			{
				error (0, 0, "Missing operator '%c'", op[i]);
				return 0;
			}

	return 1;
}

Rule *
rule_new (const char *str)
{
	assert (str != NULL);

	Rule *rule = xcalloc (1, sizeof (Rule));

	const char *rule_str = NULL;
	if ((rule_str = rule_get_rule_from_alias (str)) == NULL)
		rule_str = str;

	int rc = rule_parse_into_table (rule->table, rule_str);

	assert (rc);

	return rule;
}

void
rule_free (Rule *rule)
{
	xfree (rule);
}

int
rule_is_valid (const char *str)
{
	assert (str != NULL);

	return rule_get_rule_from_alias (str) != NULL
		|| rule_parse_into_table (NULL, str);
}

int
rule_next_state (Rule *rule, int state, int neighbors)
{
	assert (rule != NULL);
	assert (state >= 0 && state < STATES);
	assert (neighbors >= 0 && neighbors < NEIGHBORS);

	return rule->table[state][neighbors];
}
