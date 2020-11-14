# This Makefile will compile every file in SOURCES, then link them together into one ELF binary, 'kernel'. It uses a linker script, 'link.ld' to do this

# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler
# rule, as we use nasm instead of GNU as.

SOURCES=boot.o main.o \
drivers/keyboard.o \
fs/fs.o fs/initrd.o \
mm/kheap.o mm/paging.o mm/process.o mm/task.o \
screen/monitor.o \
tb_itr/descriptor_tables.o tb_itr/gdt.o tb_itr/interrupt.o tb_itr/isr.o tb_itr/timer.o \
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
