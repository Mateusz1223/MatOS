#include "include/common.h"
#include "include/screen.h"
#include "include/multiboot.h"

//definition of structure
typedef struct memory_map_entry {
	uint64_t baseAddress;
	uint64_t length;
	uint32_t type;
	uint32_t acpi_null;
} memory_map_entry;

//existing structure 
struct memory_map {
	int length;
	memory_map_entry *map;
} memory_map;

void memory_init(multiboot_info *bootinfo, int KernelBase, int KernelImgSize);

void *memory_alloc_block();

void memory_free_block(void* p);

void print_memory_map();