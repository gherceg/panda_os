#ifndef TASK_H
#define TASK_H

#include "../tools.h"
#include "../memory/paging.h"

/**
 * A task/process stores information needed to properly stop/start the process in case of interrupts and/or context switching
 */

typedef struct task {
    int id; // self explanatory, but unique identifies the process   
    size_t esp, ebp; // stack and base pointers      
    size_t eip; // instruction pointer           
    page_directory_t *page_directory; // page directory
    struct task *next; // next task in linked list    
} task_t;

//
void initialise_tasking();

// when the timer interrupt fires, it calls this method to change the running process
void switch_task();

// forks/clones the existing process to a new address space
int fork();

// moves a process' stack the new desired location
void move_stack(void *new_stack_start, size_t size);

// id of the current running process
int getpid();

#endif