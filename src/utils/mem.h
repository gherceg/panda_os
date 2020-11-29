#ifndef MEM
#define MEM

#include "dttp.h"
#include "stddef.h"

void memcpy(uint8 *dest, const uint8 *src, size_t len);
void memset(uint8 *dest, uint8 val, size_t len);

#endif