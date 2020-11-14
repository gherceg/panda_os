
#ifndef KHEAP_H
#define KHEAP_H

#include "../tools.h"
#include "../utils/ordered_array.h"
#include "paging.h"

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x70000

typedef struct {
    uint32 magic;  
    uint8 is_hole;  
    uint32 size;
} header_t;

typedef struct {
    uint32 magic;    
    header_t *header;
} footer_t;

typedef struct {
    ordered_array_t index;
    uint32 start_address;
    uint32 end_address;
    uint32 max_address;
    uint8 supervisor;
    uint8 readonly;
} heap_t;

heap_t *create_heap(uint32 start, uint32 end, uint32 max, uint8 supervisor, uint8 readonly);

void *alloc(uint32 size, uint8 page_align, heap_t *heap);

void free(void *p, heap_t *heap);

uint32 kmalloc_int(uint32 sz, int align, uint32 *phys);

uint32 kmalloc_a(uint32 sz);

uint32 kmalloc_p(uint32 sz, uint32 *phys);

uint32 kmalloc_ap(uint32 sz, uint32 *phys);

uint32 kmalloc(uint32 sz);

void kfree(void *p);

#endif