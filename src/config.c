#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <error.h>
#include "wrapper.h"

#define PROGNAME     "conga"
#define ROWS         20
#define COLS         30
#define SEED         17
#define DELAY        500000
#define LIVE_PERCENT 0.50

static void
config_print_usage (FILE *fp)
{
	fprintf (fp,
		"%s\n"
		"\n"
		"Usage: %s [-hv] [-r INT] [-c INT] [-t INT] [-p FLOAT] [-s INT]\n"
		"\n"
		"Options:\n"
		"   -h, --help           Show help options\n"
		"   -r, --rows           Number of grid rows [%d]\n"
		"   -c, --cols           Number of grid cols [%d]\n"
		"   -s, --seed           Seed for reproducibility [%d]\n"
		"   -p, --live-percent   Percentage of living cells [%f]\n"
		"   -t, --delay          Generation delay in microseconds [%d]\n"
		"\n",
		PROGNAME, PROGNAME, ROWS, COLS, SEED, LIVE_PERCENT, DELAY);
}

static void
config_print_progname (void)
{
	fprintf (stderr, "%s: ", PROGNAME);
}

void
config_setup_environment (void)
{
	// Set error progname
	error_print_progname = config_print_progname;
}

Config *
config_new (void)
{
	Config *c = xcalloc (1, sizeof (Config));

	*c = (Config) {
		.seed         = SEED,
		.rows         = ROWS,
		.cols         = COLS,
		.delay        = DELAY,
		.live_percent = LIVE_PERCENT
	};

	return c;
}

void
config_free (Config *c)
{
	xfree (c);
}

static void
config_validate_args (const Config *c)
{
	if (c->rows <= 0)
		error (1, 0, "--rows must be > 0");

	if (c->cols <= 0)
		error (1, 0, "--cols must be > 0");

	if (c->delay <= 0)
		error (1, 0, "--delay must be > 0");

	if (c->live_percent <= 0.0 || c->live_percent >= 1.1)
		error (1, 0, "--live-percent must be [0.0, 1.0[");
}

void
config_apply_args (Config *c, int argc, char **argv)
{
	assert (c != NULL);
	assert (argc > 0 && argv != NULL);

	int option_index;
	int o;

	struct option opt[] =
	{
		{"help",         no_argument,       0, 'h'},
		{"rows",         required_argument, 0, 'r'},
		{"cols",         required_argument, 0, 'c'},
		{"seed",         required_argument, 0, 's'},
		{"delay",        required_argument, 0, 't'},
		{"live-percent", required_argument, 0, 'p'},
		{0,              0,                 0,  0 }
	};

	// progname for getopt
	argv[0] = PROGNAME;

	while ((o = getopt_long (argc, argv, "hr:c:s:t:p:", opt, &option_index)) >= 0)
		{
			switch (o)
				{
				case 'h':
					{
						config_print_usage (stdout);
						exit (EXIT_SUCCESS);
					}
				case 'r':
					{
						c->rows = atoi (optarg);
						break;
					}
				case 'c':
					{
						c->cols = atoi (optarg);
						break;
					}
				case 's':
					{
						c->seed = atoi (optarg);
						break;
					}
				case 't':
					{
						c->delay = atoi (optarg);
						break;
					}
				case 'p':
					{
						c->live_percent = atof (optarg);
						break;
					}
				case '?':
				case ':':
					{
						fprintf (stderr,
							"Try '%s --help' for more information\n",
							PROGNAME);
						exit (EXIT_FAILURE);
					}
				}
		}

	config_validate_args (c);
}
