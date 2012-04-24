#ifndef _VM8086_H_
#define _VM8086_H_

#include "headers.h"

#define INT_VEC_GPF 0xD 

typedef struct
{

} vm8086_context_t;

void __vm8086_init(void);

void _isr_gpf(int vector, int code);

#endif
