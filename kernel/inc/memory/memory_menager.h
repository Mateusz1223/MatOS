//https://wiki.osdev.org/Detecting_Memory_(x86)

#pragma once
#include "inc/common.h"
#include "inc/bootinfo.h"

void memory_init(bootinfo *boot_info);

// those functions allows to allocate SINGLE block of 4096 bytes will be used to allocate pages for processes
void *memory_alloc_block();
void memory_free_block(void* p);