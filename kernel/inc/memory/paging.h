#pragma once
#include "inc/common.h"

void paging_init();

void paging_flush_tlb_entry( uint32_t addr );

// Not sure if usefull

/*bool paging_alloc_page( PTE *e ) // don't know whether useful

void paging_free_page( PTE *e ) // don't know whether useful

inline PTE *paging_ptable_lookup_entry( pTable *p, uint32_t addr )

inline PDE *paging_pdirectory_table_lookup_entry( pDirectoryTable *p, uint32_t addr )

void paging_set_pDirectoryTable( pDirectoryTable *dir )s 

void paging_map_page( void *phys, void *virt );*/