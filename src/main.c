#include "descriptor_tables/descriptor_tables.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/timer.h"
#include "filesystem/fs.h"
#include "filesystem/initrd.h"
#include "memory/paging.h"
#include "process/task.h"
#include "multiboot.h"
#include "screen/monitor.h"
#include "utils/asm.h"
#include <stddef.h>

extern size_t placement_address; //fs start
size_t initial_esp;

// helpers defined below
size_t create_filesystem(struct multiboot *mboot_ptr);

// test helpers
void force_page_fault();
void test_heap();
void print_filesystem_contents();


int main(struct multiboot *mboot_ptr, size_t initial_stack) {

	// setup
	initial_esp = initial_stack;
	init_descriptor_tables();
	monitor_clear();

    // start PIT - number specified equals interrupts per second
	// init_timer(1);

    // reserve beginning of memory for filesystem befor enabling paging
    size_t initrd_location = create_filesystem(mboot_ptr);

    // comment out initialise_paging if testing heap
    // test_heap();
    initialise_paging();

    // mulitasking
    // initialise_tasking();

    // create kernel in-memory filesystem
    fs_root = initialise_initrd(initrd_location);

    // register handler for IRQ1
    install_keyboard_driver(); 

    // enables interrupts
	act_itr();


	monitor_write("Welcome to Josue's Panda OS\n");
	monitor_write("\n==========================\n");

    
    // Create a new process in a new address space which is a clone of this.
    // monitor_write("forking process\n");
    // int ret = fork();

    // monitor_write("fork() returned ");
    // monitor_write_hex(ret);
    // monitor_write(", and getpid() returned ");
    // monitor_write_hex(getpid());
    // monitor_write("\n============================================================================\n");

    // monitor_write("switching process\n");
    // switch_task();

    print_filesystem_contents();

    return 0;
}

size_t create_filesystem(struct multiboot *mboot_ptr) {

    // check if initrd is installed
    ASSERT(mboot_ptr->mods_count > 0); 
    // Find the location of our initial ramdisk.
    size_t initrd_location = *((size_t*)mboot_ptr->mods_addr);
    size_t initrd_end = *(size_t*)(mboot_ptr->mods_addr+4);
    // Don't trample our module with placement accesses, please!
    placement_address = initrd_end;
    return initrd_location;
}


// TEST HELPERS

void run_tests() {
    test_heap();
    force_page_fault();
}

void print_filesystem_contents() {
    // not reentrant so make sure we are not interrupted

    deact_itr();

    if (!fs_root) {
        monitor_write("\nNo filesystem found\n");
    }

    int i = 0;
    struct dirent *node = 0;
    while((node = readdir_fs(fs_root, i)) != 0) {
        monitor_write("Found file ");
        monitor_write_sys(node->name);
        fs_node_t *fsnode = finddir_fs(fs_root, node->name);

        if ((fsnode->flags&0x7) == FS_DIRECTORY) {
            monitor_write_sys("\n\t(directory)\n"); 
        } else {
            monitor_write_sys("\n\t contents: \"");
            char buf[256];
            size_t sz = read_fs(fsnode, 0, 256, buf);
            int j;
            for (j = 0; j < sz; j++)
                monitor_put(buf[j]);
            
            monitor_write_sys("\"\n");
        }
        i++;
    }
    monitor_write("\n");

    act_itr();
}

void force_page_fault() {
    size_t *ptr = (size_t*)0xA0000000;
    size_t do_page_fault = *ptr;
}

void test_heap() {
    // EXPECTATION: b == d because b is allocated on the heap, then freed
    // d is then allocated after b is freed, so it should reclaim that same memory

   // paging is not yet enabled, so this will be placed at the current placement_address 
    size_t a = kmalloc(8);
    initialise_paging();

    // paging is enabled, so allocated on the heap
    size_t b = kmalloc(8);
    size_t c = kmalloc(8);
    monitor_write("a: ");
    monitor_write_hex(a);
    monitor_write(", b: ");
    monitor_write_hex(b);
    monitor_write("\nc: ");
    monitor_write_hex(c);

    kfree((void *)c);
    kfree((void *)b);
    // now allocate d and expect it to be at the same location as b
    size_t d = kmalloc(12);
    monitor_write(", d: ");
    monitor_write_hex(d);
    monitor_write("\n");
}