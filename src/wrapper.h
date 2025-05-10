#pragma once

#include <stdlib.h>

void * xmalloc  (size_t size);
void * xcalloc  (size_t nmemb, size_t size);
void   xfree    (void *ptr);
