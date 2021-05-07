#include "inc/drivers/timers/PIT.h"

#include "inc/HAL.h"
#include "inc/interrupts/interrupts.h"
#include "inc/drivers/PIC.h"

#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

// Base frequency = 1193182

static struct PITStruct
{
	unsigned long long millisSinceInit;

	unsigned long long UIResidentLastCall;

}PIT;

//_________________________________________________________________________________________

static void set_divisor(uint16_t div)
{
	disable_interrupts();
 
	// Set low byte
	outb(0x40,(uint8_t)(div & 0xFF));		// Low byte
	io_wait();
	outb(0x40,(uint8_t)((div & 0xFF00) >> 8));	// High byte

	enable_interrupts();
}

static void disable_PIC_irq()
{
	IRQ_set_mask(0);
}

static void enable_PIC_irq()
{
	IRQ_clear_mask(0);
}

//_________________________________________________________________________________________

void PIT_init()
{
	PIT.millisSinceInit = 0;
	PIT.UIResidentLastCall = 0;

	set_divisor(1193 * 5); // frequency = 100

	enable_PIC_irq();

	terminal_print(debugTerminal, "PIT ready!\n");
}

unsigned long long PIT_millis()
{
	return PIT.millisSinceInit;
}

void PIT_irq()
{
	// Tick
	PIT.millisSinceInit += 5; // frequency = 500

	if(PIT.millisSinceInit - PIT.UIResidentLastCall > 40) // 25 fps
	{
		PIT.UIResidentLastCall = PIT.millisSinceInit;
		UI_manager_PIT_irq_resident();
	}

	//terminal_print(debugTerminal, "PIT int, millis: %u\n", (int)PIT.millisSinceInit);
}