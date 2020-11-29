
#include "dttp.h"
#include "../screen/monitor.h"
#include <stddef.h>

extern void panic(const char *message, const char *file, size_t line) {
    
    asm volatile("cli"); 

    monitor_write_sys("PANIC(");
    monitor_write_sys(message);
    monitor_write_sys(") at ");
    monitor_write_sys(file);
    monitor_write_sys(":");
    monitor_write_dec(line);
    monitor_write_sys("\n");
    
    for(;;);
}

extern void panic_assert(const char *file, size_t line, const char *desc) {
    
    asm volatile("cli"); 

    monitor_write_sys("ASSERTION-FAILED(");
    monitor_write_sys(desc);
    monitor_write_sys(") at ");
    monitor_write_sys(file);
    monitor_write_sys(":");
    monitor_write_dec(line);
    monitor_write_sys("\n");
    
    for(;;);
}