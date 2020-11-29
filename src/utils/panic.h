#ifndef PANIC_H
#define PANIC_H

#include "dttp.h"
#include <stddef.h>

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

extern void panic(const char *message, const char *file, size_t line);
extern void panic_assert(const char *file, size_t line, const char *desc);

#endif