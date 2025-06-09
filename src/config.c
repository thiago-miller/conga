#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>
#include "wrapper.h"
#include "rule.h"
#include "pattern.h"
#include "event.h"
#include "error.h"
#include "screen.h"

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
	int pkg_len = strlen (PROGNAME);
	fprintf (fp,
		"%s %s\n"
		"\n"
		"Usage: %s [-hV] [-R STR] [-r INT] [-c INT] [-t INT] [-p FLOAT] [-s INT]\n"
		"       %*c [-P [STR|FILE]] [--list-rules] [--list-patterns]\n"
		"\n"
		"Options:\n"
		"   -h, --help           Show help options\n"
		"   -V, --version        Show current version\n"
		"   -r, --rows           Number of grid rows [%d]\n"
		"   -c, --cols           Number of grid cols [%d]\n"
		"   -s, --seed           Seed for reproducibility [time(NULL)]\n"
		"   -p, --live-percent   Percentage of living cells [%f]\n"
		"   -t, --delay          Generation delay in microseconds [%d]\n"
		"   -R, --rule           Cellular automaton rule [%s]\n"
		"                        Use a named alias (e.g., conway, highlife)\n"
		"                        or a custom rule Golly/RLE (Bx/Sy) format.\n"
		"                        See section RULE below for details\n"
		"   -P, --pattern        Load initial pattern. Use a named alias \n"
		"                        (e.g., glider, tumbler). See section PATTERN\n"
		"                        below for details\n"
		"       --pattern-file   Custom pattern from Golly/TLE file format\n"
		"       --list-rules     List all available rule aliases and exit\n"
		"       --list-patterns  List all available pattern aliases and exit\n"
		"\n"
		"RULE\n"
		" A cellular automaton rule defines how cells are born and survive\n"
		" based on the number of live neighbors.\n"
		" \n"
		" Rules can be specified in one of two ways:\n"
		" \n"
		" 1. Named aliases:\n"
		" \n"
		" Common pre-defined rules can be referenced by name.\n"
		" \n"
		" Examples:\n"
		"   conway   - B3/S23\n"
		"   highlife - B36/S23\n"
		"   seeds    - B2/S\n"
		" \n"
		" 2. Custom rules in Golly/RLE format:\n"
		" \n"
		" This format explicitly defines birth (B) and survival (S)\n"
		" conditions.\n"
		" \n"
		" For example:\n"
		"   B3/S23   - Classic Conway’s rule: born with 3 neighbors,\n"
		"              survives with 2 or 3\n"
		"   S23/B3   - Same as above (order and case are ignored)\n"
		"   b3/s23   - Lowercase also works\n"
		"   3/23     - Shortcut form: birth=3, survival=2 or 3\n"
		" \n"
		" Notes:\n"
		"   - Digits must be in the range 0–8.\n"
		"   - Order of B and S is flexible in the Bx/Sy format.\n"
		"   - In the shortcut (x/y), order is fixed: birth/survival.\n"
		" \n"
		" A cell is born if it is dead and has exactly one of the listed\n"
		" birth neighbor counts. It survives if it is alive and has\n"
		" exactly one of the listed survival neighbor counts.\n"
		"\n"
		"PATTERN\n"
		" By default, the initial generation (generation 0) is created\n"
		" randomly. The random seed is set with --seed and the initial\n"
		" percentage of live cells is controlled by --live-percent.\n"
		"\n"
		" Initial patterns can be loaded from:\n"
		"   - Named aliases: classic patterns embedded in the program.\n"
		"   - Golly/RLE files: use the standard run-length encoded (RLE)\n"
		"     format supported by Golly. This format specifies:\n"
		"     - A header with grid dimensions and optional rule.\n"
		"     - Lines with cells encoded as counts and states (e.g., 3o for\n"
		"       three live cells, 2b for two dead cells).\n"
		"     - Rows separated by '$' and end marked by '!'.\n"
		" \n"
		" Example RLE snippet:\n"
		"   x = 3, y = 3, rule = B3/S23\n"
		"   bo$2bo$3o!\n"
		"\n",
		PROGNAME, VERSION, PROGNAME, pkg_len, ' ', ROWS, COLS,
		LIVE_PERCENT, DELAY, RULE);
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
config_print_patterns (FILE *fp)
{
	const PatternDef *def = NULL;

	int biggest_len = 0;
	int len = 0;

	if (pattern_defs[0].name == NULL)
		{
			fprintf (fp,
					"No aliases available for the patterns :(\n");
			return;
		}

	biggest_len = strlen (pattern_defs[0].name);
	for (def = &pattern_defs[1]; def->name != NULL; def++)
		{
			len = strlen (def->name);
			if (len > biggest_len)
				biggest_len = len;
		}

	fprintf (fp, "Available pattern aliases:\n");

	for (def = pattern_defs; def->name != NULL; def++)
		{
			fprintf (fp,
					"  - %-*s  rule=%s, dim=%dx%d\n"
					"    %-*c  %s\n",
					biggest_len, def->name, def->header.rule,
					def->header.rows, def->header.cols,
					biggest_len, ' ', def->desc);
		}

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

	// Set cleanup to call screen_finish before
	// exiting in case of error
	error_exit_cleanup = screen_finish;
}

Config *
config_new (void)
{
	Config *cfg = xcalloc (1, sizeof (Config));

	*cfg = (Config) {
		.progname     = PROGNAME,
		.version      = VERSION,
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

	if (cfg->delay < EVENT_DELAY_MIN || cfg->delay > EVENT_DELAY_MAX)
		error (1, 0, "--delay must be [%d, %d]",
				EVENT_DELAY_MIN, EVENT_DELAY_MAX);

	if (cfg->live_percent <= 0.0 || cfg->live_percent >= 1.0)
		error (1, 0, "--live-percent must be (0.0, 1.0)");

	if (!rule_is_valid (cfg->rule))
		error (1, 0, "--rule is not a valid rule or alias");

	if (cfg->pattern != NULL && cfg->pattern_file != NULL)
		error (1, 0, "--pattern and --pattern-file cannot be set together");

	if (cfg->pattern != NULL && !pattern_alias_is_valid (cfg->pattern))
		error (1, 0, "--pattern is not a valid alias");

	if (cfg->pattern_file != NULL && !pattern_file_is_valid (cfg->pattern_file))
		error (1, 0, "--pattern is not a valid file or alias");
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
		{"help",          no_argument,       0, 'h'},
		{"version",       no_argument,       0, 'V'},
		{"rows",          required_argument, 0, 'r'},
		{"cols",          required_argument, 0, 'c'},
		{"seed",          required_argument, 0, 's'},
		{"delay",         required_argument, 0, 't'},
		{"live-percent",  required_argument, 0, 'p'},
		{"rule",          required_argument, 0, 'R'},
		{"list-rules",    no_argument,       0,  1 },
		{"pattern",       required_argument, 0, 'P'},
		{"pattern-file",  required_argument, 0,  2 },
		{"list-patterns", no_argument,       0,  3 },
		{0,               0,                 0,  0 }
	};

	// progname for getopt
	argv[0] = PROGNAME;

	while ((o = getopt_long (argc, argv, "hVr:c:s:t:p:R:P:", opt, &option_index)) >= 0)
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
				case 1:
					{
						config_print_rules (stdout);
						exit (EXIT_SUCCESS);
					}
				case 'P':
					{
						cfg->pattern = optarg;
						break;
					}
				case 2:
					{
						cfg->pattern_file = optarg;
						break;
					}
				case 3:
					{
						config_print_patterns (stdout);
						exit (EXIT_SUCCESS);
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
