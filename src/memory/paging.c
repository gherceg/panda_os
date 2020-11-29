
#include "paging.h"
#include "kheap.h"
#include "../tools.h"
#include "../screen/monitor.h"

page_directory_t *kernel_directory=0;
page_directory_t *current_directory=0;

// bitset of frames used/free
size_t *frames;
size_t frame_count;

extern size_t placement_address;
extern heap_t *kheap;
extern void copy_page_physical(size_t, size_t);

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(size_t frame_addr) {
    size_t frame = frame_addr/0x1000;
    size_t idx = INDEX_FROM_BIT(frame);
    size_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static void clear_frame(size_t frame_addr) {
    size_t frame = frame_addr/0x1000;
    size_t idx = INDEX_FROM_BIT(frame);
    size_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

static size_t test_frame(size_t frame_addr) {
    size_t frame = frame_addr/0x1000;
    size_t idx = INDEX_FROM_BIT(frame);
    size_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

static size_t first_frame() {
    size_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(frame_count); i++) {
        if (frames[i] != 0xFFFFFFFF) {
           
            for (j = 0; j < 32; j++) {
                size_t toTest = 0x1 << j;
                if ( !(frames[i]&toTest) ) {
                    return i*4*8+j;
                }
            }
        }
    }
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    if (page->frame != 0) {
        // frame already allocated
        return;
    } else {
        // find available frame, 
        size_t frame_address = first_frame();

        if (frame_address == (size_t)-1) {
            PANIC("No free frames!");
        }
        
        set_frame(frame_address*0x1000);
        page->present = 1;
        page->rw = (is_writeable)?1:0;
        page->user = (is_kernel)?0:1;
        page->frame = frame_address;
    }
}

void free_frame(page_t *page) {
    size_t frame;
    if (!(frame=page->frame)) {
        return;
    } else {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

void initialise_paging() {
    // specifies the size of memory
    // assume 16MB for now
    size_t mem_end_page = 0x1000000;
    
    frame_count = mem_end_page / 0x1000;
    frames = (size_t*) kmalloc(INDEX_FROM_BIT(frame_count));
    memset(frames, 0, INDEX_FROM_BIT(frame_count));

    size_t phys;
    kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physical_address_of_tables_physical = (size_t)kernel_directory->tables_physical;

    // map pages for the kernel heap addresses (creates page tables)
    // we cannot change the placement_address between identity mapping and enabling paging
    // so we create any page tables that need to be allocated for the heap here
    // and will allocate the pages after identity-mapping 
    int i = 0;
    for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000) {
        get_page(i, 1, kernel_directory);
    }

    // identity mapped paging: the virtual page addresses are equal to the physical frame addresses
    // we need to do this to access memory as if paging was not enabled (since it will not be enabled until switch_page_directory is called)
    i = 0;
    while (i < placement_address+0x1000) {
        // kernel pages are read-only from user mode
        alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    // now we can allocate frames for pages in heap address space
    for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000) {
        alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
    }

    // register for page fault interrupt
    register_interrupt_handler(14, page_fault);

    // enable paging
    switch_page_directory(kernel_directory);

    // we are ready to create the kernel heap
    kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);

    // current_directory = clone_directory(kernel_directory);  
    
    // switch_page_directory(current_directory); 
}

void switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3":: "r"(dir->physical_address_of_tables_physical));
    size_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // enables paging
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(size_t address, int make, page_directory_t *dir) {
    // create index from address
    size_t page_index = address / 0x1000;
   // find page table that contains this page index
    size_t table_index = page_index / 1024;
    if (dir->tables[table_index]) {
        // straightforward, return the page cause we have it
        return &dir->tables[table_index]->pages[page_index%1024];
    } else if (make) {
        // if make is set to 1, create the page table 
        size_t tmp;
        dir->tables[table_index] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
        // zero out the 4KB block
        memset(dir->tables[table_index], 0, 0x1000);
        // set flags 0b0111: present, read-write, user mode accessible
        dir->tables_physical[table_index] = tmp | 0x7;
        // return the page via the page table
        return &dir->tables[table_index]->pages[page_index%1024];
    } else {
        return 0;
    }
}


void page_fault(registers_t *regs) {
   
    size_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    int present = !(regs->err_code & 0x1);
    int rw = regs->err_code & 0x2;          
    int us = regs->err_code & 0x4;          
    int reserved = regs->err_code & 0x8;    
    int id = regs->err_code & 0x10;         

    monitor_write_sys("Page fault! ( ");
    monitor_write_hex(regs->err_code);
    if (present) {monitor_write_sys(" not present ");}
    if (rw) {monitor_write_sys("read-only ");}
    if (us) {monitor_write_sys("user-mode ");}
    if (reserved) {monitor_write_sys("reserved ");}
    monitor_write_sys(") at ");
    monitor_write_hex(faulting_address);
    monitor_write_sys(" - EIP: ");
    monitor_write_hex(regs->eip);
    monitor_write_sys("\n");
    PANIC("Page fault");
}

/**
 * Used when cloning a directory. Each table (and their pages/frames) need to cloned.
 */
static page_table_t *clone_table(page_table_t *src, size_t *physical_address) {
    // create a new blank page table
    page_table_t *table = (page_table_t*) kmalloc_ap(sizeof(page_table_t), physical_address);
    memset(table, 0, sizeof(page_directory_t));

    for (int i = 0; i < 1024; i++) {
        if (src->pages[i].frame) {

            alloc_frame(&table->pages[i], 0, 0);
    
            if (src->pages[i].present) table->pages[i].present = 1;
            if (src->pages[i].rw) table->pages[i].rw = 1;
            if (src->pages[i].user) table->pages[i].user = 1;
            if (src->pages[i].accessed) table->pages[i].accessed = 1;
            if (src->pages[i].dirty) table->pages[i].dirty = 1;
            // copy_page_physical lives in process.s
            // responsible for physically copying contents of one frame into another
            // NOTE: this involves disabling paging
            copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
        }
    }
    return table;
}

/**
 * Used when creating a new process since it needs a new page directory
 * Certain page tables need to be linked like the kernel in case there are changes that should apply to all processes
 * Other page tables need only be copied, which clone_table() is for
 */
page_directory_t *clone_directory(page_directory_t *src) {
    size_t physical_address;
    
    // create a new blank page directory
    page_directory_t *dir = (page_directory_t*) kmalloc_ap(sizeof(page_directory_t), &physical_address);
    memset(dir, 0, sizeof(page_directory_t));

    // now that a new page directory has been created, we need to set the physical address of the tables_physical member
    // this is needed to properly load the CR3 register
    size_t offset = (size_t)dir->tables_physical - (size_t)dir;
    dir->physical_address_of_tables_physical = physical_address + offset;

    // copy/link depending on the page table
    for (int i = 0; i < 1024; i++) {
        if (!src->tables[i])
            continue;

        // check if the page table is in the kernel directory
        if (kernel_directory->tables[i] == src->tables[i]) {
            // link to the kernel page table
            dir->tables[i] = src->tables[i];
            dir->tables_physical[i] = src->tables_physical[i];
        } else {
            // create a copy since it will be potentially modified
            dir->tables[i] = clone_table(src->tables[i], &physical_address);
            dir->tables_physical[i] = physical_address | 0x07;
        }
    }
    return dir;
}