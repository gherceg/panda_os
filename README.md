# Panda OS
## Motivation
Started by Josue Rivera, Panda OS is a collaborative effort amongst students of CIS 573 to create an educational operating system that is **easy** to contribute to. We aim to make 
this project as accessible as possible for future students to use as a reference, 
a playground, and a starting point for final projects. If you contribute to this project, please keep in mind that your contributions will need to be understood by future students.

This project was initially based on [this tutorial](https://web.archive.org/web/20160408171631/http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html) so it serves as a useful reference.

This OS is largely derived from the [James Molloy Tutorial](http://www.jamesmolloy.co.uk/tutorial_html/). While the tutorial is very informative and we recommend going through it before making changes here, there are known bugs that the OS Dev wiki has kept track of [here](https://wiki.osdev.org/James_Molloy's_Tutorial_Known_Bugs). 

We've added a checklist of the known bugs here to better track which ones have been addressed. 

- [ ] Add a proper cross-compiler
- [ ] Do not rely on GRUB stack, setup our own
- [x] Rename main
- [ ] Use standard C datatypes by including stdint.h (which comes with freestanding mode)
- [ ] Inspect inline assembly (compare to examples [here](https://wiki.osdev.org/Inline_Assembly/Examples))
- [x] Add memmove and memcmp to mem.c for compiler to use
- [x] Do not pass instance of struct `registers` by value to `isr_handler` in `interrupt.s`, pass by reference
- [ ] ISR 17 and 21 push fake error codes even though the CPU handles
- [ ] `struct registers` has a useless member `esp`
- [ ] overuse of `__attribute__((packed))`
- [x] cli and sti in interrupt handlers in `interrupt.s`. Specify in IDT entry if interrupts should be on or off.
- [ ] kmalloc isn't properly aligned ( only aligns for page and 1 byte values, but should take into account all sizes )
- [x] use `size_t` instead of `uint32` where applicable
- [x] fix check for page aligned
- [ ] refactor paging code
- [x] do not reenable paging in `switch_page_directory`, instead rely on special function to do so on the first time
- [ ] assembly used in section 6.4.5 doesn't need to be volatile
- [ ] refactor heap code
- [x] fix bug in `find_smallest_hole`
- [ ] store filename in directory entries rather than the inode itself (`struct fs_node`)
- [ ] use copy of multiboot.h from GRUB source code rather than the tutorial
- [ ] refactor multitasking code
- [ ] inline assembly optimiser with gcc
- [ ] nasm byte keyword causes 0x80 to becomme 0xffffff80
- [ ] allow interrupts in user mode in idt_set_gate
- [x] regs var must be called by reference instead of value in irq and isr handlers
- [ ] missing documentation around set_kernel_stack


## Setup

Hopefully the build process and workflow only become more flexible as time goes on. 
At the moment there are 2 recommended emulators to use for fast development:
- [QEMU](https://www.qemu.org/)
- [bochs](http://bochs.sourceforge.net/)

Additionally, we use the [NASM assembler](https://www.nasm.us/xdoc/2.15.05/html/nasmdoc1.html#section-1.1) 
which is an open-source assembler for x86 architectures. 

#### Linux

Install NASM:\
`sudo apt install nasm`

Install genisoimage:\
`sudo apt install genisoimage`

Compile any changed files:\
`gcc -m32 -c <file.c>`

Run build script:\
`source ./build_panda.sh`

#### Windows

From C:\Program Files\Qemu:\
I happen to be using Ubuntu-20.04 for my WSL, but be sure to adjust this path as needed.\
`qemu-system-x86_64.exe -cdrom \\wsl$\Ubuntu-20.04\home\<user>\path\to\panda_os\panda.iso`

A 100% Windows development workflow has not been added yet. The easiest path forward 
is to use the Windows Subsystem for Linux for OS development, and then run bochs from the Windows side. 

The WSL is not a complete linux distro out of the box, so beware of attempting to follow the Linux 
instructions above on WSL only. You might run into issues.

## Under the Hood
If you are taking CIS 573 or a similar course focused on Operating Systems, you will most likely have focused on the kernel. This includes responsibilities suchas memory management, process management, filesystems and I/O. However, the kernel is not the first piece of software to run when a computer is booted. 

### BIOS
In addition to the motherboard BIOS, each piece of attached hardware has its own BIOS. This is going to be our operating system's interface for communicating with hardware. 

### Bootloader
The bootloader is essential to any operating system. It is the first software to run after boot, and its responsibilities include preparing the CPU to run the kernel, and eventually transferring control to the kernel. 

Panda OS uses the GRUB bootloader to allow for more focus on implementing the kernel itself. If you love programming in Assembly perhaps you can try setting up a custom bootloader for Panda OS.

## Kernel


## Contributing

