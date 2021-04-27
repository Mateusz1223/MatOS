#include "inc/drivers/busses/ATA.h"

#include "inc/UI/terminal.h"

//http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio

void ATA_check()
{
	terminal_print(debugTerminal, "ATA check:\n");

	outb(0x1F6, 0xA0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	uint8_t tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		terminal_print(debugTerminal, "Primary master exists\n");
	}else
	{
		terminal_print(debugTerminal, "Primary master does not exists\n");
	}

	outb(0x1F6, 0xB0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		terminal_print(debugTerminal, "Primary slave exists\n");
	}else
	{
		terminal_print(debugTerminal, "Primary slave does not exists\n");
	}




	outb(0x176, 0xA0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		terminal_print(debugTerminal, "Secondary master exists\n");
	}else
	{
		terminal_print(debugTerminal, "Secondary master does not exists\n");
	}

	outb(0x176, 0xB0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		terminal_print(debugTerminal, "Secondary slave exists\n");
	}else
	{
		terminal_print(debugTerminal, "Secondary slave does not exists\n\n");
	}
}