#include "utils.h"

#include <stdio.h>
#include <assert.h>

void
shuffle (int *vet, int nmemb, Rand *rng)
{
	assert (vet != NULL && nmemb > 0);
	assert (rng != NULL);

	int temp = 0;
	int r = 0;

	if (nmemb <= 1)
		return;

	// Fisher-Yates
	for (int i = nmemb - 1; i > 0; i--)
		{
			r = RAND_INT (rng, i + 1);
			temp = vet[r];
			vet[r] = vet[i];
			vet[i] = temp;
		}
}
