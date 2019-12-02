#include "inc/drivers/keyboard.h"

#include "inc/HAL.h"
#include "inc/drivers/screen.h"
#include "inc/drivers/PIC.h"

#define TASK_NONEOPERATION 0x0
#define TASK_GETRESPONSE 0x1
#define TASK_SCAN 0x2

static struct KeyboardState
{
	int CapsLock;
	int NumLock;
	int Shift;
	int Ctrl;
	int Alt;
}Keyboard;

void disable_keyboard_irq()
{
	IRQ_set_mask(1);
}

void enable_keyboard_irq()
{
	IRQ_clear_mask(1);
}

char getScancode()
{
	while(1)
	{
		unsigned char c = inb(0x60);
		if(c != 0)
			return c;
	}
}

void keyboard_init()
{
	disable_keyboard_irq();
}

void keyboard_irq()
{
	getScancode();
	screen_set_color(RED);
	screen_print("Keybord interrupt.\n");
	screen_set_color(LIGH_GREEN);
}