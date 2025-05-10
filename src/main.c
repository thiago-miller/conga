#include <stdlib.h>
#include "config.h"

int
main (int argc, char **argv)
{
	config_setup_environment ();

	Config *c = config_new ();
	config_apply_args (c, argc, argv);
	config_free (c);

	return EXIT_SUCCESS;
}
