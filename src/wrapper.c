#include "wrapper.h"

#include <stdio.h>
#include <string.h>
#include <error.h>

void *
xmalloc (size_t size)
{
	void *ret = malloc (size);

	if (ret == NULL && !size)
		ret = malloc (1);

	if (ret == NULL)
		error (1, 1, "malloc failed");

	return ret;
}

void *
xcalloc (size_t nmemb, size_t size)
{
	void *ret = calloc (nmemb, size);

	if (ret == NULL && (!nmemb || !size))
		ret = calloc (1, 1);

	if (ret == NULL)
		error (1, 1, "calloc failed");

	return ret;
}

void
xfree (void *ptr)
{
	if (ptr == NULL)
		return;
	free (ptr);
}

char *
xstrdup (const char *str)
{
	size_t len = strlen (str) + 1;
	void *new = malloc (len);

	if (new == NULL)
		error (1, 1, "strdup failed");

	return (char *) memcpy (new, str, len);
}
