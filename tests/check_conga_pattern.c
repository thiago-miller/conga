#include "check_conga.h"

#include "../src/pattern.c"

#include <stdio.h>

START_TEST (test_pattern_header_lex)
{
	PatternVal val = {0};

	const char header[] = "x = 10, Y = 20, ruLe = B3/S23\n+";
	const char *p = NULL;

	const char rule[] = "B3/S23";
	int num[] = {10, 20};
	int ni = 0;

	for (int token = pattern_header_lex (header, &p, &val), i = 0; token;
			token = pattern_header_lex (NULL, &p, &val))
		{
			switch (token)
				{
				case SEP:
					{
						ck_assert_int_eq (header[i], ',');
						i += 2;
						break;
					}
				case EOL:
					{
						i += 4;
						ck_assert_int_eq (header[i], '\n');
						i++;
						break;
					}
				case ROWS:
					{
						ck_assert_int_eq (header[i], 'Y');
						i += 2;
						break;
					}
				case COLUNS:
					{
						ck_assert_int_eq (header[i], 'x');
						i += 2;
						break;
					}
				case RULE:
					{
						i += 5;
						break;
					}
				case EQUAL:
					{
						ck_assert_int_eq (header[i], '=');
						i += 4;
						break;
					}
				case NUMBER:
					{
						ck_assert_int_eq (val.num, num[ni++]);
						break;
					}
				case STRING:
					{
						ck_assert_str_eq (val.str, rule);
						break;
					}
				case MISTERY:
					{
						ck_assert_int_eq (header[i], '+');
						break;
					}
				default:
					ck_abort ();
				}
		}
}
END_TEST

Suite *
make_pattern_suite (void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create ("Pattern");

	/* Core test case */
	tc_core = tcase_create ("Core");

	tcase_add_test (tc_core, test_pattern_header_lex);

	suite_add_tcase (s, tc_core);

	return s;
}
