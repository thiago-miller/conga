#include "utils.h"

#include <stdio.h>
#include <assert.h>

void
shuffle (int *vet, int nmemb, int (*func) (void))
{
	assert (vet != NULL && nmemb > 0);
	assert (func != NULL);

	int temp = 0;
	int r = 0;

	if (nmemb <= 1)
		return;

	// Fisher-Yates
	for (int i = nmemb - 1; i > 0; i--)
		{
			r = func () % (i + 1);
			temp = vet[r];
			vet[r] = vet[i];
			vet[i] = temp;
		}
}
