# Panda OS
## Motivation
Started by Josue Rivera, Panda OS is a collaborative effort amongst students of CIS 573 to create an educational operating system that is **easy** to contribute to. We aim to make 
this project as accessible as possible for future students to use as a reference, 
a playground, and a starting point for final projects. If you contribute to this project, please keep in mind that your contributions will need to be understood by future students.

This project was initially based on [this tutorial](https://web.archive.org/web/20160408171631/http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html) so it serves as a useful reference.

## Setup

Hopefully the build process and workflow only become more flexible as time goes on. 
At the moment there are 2 recommended emulators to use for fast development:
- [QEMU](https://www.qemu.org/)
- [bochs](http://bochs.sourceforge.net/)

Additionally, we use the [NASM assembler](https://www.nasm.us/xdoc/2.15.05/html/nasmdoc1.html#section-1.1) 
which is an open-source assembler for x86 architectures. 

#### Linux

#### macOS

Homebrew is used in this example, but feel free to use a package manager of your choice.

Install NASM:\
`brew install nasm`

Install QEMU:\
`brew install qemu`

From the root directory of the project, type the following to run the OS:\
`qemu-system-x86_64 -cdrom panda.iso`

#### Windows

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

