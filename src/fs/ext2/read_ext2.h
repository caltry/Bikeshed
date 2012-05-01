#ifndef _READ_EXT2_H
#define _READ_EXT2_H

#include "types.h"
#include "ext2.h"

struct ext2_superblock *get_superblock( Uint32 filesystem_start );

/*
 * Dump interesting superblock data from the filesyste, to see if we're reading
 * it properly.
 *
 * Like the output from mke2fs:
 * Filesystem label=
 * OS type: Linux
 * Block size=1024 (log=0)
 * Fragment size=1024 (log=0)
 * Stride=0 blocks, Stripe width=0 blocks
 * 2048 inodes, 8192 blocks
 * 409 blocks (4.99%) reserved for the super user
 * First data block=1
 * Maximum filesystem blocks=8388608
 * 1 block group
 * 8192 blocks per group, 8192 fragments per group
 * 2048 inodes per group
 */
void print_superblock_data(struct ext2_superblock *sb);

void ext2_debug_dump( void *virtual_address );

#endif // _READ_EXT2_H
