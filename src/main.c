#include <stdlib.h>
#include "conga.h"
#include "config.h"

int
main (int argc, char **argv)
{
	config_setup_environment ();

	Config *cfg = config_new ();
	config_apply_args (cfg, argc, argv);

	Conga *game = conga_new (cfg);
	conga_startup ();

	conga_run (game);

	conga_shutdown ();
	conga_free (game);

	config_free (cfg);

	return EXIT_SUCCESS;
}
