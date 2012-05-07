/*
 * All of the stuff that you need for mounting and unmounting filesystems.
 *
 * David Larsen <dcl9934@cs.rit.edu> May, 2012
 */

#ifndef _MOUNT_H
#define _MOUNT_H

#include "types.h"

typedef enum {
	mount_success	= 0,
	mount_bad_fs	= 1	// Unknown or unsupported filesystem
} MountStatus;

/*
 * Mount a ramdisk locatated at the specified _address_ onto the _path_ in the
 * current filesystem.
 *
 * Returns
 * 	mount_success	=> Mounted successfuly at that address
 */
MountStatus mount_ramdisk( void *address, char *path );

#endif // _MOUNT_H
