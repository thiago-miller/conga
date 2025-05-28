#pragma once

#include "rand.h"

void   shuffle    (int *vet, int nmemb, Rand *rng);
char * chomp      (char *str);
char * trimc      (char *str, int c);
char * trim       (char *str);
char * file_slurp (const char *filename);
