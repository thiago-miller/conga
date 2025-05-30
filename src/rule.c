#include "rule.h"

#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <assert.h>
#include "error.h"
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
rule_parse_into_table (RuleTable table, const char *str)
{
#define NUM_PATTERN 3

	regex_t    preg[NUM_PATTERN];
	regmatch_t pmatch[NUM_PATTERN];
	regoff_t   off, len;

	char bs_values[STATES][NEIGHBORS + 1] = {0};

	char err[128] = {0};
	int i = 0, j = 0, i_match = 0;
	int rc = 0, rule_valid = 1;
	size_t neighbors = 0;

	char pattern[NUM_PATTERN][32] =
	{
		"^B([0-8]*)/S([0-8]*)$",
		"^([0-8]*)/([0-8]*)$",
		"^S([0-8]*)/B([0-8]*)$"
	};

	for (i = 0; i < NUM_PATTERN; i++)
		{
			rc = regcomp (&preg[i], pattern[i],
					REG_EXTENDED|REG_ICASE);
			if (rc != 0)
				{
					regerror (rc, &preg[i], err, 128);
					error (1, 0, "regcomp '%s' failed with '%s'",
							pattern[i], err);
				}
		}

	i_match = -1;

	for (i = 0; i < NUM_PATTERN; i++)
		{
			rc = regexec (&preg[i], str, NUM_PATTERN, pmatch, 0);
			if (rc == 0)
				{
					i_match = i;
					break;
				}
		}

	if (i_match == -1)
		{
			error (0, 0, "Invalid rule format '%s'", str);
			rule_valid = 0;
			goto __CLEAN;
		}

	for (i = 0; i < STATES; i++)
		{
			off = pmatch[i + 1].rm_so;
			len = pmatch[i + 1].rm_eo - pmatch[i + 1].rm_so;

			if (len > 9)
				{
					error (0, 0, "Invalid rule format '%s'", str);
					rule_valid = 0;
					goto __CLEAN;
				}

			// i_match == 2 | SyBx
			strncpy (bs_values[i_match == 2 ? !i : i],
					str + off, len);
		}

	if (table == NULL)
		goto __CLEAN;

	for (i = 0; i < STATES; i++)
		{
			neighbors = strlen (bs_values[i]);
			for (j = 0; j < neighbors; j++)
				table[i][bs_values[i][j] - '0'] = 1;
		}

__CLEAN:
	for (i = 0; i < NUM_PATTERN; i++)
		regfree (&preg[i]);

#undef NUM_PATTERN

	return rule_valid;
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
