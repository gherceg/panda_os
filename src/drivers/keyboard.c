#include "keyboard.h"
#include "../tb_itr/isr.h"
#include "../screen/monitor.h"

/**
 * A keyboard interfaces with a Keyboard Controller to specify when and what key is pressed/released.
 * The keyboard does so by sending scan codes to the controller. The controller has two registers:
 * - a data register located at 0x60: this is where the scan code is stored
 * - a control register located at 0x64: TODO: look into the this more
 */

static void keyboard_handler(registers_t regs) {
    
    // read from the keyboard controller's data register
    // if the value is not read from this register, the keyboard will not be able to write future values 
    // and therefore not trigger an interrupt
    uint8 scan_code = inb(0x60);

    // the highest bit is used to communicate if key was pressed (make) or released (break)
    if (scan_code & 0x80) {
        monitor_write("key released\n");
    } else {
        monitor_write("key pressed\n");
    }
}

void install_keyboard_driver() {
    register_interrupt_handler(IRQ1, &keyboard_handler);
}