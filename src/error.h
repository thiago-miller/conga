#pragma once

extern void (*error_print_progname) (void);

void error (int status, int errnum, const char *format, ...)
	__attribute__((format (printf, 3, 4)));
