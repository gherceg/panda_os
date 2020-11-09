#ifndef ASM
#define ASM

#include "dttp.h"

// Writes a byte to the specified port
void outb(uint16 port, uint8 value);

// Reads a byte from the specified port
uint8 inb(uint16 port);

// Reads a word (2 bytes) from the specified port
uint16 inw(uint16 port);

void act_itr();
void deact_itr();

#endif