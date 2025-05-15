#pragma once

typedef struct _Rule Rule;

typedef struct
{
	const char *name;
	const char *rule;
} RuleAlias;

extern const RuleAlias rule_aliases[];

Rule * rule_new        (const char *str);
int    rule_next_state (Rule *rule, int state, int neighbors);
int    rule_is_valid   (const char *str);
void   rule_free       (Rule *rule);
