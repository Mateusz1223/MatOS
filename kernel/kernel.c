#include "include/kernel.h"

void _start(bootinfo* boot_info)// KernelBase - ImageBase, KernelImgSize - SizeOfImage
{
	screen_init();
	idt_init();
	memory_init(boot_info);
	
	//screen_print("%s\n", keyboard_scan_input());
		
	for(;;);
}
