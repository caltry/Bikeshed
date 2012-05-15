#include "mount.h"
#include "ext2.h"
#include "kernel/memory/kmalloc.h"
#include "kernel/lib/klib.h"
#include "kernel/lib/string.h"
#include "read_ext2.h"

struct ext2_filesystem_context*
ext2_mount_ramdisk( void *virtual_address )
{
	struct ext2_filesystem_context *context;
	context = __kmalloc( sizeof(struct ext2_filesystem_context) );

	context->sb = get_superblock( (Uint32) virtual_address );
	context->base_address = (Uint32) virtual_address;

	return context;
}
