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
	int lenght;
	memory_map_entry *map;
} memory_map;

void memory_init(multiboot_info *bootinfo);

void print_memory_map();