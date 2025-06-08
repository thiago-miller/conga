#include "conga.h"
#include "config.h"
#include "screen.h"

int
main (int argc, char **argv)
{
	config_setup_environment ();

	Config *cfg = config_new ();
	config_apply_args (cfg, argc, argv);

	screen_init ();

	Conga *game = conga_new (cfg);
	conga_run (game);
	conga_free (game);

	screen_finish ();

	config_free (cfg);

	return 0;
}
