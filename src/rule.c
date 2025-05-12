#include "rule.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "wrapper.h"

#define STATES    2
#define NEIGHBORS 9

typedef int RuleTable[STATES][NEIGHBORS];

struct _Rule
{
	RuleTable table;
};

const RuleAlias rule_aliases[] =
{
	{"anneal",             "B4678/S35678"  },
	{"conway",             "B3/S23"        },
	{"day_and_night",      "B3678/S34678"  },
	{"highlife",           "B36/S23"       },
	{"life34",             "B34/S34"       },
	{"life_without_death", "B3/S012345678" },
	{"maze",               "B3/S12345"     },
	{"mazectric",          "B3/S1234"      },
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
rule_parse_into_table (RuleTable table, const char *str)
{
	if (str == NULL)
		return 0;

	const char *rule = NULL;

	if ((rule = rule_get_rule_from_alias (str)) == NULL)
		rule = str;

	memset (table, 0, sizeof (RuleTable));

	// -1 (init or /), 0 = B (born), 1 = S (survive)
	int mode = -1;

	// avoid B/S duplication and ensure that they will be counted
	int check[STATES] = {0};

	char c = 0;
	int  n = 0;

	for (const char *p = rule; *p != '\0'; p++)
		{
			c = tolower (*p);

			switch (c)
				{
				case 'b' :
					{
						if (mode != -1 || check[0])
							return 0;
						mode = 0;
						check[0] = 1;
						break;
					}
				case 's' :
					{
						if (mode != -1 || check[1])
							return 0;
						mode = 1;
						check[1] = 1;
						break;
					}
				case '/' :
					{
						if (mode == -1)
							return 0;
						mode = -1;
						break;
					}
				default  :
					{
						// number without B/S before
						if (mode == -1)
							return 0;

						if (!isdigit (c))
							return 0;

						n = c - '0';

						if (!(n >= 0 && n < NEIGHBORS))
							return 0;

						table[mode][n] = 1;
					}
				}
		}

	for (int i = 0; i < STATES; i++)
		if (!check[i])
			return 0;

	return 1;
}

Rule *
rule_new (const char *str)
{
	assert (str != NULL);

	Rule *r = xcalloc (1, sizeof (Rule));
	int rc = rule_parse_into_table (r->table, str);

	assert (rc);

	return r;
}

void
rule_free (Rule *r)
{
	xfree (r);
}

int
rule_is_valid (const char *str)
{
	RuleTable dummy;
	return rule_parse_into_table (dummy, str);
}

int
rule_next_state (Rule *r, int state, int neighbors)
{
	assert (r != NULL);
	assert (state >= 0 && state < STATES);
	assert (neighbors >= 0 && neighbors < NEIGHBORS);

	return r->table[state][neighbors];
}
