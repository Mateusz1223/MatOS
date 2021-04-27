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

void terminal_init( Terminal *term );

void terminal_print( Terminal *term, const char *str, ... );
void terminal_putchar( Terminal *term, char ch );
void terminal_set_color( Terminal *term, char ch );

void terminal_scan( Terminal *term, const char *str, ... );

void terminal_scroll_up( Terminal *term );
void terminal_scroll_down( Terminal *term );