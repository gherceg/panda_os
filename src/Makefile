# This Makefile will compile every file in SOURCES, then link them together into one ELF binary, 'kernel'. It uses a linker script, 'link.ld' to do this

# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler
# rule, as we use nasm instead of GNU as.

SOURCES=boot.o main.o \
descriptor_tables/descriptor_tables.o descriptor_tables/gdt.o descriptor_tables/idt.o \
drivers/keyboard/keyboard.o drivers/timer/timer.o \
filesystem/fs.o filesystem/initrd.o \
memory/kheap.o memory/paging.o \
process/process.o process/task.o \
screen/monitor.o \
interrupts/interrupt.o interrupts/isr.o \
utils/asm.o utils/mem.o utils/ordered_array.o utils/panic.o utils/string.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf32

all: $(SOURCES) link

clean:
	-rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
