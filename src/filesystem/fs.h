
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

typedef size_t (*read_type_t)(struct fs_node*,size_t,size_t,uint8*);
typedef size_t (*write_type_t)(struct fs_node*,size_t,size_t,uint8*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent * (*readdir_type_t)(struct fs_node*,size_t);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*,char *name);

typedef struct fs_node {
    char name[128];     
    size_t mask;        
    size_t uid;         
    size_t gid;         
    size_t flags;       
    size_t inode;       
    size_t length;      
    size_t impl;        
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
    size_t ino;     
};

extern fs_node_t *fs_root; 

size_t read_fs(fs_node_t *node, size_t offset, size_t size, uint8 *buffer);
size_t write_fs(fs_node_t *node, size_t offset, size_t size, uint8 *buffer);
void open_fs(fs_node_t *node, uint8 read, uint8 write);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, size_t index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

#endif
