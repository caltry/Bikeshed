/*
 * Implementation for the Virtual Filesystem Interface
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#include "ext2/read_ext2.h"
#include "vfs.h"

VFSStatus raw_read
	(const char *path,
	void *buf,
	Uint32 *bytes_read,
	Uint32 offset,
	Uint32 nbytes)
{
	VFSStatus vfs_read_status;

	// Right now we only support the 1 bikeshed ramdisk.
	ext2_read_status read_error;
	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		path,
		buf,
		bytes_read,
		offset,
		nbytes);

	switch( read_error )
	{
		case EXT2_READ_SUCCESS:
			vfs_read_status = FS_E_OK;
			break;
		case EXT2_READ_FILE_NOT_FOUND:
			vfs_read_status = FS_E_NOFILE;
			break;
		case EXT2_READ_NO_LEADING_SLASH:
			vfs_read_status = FS_E_NOT_FQN;
			break;
		default:
			vfs_read_status = FS_E_IO;
	}

	return vfs_read_status;
}
