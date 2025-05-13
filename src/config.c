#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <error.h>
#include "wrapper.h"
#include "rule.h"

#define PROGNAME     "conga"
#define ROWS         20
#define COLS         30
#define SEED         17
#define DELAY        500000
#define LIVE_PERCENT 0.50
#define RULE         "conway"

static void
config_print_usage (FILE *fp)
{
	fprintf (fp,
		"%s\n"
		"\n"
		"Usage: %s [-hv] [-R STR] [-r INT] [-c INT] [-t INT] [-p FLOAT] [-s INT]\n"
		"\n"
		"Options:\n"
		"   -h, --help           Show help options\n"
		"   -r, --rows           Number of grid rows [%d]\n"
		"   -c, --cols           Number of grid cols [%d]\n"
		"   -s, --seed           Seed for reproducibility [%d]\n"
		"   -p, --live-percent   Percentage of living cells [%f]\n"
		"   -t, --delay          Generation delay in microseconds [%d]\n"
		"   -R, --rule           Cellular automaton rule [%s]\n"
		"                        Use a named alias (e.g., conway, highlife)\n"
		"                        or a custom rule in RLE (Bx/Sy) format.\n"
		"                        See section RULE below for details\n"
		"\n"
		"RULE\n"
		"A cellular automaton rule defines how cells are born and survive\n"
		"based on the number of live neighbors.\n"
		"\n"
		"Rules can be specified in one of two ways:\n"
		"\n"
		"1. Named aliases:\n"
		"\n"
		"Common pre-defined rules can be referenced by name. Examples:\n"
		"\n"
		"* conway - Classic Game of Life (B3/S23)\n"
		"  Cells are born with exactly 3 neighbors and\n"
		"  survive with 2 or 3.\n"
		"\n"
		"* highlife - Variant of Conway's rule (B36/S23)\n"
		"  Same as conway but also allows birth with 6 neighbors,\n"
		"  which enables the appearance of replicators.\n"
		"\n"
		"* seeds - Minimalistic explosive rule (B2/S)\n"
		"  All live cells die every generation; dead cells with\n"
		"  exactly 2 neighbors are born.\n"
		"\n"
		"* life34 - Only cells with 3 or 4 neighbors are born\n"
		"  or survive (B34/S34)\n"
		"  This rule is known for its stable behavior, with contained\n"
		"  growth. It eliminates the explosive behavior common in\n"
		"  other variants.\n"
		"\n"
		"* life_without_death - (B3/S012345678)\n"
		"  Once a cell is born, it never dies. Growth is irreversible.\n"
		"\n"
		"* replicator - (B1357/S1357)\n"
		" A rule where every pattern replicates over time. Both birth\n"
		" and survival occur for odd neighbor counts.\n"
		"\n"
		"* day_and_night - (B3678/S34678)\n"
		"  Symmetric under live/dead inversion: same behavior if\n"
		"  0s and 1s are flipped.\n"
		"\n"
		"* anneal - (B4678/S35678)\n"
		"  Tends toward stability or complete activation depending\n"
		"  on initial state.\n"
		"\n"
		"* mazectric - (B3/S1234)\n"
		"  Maze-like patterns with branching behavior.\n"
		"\n"
		"* maze - (B3/S12345)\n"
		"  Tends to grow into complex maze-like structures that freeze\n"
		"  into stable forms.\n"
		"\n"
		"2. Custom rules in RLE format:\n"
		"\n"
		"This format explicitly defines birth (B) and survival (S)\n"
		"conditions.\n"
		"\n"
		"For example:\n"
		"B3/S23   - Classic Conway’s rule: born with 3 neighbors,\n"
		"           survives with 2 or 3\n"
		"S23/B3   - Same as above (order and case are ignored)\n"
		"B36/S23  - Highlife rule\n"
		"\n"
		"The format is case-insensitive, and B and S can appear in\n"
		"any order.  Digits must range from 0 to 8.\n"
		"\n"
		"A cell is born if it is currently dead and the number of\n"
		"live neighbors matches any of the values listed after 'B'.\n"
		"A cell survives if it is currently alive and the number of\n"
		"live neighbors matches any of the values listed after 'S'.\n"
		"\n",
		PROGNAME, PROGNAME, ROWS, COLS, SEED, LIVE_PERCENT, DELAY, RULE);
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
		.live_percent = LIVE_PERCENT,
		.rule         = RULE
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

	if (c->live_percent <= 0.0 || c->live_percent >= 1.0)
		error (1, 0, "--live-percent must be [0.0, 1.0[");

	if (!rule_is_valid (c->rule))
		error (1, 0, "--rule is not a valid rule or alias");
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
		{"rule",         required_argument, 0, 'R'},
		{0,              0,                 0,  0 }
	};

	// progname for getopt
	argv[0] = PROGNAME;

	while ((o = getopt_long (argc, argv, "hr:c:s:t:p:R:", opt, &option_index)) >= 0)
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
				case 'R':
					{
						c->rule = optarg;
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
