
#ifndef PAGING_H
#define PAGING_H

#include "../tools.h"
#include "../interrupts/isr.h"
#include <stddef.h>

/**
 * The Memory Managemment Unit (MMU) is responsible for mapping the virtual address space to the physical address space
 * This can be achieved through either segmenting or paging. Paging is the modern approach.
 * On 32 bit x86 architecture, a page is a 4KB block in the virtual address space mapped to a frame in the physical address space
 * 32 bit x86 supports 32 bit addressing for up to 4GB of virtual memory
 * 64 bit x86 supports 48 bit addressing for up to 256 TB of virtual memory
 */

typedef struct page {
    size_t present    : 1; // page present in memory
    size_t rw         : 1; // read-only if clear, read-write if set
    size_t user       : 1; // kernel mode if clear
    size_t accessed   : 1; // accessed since last refresh
    size_t dirty      : 1; // written to since last refresh
    size_t unused     : 7; // unused/reserved bits
    size_t frame      : 20; // frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {

    page_table_t *tables[1024];
    // physical addresses of page tables, useful for loading into CR3 register
    size_t tables_physical[1024];
    // useful once paging is enabled and the directory may be in a different location in virtual memory
    size_t physical_address_of_tables_physical;
} page_directory_t;

/**
 * sets up environment, page directories, and enables paging
 */
void initialise_paging();

/**
 * Load the new page directory into the CR3 register
 */
void switch_page_directory(page_directory_t *new);

/**
 * Retrieve page for address
 * If make == 1, create page if it does not already exist
 */
page_t *get_page(size_t address, int make, page_directory_t *dir);

/**
 * Handler for page faults
 */
void page_fault(registers_t *regs);

page_directory_t *clone_directory(page_directory_t *src);

void alloc_frame(page_t *page, int is_kernel, int is_writeable);

void free_frame(page_t *page);

#endif
