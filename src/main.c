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

extern uint32 placement_address; //fs start
uint32 initial_esp;

// helpers defined below
uint32 create_filesystem(struct multiboot *mboot_ptr);

// test helpers
void force_page_fault();
void test_heap();
void print_filesystem_contents();

// kernel entry point
int main(struct multiboot *mboot_ptr, uint32 initial_stack) {

	// setup
	initial_esp = initial_stack;
	init_descriptor_tables();
	monitor_clear();

    // reserve beginning of memory for filesystem befor enabling paging
    uint32 initrd_location = create_filesystem(mboot_ptr);

    // comment out initialise_paging if testing heap
    // test_heap();
    initialise_paging();

    // create kernel in-memory filesystem
    fs_root = initialise_initrd(initrd_location);

    // register handler for IRQ1
    install_keyboard_driver(); 

    // enables interrupts
	act_itr();

    // start PIT - number specified equals interrupts per second
	// init_timer(1);

	monitor_write("Welcome to Josue's Panda OS\n");
	monitor_write("\n");

    print_filesystem_contents();

	//initialise_tasking(); //Multi tasking

    return 0;
}

uint32 create_filesystem(struct multiboot *mboot_ptr) {

    // check if initrd is installed
    ASSERT(mboot_ptr->mods_count > 0); 
    // Find the location of our initial ramdisk.
    uint32 initrd_location = *((uint32*)mboot_ptr->mods_addr);
    uint32 initrd_end = *(uint32*)(mboot_ptr->mods_addr+4);
    // Don't trample our module with placement accesses, please!
    placement_address = initrd_end;
    return initrd_location;
}


// TEST HELPERS

void print_filesystem_contents() {
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
            uint32 sz = read_fs(fsnode, 0, 256, buf);
            int j;
            for (j = 0; j < sz; j++)
                monitor_put(buf[j]);
            
            monitor_write_sys("\"\n");
        }
        i++;
    }
    monitor_write("\n");
}

void force_page_fault() {
    uint32 *ptr = (uint32*)0xA0000000;
    uint32 do_page_fault = *ptr;
}

void test_heap() {
    uint32 a = kmalloc(8);
    initialise_paging();
    uint32 b = kmalloc(8);
    uint32 c = kmalloc(8);
    monitor_write("a: ");
    monitor_write_hex(a);
    monitor_write(", b: ");
    monitor_write_hex(b);
    monitor_write("\nc: ");
    monitor_write_hex(c);

    kfree((void *)c);
    kfree((void *)b);
    uint32 d = kmalloc(12);
    monitor_write(", d: ");
    monitor_write_hex(d);
}