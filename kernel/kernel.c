#include "include/kernel.h"

void _start(multiboot_info* bootinfo, int KernelBase, int KernelImgSize)// KernelBase - ImageBase, KernelImgSize - SizeOfImage
{
	screen_init();
	idt_init();
	memory_init(bootinfo, KernelBase, KernelImgSize);
	
	//screen_print("%s\n", keyboard_scan_input());
		
	for(;;);
}
