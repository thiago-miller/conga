#include "check_conga.h"

#include "../src/wrapper.h"
#include "../src/pattern.c"

#define HEADER_SIZE(h) sizeof (h)/ sizeof (char *)

static const char *header[] =
{
	"x = 10, y = 20, rule = B3/S23",
	"x=30,y=10,rule=S12345/B0123",
	"x   = 5,   y =   5, rule  =  B3/S23",
	"x = 10, y = 20"
};

static union
{
	int  num;
	char str[128];
}
val[][3] =
{
	{{.num = 10}, {.num = 20},  {.str = "B3/S23"}},
	{{.num = 30}, {.num = 10},  {.str = "S12345/B0123"}},
	{{.num = 5},  {.num = 5},   {.str = "B3/S23"}},
	{{.num = 10}, {.num = 20},  {}}
};

static const char *header_fail[] =
{
	"x 10, y = 20, rule = B3/S23",
	"x = 10 y = 20, rule = B3/S23",
	"x = 10, y 20, rule = B3/S23",
	" = 10, y = 20, rule = B3/S23",
	"x = 10,  = 20, rule = B3/S23",
	"x = , y = 20, rule = B3/S23",
	"x = 10, y = , rule = B3/S23",
	", y = 20, rule = B3/S23",
	"x = 10, , rule = B3/S23",
	"y = 20, rule = B3/S23",
	"x = 10, rule = B3/S23"
};

static Pattern *pattern = NULL;

static void
setup (void)
{
	pattern = xcalloc (1, sizeof (Pattern));
}

static void
teardown (void)
{
	pattern_free (pattern);
	pattern = NULL;
}

START_TEST (test_pattern_header_parse)
{
	ck_assert (pattern_header_parse (pattern, header[_i]));

	ck_assert_int_eq (pattern->cols, val[_i][0].num);
	ck_assert_int_eq (pattern->rows, val[_i][1].num);

	if (pattern->rule != NULL)
		ck_assert_str_eq (pattern->rule, val[_i][2].str);
}
END_TEST

START_TEST (test_pattern_header_parse_fail)
{
	ck_assert (!pattern_header_parse (pattern, header_fail[_i]));
}
END_TEST

START_TEST (test_pattern_rle_parse)
{
	char rle[] = "b2o$2o$bo!";

	FILE *fp = fmemopen (rle, sizeof (rle), "r");

	int rows = 0, cols = 0;
	int rc = pattern_rle_parse (NULL, fp, &rows, &cols);

	printf (":: [%d] %d %d\n", rc, rows, cols);

	pattern->grid = grid_new (rows, cols);
	rewind (fp);

	rc = pattern_rle_parse (pattern, fp, NULL, NULL);

	for (int i = 0; i < pattern->grid->rows; i++)
		{
			printf ("%d", GRID_GET (pattern->grid, i, 0));

			for (int j = 1; j < pattern->grid->cols; j++)
				printf (" %d", GRID_GET (pattern->grid, i, j));

			printf ("\n");
		}

	fclose (fp);
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

	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_loop_test (tc_core, test_pattern_header_parse,
			0, HEADER_SIZE (header));
	tcase_add_loop_test (tc_core, test_pattern_header_parse_fail,
			0, HEADER_SIZE (header_fail));
	tcase_add_test (tc_core, test_pattern_rle_parse);

	suite_add_tcase (s, tc_core);

	return s;
}
