#include "check_conga.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "../src/wrapper.h"

static int alloc_failed = 0;

// Mocking
void * __real_malloc  (size_t bytes);
void * __real_calloc  (size_t nmemb, size_t bytes);

void
setup_oom (void)
{
	alloc_failed = 1;
}

void
teardown_oom (void)
{
	alloc_failed = 0;
}

void *
__wrap_malloc (size_t bytes)
{
	if (alloc_failed)
		{
			errno = ENOMEM;
			return NULL;
		}

	return __real_malloc (bytes);
}

void *
__wrap_calloc (size_t nmemb, size_t bytes)
{
	if (alloc_failed)
		{
			errno = ENOMEM;
			return NULL;
		}

	return __real_calloc (nmemb, bytes);
}

START_TEST (test_xmalloc)
{
	void *p = xmalloc (sizeof (int));
	ck_assert_ptr_nonnull (p);

	* (int *) p = 66;
	ck_assert_int_eq (* (int *) p, 66);
	xfree (p);

	p = xmalloc (0);
	xfree (p);
}
END_TEST

START_TEST (test_xcalloc)
{
	void *p = xcalloc (1, sizeof (int));
	ck_assert_ptr_nonnull (p);

	ck_assert_int_eq (* (int *) p, 0);
	xfree (p);

	p = xcalloc (0, sizeof (int));
	xfree (p);

	p = xcalloc (1, 0);
	xfree (p);
}
END_TEST

START_TEST (test_xfree)
{
	void *p = xmalloc (1);
	xfree (p);
	xfree (NULL);
}
END_TEST

START_TEST (test_xstrdup)
{
	char *s = xstrdup ("PONGA");
	ck_assert_str_eq (s, "PONGA");
	xfree (s);
}
END_TEST

START_TEST (test_xmalloc1_oom)
{
	void *p = xmalloc (10);
	xfree (p);
}
END_TEST

START_TEST (test_xmalloc2_oom)
{
	void *p = xmalloc (0);
	xfree (p);
}
END_TEST

START_TEST (test_xcalloc1_oom)
{
	void *p = xcalloc (10, 10);
	xfree (p);
}
END_TEST

START_TEST (test_xcalloc2_oom)
{
	void *p = xcalloc (0, 10);
	xfree (p);
}
END_TEST

START_TEST (test_xstrdup_oom)
{
	char *s = xstrdup ("PONGA");
	xfree (s);
}
END_TEST

START_TEST (test_xfopen)
{
	char path[] = "/tmp/pongaXXXXXX";
	int fd = mkstemp (path);
	close (fd);

	FILE *fp = xfopen (path, "w");
	ck_assert (fp != NULL);
	xfclose (fp);

	fp = xfopen (path, "r");
	ck_assert (fp != NULL);
	xfclose (fp);

	unlink (path);
}
END_TEST

START_TEST (test_xfclose)
{
	xfclose (NULL);
}
END_TEST

START_TEST (test_xfopen_rw_abort)
{
	xfopen (NULL, "r+");
}
END_TEST

START_TEST (test_xfopen_w_abort)
{
	xfopen (NULL, "w");
}
END_TEST

START_TEST (test_xfopen_r_abort)
{
	xfopen (NULL, "r");
}
END_TEST

Suite *
make_wrapper_suite (void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_abort;

	s = suite_create ("Wrapper");

	/* Core test case */
	tc_core = tcase_create ("Core");

	/* Abort test case */
	tc_abort = tcase_create ("Abort");
	tcase_add_checked_fixture (tc_abort,
			setup_oom, teardown_oom);

	tcase_add_test (tc_core, test_xmalloc);
	tcase_add_test (tc_core, test_xcalloc);
	tcase_add_test (tc_core, test_xfree);
	tcase_add_test (tc_core, test_xstrdup);
	tcase_add_test (tc_core, test_xfopen);
	tcase_add_test (tc_core, test_xfclose);

	tcase_add_exit_test (tc_abort,
			test_xmalloc1_oom, EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xmalloc2_oom, EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xcalloc1_oom, EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xcalloc2_oom, EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xstrdup_oom, EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xfopen_rw_abort,  EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xfopen_w_abort,   EXIT_FAILURE);
	tcase_add_exit_test (tc_abort,
			test_xfopen_r_abort,   EXIT_FAILURE);

	suite_add_tcase (s, tc_core);
	suite_add_tcase (s, tc_abort);

	return s;
}
