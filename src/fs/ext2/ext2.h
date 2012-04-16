/*
 * Second Extended Filesystem
 *
 * This is the "private" include file, with all of the gory data structures.
 *
 * Superblock structure derived from
 * 		"The Second Extended File System: Internal Layout"
 * 		http://www.nongnu.org/ext2-doc/ext2.html
 * 		David Poirier <instinc@gmail.com> 2001-2011
 *
 * David Larsen <dcl9934@cs.rit.edu> Sat, 14 Apr 2012
 */

#ifndef _EXT2_H
#define _EXT2_H

#include "types.h"

// Magic number for ext2 filesystems
#define EXT2_SUPER_MAGIC 0xEF53

typedef Uint32 BlockNumber;

struct ext2_superblock{
	// Total number of inodes in the FS.
	Uint32 inodes_count;

	// Total number of blocks in the FS.
	Uint32 blocks_count;

	// Total nubmer of blocks reserved for the super user.
	Uint32 reserved_blocks_count;

	// Total number of free blocks (including reserved ones).
	Uint32 free_blocks_count;

	// Number of free inodes over all of the block groups.
	Uint32 free_inodes_count;

	// Location of the first superblock
	Uint32 first_data_block;

	// Determines the block size: block_size = 1024 << logarithmic_block_size
	// Valid values (and sizes) are: 1 (1KiB), 2 (2KiB), 3 (4KiB), 4 (8KiB)
	Uint32 logarithmic_block_size;

	// Fragment size = 1024 << logarithmic_fragment_size
	// UNUSED, but RESERVED
	Int32 logarithmic_fragment_size;

	// Number of blocks per group. Used with first_data_block to set up block
	// group boundaries.
	Uint32 blocks_per_group;

	// Total number of (data block) fragments in a group. Used to determine
	// the size of the block bitmap for each block group.
	// UNUSED, but RESERVED
	Uint32 fragments_per_group;

	// Total number of inodes per group. Used to determine the size of the
	// inode bitmap in each block group. The inode bitmap must fit with within
	// a single block and evenly divide the number of inodes that can fit in a
	// block ( block_size / inode size).
	Uint32 inodes_per_group;

	// Signed UNIX time of the last time the system was mounted.
	Int32 m_time;

	// Signed UNIX time of the last write to the file system.
	Int32 w_time;

	// Number of times the filesystem was mounted since last fsck.
	Uint32 mnt_count;

	// Number of times the filesystem may be mounted before a fsck is forced.
	Uint32 max_mnt_count;

	// Magic number identifying the filesystem as ext2
	Uint16 magic_number;

	// Describes the mount state of the filesystem.
	// 1 => clean (and not mounted)
	// 2 => dirty (or mounted)
	Uint16 state;

	// Describes how error handling should be done
	// 1 => continue on errors
	// 2 => mount read only
	// 3 => kernel panic
	Uint16 error_handling;

	// Minor revision level
	Uint16 minor_revision;

	// The last (UNIX) time the filesystem was fscked.
	Int32 time_lastchecked;

	// Maximum UNIX time interval allowed between fscks.
	Int32 time_check_interval;

	// What OS created the filesystem?
	// 0 => Linux
	// 1 => HURD
	// 2 => MASIX
	// 3 => FreeBSD
	// 4 => LITES
	Uint32 creator_os;

	// Major revision level
	// 0 => First revision
	// 1 => Second revision (variable inode sizes, other nice features)
	Uint32 major_revision;

	// What UID may use reserved blocks? (Typically 0)
	Uint16 reserved_uid;

	// What GID may use reserved blocks? (Typically 0)
	Uint16 reserved_gid;

	/*
	 * EXT2, revison 1 specific
	 */

	// Where's the first inode? (This is how we know where the root is)
	Uint32 first_inode;

	// How big are inodes? Must be a perfect power of 2 and <= block size
	Uint16 inode_size;

	// The block group that this superblock is a part of.
	Uint16 block_group_number;

	// Features we can support or not with no risk. (Indexing, Journaling,...)
	Uint32 features_nonbreaking;

	// Extended features that will break the filesystem if we're not
	// compatable.
	Uint32 features_breaking;

	// Extended features that will only break if we try to write to the
	// filesystem without properly supporting them. (Bitmask)
	// 1 => Sparse Superblock
	// 2 => 64-bit file sizes
	// 4 => Binary tree sorted directory files
	Uint32 features_read_only_nonbreaking;

	// 128bit value for the volume ID.
	Uint32 UUID[4];

	// 16 byte name of the volume. ISO-Latin-1 charset, NULL terminated.
	Uint8 volume_name[16];

	// 64 byte directory path indicating the last mount point.
	Uint8 last_mountpoint[64];

	// Bitmask for compression algorithm used.
	Uint32 compression_algorithm;

	// Number of blocks that should be pre-allocated for a file upon creation.
	Uint8 prealloc_blocks;

	// Number of blocks that should be pre-allocated when creating a directory.
	Uint8 prealloc_dir_blocks;

	// UUID of the (ext3) journal superblock.
	Uint16 journal_uuid;

	// Inode number for the (ext3) journal file.
	Uint32 journal_inode;

	// Device number for the (ext3) journal file.
	Uint32 journal_dev;

	// First inode to delete. Part of the (ext3) journaling system.
	Uint32 last_orphan;

	// Seeds used for the hash algorithm in directory indexing.
	Uint32 hash_seeds[4];

	// Default hash version for directory indexing.
	Uint8 hash_version;

	// Default mount options for the filesystem
	Uint32 default_mount_options;

	// Block group ID of the first meta block group. (ext3).
	Uint32 first_meta_blockgroup;
};


// Each block group has a copy of the group descripor table. Each index of the
// GDT points to a group descriptor for a different block group.

struct ext2_group_descriptor {
	// Block number of the block allocation bitmap for this block group
	BlockNumber blocks_bitmap;

	// Block where the inode allocation bitmap for this block group is.
	BlockNumber inode_bitmap;

	// Block number containing the first block of the inode table for this
	// block group
	BlockNumber inode_table_start;

	unsigned int free_blocks_count;
	unsigned int free_inodes_count;
	unsigned int used_directory_count;
};

struct ext2_inode {
	// Owner
	// Size
	// Mode
	// Timestamps
	// Direct blocks
	// Indirect blocks
	// Double indirect
	// Treble indirect block
};

struct ext2_data_block {
	// XXX
};

struct ext2_directory_entry {
	unsigned int inode_number;
	unsigned int entry_length;
	unsigned int name_length;
	char* filename;
};

#endif // _EXT2_H
