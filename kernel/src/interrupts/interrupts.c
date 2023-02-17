#include "inc/interrupts/interrupts.h"

void disable_interrupts(){
	__asm("cli");
}

void enable_interrupts(){
	__asm("sti");
}