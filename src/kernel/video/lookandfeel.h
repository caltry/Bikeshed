/*
** File:	lookandfeel.h
**
** Author:	Sean Congden
**
** Description:	Defines for the GUI look and feel
*/

#ifndef _LOOK_AND_FEEL_H
#define _LOOK_AND_FEEL_H


// Desktop colors
#define DESKTOP_BACKGROUND_COLOR		0x6490a7

// Window colors
#define WINDOW_TITLE_COLOR_1			0x5a8a37
#define WINDOW_TITLE_COLOR_2			0x4c732e
#define WINDOW_TITLE_COLOR_3			0x689f40
#define WINDOW_TITLE_COLOR_4			0x62963c

#define WINDOW_BASE_COLOR_1				0x354351
#define WINDOW_BASE_COLOR_2				0x2d3843
#define WINDOW_BASE_COLOR_3				0x3d4d5d
#define WINDOW_BASE_COLOR_4				0x3a4857

#define WINDOW_ACTIVE_COLOR				0x284117
#define WINDOW_INACTIVE_COLOR			WINDOW_TITLE_COLOR_2

// Window dimensions
#define WINDOW_TITLE_Y_SIZE				32
#define WINDOW_CLOSE_X_SIZE				16
#define WINDOW_CLOSE_Y_SIZE				4
#define WINDOW_CLOSE_OFFSET				((WINDOW_TITLE_Y_SIZE \
											- WINDOW_CLOSE_Y_SIZE) / 2)

#endif
