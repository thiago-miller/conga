#include <stdlib.h>
#include "conga.h"
#include "config.h"

int
main (int argc, char **argv)
{
	config_setup_environment ();

	Config *c = config_new ();
	config_apply_args (c, argc, argv);

	conga_init (c);
	conga_run ();
	conga_finish ();

	config_free (c);

	return EXIT_SUCCESS;
}
