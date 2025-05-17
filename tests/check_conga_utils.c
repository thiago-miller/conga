#include "check_conga.h"

#include <signal.h>

#include "../src/utils.h"

#define SEED 17

START_TEST (test_shuffle)
{
	Rand *rng = rand_new (SEED);

	int n = 10;
	int vet[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int vet_cp[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int acm = 0;

	shuffle (vet, 1, rng);

	for (int i = 0; i < n; i++)
		ck_assert_int_eq (vet[i], vet_cp[i]);

	shuffle (vet, n, rng);

	for (int i = 0; i < n; i++)
		if (vet[i] == vet_cp[i])
			acm++;

	ck_assert_int_lt (acm, 6);

	rand_free (rng);
}
END_TEST

START_TEST (test_shuffle_fatal1)
{
	shuffle (NULL, 10, NULL);
}
END_TEST

START_TEST (test_shuffle_fatal2)
{
	int vet[] = {0};
	shuffle (vet, 10, NULL);
}
END_TEST

Suite *
make_utils_suite (void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_abort;

	s = suite_create ("Utils");

	/* Core test case */
	tc_core = tcase_create ("Core");

	tcase_add_test (tc_core, test_shuffle);

	/* Abort test case */
	tc_abort = tcase_create ("Abort");

	tcase_add_test_raise_signal (tc_abort,
			test_shuffle_fatal1, SIGABRT);
	tcase_add_test_raise_signal (tc_abort,
			test_shuffle_fatal2, SIGABRT);

	suite_add_tcase (s, tc_core);
	suite_add_tcase (s, tc_abort);

	return s;
}
