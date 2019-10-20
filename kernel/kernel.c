#include "include/kernel.h"

void _start(multiboot_info* bootinfo)
{
	screen_init();
	idt_init();
	memory_init(bootinfo);
	//ATA_check();
	
	//screen_print("%s\n", keyboard_scan_input());
	//screen_print("\tHello_World!! %d \r", 125);
		
	for(;;);
}
