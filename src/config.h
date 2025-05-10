#pragma once

typedef struct
{
	int   seed;
	int   rows;
	int   cols;
	int   delay;
	float live_percent;
} Config;

void     config_setup_environment (void);
Config * config_new               (void);
void     config_apply_args        (Config *c, int argc, char **argv);
void     config_free              (Config *c);
