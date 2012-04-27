/*
** File:	bios.h
**
** Author:	Sean Congden
**
** Description:	Support functions to interact with the bios
*/

#ifndef _BIOS_H
#define _BIOS_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// Where the real mode bios code was loaded by the boot loader
#define	REAL_LOAD_ADDRESS		0x200000

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

typedef struct registers {
	Uint16 di;
	Uint16 si;
	Uint16 bx;
	Uint16 dx;
	Uint16 cx;
	Uint16 ax;
} Registers;

/*
** Prototypes
*/

/*
** _bios_init()
**
** initialize the bios module
*/

void _bios_init(void);

void _bios_call(Uint16 interrupt, Registers *parameters);

#endif

#endif
