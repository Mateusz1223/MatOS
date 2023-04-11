#include "inc/memory/heap.h"

#include "inc/UI/terminal.h"
#include "inc/UI/UIManager.h"
#include "inc/drivers/VGA.h"

static struct HeapStructure{
	uint8_t *addr;
	size_t size;
} Heap;

typedef struct HeapEntry HeapEntry; // ?????

struct HeapEntry{
	uint8_t *prev;
	bool first;
	size_t size; // without HeapEntry itself
	bool free;
}__attribute__((packed)); // make sure it's not alligned by compiler

//_________________________________________________________________________________________

static void debug_print_entries(){
	terminal_set_color(debugTerminal, RED);
	terminal_print(debugTerminal, "Heap entries:\n");
	uint8_t *it = Heap.addr;
	while(it < Heap.addr + Heap.size)
	{
		HeapEntry *entry = (HeapEntry *)it;

		uint8_t *prev;
		if(entry->prev == 0)
			prev = 0;
		else
			prev = (uint8_t *)(entry->prev - Heap.addr);
		
		terminal_print(debugTerminal, "Location: %x, prev: %x, first: %b, size: %x, free: %b\n", it - Heap.addr, prev, entry->first, entry->size, entry->free);

		it += sizeof(HeapEntry);
		it += entry->size;
	}
	terminal_set_color(debugTerminal, LIGHT_GREEN);
}

//_________________________________________________________________________________________

void heap_init(void *addr, size_t size) // size in bytes
{
	Heap.addr = (uint8_t *)addr;
	Heap.size = size;

	HeapEntry *firstEntry = (HeapEntry *)addr;

	firstEntry->prev = 0;
	firstEntry->first = true;
	firstEntry->size = size - sizeof(HeapEntry);
	firstEntry->free = true;

	terminal_print(debugTerminal, "Heap initialized at %x, size: %x\n", Heap.addr, Heap.size);

	//debug_print_entries();
}

void *heap_malloc(size_t size){
	uint8_t *it = Heap.addr;

	while(it < (uint8_t *)(Heap.addr + Heap.size)){
		HeapEntry *entry = (HeapEntry *)it;
		if(entry->free && entry->size >= size){
			entry->free = false;

			if(entry->size - size > sizeof(HeapEntry)){
				HeapEntry *newEntry = (HeapEntry *)(it + sizeof(HeapEntry) + size);
				newEntry->prev = (uint8_t *)entry;
				newEntry->first = false;
				newEntry->size = entry->size - size - sizeof(HeapEntry);
				newEntry->free = true;

				HeapEntry *nextEntry = (HeapEntry *)(it + sizeof(HeapEntry) + entry->size);
				if(nextEntry < (HeapEntry *)(Heap.addr + Heap.size)) // next entry exists
					nextEntry->prev = (uint8_t *)newEntry;

				entry->size = size;
			}

			//debug_print_entries();

			return (void *)(it + sizeof(HeapEntry));
		}
		else{ // next entry
			it += sizeof(HeapEntry);
			it += entry->size;
		}
	}

	terminal_set_color(debugTerminal, LIGHT_RED);
	terminal_print(debugTerminal, "[Heap error] Heap full. Cannot allocate memory\n");
	terminal_set_color(debugTerminal, LIGHT_GREEN);

	return 0; // Error, sufficient memory block hasn't been found
}

void heap_free(void *addr){ // Errors to be debuged
	uint8_t *ad = (uint8_t *)addr;
	ad -= sizeof(HeapEntry);
	HeapEntry *entry = (HeapEntry *)ad;
	entry->free = true;

	HeapEntry *nextEntry = (HeapEntry *)(ad + sizeof(HeapEntry) + entry->size);
	if(nextEntry < (HeapEntry *)(Heap.addr + Heap.size) && nextEntry->free){
		HeapEntry *nextNextEntry = (HeapEntry *)((uint8_t *)nextEntry + sizeof(HeapEntry) + nextEntry->size);
		entry->size = entry->size + sizeof(HeapEntry) + nextEntry->size;
		nextNextEntry->prev = nextEntry->prev;
	}

	HeapEntry *prevEntry = (HeapEntry *)entry->prev;
	if(!entry->first && prevEntry->free){
		nextEntry = (HeapEntry *)(ad + sizeof(HeapEntry) + entry->size);
		prevEntry->size = prevEntry->size + sizeof(HeapEntry) + entry->size;
		nextEntry->prev = entry->prev;
	}

	//debug_print_entries();
}