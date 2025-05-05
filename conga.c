#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>

#define ROWS         20
#define COLS         30
#define SEED         17
#define DELAY        1000000
#define LIVE_PERCENT 0.50
#define CURSOR_OFF   "\e[?25l"
#define CURSOR_ON    "\e[?25h"
#define CURSOR_UP    "\e[H"
#define CLEAN_SCREEN "\e[H\e[2J"
#define CLEAN_LINE   "\e[A\e[k"
#define DEAD         "\u2B1B"
#define ALIVE        "\u2B1C"
#define NEIGHBORS    8

int delta_i[] = {-1, -1, -1,  0, 0, +1, +1, +1};
int delta_j[] = {-1,  0, +1, -1, +1, -1, 0, +1};

char *status[] = {DEAD, ALIVE};

int buffer1[ROWS][COLS] = {0};
int buffer2[ROWS][COLS] = {0};

int (*grid)[COLS]     = NULL;
int (*new_grid)[COLS] = NULL;

float live_percent = LIVE_PERCENT;
int seed = SEED;
int done = 0;

int
count_neighbors (int i, int j)
{
	int ni = 0, nj = 0;
	int live_neighbors = 0;

	for (int k = 0; k < NEIGHBORS; k++)
		{
			ni = (i + delta_i[k] + ROWS) % ROWS;
			nj = (j + delta_j[k] + COLS) % COLS;

			if (grid[ni][nj])
				live_neighbors++;
		}

	return live_neighbors;
}

void
step_generation (void)
{
	int (*swap)[COLS]  = NULL;
	int live_neighbors = 0;

	for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < COLS; j++)
				{
					live_neighbors = count_neighbors (i, j);

					if (grid[i][j] && (live_neighbors == 2 || live_neighbors == 3))
						new_grid[i][j] = 1;
					else if (!grid[i][j] && live_neighbors == 3)
						new_grid[i][j] = 1;
					else
						new_grid[i][j] = 0;
				}
		}

	swap = grid;
	grid = new_grid;
	new_grid = swap;
}

void
shutdown (int signum)
{
	done = 1;
}

void
cleanup (void)
{
	for  (int i = 0; i <= ROWS; i++)
		printf (CLEAN_LINE);

	printf (CURSOR_ON);
}

void
set_first_generation (void)
{
	int total_cells = ROWS * COLS;
	int live_cells  =  total_cells * live_percent;
	int r = 0, temp = 0;

	int pos[total_cells];

	for (int i = 0; i < total_cells; i++)
		pos[i] = i;

	// Fisher-Yates
	for (int i = total_cells - 1; i > 0; i--)
		{
			r = rand() % (i + 1);
			temp = pos[r];
			pos[r] = pos[i];
			pos[i] = temp;
		}

	for (int i = 0; i < live_cells; i++)
		grid[pos[i] / COLS][pos[i] % COLS] = 1;
}

void
init (void)
{
	signal (SIGINT,  shutdown);
	signal (SIGQUIT, shutdown);
	signal (SIGTERM, shutdown);

	srand (seed);

	grid = buffer1;
	new_grid = buffer2;

	set_first_generation ();

	printf (CLEAN_SCREEN);
	printf (CURSOR_OFF);
}

void
run (void)
{
	while (!done)
		{
			printf (CURSOR_UP);

			for (int i = 0; i < ROWS; i++)
				{
					for (int j = 0; j < COLS; j++)
						printf (status[grid[i][j]]);

					printf ("\n");
				}

			step_generation ();
			usleep (DELAY);
		}
}

void
parse_args (int argc, char **argv)
{
	int option_index;
	int c;

	struct option opt[] =
	{
		{"help",         no_argument,       0, 'h'},
		{"seed",         required_argument, 0, 's'},
		{"live-percent", required_argument, 0, 'p'},
		{0,              0,                 0,  0 }
	};

	while ((c = getopt_long (argc, argv, "hs:p:", opt, &option_index)) >= 0)
		{
			switch (c)
				{
				case 'h':
					{
						printf (
								"Usage: %s [-p FLOAT] [-s INT]\n"
								"\n"
								"  -s, --seed           set seed for reproducibility [%d]\n"
								"  -p, --live-percent   percentage of living cells [%f]\n"
								"\n", argv[0], SEED, LIVE_PERCENT
						);
						exit (EXIT_SUCCESS);
						break;
					}
				case 's':
					{
						seed = atoi (optarg);
						break;
					}
				case 'p':
					{
						live_percent = atof (optarg);
						if (live_percent <= 0.0 || live_percent >= 1.0)
							{
								fprintf (stderr,
										"%s: --live-percent must be [0.0, 1.0[\n", argv[0]);
								exit (EXIT_FAILURE);
							}
						break;
					}
				case '?':
				case ':':
					{
						fprintf (stderr,
								"Try '%s  --help' for more information\n", argv[0]);
						exit (EXIT_FAILURE);
						break;
					}
				}
		}
}

int
main (int argc, char **argv)
{
	parse_args (argc, argv);

	init ();
	run ();
	cleanup ();

	return EXIT_SUCCESS;
}
