#pragma once

#include "inc/common.h"

// buffer will be used in screen_scan
/*typedef struct KeyboardBuffer
{
	int maxSize;
	char *buffer;

	int position;
	int size;

	int updatedPosition;

	// pointer to add_character(char ch);
} Keyboard_buffer;

Keyboard_buffer Nullbuffer; */

void keyboard_init(); // to do

void keyboard_irq(); // to do

bool keyboard_is_key_pressed(int id); // to do