#include "include/memory_menager.h"

//AMM- allocation_memory_map
// 8 blocks per byte
#define AMM_BLOCKS_PER_BYTE 8

// block size (4k)
#define AMM_BLOCK_SIZE	4096

// block alignment
#define AMM_BLOCK_ALIGN	PMMNGR_BLOCK_SIZE

//128 KiB for allocation memory map (4GB)
static uint32_t *allocation_memory_map = (uint32_t *)0x00100000; //0x00100000-0x00EFFFFF	0x00E00000 (14 MiB)	RAM -- free for use (if it exists)	Extended memory 1, 2

// size of physical memory
static	uint32_t memory_size=0;

// number of blocks currently in use
static	uint32_t used_blocks=0;

// maximum number of available memory blocks
static	uint32_t max_blocks=0;



static void amm_set (int bit) {
 
  allocation_memory_map[bit / 32] |= (1 << (bit % 32)); // should be in the other site i think
}



static void print_memory_map_entry(memory_map_entry *map_entry)
{
	screen_print("%x | ", map_entry->baseAddress);
	screen_print("%x | ", map_entry->length);
	screen_print("%x\n", map_entry->type);

	//screen_print("%x | %x | %x\n\n",map_entry->baseAddress, map_entry->length, map_entry->type); //doesn't work to be fixed
}

void memory_init(multiboot_info *bootinfo)
{
	memory_map.lenght = bootinfo->mmap_length;
	memory_map.map = bootinfo->mmap_addr;

	print_memory_map();

	memset(allocation_memory_map, 0xff, 131072);
	//screen_print("\n%x", allocation_memory_map[1]);

	allocation_memory_map[0] = 0;
	amm_set(0);
	screen_print("\n%x", allocation_memory_map[0]);
}

void print_memory_map()
{
	screen_set_color(BACKGROUND_GREEN);
	screen_print("Memory Map:\n");
	screen_set_color(LIGH_GREEN);
	screen_print("Lenght: %x\nAddress: %x\n\nEntries:\n", memory_map.lenght, memory_map.map);
	screen_print("Base | Lenght | Type\n\n");

	int c = memory_map.lenght/24;

	for(int i=0; i<c; i++)
	{
		print_memory_map_entry(&memory_map.map[i]);
	}
}




