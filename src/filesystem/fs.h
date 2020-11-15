// Defines the interface for and structures relating to the virtual file system.
#ifndef FS_H
#define FS_H

#include "../tools.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?

struct fs_node;

typedef uint32 (*read_type_t)(struct fs_node*,uint32,uint32,uint8*);
typedef uint32 (*write_type_t)(struct fs_node*,uint32,uint32,uint8*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent * (*readdir_type_t)(struct fs_node*,uint32);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*,char *name);

typedef struct fs_node {
    char name[128];     // File name
    uint32 mask;        // Permissions mask
    uint32 uid;         // Owning user
    uint32 gid;         // Owning group
    uint32 flags;       // Includes node type
    uint32 inode;       // Allows file system to identify files
    uint32 length;      // Size of file, in bytes
    uint32 impl;        // Implementation defined number
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir; // Returns the n'th child of a directory.
    finddir_type_t finddir; // Try to find a child in a directory by name.
    struct fs_node *ptr; // Used by mountpoints and symlinks
} fs_node_t;

struct dirent { // Returned by readdir call
    char name[128]; // File name
    uint32 ino;     // Inode number
};

extern fs_node_t *fs_root; // File system root


// Standard read/write/open/close functions. Note that these are all suffixed with
// _fs to distinguish them from the read/write/open/close which deal with file descriptors
// not file nodes.
uint32 read_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer);
uint32 write_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer);
void open_fs(fs_node_t *node, uint8 read, uint8 write);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint32 index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

#endif
