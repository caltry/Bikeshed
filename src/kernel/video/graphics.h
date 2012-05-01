/*
** File:	graphics.h
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

#include "headers.h"

#include "video.h"

/*
** Start of C-only definitions
*/

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

void clear_screen(Screen *screen, Uint32 color);

void draw_char(Screen *screen, char letter, int x, int y, Uint32 color);

void draw_string(Screen *screen, char *string, int x, int y, Uint32 color);

void test_pattern(Screen *screen);

void set_pixel(Screen *screen, Uint32 x, Uint32 y, Uint32 color);

void fill_rect(Screen *screen, Uint32 x, Uint32 y, Uint32 width, Uint32 height, Uint32 color);

#endif

#endif
