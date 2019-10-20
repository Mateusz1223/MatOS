#pragma once
#include "include/common.h"
#include "include/PIC.h"
#include "include/keyboard.h"

typedef struct TrapFrame {
	uint32_t EDI;
	uint32_t ESI;
	uint32_t EBP;
	uint32_t ESP;
	uint32_t EBX;
	uint32_t EDX;
	uint32_t ECX;
	uint32_t EAX;
} TrapFrame;

void interrupt_handler(uint32_t int_num, TrapFrame *frame);

void pic_handler();