#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <assert.h>
#include "wrapper.h"

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

char *
chomp (char *str)
{
	if (str == NULL)
		return NULL;

	size_t len = strlen (str);

	if (len && str[len - 1] == '\n')
		str[len - 1] = '\0';

	return str;
}

char *
trimc (char *str, int c)
{
	if (str == NULL)
		return NULL;

	size_t start, end;
	size_t len = strlen (str);

	// Empty string
	if (len == 0)
		return str;

	// Leading chars 'c'
	for (start = 0; start < len && str[start] == c; start++)
		;

	// All 'c' characters
	if (start == len)
		{
			str[0] = '\0';
			return str;
		}

	// Trailing chars 'c'
	for (end = len - 1; end >= start && str[end] == c; end--)
		;

	memmove (str, str + start, sizeof (char) * (end - start + 1));
	str[end - start + 1] = '\0';

	return str;
}

char *
trim (char *str)
{
	return trimc (str, ' ');
}

char *
file_slurp (const char *filename)
{
	FILE *fp = NULL;
	char *buf = NULL;
	long size = 0, ret = 0;

	fp = xfopen (filename, "r");

	xfseek (fp, 0L, SEEK_END);
	size = xftell (fp);
	buf = xmalloc (size + 1);

	xfseek (fp, 0L, SEEK_SET);
	ret = fread (buf, sizeof (char), size, fp);
	if (ret != size)
		error (1, 0, "fread failed");

	buf[size] = '\0';
	xfclose (fp);

	return buf;
}
