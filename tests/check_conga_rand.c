#include "check_conga.h"

#include <signal.h>

#include "../src/rand.h"

#define SEED 17

static Rand *rng = NULL;

static void
setup (void)
{
	rng = rand_new (SEED);
}

static void
teardown (void)
{
	rand_free (rng);
	rng = NULL;
}

START_TEST (test_rand_uniform)
{
	double x = rand_uniform (rng);
	ck_assert (x >= 0.0 && x < 5.0);
}
END_TEST

START_TEST (test_rand_int)
{
	int x = RAND_INT (rng, _i);
	ck_assert_int_lt (x, _i);
}
END_TEST

START_TEST (test_reproducibility)
{
	Rand *rng2 = rand_new (SEED);

	for (int i = 0; i < 5; i++)
		ck_assert_double_eq (rand_uniform (rng),
				rand_uniform (rng2));

	rand_free (rng2);
}
END_TEST

START_TEST (test_rand_uniform_fatal)
{
	rand_uniform (NULL);
}
END_TEST

Suite *
make_rand_suite (void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_abort;

	s = suite_create ("Rand");

	/* Core test case */
	tc_core = tcase_create ("Core");

	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_loop_test (tc_core, test_rand_uniform, 1, 10);
	tcase_add_loop_test (tc_core, test_rand_int, 1, 10);
	tcase_add_test (tc_core, test_reproducibility);

	/* Abort test case */
	tc_abort = tcase_create ("Abort");

	tcase_add_test_raise_signal (tc_abort,
			test_rand_uniform_fatal, SIGABRT);

	suite_add_tcase (s, tc_core);
	suite_add_tcase (s, tc_abort);

	return s;
}
