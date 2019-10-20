#pragma once
#include "include/common.h"
#include "include/HAL.h"
#include "include/screen.h"
#include "include/idt.h"

unsigned char task;

unsigned char task_answer;

int CapsLock;
int NumLock;
int Shift;
int Ctrl;
int Alt;


int send_byte_to_keyboard(uint8_t byte);

void disable_keyboard_irq();

void enable_keyboard_irq();

void disable_keyboard_scanning();

void enable_keyboard_scanning();

void keyboard_set_ScanCodeSet(uint8_t scan_code_set);

int keyboard_get_ScanCodeSet();

char *keyboard_scan_input();

char getScancode();

char ScancodeToASCII(char c);

void keyboard_driver();