#include "check_conga.h"

#include <stdio.h>
#include "../src/rule.c"

START_TEST (test_rule_lex)
{
	const char str[] = "BS/01234X";
	const char *p = NULL;
	int val = 0;

	for (int token = rule_lex (str, &p, &val), i = 0; token;
			token = rule_lex (NULL, &p, &val), i++)
		{
			switch (token)
				{
				case BORN:    ck_assert_int_eq (str[i], 'B');       break;
				case SURVIVE: ck_assert_int_eq (str[i], 'S');       break;
				case SEP:     ck_assert_int_eq (str[i], '/');       break;
				case NUMBER:  ck_assert_int_eq (str[i] - '0', val); break;
				case MISTERY: ck_assert_int_eq (str[i], 'X');       break;
				default:      ck_abort ();
				}
		}
}
END_TEST

START_TEST (test_parse_into_table)
{
	RuleTable table;

	ck_assert (rule_parse_into_table (table, "B3/S23"));

	ck_assert (table[0][3]);   // B3
	ck_assert (table[1][2]);   // S2
	ck_assert (table[1][3]);   // S3

	ck_assert (rule_parse_into_table (table, "S012345678/B0"));

	for (int i = 0; i < 9; i++) {
		ck_assert (table[1][i]);  // S0-S8
	}

	ck_assert (table[0][0]); // B0
}
END_TEST

START_TEST (test_parse_into_table_fail)
{
	ck_assert (!rule_parse_into_table (NULL, "B9/S23"));
	ck_assert (!rule_parse_into_table (NULL, "B3//S23"));
	ck_assert (!rule_parse_into_table (NULL, "B3B4/S23"));
	ck_assert (!rule_parse_into_table (NULL, "3/S23"));
	ck_assert (!rule_parse_into_table (NULL, "/B3S23"));
	ck_assert (!rule_parse_into_table (NULL, "B3/"));
	ck_assert (!rule_parse_into_table (NULL, ""));
	ck_assert (!rule_parse_into_table (NULL, "B3#S23"));
}
END_TEST

START_TEST(test_rule_new_and_free)
{
	Rule *rule = rule_new ("B3/S23");
	ck_assert_ptr_nonnull (rule);

	ck_assert (rule->table[0][3]);  // B3
	ck_assert (rule->table[1][2]);  // S2
	ck_assert (rule->table[1][3]);  // S3

	rule_free (rule);
	rule_free (NULL);
}
END_TEST

START_TEST(test_rule_is_valid)
{
	ck_assert (rule_is_valid ("B3/S23"));
	ck_assert (rule_is_valid ("B/S"));
	ck_assert (!rule_is_valid ("B9/S23"));
	ck_assert (!rule_is_valid ("/B3S23"));
	ck_assert (!rule_is_valid ("B3#S23"));
}
END_TEST

START_TEST(test_rule_next_state)
{
	Rule *rule = rule_new ("B3/S23");

	ck_assert (rule_next_state (rule, 0, 3));  // B3
	ck_assert (!rule_next_state (rule, 0, 2));  // B2
	ck_assert (rule_next_state (rule, 1, 2));  // S2
	ck_assert (!rule_next_state (rule, 1, 4));  // S4

	rule_free (rule);
}
END_TEST

START_TEST (test_rule_get_rule_from_alias)
{
	for (const RuleAlias *a = rule_aliases; a->name != NULL; a++)
		ck_assert (rule_get_rule_from_alias (a->name));

	ck_assert (!rule_get_rule_from_alias ("ponga"));
}
END_TEST

Suite *
make_rule_suite (void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create ("Rule");

	/* Core test case */
	tc_core = tcase_create ("Core");

	tcase_add_test (tc_core, test_rule_lex);
	tcase_add_test (tc_core, test_parse_into_table);
	tcase_add_test (tc_core, test_parse_into_table_fail);
	tcase_add_test (tc_core, test_rule_new_and_free);
	tcase_add_test (tc_core, test_rule_is_valid);
	tcase_add_test (tc_core, test_rule_next_state);
	tcase_add_test (tc_core, test_rule_get_rule_from_alias);

	suite_add_tcase (s, tc_core);

	return s;
}
