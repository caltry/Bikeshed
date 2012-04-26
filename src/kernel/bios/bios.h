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
#define	REAL_LOAD_ADDRESS		0x400000

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

/*
** Prototypes
*/

/*
** _bios_init()
**
** initialize the bios module
*/

void _bios_init( void );

#endif

#endif
