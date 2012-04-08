/*
** File:	vesa.h
**
** Author:	Sean Congden
**
** Description:	VESA support routines and bios calls
*/

#ifndef _VESA_H
#define _VESA_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#define VESA_INFO_ADDRESS	0x00004000

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

typedef short VesaFarPtr;

struct vesa_controller_info {
	char signature[4];
	Int16 version;
	VesaFarPtr oemString[2];
	Uint8 capabilities[4];
	VesaFarPtr videoModes[2];
	Uint8 totalMemory;
};

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

void print_vesa_info(void);

#endif

#endif
