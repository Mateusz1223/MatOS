#pragma once

typedef struct bootinfo {
	uint32_t mmap_length;
	void 	*mmap_addr;
	uint32_t kernel_base;
	uint32_t kernel_img_size;
	uint16_t BIOS_equipment_list;
} bootinfo;

/*           BIOS_equipment_list

	Bit       Description
	  ---------------------

	  0         Set if any floppy disk drives present

	  1         Set if math coprocessor installed

	  2         Set if pointing device attached (PS/2)

	  3-2       System board RAM size (only for original IBM PC, PCjr):

	                00 = 16K
	                01 = 32K
	                10 = 48K
	                11 = 64K

	  5-4        Initial video mode:

	                00 = reserved
	                01 = 40-column color
	                10 = 80-column color
	                11 = 80-column monochrome

	  7-6        Number of floppy disk drives (if bit 0 set):

	                00 = 1 drive
	                01 = 2 drives
	                10 = 3 drives
	                11 = 4 drives

	  8         Reserved

	  11-9      Number of serial ports

	  12        Set if game adapter installed

	  13        Set if serial printer attached (PCjr)
	            Set it internal modem installed (PC and XT only)

	  15-14     Number of parallel ports
*/