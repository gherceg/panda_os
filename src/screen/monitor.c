#include "monitor.h"

uint16 * video_memory = (uint16 *) 0xB8000;

uint8 cursor_x = 0;
uint8 cursor_y = 0;

uint8 backColour = 3;
uint8 foreColour = 15;

static void move_cursor() {
   uint16 cursorLocation = cursor_y * 80 + cursor_x;
   outb(0x3D4, 14);                     
}


static void scroll() {

    uint8 attributeByte = (backColour << 4) | (foreColour & 0x0F);
    uint16 blank = 0x20 | (attributeByte << 8);

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

    uint8  attributeByte = (backColour << 4) | (foreColour & 0x0F);
    uint16 attribute = attributeByte << 8;
    uint16 *location;

    if(c == 0x08 && cursor_x) {
        cursor_x--;
    }

    else if(c == 0x09) {
        cursor_x = (cursor_x+8) & ~(8-1);
    }

    else if(c == '\r') {
        cursor_x = 0;
    }

    else if(c == '\n') {
        cursor_x = 0;
        cursor_y++;
    }
    else if(c >= ' ') {
        location = video_memory + (cursor_y*80 + cursor_x);
        *location = c | attribute;
        cursor_x++;
    }

    if(cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    scroll();
    move_cursor();
}

void monitor_clear() {
   uint8 attributeByte = (backColour << 4) | (foreColour & 0x0F);
   uint16 blank = 0x20 | (attributeByte << 8); // write space

   int i;
   for (i = 0; i < 80*25; i++) {
       video_memory[i] = blank;
   }

   cursor_x = 0;
   cursor_y = 0;
   move_cursor();
}

void monitor_write(char *c) {
   int i = 0;

   while (c[i]) {
       monitor_put(c[i++]);
   }
}

void monitor_write_color(char *c, uint8 color) {

    uint8 temp = foreColour;
    foreColour = color;

    monitor_write(c);

    foreColour = temp;
}

void monitor_write_sys(char * c) {
    monitor_write_color(c, 4);
}


void monitor_write_hex(uint32 n) {
    int32 tmp;

    monitor_write("0x");

    char noZeroes = 1;

    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0) {
            continue;
        }
    
        if (tmp >= 0xA) {
            noZeroes = 0;
            monitor_put (tmp-0xA+'a' );
        } else {
            noZeroes = 0;
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