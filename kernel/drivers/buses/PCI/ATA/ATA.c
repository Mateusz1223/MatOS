#include "include/ATA.h"

//http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio

void ATA_check()
{
	screen_print("ATA check:\n");

	outb(0x1F6, 0xA0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	uint8_t tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		screen_print("Primary master exists\n");
	}else
	{
		screen_print("Primary master does not exists\n");
	}

	outb(0x1F6, 0xB0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		screen_print("Primary slave exists\n");
	}else
	{
		screen_print("Primary slave does not exists\n");
	}




	outb(0x176, 0xA0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		screen_print("Secondary master exists\n");
	}else
	{
		screen_print("Secondary master does not exists\n");
	}

	outb(0x176, 0xB0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x1F7); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
	{
		screen_print("Secondary slave exists\n");
	}else
	{
		screen_print("Secondary slave does not exists\n\n");
	}
}