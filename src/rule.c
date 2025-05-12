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
	{"anneal",             "B4678/S35678" },
	{"conway",             "B3S23"        },
	{"day_and_night",      "B3678S34678"  },
	{"highlife",           "B36S23"       },
	{"life34",             "B34S34"       },
	{"life_without_death", "B3S012345678" },
	{"maze",               "B3S12345"     },
	{"mazectric",          "B3S1234"      },
	{"replicator",         "B1357S1357"   },
	{"seeds",              "B2S"          },
	{NULL,                 NULL           }
};

static int
rule_parse_into_table (RuleTable table, const char *str)
{
	if (str == NULL)
		return 0;

	for (const RuleAlias *a = rule_aliases; a->name; ++a)
		{
			if (strcasecmp (str, a->name) == 0)
				{
					str = a->rule;
					break;
				}
		}

	memset (table, 0, sizeof (RuleTable));

	// 0 = B (born), 1 = S (survive)
	int mode = -1;
	int n = 0;
	char c = 0;

	for (const char *p = str; *p != '\0'; p++)
		{
			c = tolower (*p);

			if (c == 'b')
				mode = 0;
			else if (c == 's')
				mode = 1;
			else if (isdigit (c))
				{
					// number without B/S before
					if (mode == -1)
						return 0;

					n = c - '0';

					if (n < 0 || n > 8)
						return 0;

					table[mode][n] = 1;
				}
			// invalid char
			else
				return 0;
		}

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
