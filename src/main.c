#include "screen/monitor.h"
#include "tb_itr/descriptor_tables.h"
#include "tb_itr/timer.h"
#include "drivers/keyboard.h"
#include "mm/paging.h"
#include "multiboot.h"
#include "fs/fs.h"
#include "fs/initrd.h"
#include "mm/task.h"
#include "utils/asm.h"

extern uint32 placement_address; //fs start
uint32 initial_esp;

int main(struct multiboot *mboot_ptr, uint32 initial_stack) {

	//Set up
	initial_esp = initial_stack;
	init_descriptor_tables();
	monitor_clear();

	act_itr();

    // start PIT - number specified equals interrupts per second
	// init_timer(1);

	monitor_write("Welcome to Josue's Panda OS\n");
	monitor_write("\n");

    ASSERT(mboot_ptr->mods_count > 0); //check to see if initrd is installed

	uint32 initrd_location = *((uint32*)(mboot_ptr->mods_addr));
	uint32 initrd_end = (*(uint32*)(mboot_ptr->mods_addr+4));
	placement_address = initrd_end;

    asm volatile ("int $0x3");
    asm volatile ("int $0x4");

    // register handler for IRQ1
    install_keyboard_driver(); 
    
	// initialise_paging();

    
	//initialise_tasking(); //Multi tasking

	// fs_root = initialise_initrd(initrd_location);

	// int i = 0;
    // struct dirent *node = 0;

    // while((node = readdir_fs(fs_root, i)) != 0) {
    //     monitor_write("Found file ");
    //     monitor_write_sys(node->name);
    //     fs_node_t *fsnode = finddir_fs(fs_root, node->name);

    //     if ((fsnode->flags&0x7) == FS_DIRECTORY) {
    //         monitor_write_sys("\n\t(directory)\n"); 
    //     } else {
    //         monitor_write_sys("\n\t contents: \"");
    //         char buf[256];
    //         uint32 sz = read_fs(fsnode, 0, 256, buf);
    //         int j;
    //         for (j = 0; j < sz; j++)
    //             monitor_put(buf[j]);
            
    //         monitor_write_sys("\"\n");
    //     }
    //     i++;
    // }

    // monitor_write("\n");

    return 0;
}
