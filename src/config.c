#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>
#include <error.h>
#include "wrapper.h"
#include "rule.h"

#ifdef HAVE_VERSION_H
#include "version.h"
#else
#define VERSION "0.0.0-dummy"
#endif

#define PROGNAME     "conga"
#define ROWS         20
#define COLS         30
#define SEED         time (NULL)
#define DELAY        500000
#define LIVE_PERCENT 0.50
#define RULE         "conway"

static void
config_print_usage (FILE *fp)
{
	fprintf (fp,
		"%s %s\n"
		"\n"
		"Usage: %s [-hL] [-R STR] [-r INT] [-c INT] [-t INT] [-p FLOAT] [-s INT]\n"
		"\n"
		"Options:\n"
		"   -h, --help           Show help options\n"
		"   -r, --rows           Number of grid rows [%d]\n"
		"   -c, --cols           Number of grid cols [%d]\n"
		"   -s, --seed           Seed for reproducibility [time(NULL)]\n"
		"   -p, --live-percent   Percentage of living cells [%f]\n"
		"   -t, --delay          Generation delay in microseconds [%d]\n"
		"   -L, --list-rules     List all available rule aliases and exit\n"
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
		"Common pre-defined rules can be referenced by name.\n"
		"\n"
		"Examples:\n"
		"  conway   - B3/S23\n"
		"  highlife - B36/S23\n"
		"  seeds    - B2/S\n"
		"\n"
		"2. Custom rules in RLE format:\n"
		"\n"
		"This format explicitly defines birth (B) and survival (S)\n"
		"conditions.\n"
		"\n"
		"For example:\n"
		"  B3/S23   - Classic Conway’s rule: born with 3 neighbors,\n"
		"             survives with 2 or 3\n"
		"  S23/B3   - Same as above (order and case are ignored)\n"
		"  B36/S23  - Highlife rule\n"
		"\n"
		"The format is case-insensitive, and B and S can appear in\n"
		"any order.  Digits must range from 0 to 8.\n"
		"\n"
		"A cell is born if it is currently dead and the number of\n"
		"live neighbors matches any of the values listed after 'B'.\n"
		"A cell survives if it is currently alive and the number of\n"
		"live neighbors matches any of the values listed after 'S'.\n"
		"\n",
		PROGNAME, VERSION, PROGNAME, ROWS, COLS, LIVE_PERCENT, DELAY, RULE);
}

static void
config_print_version (FILE *fp)
{
	fprintf (fp, "%s %s\n", PROGNAME, VERSION);
}

static void
config_print_rules (FILE *fp)
{
	const RuleAlias *a = NULL;

	int biggest_len = 0;
	int len = 0;

	if (rule_aliases[0].name == NULL)
		{
			fprintf (fp,
					"No aliases available for the rules :(\n");
			return;
		}

	biggest_len = strlen (rule_aliases[0].name);
	for (a = &rule_aliases[1]; a->name != NULL; a++)
		{
			len = strlen (a->name);
			if (len > biggest_len)
				biggest_len = len;
		}

	fprintf (fp, "Available rule aliases:\n");

	for (a = rule_aliases; a->name != NULL; a++)
		fprintf (fp, "  - %-*s  %s\n",
				biggest_len, a->name, a->rule);

	fprintf (fp, "\n");
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
	Config *cfg = xcalloc (1, sizeof (Config));

	*cfg = (Config) {
		.seed         = SEED,
		.rows         = ROWS,
		.cols         = COLS,
		.delay        = DELAY,
		.live_percent = LIVE_PERCENT,
		.rule         = RULE
	};

	return cfg;
}

void
config_free (Config *cfg)
{
	xfree (cfg);
}

static void
config_validate_args (const Config *cfg)
{
	if (cfg->rows <= 0)
		error (1, 0, "--rows must be > 0");

	if (cfg->cols <= 0)
		error (1, 0, "--cols must be > 0");

	if (cfg->delay <= 0)
		error (1, 0, "--delay must be > 0");

	if (cfg->live_percent <= 0.0 || cfg->live_percent >= 1.0)
		error (1, 0, "--live-percent must be (0.0, 1.0)");

	if (!rule_is_valid (cfg->rule))
		error (1, 0, "--rule is not a valid rule or alias");
}

void
config_apply_args (Config *cfg, int argc, char **argv)
{
	assert (cfg != NULL);
	assert (argc > 0 && argv != NULL);

	int option_index;
	int o;

	struct option opt[] =
	{
		{"help",         no_argument,       0, 'h'},
		{"version",      no_argument,       0, 'V'},
		{"rows",         required_argument, 0, 'r'},
		{"cols",         required_argument, 0, 'c'},
		{"seed",         required_argument, 0, 's'},
		{"delay",        required_argument, 0, 't'},
		{"live-percent", required_argument, 0, 'p'},
		{"rule",         required_argument, 0, 'R'},
		{"list-rules",   no_argument,       0, 'L'},
		{"pattern",      required_argument, 0, 'P'},
		{0,              0,                 0,  0 }
	};

	// progname for getopt
	argv[0] = PROGNAME;

	while ((o = getopt_long (argc, argv, "hVr:c:s:t:p:R:LP:", opt, &option_index)) >= 0)
		{
			switch (o)
				{
				case 'h':
					{
						config_print_usage (stdout);
						exit (EXIT_SUCCESS);
					}
				case 'V':
					{
						config_print_version (stdout);
						exit (EXIT_SUCCESS);
					}
				case 'r':
					{
						cfg->rows = atoi (optarg);
						break;
					}
				case 'c':
					{
						cfg->cols = atoi (optarg);
						break;
					}
				case 's':
					{
						cfg->seed = atol (optarg);
						break;
					}
				case 't':
					{
						cfg->delay = atoi (optarg);
						break;
					}
				case 'p':
					{
						cfg->live_percent = atof (optarg);
						break;
					}
				case 'R':
					{
						cfg->rule = optarg;
						break;
					}
				case 'L':
					{
						config_print_rules (stdout);
						exit (EXIT_SUCCESS);
					}
				case 'P':
					{
						cfg->pattern_file = optarg;
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

	config_validate_args (cfg);
}
