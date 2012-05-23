/*
** File:	video.h
**
** Author:	Sean Congden
**
** Description:	A video module for initializing and requesting a
**   a frame buffer to draw on.
*/

#ifndef _VIDEO_H
#define _VIDEO_H

#include "types.h"

/*
** General C definitions
*/

#define FRAMEBUFFER_ADDRESS			0xc8000000

/*
** Types
*/

/* A video display to draw on */
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

/* The default screen to draw on */
extern Screen *kScreen;


/*
** Prototypes
*/

/*
** _video_init()
**
** Iterates through the video modes available and chooses
** a supported video mode closest to the provided resolution
**
** returns:
**      success of the operation
*/
Status _video_init(void);


/*
** _video_aquire_buffer(screen)
**
** Aquires access to the frame buffer
**
** returns:
**      a pointer to the frame buffer to draw on
*/
Uint16 *_video_aquire_buffer(Screen *screen);


/*
** _video_release_buffer
**
** Releases acces to the frame buffer
*/
void _video_release_buffer(Screen *screen);


#endif
