#include "inc/common.h"
#include "inc/bootinfo.h"
#include "inc/drivers/screen.h"
#include "inc/drivers/keyboard.h"
#include "inc/interrupts/idt.h"
#include "inc/drivers/memory_menager.h"

void _start(bootinfo* boot_info)
{
	screen_init(boot_info);
	screen_print("...\n");
	idt_init();
	screen_print("...\n");
	memory_init(boot_info);
	screen_print("\n");

	keyboard_init();//disables keyboard interrupts

	screen_set_color(BACKGROUND_RED);
	screen_print("This kernel is under development and doesn't\nprovide any functionalities yet. :-(");
	screen_disable_cursor();
		
	for(;;);
}
