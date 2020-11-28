#include "kheap.h"
#include "paging.h"
#include "../screen/monitor.h"

#define PAGE_SIZE 0x1000

extern uint32 end;
uint32 placement_address = (uint32) &end;
extern page_directory_t *kernel_directory;
heap_t *kheap=0;

uint32 kmalloc_int(uint32 sz, int align, uint32 *phys) {
    if (kheap != 0) {
        void *addr = alloc(sz, (uint8)align, kheap);
        if (phys) {
            page_t *page = get_page((uint32)addr, 0, kernel_directory);
            // bitshifted right by 12 bits, move back
            uint32 frame_address = page->frame << 12;
            // TODO: figure out why this would be needed
            uint32 unknown_offset = ((uint32)addr)&0xFFF;
            *phys = frame_address + unknown_offset;
        }
        return (uint32)addr;
    } else {
        if (align == 1 && (placement_address & 0x00000FFF) != 0) {
            // align the address if not already
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }
        if (phys) {
            *phys = placement_address;
        }
        uint32 tmp = placement_address;
        placement_address += sz;
        return tmp;
    }
}

void kfree(void *p) {
    free(p, kheap);
}

uint32 kmalloc_a(uint32 sz) {
    return kmalloc_int(sz, 1, 0);
}

uint32 kmalloc_p(uint32 sz, uint32 *phys) {
    return kmalloc_int(sz, 0, phys);
}

uint32 kmalloc_ap(uint32 sz, uint32 *phys) {
    return kmalloc_int(sz, 1, phys);
}

uint32 kmalloc(uint32 sz) {
    return kmalloc_int(sz, 0, 0);
}

static void expand(uint32 new_size, heap_t *heap) {
    ASSERT(new_size > heap->end_address - heap->start_address);

    if ((new_size&0xFFFFF000) != 0) {
        new_size &= 0xFFFFF000;
        new_size += 0x1000;
    }

    ASSERT(heap->start_address+new_size <= heap->max_address);

    uint32 old_size = heap->end_address-heap->start_address;

    uint32 i = old_size;
    while (i < new_size) {
        alloc_frame(get_page(heap->start_address+i, 1, kernel_directory), (heap->supervisor)?1:0, (heap->readonly)?0:1);
        i += 0x1000 /* page size */;
    }

    heap->end_address = heap->start_address+new_size;
}

static uint32 contract(uint32 new_size, heap_t *heap) {
    ASSERT(new_size < heap->end_address-heap->start_address);

    if (new_size&0x1000) {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    uint32 old_size = heap->end_address-heap->start_address;
    uint32 i = old_size - 0x1000;
    while (new_size < i) {
        free_frame(get_page(heap->start_address+i, 0, kernel_directory));
        i -= 0x1000;
    }

    heap->end_address = heap->start_address + new_size;
    return new_size;
}

static int32 find_smallest_hole(uint32 size, uint8 page_align, heap_t *heap) {
    uint32 iterator = 0;
    while (iterator < heap->index.size) {
        header_t *header = (header_t *)lookup_ordered_array(iterator, &heap->index);
        
        if (page_align > 0) {
       
            uint32 location = (uint32)header;
            int32 offset = 0;
            // check if the start of usable memory is page aligned (exclude the header)
            if ( (location + sizeof(header_t) & 0x00000FFF) != 0)
                // page size - (empty space needed to properly page align)
                offset = 0x1000 - ((location + sizeof(header_t)) % 0x1000);
            
            int32 hole_size = (int32)header->size - offset;

            if (hole_size >= (int32)size)
                break;
        } else if (header->size >= size)
            break;
        
        iterator++;
    }
    if (iterator == heap->index.size)
        return -1;
    else
        return iterator;
}

static int8 header_t_less_than(void*a, void *b) {
    return (((header_t*)a)->size < ((header_t*)b)->size)?1:0;
}

heap_t *create_heap(uint32 start, uint32 end_addr, uint32 max, uint8 supervisor, uint8 readonly) {
    heap_t *heap = (heap_t*)kmalloc(sizeof(heap_t));

    // assumptions are based on the heap start and end address being page aligned, so make sure they are
    ASSERT(start%0x1000 == 0);
    ASSERT(end_addr%0x1000 == 0);
    
    // create heap index
    heap->index = place_ordered_array( (void*)start, HEAP_INDEX_SIZE, &header_t_less_than);
    
    // shift start address forward to where we can start putting data
    start += sizeof(type_t)*HEAP_INDEX_SIZE;

    // ensure it is page aligned
    if ((start & 0x00000FFF) != 0) {
        start &= 0xFFFFF000;
        start += 0x1000;
    }

    heap->start_address = start;
    heap->end_address = end_addr;
    heap->max_address = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    header_t *hole = (header_t *)start;
    hole->size = end_addr-start;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array((void*)hole, &heap->index);     

    return heap;
}

void *alloc(uint32 size, uint8 page_align, heap_t *heap) {
    // add size of header and footer to size to allocate
    uint32 new_size = size + sizeof(header_t) + sizeof(footer_t);
    // find the smallest hole to allocate the new page(s) in
    int32 iterator = find_smallest_hole(new_size, page_align, heap);

    // if no hole was found
    if (iterator == -1) {
   
        uint32 old_length = heap->end_address - heap->start_address;
        uint32 old_end_address = heap->end_address;

        expand(old_length+new_size, heap);
        uint32 new_length = heap->end_address-heap->start_address;

        iterator = 0;
   
        uint32 idx = -1; uint32 value = 0x0;
        while (iterator < heap->index.size) {
            uint32 tmp = (uint32)lookup_ordered_array(iterator, &heap->index);
            if (tmp > value) {
                value = tmp;
                idx = iterator;
            }
            
            iterator++;
        }
   
        if (idx == -1) {
            header_t *header = (header_t *)old_end_address;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;
            footer_t *footer = (footer_t *) (old_end_address + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void*)header, &heap->index);
        } else {
       
            header_t *header = lookup_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;
       
            footer_t *footer = (footer_t *) ( (uint32)header + header->size - sizeof(footer_t) );
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
   
        return alloc(size, page_align, heap);
    }

    // get the header for the index retrieved from find_smallest_hole
    header_t *orig_hole_header = (header_t *)lookup_ordered_array(iterator, &heap->index);
    uint32 orig_hole_pos = (uint32)orig_hole_header;
    uint32 orig_hole_size = orig_hole_header->size;
    
    // check that we are using the hole efficiently 
    if (orig_hole_size - new_size < sizeof(header_t) + sizeof(footer_t)) {
        // if we are, add the remaining empty space to the originally requested size
        size += orig_hole_size - new_size;
        // new_size should equal the entire size of the hole
        new_size = orig_hole_size;
    }

    // if page align is needed, do so now and make a hole in front of the block
    if (page_align && orig_hole_pos&0x00000FFF) {
        // go to the next page boundary, then subtract the size of the header to determine new location
        uint32 new_location = orig_hole_pos&0xFFFFF000 + PAGE_SIZE - sizeof(header_t);

        // create a hole at the old location and update its values (mainly size)
        header_t *hole_header = (header_t *)orig_hole_pos;
        // hole_header->size     = PAGE_SIZE - (orig_hole_pos&0xFFFFF000) - sizeof(header_t);
        hole_header->size = new_location - orig_hole_pos;
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        footer_t *hole_footer = (footer_t *) ( (uint32)new_location - sizeof(footer_t) );
        hole_footer->magic    = HEAP_MAGIC;
        hole_footer->header   = hole_header;

        // update position and size with new values for page-aligned hole
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->size;
    } else {
        // the hole can be removed from the heap->index
        remove_ordered_array(iterator, &heap->index);
    }

    // now we can setup our new block
    header_t *block_header  = (header_t *)orig_hole_pos;
    block_header->magic     = HEAP_MAGIC;
    block_header->is_hole   = 0;
    block_header->size      = new_size;
    
    footer_t *block_footer  = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
    block_footer->magic     = HEAP_MAGIC;
    block_footer->header    = block_header;

    
    if (orig_hole_size - new_size > 0) {
        header_t *hole_header = (header_t *) (orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        hole_header->size     = orig_hole_size - new_size;
        footer_t *hole_footer = (footer_t *) ( (uint32)hole_header + orig_hole_size - new_size - sizeof(footer_t) );
        
        if ((uint32)hole_footer < heap->end_address) {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }
   
        insert_ordered_array((void*)hole_header, &heap->index);
    }
    
    return (void *) ( (uint32)block_header+sizeof(header_t) );
}

void free(void *p, heap_t *heap) {
    if (p == 0)
        return;

    header_t *header = (header_t*) ( (uint32)p - sizeof(header_t) );
    footer_t *footer = (footer_t*) ( (uint32)header + header->size - sizeof(footer_t));

    ASSERT(header->magic == HEAP_MAGIC);
    ASSERT(footer->magic == HEAP_MAGIC);

    header->is_hole = 1;

    char do_add = 1;

    footer_t *test_footer = (footer_t*) ( (uint32)header - sizeof(footer_t));
    if (test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole == 1) {
        uint32 cache_size = header->size;
        header = test_footer->header;
        footer->header = header;     
        header->size += cache_size;  
        do_add = 0;                  
    }

    header_t *test_header = (header_t*) ( (uint32)footer + sizeof(footer_t) );
    if (test_header->magic == HEAP_MAGIC && test_header->is_hole) {
        header->size += test_header->size;
        test_footer = (footer_t*) ( (uint32)test_header + test_header->size - sizeof(footer_t) );
        footer = test_footer;
   
        uint32 iterator = 0;
        while ( (iterator < heap->index.size) && (lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
            iterator++;

   
        ASSERT(iterator < heap->index.size);
   
        remove_ordered_array(iterator, &heap->index);
    }

    if ((uint32)footer+sizeof(footer_t) == heap->end_address) {
        uint32 old_length = heap->end_address-heap->start_address;
        uint32 new_length = contract( (uint32)header - heap->start_address, heap);
   
        if (header->size - (old_length-new_length) > 0) {
       
            header->size -= old_length-new_length;
            footer = (footer_t*) ( (uint32)header + header->size - sizeof(footer_t) );
            footer->magic = HEAP_MAGIC;
            footer->header = header;
        } else {
       
            uint32 iterator = 0;
            while ((iterator < heap->index.size) && (lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
                iterator++;
       
            if (iterator < heap->index.size)
                remove_ordered_array(iterator, &heap->index);
        }
    }

    if (do_add == 1)
        insert_ordered_array((void*)header, &heap->index);

}
