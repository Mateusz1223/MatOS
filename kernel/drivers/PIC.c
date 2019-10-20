#include "include/PIC.h"

//https://wiki.osdev.org/8259_PIC

#define PIC1_COMMAND	0x20
#define PIC1_DATA	0x21
#define PIC2_COMMAND	0xA0
#define PIC2_DATA	0xA1

#define PIC_EOI		0x20		// End-of-interrupt command code

void PIC_sendEOI(unsigned char irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND, PIC_EOI);
	
	outb(PIC1_COMMAND, PIC_EOI);
}
 
#define ICW4_8086	0x01		// 8086/88 (MCS-80/85) mode  

void PIC_remap(int offset1, int offset2)
{ 
	uint8_t mask1 = inb(PIC1_DATA);
	uint8_t mask2 = inb(PIC2_DATA);
	
	outb(PIC1_COMMAND, 0x11);	// starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, 0x11);
	io_wait();
	
	outb(PIC1_DATA, offset1);	// ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);	// ICW2: Slave PIC vector offset
	io_wait();
	
	outb(PIC1_DATA, 4);    // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);    // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
	
	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);
}

void IRQ_set_mask(unsigned char IRQline)
{
	uint16_t port;
	uint8_t value;
	
	if(IRQline >=8)
	{
		port = PIC2_DATA;
		IRQline -=8;
	}
	else{
		port = PIC1_DATA;
	}
	value = inb(port) | (1 << IRQline);
	outb(port, value);
}

void IRQ_clear_mask(unsigned char IRQline)
{
	uint16_t port;
	uint8_t value;
	
	if(IRQline >=8)
	{
		port = PIC2_DATA;
		IRQline -=8;
	}
	else{
		port = PIC1_DATA;
	}
	value = inb(port) & ~(1 << IRQline);
	outb(port, value);
}


#define PIC_READ_IRR                0x0a    // OCW3 irq ready next CMD read
#define PIC_READ_ISR                0x0b

static uint16_t __pic_get_irq_reg(int ocw3)
{
	outb(PIC1_COMMAND, ocw3);
	outb(PIC2_COMMAND, ocw3);
	return (inb(PIC1_COMMAND) << 8) | inb(PIC2_COMMAND);
}

uint16_t pic_get_irr(void)
{
	return __pic_get_irq_reg(PIC_READ_IRR);
}

uint16_t pic_get_isr(void)
{
	return __pic_get_irq_reg(PIC_READ_ISR);
}