#include "inc/drivers/PIC.h"

#include "inc/HAL.h"

#include "inc/drivers/timers/PIT.h"
#include "inc/drivers/keyboard.h"
#include "inc/drivers/timers/RTC.h"

#include "inc/UI/terminal.h"

#include "inc/interrupts/interrupts.h"

//https://wiki.osdev.org/8259_PIC

#define PIC1_COMMAND	0x20
#define PIC1_DATA	0x21
#define PIC2_COMMAND	0xA0
#define PIC2_DATA	0xA1

#define PIC_EOI		0x20		// End-of-interrupt command code

#define ICW4_8086	0x01		// 8086/88 (MCS-80/85) mode

#define PIC_READ_IRR                0x0a    // OCW3 irq ready next CMD read
#define PIC_READ_ISR                0x0b

//_____________________________________________________________________

static uint16_t pic_get_irq_reg(int ocw3){
	outb(PIC1_COMMAND, ocw3);
	outb(PIC2_COMMAND, ocw3);
	return (inb(PIC1_COMMAND) << 8) | inb(PIC2_COMMAND);
}

static void PIC_remap(int offset1, int offset2){ 
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

//_______________________________________________________________________

void PIC_init(){
	PIC_remap(0x20, 0x28);

	for(int i=0; i<=15; i++) // Mask PIC interrupts
		IRQ_set_mask(i);

	IRQ_clear_mask(2); // Enable Slave PIC

	terminal_print(debugTerminal, "[X] PIC ready!\n");
}

void PIC_sendEOI(uint8_t irq){
	if(irq >= 8)
		outb(PIC2_COMMAND, PIC_EOI);
	
	outb(PIC1_COMMAND, PIC_EOI);
}

void IRQ_set_mask(uint8_t IRQline){
	uint16_t port;
	uint8_t value;
	
	if(IRQline >=8){
		port = PIC2_DATA;
		IRQline -=8;
	}
	else{
		port = PIC1_DATA;
	}
	value = inb(port) | (1 << IRQline);
	outb(port, value);
}

void IRQ_clear_mask(uint8_t IRQline){
	uint16_t port;
	uint8_t value;
	
	if(IRQline >= 8){
		port = PIC2_DATA;
		IRQline -=8;
	}
	else{
		port = PIC1_DATA;
	}
	value = inb(port) & ~(1 << IRQline);
	outb(port, value);
}

uint16_t pic_get_irr(void){
	return pic_get_irq_reg(PIC_READ_IRR);
}

uint16_t pic_get_isr(void){
	return pic_get_irq_reg(PIC_READ_ISR);
}

void PIC_handler(int irq_num){
	disable_interrupts();
	//terminal_print(debugTerminal, "IRQ num: %d\n", irq_num);

	switch( irq_num ){
	case 0:
		PIT_irq();
		break;
		
	case 1:
	    keyboard_irq();
	    break;
	   
	case 8:
	    RTC_irq();
	    break;

	// Watch out for spurious interrupts 7 and probably 15 for slave isr is not set, must not send EOI in case of 7, in case of 15 EOI needs to be sent to Master !!! https://wiki.osdev.org/PIC -> Spurious IRQs
	   
	default:
	    ;
	    break;
	}
	/*
	0	Programmable Interrupt Timer Interrupt
	1	Keyboard Interrupt
	2	Cascade (used internally by the two PICs. never raised)
	3	COM2 (if enabled)
	4	COM1 (if enabled)
	5	LPT2 (if enabled)
	6	Floppy Disk
	7	LPT1 / Unreliable "spurious" interrupt (usually)
	8	CMOS real-time clock (if enabled)
	9	Free for peripherals / legacy SCSI / NIC
	10	Free for peripherals / SCSI / NIC
	11	Free for peripherals / SCSI / NIC
	12	PS2 Mouse
	13	FPU / Coprocessor / Inter-processor
	14	Primary ATA Hard Disk
	15	Secondary ATA Hard Disk
	*/
	
	enable_interrupts();
	PIC_sendEOI(irq_num);
}