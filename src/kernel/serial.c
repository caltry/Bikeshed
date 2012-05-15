#include "serial.h"
#include "startup.h"
#include "defs.h"

#define SERIAL_PORT_A 0x3F8

void serial_install(void)
{
#ifdef QEMU_SERIAL
	__outb(SERIAL_PORT_A + 1, 0x00);
	__outb(SERIAL_PORT_A + 3, 0x80);
	__outb(SERIAL_PORT_A + 0, 0x03);
	__outb(SERIAL_PORT_A + 1, 0x00);
	__outb(SERIAL_PORT_A + 3, 0x03);
	__outb(SERIAL_PORT_A + 2, 0xC7);
	__outb(SERIAL_PORT_A + 4, 0x08);
#endif
	
}

int serial_transmit_empty(void)
{
#ifdef QEMU_SERIAL
	return __inb(SERIAL_PORT_A + 5) & 0x20;
#else
	return 0;
#endif
}

void serial_char(char out)
{
#ifdef QEMU_SERIAL
	while (serial_transmit_empty() == 0);
	__outb(SERIAL_PORT_A, out);
#else
	UNUSED(out);
#endif
}

void serial_string(const char *out)
{
#ifdef QEMU_SERIAL
	while (*out != 0)
	{
		serial_char(*out);
		++out;
	}
#else
	UNUSED(out);
#endif
}

#ifdef QEMU_SERIAL
static char* cvtdec0( char *buf, int value ){
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

static int pad(int extra, int padchar){
	while( extra > 0 ){
		serial_char( padchar );
		extra -= 1;
	}

	return 0;
}

static unsigned int __c_strlen( char const *str ){
	unsigned int	len = 0;

	while( *str++ != '\0' ){
		len += 1;
	}
	return len;
}

static int padstr(char *str, int len, int width, int leftadjust, int padchar){
	int	extra;
	int x = 0;

	if( len < 0 ){
		len = __c_strlen( str );
	}
	extra = width - len;
	if( extra > 0 && !leftadjust ){
		x = pad(extra, padchar);
	}

	serial_string( str );

	if( extra > 0 && leftadjust ){
		x = pad(extra, padchar);
	}
	return x;
}
#endif

static void __serial_printf(const char **f);

void serial_printf(const char *fmt, ...)
{
	__serial_printf(&fmt);	
}

static void __serial_printf(const char **f)
{
#ifdef QEMU_SERIAL
	const char *fmt = *f;
	int	 *ap;
	char buf[ 12 ];
	char ch;
	char *str;
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
				padstr(buf, 1, width, leftadjust, padchar );
				break;

			case 'd':
				// len = cvtdec( buf, *( (int *)ap )++ );
				len = cvtdec( buf, *ap++ );
				padstr(buf, len, width, leftadjust, padchar );
				break;

			case 's':
				// str = *( (char **)ap )++;
				str = (char *) (*ap++);
				padstr(str, -1, width, leftadjust, padchar );
				break;

			case 'x':
				// len = cvthex( buf, *( (int *)ap )++ );
				len = cvthex( buf, *ap++ );
				padstr(buf, len, width, leftadjust, padchar );
				break;

			case 'o':
				// len = cvtoct( buf, *( (int *)ap )++ );
				len = cvtoct( buf, *ap++ );
				padstr(buf, len, width, leftadjust, padchar );
				break;

			}
		}
		else {
			serial_char( ch );
		}
	}
#else
	UNUSED(f);
#endif
}
