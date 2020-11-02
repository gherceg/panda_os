#ifndef ASM
#define ASM

#include "dttp.h"

void outb(uint16 port, uint8 value);
uint8 inb(uint16 port);
uint16 inw(uint16 port);
void act_itr();
void deact_itr();

#endif