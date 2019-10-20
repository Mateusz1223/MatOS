#pragma once

typedef struct multiboot_info {
	uint32_t flags;
	uint32_t memoryLo;
	uint32_t memoryHi;
	uint32_t bootDevice;
	uint32_t cmdLine;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms0;
	uint32_t syms1;
	uint32_t syms2;
	uint32_t mmap_length;
	void *mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t bootloader_name;
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;

	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
} multiboot_info;