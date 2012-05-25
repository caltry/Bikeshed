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
	FS_E_OK         = 0,	// No error
	FS_E_NOFILE     = 1,	// File not found
	FS_E_BADFD      = 2,	// Not a valid file descriptor
	FS_E_IO         = 3,	// An I/O error occurred.
	FS_E_NOT_FQN    = 4,	// The path give is not fully qualified
	FS_E_BAD_OFFSET	= 5,	// Offset is past the end of the file
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
 * Interacting with the contents of files.
 ******************************************/

/*
 *
 * Read _nbytes_ from a file (given its _path_) into a _buf_. Reading begins at
 * _offset_ bytes into the file. Stores the number of _bytes_read_.
 *
 * Returns:
 * 	FS_E_OK     If we read any number of bytes.
 * 	FS_E_NOFILE If there is no file with that name.
 * 	FS_E_IO	    If there was some I/O error.
 */
VFSStatus raw_read
	(const char *path,
	void *buf,
	Uint32 *bytes_read,
	Uint32 offset,
	Uint32 nbytes);

/*
 * From the _offset_, write _nbytes_ from _buf_ into a file at _path_. Stores
 * the number of _bytes_read_.
 *
 * Returns:
 * 	E_OK           If we wrote any number of bytes.
 * 	FS_E_NOFILE    If there is no file with that name.
 * 	FS_E_BAD_START If the initial offset into the file doesn't yet exist.
 * 	E_IO           If there was some I/O error.
 */
VFSStatus raw_write
	(const char *path,
	const void *buf,
	Uint32 *bytes_written,
	Uint32 offset,
	Uint32 nbytes);

#endif // VFS_H
