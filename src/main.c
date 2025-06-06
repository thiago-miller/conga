#include "conga.h"
#include "config.h"
#include "screen.h"

int
main (int argc, char **argv)
{
	screen_init ();
	config_setup_environment ();

	Config *cfg = config_new ();
	config_apply_args (cfg, argc, argv);

	Conga *game = conga_new (cfg);
	conga_run (game);
	conga_free (game);

	config_free (cfg);
	screen_finish ();

	return 0;
}
