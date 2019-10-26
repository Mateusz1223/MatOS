[bits 16]

struc bootinfo
	.mmap_length		resd	1 ; memory map. Present if flags[6] is set
	.mmap_addr			resd	1
	.kernel_base		resd	1
	.kernel_img_size	resd	1
endstruc