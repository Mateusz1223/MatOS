#include "inc/drivers/VGA.h"

#include "inc/HAL.h"

static struct VGAStruct
{
	unsigned char *textram;

	int width;
	int height;

	uint16_t IO_Port_RegisterIndex;
	uint16_t IO_Port_DataRegister;
}VGA;

//___________________________________________________________________________________________________

static void clear()
{
	int n = VGA.height*VGA.width*2;
	int i;
	for(i=0; i<n; i+=2)
	{
		VGA.textram[i] = ' ';
		VGA.textram[i+1] = LIGHT_GREEN;
	}

	VGA_set_cursor(0, 0, LIGHT_GREEN);
}

//___________________________________________________________________________________________________

void VGA_init(bootinfo* bootInfo)
{
	VGA.height = 25;
	if((bootInfo->BIOS_equipment_list & 0x30)>>4 == 0x2)
		VGA.width = 80;
	else if((bootInfo->BIOS_equipment_list & 0x30)>>4 == 0x1)
		VGA.width = 40;

	VGA.textram = (unsigned char *)0xb8000;
	VGA.IO_Port_RegisterIndex = 0x3d4;
	VGA.IO_Port_DataRegister = 0x3d5;

	clear();
}

void VGA_get_display_size(int *x, int *y)
{
	*x = VGA.width;
	*y = VGA.height;
}

void VGA_copy_to_textram(int pos, void *src, int count) // ( pos -> number of staring charcter, src -> pinter to sorce, count -> number of characters to be copied
{
	void *dest = &VGA.textram[2*pos]; // ??????

	if(pos + count > VGA.height * VGA.width) // make sure it wont override any further than textram
		count = VGA.height * VGA.width - pos + 1;

	memmove(dest, src, count*2); // count*2 because one character in textram consist of 2 bytes - character and color
}

void VGA_set_cursor(int x, int y, unsigned char color)
{
	uint16_t pos = y * VGA.width + x;
	
	outb(VGA.IO_Port_RegisterIndex, 0xf);
	outb(VGA.IO_Port_DataRegister, (uint8_t)(pos & 0xff));
	
	outb(VGA.IO_Port_RegisterIndex, 0xe);
	outb(VGA.IO_Port_DataRegister, (uint8_t)((pos >> 8) & 0xff));

	VGA.textram[2*pos+1] = color;
}

void VGA_disable_cursor()
{
	outb(VGA.IO_Port_RegisterIndex, 0x0A);
	outb(VGA.IO_Port_DataRegister, 0x20);
}

void VGA_enable_cursor()
{
	outb(VGA.IO_Port_RegisterIndex, 0x0A);
	outb(VGA.IO_Port_DataRegister, (inb(0x3D5) & 0xC0) | 14);
 
	outb(VGA.IO_Port_RegisterIndex, 0x0B);
	outb(VGA.IO_Port_DataRegister, (inb(0x3E0) & 0xE0) | 17);
}