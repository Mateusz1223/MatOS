#include "inc/memory/memory_menager.h"

#include "inc/memory/heap.h"
#include "inc/memory/paging.h"

#include "inc/UI/terminal.h"
#include "inc/drivers/VGA.h"

/*
Memory structure
________________
0 
	stack at 0x0007fff0
0x?????? (info in memory map)
	Reserved
0x100000
	Allocation Memory Map (1/8 MiB)
0x120000
	Heap (2 7/8 MiB)
0x400000
	Kernel
0x4????? (info in boot_info structure)
	Pages
0x?????? (info in memory map)
... Not important
*/

//AMM - allocation memory map

// block size (4k)
#define AMM_PAGE_SIZE	4096

#define AMM_SIZE 1048576 / 8 // in bytes

//definition of structure
struct MemoryMapEntry{
	uint64_t baseAddress;
	uint64_t length;
	uint32_t type; // 1 -> Free Memory, 2 -> Reserved Memory, 3 -> ACPI reclaimable memory, 4 -> ACPI NVS memory. 5 -> Area containing bad memory
	uint32_t acpiNull;
} __attribute__((packed));

typedef struct MemoryMapEntry MemoryMapEntry;

//existing structure 
struct MemoryMap{
	int length;
	MemoryMapEntry *map;
} __attribute__((packed)) MemoryMap;

struct AMMStruct {
	// 1 MiB for allocation memory map (4GB)
	uint32_t *addr;
	// index of first block after reserved memory (after kernel)
	int lastReservedBlock;	
	// number of blocks currently in use
	uint32_t usedBlocks;
	// maximum number of available memory blocks
	uint32_t maxBlocks; // AMM_SIZE * 8 * 8 * AMM_PAGE_SIZE = 4 294 967 296 (4 GB)
}AMM;

//_________________________________________________________________________________________

static uint32_t get_free_block_count(){
	return AMM.maxBlocks - AMM.usedBlocks;
}

static void amm_set(int bit){
	AMM.addr[bit / 32] |= (1 << (bit % 32));
}

static void amm_unset(int bit){
 	AMM.addr[bit / 32] &= ~ (1 << (bit % 32));
}

static void amm_set_multiple(int base, size_t count){ //to be speed up
 	for(int i=base; i<base+count; i++)
 		amm_set(i);
}

static void amm_unset_multiple(int base, size_t count){ //to be speed up
 	for(int i=base; i<base+count; i++)
		amm_unset(i);
}

static bool amm_test(int bit){
 	return AMM.addr[bit / 32] &  (1 << (bit % 32));
}

static int amm_first_free(){
	for(uint32_t i=0; i<AMM.maxBlocks / 32; i++)
		if (AMM.addr[i] != 0xffffffff)
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (AMM.addr[i] & bit) )
					return i*32+j;
			}

	return -1;
}

static void init_regions(){
	for(int i=0; i<MemoryMap.length; i++){
		if(MemoryMap.map[i].type==1 && MemoryMap.map[i].baseAddress>0x000FFFFF){ // make sure it is free for use section and it is not section 0 - 0x9f000 (reserved for stack)
			int base = MemoryMap.map[i].baseAddress/AMM_PAGE_SIZE;
			if(MemoryMap.map[i].baseAddress%AMM_PAGE_SIZE != 0)
				base++;
			int count = MemoryMap.map[i].length/AMM_PAGE_SIZE;

			amm_unset_multiple(base, count);
			AMM.usedBlocks = AMM.usedBlocks-count;
		}
	}
}

static void reserve_multiple(int addr, size_t size){
	int base = addr/AMM_PAGE_SIZE;
	int count = size/AMM_PAGE_SIZE;
	if(count * AMM_PAGE_SIZE < size)
		count++;

	amm_set_multiple(base, count);
	AMM.usedBlocks += count;
}

static void print_memory_map_entry(MemoryMapEntry *map_entry){
	terminal_print(debugTerminal, "%x | ", map_entry->baseAddress);
	terminal_print(debugTerminal, "%x | ", map_entry->length);
	terminal_print(debugTerminal, "%x\n", map_entry->type);

	//terminal_print(debugTerminal, "%x | %x | %x\n\n",map_entry->baseAddress, map_entry->length, map_entry->type); //doesn't work to be fixed
}

static void print_memory_map(){
	terminal_print(debugTerminal, "_____________________________\n");
	terminal_set_color(debugTerminal, BACKGROUND_GREEN);
	terminal_print(debugTerminal, "Memory Map:\n");
	terminal_set_color(debugTerminal, LIGHT_GREEN);
	terminal_print(debugTerminal, "length: %x\nAddress: %x\n\nEntries:\n", MemoryMap.length, MemoryMap.map);
	terminal_print(debugTerminal, "Base | length | Type\n\n");

	int c = MemoryMap.length/24;

	for(int i=0; i<c; i++)
		print_memory_map_entry(&MemoryMap.map[i]);

	terminal_print(debugTerminal, "_____________________________\n");
}

//_________________________________________________________________________________________


void memory_init(bootinfo *bootInfo){
	// Memory map init
	MemoryMap.length = bootInfo->mmap_length;  //1 047 488; 0x3fef0  vs 1 046 464‬
	MemoryMap.map = bootInfo->mmap_addr;

	print_memory_map();

	// AMM init

	AMM.addr = (uint32_t *)0x00100000; // 0x00100000-0x00EFFFFF	0x00E00000 (14 MiB)	RAM -- free for use (if it exists)	Extended memory 1, 2
	AMM.maxBlocks = AMM_SIZE * 8;
	memsetk(AMM.addr, 0xff, AMM.maxBlocks); // mark every page as used

	AMM.usedBlocks = AMM.maxBlocks;

	init_regions();

	reserve_multiple(0x100000, AMM_SIZE); // Allocate AMM
	reserve_multiple(0x100000 + AMM_SIZE, 0x400000 - 0x100000 - AMM_SIZE); // Allocate Heap
	reserve_multiple(bootInfo->kernel_base, bootInfo->kernel_img_size); // Allocate Kernel
	AMM.lastReservedBlock = (bootInfo->kernel_base + bootInfo->kernel_img_size) / AMM_PAGE_SIZE;

	// heap initialization
	heap_init((void *)(0x100000 + AMM_SIZE + 0x10), (size_t)(0x400000 - 0x100000 - AMM_SIZE - 0x10)); // 0x10 for security reasons

	// debug print
	terminal_print(debugTerminal, "Kernel loaded at %x, size of kernel: %d B\n", bootInfo->kernel_base, bootInfo->kernel_img_size);
	terminal_print(debugTerminal, "Stack at 0x0007FFF0\n"); 

	terminal_print(debugTerminal, "Free blocks: %d (%d MiB)\n", get_free_block_count(), get_free_block_count() * AMM_PAGE_SIZE / 1048576); // 1048576 size of 1 MB in bytes

	paging_init();

	terminal_print(debugTerminal, "[X] Memory ready!\n");
}

void *memory_alloc_block(){
	if (get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = amm_first_free();
 
	if (frame == -1)
		return 0;	//out of memory
 
	amm_set(frame);
 
	uint32_t addr = frame * AMM_PAGE_SIZE;

	AMM.usedBlocks++;
 
	return (void*)addr;
}


void memory_free_block(void* p){
	uint32_t addr = (uint32_t)p;
	int frame = addr / AMM_PAGE_SIZE;
 
	amm_unset(frame);

	AMM.usedBlocks--;
}