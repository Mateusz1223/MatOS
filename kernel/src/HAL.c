#include "inc/HAL.h"

void outb(uint16_t port, uint8_t data){
 	__asm("out dx, al" : : "a" (data), "d" (port));
}

void outw(uint16_t port, uint16_t data){
	__asm("out dx, ax" : : "a" (data), "d" (port));
}

void outd(uint16_t port, uint32_t data){
	__asm("out dx, eax" : : "a" (data), "d" (port));
}

uint8_t inb(uint16_t port){
	__asm("in al, dx" : : "d" (port));
}

uint16_t inw(uint16_t port){
	__asm("in ax, dx" : : "d" (port));
}

uint32_t ind(uint16_t port){
	__asm("in eax, dx" : : "d" (port));
}

void insd(uint16_t port, uint32_t *buffer, int n){
    for(int i=0; i<n; i++)
    	buffer[i] = ind(port);
}

void io_wait(void){
    __asm( "jmp 1f\n\t"
           "1:jmp 2f\n\t"
           "2:" );
}