
#ifndef KHEAP_H
#define KHEAP_H

#include "../tools.h"
#include "../utils/ordered_array.h"
#include "paging.h"
#include <stddef.h>

/**
 * The kernel heap is necessary for dynamic memory allocation
 * Prior to implementing this, the OS uses the placement_address to write to unused memory
 * This is sufficient for allocating memory, but provides no mechanism for deallocation, hence the need for the kernel heap
 */

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000 // 1MB

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB // a random number used to easily identity the header/footer part of memory
#define HEAP_MIN_SIZE     0x70000

typedef struct {
    size_t magic; // header/footer identifier
    uint8 is_hole; // 0 is block, 1 if hole
    size_t size; // includes header/footer/actual block
} header_t;

typedef struct {
    size_t magic; // header/footer identifier
    header_t *header; // pointer to the header_t struct
} footer_t;

typedef struct {
    ordered_array_t index;
    size_t start_address;
    size_t end_address;
    size_t max_address;
    uint8 supervisor;
    uint8 readonly;
} heap_t;

heap_t *create_heap(size_t start, size_t end, size_t max, uint8 supervisor, uint8 readonly);

void *alloc(size_t size, uint8 page_align, heap_t *heap);

void free(void *p, heap_t *heap);

size_t kmalloc_int(size_t sz, int align, size_t *phys);

// ensure allocated memory is page aligned
size_t kmalloc_a(size_t sz);

// returns physical address
size_t kmalloc_p(size_t sz, size_t *phys);

// returns physical address and page aligned
size_t kmalloc_ap(size_t sz, size_t *phys);

// not page aligned
size_t kmalloc(size_t sz);

void kfree(void *p);

#endif