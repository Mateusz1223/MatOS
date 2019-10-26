#include "include/memory_menager.h"

//AMM- allocation_memory_map
// 8 blocks per byte
#define AMM_BLOCKS_PER_BYTE 8

// block size (4k)
#define AMM_BLOCK_SIZE	4096

// block alignment
#define AMM_BLOCK_ALIGN	AMM_BLOCK_SIZE

//128 KiB for allocation memory map (4GB)
static uint32_t *allocation_memory_map = (uint32_t *)0x00100000; //0x00100000-0x00EFFFFF	0x00E00000 (14 MiB)	RAM -- free for use (if it exists)	Extended memory 1, 2

// size of physical memory
//static	uint32_t memory_size=0;

// number of blocks currently in use
static	uint32_t used_blocks=0;

// maximum number of available memory blocks
static	uint32_t max_blocks=0;


/*static size_t get_memory_size()
{
	return memory_size;
}*/

static uint32_t get_block_count()
{
	return max_blocks;
}

static uint32_t get_used_block_count()
{
	return used_blocks;
}

static uint32_t get_free_block_count()
{
	return max_blocks - used_blocks;
}

static uint32_t get_block_size()
{
	return AMM_BLOCK_SIZE;
}


static void amm_set(int bit)
{
	allocation_memory_map[bit / 32] |= (1 << (bit % 32));
}

static void amm_unset(int bit)
{
 	allocation_memory_map[bit / 32] &= ~ (1 << (bit % 32));
}

static void amm_set_multiple(int base, int limit)
{
 	for(int i=base; i<base+limit; i++)
 		amm_set(i);
}

static void amm_unset_multiple(int base, int limit)
{
 	for(int i=base; i<base+limit; i++)
		amm_unset(i);
}

static bool amm_test(int bit)
{
 	return allocation_memory_map[bit / 32] &  (1 << (bit % 32));
}

static int amm_first_free() 
{
	for (uint32_t i=0; i< get_block_count() / 32; i++)
		if (allocation_memory_map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (allocation_memory_map[i] & bit) )
					return i*32+j;
			}

	return -1;
}

static void init_regions()
{
	for(int i=0; i<memory_map.length; i++)
	{
		if(memory_map.map[i].type==1 && memory_map.map[i].baseAddress>0x000FFFFF)
		{
			int base = memory_map.map[i].baseAddress/AMM_BLOCK_SIZE;
			if(memory_map.map[i].baseAddress%AMM_BLOCK_SIZE != 0)
				base++;
			int limit = memory_map.map[i].length/AMM_BLOCK_SIZE;

			amm_unset_multiple(base, limit);
			used_blocks = used_blocks-limit;
		}
	}
}

static void alocate_kernel_executable(int KernelBase, int KernelImgSize)
{
	int base = KernelBase/AMM_BLOCK_SIZE;
	int limit = KernelImgSize/AMM_BLOCK_SIZE;
	if(KernelImgSize%AMM_BLOCK_SIZE != 0)
		limit++;

	amm_set_multiple(base, limit);
	used_blocks = used_blocks+limit;
}


static void print_memory_map_entry(memory_map_entry *map_entry)
{
	screen_print("%x | ", map_entry->baseAddress);
	screen_print("%x | ", map_entry->length);
	screen_print("%x\n", map_entry->type);

	//screen_print("%x | %x | %x\n\n",map_entry->baseAddress, map_entry->length, map_entry->type); //doesn't work to be fixed
}



void memory_init(bootinfo *boot_info)
{
	memory_map.length = boot_info->mmap_length;  //1 047 488; 0x3fef0  vs 1 046 464‬
	memory_map.map = boot_info->mmap_addr;

	print_memory_map();

	memset(allocation_memory_map, 0xff, 131072);

	//memory_size=0;
	max_blocks = 1048576;
	used_blocks = get_block_count();

	init_regions();

	alocate_kernel_executable(boot_info->kernel_base, boot_info->kernel_img_size);

	screen_print("\nFree blocks: %d", get_free_block_count());
}

void *memory_alloc_block()
{
	if (get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = amm_first_free();
 
	if (frame == -1)
		return 0;	//out of memory
 
	amm_set(frame);
 
	uint32_t addr = frame * AMM_BLOCK_SIZE;

	used_blocks++;
 
	return (void*)addr;
}


void memory_free_block(void* p)
{
	uint32_t addr = (uint32_t)p;
	int frame = addr / AMM_BLOCK_SIZE;
 
	amm_unset(frame);

	used_blocks--;
}

void print_memory_map()
{
	screen_set_color(BACKGROUND_GREEN);
	screen_print("Memory Map:\n");
	screen_set_color(LIGH_GREEN);
	screen_print("length: %x\nAddress: %x\n\nEntries:\n", memory_map.length, memory_map.map);
	screen_print("Base | length | Type\n\n");

	int c = memory_map.length/24;

	for(int i=0; i<c; i++)
	{
		print_memory_map_entry(&memory_map.map[i]);
	}
}




