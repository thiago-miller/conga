#include "input.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <error.h>

static struct termios saved_attributes;
static int initialized = 0;
#endif

void
input_init (void)
{
#ifdef _WIN32
	// <conio.h>
#else
	struct termios tattr;

	if (tcgetattr (STDIN_FILENO, &saved_attributes) == -1)
		error (1, 1, "tcgetattr");

	tattr = saved_attributes;
	tattr.c_lflag &= ~(ICANON | ECHO);
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;

	if (tcsetattr (STDIN_FILENO, TCSANOW, &tattr) == -1)
		error (1, 1, "tcsetattr");

	initialized = 1;
#endif
}

void
input_finish (void)
{
#ifdef _WIN32
	// <conio.h>
#else
	if (initialized)
		if (tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes) == -1)
			error (1, 1, "tcsetattr");
#endif
}

static inline int
input_available (void)
{
#ifdef _WIN32
	return _kbhit ();
#else
	struct timeval tv = {0L, 0L};
	fd_set fds;

	FD_ZERO (&fds);
	FD_SET (STDIN_FILENO, &fds);

	return select (STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
#endif
}

int
input_read_key (void)
{
	if (!input_available ())
		return -1;

#ifdef _WIN32
	return _getch ()
#else
	char c;

	if (read (STDIN_FILENO, &c, 1) == -1)
		error (1, 1, "read");

	return c;
#endif
}
