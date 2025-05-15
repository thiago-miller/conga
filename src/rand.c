#include "rand.h"

#include <stdlib.h>
#include <assert.h>
#include "wrapper.h"

struct _Rand
{
	struct drand48_data state;
};

Rand *
rand_new (long seed)
{
	Rand *rng = xcalloc (1, sizeof (Rand));
	srand48_r (seed, &rng->state);
	return rng;
}

void
rand_free (Rand *rng)
{
	xfree (rng);
}

double
rand_uniform (Rand *rng)
{
	assert(rng != NULL);

	double x;

	drand48_r (&rng->state, &x);

	return x;
}
