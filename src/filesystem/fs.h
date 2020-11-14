
#ifndef FS_H
#define FS_H

#include "../tools.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 

struct fs_node;

typedef uint32 (*read_type_t)(struct fs_node*,uint32,uint32,uint8*);
typedef uint32 (*write_type_t)(struct fs_node*,uint32,uint32,uint8*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent * (*readdir_type_t)(struct fs_node*,uint32);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*,char *name);

typedef struct fs_node {
    char name[128];     
    uint32 mask;        
    uint32 uid;         
    uint32 gid;         
    uint32 flags;       
    uint32 inode;       
    uint32 length;      
    uint32 impl;        
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    struct fs_node *ptr; 
} fs_node_t;

struct dirent {
    char name[128]; 
    uint32 ino;     
};

extern fs_node_t *fs_root; 

uint32 read_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer);
uint32 write_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer);
void open_fs(fs_node_t *node, uint8 read, uint8 write);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint32 index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

#endif
