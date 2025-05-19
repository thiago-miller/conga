#include "check_conga.h"

int
main (void)
{
	int number_failed = 0;
	SRunner *sr = NULL;

	sr = srunner_create (NULL);
	srunner_set_tap (sr, "-");

	srunner_add_suite (sr, make_wrapper_suite ());
	srunner_add_suite (sr, make_rand_suite ());
	srunner_add_suite (sr, make_utils_suite ());
	srunner_add_suite (sr, make_rule_suite ());

	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return number_failed;
}
