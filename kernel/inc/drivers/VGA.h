#pragma once
#include "inc/common.h"
#include "inc/bootinfo.h"

//usage of colors COLOR/BACKGROUND_COLOR | BLINKING

//font colors
#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define MAGNETA 0x5
#define BROWN 0x6
#define WHITE 0x7
#define GRAY 0x8
#define LIGHT_BLUE 0x9
#define LIGH_GREEN 0xa
#define LIGH_CYAN 0xb
#define LIGH_RED 0xc
#define LIGH_MAGNETA 0xd
#define YELLOW 0xe
#define BRIGHT_WHITE 0xf

//background colors
#define BACKGROUND_BLACK 0x0
#define BACKGROUND_BLUE 0x10
#define BACKGROUND_GREEN 0x20
#define BACKGROUND_CYAN 0x30
#define BACKGROUND_RED 0x40
#define BACKGROUND_MAGNETA 0x50
#define BACKGROUND_BROWN 0x60
#define BACKGROUND_WHITE 0x70

//blinking
#define BLINKING 0x80

void VGA_init(bootinfo* boot_info);

void VGA_get_display_size(int *x, int *y);

void VGA_copy_to_textram(int pos, void *src, int count); // will copy just to the end of textram ( pos -> number of staring charcter, src -> pinter to sorce, count -> number of characters to be copied

void VGA_set_cursor_position(int x, int y);

void VGA_disable_cursor();
void VGA_enable_cursor();