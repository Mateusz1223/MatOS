#pragma once

#include "inc/common.h"

typedef struct ScanBuffer
{
	char *buffer; // max 500 characters
	size_t size;

	int pos;

	int lastCharacter;

} ScanBuffer;

typedef struct Terminal Terminal;

typedef struct Terminal
{
	char buffer[8010];
	size_t buffSize;

	int height;
	int width;
	int cursorX;
	int cursorY;
	bool cursorEnabled;
	char color;

	int displayY;
	int lastCharacterY;

	bool displayUpdated;

	void (*thread)(Terminal *);

	ScanBuffer scanBuffer;

	// states
	bool processInProgress;
	bool scanInProgress;

} Terminal;

Terminal *debugTerminal;

void terminal_init(Terminal *this, bool debugMode);

void terminal_clear(Terminal *this);
int terminal_print(Terminal *this, const char *str, ...); // returns 0 when print was not possible, 1 when executed properly
void terminal_putchar(Terminal *this, char ch);
void terminal_set_color(Terminal *this, char ch);

// Functions for UI manager keyboard irq resident
void terminal_scroll_up(Terminal *this);
void terminal_scroll_down(Terminal *this);
void terminal_move_cursor_left(Terminal *this);
void terminal_move_cursor_right(Terminal *this);
void terminal_add_char_to_scan_buffer(Terminal *this, char ch);
void terminal_backspace_in_scan_buffer(Terminal *this);
void terminal_delete_in_scan_buffer(Terminal *this);
void terminal_end_scan(Terminal *this); // adds end of string to the end of buffer

// tmp
void terminal_scan(Terminal *this, char *buff, size_t size);