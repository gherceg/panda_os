#ifndef TIMER_H
#define TIMER_H

#include "../tools.h"

/**
 * The Programmable Interval Timer (PIT) is responsible for interrupting the CPU at a specified interval.
 * It is a chip connected to pin IRQ0 on the Programmable Interrupt Controller (PIC)
 * The ability to interrupt the CPU at a specified interval is mandatory for handling multiple processes.
 * This is also how a system clock is implemented
*/

void init_timer(uint32 frequency);

#endif