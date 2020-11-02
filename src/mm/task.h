#ifndef TASK_H
#define TASK_H

#include "../tools.h"
#include "../mm/paging.h"

typedef struct task {
    int id;               
    uint32 esp, ebp;      
    uint32 eip;           
    page_directory_t *page_directory;
    struct task *next;    
} task_t;

void initialise_tasking();

void switch_task();

int fork();

void move_stack(void *new_stack_start, uint32 size);

int getpid();

#endif