#ifndef __E1000_H__
#define __E1000_H__

#include "types.h"

#define E100_VENDOR 0x8086
//#define E100_DEVICE 0x1229
#define E100_DEVICE 0x1209
#define E100_BASE_CLASS 0x02
#define E100_SUB_CLASS  0x00
#define E100_PROG_IF    0x00


void __net_init(void);

#endif
