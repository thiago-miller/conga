#pragma once

#include <stdio.h>
#include <stdlib.h>

void * xmalloc   (size_t size);
void * xcalloc   (size_t nmemb, size_t size);
void   xfree     (void *ptr);
char * xstrdup   (const char *str);

FILE * xfopen    (const char *path, const char *mode);
void   xfclose   (FILE *fp);
void   xfseek    (FILE *fp, long offset, int whence);
long   xftell    (FILE *fp);

int    xasprintf (char **buf, const char *fmt, ...)
	__attribute__ ((format (printf,2,3)));
