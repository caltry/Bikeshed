/*
 * All of the stuff that you need for mounting and unmounting filesystems.
 *
 * David Larsen <dcl9934@cs.rit.edu> May, 2012
 */

#include "mount.h"

#include "ext2/ext2.h"
#include "ext2/mount.h"

// TODO need some sort of way to store mount points and their locations

typedef enum {
	fs_unknown = 0,
	fs_ext2 = EXT2_SUPER_MAGIC
} FSType;

static FSType get_filesystem_type( void *address )
{
	struct ext2_superblock *ext2_sb
		= (struct ext2_superblock*)(address + EXT2_SUPERBLOCK_LOCATION);
	if( ext2_sb->magic_number == EXT2_SUPER_MAGIC )
	{
		return EXT2_SUPER_MAGIC;
	} else {
		return fs_unknown;
	}
}

MountStatus mount_ramdisk( void *address, char *path )
{
	FSType fs_type = get_filesystem_type( address );

	if( fs_type == fs_ext2 )
	{
		struct ext2_filesystem_context *context =
			ext2_mount_ramdisk( address, path );
		// TODO: Keep track of where the filesystems are mounted
		(void)context;

		return mount_success;
	} else {
		return mount_bad_fs;
	}
}
