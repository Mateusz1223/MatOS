#pragma once

#include "inc/common.h"

typedef struct Terminal
{
	char buffer[8010]; // to be dynamically allocated, 10 for security
	int buffSize; // in VGA charcters - 2 bytes

	int height;
	int width;
	int cursorX;
	int cursorY;
	bool cursorEnabled;
	char color;

	int displayY;
	int lastCharacterY;

	bool displayUpdated;
} Terminal;

Terminal *debugTerminal;

void terminal_init( Terminal *this );

void terminal_print( Terminal *this, const char *str, ... );
void terminal_putchar( Terminal *this, char ch );
void terminal_set_color( Terminal *this, char ch );

void terminal_scroll_up( Terminal *this );
void terminal_scroll_down( Terminal *this );