#ifndef __MEM_UTILS_H__
#define __MEM_UTILS_H__

#include "defs.h"

void _kmemset(void *ptr, unsigned char value, uint32 num);
void _kmemcpy(void *dest, const void *source, uint32 num);
int32 _kmemcmp(const void *ptr1, const void *ptr2, uint32 num);

#endif
