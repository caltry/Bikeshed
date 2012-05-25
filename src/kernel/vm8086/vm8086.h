#ifndef _VM8086_H_
#define _VM8086_H_

#include "types.h"

/* VM 8086 support is not currently implemented
 *
 * This file is only kept around for it's GPF handler
 */

#define INT_VEC_GPF 0xD 

/* Supposed to intialize VM 8086 support, but currently
 * only installs the GPF handler.
 */
void __vm8086_init(void);

/* The General Protection Fault Handler. Prints where
 * the GPF happend
 */
void _isr_gpf(int vector, int code);

#endif
