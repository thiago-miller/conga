#pragma once

extern void (*error_print_progname) (void);
extern void (*error_exit_cleanup)   (void);

void error (int status, int errnum, const char *format, ...)
	__attribute__((format (printf, 3, 4)));
