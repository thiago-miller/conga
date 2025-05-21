#pragma once

enum InputKey
{
	INPUT_NONE        = -1,
	INPUT_ESC         = 27,
#ifdef _WIN32
	INPUT_ENTER       = '\r',
#else
	INPUT_ENTER       = '\n',
#endif
	INPUT_SPACE       = ' ',
	INPUT_TAB         = '\t',

	INPUT_ARROW_UP    = 1000,
	INPUT_ARROW_DOWN,
	INPUT_ARROW_RIGHT,
	INPUT_ARROW_LEFT
};

void input_init      (void);
void input_finish    (void);
int  input_read_key  (void);
