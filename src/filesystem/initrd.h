
#ifndef INITRD_H
#define INITRD_H

#include "../tools.h"
#include "../memory/kheap.h"
#include "fs.h"

/**
 * Initialize Ram Disc (initrd) is used to create a simple filesystem in memory for the kernel to use on startup
 */

typedef struct {
	uint32 nfiles;
} initrd_header_t;

typedef struct {
    uint8 magic;
    int8 name[64];
    uint32 offset;  
    uint32 length;
} initrd_file_header_t;

fs_node_t *initialise_initrd(uint32 location);

#endif