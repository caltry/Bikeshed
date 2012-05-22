/*
** File:	video.h
**
** Author:	Sean Congden
**
** Description:	Generic video module
*/

#ifndef _VIDEO_H
#define _VIDEO_H

#include "types.h"

/*
** General C definitions
*/

#define FRAMEBUFFER_ADDRESS			0xc2000000

/*
** Types
*/

typedef struct screen {
	Uint16 *frame_buffer;
	Uint16 *back_buffer;
	Sem *buffer_lock;

	Uint16 width;
	Uint16 height;
	Uint16 bpp;
	Uint16 pitch;
	Uint32 size;
} Screen;

/*
** Globals
*/

extern Screen *kScreen;

/*
** Prototypes
*/

Status _video_init(void);

Uint16 *_video_aquire_buffer(Screen *screen);
void _video_release_buffer(Screen *screen);

void _video_run(void);

#endif
