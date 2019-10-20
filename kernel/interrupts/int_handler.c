#include "include/int_handler.h"

char *int_reasons[32] ={
	"Divide Error",
	"Debug Exception",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode (Undefined Opcode)",
	"Device Not Available (No Math Coprocessor)",
	"Double Fault",
	" Coprocessor Segment Overrun (reserved)",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection",
	"Page Fault",
	"Intel reserved. I have no idea how it happend!!!!",
	"x87 FPU Floating-Point Error (Math Fault)",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
	"Intel reserved. I have no idea how it happend!!!!",
};

void interrupt_handler(uint32_t int_num, TrapFrame *frame)
{
	screen_clear();
	screen_set_color(0x4);

	screen_print("Interrupt number: %x\n", int_num);
	screen_print("Interrupt reason: %s\n\n", int_reasons[int_num]);
	
	screen_print("TrapFrame:\n");
	screen_print("EAX: %x\n",frame->EAX);
	screen_print("EBX: %x\n",frame->EBX);
	screen_print("ECX: %x\n",frame->ECX);
	screen_print("EDX: %x\n",frame->EDX);
	screen_print("EDI: %x\n",frame->EDI);
	screen_print("ESI: %x\n",frame->ESI);
	screen_print("EBP: %x\n\n",frame->EBP);

	screen_print("System halted");
	
	for(;;);
}

void pic_handler()
{
	uint16_t isr = pic_get_isr();
	uint8_t isr1 = isr >> 8;
	uint8_t isr2 = isr << 8;
	int irq_num;
	
	if(isr2 == 0)
		irq_num = isr1 - 1;
	else
		irq_num = isr2 + 7;
	
	if(irq_num == 1)
	{
		keyboard_driver();
	}
	
	PIC_sendEOI(irq_num);
}