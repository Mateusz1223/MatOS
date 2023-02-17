#include "inc/common.h"
#include "inc/bootinfo.h"
#include "inc/drivers/VGA.h"
#include "inc/drivers/keyboard.h"
#include "inc/interrupts/idt.h"
#include "inc/memory/memory_menager.h"
#include "inc/drivers/timers/RTC.h"
#include "inc/drivers/timers/PIT.h"

#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

#include "inc/drivers/busses/ATA.h"

void _start(bootinfo* bootInfo){
	VGA_init(bootInfo);
	UI_manager_init();

	idt_init();
	memory_init(bootInfo);
	keyboard_init();
	RTC_init();
	PIT_init();

	//ATA_check();
	//ATA_init();

	for(;;);
}
