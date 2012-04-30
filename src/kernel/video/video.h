/*
** File:	video.h
**
** Author:	Sean Congden
**
** Description:	Generic video module
*/

#ifndef _VIDEO_H
#define _VIDEO_H

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

#include "headers.h"

/*
** Start of C-only definitions
*/

/*
** Types
*/

typedef struct screen {
	Uint16 *frame_buffer;
	Uint16 width;
	Uint16 height;
	Uint16 bpp;
	Uint16 pitch;
	Uint32 size;
} Screen;

/*
** Globals
*/

/*
** Prototypes
*/

Status _video_init(void);

#endif

#endif
