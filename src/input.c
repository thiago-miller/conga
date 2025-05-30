#include "input.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include "error.h"

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
		return INPUT_NONE;

#ifdef _WIN32
	int ch = _getch();

	if (ch == 224 || ch == 0)
		{
			if (!input_available ())
				return INPUT_NONE;

			// Windows arrow/function keys
			int ext = _getch();

			switch (ext)
				{
				case 72 : return INPUT_ARROW_UP;
				case 80 : return INPUT_ARROW_DOWN;
				case 75 : return INPUT_ARROW_LEFT;
				case 77 : return INPUT_ARROW_RIGHT;
				default : return INPUT_NONE;
				}
		}

	return ch;
#else
	unsigned char ch;

	if (read (STDIN_FILENO, &ch, 1) != 1)
		error (1, 1, "read");

	if (ch == 27)
		{
			// ESC
			unsigned char ext[2];

			for (int i = 0; i < 2; i++)
				if (!input_available ())
					return INPUT_ESC;
				else if (read (STDIN_FILENO, &ext[i], 1) != 1)
					error (1, 1, "read");

			if (ext[0] == '[')
				{
					switch (ext[1])
						{
						case 'A': return INPUT_ARROW_UP;
						case 'B': return INPUT_ARROW_DOWN;
						case 'C': return INPUT_ARROW_RIGHT;
						case 'D': return INPUT_ARROW_LEFT;
						default : return INPUT_ESC;
						}
				}

			// fallback
			return INPUT_ESC;
		}

	return ch;
#endif
}
