#include "timer.h"
#include "isr.h"
#include "../screen/monitor.h"
#include "../mm/task.h"

uint32 tick = 0;

static void timer_callback(registers_t regs) {
    tick++;
    switch_task();
    // monitor_write("Tick ");
    // monitor_write_dec(tick);
    // monitor_write("\n");
}

void init_timer(uint32 frequency) {
    register_interrupt_handler(IRQ0, &timer_callback);

    uint32 divisor = 1193180 / frequency;

    outb(0x43, 0x36);

    uint8 l = (uint8)(divisor & 0xFF);
    uint8 h = (uint8)( (divisor>>8) & 0xFF );

    outb(0x40, l);
    outb(0x40, h);
}