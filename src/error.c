#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

void (*error_print_progname) (void) = NULL;
void (*error_exit_cleanup)   (void) = NULL;

void
error (int status, int errnum, const char *format, ...)
{
	va_list args;

	if (error_print_progname != NULL)
		(*error_print_progname) ();

	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);

	if (errnum)
		fprintf (stderr, ": %s", strerror (errno));

	fprintf (stderr, "\n");
	fflush( stderr);

	if (status)
		{
			if (error_exit_cleanup != NULL)
				(*error_exit_cleanup) ();
			exit (status);
		}
}
