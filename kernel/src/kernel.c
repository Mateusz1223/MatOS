#include "inc/common.h"
#include "inc/bootinfo.h"
#include "inc/drivers/VGA.h"
#include "inc/drivers/keyboard.h"
#include "inc/interrupts/idt.h"
#include "inc/drivers/memory_menager.h"
#include "inc/UI/UIManager.h"

#include "inc/drivers/busses/ATA.h"

void _start(bootinfo* boot_info)
{
	VGA_init(boot_info);
	UI_manager_init();

	idt_init();
	memory_init(boot_info);
	keyboard_init();

	//ATA_check();

	UI_manager_run();
}
