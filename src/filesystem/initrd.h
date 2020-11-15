// Defines the interface for and structures relating to the initial
#ifndef INITRD_H
#define INITRD_H

#include "../tools.h"
#include "../memory/kheap.h"
#include "fs.h"

typedef struct {
	uint32 nfiles;	// The number of files in the ramdisk.
} initrd_header_t;

typedef struct {
    uint8 magic;	// Magic number, for error checking.
    int8 name[64];	// Filename.
    uint32 offset;  // Offset in the initrd that the file starts.
    uint32 length;	// Length of the file.
} initrd_file_header_t;


// Initialises the initial ramdisk. It gets passed the address of the multiboot module,
// and returns a completed filesystem node.
fs_node_t *initialise_initrd(uint32 location);

#endif