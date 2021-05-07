#include "inc/memory/paging.h"

#include "inc/UI/terminal.h"
#include "inc/drivers/VGA.h"

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

//_________________________________________________________________________________________


//              --------PTE--------
/*static void PTE_set_present_flag(PTE *p, bool b)
{
	if(b)
		*p |= 1;
	else
		*p ~= 1;
}

static void PTE_set_rw_flag(PTE *p, bool b) // Read/Write flag set
{
	if(b)
		*p |= 2;
	else
		*p ~= 2;
}

static void PTE_set_usm_flag(PTE *p, bool b) // User/Supervisor mode flag set
{
	if(b)
		*p |= 4;
	else
		*p ~= 4;
}

static void PTE_set_adress(PTE *p, uint32_t address)
{
	address &= 0xfffff000; 
	*p |= address;
}

//              --------PDE--------

static void PDE_set_present_flag(PDE *p, bool b)
{
	if(b)
		*p |= 1;
	else
		*p ~= 1;
}

static void PDE_set_rw_flag(PDE *p, bool b) // Read/Write flag set
{
	if(b)
		*p |= 2;
	else
		*p ~= 2;
}

static void PDE_set_usm_flag(PDE *p, bool b) // User/Supervisor mode flag set
{
	if(b)
		*p |= 4;
	else
		*p ~= 4;
}

static void PDE_set_pwt_flag(PDE *p, bool b) // Write-through flag set
{
	if(b)
		*p |= 8;
	else
		*p ~= 8;
}

static void PDE_set_pcd_flag(PDE *p, bool b) // Cache disabled flag set
{
	if(b)
		*p |= 16;
	else
		*p ~= 16;
}

static void PDE_set_ps_flag(PDE *p, bool b) // Page Size flag set
{
	if(b)
		*p |= 128;
	else
		*p ~= 128;
}

static void PDE_set_adress(PDE *p, uint32_t address) 
{
	address &= 0xfffff000; 
	*p |= address;
}*/


//_________________________________________________________________________________________