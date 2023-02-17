#include "inc/memory/paging.h"

#include "inc/memory/memory_menager.h"
#include "inc/interrupts/interrupts.h"

#include "inc/UI/terminal.h"
#include "inc/drivers/VGA.h"

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

//! page sizes are 4k
//! page table represents 4mb address space
//! directory table represents 4gb address space

typedef uint32_t PTE; // Page Table Entry
/*
Bit 0 (P): Present flag
	0: Page is not in memory
	1: Page is present (in memory)
Bit 1 (R/W): Read/Write flag
	0: Page is read only
	1: Page is writable
Bit 2 (U/S):User mode/Supervisor mode flag
	0: Page is kernel (supervisor) mode
	1: Page is user mode. Cannot read or write supervisor pages
Bits 3-4 (RSVD): Reserved by Intel
Bit 5 (A): Access flag. Set by processor
	0: Page has not been accessed
	1: Page has been accessed
Bit 6 (D): Dirty flag. Set by processor
	0: Page has not been written to
	1: Page has been written to
Bits 7-8 (RSVD): Reserved
Bits 9-11 (AVAIL): Available for use
Bits 12-31 (FRAME): Frame address
*/

typedef uint32_t PDE; // Page Directory Entry

/*
Bit 0 (P): Present flag
	0: Page is not in memory
	1: Page is present (in memory)
Bit 1 (R/W): Read/Write flag
	0: Page is read only
	1: Page is writable
Bit 2 (U/S):User mode/Supervisor mode flag
	0: Page is kernel (supervisor) mode
	1: Page is user mode. Cannot read or write supervisor pages
Bit 3 (PWT):Write-through flag
	0: Write back caching is enabled
	1: Write through caching is enabled
Bit 4 (PCD):Cache disabled
	0: Page table will not be cached
	1: Page table will be cached
Bit 5 (A): Access flag. Set by processor
	0: Page has not been accessed
	1: Page has been accessed
Bit 6 (D): Reserved by Intel
Bit 7 (PS): Page Size
	0: 4 KB pages
	1: 4 MB pages
Bit 8 (G): Global Page (Ignored)
Bits 9-11 (AVAIL): Available for use
Bits 12-31 (FRAME): Page Table Base address
*/

// page table
struct pTable{
	PTE entries[1024];
} __attribute__((packed)); // make sure it's not alligned by compiler

typedef struct pTable pTable;
 
// page directory
struct pDirectoryTable{
	PDE entries[1024];
} __attribute__((packed)); // make sure it's not alligned by compiler

typedef struct pDirectoryTable pDirectoryTable;

static struct PagingStruct{
	pDirectoryTable *currPDirectoryTable;
} Paging;

//_________________________________________________________________________________________


//              --------PTE--------
static void PTE_set_present_flag(PTE *p, bool b){
	if(b)
		*p |= 1;
	else
		*p &= (uint32_t)(0xffffffff - 1);
}

inline static bool PTE_is_present(PTE p){
	return p & 1;
}

static void PTE_set_rw_flag(PTE *p, bool b) // Read/Write flag set
{
	if(b)
		*p |= 2;
	else
		*p &= (uint32_t)(0xffffffff - 2);
}

static void PTE_set_usm_flag(PTE *p, bool b) // User/Supervisor mode flag set
{
	if(b)
		*p |= 4;
	else
		*p &= (uint32_t)(0xffffffff - 4);
}

static void PTE_set_adress(PTE *p, uint32_t address){
	address &= (uint32_t)0xfffff000;
	*p &= (uint32_t)0x00000fff;
	*p |= address;
}

static uint32_t PTE_get_adress(PTE p){
	return p & (uint32_t)0xfffff000;
}

//              --------PDE--------

static void PDE_set_present_flag(PDE *p, bool b){
	if(b)
		*p |= 1;
	else
		*p &= (uint32_t)(0xffffffff - 1);
}

inline static bool PDE_is_present(PDE p){
	return p & 1;
}

static void PDE_set_rw_flag(PDE *p, bool b) // Read/Write flag set
{
	if(b)
		*p |= 2;
	else
		*p &= (uint32_t)(0xffffffff - 2);
}

static void PDE_set_usm_flag(PDE *p, bool b) // User/Supervisor mode flag set
{
	if(b)
		*p |= 4;
	else
		*p &= (uint32_t)(0xffffffff - 4);
}

static void PDE_set_pwt_flag(PDE *p, bool b) // Write-through flag set
{
	if(b)
		*p |= 8;
	else
		*p &= (uint32_t)(0xffffffff - 8);
}

static void PDE_set_pcd_flag(PDE *p, bool b) // Cache disabled flag set
{
	if(b)
		*p |= 16;
	else
		*p &= (uint32_t)(0xffffffff - 16);
}

static void PDE_set_ps_flag(PDE *p, bool b) // Page Size flag set
{
	if(b)
		*p |= 128;
	else;
		*p &= (uint32_t)(0xffffffff - 128);
}

static void PDE_set_adress(PDE *p, uint32_t address) 
{
	address &= (uint32_t)0xfffff000;
	*p &= (uint32_t)0x00000fff;
	*p |= address;
}

static uint32_t PDE_get_adress(PDE p){
	return p & (uint32_t)0xfffff000;
}

static void enable_paging(){
	uint32_t cr0;

	__asm("mov %%cr3, %0" : : "r" (Paging.currPDirectoryTable)); // __asm("mov %0, %%cr3" : : "r" (Paging.currPDirectoryTable)); is a wrong order. It surprises me as I thought it uses Source before the destination syntax. 

	__asm("mov %0, %%cr0" : "=r" (cr0)); // same problem as above
	cr0 |= 0x80000001;
	__asm("mov %%cr0, %0" : : "r" (cr0)); // same problem as above
}

//_________________________________________________________________________________________

void paging_init(){
	// 0-4 MiB (identity mapped)
	pTable *table1 = (pTable *)memory_alloc_block();
	if (!table1){
		terminal_print(debugTerminal, "paging.c, paging_initialize(): initialization failed - no space for Paging Table 1");
		return;
	}
	memsetk(table1, (uint8_t)0x00, sizeof(pTable));
 
	// 4 MiB - 8MiB (identity mapped)
	pTable *table2 = (pTable *)memory_alloc_block();
	if (!table2){
		terminal_print(debugTerminal, "paging.c, paging_initialize(): initialization - failed no space for Paging Table 2");
		return;
	}
	memsetk(table2, (uint8_t)0x00, sizeof(pTable));

	// 1st 4mb are idenitity mapped
	for (int i=0, frame=0x0; i<1024; i++, frame+=4096){
 		// create a new page
		PTE page = 0;
		PTE_set_adress(&page, frame);
		PTE_set_present_flag(&page, true); // present
		PTE_set_rw_flag(&page, true); // writable
		PTE_set_usm_flag(&page, false); // supervisor mode

		// ...and add it to the page table
		table1->entries[i] = page;
	}

	// 2nd 4mb are also idenitity mapped
	for (int i=0, frame=0x400000; i<1024; i++, frame+=4096){
 		// create a new page
		PTE page = 0;
		PTE_set_adress (&page, frame);
		PTE_set_present_flag(&page, true); // present
		PTE_set_rw_flag(&page, true); // writable
		PTE_set_usm_flag(&page, false); // supervisor mode

		// ...and add it to the page table
		table2->entries[i] = page;
	}

	// create default directory table
	pDirectoryTable *dir = (pDirectoryTable *)memory_alloc_block(); // memory_alloc_block() returns 4096 alligned pointer
	if (!dir){
		terminal_print(debugTerminal, "paging.c, paging_initialize(): initialization failed no space for Paging Directory Table");
		return;
	}
	memsetk(dir, 0, sizeof(pDirectoryTable));

	PDE *entry = &dir->entries[0];
	PDE_set_adress(entry, (uint32_t)table1);
	PDE_set_present_flag(entry, true);
	PDE_set_rw_flag(entry, true);
	PDE_set_usm_flag(entry, false); // supervisor mode

	entry = &dir->entries[1];
	PDE_set_adress(entry, (uint32_t)table2);
	PDE_set_present_flag(entry, true);
	PDE_set_rw_flag(entry, true);
	PDE_set_usm_flag(entry, false); // supervisor mode

	Paging.currPDirectoryTable = dir;
	enable_paging();

	terminal_print(debugTerminal, "Paging initialized and enabled!\n");
}

void paging_flush_tlb_entry( uint32_t addr ){
	disable_interrupts();
	__asm("invlpg %0" : : "m"(addr));
	enable_interrupts();
}

// Not sure if usefull

/*bool paging_alloc_page( PTE *e ) // don't know whether useful
{
	// allocate a free physical frame
	void *p = memory_alloc_block ();
	if (!p)
		return false;
 
	// map it to the page
	PTE_set_adress(e, (uint32_t)p);
	PTE_set_present_flag(e, true);
 
	return true;
}

void paging_free_page( PTE *e ) // don't know whether useful
{
	void *p = (void*)PTE_get_adress(*e);
	if (p)
		memory_free_block(p);
 
	PTE_set_present_flag(e, false);
}

inline PTE *paging_ptable_lookup_entry( pTable *p, uint32_t addr ){
	if (p)
		return &p->entries[ PAGE_TABLE_INDEX(addr) ];
	return 0;
}

inline PDE *paging_pdirectory_table_lookup_entry( pDirectoryTable *p, uint32_t addr ){
	if (p)
		return &p->entries[ PAGE_TABLE_INDEX(addr) ];
	return 0;
}

void paging_map_page( void *phys, void *virt ){
	// get page directory
	pDirectoryTable *pageDirectory = Paging.currPDirectoryTable;

	// get page table
	PDE *entry = &pageDirectory->entries[PAGE_DIRECTORY_INDEX((uint32_t) virt)];
	if(!(PDE_is_present(*entry)))
	{
		// page table not present, allocate it
		pTable *table = (pTable*) memory_alloc_block();
		if (!table)
		{
		   terminal_print(debugTerminal, "paging.c, paging_map_page(): mapping page failed!");
		   return;
		}

		// clear page table
		memsetk(table, 0, sizeof(pTable));

		// map in the table (Can also just do *entry |= 3) to enable these bits
		PDE_set_present_flag(entry, true);
		PDE_set_rw_flag(entry, true); // read/write
		PDE_set_adress(entry, (uint32_t)&table);
	}

	// get table
	pTable *table = (pTable*) PAGE_GET_PHYSICAL_ADDRESS(entry); // physical adress of PDE points to pTable

	// get page
	PTE *page = &table->entries[PAGE_TABLE_INDEX((uint32_t)virt)];

	// map it in (Can also do (*page |= 3 to enable..)
	PTE_set_adress(page, (uint32_t)phys);
	PTE_set_present_flag(page, true);
	PTE_set_rw_flag(page, true); // writable
	PTE_set_usm_flag(page, false); // supervisor mode
}*/