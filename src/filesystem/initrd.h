
#ifndef INITRD_H
#define INITRD_H

#include "../tools.h"
#include "../memory/kheap.h"
#include "fs.h"
#include <stddef.h>

/**
 * Initialize Ram Disc (initrd) is used to create a simple filesystem in memory for the kernel to use on startup
 */

typedef struct {
	size_t nfiles;
} initrd_header_t;

typedef struct {
    uint8 magic;
    int8 name[64];
    size_t offset;  
    size_t length;
} initrd_file_header_t;

fs_node_t *initialise_initrd(size_t location);

#endif