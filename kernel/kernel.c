#include "include/kernel.h"

void _start(bootinfo* boot_info)
{
	screen_init(boot_info);
	idt_init();
	memory_init(boot_info);

	screen_print("\nnBIOS_equipment_list: %x", boot_info->BIOS_equipment_list);
	
	//screen_print("%s\n", keyboard_scan_input());
		
	for(;;);
}
