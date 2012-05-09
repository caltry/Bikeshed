/*
** File:	gconsole.c
**
** Author:	K. Reek
**
** Contributor:	Jon Coles, Warren R. Carithers, Sean Congden
**
** Description:	Graphical console output routines
**
**	This module implements a simple set of output routines that
**  creates a graphical console application.
**
*/

#include "video.h"
#include "graphics.h"

#include "gconsole.h"

/*
** Video parameters, and state variables
*/
#define	SCREEN_X_SIZE	80
#define	SCREEN_Y_SIZE	25

#define	SCREEN_MIN_X	0
#define	SCREEN_MIN_Y	0
#define	SCREEN_MAX_X	SCREEN_X_SIZE
#define	SCREEN_MAX_Y	SCREEN_Y_SIZE

unsigned int	scroll_min_x, scroll_min_y;
unsigned int	scroll_max_x, scroll_max_y;
unsigned int	cursor_x, cursor_y;
unsigned int	min_x, min_y;
unsigned int	max_x, max_y;

char screen[SCREEN_X_SIZE * SCREEN_Y_SIZE];

#define	SCREEN_ADDR(x,y)	( screen + x + ( y * SCREEN_X_SIZE ) )

/*
** Support routines.
**
** bound: confine an argument within given bounds
** __c_putchar_at: physical output to the video memory
** __c_setcursor: set the cursor location (screen coordinates)
** __c_strlen: compute the length of a string
*/
static unsigned int bound( unsigned int min, unsigned int value, unsigned int max ){
	if( value < min ){
		value = min;
	}
	if( value > max ){
		value = max;
	}
	return value;
}

static unsigned int __gcon_strlen( char const *str ){
	unsigned int	len = 0;

	while( *str++ != '\0' ){
		len += 1;
	}
	return len;
}

static void __gcon_putchar_at( unsigned int x, unsigned int y, char c ){
	/*
	** If x or y is too big or small, don't do any output.
	*/
	if( x <= max_x && y <= max_y ) {
		char *to = SCREEN_ADDR(x,y);
		*to = c;
	}
}

void gcon_setscroll( unsigned int s_min_x, unsigned int s_min_y, unsigned int s_max_x, unsigned int s_max_y ){
	scroll_min_x = bound( min_x, s_min_x, max_x );
	scroll_min_y = bound( min_y, s_min_y, max_y );
	scroll_max_x = bound( scroll_min_x, s_max_x, max_x );
	scroll_max_y = bound( scroll_min_y, s_max_y, max_y );
	cursor_x = scroll_min_x;
	cursor_y = scroll_min_y;
}

/*
** Cursor movement in the scroll region
*/
void gcon_moveto( unsigned int x, unsigned int y ){
	cursor_x = bound( scroll_min_x, x + scroll_min_x, scroll_max_x );
	cursor_y = bound( scroll_min_y, y + scroll_min_y, scroll_max_y );
}

/*
** The putchar family
*/
static void __gcon_putchar( char c ) {
	/*
	** If we're off the bottom of the screen, scroll the window.
	*/
	if( cursor_y > scroll_max_y ){
		gcon_scroll( cursor_y - scroll_max_y );
		cursor_y = scroll_max_y;
	}

	switch( c ){
	case '\n':
		/*
		** Erase to the end of the line, then move to new line
		** (actual scroll is delayed until next output appears).
		*/
		while( cursor_x <= scroll_max_x ){
			__gcon_putchar_at( cursor_x, cursor_y, ' ' );
			cursor_x += 1;
		}
		cursor_x = scroll_min_x;
		cursor_y += 1;
		break;

	case '\r':
		cursor_x = scroll_min_x;
		break;

	default:
		__gcon_putchar_at( cursor_x, cursor_y, c );
		cursor_x += 1;
		if( cursor_x > scroll_max_x ){
			cursor_x = scroll_min_x;
			cursor_y += 1;
		}
		break;
	}
}

void gcon_putchar( char c ) {
	__gcon_putchar(c);
	gconsole_flush();
}

void gcon_putchar_at( unsigned int x, unsigned int y, char c ){
	if( c == '\n' ){
		unsigned int	limit;

		/*
		** If we're in the scroll region, don't let this loop
		** leave it.  If we're not in the scroll region, don't
		** let this loop enter it.
		*/
		if( x > scroll_max_x ){
			limit = max_x;
		}
		else if( x >= scroll_min_x ){
			limit = scroll_max_x;
		}
		else {
			limit = scroll_min_x - 1;
		}
		while( x <= limit ){
			__gcon_putchar_at( x, y, ' ' );
			x += 1;
		}
	}
	else {
		__gcon_putchar_at( x, y, c );
	}

	gconsole_flush();
}

static void __gcon_puts( char *str ){
	char	ch;

	while( (ch = *str++) != '\0' ){
		__gcon_putchar( ch );
	}
}

void gcon_puts( char *str ){
	__gcon_puts(str);
	gconsole_flush();
}

/*
** The puts family
*/
static void __gcon_puts_at( unsigned int x, unsigned int y, char *str ){
	char ch;

	while( (ch = *str++) != '\0' && x <= max_x ){
		gcon_putchar_at( x, y, ch );
		x += 1;
	}
}

void gcon_puts_at( unsigned int x, unsigned int y, char *str ){
	__gcon_puts_at(x, y, str);
	gconsole_flush();
}

void gcon_clearscroll( void ){
	unsigned int	nchars = scroll_max_x - scroll_min_x + 1;
	unsigned int	l;
	unsigned char	c;

	for( l = scroll_min_y; l <= scroll_max_y; l += 1 ){
		char *to = SCREEN_ADDR( scroll_min_x, l );

		for( c = 0; c < nchars; c += 1 ){
			*to++ = ' ';
		}
	}

	gconsole_flush();
}

void gcon_clearscreen( void ){
	char			*to = SCREEN_ADDR( min_x, min_y );
	unsigned int	nchars = ( max_y - min_y + 1 ) * ( max_x - min_x + 1 );

	while( nchars > 0 ){
		*to++ = ' ';
		nchars -= 1;
	}

	gconsole_flush();
}


void gcon_scroll( unsigned int lines ){
	char 			*from, *to;
	unsigned int	nchars = scroll_max_x - scroll_min_x + 1;
	unsigned int	line;
	unsigned char	c;

	/*
	** If # of lines is the whole scrolling region or more, just clear.
	*/
	if( lines > scroll_max_y - scroll_min_y ){
		gcon_clearscroll();
		cursor_x = scroll_min_x;
		cursor_y = scroll_min_y;
		return;
	}

	/*
	** Must copy it line by line.
	*/
	for( line = scroll_min_y; line <= scroll_max_y - lines; line += 1 ){
		from = SCREEN_ADDR( scroll_min_x, line + lines );
		to = SCREEN_ADDR( scroll_min_x, line );
		for( c = 0; c < nchars; c += 1 ){
			*to++ = *from++;
		}
	}

	for( ; line <= scroll_max_y; line += 1 ){
		to = SCREEN_ADDR( scroll_min_x, line );
		for( c = 0; c < nchars; c += 1 ){
			*to++ = ' ';
		}
	}

	gconsole_flush();
}

static char * cvtdec0( char *buf, int value ){
	int	quotient;

	quotient = value / 10;
	if( quotient < 0 ){
		quotient = 214748364;
		value = 8;
	}
	if( quotient != 0 ){
		buf = cvtdec0( buf, quotient );
	}
	*buf++ = value % 10 + '0';
	return buf;
}

static int cvtdec( char *buf, int value ){
	char	*bp = buf;

	if( value < 0 ){
		*bp++ = '-';
		value = -value;
	}
	bp = cvtdec0( bp, value );
	*bp = '\0';

	return bp - buf;
}

static char hexdigits[] = "0123456789ABCDEF";

static int cvthex( char *buf, int value ){
	int	i;
	int	chars_stored = 0;
	char	*bp = buf;

	for( i = 0; i < 8; i += 1 ){
		int	val;

		val = ( value & 0xf0000000 );
		if( i == 7 || val != 0 || chars_stored ){
			chars_stored = 1;
			val >>= 28;
			val &= 0xf;
			*bp++ = hexdigits[ val ];
		}
		value <<= 4;
	}
	*bp = '\0';

	return bp - buf;
}

static int cvtoct( char *buf, int value ){
	int	i;
	int	chars_stored = 0;
	char	*bp = buf;
	int	val;

	val = ( value & 0xc0000000 );
	val >>= 30;
	for( i = 0; i < 11; i += 1 ){

		if( i == 10 || val != 0 || chars_stored ){
			chars_stored = 1;
			val &= 0x7;
			*bp++ = hexdigits[ val ];
		}
		value <<= 3;
		val = ( value & 0xe0000000 );
		val >>= 29;
	}
	*bp = '\0';

	return bp - buf;
}

static int pad( int x, int y, int extra, int padchar ){
	while( extra > 0 ){
		if( x != -1 || y != -1 ){
			gcon_putchar_at( x, y, padchar );
			x += 1;
		}
		else {
			gcon_putchar( padchar );
		}
		extra -= 1;
	}
	return x;
}

static int padstr( int x, int y, char *str, int len, int width, int leftadjust, int padchar ){
	int	extra;

	if( len < 0 ){
		len = __gcon_strlen( str );
	}
	extra = width - len;
	if( extra > 0 && !leftadjust ){
		x = pad( x, y, extra, padchar );
	}
	if( x != -1 || y != -1 ){
		gcon_puts_at( x, y, str );
		x += len;
	}
	else {
		gcon_puts( str );
	}
	if( extra > 0 && leftadjust ){
		x = pad( x, y, extra, padchar );
	}
	return x;
}

static void __gcon_do_printf( int x, int y, const char **f ){
	const char	*fmt = *f;
	int	*ap;
	char	buf[ 12 ];
	char	ch;
	char	*str;
	int	leftadjust;
	int	width;
	int	len;
	int	padchar;

	/*
	** Get characters from the format string and process them
	*/
	ap = (int *)( f + 1 );
	while( (ch = *fmt++) != '\0' ){
		/*
		** Is it the start of a format code?
		*/
		if( ch == '%' ){
			/*
			** Yes, get the padding and width options (if there).
			** Alignment must come at the beginning, then fill,
			** then width.
			*/
			leftadjust = 0;
			padchar = ' ';
			width = 0;
			ch = *fmt++;
			if( ch == '-' ){
				leftadjust = 1;
				ch = *fmt++;
			}
			if( ch == '0' ){
				padchar = '0';
				ch = *fmt++;
			}
			while( ch >= '0' && ch <= '9' ){
				width *= 10;
				width += ch - '0';
				ch = *fmt++;
			}

			/*
			** What data type do we have?
			*/
			switch( ch ){
			case 'c':
				// ch = *( (int *)ap )++;
				ch = *ap++;
				buf[ 0 ] = ch;
				buf[ 1 ] = '\0';
				x = padstr( x, y, buf, 1, width, leftadjust, padchar );
				break;

			case 'd':
				// len = cvtdec( buf, *( (int *)ap )++ );
				len = cvtdec( buf, *ap++ );
				x = padstr( x, y, buf, len, width, leftadjust, padchar );
				break;

			case 's':
				// str = *( (char **)ap )++;
				str = (char *) (*ap++);
				x = padstr( x, y, str, -1, width, leftadjust, padchar );
				break;

			case 'x':
				// len = cvthex( buf, *( (int *)ap )++ );
				len = cvthex( buf, *ap++ );
				x = padstr( x, y, buf, len, width, leftadjust, padchar );
				break;

			case 'o':
				// len = cvtoct( buf, *( (int *)ap )++ );
				len = cvtoct( buf, *ap++ );
				x = padstr( x, y, buf, len, width, leftadjust, padchar );
				break;

			}
		}
		else {
			if( x != -1 || y != -1 ){
				__gcon_putchar_at( x, y, ch );
				switch( ch ){
				case '\n':
					y += 1;
					/* FALL THRU */

				case '\r':
					x = scroll_min_x;
					break;

				default:
					x += 1;
				}
			}
			else {
				__gcon_putchar( ch );
			}
		}
	}
}

void gcon_printf_at( unsigned int x, unsigned int y, const char *fmt, ... ){
	__gcon_do_printf( x, y, &fmt );
	gconsole_flush();
}

void gcon_printf( const char *fmt, ... ){
	__gcon_do_printf( -1, -1, &fmt );
	gconsole_flush();
}

/*
** Initialization routines
*/
void gconsole_init( void ){
	/*
	** Screen dimensions
	*/
	min_x  = SCREEN_MIN_X;	
	min_y  = SCREEN_MIN_Y;
	max_x  = SCREEN_MAX_X;
	max_y  = SCREEN_MAX_Y;

	/*
	** Scrolling region
	*/
	scroll_min_x = SCREEN_MIN_X;
	scroll_min_y = SCREEN_MIN_Y;
	scroll_max_x = SCREEN_MAX_X;
	scroll_max_y = SCREEN_MAX_Y;

	/*
	** Initial cursor location
	*/
	cursor_y = min_y;
	cursor_x = min_x;
}


void gconsole_flush(void) {
	gconsole_draw(0, 0);
}


void gconsole_draw(unsigned int x, unsigned int y) {
	if (kScreen != NULL) {
		// Draw the window background
		fill_rect(kScreen, x, y, SCREEN_MAX_X * 12, SCREEN_MAX_Y * 16, 0x3C5D5F);
		
		char *start = screen;
		char *end = screen + SCREEN_MAX_X;

		while( end != (screen + (SCREEN_MAX_X * SCREEN_MAX_Y))) {
			draw_chars(kScreen, start, end, x, y, 2, 0xD3D9C3);
			start += SCREEN_MAX_X;
			end += SCREEN_MAX_X;
			y += 16;
		}
	}
}

