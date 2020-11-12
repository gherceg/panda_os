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

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint32 divisor = 1193180 / frequency;

    // 0x36 sets clock to repeating mode (refresh after divisor hits 0)
    // it also says we want to set the divisor value
    // TODO: not sure what 0x43 is for so look into that
    outb(0x43, 0x36);

    // must be sent as 2 separate bytes, not one 16 bit value. 
    uint8 l = (uint8)(divisor & 0xFF);
    uint8 h = (uint8)((divisor>>8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);
}