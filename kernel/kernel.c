#include "include/kernel.h"

void _start(multiboot_info* bootinfo)
{
	screen_init();
	idt_init();
	memory_init(bootinfo);
	
	//screen_print("%s\n", keyboard_scan_input());
		
	for(;;);
}
