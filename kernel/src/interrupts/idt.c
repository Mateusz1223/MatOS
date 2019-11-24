#include "inc/interrupts/idt.h"

#include "inc/drivers/screen.h"


#define SETIDTDESCR(d, offset) { \
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

IDTEntry IDT[256];

IDTR ptr = {
		(uint16_t)((256 * 8) - 1),
		(uint32_t)&IDT
	};

void idt_init()
{
	PIC_remap(0x20, 0x28);
	IRQ_set_mask(0);
	
	for(int i=0; i<32; i++)
		SETIDTDESCR(IDT[i], int_handlers[i]);
	
	for(int i=0; i<16; i++)
		SETIDTDESCR(IDT[i+32], irq_handler);
	
	__asm("lidt [%0]" : : "r"(&ptr));

	screen_print("Interrupts initialized!\n");
}