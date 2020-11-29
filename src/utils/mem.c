
#include "dttp.h"
#include "mem.h"
#include "stddef.h"

void memcpy(uint8 *dest, const uint8 *src, size_t len) {
    const uint8 *sp = (const uint8 *)src;
    uint8 *dp = (uint8 *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

void memset(uint8 *dest, uint8 val, size_t len) {
    uint8 *temp = (uint8 *)dest;
    for (; len != 0; len--) *temp++ = val;
}

void memmove(uint8 *dest, const uint8 *src, size_t len) {
	uint8 *ret = (uint8 *)dest;
	if(dest <= src || (uint8 *)dest >= ((uint8 *)src + len)) {
		for(; len != 0; len--) {
			*(uint8 *)dest = *(uint8 *)src;
			dest = (uint8 *)dest + 1;
			src = (uint8 *)src + 1;
		}
	} else {
		dest = (uint8 *)dest + len - 1;
		src = (uint8 *)src + len - 1;
		for(; len != 0; len--) {
			*(uint8 *)dest = *(uint8 *)src;
			dest = (uint8 *)dest - 1;
			src = (uint8 *)src - 1;
		}
	}
}

int memcmp(uint8 *dest, uint8 *src, size_t len) {
	if(!len) {
		return 0;
	}

	while(--n && *(uint8 *)dest == *(uint8 *)src) {
		dest = (uint8 *)dest + 1;
		src = (uint8 *)src + 1;
	}

	return *((unsigned uint8 *)dest) - *((unsigned uint8 *)src);
}