# Panda OS
## The Goal
Started by Josue Rivera, we hope to build upon his work to create an educational 
operating system for the students of UMass Dartmouth. We aim to make 
this project as accessible as possible for future students to use as a reference, 
a playground, and a starting point for final projects.

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

## Code Structure

## Contributing

