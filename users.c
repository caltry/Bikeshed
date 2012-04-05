/*
** SCCS ID:	@(#)users.c	1.1	4/5/12
**
** File:	user.c
**
** Author:	4003-506 class of 20133
**
** Contributor:
**
** Description:	User routines.
*/

#include "headers.h"

#include "users.h"

/*
** USER PROCESSES
**
** Each is designed to test some facility of the OS:
**
**	User(s)		Tests/Features
**	=======		===============================================
**	A, B, C		Basic operation
**	D		Spawns Z and exits
**	E, F, G		Sleep for different lengths of time
**	H		Doesn't call exit()
**	J		Tries to spawn 2*N_PCBS copies of Y
**	K		Spawns several copies of X, sleeping between
**	L		Spawns several copies of X, preempting between
**	M		Spawns W three times at low priority, reporting PIDs
**	N		Like M, but spawns X at high priority and W at low
**	P		Iterates three times, printing system time
**	Q		Tries to execute a bogus system call (bad code)
**	R		Reading and writing
**	S		Loops forever, sleeping 30 seconds at a time
**	T		Loops, fiddles with priority
**
**	W, X, Y, Z	Print characters (spawned by other processes)
**
** Output from user processes is always alphabetic.  Uppercase 
** characters are "expected" output; lowercase are "erroneous"
** output.
**
** More specific information about each user process can be found in
** the header comment for that function (below).
**
** To spawn a specific user process, uncomment its SPAWN_x
** definition in the user.h header file.
*/

/*
** Prototypes for all one-letter user main routines
*/

void user_a( void ); void user_b( void ); void user_c( void );
void user_d( void ); void user_e( void ); void user_f( void );
void user_g( void ); void user_h( void ); void user_i( void );
void user_j( void ); void user_k( void ); void user_l( void );
void user_m( void ); void user_n( void ); void user_o( void );
void user_p( void ); void user_q( void ); void user_r( void );
void user_s( void ); void user_t( void ); void user_u( void );
void user_v( void ); void user_w( void ); void user_x( void );
void user_y( void ); void user_z( void );

/*
** Users A, B, and C are identical, except for the character they
** print out via write().  Each prints its ID, then loops 30
** times delaying and printing, before exiting.  They also verify
** the status return from write().
*/

void user_a( void ) {
	int i, j;
	Status status;

	status = write( 'A' );
	if( status != SUCCESS ) {
		prt_status( "User A, write 1 status %s\n", status );
	}
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		status = write( 'A' );
		if( status != SUCCESS ) {
			prt_status( "User A, write 2 status %s\n", status );
		}
	}

	c_puts( "User A exiting\n" );
	exit();

	status = write( 'a' );	/* shouldn't happen! */
	if( status != SUCCESS ) {
		prt_status( "User A, write 3 status %s\n", status );
	}

}

void user_b( void ) {
	int i, j;
	Status status;

	c_puts( "User B running\n" );
	status = write( 'B' );
	if( status != SUCCESS ) {
		prt_status( "User B, write 1 status %s\n", status );
	}
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		status = write( 'B' );
		if( status != SUCCESS ) {
			prt_status( "User B, write 2 status %s\n", status );
		}
	}

	c_puts( "User B exiting\n" );
	exit();

	status = write( 'b' );	/* shouldn't happen! */
	if( status != SUCCESS ) {
		prt_status( "User B, write 3 status %s\n", status );
	}

}

void user_c( void ) {
	int i, j;
	Status status;

	c_puts( "User C running\n" );
	status = write( 'C' );
	if( status != SUCCESS ) {
		prt_status( "User C, write 1 status %s\n", status );
	}
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		status = write( 'C' );
		if( status != SUCCESS ) {
			prt_status( "User C, write 2 status %s\n", status );
		}
	}

	c_puts( "User C exiting\n" );
	exit();

	status = write( 'c' );	/* shouldn't happen! */
	if( status != SUCCESS ) {
		prt_status( "User C, write 3 status %s\n", status );
	}

}


/*
** User D spawns user Z.
*/

void user_d( void ) {
	Status status;
	Pid pid;

	c_puts( "User D running\n" );
	write( 'D' );
	status = spawn( &pid, user_z );
	if( status != SUCCESS ) {
		prt_status( "User D, exec of Z status %s\n", status );
	}
	write( 'D' );

	c_puts( "User D exiting\n" );
	exit();

}


/*
** Users E, F, and G test the sleep facility.
**
** User E sleeps for 10 seconds at a time.
*/

void user_e( void ) {
	int i;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User E get_pid status %s\n", status );
	}
	c_printf( "User E (%d) running\n", pid );
	write( 'E' );
	for( i = 0; i < 5 ; ++i ) {
		sleep( 10 );
		write( 'E' );
	}

	c_puts( "User E exiting\n" );
	exit();

}


/*
** User F sleeps for 5 seconds at a time.
*/

void user_f( void ) {
	int i;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User F get_pid status %s\n", status );
	}
	c_printf( "User F (%d) running\n", pid );
	write( 'F' );
	for( i = 0; i < 5 ; ++i ) {
		sleep( 5 );
		write( 'F' );
	}

	c_puts( "User F exiting\n" );
	exit();

}


/*
** User G sleeps for 15 seconds at a time.
*/

void user_g( void ) {
	int i;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User G get_pid status %s\n", status );
	}
	c_printf( "User G (%d) running\n", pid );
	write( 'G' );
	for( i = 0; i < 5; ++i ) {
		sleep( 15 );
		write( 'G' );
	}

	c_puts( "User G exiting\n" );
	exit();

}


/*
** User H is like A-C except it only loops 5 times and doesn't
** call exit().
*/

void user_h( void ) {
	int i, j;

	c_puts( "User H running\n" );
	write( 'H' );
	for( i = 0; i < 5; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'H' );
	}

	c_puts( "User H returning without exiting!\n" );

}


/*
** User J tries to spawn 2*N_PCBS copies of user_y.
*/

void user_j( void ) {
	int i;
	Pid pid;
	Status status;

	c_puts( "User J running\n" );
	write( 'J' );

	for( i = 0; i < N_PCBS * 2 ; ++i ) {
		status = spawn( &pid, user_y );
		if( status != SUCCESS ) {
			prt_status( "User J, exec of y status %s\n", status );
		} else {
			write( 'J' );
		}
	}

	c_puts( "User J exiting\n" );
	exit();

}


/*
** User K prints, goes into a loop which runs three times, and exits.
** In the loop, it does a spawn of user_x, sleeps 30 seconds, and prints.
*/

void user_k( void ) {
	int i;
	Pid pid;
	Status status;

	c_puts( "User K running\n" );
	write( 'K' );

	for( i = 0; i < 3 ; ++i ) {
		status = spawn( &pid, user_x );
		if( status != SUCCESS ) {
			prt_status( "User K, user x exec status %s\n", status );
		} else {
			sleep( 30 );
			write( 'K' );
		}
	}

	c_puts( "User K exiting\n" );
	exit();

}


/*
** User L is like user K, except that it prints times and doesn't sleep
** each time, it just preempts itself.
*/

void user_l( void ) {
	int i;
	Pid pid;
	Time time;
	Status status;

	c_puts( "User L running, " );
	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "User L, get time status %s\n", status );
	} else {
		c_printf( " initial time %u\n", time );
	}
	write( 'L' );

	for( i = 0; i < 3 ; ++i ) {
		status = get_time( &time );
		if( status != SUCCESS ) {
			prt_status( "User L, get_time status %s\n", status );
		}
		status = spawn( &pid, user_x );
		if( status != SUCCESS ) {
			prt_status( "User L, user_x spawn status %s\n", status );
		} else {
			sleep( 30 );
			write( 'L' );
		}
	}

	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "User L, exiting get time status %s\n", status );
	} else {
		c_printf( "User L exiting at time %u\n", time );
	}
	exit();

}


/*
** User M iterates spawns three copies of user W at low priority,
** reporting their PIDs.
*/

void user_m( void ) {
	int i;
	Pid pid;
	Status status;

	c_puts( "User M running\n" );
	for( i = 0; i < 3; ++i ) {
		status = spawnp( &pid, PRIO_LOW, user_w );
		if( status != SUCCESS ) {
			prt_status( "User M, user w exec status %s\n", status );
		} else {
			c_printf( "User M spawned W, PID %d\n", pid );
			write( 'M' );
		}
	}

	c_puts( "User M exiting\n" );
	exit();

}


/*
** User N is like user M, except that it spawns user W at low priority
** and user Z at high priority.
*/

void user_n( void ) {
	int i;
	Pid pid;
	Status status;

	c_puts( "User N running\n" );
	for( i = 0; i < 3; ++i ) {
		status = spawnp( &pid, PRIO_LOW, user_w );
		if( status != SUCCESS ) {
			prt_status( "User N, user w exec status %s\n", status );
		} else {
			c_printf( "User N spawned W, PID %d\n", pid );
			write( 'N' );
		}
		status = spawnp( &pid, PRIO_HIGH, user_z );
		if( status != SUCCESS ) {
			prt_status( "User N, user z exec status %s\n", status );
		} else {
			c_printf( "User N spawned Z, PID %d\n", pid );
			write( 'N' );
		}
	}

	c_puts( "User N exiting\n" );
	exit();

}


/*
** User P iterates three times.  Each iteration sleeps for two seconds,
** then gets and prints the system time.
*/

void user_p( void ) {
	Time time;
	int i;
	Status status;

	c_printf( "User P running, " );
	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "get_time status %s\n", status );
	} else {
		c_printf( " start at %08x\n", time );
	}

	write( 'P' );

	for( i = 0; i < 3; ++i ) {
		sleep( 2 );
		status = get_time( &time );
		if( status != SUCCESS ) {
			prt_status( "get_time status %s\n", status );
		} else {
			c_printf( "User P reporting time %08x\n", time );
		}
		write( 'P' );
	}

	c_printf( "User P exiting\n" );
	exit();

}


/*
** User Q does a bogus system call
*/

void user_q( void ) {

	c_puts( "User Q running\n" );
	write( 'Q' );
	bogus();
	c_puts( "User Q returned from bogus syscall!?!?!\n" );
	exit();

}


/*
** User R loops 3 times reading/writing, then exits.
*/

void user_r( void ) {
	int i;
	int ch = '&';
	Status status;

	c_puts( "User R running\n" );
	sleep( 10 );
	for( i = 0; i < 3; ++i ) {
		do {
			write( 'R' );
			status = read( &ch );
			if( status != SUCCESS ) {
				prt_status( "User R, read status %s\n", status );
			} else if( ch == -1 ) {	/* wait a bit */
				sleep( 1 );
			}
		} while( ch == -1 );
		write( ch );
	}

	c_puts( "User R exiting\n" );
	exit();

}


/*
** User S sleeps for 30 seconds at a time, and loops forever.
*/

void user_s( void ) {

	c_puts( "User S running\n" );
	write( 'S' );
	for(;;) {
		sleep( 30 );
		write( 'S' );
	}

	c_puts( "User S exiting!?!?!n" );
	exit();

}


/*
** User T changes its priority back and forth several times
*/

void user_t( void ) {
	Prio priority, prio2;
	int i, j;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User T get_pid status %s\n", status );
	}
	status = get_priority( &priority );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio status %s\n", status );
	} else {
		c_printf( "User T (%d) running, initial prio %d\n",
		  	  pid, priority );
	}
	
	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	status = set_priority( PRIO_HIGH );
	if( status != SUCCESS ) {
		write( 't' );
		prt_status( "User T, set prio #1 status %s\n", status );
	}
	status = get_priority( &prio2 );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio2 #1 status %s\n", status );
	} else {
		c_printf( "User T, prio was %d now %d\n", priority, prio2 );
	}
	priority = prio2;
	
	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	status = set_priority( PRIO_LOW );
	if( status != SUCCESS ) {
		write( 't' );
		prt_status( "User T, set prio #2 status %s\n", status );
	}
	status = get_priority( &prio2 );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio2 #2 status %s\n", status );
	} else {
		c_printf( "User T, prio was %d now %d\n", priority, prio2 );
	}
	priority = prio2;

	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	status = set_priority( PRIO_STD );
	if( status != SUCCESS ) {
		write( 't' );
		prt_status( "User T, set prio #3 status %s\n", status );
	}
	status = get_priority( &prio2 );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio2 #3 status %s\n", status );
	} else {
		c_printf( "User T, prio was %d now %d\n", priority, prio2 );
	}
	priority = prio2;
	
	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	c_puts( "User T exiting\n" );
	exit();

}


/*
** User W prints W characters 20 times, sleeping 3 seconds between.
*/

void user_w( void ) {
	int i;
	Pid pid;
	Status status;

	c_printf( "User W running, " );
	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User W get_pid status %s\n", status );
	}
	c_printf( " PID %d\n", pid );
	for( i = 0; i < 20 ; ++i ) {
		write( 'W' );
		sleep( 3 );
	}

	c_printf( "User W exiting, PID %d\n", pid );
	exit();

}


/*
** User X prints X characters 20 times.  It is spawned multiple
** times, and prints its PID when started and exiting.
*/

void user_x( void ) {
	int i, j;
	Pid pid;
	Status status;

	c_puts( "User X running, " );
	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User X get_pid status %s\n", status );
	}
	c_printf( "PID %d, ", pid );

	for( i = 0; i < 20 ; ++i ) {
		write( 'X' );
		for( j = 0; j < DELAY_STD; ++j )
			continue;
	}

	c_printf( "User X exiting, PID %d\n", pid );
	exit();

}


/*
** User Y prints Y characters 10 times.
*/

void user_y( void ) {
	int i, j;

	c_puts( "User Y running\n" );
	for( i = 0; i < 10 ; ++i ) {
		write( 'Y' );
		for( j = 0; j < DELAY_ALT; ++j )
			continue;
		sleep( 1 );
	}

	c_puts( "User Y exiting\n" );
	exit();

}


/*
** User Z prints Z characters 10 times.
*/

void user_z( void ) {
	int i, j;

	c_puts( "User Z running\n" );
	for( i = 0; i < 10 ; ++i ) {
		write( 'Z' );
		for( j = 0; j < DELAY_STD; ++j )
			continue;
	}

	c_puts( "User Z exiting\n" );
	exit();

}


/*
** SYSTEM PROCESSES
*/


/*
** Initial process; it starts the other top-level user processes.
*/

void init( void ) {
	int i;
	Pid pid;
	Time time;
	Status status;

	c_puts( "Init started\n" );

	write( '$' );

	// we'll start the first three "manually"
	// by doing fork() and exec() ourselves

#ifdef SPAWN_A
	status = fork( &pid );
	if( status != SUCCESS ) {
		prt_status( "init: can't fork() user A, status %s\n", status );
	} else if( pid == 0 ) {
		status = exec( user_a );
		prt_status( "init: can't exec() user A, status %s\n", status );
		exit();
	}
#endif

#ifdef SPAWN_B
	status = fork( &pid );
	if( status != SUCCESS ) {
		prt_status( "init: can't fork() user B, status %s\n", status );
	} else if( pid == 0 ) {
		status = exec( user_b );
		prt_status( "init: can't exec() user B, status %s\n", status );
		exit();
	}
#endif

#ifdef SPAWN_C
	status = fork( &pid );
	if( status != SUCCESS ) {
		prt_status( "init: can't fork() user C, status %s\n", status );
	} else if( pid == 0 ) {
		status = exec( user_c );
		prt_status( "init: can't exec() user C, status %s\n", status );
		exit();
	}
#endif

	// for most of the rest, we'll use spawn()

#ifdef SPAWN_D
	status = spawn( &pid, user_d );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user D, status %s\n", status );
	}
#endif

#ifdef SPAWN_E
	status = spawn( &pid, user_e );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user E, status %s\n", status );
	}
#endif

#ifdef SPAWN_F
	status = spawn( &pid, user_f );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user F, status %s\n", status );
	}
#endif

#ifdef SPAWN_G
	status = spawn( &pid, user_g );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user G, status %s\n", status );
	}
#endif

#ifdef SPAWN_H
	status = spawn( &pid, user_h );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user H, status %s\n", status );
	}
#endif

#ifdef SPAWN_J
	status = spawn( &pid, user_j );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user J, status %s\n", status );
	}
#endif

#ifdef SPAWN_K
	status = spawn( &pid, user_k );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user K, status %s\n", status );
	}
#endif

#ifdef SPAWN_L
	status = spawn( &pid, user_l );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user L, status %s\n", status );
	}
#endif

#ifdef SPAWN_M
	status = spawn( &pid, user_m );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user N, status %s\n", status );
	}
#endif

#ifdef SPAWN_N
	status = spawn( &pid, user_n );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user N, status %s\n", status );
	}
#endif

#ifdef SPAWN_P
	status = spawn( &pid, user_p );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user P, status %s\n", status );
	}
#endif

#ifdef SPAWN_Q
	status = spawn( &pid, user_q );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user Q, status %s\n", status );
	}
#endif

#ifdef SPAWN_R
	status = spawn( &pid, user_r );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user R, status %s\n", status );
	}
#endif

#ifdef SPAWN_S
	status = spawn( &pid, user_s );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user S, status %s\n", status );
	}
#endif

#ifdef SPAWN_T
	status = spawn( &pid, user_t );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user T, status %s\n", status );
	}
#endif

	write( '!' );

	/*
	** And now we start twiddling our thumbs
	*/

	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "idle: get_time status %s\n", status );
	}
	c_printf( "init => idle at time %08x\n", time );

	status = set_priority( PRIO_IDLE );
	if( status != SUCCESS ) {
		prt_status( "idle: priority change status %s\n", status );
	}

	write( '.' );

	for(;;) {
		for( i = 0; i < DELAY_LONG; ++i )
			continue;
		write( '.' );
	}

	/*
	** SHOULD NEVER REACH HERE
	*/

	c_printf( "*** IDLE IS EXITING???\n" );
	exit();

}
