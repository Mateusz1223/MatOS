#include "inc/common.h"
#include "inc/bootinfo.h"
#include "inc/drivers/VGA.h"
#include "inc/drivers/keyboard.h"
#include "inc/interrupts/idt.h"
#include "inc/memory/memory_menager.h"
#include "inc/drivers/timers/RTC.h"
#include "inc/drivers/timers/PIT.h"
#include "inc/drivers/system.h"

#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

#include "inc/drivers/busses/ATA.h"

void print_welcome_sign();

void _start(bootinfo* bootInfo){
	VGA_init(bootInfo);
	UI_manager_init();
	memory_init(bootInfo);
	idt_init();
	keyboard_init();
	RTC_init();
	PIT_init();
	
	//ATA_check();
	//ATA_init(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

	system_init();

	print_welcome_sign();

	for(;;);
}

void print_welcome_sign(){
	terminal_set_color(debugTerminal, BRIGHT_WHITE);;
	terminal_print(debugTerminal, "\n\n           Welcome to\n\n");
	terminal_set_color(debugTerminal, BROWN);
	terminal_print(debugTerminal, "           MMMM      MMMM           tt      OOOOO     SSSSSSS   !!!\n");
	terminal_print(debugTerminal, "           MM MM    MM MM           tt    OO     OO  SS      S  !!!\n");
	terminal_print(debugTerminal, "           MM  MM  MM  MM         tttttt OO       OO   SS       !!!\n");
	terminal_print(debugTerminal, "           MM   MMMM   MM   aaaa    tt   OO       OO    SS      !!!\n");
	terminal_print(debugTerminal, "           MM    MM    MM  aa  aa   tt   OO       OO     SS        \n");
	terminal_print(debugTerminal, "           MM          MM  aa  aa   tt   OO       OO      SS     ! \n");
	terminal_print(debugTerminal, "           MM          MM  aa  aa   tt    OO     OO   S     SS  !!!\n");
	terminal_print(debugTerminal, "           MM          MM   aaaa a   ttt    OOOOO      SSSSSS    ! \n");
	terminal_print(debugTerminal, "\n\n");
	terminal_set_color(debugTerminal, BRIGHT_WHITE);
	terminal_print(debugTerminal, "           https://github.com/Mateusz1223/MatOS\n");
	terminal_print(debugTerminal, "           Mateusz Piasecki https://piaseckimateusz.pl/\n\n\n\n");
	terminal_print(debugTerminal, "                         ");
	terminal_set_color(debugTerminal, BACKGROUND_WHITE | BLINKING);
	terminal_print(debugTerminal, "Press F9 to open terminal...\n\n\n");
	terminal_set_color(debugTerminal, LIGHT_GREEN);
}