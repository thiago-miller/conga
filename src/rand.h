#pragma once

typedef struct _Rand Rand;

Rand * rand_new     (long seed);
double rand_uniform (Rand *rng);
void   rand_free    (Rand *rng);

#define RAND_INT(r,n) ((int)(rand_uniform (r) * (n)))
