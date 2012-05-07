/*
 * Virtual Filesystem Interface
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>
 * Fri, 13 Apr 2012
 */

#ifndef VFS_H
#define VFS_H

#include <types.h>

typedef enum {
	E_OK		= 0,	// No error
	E_NOFILE	= 1,	// File not found
	E_BADFD		= 2,	// Not a valid file descriptor
	E_IO		= 3,	// An I/O error occurred.
} VFSStatus;

typedef Uint32 FileDescriptor;


/******************************
 * File creation and deletion.
 ******************************/

typedef Uint32 FileMode;

/*
 * Open a new file, with a fully-qualified _name_. Set the _permissions_
 * according to the unix octal permissions.
 * 
 * Returns status codes:
 * 	E_OK No error
 */
VFSStatus create( const char *name, FileMode permissions );

/*
 * Delete a file, given by the fully-qualified _name_.
 *
 * Return E_OK on success
 */
VFSStatus unlink( const char *name );


/******************************************
 * Creating and deleting file descriptors.
 ******************************************/

typedef enum {
	OPEN_RO	= 1,	// Open for reading only
	OPEN_WO	= 2,	// Open for writing only
	OPEN_RW	= 3,	// Open for reading and writing
} OpenMode;

/*
 * Open an already created file, by its fully-qualified _name_. On success, it
 * sets _fd_ to a valid file descriptor for the open file.
 *
 * Returns:
 * 	E_OK		If the file was opened properly
 * 	E_NOFILE	If the file doesn't exist (you must create() it first)
 */
VFSStatus open( const char *name, OpenMode oflags, FileDescriptor *fd );

/*
 * Close an open file by its file descriptor (_fd_).
 */
VFSStatus close( FileDescriptor fd );


/**********************************************
 * Interacting with the contents of the files.
 **********************************************/

/*
 * Read _nbytes_ bytes from a file (_fd_) into a _buf_. Stores the number of
 * _bytes_read_.
 *
 * Returns:
 * 	E_OK	If we read any number of bytes.
 * 	E_IO	If there was some I/O error.
 */
VFSStatus read( FileDescriptor fd, void *buf, Uint nbytes, Uint *bytes_read );

/*
 * Write _nbytes_ bytes from a _buf_ to a file described in the file descriptor
 * (_fd_).
 *
 * Returns:
 * 	E_OK	If we wrote any number of bytes.
 * 	E_IO	If there was some I/O error.
 */
VFSStatus write( FileDescriptor fd, const void *buf, Uint nbytes );

#endif // VFS_H
