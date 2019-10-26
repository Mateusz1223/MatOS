#pragma once

typedef struct bootinfo {
	uint32_t mmap_length;
	void 	*mmap_addr;
	uint32_t kernel_base;
	uint32_t kernel_img_size;
} bootinfo;