#include "wrapper.h"

#include <string.h>
#include <stdarg.h>
#include "error.h"

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

int
xasprintf (char **buf, const char *fmt, ...)
{
	va_list argp;
	va_start (argp, fmt);
	char one_char[1];

	int len = vsnprintf (one_char, 1, fmt, argp);
	if (len < 1)
		error (1, 0, "asprintf failed");

	va_end (argp);

	*buf = xmalloc (len + 1);

	va_start (argp, fmt);
	vsnprintf (*buf, len + 1, fmt, argp);
	va_end (argp);

	return len;
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

void
xfseek (FILE *fp, long offset, int whence)
{
	if (fseek (fp, offset, whence) == EOF)
		error (1, 1, "fseek failed");
}

long
xftell (FILE *fp)
{
	long offset = ftell (fp);

	if (offset == EOF)
		error (1, 1, "ftell failed");

	return offset;
}
