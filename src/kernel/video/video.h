/*
** File:	video.h
**
** Author:	Sean Congden
**
** Description:	Generic video module
*/

#ifndef _VIDEO_H
#define _VIDEO_H

#include "headers.h"

/*
** General C definitions
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

Screen *kScreen;

/*
** Prototypes
*/

Status _video_init(void);

#endif