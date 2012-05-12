/*
 * Mounting functions for ext2 filesystems.
 *
 * David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#ifndef _EXT2_MOUNT_H
#define _EXT2_MOUNT_H

/*
 * Mount a ramdisk found at the given virtual address onto the _mountpoint_
 */
struct ext2_filesystem_context*
ext2_mount_ramdisk( void *virtual_address );

#endif //_EXT2_MOUNT_H
