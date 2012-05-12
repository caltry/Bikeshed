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

// Structure sizes, in bytes
#define EXT2_SUPERBLOCK_SIZE 1024
#define EXT2_BLOCK_GROUP_DESCRIPTOR_SIZE 32
#define EXT2_INODE_SIZE 128

// Important locations, relative to the start of the filesystem
#define EXT2_SUPERBLOCK_LOCATION 0x400

#define EXT2_INODE_DIRECT_BLOCKS	12
#define EXT2_INODE_INDIRECT_BLOCKS	1
#define EXT2_INODE_DOUBLE_INDIRECT_BLOCKS	1
#define EXT2_INODE_TRIPLE_INDIRECT_BLOCKS	1
#define EXT2_INODE_TOTAL_BLOCKS (EXT2_INODE_DIRECT_BLOCKS \
				+ EXT2_INODE_INDIRECT_BLOCKS \
				+ EXT2_INODE_DOUBLE_INDIRECT_BLOCKS \
				+ EXT2_INODE_TRIPLE_INDIRECT_BLOCKS)

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x1

#define DIRECTORY_SEPARATOR '/'

/*
 * A hook for the kernel to call when enabling ext2.
 */
void _fs_ext2_init(void);

typedef Uint32 BlockNumber;

struct ext2_superblock {
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
	Uint16 mnt_count;

	// Number of times the filesystem may be mounted before a fsck is forced.
	Uint16 max_mnt_count;

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

	// Make sure we're aligned properly.
	Uint16 padding1;

	// UUID of the (ext3) journal superblock.
	Uint8 journal_uuid[16];

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

	// More padding
	Uint8 reserved_char_pad;
	Uint16 reserved_word_pad;

	// Default mount options for the filesystem
	Uint32 default_mount_options;

	// Block group ID of the first meta block group. (ext3).
	Uint32 first_meta_blockgroup;

	Uint8 reserved[760];
};


// Each block group has a copy of the group descripor table. Each index of the
// GDT points to a group descriptor for a different block group.

struct ext2_block_group_descriptor {
	// Block number of the block allocation bitmap for this block group
	BlockNumber block_bitmap;

	// Block where the inode allocation bitmap for this block group is.
	BlockNumber inode_bitmap;

	// Block number containing the first block of the inode table for this
	// block group
	BlockNumber inode_table_start;

	// Number of free blocks in this block group
	Uint16 free_blocks_count;

	// Number of free indoes in this block group
	Uint16 free_inodes_count;

	// Number of inodes allocated to directories in this block group
	Uint16 used_directory_count;

	// Padding to put this structure on a 32bit boundary.
	Uint16 padding;

	// 12 bytes of space reserved for future revisions.
	Uint8 reserved[12];
};

typedef enum {
	/*
	 * File type
	 */
	socket	= 0xC000,	// This is a socket
	symlink	= 0xA000,
	regular	= 0x8000,
	block	= 0x6000,
	directory	= 0x4000,
	character	= 0x2000,	// Character device
	fifo	= 0x1000,	// Named pipe

	/*
	 * Special permission override bits
	 */
	suid	= 0x0800,
	sgid	= 0x0400,
	sticky	= 0x0200,

	/*
	 * Access rights
	 */
	user_read	= 0x0100,
	user_write	= 0x0080,
	user_exec	= 0x0040,
	group_read	= 0x0020,
	group_write	= 0x0010,
	group_exec	= 0x0008,
	other_read	= 0x0004,
	other_write	= 0x0002,
	other_exec	= 0x0001
} FileModeBitmasks;

typedef enum {
	secure_deletion	= 0x1,
	undelete	= 0x2,	// Record for undelete
	compressed	= 0x4,	// File is compressed
	sync_updates	= 0x8,	// Sync writes to disk immediately
	blocks_immutable= 0x10,	// Blocks for this file will not change
	append_only	= 0x20, // Writing can only append to the file
	do_not_delete	= 0x40, // Never delete this file
	no_atime	= 0x80,	// Never update the atime for this file
	// Reserved for compression
	dirty		= 0x100,
	compressed_blks	= 0x200, // One or more blocks are compressed
	no_uncompress	= 0x400, // Do not uncompress compressed data
	compression_err	= 0x800, // An error was detected during decompression
	// End of compression flags
	b_tree_dir	= 0x1000, // Directory formatted as a b-tree
	index		= 0x1000, // hash-indexed directory
	imagic_fl	= 0x2000, // AFS directory
	journal_data	= 0x4000, // Journal file data
	reserved	= 0x80000000
} inode_flags;

/*
 * Reserved inodes
 */
#define EXT2_INODE_BAD  1	// Inode for bad blocks
#define EXT2_INODE_ROOT 2	// The inode where '/' is located

struct ext2_inode {
	// File mode bitmasks
	Uint16 mode;

	// Owner
	Uint16 uid;

	// File size in bytes
	Uint32 size;

	// UNIX time since this inode was accessed
	Int32 atime;

	// UNIX creation time
	Int32 ctime;

	// Last modification time
	Int32 mtime;

	// Time when the file was deleted
	Int32 dtime;

	// POSIX group for this file
	Uint16 gid;

	// Number of times this inode is referred to.
	Uint16 incident_links;

	// Number of 512 byte data blocks
	Uint32 num_blocks;

	// File flags (see the inode_flags enum)
	Uint32 flags;

	// First OS dependent value
	Uint32 osdependent1;

	// The first 12 are (pointers to) direct blocks.
	// blocks[12] is a block containing an array of block ids.
	// blocks[13] is a double-indirect block: pointing to a block with an
	// 	index of indirect blocks.
	// blocks[14] is a triple-indirect block: pointing to a block with an
	// 	index of double-indirect blocks.
	Uint32 blocks[EXT2_INODE_TOTAL_BLOCKS];

	// Used by NFS
	Uint32 file_version;

	// Block number containing the extended attributes.
	Uint32 file_acl;

	union {
	    // Block number containing extended attributes
	    Uint32 dir_acl;

	    // high 32 bits of the 64 bit file size
	    Uint32 upper_file_size;
	};

	// Location of the file fragment. Always 0.
	Uint32 file_fragment;

	union {
	    struct {
		// Always 0
		Uint8 frag;
		Uint8 frag_size;

		Uint16 reserved1;

		// High bytes of UID and GID
		Uint16 high_uid;
		Uint16 high_gid;

		Uint32 reserved2;
	    } Linux2;
	} osdependent2;

};

typedef enum {
	FT_UNKNOWN	= 0,
	FT_REGULAR	= 1,
	FT_DIR		= 2,
	FT_CHARACTER	= 3,	// Character device
	FT_BLOCK	= 4,	// Block device
	FT_FIFO		= 5,
	FT_SOCKET	= 6,
	FT_SYMLINK	= 7
} DirectoryEntryFiletype;

struct ext2_directory_entry {
	// Inode where this directory entry is located.
	Uint32 inode_number;

	// Displacement to the next directory entry.
	Uint16 entry_length;

	// Number of characters are in the name
	Uint8 name_length;

	// The DirectoryEntryFiletype
	Uint8 file_type;

	// Name of the file
	char filename[];
};


/*
 * Data for manipulating a specific ext2 filesystem.
 */
struct ext2_filesystem_context {
	// The superblock for the filesystem
	struct ext2_superblock *sb;

	// The base address of the filesystem
	Uint32 base_address;

	char mountpoint[];
};

#endif // _EXT2_H
