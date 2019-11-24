#include "inc/kernel.h"

void _start(bootinfo* boot_info)
{
	screen_init(boot_info);
	screen_print("...\n");
	idt_init();
	screen_print("...\n");
	memory_init(boot_info);
	screen_print("\n");
		
	for(;;);
}
