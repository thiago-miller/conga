#pragma once

typedef struct
{
	const char *progname;
	const char *version;
	const char *pattern_file;
	const char *rule;
	long        seed;
	int         rows;
	int         cols;
	int         delay;
	float       live_percent;
} Config;

void     config_setup_environment (void);
Config * config_new               (void);
void     config_apply_args        (Config *cfg, int argc, char **argv);
void     config_free              (Config *cfg);
