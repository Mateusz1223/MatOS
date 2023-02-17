#include "inc/interrupts/idt.h"

#include "inc/interrupts/interrupts.h"
#include "inc/interrupts/int_asm_handlers.h"
#include "inc/drivers/PIC.h"

#include "inc/UI/terminal.h"

struct IDTEntry{
   uint16_t offset_0_15; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t flags; // type and attributes, see below
   uint16_t offset_16_31; // offset bits 16..31
}__attribute__((packed));

struct IDTR{
	uint16_t limit;
	uint32_t base;
}__attribute__((packed));

typedef struct IDTEntry IDTEntry;
typedef struct IDTR IDTR;


#define SETIDTDESCR(d, offset){ \
	d.offset_0_15 = ((uint32_t)offset & 0xffff); \
	d.selector = 0x8; \
	d.zero = 0; \
	d.flags = 0x8E; \
	d.offset_16_31 = (((uint32_t)offset >> 16) & 0xffff); \
}

void *int_handlers[32] = {
	int_0_handler,
	int_1_handler,
	int_2_handler,
	int_3_handler,
	int_4_handler,
	int_5_handler,
	int_6_handler,
	int_7_handler,
	int_8_handler,
	int_9_handler,
	int_10_handler,
	int_11_handler,
	int_12_handler,
	int_13_handler,
	int_14_handler,
	int_15_handler,
	int_16_handler,
	int_17_handler,
	int_18_handler,
	int_19_handler,
	int_20_handler,
	int_21_handler,
	int_22_handler,
	int_23_handler,
	int_24_handler,
	int_25_handler,
	int_26_handler,
	int_27_handler,
	int_28_handler,
	int_29_handler,
	int_30_handler,
	int_31_handler,
};

void *irq_handlers[16] = {
	irq_0_handler,
    irq_1_handler,
    irq_2_handler,
    irq_3_handler,
    irq_4_handler,
    irq_5_handler,
    irq_6_handler,
    irq_7_handler,
    irq_8_handler,
    irq_9_handler,
    irq_10_handler,
	irq_11_handler,
	irq_12_handler,
	irq_13_handler,
	irq_14_handler, 
	irq_15_handler,
  };

IDTEntry IDT[256];

IDTR ptr = {
		(uint16_t)((256 * 8) - 1),
		(uint32_t)&IDT
	};

void idt_init(){
	PIC_init();

	for(int i=0; i<32; i++)
		SETIDTDESCR(IDT[i], int_handlers[i]);
	
	for(int i=0; i<16; i++)
		SETIDTDESCR(IDT[i+32], irq_handlers[i]);

	//Rest of ISR like:
	//SETIDTDESCR(IDT[48], API_isr);
	
	__asm("lidt %0" : : "m"(ptr));

	enable_interrupts();

	terminal_print(debugTerminal, "IDT loaded!\n");
}