#include "wrapper.h"

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

FILE *
xfopen (const char *path, const char *mode)
{
	FILE *fp = fopen (path, mode);
	if (fp != NULL)
		return fp;

	if (*mode && mode[1] == '+')
		error (1, 1, "Could not open '%s' for reading and writing", path);
	else if (*mode == 'w' || *mode == 'a')
		error (1, 1, "Could not open '%s' for writing", path);
	else
		error (1, 1, "Could not open '%s' for reading", path);

	return fp;
}

void
xfclose (FILE *fp)
{
	if (fp == NULL)
		return;

	if (fclose (fp) == EOF)
		error (1, 1, "Could not close file stream");
}
