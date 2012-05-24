/*
** SCCS ID:	@(#)ulibc.c	1.1	4/5/12
**
** File:	ulibc.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	C implementations of user-level library functions
*/

#include "defs.h"
#include "ulib.h"

#define c_printf(...)
#define prt_status(...)

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

// Status value strings
//
// this is identical to the kernel _kstatus_strings array,
// but is separate to simplify life for people implementing VM.

const char *ustatus_strings[] = {
	"success",		/* SUCCESS */
	"failure",		/* FAILURE */
	"bad parameter",	/* BAD_PARAM */
	"empty queue",		/* EMPTY_QUEUE */
	"not empty queue",	/* NOT_EMPTY_QUEUE */
	"allocation failed",	/* ALLOC_FAILED */
	"not found",		/* NOT_FOUND */
	"no queues",		/* NO_QUEUES */
	"bad priority",		/* BAD_PRIO */
	"feature unimplemented" /* FEATURE_UNIMPLEMENTED */
};

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** prt_status - print a status value to the console
**
** the 'msg' argument should contain a %s where
** the desired status value should be printed
*/

/*
void prt_status( char *msg, Status stat ) {

	if( msg == NULL ) {
		return;
	}

	c_printf( msg, ustatus(stat) );

	if( stat >= STATUS_SENTINEL ) {
		c_printf( "bad code: %d", stat );
	}

}
*/

/*
** spawnp - create a new process running a different program
**		at a specific priority
**
** usage:  status = spawnp( &pid, prio, "/path/to/program" );
**
** returns the PID of the child via the 'pid' parameter on
** success, and the status of the creation attempt
*/

Status spawnp( Pid *pid, Prio prio, const char* program_path ) {
	Pid new;
	Status status, status2;

	// create the process
	status = fork( &new );

	// if that failed, return failure status
	if( status != SUCCESS ) {
		return( status );
	}

	// we have a child; it should do the exec(),
	// and the parent should see success

	if( new == 0 ) {	// we're the child
		// change the process priority
		status = set_priority( prio );
		if( status != SUCCESS ) {
			status2 = get_pid( &new );
			if( status2 == SUCCESS ) {
			    c_printf( "Child pid %d", new );
			    prt_status( ", set_priority() status %s\n", status );
			} else {
			    c_printf( "Unable to set_priority() or get pid of"
					"%d's child", pid );
			    prt_status( ", get_pid's status: %s\n", status2 );
			}
			exit();
		}
		status = exec( program_path );
		// if we got here, the exec() failed
		status2 = get_pid( &new );
		if( status2 == SUCCESS ) {
		    c_printf( "Child pid %d", new );
		    prt_status( ", exec() status %s\n", status );
		} else {
		    c_printf( "spawn(), exec failed. Unable to get the pid"
				"of %d's child.", pid );
		    prt_status( ", exec() status %s\n", status );
		}
		exit();
	}

	*pid = new;
	return( SUCCESS );

}

/*
** spawn - create a new process running a different program
**		at standard priority
**
** usage:  status = spawn( &pid, "/path/to/program" );
**
** returns the PID of the child via the 'pid' parameter on
** success, and the status of the creation attempt
*/

Status spawn( Pid *pid, const char* program_path ) {

	// take the easy way out

	return( spawnp(pid,PRIO_STD,program_path) );

}


/* begin writef stuff */
void serial_string(const char *out)
{
	while (*out != 0)
	{
		write(*out);
		++out;
	}
}

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
		write( padchar );
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


static void __serial_printf(const char **f);

void writef(const char *fmt, ...)
{
	__serial_printf(&fmt);	
}

static void __serial_printf(const char **f)
{
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
			write( ch );
		}
	}
}