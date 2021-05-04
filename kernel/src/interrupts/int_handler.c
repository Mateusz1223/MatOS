#include "inc/interrupts/int_handler.h"

#include "inc/UI/terminal.h"
#include "inc/UI/UIManager.h"
#include "inc/drivers/VGA.h" // for colors

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
	terminal_set_color(debugTerminal, RED);

	terminal_print(debugTerminal, "\nMESSAGE OF DEATH!\n");
	terminal_print(debugTerminal, "Interrupt number: %x\n", int_num);
	terminal_print(debugTerminal, "Interrupt reason: %s\n\n", int_reasons[int_num]);
	
	terminal_print(debugTerminal, "TrapFrame:\n");
	terminal_print(debugTerminal, "EAX: %x\n",frame->EAX);
	terminal_print(debugTerminal, "EBX: %x\n",frame->EBX);
	terminal_print(debugTerminal, "ECX: %x\n",frame->ECX);
	terminal_print(debugTerminal, "EDX: %x\n",frame->EDX);
	terminal_print(debugTerminal, "EDI: %x\n",frame->EDI);
	terminal_print(debugTerminal, "ESI: %x\n",frame->ESI);
	terminal_print(debugTerminal, "EBP: %x\n\n",frame->EBP);

	terminal_print(debugTerminal, "System halted");

	UI_manager_request_emergency_display_update(debugTerminal);
	
	for(;;);
}