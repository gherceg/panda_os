#include "keyboard.h"
#include "keyboard_mapping.h"
#include "../../interrupts/isr.h"
#include "../../screen/monitor.h"

/**
 * A keyboard interfaces with a Keyboard Controller to specify when and what key is pressed/released.
 * The keyboard does so by sending scan codes to the controller. The controller has two registers:
 * - a data register located at 0x60: this is where the scan code is stored
 * - a control register located at 0x64: TODO: look into the this more
 */

static uint8 keyboard_flags = 0;

// Flag which identifies shift
#define SHIFT 1

// Flag which identifies the caps lock
#define CAPS_LOCK 2

// Flag which identifies control
#define CTRL 4 


static void update_keyboard_flags_on_press(uint8 scan_code) {
    if (scan_code == KEY_LEFT_SHIFT || scan_code == KEY_RIGHT_SHIFT) {
        keyboard_flags = keyboard_flags | SHIFT;
    } else if (scan_code == KEY_LEFT_CONTROL || scan_code == KEY_RIGHT_CONTROL) {
        keyboard_flags = keyboard_flags | CTRL;
    } else if (scan_code == KEY_CAPS_LOCK) {
        keyboard_flags = keyboard_flags | CAPS_LOCK;
    }
}

static void update_keyboard_flags_on_release(uint8 scan_code) {
    if (scan_code == (KEY_LEFT_SHIFT | 0x80) || scan_code == (KEY_RIGHT_SHIFT | 0x80)) {
        keyboard_flags = keyboard_flags & ~SHIFT;
    } else if (scan_code == (KEY_CAPS_LOCK | 0x80)) {
        keyboard_flags = keyboard_flags & ~CAPS_LOCK;
    } else if (scan_code == (KEY_LEFT_CONTROL | 0x80) || scan_code == (KEY_RIGHT_CONTROL | 0x80)) {
        keyboard_flags = keyboard_flags & ~CTRL;
    }
}

static char get_char_for_scan_code(uint8 scan_code) {
    char char_for_scan_code;
    if ((keyboard_flags & SHIFT) != 0) {
        char_for_scan_code = keymap.shift[scan_code];
    } else if ((keyboard_flags & CAPS_LOCK) != 0) {
        char_for_scan_code = keymap.base[scan_code];
        // uppercase if a letter
        if (char_for_scan_code >= 'a' && char_for_scan_code <= 'z') {
            char_for_scan_code = keymap.shift[scan_code];
        }
    } else {
        char_for_scan_code = keymap.base[scan_code];
    } 
    return char_for_scan_code;
}



static void keyboard_handler(registers_t regs) {
    
    // read from the keyboard controller's data register
    // if the value is not read from this register, the keyboard will not be able to write future values 
    // and therefore not trigger an interrupt
    uint8 scan_code = inb(0x60);

    // the highest bit is used to communicate if key was pressed (make) or released (break)
    if (scan_code & 0x80) {
        update_keyboard_flags_on_release(scan_code);
    } else {
        update_keyboard_flags_on_press(scan_code);
        char key_pressed = get_char_for_scan_code(scan_code);
        monitor_put(key_pressed);
    }
}

void install_keyboard_driver() {
    register_interrupt_handler(IRQ1, &keyboard_handler);
}