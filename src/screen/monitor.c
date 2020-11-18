#include "monitor.h"

/** This controls the framebuffer provided by the Grub bootloader. It's smallest unit is not pixels, just characters.
 * It is 80 characters wide and 25 characters tall. This is sufficient for debugging and demonstrations of OS components,
 * but to get access to pixels you would need to use VESA graphics.
 * 
 * See here for reference https://web.archive.org/web/20160326064341/http://jamesmolloy.co.uk/tutorial_html/3.-The%20Screen.html
*/

// current position in the framebuffer which starts at 0xB8000
// this memory is available to us just like RAM, but is not actually part of the system's memory
// it has been mapped from the VGA controller's dedicated memory
uint16 *video_memory = (uint16 *) 0xB8000;

// x and y values for the cursor to display properly
// should be able to derive the framebuffer loc from these values
uint8 cursor_x = 0;
uint8 cursor_y = 0;

uint8 background_color = 3;
uint8 foreground_color = 15;

static uint8 get_attribute_byte() {
    return (background_color << 4) | (foreground_color & 0x0F);
}

/**
 * 
 */
static void move_cursor() {
    // screen is 80 characters wide
    uint16 cursor_loc = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, cursor_loc >> 8); // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, cursor_loc);      // Send the low cursor byte.                   
}


/**
 * If the screen fills up with text, this will scroll up one line to allow us to continue to see the most recent output
 */
static void scroll() {

    uint16 blank = 0x20 | (get_attribute_byte() << 8);

     if(cursor_y >= 25) {

        int i;
        for (i = 0*80; i < 24*80; i++) {
            video_memory[i] = video_memory[i+80];
        }

        for (i = 24*80; i < 25*80; i++) {
            video_memory[i] = blank;
        }
        
        cursor_y = 24;
    }
}

void monitor_put(char c) {

    uint8  attribute_byte = get_attribute_byte();
    uint16 attribute = attribute_byte << 8;
    uint16 blank = 0x20 | (attribute_byte << 8);
    uint16 *location;

    // handle backspace
    if(c == 0x08 && cursor_x) {
        // write a blank space to the previous spot in memory
        cursor_x--;
        location = video_memory + (cursor_y*80 + cursor_x);
        *location = blank;
    } 
    // handle a tab
    else if(c == 0x09) {
        cursor_x = (cursor_x+8) & ~(8-1);
    }
    // handle new line
    else if(c == '\n' || c == '\r') {
        cursor_x = 0;
        cursor_y++;
    }
    // handle any other character
    else if(c >= ' ') {
        location = video_memory + (cursor_y*80 + cursor_x);
        *location = c | attribute;
        cursor_x++;
    }

    // wrap cursor to the next line if necessary
    if(cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    // scroll if necssary
    scroll();
    
    move_cursor();
}

/**
 * clears the entire screen
 */ 
void monitor_clear() {
   uint16 blank = 0x20 | (get_attribute_byte() << 8); // write space

   int i;
   for (i = 0; i < 80*25; i++) {
       video_memory[i] = blank;
   }

   cursor_x = 0;
   cursor_y = 0;
   move_cursor();
}

/**
 * Wrapper to write an entire string
 */ 
void monitor_write(char *c) {
   int i = 0;

   while (c[i]) {
       monitor_put(c[i++]);
   }
}

void monitor_write_color(char *c, uint8 color) {

    uint8 temp = foreground_color;
    foreground_color = color;

    monitor_write(c);

    foreground_color = temp;
}

void monitor_write_sys(char * c) {
    monitor_write_color(c, 4);
}


void monitor_write_hex(uint32 n) {
    int32 tmp;

    monitor_write("0x");

    char no_zeroes = 1;

    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && no_zeroes != 0) {
            continue;
        }
    
        if (tmp >= 0xA) {
            no_zeroes = 0;
            monitor_put (tmp-0xA+'a' );
        } else {
            no_zeroes = 0;
            monitor_put( tmp+'0' );
        }
    }
  
    tmp = n & 0xF;
    if (tmp >= 0xA) {
        monitor_put (tmp-0xA+'a');
    } else {
        monitor_put (tmp+'0');
    }

}

void monitor_write_dec(uint32 n) {

    if (n == 0) {
        monitor_put('0');
        return;
    }

    int32 acc = n;
    char c[32];
    int i = 0;

    while (acc > 0) {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }

    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;

    while(i >= 0) {
        c2[i--] = c[j++];
    }

    monitor_write(c2);
}