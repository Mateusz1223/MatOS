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

char keyIdLookUpTable[76];

void keyboard_init();

bool keyboard_is_key_pressed(int id); 

bool keyboard_is_caps_lock();

void keyboard_irq();