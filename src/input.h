#pragma once

enum InputKey
{
	INPUT_NONE        = -1,
	INPUT_ESC         = 27,
	INPUT_ENTER       = '\n',
	INPUT_SPACE       = ' ',
	INPUT_BACKSPACE   = 127,
	INPUT_TAB         = '\t',

	INPUT_ARROW_UP    = 1000,
	INPUT_ARROW_DOWN,
	INPUT_ARROW_RIGHT,
	INPUT_ARROW_LEFT,

	INPUT_DELETE,
	INPUT_HOME,
	INPUT_END,
	INPUT_PAGE_UP,
	INPUT_PAGE_DOWN,

	INPUT_F1,
	INPUT_F2,
	INPUT_F3,
	INPUT_F4,
	INPUT_F5,
	INPUT_F6,
	INPUT_F7,
	INPUT_F8,
	INPUT_F9,
	INPUT_F10,
	INPUT_F11,
	INPUT_F12
};

void input_init      (void);
void input_finish    (void);
int  input_read_key  (void);
