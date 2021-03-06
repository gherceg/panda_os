#ifndef MONITOR_H
#define MONITOR_H

#include "../tools.h"

void monitor_put(char c);

void monitor_clear();

void monitor_write(char *c);

void monitor_write_hex(size_t n);

void monitor_write_dec(size_t n);

void monitor_write_color(char *c, uint8 color);

void monitor_write_sys(char * c);

#endif