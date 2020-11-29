#include "task.h"
#include "../memory/paging.h"
#include "../memory/kheap.h"
#include "../screen/monitor.h"
#include "../tools.h"


// current running stack
volatile task_t *current_task;
// linked list of tasks
volatile task_t *ready_queue;

// needed to access members of paging.c
extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;
extern size_t initial_esp;
extern void alloc_frame(page_t*, int, int);
extern size_t read_eip();

// ensures unique pid
size_t next_pid = 1;

/**
 * Kicks off the first process
 */ 
void initialise_tasking() {

    deact_itr();

    move_stack((void*)0xE0000000, 0x2000);

    // setup the root task which is the kernel
    current_task = ready_queue = (task_t*) kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_directory;
    current_task->next = 0;

    act_itr();
}

/**
 * @param new_stack_start: desired address to move stack to
 * @param size: size of the stack being moved
 */ 
void move_stack(void *new_stack_start, size_t size) {

  size_t i;
  for(i = (size_t)new_stack_start; i >= ((size_t)new_stack_start-size); i -= 0x1000) {
    // create frames for stack, user mode is sufficient here
    alloc_frame(get_page(i, 1, current_directory), 0 /* User mode */, 1 /* Is writable */ );
  }
  
  // flush the TLB by writing by reading and writing the page directory address
  // to partially flush, we could use the invlpg instruction but this is simpler
  size_t pd_addr;
  asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
  asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

  // get current stack and base pointers to determine offset to apply to new stack
  size_t old_stack_pointer; asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
  size_t old_base_pointer;  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer));

  size_t offset = (size_t)new_stack_start - initial_esp;

  size_t new_stack_pointer = old_stack_pointer + offset;
  size_t new_base_pointer = old_base_pointer  + offset;

  memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp-old_stack_pointer);

  // naive algorithm for updating any base pointers to apply the offset to
  // assumes any value that falls within the old stacks address space is a base pointer, which isn't necessarily true
  // TODO: improve this so that only base pointers modified, not data we want to keep
  for(i = (size_t)new_stack_start; i > (size_t)new_stack_start-size; i -= 4) {
    size_t tmp = * (size_t*)i;

    if ( (old_stack_pointer < tmp) && (tmp < initial_esp) ) {
      tmp = tmp + offset;
      size_t *tmp2 = (size_t*)i;
      *tmp2 = tmp;
    }
  }

  // update the stack and current base pointer
  asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
  asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}

void switch_task() {

  if (!current_task) {
    monitor_write("Error: attempted to switch tasks before enabling tasking\n");
    return;
  }

  // check if it is necessary to switch
  if (current_task == ready_queue && current_task->next == 0) {
    monitor_write("only one task found, not going to switch\n");
    return;
  }

  size_t esp, ebp, eip;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  asm volatile("mov %%ebp, %0" : "=r"(ebp));

  eip = read_eip();

  // after read_eip, we could be in one of two states
  // 1) we received the current instruction pointer and want to continue switching tasks
  // 2) we have just switched tasks, and because the saved EIP is essentially
   // -- the instruction after read_eip(), it will seem as if read_eip has just returned
  // we add a dummy value to the eax (overwriting the actual return value) below to identify the second scenario
  if (eip == 0x12345) {
    // the new task has already been setup and started execution
    monitor_write("parent task found, return\n");
    return;
  } 

  // before switching, save off current register values
  current_task->eip = eip;
  current_task->esp = esp;
  current_task->ebp = ebp;

  // switch
  current_task = current_task->next;

  // if reached the end, go back to the beginning of the queue
  if (!current_task) current_task = ready_queue;

  // strictly for readability below
  eip = current_task->eip;
  esp = current_task->esp;
  ebp = current_task->ebp;
  // update the current directory
  current_directory = current_task->page_directory;

  // Here we:
  // * Stop interrupts so we don't get interrupted.
  // * Temporarily put the new EIP location in ECX.
  // * Load the stack and base pointers from the new task struct.
  // * Change page directory to the physical address (physical_address_of_tables_physical) of the new directory.
  // * Put a dummy value (0x12345) in EAX so that above we can recognise that we've just
  // switched task.
  // * Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
  // the next instruction.
  // * Jump to the location in ECX (remember we put the new EIP in there).
  asm volatile("cli");
  asm volatile("mov %0, %%ecx" : : "r"(eip));
  asm volatile("mov %0, %%esp" : : "r"(esp));
  asm volatile("mov %0, %%ebp" : : "r"(ebp));
  asm volatile("mov %0, %%cr3" : : "r"(current_directory->physical_address_of_tables_physical));
  asm volatile("mov $0x12345, %eax");
  asm volatile("sti");
  asm volatile("jmp *%ecx");
}

int fork() {
  // do not want to get interrupted while modifying kernel structures
  deact_itr();

  // need to reference the parent task later
  task_t *parent_task = (task_t*)current_task;

  page_directory_t *directory = clone_directory(current_directory);

  // Create a new task/process
  task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
  new_task->id = next_pid++;
  new_task->esp = new_task->ebp = 0;
  new_task->eip = 0;
  new_task->page_directory = directory;
  new_task->next = 0;

  // add to the end of the task queue
  task_t *tmp_task = (task_t*)ready_queue;
  while (tmp_task->next)
      tmp_task = tmp_task->next;
  tmp_task->next = new_task;

  // defined in process.s, quickly 
  // need to tell the task where to start executing which can be found via the current instruction
  size_t eip = read_eip();

  // at this point we could be either the parent or child task and need to check
  if (current_task == parent_task) {

    // only setup esp, ebp, and eip for child if we are the parent
    size_t esp; asm volatile("mov %%esp, %0" : "=r"(esp));
    size_t ebp; asm volatile("mov %%ebp, %0" : "=r"(ebp));
    new_task->esp = esp;
    new_task->ebp = ebp;
    new_task->eip = eip;

    // all done modifying so interrupts can be reenabled
    act_itr();

    // parent returns id of newly created child
    return new_task->id;
  } else {
    // child returns 0 to inform caller which process is executing
    return 0;
  }
}

int getpid() {
  return current_task->id;
}