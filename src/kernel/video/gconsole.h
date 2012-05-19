/*
** File:	gconsole.h
**
** Author:	K. Reek
**
** Contributor:	Jon Coles, Warren R. Carithers, Sean Congden
**
** Description:	Graphical console output routines
**
**	These routines provide a rudimentary capability for printing to
**	a graphical console.
**
** Console output:
**	There are two families of functions.  The first provides a window
**	that behaves in the usual manner: writes extending beyond the right
**	edge of the window wrap around to the next line, the top line
**	scrolls off the window to make room for new lines at the bottom.
**	However, you may choose what part of the screen contains this
**	scrolling window.  This allows you to print some text at fixed
**	locations on the screen while the rest of the screen scrolls.
**
**	The second family allows for printing at fixed locations on the
**	screen.  No scrolling or line wrapping are done for these functions.
**	It is not intended that these functions be used to write in the
**	scrolling area of the screen.
**
**	In both sets of functions, the (x,y) coordinates are interpreted
**	as (column,row), with the upper left corner of the screen being
**	(0,0) and the lower right corner being (79,24).
**
**	The printf provided in both sets of functions has the same
**	conversion capabilities.  Format codes are of the form:
**
**		%-0WC
**
**	where "-", "0", and "W" are all optional:
**	  "-" is the left-adjust flag (default is right-adjust)
**	  "0" is the zero-fill flag (default is space-fill)
**	  "W" is a number specifying the minimum field width (default: 1 )
**	and "C" is the conversion type, which must be one of these:
**	  "c" print a single character
**	  "s" print a null-terminated string
**	  "d" print an integer as a decimal value
**	  "x" print an integer as a hexadecimal value
**	  "o" print an integer as a octal value
*/

#ifndef _GCONSOLE_H_
#define _GCONSOLE_H_


#ifdef __cplusplus

#include "window.h"
#include "desktop.h"


class ConsoleView : public UIComponent {
public:
	ConsoleView(Painter *painter, Rect bounds);

private:
	void Draw(void);
};


class GConsole : public Window {
public:
	GConsole(Desktop *desktop, Uint32 x, Uint32 y);
	~GConsole(void);

private:
	ConsoleView *console;
};


extern GConsole *_gconsole;


extern "C" {
#endif


/*
** Name:	gconsole_init
**
** Description:	Initializes the output routines.
*/
void gcon_init( void );

void gcon_flush(void);


/*****************************************************************************
**
** SCROLLING OUTPUT ROUTINES
**
**	Each operation begins at the current cursor position and advances
**	it.  If a newline is output, the reminder of that line is cleared.
**	Output extending past the end of the line is wrapped.  If the
**	cursor is moved below the scrolling region's bottom edge, scrolling
**	is delayed until the next output is produced.
*/

/*
** Name:	gcon_setscroll
**
** Description:	This sets the scrolling region to be the area defined by
**		the arguments.  The remainder of the screen does not scroll
**		and may be used to display data you do not want to move.
**		By default, the scrolling region is the entire screen .
** Arguments:	coordinates of upper-left and lower-right corners of region
*/
void gcon_setscroll( unsigned int min_x, unsigned int min_y,
		  unsigned int max_x, unsigned int max_y );

/*
** Name:	gcon_moveto
**
** Description:	Moves the cursor to the specified position. (0,0) indicates
**		the upper left corner of the scrolling region.  Subsequent
**		output will begin at the cursor position.
** Arguments:	desired cursor position
*/
void gcon_moveto( unsigned int x, unsigned int y );

/*
** Name:	gcon_putchar
**
** Description:	Prints a single character.
** Arguments:	the character to be printed
*/
void gcon_putchar( char c );

/*
** Name:	gcon_puts
**
** Description:	Prints the characters in the string up to but not including
**		the terminating null byte.
** Arguments:	pointer to a null-terminated string
*/
void gcon_puts( char *str );

/*
** Name:	gcon_printf
**
** Description:	Limited form of printf (see the beginning of this file for
**		a list of what is implemented).
** Arguments:	printf-style format and optional values
*/
void gcon_printf( const char *fmt, ... );

/*
** Name:	gcon_scroll
**
** Description:	Scroll the scrolling region up by the given number of lines.
**		The output routines scroll automatically so normally you
**		do not need to call this routine yourself.
** Arguments:	number of lines
*/
void gcon_scroll( unsigned int lines );

/*
** Name:	gcon_clearscroll
**
** Description:	Clears the entire scrolling region to blank spaces, and
**		moves the cursor to (0,0).
*/
void gcon_clearscroll( void );

/*****************************************************************************
**
** NON-SCROLLING OUTPUT ROUTINES
**
**	Coordinates are relative to the entire screen: (0,0) is the upper
**	left corner.  There is no line wrap or scrolling.
*/

/*
** Name:	gcon_putchar_at
**
** Description:	Prints the given character.  If a newline is printed,
**		the rest of the line is cleared.  If this happens to the
**		left of the scrolling region, the clearing stops when the
**		region is reached.  If this happens inside the scrolling
**		region, the clearing stops when the edge of the region
**		is reached.
** Arguments:	coordinates, the character to be printed
*/
void gcon_putchar_at( unsigned int x, unsigned int y, char c );

/*
** Name:	gcon_puts_at
**
** Description:	Prints the given string.  c_putchar_at is used to print
**		the individual characters; see that description for details.
** Arguments:	coordinates, null-terminated string to be printed
*/
void gcon_puts_at( unsigned int x, unsigned int y, char *str );

/*
** Name:	gcon_printf_at
**
** Description:	Limited form of printf (see the beginning of this file for
**		a list of what is implemented).
** Arguments:	coordinates, printf-style format, optional values
*/
void gcon_printf_at( unsigned int x, unsigned int y, const char *fmt, ... );

/*
** Name:	gcon_clearscreen
**
** Description:	This function clears the entire screen, including the
**		scrolling region.
*/
void gcon_clearscreen( void );


#ifdef __cplusplus
}
#endif


#endif