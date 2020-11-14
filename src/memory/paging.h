
#ifndef PAGING_H
#define PAGING_H

#include "../tools.h"
#include "../interrupts/isr.h"

typedef struct page {
    uint32 present    : 1; 
    uint32 rw         : 1; 
    uint32 user       : 1; 
    uint32 accessed   : 1; 
    uint32 dirty      : 1; 
    uint32 unused     : 7; 
    uint32 frame      : 20;
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {

    page_table_t *tables[1024];

    uint32 tablesPhysical[1024];

    uint32 physicalAddr;
} page_directory_t;

void initialise_paging();

void switch_page_directory(page_directory_t *new);

page_t *get_page(uint32 address, int make, page_directory_t *dir);

void page_fault(registers_t regs);

page_directory_t *clone_directory(page_directory_t *src);

void alloc_frame(page_t *page, int is_kernel, int is_writeable);

void free_frame(page_t *page);

#endif
